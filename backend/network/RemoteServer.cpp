#include "RemoteServer.h"

#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QDataStream>
#include <QClipboard>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
// X11 defines KeyPress/KeyRelease as macros — conflict with our enum
#undef KeyPress
#undef KeyRelease
#endif

// ============================================================
// Qt Key to native key code mapping
// ============================================================

#ifdef Q_OS_WIN
static WORD qtKeyToVk(int qtKey)
{
    // Letters A-Z
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        return static_cast<WORD>('A' + (qtKey - Qt::Key_A));

    // Numbers 0-9
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
        return static_cast<WORD>('0' + (qtKey - Qt::Key_0));

    // F keys
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12)
        return static_cast<WORD>(VK_F1 + (qtKey - Qt::Key_F1));

    switch (qtKey) {
    case Qt::Key_Escape:    return VK_ESCAPE;
    case Qt::Key_Tab:       return VK_TAB;
    case Qt::Key_Backspace: return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter:     return VK_RETURN;
    case Qt::Key_Insert:    return VK_INSERT;
    case Qt::Key_Delete:    return VK_DELETE;
    case Qt::Key_Pause:     return VK_PAUSE;
    case Qt::Key_Print:     return VK_SNAPSHOT;
    case Qt::Key_Home:      return VK_HOME;
    case Qt::Key_End:       return VK_END;
    case Qt::Key_Left:      return VK_LEFT;
    case Qt::Key_Up:        return VK_UP;
    case Qt::Key_Right:     return VK_RIGHT;
    case Qt::Key_Down:      return VK_DOWN;
    case Qt::Key_PageUp:    return VK_PRIOR;
    case Qt::Key_PageDown:  return VK_NEXT;
    case Qt::Key_Shift:     return VK_SHIFT;
    case Qt::Key_Control:   return VK_CONTROL;
    case Qt::Key_Alt:       return VK_MENU;
    case Qt::Key_CapsLock:  return VK_CAPITAL;
    case Qt::Key_NumLock:   return VK_NUMLOCK;
    case Qt::Key_ScrollLock: return VK_SCROLL;
    case Qt::Key_Space:     return VK_SPACE;
    case Qt::Key_Meta:      return VK_LWIN;
    case Qt::Key_Menu:      return VK_APPS;
    case Qt::Key_Minus:     return VK_OEM_MINUS;
    case Qt::Key_Plus:
    case Qt::Key_Equal:     return VK_OEM_PLUS;
    case Qt::Key_Comma:     return VK_OEM_COMMA;
    case Qt::Key_Period:    return VK_OEM_PERIOD;
    case Qt::Key_Slash:     return VK_OEM_2;
    case Qt::Key_Backslash: return VK_OEM_5;
    case Qt::Key_Semicolon: return VK_OEM_1;
    case Qt::Key_Apostrophe: return VK_OEM_7;
    case Qt::Key_BracketLeft: return VK_OEM_4;
    case Qt::Key_BracketRight: return VK_OEM_6;
    case Qt::Key_QuoteLeft: return VK_OEM_3; // backtick/tilde
    default: return 0;
    }
}
#endif

