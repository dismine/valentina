/************************************************************************
 **
 **  @file   vbackgroundpixmapitem.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
#include "vbackgroundpixmapitem.h"

#include <QBitmap>
#include <QBuffer>
#include <QImageReader>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

extern auto qt_regionToPath(const QRegion &region) -> QPainterPath;

namespace
{
auto InvalidImage() -> QPixmap
{
    QImageReader imageReader(VBackgroundPatternImage::brokenImage);
    return QPixmap::fromImageReader(&imageReader);
}
}

//---------------------------------------------------------------------------------------------------------------------
VBackgroundPixmapItem::VBackgroundPixmapItem(const VBackgroundPatternImage &image, VAbstractPattern *doc,
                                             QGraphicsItem *parent)
    : VBackgroundImageItem(image, doc, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::SetTransformationMode(Qt::TransformationMode mode)
{
    m_transformationMode = mode;
    update();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::boundingRect() const -> QRectF
{
    QPixmap const pixmap = Pixmap();
    if (pixmap.isNull())
    {
        return {};
    }

    return Image().Matrix().mapRect(QRectF(QPointF(0, 0), pixmap.size() / pixmap.devicePixelRatio()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::shape() const -> QPainterPath
{
    if (!m_hasShape)
    {
        UpdateShape();
        m_hasShape = true;
    }
    return Image().Matrix().map(m_shape);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::contains(const QPointF &point) const -> bool
{
    return QGraphicsItem::contains(point);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform, (m_transformationMode == Qt::SmoothTransformation));

    painter->save();
    painter->setTransform(Image().Matrix(), true);
    painter->setOpacity(Image().Opacity());

    painter->drawPixmap(QPointF(), Pixmap());

    painter->restore();

    VBackgroundImageItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::isObscuredBy(const QGraphicsItem *item) const -> bool
{
    return QGraphicsItem::isObscuredBy(item);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::opaqueArea() const -> QPainterPath
{
    return shape();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::GetShapeMode() const -> ShapeMode
{
    return m_shapeMode;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::SetShapeMode(ShapeMode mode)
{
    if (m_shapeMode == mode)
    {
        return;
    }
    m_shapeMode = mode;
    m_hasShape = false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::supportsExtension(Extension extension) const -> bool
{
    Q_UNUSED(extension);
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::extension(const QVariant &variant) const -> QVariant
{
    Q_UNUSED(variant);
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    SetTransformationMode(Qt::FastTransformation);
    VBackgroundImageItem::mouseMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    SetTransformationMode(Qt::SmoothTransformation);
    VBackgroundImageItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPixmapItem::UpdateShape() const
{
    QPixmap const pixmap = Pixmap();
    m_shape = QPainterPath();
    switch (m_shapeMode)
    {
        case ShapeMode::MaskShape:
        {
            QBitmap const mask = pixmap.mask();
            if (!mask.isNull())
            {
                m_shape = qt_regionToPath(QRegion(mask));
                break;
            }
            Q_FALLTHROUGH();
        }
        case ShapeMode::BoundingRectShape:
            m_shape.addRect(QRectF(0, 0, pixmap.width(), pixmap.height()));
            break;
        case ShapeMode::HeuristicMaskShape:
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
            m_shape = qt_regionToPath(QRegion(pixmap.createHeuristicMask()));
#else
            m_shape.addRect(QRectF(0, 0, m_shape.width(), m_shape.height()));
#endif
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPixmapItem::Pixmap() const -> QPixmap
{
    if (Stale())
    {
        m_pixmap = QPixmap();

        VBackgroundPatternImage const image = Image();
        if (not image.IsValid())
        {
            m_pixmap = InvalidImage();
            MakeFresh();
            return m_pixmap;
        }

        // Scale to Valentina resolution
        auto ScaleImage = [](const QImage &image)
        {
            const double ratioX = PrintDPI / (image.dotsPerMeterX() / 100. * 2.54);
            const double ratioY = PrintDPI / (image.dotsPerMeterY() / 100. * 2.54);
            const QSize imageSize = image.size();
            return image.scaled(qRound(imageSize.width()*ratioX),
                                qRound(imageSize.height()*ratioY),
                                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        };

        if (not image.FilePath().isEmpty())
        {
            QImageReader imageReader(image.FilePath());
            QImage const image = imageReader.read();
            if (not image.isNull())
            {
                m_pixmap = QPixmap::fromImage(ScaleImage(image));
                if (m_pixmap.isNull())
                {
                    m_pixmap = InvalidImage();
                }
            }
            else
            {
                m_pixmap = InvalidImage();
            }
            MakeFresh();
            return m_pixmap;
        }

        if (not image.ContentData().isEmpty())
        {
            QByteArray array = QByteArray::fromBase64(image.ContentData());
            QBuffer buffer(&array);
            buffer.open(QIODevice::ReadOnly);

            QImageReader imageReader(&buffer);
            QImage const image = imageReader.read();
            if (not image.isNull())
            {
                m_pixmap = QPixmap::fromImage(ScaleImage(image));
                if (m_pixmap.isNull())
                {
                    m_pixmap = InvalidImage();
                }
            }
            else
            {
                m_pixmap = InvalidImage();
            }
            MakeFresh();
            return m_pixmap;
        }
    }

    return m_pixmap;
}
