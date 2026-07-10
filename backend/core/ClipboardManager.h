#pragma once

#include <QObject>
#include <qqmlintegration.h>

class ClipboardManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit ClipboardManager(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    Q_INVOKABLE void copy(const QString &text);
    Q_INVOKABLE QString paste() const;
    Q_INVOKABLE void clear();

signals:
    void textChanged();

private:
    QString m_text;
};