#ifdef Q_OS_LINUX
static KeySym qtKeyToKeysym(int qtKey)
{
    // Letters a-z (X11 keysyms are lowercase)
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        return static_cast<KeySym>(XK_a + (qtKey - Qt::Key_A));

    // Numbers 0-9
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
        return static_cast<KeySym>(XK_0 + (qtKey - Qt::Key_0));

    // F keys
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12)
        return static_cast<KeySym>(XK_F1 + (qtKey - Qt::Key_F1));

    switch (qtKey) {
    case Qt::Key_Escape:    return XK_Escape;
    case Qt::Key_Tab:       return XK_Tab;
    case Qt::Key_Backspace: return XK_BackSpace;
    case Qt::Key_Return:
    case Qt::Key_Enter:     return XK_Return;
    case Qt::Key_Insert:    return XK_Insert;
    case Qt::Key_Delete:    return XK_Delete;
    case Qt::Key_Pause:     return XK_Pause;
    case Qt::Key_Print:     return XK_Print;
    case Qt::Key_Home:      return XK_Home;
    case Qt::Key_End:       return XK_End;
    case Qt::Key_Left:      return XK_Left;
    case Qt::Key_Up:        return XK_Up;
    case Qt::Key_Right:     return XK_Right;
    case Qt::Key_Down:      return XK_Down;
    case Qt::Key_PageUp:    return XK_Page_Up;
    case Qt::Key_PageDown:  return XK_Page_Down;
    case Qt::Key_Shift:     return XK_Shift_L;
    case Qt::Key_Control:   return XK_Control_L;
    case Qt::Key_Alt:       return XK_Alt_L;
    case Qt::Key_CapsLock:  return XK_Caps_Lock;
    case Qt::Key_NumLock:   return XK_Num_Lock;
    case Qt::Key_ScrollLock: return XK_Scroll_Lock;
    case Qt::Key_Space:     return XK_space;
    case Qt::Key_Meta:      return XK_Super_L;
    case Qt::Key_Menu:      return XK_Menu;
    case Qt::Key_Minus:     return XK_minus;
    case Qt::Key_Plus:      return XK_plus;
    case Qt::Key_Equal:     return XK_equal;
    case Qt::Key_Comma:     return XK_comma;
    case Qt::Key_Period:    return XK_period;
    case Qt::Key_Slash:     return XK_slash;
    case Qt::Key_Backslash: return XK_backslash;
    case Qt::Key_Semicolon: return XK_semicolon;
    case Qt::Key_Apostrophe: return XK_apostrophe;
    case Qt::Key_BracketLeft: return XK_bracketleft;
    case Qt::Key_BracketRight: return XK_bracketright;
    case Qt::Key_QuoteLeft: return XK_grave;
    default: return NoSymbol;
    }
}
#endif

// ============================================================
// RemoteServer implementation
// ============================================================

RemoteServer::RemoteServer(QObject *parent)
    : QObject(parent)
{
    connect(&m_server,
            &QTcpServer::newConnection,
            this,
            &RemoteServer::onNewConnection);

    connect(&m_captureTimer,
            &QTimer::timeout,
            this,
            &RemoteServer::captureAndBroadcast);

    connect(&m_audioCapture,
            &AudioLoopbackCapture::audioFrameCaptured,
            this,
            &RemoteServer::onAudioFrameCaptured);

    connect(QGuiApplication::clipboard(),
            &QClipboard::dataChanged,
            this,
            &RemoteServer::onClipboardChanged);

#ifdef Q_OS_LINUX
    m_display = XOpenDisplay(nullptr);
#endif
}

RemoteServer::~RemoteServer()
{
#ifdef Q_OS_LINUX
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
#endif
}

bool RemoteServer::start(quint16 port)
{
    if (m_server.isListening())
        return true;

    return m_server.listen(QHostAddress::AnyIPv4, port);
}

void RemoteServer::stop()
{
    m_captureTimer.stop();
    m_audioCapture.stopCapture();

    for (QTcpSocket *socket : m_clients)
    {
        socket->disconnectFromHost();
        socket->deleteLater();
    }

    m_clients.clear();
    m_authenticated.clear();
    m_streams.clear();
    m_clientCompression.clear();
    m_previousFrame = QImage();

    m_server.close();
}

bool RemoteServer::isRunning() const
{
    return m_server.isListening();
}

void RemoteServer::setPassword(const QString &password)
{
    m_password = password;
}

void RemoteServer::onNewConnection()
{
    while (m_server.hasPendingConnections())
    {
        QTcpSocket *socket = m_server.nextPendingConnection();

        m_clients.append(socket);
        m_authenticated[socket] = false;
        m_clientCompression[socket] = Protocol::CompressionType::HighQuality;

        socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

        // Create PacketStream for this socket
        PacketStream *stream = new PacketStream(socket, this);
        m_streams[socket] = stream;

        connect(stream,
                &PacketStream::packetReceived,
                this,
                [this, socket](Packet packet)
                {
                    onPacketReceived(socket, packet);
                });

        connect(socket,
                &QTcpSocket::disconnected,
                this,
                &RemoteServer::onDisconnected);
    }
}

