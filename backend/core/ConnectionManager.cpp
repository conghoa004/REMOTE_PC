#include "ConnectionManager.h"

#include <QRandomGenerator>

ConnectionManager::ConnectionManager(QObject *parent)
    : QObject(parent)
{

    connect(&m_server,
            &RemoteServer::clientConnected,
            this,
            &ConnectionManager::clientConnected);

    connect(&m_client,
            &RemoteClient::frameReceived,
            this,
            &ConnectionManager::frameReceived);

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