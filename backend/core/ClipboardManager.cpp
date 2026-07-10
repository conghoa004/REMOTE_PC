#include "ClipboardManager.h"

#include <QGuiApplication>
#include <QClipboard>

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject(parent)
{
}

QString ClipboardManager::text() const
{
    return QGuiApplication::clipboard()->text();
}

void ClipboardManager::setText(const QString &text)
{
    if (text == this->text())
        return;

    QGuiApplication::clipboard()->setText(text);

    emit textChanged();
}

void ClipboardManager::copy(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);

    emit textChanged();
}

QString ClipboardManager::paste() const
{
    return QGuiApplication::clipboard()->text();
}

void ClipboardManager::clear()
{
    QGuiApplication::clipboard()->clear();

    emit textChanged();
}