void RemoteServer::onDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket)
        return;

    m_clients.removeOne(socket);
    m_authenticated.remove(socket);
    m_clientCompression.remove(socket);

    if (m_streams.contains(socket)) {
        m_streams[socket]->deleteLater();
        m_streams.remove(socket);
    }

    emit clientDisconnected(socket);

    socket->deleteLater();

    // Stop capture timer if no authenticated clients remain
    bool hasAuthenticated = false;
    for (auto it = m_authenticated.begin(); it != m_authenticated.end(); ++it) {
        if (it.value()) {
            hasAuthenticated = true;
            break;
        }
    }

    if (!hasAuthenticated) {
        m_captureTimer.stop();
        m_previousFrame = QImage();
        m_audioCapture.stopCapture();
    }
}

void RemoteServer::onPacketReceived(QTcpSocket *socket, Packet packet)
{
    switch (packet.type)
    {
    case Protocol::PacketType::AuthRequest:
    {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        QString password;
        in >> password;

        if (password == m_password)
        {
            m_authenticated[socket] = true;

            sendAuthSuccess(socket);

            emit clientConnected(socket);

            // Start capture timer if not already running
            if (!m_captureTimer.isActive()) {
                m_previousFrame = QImage();
                m_captureTimer.start(60); // ~16 FPS
            }

            if (!m_audioCapture.isRunning()) {
                m_audioCapture.start();
            }
        }
        else
        {
            sendAuthFailed(socket);
            socket->disconnectFromHost();
        }

        break;
    }

    case Protocol::PacketType::SettingsUpdate:
    {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        quint8 compressionType;
        in >> compressionType;

        m_clientCompression[socket] = static_cast<Protocol::CompressionType>(compressionType);

        // Reset previous frame to force a full frame send with new compression
        m_previousFrame = QImage();

        break;
    }

    // ===== Mouse events =====
    case Protocol::PacketType::MouseMove:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        qint32 x, y;
        in >> x >> y;

        simulateMouseMove(x, y);
        break;
    }

    case Protocol::PacketType::MousePress:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        quint32 button;
        qint32 x, y;
        in >> button >> x >> y;

        simulateMousePress(button, x, y);
        break;
    }

    case Protocol::PacketType::MouseRelease:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        quint32 button;
        qint32 x, y;
        in >> button >> x >> y;

        simulateMouseRelease(button, x, y);
        break;
    }

    case Protocol::PacketType::MouseWheel:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        qint32 delta;
        in >> delta;

        simulateMouseWheel(delta);
        break;
    }

    // ===== Keyboard events =====
    case Protocol::PacketType::KeyPress:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        qint32 key;
        in >> key;

        simulateKeyPress(key);
        break;
    }

    case Protocol::PacketType::KeyRelease:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        qint32 key;
        in >> key;

        simulateKeyRelease(key);
        break;
    }

    case Protocol::PacketType::Clipboard:
    {
        if (!m_authenticated.value(socket, false))
            break;

        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        QString text;
        in >> text;

        m_lastIncomingClipboard = text;
        QGuiApplication::clipboard()->setText(text);

        // Broadcast to all other authenticated clients
        for (QTcpSocket *clientSocket : m_clients) {
            if (clientSocket != socket && m_authenticated.value(clientSocket, false)) {
                m_streams[clientSocket]->send(Protocol::PacketType::Clipboard, packet.payload);
            }
        }
        break;
    }

    default:
        break;
    }
}

// ============================================================
// Auth helpers
// ============================================================

void RemoteServer::sendAuthSuccess(QTcpSocket *socket)
{
    if (!m_streams.contains(socket))
        return;

    m_streams[socket]->send(Protocol::PacketType::AuthSuccess, QByteArray());
}

void RemoteServer::sendAuthFailed(QTcpSocket *socket)
{
    if (!m_streams.contains(socket))
        return;

    m_streams[socket]->send(Protocol::PacketType::AuthFailed, QByteArray());
}

void RemoteServer::sendScreenFrame(QTcpSocket *socket, const QByteArray &framePayload)
{
    if (!m_streams.contains(socket))
        return;

    m_streams[socket]->send(Protocol::PacketType::ScreenFrame, framePayload);
}

