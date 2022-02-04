/************************************************************************
 **
 **  @file   vbackgroundsvgitem.cpp
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
#include "vbackgroundsvgitem.h"
#include <QSize>

#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>
#include <QPen>
#include <QPainter>

//---------------------------------------------------------------------------------------------------------------------
VBackgroundSVGItem::VBackgroundSVGItem(const VBackgroundPatternImage &image, VAbstractPattern *doc,
                                       QGraphicsItem *parent)
    : VBackgroundImageItem(image, doc, parent),
      m_renderer(new QSvgRenderer())
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    QObject::connect(m_renderer, &QSvgRenderer::repaintNeeded, this, &VBackgroundSVGItem::RepaintItem);
}

//---------------------------------------------------------------------------------------------------------------------
VBackgroundSVGItem::~VBackgroundSVGItem()
{
    delete m_renderer;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundSVGItem::boundingRect() const -> QRectF
{
    QSize size = Renderer()->defaultSize();
    constexpr double ratio = PrintDPI / 90.;
    size = QSize(qRound(size.width()*ratio), qRound(size.height()*ratio));
    return Image().Matrix().mapRect(QRectF(QPointF(0, 0), size));
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundSVGItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QSvgRenderer *renderer = Renderer();

    if (not renderer->isValid())
    {
        return;
    }

    painter->save();
    painter->setTransform(Image().Matrix(), true);
    painter->setOpacity(Image().Opacity());
    painter->scale(PrintDPI / 90., PrintDPI / 90.);

    renderer->render(painter, QRectF(QPointF(0, 0), renderer->defaultSize()));

    painter->restore();

    VBackgroundImageItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundSVGItem::RepaintItem()
{
    update();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundSVGItem::Renderer() const -> QSvgRenderer *
{
    if (Stale())
    {
        const QString brokenImage = QStringLiteral("://icon/svg/broken_path.svg");
        m_renderer->load(brokenImage);

        VBackgroundPatternImage image = Image();
        if (not image.IsValid())
        {
            MakeFresh();
            return m_renderer;
        }

        if (not image.FilePath().isEmpty())
        {
            m_renderer->load(image.FilePath());
            if (not m_renderer->isValid())
            {
                m_renderer->load(brokenImage);
            }
            MakeFresh();
            return m_renderer;
        }

        if (not image.ContentData().isEmpty())
        {
            m_renderer->load(QByteArray::fromBase64(image.ContentData()));
            if (not m_renderer->isValid())
            {
                m_renderer->load(brokenImage);
            }
            MakeFresh();
            return m_renderer;
        }
    }

    return m_renderer;
}
