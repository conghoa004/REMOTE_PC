#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include <QPainter>
#include <qqmlintegration.h>

class ScreenRenderer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenRenderer(QQuickItem *parent = nullptr);

    Q_INVOKABLE void setImage(const QImage &image);

    void paint(QPainter *painter) override;

private:
    QImage m_image;
};