QByteArray RemoteServer::compressFrame(const QImage &image, Protocol::CompressionType type)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << static_cast<quint8>(type);
    out << static_cast<quint32>(image.width());
    out << static_cast<quint32>(image.height());

    if (type == Protocol::CompressionType::Lossless)
    {
        // Convert to RGB32 and compress with zlib
        QImage rgb32 = image.convertToFormat(QImage::Format_RGB32);
        QByteArray raw(reinterpret_cast<const char*>(rgb32.constBits()),
                       rgb32.sizeInBytes());
        QByteArray compressed = qCompress(raw, 1); // Level 1 = fastest
        out << compressed;
    }
    else
    {
        // Compress as JPEG quality 90
        QByteArray jpegData;
        QBuffer buffer(&jpegData);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "JPEG", 90);
        out << jpegData;
    }

    return payload;
}

void RemoteServer::captureAndBroadcast()
{
    // Check if there are any authenticated clients
    bool hasAuthenticatedClients = false;
    for (auto it = m_authenticated.begin(); it != m_authenticated.end(); ++it) {
        if (it.value()) {
            hasAuthenticatedClients = true;
            break;
        }
    }

    if (!hasAuthenticatedClients)
        return;

    // Capture primary screen
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    QImage currentFrame = screen->grabWindow(0).toImage();

    if (currentFrame.isNull())
        return;

    // Skip if frame is identical to previous (fast comparison)
    if (!m_previousFrame.isNull()
        && m_previousFrame.size() == currentFrame.size()
        && m_previousFrame.format() == currentFrame.format()
        && m_previousFrame.sizeInBytes() == currentFrame.sizeInBytes())
    {
        if (memcmp(m_previousFrame.constBits(),
                   currentFrame.constBits(),
                   currentFrame.sizeInBytes()) == 0)
        {
            return; // No change, skip this frame
        }
    }

    m_previousFrame = currentFrame;

    // Pre-compress for each compression type that is needed
    QHash<Protocol::CompressionType, QByteArray> compressedFrames;

    for (QTcpSocket *socket : m_clients)
    {
        if (!m_authenticated.value(socket, false))
            continue;

        Protocol::CompressionType ct = m_clientCompression.value(
            socket, Protocol::CompressionType::HighQuality);

        if (!compressedFrames.contains(ct))
        {
            compressedFrames[ct] = compressFrame(currentFrame, ct);
        }

        sendScreenFrame(socket, compressedFrames[ct]);
    }
}

// ============================================================
// Input simulation — Windows
// ============================================================

#ifdef Q_OS_WIN

void RemoteServer::simulateMouseMove(int x, int y)
{
    SetCursorPos(x, y);
}

void RemoteServer::simulateMousePress(int button, int x, int y)
{
    SetCursorPos(x, y);

    INPUT input = {};
    input.type = INPUT_MOUSE;

    switch (button) {
    case Qt::LeftButton:   input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; break;
    case Qt::RightButton:  input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; break;
    case Qt::MiddleButton: input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN; break;
    default: return;
    }

    SendInput(1, &input, sizeof(INPUT));
}

void RemoteServer::simulateMouseRelease(int button, int x, int y)
{
    SetCursorPos(x, y);

    INPUT input = {};
    input.type = INPUT_MOUSE;

    switch (button) {
    case Qt::LeftButton:   input.mi.dwFlags = MOUSEEVENTF_LEFTUP; break;
    case Qt::RightButton:  input.mi.dwFlags = MOUSEEVENTF_RIGHTUP; break;
    case Qt::MiddleButton: input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP; break;
    default: return;
    }

    SendInput(1, &input, sizeof(INPUT));
}

void RemoteServer::simulateMouseWheel(int delta)
{
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = static_cast<DWORD>(delta);

    SendInput(1, &input, sizeof(INPUT));
}

void RemoteServer::simulateKeyPress(int qtKey)
{
    WORD vk = qtKeyToVk(qtKey);
    if (vk == 0) return;

    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;

    SendInput(1, &input, sizeof(INPUT));
}

