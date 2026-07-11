#pragma once

#include <QThread>
#include <QByteArray>
#include <atomic>

class AudioLoopbackCapture : public QThread
{
    Q_OBJECT
public:
    explicit AudioLoopbackCapture(QObject *parent = nullptr);
    ~AudioLoopbackCapture() override;

    void stopCapture();

signals:
    void audioFrameCaptured(const QByteArray &data, int sampleRate, int channels, int sampleSize, int sampleType);

protected:
    void run() override;

private:
    std::atomic<bool> m_running{false};
};
