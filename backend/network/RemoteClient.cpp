#include "RemoteClient.h"

#include <QBuffer>
#include <QDataStream>

RemoteClient::RemoteClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket,
            &QTcpSocket::connected,
            this,
            &RemoteClient::onConnected);

    connect(&m_socket,
            &QTcpSocket::disconnected,
            this,
            &RemoteClient::onDisconnected);

    connect(&m_socket,
            &QTcpSocket::errorOccurred,
            this,
            [this](QAbstractSocket::SocketError)
            {
                emit errorOccurred(m_socket.errorString());
            });
}

void RemoteClient::connectToHost(const QString &ip,
                                 quint16 port,
                                 const QString &password)
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        m_socket.abort();

    m_password = password;

    m_socket.connectToHost(ip, port);
}

void RemoteClient::disconnectFromHost()
{
    m_socket.disconnectFromHost();
}

bool RemoteClient::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void RemoteClient::setLosslessQuality(bool lossless)
{
    m_losslessQuality = lossless;

    if (isConnected() && m_stream)
        sendSettingsUpdate();
}

void RemoteClient::onConnected()
{
    // Create PacketStream for this connection
    if (m_stream) {
        m_stream->deleteLater();
    }

    m_stream = new PacketStream(&m_socket, this);

    connect(m_stream,
            &PacketStream::packetReceived,
            this,
            &RemoteClient::onPacketReceived);

    emit connected();

    sendAuthentication();
}

void RemoteClient::sendAuthentication()
{
    if (!m_stream)
        return;

    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << m_password;

    m_stream->send(Protocol::PacketType::AuthRequest, payload);
}

void RemoteClient::sendSettingsUpdate()
{
    if (!m_stream)
        return;

    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << static_cast<quint8>(m_losslessQuality
            ? Protocol::CompressionType::Lossless
            : Protocol::CompressionType::HighQuality);

    m_stream->send(Protocol::PacketType::SettingsUpdate, payload);
}

void RemoteClient::onDisconnected()
{
    if (m_stream) {
        m_stream->deleteLater();
        m_stream = nullptr;
    }

    emit disconnected();
}

void RemoteClient::onPacketReceived(Packet packet)
{
    switch (packet.type)
    {
    case Protocol::PacketType::AuthSuccess:
        emit authenticated();
        // Send initial quality settings after authentication
        sendSettingsUpdate();
        break;

    case Protocol::PacketType::AuthFailed:
        emit authenticationFailed();
        break;

    case Protocol::PacketType::ScreenFrame:
    {
        // Payload format: [compressionType:quint8][width:quint32][height:quint32][data]
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);

        quint8 compressionType;
        quint32 width, height;

        in >> compressionType >> width >> height;

        QByteArray imageData;
        in >> imageData;

        QImage image;

        if (static_cast<Protocol::CompressionType>(compressionType) == Protocol::CompressionType::Lossless)
        {
            // Decompress zlib data and load as raw RGBA
            QByteArray raw = qUncompress(imageData);
            if (!raw.isEmpty()) {
                image = QImage(
                    reinterpret_cast<const uchar*>(raw.constData()),
                    width, height,
                    QImage::Format_RGB32
                ).copy(); // Deep copy since raw will go out of scope
            }
        }
        else
        {
            // Load JPEG from buffer
            image.loadFromData(imageData, "JPEG");
        }

        if (!image.isNull()) {
            emit frameReceived(image);
        }

        break;
    }

    default:
        break;
    }
}
