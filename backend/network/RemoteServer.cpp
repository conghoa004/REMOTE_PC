#include "RemoteServer.h"

#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QBuffer>
#include <QDataStream>

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

    default:
        break;
    }
}

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
