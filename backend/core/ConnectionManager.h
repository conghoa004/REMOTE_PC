#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QImage>
#include <qqmlintegration.h>

#include "RemoteServer.h"
#include "RemoteClient.h"

class ConnectionManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool hosting READ hosting NOTIFY hostingChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString password READ password NOTIFY passwordChanged)

    Q_PROPERTY(ConnectionState state READ state NOTIFY stateChanged)

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool losslessQuality READ losslessQuality WRITE setLosslessQuality NOTIFY losslessQualityChanged)

public:
    explicit ConnectionManager(QObject *parent = nullptr);
    ~ConnectionManager() override;

    enum class ConnectionState
    {
        Disconnected,
        Connecting,
        Authenticating,
        Connected,
        Failed
    };
    Q_ENUM(ConnectionState)

    QString lastError() const;

    bool hosting() const;
    bool connected() const;

    quint16 port() const;
    void setPort(quint16 port);

    QString password() const;

    ConnectionState state() const;

    bool losslessQuality() const;
    void setLosslessQuality(bool lossless);

    Q_INVOKABLE bool startHost();
    Q_INVOKABLE void stopHost();

    Q_INVOKABLE void connectToHost(const QString &ip, const QString &password);
    Q_INVOKABLE void disconnectFromHost();

    Q_INVOKABLE void generatePassword();

    Q_INVOKABLE void sendMouseMove(int x, int y);
    Q_INVOKABLE void sendMousePress(int button, int x, int y);
    Q_INVOKABLE void sendMouseRelease(int button, int x, int y);
    Q_INVOKABLE void sendMouseWheel(int delta);
    Q_INVOKABLE void sendKeyPress(int key);
    Q_INVOKABLE void sendKeyRelease(int key);

signals:
    void lastErrorChanged();

    void hostingChanged();
    void connectedChanged();

    void portChanged();
    void passwordChanged();

    void clientConnected(QTcpSocket *socket);
    void clientDisconnected(QTcpSocket *socket);

    void stateChanged();
    void losslessQualityChanged();

    void frameReceived(const QImage &image);

private:
    void setState(ConnectionState state);

    QString m_lastError;

    RemoteServer m_server;
    RemoteClient m_client;

    bool m_hosting = false;
    quint16 m_port = 5000;

    QString m_password;

    ConnectionState m_state = ConnectionState::Disconnected;

    bool m_losslessQuality = false;
};