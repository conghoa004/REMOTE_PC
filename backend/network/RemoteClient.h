#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QImage>

#include "Protocol.h"
#include "PacketStream.h"

class RemoteClient : public QObject
{
    Q_OBJECT

public:
    explicit RemoteClient(QObject *parent = nullptr);

    void connectToHost(const QString &ip,
                       quint16 port,
                       const QString &password);

    void disconnectFromHost();

    bool isConnected() const;

    void setLosslessQuality(bool lossless);

signals:
    void connected();
    void disconnected();

    void authenticated();
    void authenticationFailed();

    void errorOccurred(QString error);

    void frameReceived(const QImage &image);

private slots:
    void onConnected();
    void onDisconnected();

private:
    void sendAuthentication();
    void sendSettingsUpdate();
    void onPacketReceived(Packet packet);

    QTcpSocket m_socket;
    PacketStream *m_stream = nullptr;

    QString m_password;
    bool m_losslessQuality = false;
};