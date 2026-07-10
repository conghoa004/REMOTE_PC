#include "PacketStream.h"

#include <QDataStream>
#include <QIODevice>

PacketStream::PacketStream(QTcpSocket *socket, QObject *parent)
    : QObject(parent),
    m_socket(socket)
{
    connect(m_socket,
            &QTcpSocket::readyRead,
            this,
            &PacketStream::onReadyRead);
}

void PacketStream::send(Protocol::PacketType type,
                        const QByteArray &payload)
{
    QByteArray block;

    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_6_0);

    quint32 length =
        sizeof(quint8) + payload.size();

    out << length;
    out << static_cast<quint8>(type);
    out.writeRawData(payload.constData(),
                     payload.size());

    m_socket->write(block);
}

void PacketStream::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (true)
    {
        if (m_buffer.size() < 4)
            return;

        QDataStream in(m_buffer);

        in.setVersion(QDataStream::Qt_6_0);

        quint32 length;

        in >> length;

        if (m_buffer.size() < length + 4)
            return;

        quint8 type;

        in >> type;

        QByteArray payload(length - sizeof(quint8), Qt::Uninitialized);

        in.readRawData(payload.data(),
                       payload.size());

        Packet packet;

        packet.type = static_cast<Protocol::PacketType>(type);
        packet.payload = payload;

        emit packetReceived(packet);

        m_buffer.remove(0, length + 4);
    }
}
