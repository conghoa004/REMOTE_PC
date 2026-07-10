#include "NetworkManager.h"

#include <QHostAddress>
#include <QNetworkInterface>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    refresh();
}

QString NetworkManager::localIp() const
{
    return m_localIp;
}

void NetworkManager::refresh()
{
    QString ip = "No Internet connection";

    const auto interfaces = QNetworkInterface::allInterfaces();

    for (const auto &iface : interfaces)
    {
        if (!(iface.flags() & QNetworkInterface::IsUp))
            continue;

        if (!(iface.flags() & QNetworkInterface::IsRunning))
            continue;

        if (iface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        // Bỏ các interface ảo
        QString name = iface.humanReadableName().toLower();

        if (name.contains("virtual") ||
            name.contains("vmware") ||
            name.contains("docker") ||
            name.contains("vbox") ||
            name.contains("tailscale") ||
            name.contains("hamachi") ||
            name.contains("zerotier"))
        {
            continue;
        }

        for (const auto &entry : iface.addressEntries())
        {
            const QHostAddress &address = entry.ip();

            if (address.protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            if (address.isLoopback())
                continue;

            ip = address.toString();
            break;
        }

        if (ip != "No Internet connection")
            break;
    }

    if (ip != m_localIp)
    {
        m_localIp = ip;
        emit localIpChanged();
    }
}