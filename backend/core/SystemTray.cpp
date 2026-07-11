#include "SystemTray.h"
#include <QIcon>
#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

SystemTray::SystemTray(QObject *parent)
    : QObject(parent)
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_menu = new QMenu();

    QAction *quitAction = m_menu->addAction("Exit");
    connect(quitAction, &QAction::triggered, this, &SystemTray::quitRequested);

    m_trayIcon->setContextMenu(m_menu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &SystemTray::onActivated);
}

SystemTray::~SystemTray()
{
    m_trayIcon->hide();
}

void SystemTray::setIconSource(const QString &source)
{
    if (m_iconSource != source) {
        m_iconSource = source;
        QString path = source;
        if (path.startsWith("qrc:")) {
            path = path.mid(3); // Convert "qrc:/..." to ":/..." which is the standard C++ resource path
        }
        QIcon icon(path);
        if (icon.isNull()) {
            qWarning() << "SystemTray: Failed to load icon from path:" << path;
        } else {
            m_trayIcon->setIcon(icon);
            m_trayIcon->show();
        }
        emit iconSourceChanged();
    }
}

void SystemTray::setTooltip(const QString &tooltip)
{
    if (m_tooltip != tooltip) {
        m_tooltip = tooltip;
        m_trayIcon->setToolTip(tooltip);
        emit tooltipChanged();
    }
}

void SystemTray::showMessage(const QString &title, const QString &message, int iconType, int msecs)
{
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
    if (iconType == 2) icon = QSystemTrayIcon::Warning;
    else if (iconType == 3) icon = QSystemTrayIcon::Critical;
    
    m_trayIcon->showMessage(title, message, icon, msecs);
}

void SystemTray::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    Q_UNUSED(reason);
}
