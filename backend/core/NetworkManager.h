#pragma once

#include <QObject>
#include <qqmlintegration.h>

class NetworkManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString localIp READ localIp NOTIFY localIpChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr);

    QString localIp() const;

    Q_INVOKABLE void refresh();

signals:
    void localIpChanged();

private:
    QString m_localIp;
};