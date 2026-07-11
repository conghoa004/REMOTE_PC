#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QImage>
#include <QTimer>
#include <QPoint>

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

    void sendMouseMove(int x, int y);
    void sendMousePress(int button, int x, int y);
    void sendMouseRelease(int button, int x, int y);
    void sendMouseWheel(int delta);
    void sendKeyPress(int key);
    void sendKeyRelease(int key);

signals:
    void connected();
    void disconnected();

    void authenticated();
    void authenticationFailed();

    void errorOccurred(QString error);

    void frameReceived(const QImage &image);
    void audioFrameReceived(const QByteArray &data, int sampleRate, int channels, int sampleSize, int sampleType);

private slots:
    void onConnected();
    void onDisconnected();

private:
    void sendAuthentication();
    void sendSettingsUpdate();
    void onPacketReceived(Packet packet);
    void sendPendingMouseMove();

    QTcpSocket m_socket;
    PacketStream *m_stream = nullptr;

    QString m_password;
    bool m_losslessQuality = false;

    // Mouse move throttling
    QTimer m_mouseThrottleTimer;
    QPoint m_pendingMouseMove;
    bool m_hasPendingMouseMove = false;
};