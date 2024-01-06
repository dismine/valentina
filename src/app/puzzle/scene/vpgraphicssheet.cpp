/*******************************************************************
 **
 **  @file   vpgraphicssheet.cpp
 **  @author Ronan Le Tiec
 **  @date   3 5, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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

#include "vpgraphicssheet.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vptilefactory.h"
#include "../vwidgets/vpiecegrainline.h"
#include "scenedef.h"

#include <QApplication>
#include <QFontMetrics>
#include <QtMath>

namespace
{
constexpr qreal foldArrowMargin = 20;

//---------------------------------------------------------------------------------------------------------------------
auto SwapRect(const QRectF &rect) -> QRectF
{
    return {rect.center().x() - rect.height() / 2.0, rect.center().y() - rect.width() / 2.0, rect.height(),
            rect.width()};
}

//---------------------------------------------------------------------------------------------------------------------
void PaintVerticalFoldShadow(QPainter *painter, const QRectF &sheetRect)
{
    QLineF shadowLine(sheetRect.topLeft(), sheetRect.bottomLeft());
    shadowLine.setLength(shadowLine.length() * 0.97);
    shadowLine.setAngle(shadowLine.angle() - 1.5);

    QPointF const shadowP =
        VGObject::ClosestPoint(QLineF(sheetRect.topLeft(), sheetRect.bottomLeft()), shadowLine.p2());

    painter->drawLine(shadowLine);
    painter->drawLine(QLineF(shadowLine.p2(), shadowP));

    QPolygonF const shadow{sheetRect.topLeft(), shadowLine.p2(), shadowP, sheetRect.topLeft()};

    painter->setBrush(QBrush(VSceneStylesheet::ManualLayoutStyle().SheetFoldShadowColor()));
    painter->drawPolygon(shadow);
}

//---------------------------------------------------------------------------------------------------------------------
void PaintHorizontalFoldShadow(QPainter *painter, const QRectF &sheetRect)
{
    QLineF shadowLine(sheetRect.topRight(), sheetRect.topLeft());
    shadowLine.setLength(shadowLine.length() * 0.97);
    shadowLine.setAngle(shadowLine.angle() - 1.5);

    QPointF const shadowP = VGObject::ClosestPoint(QLineF(sheetRect.topRight(), sheetRect.topLeft()), shadowLine.p2());

    painter->drawLine(shadowLine);
    painter->drawLine(QLineF(shadowLine.p2(), shadowP));

    QPolygonF const shadow{sheetRect.topRight(), shadowLine.p2(), shadowP, sheetRect.topRight()};

    painter->setBrush(QBrush(VSceneStylesheet::ManualLayoutStyle().SheetFoldShadowColor()));
    painter->drawPolygon(shadow);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsSheet::VPGraphicsSheet(const VPLayoutPtr &layout, QGraphicsItem *parent)
  : QGraphicsItem(parent),
    m_layout(layout)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    PaintMargins(painter);
    PaintBorder(painter);
    PaintFold(painter);
    PaintGrid(painter);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::GetSheetRect() const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return {};
    }

    VPSheetPtr const sheet = layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return {};
    }
    return sheet->GetSheetRect();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::GetMarginsRect() const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return {};
    }

    VPSheetPtr const sheet = layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return {};
    }
    return sheet->GetMarginsRect();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::SetShowMargin(bool value)
{
    m_showMargin = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::SetShowBorder(bool value)
{
    m_showBorder = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::RefreshBoundingRect()
{
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintVerticalFold(QPainter *painter, const QRectF &sheetRect) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (not layout.isNull() && layout->LayoutSettings().IsCutOnFold())
    {
        QString const foldText = FoldText();
        painter->save();
        QFont font = QApplication::font();
        font.setPointSize(foldFontSize);
        painter->setFont(font);
        QRectF textRect = painter->fontMetrics().boundingRect(foldText);
        int const textDescent = painter->fontMetrics().descent();
        QPointF const textPosition(sheetRect.center().x() - textRect.width() / 2.,
                                   sheetRect.topLeft().y() - foldTextMargin - textDescent);
        painter->drawText(textPosition, foldText);
        textRect.translate(sheetRect.center() - textRect.center());
        textRect.translate(0, -(sheetRect.center().y() - sheetRect.topLeft().y()) - foldTextMargin -
                                  textRect.height() / 2.);
        // painter->drawRect(textRect); // uncomment for debug
        painter->restore();

        if (sheetRect.width() >= textRect.width() * 2)
        {
            qreal const baseY = textRect.center().y();
            qreal const arrowMargin = foldArrowMargin + textRect.width() / 2.;

            QLineF const leftLine(QPointF(sheetRect.topLeft().x(), baseY),
                                  QPointF(sheetRect.center().x() - arrowMargin, baseY));
            VPieceGrainline const leftArrow(leftLine, GrainlineArrowDirection::oneWayDown);
            QPainterPath leftArrowPath = VLayoutPiece::GrainlinePath(leftArrow.Shape());
            leftArrowPath.setFillRule(Qt::WindingFill);

            painter->save();
            QPen pen = painter->pen();
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(QBrush(pen.color(), Qt::SolidPattern));
            painter->drawPath(leftArrowPath);
            painter->restore();

            QLineF const rightLine(QPointF(sheetRect.center().x() + arrowMargin, baseY),
                                   QPointF(sheetRect.topRight().x(), baseY));
            VPieceGrainline const rightArrow(rightLine, GrainlineArrowDirection::oneWayUp);
            QPainterPath rightArrowPath = VLayoutPiece::GrainlinePath(rightArrow.Shape());
            rightArrowPath.setFillRule(Qt::WindingFill);

            painter->save();
            pen = painter->pen();
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(QBrush(pen.color(), Qt::SolidPattern));
            painter->drawPath(rightArrowPath);
            painter->restore();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintHorizontalFold(QPainter *painter, const QRectF &sheetRect) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (not layout.isNull() && layout->LayoutSettings().IsCutOnFold())
    {
        QString const foldText = FoldText();
        painter->save();
        QFont font = QApplication::font();
        font.setPointSize(foldFontSize);
        painter->setFont(font);
        QRectF const textRect = painter->fontMetrics().boundingRect(foldText);
        // int const textAscent = painter->fontMetrics().ascent();
        int const textDescent = painter->fontMetrics().descent();
        QPointF const textPosition(sheetRect.center().x() - textRect.width() / 2.,
                                   sheetRect.center().y() - sheetRect.width() / 2. - foldTextMargin - textDescent);
        painter->translate(sheetRect.center());
        painter->rotate(90);
        painter->translate(-sheetRect.center());
        painter->drawText(textPosition, foldText);
        painter->restore();

        QRectF swappedRect = SwapRect(textRect);
        swappedRect.translate(sheetRect.center() - swappedRect.center());
        swappedRect.translate(
            (sheetRect.topRight().x() - sheetRect.center().x()) + foldTextMargin + textRect.height() / 2., 0);
        // painter->drawRect(swappedRect); // uncomment for debug

        if (sheetRect.height() >= textRect.width() * 2)
        {
            // qreal const baseX = sheetRect.topRight().x() + foldTextMargin + textDescent + textAscent / 2.;
            qreal const baseX = swappedRect.center().x();
            qreal const arrowMargin = foldArrowMargin + textRect.width() / 2.;
            QLineF const leftLine(QPointF(baseX, sheetRect.topRight().y()),
                                  QPointF(baseX, sheetRect.center().y() - arrowMargin));

            VPieceGrainline const leftArrow(leftLine, GrainlineArrowDirection::oneWayDown);
            QPainterPath leftArrowPath = VLayoutPiece::GrainlinePath(leftArrow.Shape());
            leftArrowPath.setFillRule(Qt::WindingFill);

            painter->save();
            QPen pen = painter->pen();
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(QBrush(pen.color(), Qt::SolidPattern));
            painter->drawPath(leftArrowPath);
            painter->restore();

            QLineF const rightLine(QPointF(baseX, sheetRect.center().y() + arrowMargin),
                                   QPointF(baseX, sheetRect.bottomRight().y()));
            VPieceGrainline const rightArrow(rightLine, GrainlineArrowDirection::oneWayUp);
            QPainterPath rightArrowPath = VLayoutPiece::GrainlinePath(rightArrow.Shape());
            rightArrowPath.setFillRule(Qt::WindingFill);

            painter->save();
            pen = painter->pen();
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(QBrush(pen.color(), Qt::SolidPattern));
            painter->drawPath(rightArrowPath);
            painter->restore();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintMargins(QPainter *painter) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (layout.isNull())
    {
        return;
    }

    bool ignoreMargins = true;
    VPSheetPtr const sheet = layout->GetFocusedSheet();
    if (!sheet.isNull())
    {
        ignoreMargins = sheet->IgnoreMargins();
    }

    if (m_showMargin && !ignoreMargins)
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetMarginColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        painter->save();
        painter->setPen(pen);
        painter->drawRect(GetMarginsRect());
        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintBorder(QPainter *painter) const
{
    QRectF const sheetRect = GetSheetRect();

    if (m_showBorder)
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetBorderColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        painter->save();
        painter->setPen(pen);
        painter->drawRect(sheetRect);

        VPLayoutPtr const layout = m_layout.toStrongRef();

        if (!layout.isNull() && layout->LayoutSettings().IsCutOnFold())
        {
            if (sheetRect.width() >= sheetRect.height())
            {
                PaintVerticalFoldShadow(painter, sheetRect);
            }
            else
            {
                PaintHorizontalFoldShadow(painter, sheetRect);
            }
        }

        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintFold(QPainter *painter) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (!layout.isNull() && layout->LayoutSettings().IsCutOnFold())
    {
        QRectF const sheetRect = GetSheetRect();
        QRectF const foldField = m_showBorder ? sheetRect : FoldField(GetMarginsRect());

        if (sheetRect.width() >= sheetRect.height())
        {
            PaintVerticalFold(painter, foldField);
        }
        else
        {
            PaintHorizontalFold(painter, foldField);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintGrid(QPainter *painter) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (not layout.isNull() && layout->LayoutSettings().GetShowGrid())
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetGridColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        painter->save();
        painter->setPen(pen);

        QRectF const sheetRect = GetSheetRect();

        qreal const colWidth = layout->LayoutSettings().GetGridColWidth();
        if (colWidth > 0)
        {
            qreal colX = colWidth;
            while (colX < sheetRect.right())
            {
                QLineF const line = QLineF(colX, 0, colX, sheetRect.bottom());
                painter->drawLine(line);
                colX += colWidth;
            }
        }

        qreal const rowHeight = layout->LayoutSettings().GetGridRowHeight();
        if (rowHeight > 0)
        {
            qreal rowY = rowHeight;

            while (rowY < sheetRect.bottom())
            {
                QLineF const line = QLineF(0, rowY, sheetRect.right(), rowY);
                painter->drawLine(line);
                rowY += rowHeight;
            }
        }
        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::FoldField(const QRectF &sheetRect) const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (layout.isNull())
    {
        return sheetRect;
    }

    VPSheetPtr const sheet = layout->GetFocusedSheet();

    qreal const xScale = layout->LayoutSettings().HorizontalScale();
    qreal const yScale = layout->LayoutSettings().VerticalScale();
    const int nbCol = layout->TileFactory()->ColNb(sheet);
    const int nbRow = layout->TileFactory()->RowNb(sheet);
    const qreal tilesWidth = (layout->TileFactory()->DrawingAreaWidth() - VPTileFactory::tileStripeWidth) / xScale;
    const qreal tilesHeight = (layout->TileFactory()->DrawingAreaHeight() - VPTileFactory::tileStripeWidth) / yScale;

    return {sheetRect.topLeft(), QSizeF(nbCol * tilesWidth, nbRow * tilesHeight)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::FoldText() -> QString
{
    return tr("FOLD");
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::boundingRect() const -> QRectF
{
    QRectF boundingRect = GetSheetRect();
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (not layout.isNull() && layout->LayoutSettings().IsCutOnFold())
    {
        QString const foldText = FoldText();
        QFont font = QApplication::font();
        font.setPointSize(foldFontSize);
        QFontMetrics const metric(font);
        QRectF textRect = metric.boundingRect(foldText);
        QRectF const foldField = m_showBorder ? boundingRect : FoldField(GetMarginsRect());

        if (boundingRect.width() >= boundingRect.height())
        {
            textRect.translate(foldField.center() - textRect.center());
            textRect.translate(0, -(foldField.center().y() - foldField.topLeft().y()) - foldTextMargin -
                                      textRect.height() / 2.);
            boundingRect = foldField.united(textRect);
        }
        else
        {
            QRectF swappedRect = SwapRect(textRect);
            swappedRect.translate(foldField.center() - swappedRect.center());
            swappedRect.translate(
                (foldField.topRight().x() - foldField.center().x()) + foldTextMargin + textRect.height() / 2., 0);
            boundingRect = foldField.united(swappedRect);
        }
    }

    return boundingRect;
}
