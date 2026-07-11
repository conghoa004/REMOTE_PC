#include "AudioLoopbackCapture.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <ksmedia.h>
#include <combaseapi.h>

#pragma comment(lib, "ole32.lib")

// GUIDs for WASAPI
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

// Local definition of KSDATAFORMAT_SUBTYPE_IEEE_FLOAT to prevent MinGW link errors
const GUID Local_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
#else
#include <QAudioSource>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QProcess>

static QString getDefaultSinkMonitor()
{
    QProcess proc;
    proc.start("pactl", QStringList() << "get-default-sink");
    if (proc.waitForFinished(1000)) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            qDebug() << "Found default sink via pactl get-default-sink:" << output;
            return output + ".monitor";
        }
    }

    // Try pactl info as fallback
    proc.start("pactl", QStringList() << "info");
    if (proc.waitForFinished(1000)) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput());
        QStringList lines = output.split('\n');
        for (const QString &line : lines) {
            if (line.trimmed().startsWith("Default Sink:")) {
                QString sink = line.trimmed().mid(13).trimmed();
                qDebug() << "Found default sink via pactl info:" << sink;
                return sink + ".monitor";
            }
        }
    }

    qWarning() << "Failed to find default sink via pactl, falling back to @DEFAULT_SINK@.monitor";
    return "@DEFAULT_SINK@.monitor";
}
#endif

AudioLoopbackCapture::AudioLoopbackCapture(QObject *parent)
    : QThread(parent)
{
}

AudioLoopbackCapture::~AudioLoopbackCapture()
{
    stopCapture();
}

void AudioLoopbackCapture::stopCapture()
{
    m_running = false;
#ifndef Q_OS_WIN
    quit(); // Stop the event loop of the thread on Linux
#endif
    wait();
}

void AudioLoopbackCapture::run()
{
    m_running = true;

#ifdef Q_OS_WIN
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        qWarning() << "Failed to initialize COM:" << hr;
        return;
    }

    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDevice = nullptr;
    IAudioClient *pAudioClient = nullptr;
    IAudioCaptureClient *pCaptureClient = nullptr;
    WAVEFORMATEX *pwfx = nullptr;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator,
        nullptr,
        CLSCTX_ALL,
        IID_IMMDeviceEnumerator,
        (void**)&pEnumerator
    );
    if (FAILED(hr)) {
        qWarning() << "Failed to create MMDeviceEnumerator:" << hr;
        CoUninitialize();
        return;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        qWarning() << "Failed to get default audio endpoint:" << hr;
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = pDevice->Activate(
        IID_IAudioClient,
        CLSCTX_ALL,
        nullptr,
        (void**)&pAudioClient
    );
    if (FAILED(hr)) {
        qWarning() << "Failed to activate IAudioClient:" << hr;
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        qWarning() << "Failed to get mix format:" << hr;
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    int sampleType = 0; // 0 for Int, 1 for Float
    if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        WAVEFORMATEXTENSIBLE *pEx = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
        if (pEx->SubFormat == Local_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            sampleType = 1;
        }
    } else if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        sampleType = 1;
    }

    int sampleRate = pwfx->nSamplesPerSec;
    int channels = pwfx->nChannels;
    int sampleSize = pwfx->wBitsPerSample;

    REFERENCE_TIME hnsRequestedDuration = 10000000 / 10; // 100ms buffer

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsRequestedDuration,
        0,
        pwfx,
        nullptr
    );
    if (FAILED(hr)) {
        qWarning() << "Failed to initialize IAudioClient in loopback mode:" << hr;
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = pAudioClient->GetService(
        IID_IAudioCaptureClient,
        (void**)&pCaptureClient
    );
    if (FAILED(hr)) {
        qWarning() << "Failed to get IAudioCaptureClient service:" << hr;
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        qWarning() << "Failed to start IAudioClient:" << hr;
        pCaptureClient->Release();
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    while (m_running) {
        QThread::msleep(10); // Capture loop sleep

        UINT32 packetLength = 0;
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) {
            break;
        }

        while (packetLength > 0) {
            BYTE *pData = nullptr;
            UINT32 numFramesToRead = 0;
            DWORD flags = 0;

            hr = pCaptureClient->GetBuffer(
                &pData,
                &numFramesToRead,
                &flags,
                nullptr,
                nullptr
            );

            if (SUCCEEDED(hr)) {
                int bytesToRead = numFramesToRead * pwfx->nBlockAlign;
                if (bytesToRead > 0) {
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                        int silentSize = (sampleType == 1) ? (bytesToRead / 2) : bytesToRead;
                        QByteArray silentData(silentSize, 0);
                        emit audioFrameCaptured(silentData, sampleRate, channels, 16, 0);
                    } else {
                        if (sampleType == 1 && sampleSize == 32) {
                            // Convert 32-bit Float PCM to 16-bit Integer PCM for maximum compatibility and reduced bandwidth
                            int numSamples = bytesToRead / sizeof(float);
                            QByteArray convertedData;
                            convertedData.resize(numSamples * sizeof(int16_t));
                            
                            const float *src = reinterpret_cast<const float*>(pData);
                            int16_t *dst = reinterpret_cast<int16_t*>(convertedData.data());
                            
                            for (int i = 0; i < numSamples; ++i) {
                                float f = src[i];
                                if (f > 1.0f) f = 1.0f;
                                else if (f < -1.0f) f = -1.0f;
                                dst[i] = static_cast<int16_t>(f * 32767.0f);
                            }
                            
                            emit audioFrameCaptured(convertedData, sampleRate, channels, 16, 0);
                        } else if (sampleType == 0 && sampleSize == 32) {
                            // Convert 32-bit Integer PCM to 16-bit Integer PCM
                            int numSamples = bytesToRead / sizeof(int32_t);
                            QByteArray convertedData;
                            convertedData.resize(numSamples * sizeof(int16_t));
                            
                            const int32_t *src = reinterpret_cast<const int32_t*>(pData);
                            int16_t *dst = reinterpret_cast<int16_t*>(convertedData.data());
                            
                            for (int i = 0; i < numSamples; ++i) {
                                dst[i] = static_cast<int16_t>(src[i] >> 16);
                            }
                            
                            emit audioFrameCaptured(convertedData, sampleRate, channels, 16, 0);
                        } else {
                            QByteArray capturedData(reinterpret_cast<const char*>(pData), bytesToRead);
                            emit audioFrameCaptured(capturedData, sampleRate, channels, sampleSize, sampleType);
                        }
                    }
                }
                pCaptureClient->ReleaseBuffer(numFramesToRead);
            }

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                break;
            }
        }
    }

    pAudioClient->Stop();
    pCaptureClient->Release();
    CoTaskMemFree(pwfx);
    pAudioClient->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();
