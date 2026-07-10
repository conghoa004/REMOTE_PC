#pragma once

#include <QObject>
#include <QTcpSocket>

#include "Packet.h"
#include "Protocol.h"

class PacketStream : public QObject
{
    Q_OBJECT

public:
    explicit PacketStream(QTcpSocket *socket,
                          QObject *parent = nullptr);

    void send(Protocol::PacketType type,
              const QByteArray &payload);

signals:
    void packetReceived(Packet packet);

private slots:
    void onReadyRead();

private:
    QTcpSocket *m_socket;

    QByteArray m_buffer;
};