#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QTimer>
#include <QImage>

#include "PacketStream.h"
#include "Protocol.h"
#include "AudioLoopbackCapture.h"

#ifdef Q_OS_LINUX
struct _XDisplay; // Forward declaration to avoid including X11 headers in the header
#endif

class RemoteServer : public QObject
{
    Q_OBJECT

public:
    explicit RemoteServer(QObject *parent = nullptr);
    ~RemoteServer();

    bool start(quint16 port);
    void stop();

    bool isRunning() const;

    void setPassword(const QString &password);

signals:
    void clientConnected(QTcpSocket *socket);
    void clientDisconnected(QTcpSocket *socket);

private slots:
    void onNewConnection();
    void onDisconnected();
    void captureAndBroadcast();
    void onAudioFrameCaptured(const QByteArray &data, int sampleRate, int channels, int sampleSize, int sampleType);
    void onClipboardChanged();

private:
    void onPacketReceived(QTcpSocket *socket, Packet packet);

    void sendAuthSuccess(QTcpSocket *socket);
    void sendAuthFailed(QTcpSocket *socket);
    void sendScreenFrame(QTcpSocket *socket, const QByteArray &framePayload);

    QByteArray compressFrame(const QImage &image, Protocol::CompressionType type);

    // Input simulation helpers
    void simulateMouseMove(int x, int y);
    void simulateMousePress(int button, int x, int y);
    void simulateMouseRelease(int button, int x, int y);
    void simulateMouseWheel(int delta);
    void simulateKeyPress(int qtKey);
    void simulateKeyRelease(int qtKey);

private:
    QTcpServer m_server;

    QList<QTcpSocket*> m_clients;

    // socket đã xác thực hay chưa
    QHash<QTcpSocket*, bool> m_authenticated;

    // PacketStream for each socket
    QHash<QTcpSocket*, PacketStream*> m_streams;

    // Preferred compression per client
    QHash<QTcpSocket*, Protocol::CompressionType> m_clientCompression;

    QString m_password;

    // Screen capture timer
    QTimer m_captureTimer;

    // Previous frame for diff detection
    QImage m_previousFrame;

    AudioLoopbackCapture m_audioCapture;
    QString m_lastIncomingClipboard;

#ifdef Q_OS_LINUX
    _XDisplay *m_display = nullptr;
#endif
};