#include "ConnectionManager.h"

#include <QRandomGenerator>
#include <QAudioSink>
#include <QMediaDevices>
#include <QIODevice>

#ifdef Q_OS_WIN
#include <windows.h>

static ConnectionManager* s_instance = nullptr;
static HHOOK s_keyboardHook = NULL;

static int vkToQtKey(WORD vk)
{
    if (vk >= 'A' && vk <= 'Z')
        return Qt::Key_A + (vk - 'A');
    if (vk >= '0' && vk <= '9')
        return Qt::Key_0 + (vk - '0');
    if (vk >= VK_F1 && vk <= VK_F12)
        return Qt::Key_F1 + (vk - VK_F1);

    switch (vk) {
    case VK_ESCAPE:    return Qt::Key_Escape;
    case VK_TAB:       return Qt::Key_Tab;
    case VK_BACK:      return Qt::Key_Backspace;
    case VK_RETURN:    return Qt::Key_Return;
    case VK_INSERT:    return Qt::Key_Insert;
    case VK_DELETE:    return Qt::Key_Delete;
    case VK_PAUSE:     return Qt::Key_Pause;
    case VK_SNAPSHOT:  return Qt::Key_Print;
    case VK_HOME:      return Qt::Key_Home;
    case VK_END:       return Qt::Key_End;
    case VK_LEFT:      return Qt::Key_Left;
    case VK_UP:        return Qt::Key_Up;
    case VK_RIGHT:     return Qt::Key_Right;
    case VK_DOWN:      return Qt::Key_Down;
    case VK_PRIOR:     return Qt::Key_PageUp;
    case VK_NEXT:      return Qt::Key_PageDown;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:    return Qt::Key_Shift;
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:  return Qt::Key_Control;
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:     return Qt::Key_Alt;
    case VK_CAPITAL:   return Qt::Key_CapsLock;
    case VK_NUMLOCK:   return Qt::Key_NumLock;
    case VK_SCROLL:    return Qt::Key_ScrollLock;
    case VK_SPACE:     return Qt::Key_Space;
    case VK_LWIN:
    case VK_RWIN:      return Qt::Key_Meta;
    case VK_APPS:      return Qt::Key_Menu;
    case VK_OEM_MINUS: return Qt::Key_Minus;
    case VK_OEM_PLUS:  return Qt::Key_Equal;
    case VK_OEM_COMMA: return Qt::Key_Comma;
    case VK_OEM_PERIOD: return Qt::Key_Period;
    case VK_OEM_2:      return Qt::Key_Slash;
    case VK_OEM_5:      return Qt::Key_Backslash;
    case VK_OEM_1:      return Qt::Key_Semicolon;
    case VK_OEM_7:      return Qt::Key_Apostrophe;
    case VK_OEM_4:      return Qt::Key_BracketLeft;
    case VK_OEM_6:      return Qt::Key_BracketRight;
    case VK_OEM_3:      return Qt::Key_QuoteLeft;
    default:           return 0;
    }
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && s_instance != nullptr)
    {
        HWND activeWnd = GetForegroundWindow();
        DWORD activeProcId = 0;
        GetWindowThreadProcessId(activeWnd, &activeProcId);
        
        if (activeProcId == GetCurrentProcessId())
        {
            wchar_t windowTitle[256] = {0};
            if (GetWindowTextW(activeWnd, windowTitle, 256) > 0)
            {
                if (wcscmp(windowTitle, L"Remote Desktop Connection") == 0)
                {
                    KBDLLHOOKSTRUCT* pKey = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
                    int qtKey = vkToQtKey(static_cast<WORD>(pKey->vkCode));
                    
                    if (qtKey != 0)
                    {
                        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
                        {
                            s_instance->sendKeyPress(qtKey);
                        }
                        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
                        {
                            s_instance->sendKeyRelease(qtKey);
                        }
                    }
                    
                    // Consume the key so that local OS and local hooks (like UniKey) don't receive it
                    return 1;
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void installKeyboardHook()
{
    if (s_keyboardHook == NULL)
    {
        s_keyboardHook = SetWindowsHookEx(
            WH_KEYBOARD_LL,
            LowLevelKeyboardProc,
            GetModuleHandle(NULL),
            0
        );
    }
}

static void uninstallKeyboardHook()
{
    if (s_keyboardHook != NULL)
    {
        UnhookWindowsHookEx(s_keyboardHook);
        s_keyboardHook = NULL;
    }
}
#endif

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    s_instance = this;
#endif

    connect(&m_server,
            &RemoteServer::clientConnected,
            this,
            &ConnectionManager::clientConnected);

    connect(&m_client,
            &RemoteClient::frameReceived,
            this,
            &ConnectionManager::frameReceived);

    connect(&m_client,
            &RemoteClient::audioFrameReceived,
            this,
            &ConnectionManager::playAudioFrame);

    connect(&m_server,
            &RemoteServer::clientDisconnected,
            this,
            &ConnectionManager::clientDisconnected);

    connect(&m_client,
            &RemoteClient::connected,
            this,
            [this]()
            {
                emit connectedChanged();

                setState(ConnectionState::Authenticating);
            });

    connect(&m_client,
            &RemoteClient::authenticated,
            this,
            [this]()
            {
                setState(ConnectionState::Connected);
            });

    connect(&m_client,
            &RemoteClient::authenticationFailed,
            this,
            [this]()
            {
                m_lastError = "Incorrect password";

                emit lastErrorChanged();

                setState(ConnectionState::Disconnected);
            });

    connect(&m_client,
            &RemoteClient::disconnected,
            this,
            [this]()
            {
                emit connectedChanged();

                setState(ConnectionState::Disconnected);
            });

    connect(&m_client,
            &RemoteClient::errorOccurred,
            this,
            [this](const QString &error)
            {
                qDebug() << error;

                if (m_lastError != error)
                {
                    m_lastError = error;
                    emit lastErrorChanged();
                }

                emit connectedChanged();

                setState(ConnectionState::Disconnected);
            });
}

ConnectionManager::~ConnectionManager()
{
#ifdef Q_OS_WIN
    uninstallKeyboardHook();
    if (s_instance == this)
    {
        s_instance = nullptr;
    }
#endif

    if (m_audioSink) {
        m_audioSink->stop();
        m_audioSink->deleteLater();
        m_audioSink = nullptr;
    }
}

bool ConnectionManager::hosting() const
{
    return m_hosting;
}

bool ConnectionManager::connected() const
{
    return m_client.isConnected();
}

quint16 ConnectionManager::port() const
{
    return m_port;
}

void ConnectionManager::setPort(quint16 port)
{
    if (m_port == port)
        return;

    m_port = port;

    emit portChanged();
}

QString ConnectionManager::password() const
{
    return m_password;
}

ConnectionManager::ConnectionState ConnectionManager::state() const
{
    return m_state;
}

QString ConnectionManager::lastError() const
{
    return m_lastError;
}

void ConnectionManager::setState(ConnectionState state)
{
    if (m_state == state)
        return;

    m_state = state;

#ifdef Q_OS_WIN
    if (m_state == ConnectionState::Connected)
    {
        installKeyboardHook();
    }
    else
    {
        uninstallKeyboardHook();
    }
#endif

    if (m_state != ConnectionState::Connected)
    {
        if (m_audioSink) {
            m_audioSink->stop();
            m_audioSink->deleteLater();
            m_audioSink = nullptr;
            m_audioIoDevice = nullptr;
        }
    }

    emit stateChanged();
}

bool ConnectionManager::startHost()
{
    if (m_hosting)
        return true;

    generatePassword();

    m_server.setPassword(m_password);

    if (!m_server.start(m_port))
        return false;

    m_hosting = true;

    emit hostingChanged();

    return true;
}

void ConnectionManager::stopHost()
{
    if (!m_hosting)
        return;

    m_server.stop();

    m_server.setPassword("");

    m_hosting = false;

    m_password.clear();
    emit passwordChanged();

    emit hostingChanged();
}

void ConnectionManager::connectToHost(const QString &ip,
                                      const QString &password)
{
    if (connected())
        return;

    m_lastError.clear();
    emit lastErrorChanged();

    setState(ConnectionState::Connecting);

    m_client.connectToHost(ip,
                           m_port,
                           password);
}

void ConnectionManager::disconnectFromHost()
{
    m_client.disconnectFromHost();
}

void ConnectionManager::generatePassword()
{
    m_password = QString("%1")
    .arg(QRandomGenerator::global()->bounded(1000000),
         6,
         10,
         QChar('0'));

    emit passwordChanged();

    if (m_hosting)
        m_server.setPassword(m_password);
}

bool ConnectionManager::losslessQuality() const
{
    return m_losslessQuality;
}

void ConnectionManager::setLosslessQuality(bool lossless)
{
    if (m_losslessQuality == lossless)
        return;

    m_losslessQuality = lossless;
    emit losslessQualityChanged();

    m_client.setLosslessQuality(lossless);
}

void ConnectionManager::sendMouseMove(int x, int y)
{
    m_client.sendMouseMove(x, y);
}

void ConnectionManager::sendMousePress(int button, int x, int y)
{
    m_client.sendMousePress(button, x, y);
}

void ConnectionManager::sendMouseRelease(int button, int x, int y)
{
    m_client.sendMouseRelease(button, x, y);
}

void ConnectionManager::sendMouseWheel(int delta)
{
    m_client.sendMouseWheel(delta);
}

void ConnectionManager::sendKeyPress(int key)
{
    m_client.sendKeyPress(key);
}

void ConnectionManager::sendKeyRelease(int key)
{
    m_client.sendKeyRelease(key);
}

bool ConnectionManager::isMuted() const
{
    return m_isMuted;
}

void ConnectionManager::setIsMuted(bool muted)
{
    if (m_isMuted == muted)
        return;

    m_isMuted = muted;
    emit isMutedChanged();

    if (m_audioSink) {
        if (m_isMuted) {
            m_audioSink->suspend();
        } else {
            m_audioSink->resume();
        }
    }
}

void ConnectionManager::playAudioFrame(const QByteArray &data, int sampleRate, int channels, int sampleSize, int sampleType)
{
    if (m_isMuted)
        return;

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    if (sampleType == 1) {
        format.setSampleFormat(QAudioFormat::Float);
    } else {
        if (sampleSize == 16) {
            format.setSampleFormat(QAudioFormat::Int16);
        } else if (sampleSize == 32) {
            format.setSampleFormat(QAudioFormat::Int32);
        } else {
            format.setSampleFormat(QAudioFormat::Int16);
        }
    }

    if (!m_audioSink || m_currentAudioFormat != format) {
        if (m_audioSink) {
            m_audioSink->stop();
            m_audioSink->deleteLater();
            m_audioSink = nullptr;
            m_audioIoDevice = nullptr;
        }

        m_currentAudioFormat = format;
        m_audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), m_currentAudioFormat, this);
        
        // Use a 350ms system buffer size for the audio device
        int bytesPerSecond = sampleRate * channels * (sampleSize / 8);
        int bufferSizeBytes = bytesPerSecond * 0.35; 
        m_audioSink->setBufferSize(bufferSizeBytes);
        
        if (m_isMuted) {
            m_audioSink->suspend();
        }
        
        m_audioIoDevice = m_audioSink->start();
        m_isBuffering = true;
        m_audioBuffer.clear();
    }

    // Trigger pre-buffering if the sink runs out of data (IdleState) to absorb network gaps
    if (m_audioSink && m_audioSink->state() == QAudio::IdleState) {
        if (!m_isBuffering) {
            m_isBuffering = true;
            m_audioBuffer.clear();
        }
    }

    if (m_isBuffering) {
        m_audioBuffer.append(data);
        
        int bytesPerSecond = sampleRate * channels * (sampleSize / 8);
        int prebufferThreshold = bytesPerSecond * 0.25; // Pre-buffer 250ms of audio before starting play
        
        if (m_audioBuffer.size() >= prebufferThreshold) {
            m_isBuffering = false;
            if (m_audioIoDevice && m_audioIoDevice->isOpen()) {
                m_audioIoDevice->write(m_audioBuffer);
            }
            m_audioBuffer.clear();
        }
    } else {
        if (m_audioIoDevice && m_audioIoDevice->isOpen()) {
            m_audioIoDevice->write(data);
            
            if (m_audioSink && m_audioSink->state() == QAudio::StoppedState) {
                if (m_audioSink->error() != QAudio::NoError) {
                    qWarning() << "ConnectionManager: QAudioSink StoppedState due to error:" << m_audioSink->error();
                }
            }
        }
    }
}