#else
    // Linux implementation using parec tool with fallback to QAudioSource
    QString deviceName = getDefaultSinkMonitor();
    QProcess parecProcess;
    QStringList arguments;
    arguments << "-d" << deviceName
              << "--format=s16le"
              << "--rate=48000"
              << "--channels=2";

    parecProcess.setProgram("parec");
    parecProcess.setArguments(arguments);
    parecProcess.start();

    bool started = parecProcess.waitForStarted(1000);
    bool crashed = false;
    if (started) {
        // Wait a short duration (300ms) to check if the process exits immediately with an error (e.g. invalid device name)
        crashed = parecProcess.waitForFinished(300);
    }

    if (!started || crashed) {
        if (!started) {
            qWarning() << "parec failed to start. Falling back to QAudioSource.";
        } else {
            qWarning() << "parec started but crashed immediately (exit code:" << parecProcess.exitCode() << "). Falling back to QAudioSource.";
        }
        
        QAudioFormat format;
        format.setSampleRate(48000);
        format.setChannelCount(2);
        format.setSampleFormat(QAudioFormat::Int16);

        QAudioDevice selectedDevice;
        const auto inputs = QMediaDevices::audioInputs();
        for (const auto &device : inputs) {
            QString name = device.description().toLower();
            QString id = device.id().toLower();
            if (name.contains("monitor") || id.contains("monitor") || name.contains("loopback") || id.contains("loopback")) {
                selectedDevice = device;
                break;
            }
        }
        if (selectedDevice.isNull()) {
            selectedDevice = QMediaDevices::defaultAudioInput();
        }

        QAudioSource *audioSource = new QAudioSource(selectedDevice, format);
        QIODevice *ioDevice = audioSource->start();

        if (!ioDevice) {
            qWarning() << "Failed to start QAudioSource fallback";
            delete audioSource;
            return;
        }

        QObject::connect(ioDevice, &QIODevice::readyRead, [=]() {
            if (!m_running) return;
            QByteArray data = ioDevice->readAll();
            if (!data.isEmpty()) {
                emit audioFrameCaptured(data, 48000, 2, 16, 0); // 16-bit Int PCM
            }
        });

        exec(); // Thread event loop for readyRead signal handling

        audioSource->stop();
        delete audioSource;
    } else {
        QObject::connect(&parecProcess, &QProcess::readyReadStandardOutput, [=, &parecProcess]() {
            if (!m_running) return;
            QByteArray data = parecProcess.readAllStandardOutput();
            if (!data.isEmpty()) {
                emit audioFrameCaptured(data, 48000, 2, 16, 0); // 16-bit Int PCM
            }
        });

        // Handle process finished/exit signal
        QObject::connect(&parecProcess, &QProcess::finished, [=](int exitCode, QProcess::ExitStatus exitStatus) {
            qWarning() << "parec process finished unexpectedly with exit code:" << exitCode << "status:" << exitStatus;
            const_cast<AudioLoopbackCapture*>(this)->quit();
        });

        // Run the QThread event loop so asynchronous QProcess signals can be processed
        exec();

        // Cleanup process on thread exit
        if (parecProcess.state() == QProcess::Running) {
            parecProcess.kill();
            parecProcess.waitForFinished(1000);
        }
    }
#endif
}