void RemoteServer::simulateKeyRelease(int qtKey)
{
    WORD vk = qtKeyToVk(qtKey);
    if (vk == 0) return;

    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &input, sizeof(INPUT));
}

#endif // Q_OS_WIN

// ============================================================
// Input simulation — Linux (X11 + XTest)
// ============================================================

#ifdef Q_OS_LINUX

void RemoteServer::simulateMouseMove(int x, int y)
{
    if (!m_display) return;

    XTestFakeMotionEvent(m_display, DefaultScreen(m_display), x, y, CurrentTime);
    XFlush(m_display);
}

void RemoteServer::simulateMousePress(int button, int x, int y)
{
    if (!m_display) return;

    XTestFakeMotionEvent(m_display, DefaultScreen(m_display), x, y, CurrentTime);

    unsigned int xButton = 1; // Left
    switch (button) {
    case Qt::LeftButton:   xButton = 1; break;
    case Qt::MiddleButton: xButton = 2; break;
    case Qt::RightButton:  xButton = 3; break;
    default: return;
    }

    XTestFakeButtonEvent(m_display, xButton, True, CurrentTime);
    XFlush(m_display);
}

void RemoteServer::simulateMouseRelease(int button, int x, int y)
{
    if (!m_display) return;

    XTestFakeMotionEvent(m_display, DefaultScreen(m_display), x, y, CurrentTime);

    unsigned int xButton = 1;
    switch (button) {
    case Qt::LeftButton:   xButton = 1; break;
    case Qt::MiddleButton: xButton = 2; break;
    case Qt::RightButton:  xButton = 3; break;
    default: return;
    }

    XTestFakeButtonEvent(m_display, xButton, False, CurrentTime);
    XFlush(m_display);
}

void RemoteServer::simulateMouseWheel(int delta)
{
    if (!m_display) return;

    // In X11: button 4 = scroll up, button 5 = scroll down
    unsigned int button = (delta > 0) ? 4 : 5;
    int clicks = qAbs(delta) / 120;
    if (clicks < 1) clicks = 1;

    for (int i = 0; i < clicks; ++i) {
        XTestFakeButtonEvent(m_display, button, True, CurrentTime);
        XTestFakeButtonEvent(m_display, button, False, CurrentTime);
    }

    XFlush(m_display);
}

void RemoteServer::simulateKeyPress(int qtKey)
{
    if (!m_display) return;

    KeySym ks = qtKeyToKeysym(qtKey);
    if (ks == NoSymbol) return;

    KeyCode kc = XKeysymToKeycode(m_display, ks);
    if (kc == 0) return;

    XTestFakeKeyEvent(m_display, kc, True, CurrentTime);
    XFlush(m_display);
}

void RemoteServer::simulateKeyRelease(int qtKey)
{
    if (!m_display) return;

    KeySym ks = qtKeyToKeysym(qtKey);
    if (ks == NoSymbol) return;

    KeyCode kc = XKeysymToKeycode(m_display, ks);
    if (kc == 0) return;

    XTestFakeKeyEvent(m_display, kc, False, CurrentTime);
    XFlush(m_display);
}

#endif // Q_OS_LINUX

void RemoteServer::onAudioFrameCaptured(const QByteArray &data, int sampleRate, int channels, int sampleSize, int sampleType)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << static_cast<quint32>(sampleRate)
        << static_cast<quint8>(channels)
        << static_cast<quint8>(sampleSize)
        << static_cast<quint8>(sampleType)
        << data;

    for (QTcpSocket *socket : m_clients)
    {
        if (m_authenticated.value(socket, false))
        {
            m_streams[socket]->send(Protocol::PacketType::AudioFrame, payload);
        }
    }
}

void RemoteServer::onClipboardChanged()
{
    if (m_clients.isEmpty())
        return;

    QString text = QGuiApplication::clipboard()->text();
    if (text.isEmpty())
        return;

    if (text == m_lastIncomingClipboard)
        return;

    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << text;

    for (QTcpSocket *socket : m_clients) {
        if (m_authenticated.value(socket, false)) {
            m_streams[socket]->send(Protocol::PacketType::Clipboard, payload);
        }
    }
}
