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

QPointF ScreenRenderer::mapToSource(const QPointF &localPoint) const
{
    if (m_image.isNull())
        return localPoint;

    QRectF targetRect = boundingRect();
    QSizeF imageSize = m_image.size();

    qreal widthRatio = targetRect.width() / imageSize.width();
    qreal heightRatio = targetRect.height() / imageSize.height();
    qreal scaleRatio = qMin(widthRatio, heightRatio);

    if (scaleRatio <= 0.0)
        return localPoint;

    QSizeF scaledSize = imageSize * scaleRatio;
    QRectF drawRect(
        targetRect.x() + (targetRect.width() - scaledSize.width()) / 2.0,
        targetRect.y() + (targetRect.height() - scaledSize.height()) / 2.0,
        scaledSize.width(),
        scaledSize.height()
    );

    // Compute coordinate relative to drawRect
    qreal localX = localPoint.x() - drawRect.x();
    qreal localY = localPoint.y() - drawRect.y();

    // Clamp inside drawRect to prevent clicking outside the remote desktop bounds
    localX = qBound(0.0, localX, drawRect.width());
    localY = qBound(0.0, localY, drawRect.height());

    // Scale back to host's resolution coordinate
    qreal sourceX = localX / scaleRatio;
    qreal sourceY = localY / scaleRatio;

    return QPointF(sourceX, sourceY);
}
