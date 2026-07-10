#include "ScreenRenderer.h"

ScreenRenderer::ScreenRenderer(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    // Set flag to optimize rendering if needed, though default QQuickPaintedItem configuration is fine.
}

void ScreenRenderer::setImage(const QImage &image)
{
    m_image = image;
    update(); // Redraw item
}

void ScreenRenderer::paint(QPainter *painter)
{
    if (m_image.isNull()) {
        return;
    }

    QRectF targetRect = boundingRect();
    QSizeF imageSize = m_image.size();

    // Scale keeping aspect ratio
    qreal widthRatio = targetRect.width() / imageSize.width();
    qreal heightRatio = targetRect.height() / imageSize.height();
    qreal scaleRatio = qMin(widthRatio, heightRatio);

    QSizeF scaledSize = imageSize * scaleRatio;
    QRectF drawRect(
        targetRect.x() + (targetRect.width() - scaledSize.width()) / 2.0,
        targetRect.y() + (targetRect.height() - scaledSize.height()) / 2.0,
        scaledSize.width(),
        scaledSize.height()
    );

    // High quality scaling
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawImage(drawRect, m_image, m_image.rect());
}
