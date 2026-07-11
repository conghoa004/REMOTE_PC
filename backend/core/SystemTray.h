#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QtQml>

class SystemTray : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged)

public:
    explicit SystemTray(QObject *parent = nullptr);
    ~SystemTray();

    QString iconSource() const { return m_iconSource; }
    void setIconSource(const QString &source);

    QString tooltip() const { return m_tooltip; }
    void setTooltip(const QString &tooltip);

    Q_INVOKABLE void showMessage(const QString &title, const QString &message, int iconType = 1, int msecs = 5000);

signals:
    void showRequested();
    void quitRequested();
    void iconSourceChanged();
    void tooltipChanged();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_menu;
    QString m_iconSource;
    QString m_tooltip;
};
