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
#include "../vformat/vsinglelineoutlinechar.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vptilefactory.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vpiecegrainline.h"
#include "scenedef.h"

#include <QApplication>
#include <QFontMetrics>
#include <QtMath>

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

namespace
{
constexpr qreal foldArrowMargin = 20;
constexpr qreal minTextFontSize = 5.0;

//---------------------------------------------------------------------------------------------------------------------
inline auto SwapRect(const QRectF &rect) -> QRectF
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

    QPolygonF const shadow({sheetRect.topLeft(), shadowLine.p2(), shadowP, sheetRect.topLeft()});

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

    QPolygonF const shadow({sheetRect.topRight(), shadowLine.p2(), shadowP, sheetRect.topRight()});

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
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); layout.isNull() || !layout->LayoutSettings().IsCutOnFold())
    {
        return;
    }

    QPainterStateGuard const guard(painter);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    const bool penPrinting = m_printMode && (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont());

    if (m_printMode)
    {
        QPen pen = painter->pen();
        pen.setWidthF(settings->WidthHairLine());
        pen.setColor(tileColor);
        painter->setPen(pen);
    }

    const QRectF textRect = !m_printMode || settings->GetSingleStrokeOutlineFont() || !settings->GetSingleLineFonts()
                                ? PaintVerticalFoldTextOutlineFont(painter, sheetRect)
                                : PaintVerticalFoldTextSVGFont(painter, sheetRect);

    if (sheetRect.height() < textRect.height() * 2)
    {
        return;
    }

    qreal const baseX = textRect.center().x();
    qreal const arrowMargin = foldArrowMargin + textRect.height() / 2.;
    QLineF const leftLine(QPointF(baseX, sheetRect.topRight().y()),
                          QPointF(baseX, sheetRect.center().y() - arrowMargin));

    VPieceGrainline const leftArrow(leftLine, GrainlineArrowDirection::oneWayDown);
    QPainterPath leftArrowPath = VLayoutPiece::GrainlinePath(leftArrow.Shape());
    leftArrowPath.setFillRule(Qt::WindingFill);

    QPen pen = painter->pen();
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (m_printMode)
    {
        pen.setBrush(Qt::NoBrush);
        pen.setCosmetic(true);
        pen.setColor(tileColor);

        if (!penPrinting)
        {
            painter->setBrush(QBrush(tileColor, Qt::SolidPattern));
        }
    }
    else
    {
        painter->setBrush(QBrush(pen.color(), Qt::SolidPattern));
    }
    painter->setPen(pen);
    painter->drawPath(leftArrowPath);

    QLineF const rightLine(QPointF(baseX, sheetRect.center().y() + arrowMargin),
                           QPointF(baseX, sheetRect.bottomRight().y()));
    VPieceGrainline const rightArrow(rightLine, GrainlineArrowDirection::oneWayUp);
    QPainterPath rightArrowPath = VLayoutPiece::GrainlinePath(rightArrow.Shape());
    rightArrowPath.setFillRule(Qt::WindingFill);

    painter->drawPath(rightArrowPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintHorizontalFold(QPainter *painter, const QRectF &sheetRect) const
{
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); layout.isNull() || !layout->LayoutSettings().IsCutOnFold())
    {
        return;
    }

    QPainterStateGuard const guard(painter);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    const bool penPrinting = m_printMode && (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont());

    if (m_printMode)
    {
        QPen pen = painter->pen();
        pen.setWidthF(settings->WidthHairLine());
        pen.setColor(tileColor);
        painter->setPen(pen);
    }

    const QRectF textRect = !m_printMode || settings->GetSingleStrokeOutlineFont() || !settings->GetSingleLineFonts()
                                ? PaintHorizontalFoldTextOutlineFont(painter, sheetRect)
                                : PaintHorizontalFoldTextSVGFont(painter, sheetRect);

    if (sheetRect.height() < textRect.width() * 2)
    {
        return;
    }

    qreal const baseY = textRect.center().y();
    qreal const arrowMargin = foldArrowMargin + textRect.width() / 2.;

    QLineF const leftLine(QPointF(sheetRect.topLeft().x(), baseY), QPointF(sheetRect.center().x() - arrowMargin, baseY));
    VPieceGrainline const leftArrow(leftLine, GrainlineArrowDirection::oneWayDown);
    QPainterPath leftArrowPath = VLayoutPiece::GrainlinePath(leftArrow.Shape());
    leftArrowPath.setFillRule(Qt::WindingFill);

    QPen pen = painter->pen();
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (m_printMode)
    {
        pen.setBrush(Qt::NoBrush);
        pen.setCosmetic(true);
        pen.setColor(tileColor);

        if (!penPrinting)
        {
            painter->setBrush(QBrush(tileColor, Qt::SolidPattern));
        }
    }
    else
    {
        painter->setBrush(QBrush(pen.color(), Qt::SolidPattern));
    }
    painter->setPen(pen);
    painter->drawPath(leftArrowPath);

    QLineF const rightLine(QPointF(sheetRect.center().x() + arrowMargin, baseY),
                           QPointF(sheetRect.topRight().x(), baseY));
    VPieceGrainline const rightArrow(rightLine, GrainlineArrowDirection::oneWayUp);
    QPainterPath rightArrowPath = VLayoutPiece::GrainlinePath(rightArrow.Shape());
    rightArrowPath.setFillRule(Qt::WindingFill);

    painter->drawPath(rightArrowPath);
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
    if (VPSheetPtr const sheet = layout->GetFocusedSheet(); !sheet.isNull())
    {
        ignoreMargins = sheet->IgnoreMargins();
    }

    if (m_showMargin && !ignoreMargins)
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetMarginColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        QPainterStateGuard const guard(painter);
        painter->setPen(pen);
        painter->drawRect(GetMarginsRect());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintBorder(QPainter *painter) const
{
    if (!m_showBorder)
    {
        return;
    }

    QRectF const sheetRect = GetSheetRect();

    QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetBorderColor(), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);

    QPainterStateGuard const guard(painter);
    painter->setPen(pen);
    painter->drawRect(sheetRect);

    if (VPLayoutPtr const layout = m_layout.toStrongRef(); !layout.isNull() && layout->LayoutSettings().IsCutOnFold())
    {
        GrainlineType grainlineType = GrainlineType::NotFixed;
        if (VPSheetPtr const sheet = layout->GetFocusedSheet(); !sheet.isNull())
        {
            grainlineType = sheet->GetGrainlineType();
        }

        if (const bool isWide = sheetRect.width() >= sheetRect.height();
            grainlineType == GrainlineType::Horizontal || (grainlineType == GrainlineType::NotFixed && isWide))
        {
            PaintVerticalFoldShadow(painter, sheetRect);
        }
        else
        {
            PaintHorizontalFoldShadow(painter, sheetRect);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintFold(QPainter *painter) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull() || !layout->LayoutSettings().IsCutOnFold())
    {
        return;
    }

    GrainlineType grainlineType = GrainlineType::NotFixed;
    if (VPSheetPtr const sheet = layout->GetFocusedSheet(); !sheet.isNull())
    {
        grainlineType = sheet->GetGrainlineType();
    }

    QRectF const sheetRect = GetSheetRect();
    QRectF const foldField = m_showBorder ? sheetRect : FoldField(GetMarginsRect());

    if (const bool isWide = sheetRect.width() >= sheetRect.height();
        grainlineType == GrainlineType::Horizontal || (grainlineType == GrainlineType::NotFixed && isWide))
    {
        PaintHorizontalFold(painter, foldField);
    }
    else
    {
        PaintVerticalFold(painter, foldField);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::PaintGrid(QPainter *painter) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (not layout.isNull() && layout->LayoutSettings().GetShowGrid())
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetGridColor(), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setCosmetic(true);

        QPainterStateGuard const guard(painter);
        painter->setPen(pen);

        QRectF const sheetRect = GetSheetRect();

        if (qreal const colWidth = layout->LayoutSettings().GetGridColWidth(); colWidth > 0)
        {
            qreal colX = colWidth;
            while (colX < sheetRect.right())
            {
                auto const line = QLineF(colX, 0, colX, sheetRect.bottom());
                painter->drawLine(line);
                colX += colWidth;
            }
        }

        if (qreal const rowHeight = layout->LayoutSettings().GetGridRowHeight(); rowHeight > 0)
        {
            qreal rowY = rowHeight;

            while (rowY < sheetRect.bottom())
            {
                auto const line = QLineF(0, rowY, sheetRect.right(), rowY);
                painter->drawLine(line);
                rowY += rowHeight;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::FoldField(const QRectF &sheetRect) const -> QRectF
{
    if (!m_PDFTiledExport)
    {
        return sheetRect;
    }

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
auto VPGraphicsSheet::PaintHorizontalFoldTextOutlineFont(QPainter *painter, const QRectF &sheetRect) const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (layout.isNull())
    {
        return {};
    }

    QPainterStateGuard guard(painter);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    QString const foldText = FoldText();

    // Font setup
    QFont font = m_printMode && settings->GetSingleStrokeOutlineFont()
                     ? layout->TileFactory()->SingleStrokeOutlineFont()
                     : QApplication::font();
    font.setPointSize(foldFontSize);
    painter->setFont(font);

    const QFontMetrics fm(font);
    const int textDescent = fm.descent();
    QRectF textRect = painter->fontMetrics().boundingRect(foldText);

    // Pen setup for print mode
    if (m_printMode)
    {
        QPen pen = painter->pen();
        pen.setColor(tileColor);
        painter->setPen(pen);
    }

    // Text position calculation
    const qreal centerX = sheetRect.center().x();
    const qreal topY = sheetRect.topLeft().y();
    const QPointF textPosition = m_printMode
                                     ? QPointF(centerX - textRect.width() / 2., topY + textRect.height() - textDescent)
                                     : QPointF(centerX - textRect.width() / 2., topY - foldTextMargin - textDescent);

    // Rendering
    if (m_printMode && settings->GetSingleStrokeOutlineFont())
    {
        static VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        QPainterPath path;
        int w = 0;
        for (auto c : foldText)
        {
            path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        QPen pen = painter->pen();
        pen.setStyle(Qt::SolidLine);
        pen.setWidthF(settings->WidthHairLine());
        painter->setPen(pen);

        QPointF const textOffset(textRect.width() / 2.0, textRect.height() / 2.0);

        QTransform transform;
        transform.translate(sheetRect.center().x() - textOffset.x(), 0);

        painter->drawPath(transform.map(path));
    }
    else if ((m_printMode && !settings->GetSingleStrokeOutlineFont())
             || (!m_printMode && font.pointSizeF() * SceneScale(scene()) >= minTextFontSize))
    {
        painter->drawText(textPosition, foldText);
    }
    else
    {
        QPainterPath path;
        path.addText(QPointF(), font, foldText);

        QPointF const textOffset(textRect.width() / 2.0, textRect.height() / 2.0);

        QTransform transform;
        transform.translate(sheetRect.center().x() - textOffset.x(), -textRect.height() / 2 + fm.descent());

        guard.save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QBrush(painter->pen().color(), Qt::SolidPattern));
        painter->drawPath(transform.map(path));
        guard.restore();
    }

    // Adjust returned rect for alignment
    textRect.translate(sheetRect.center() - textRect.center());

    if (const qreal verticalAdjust = (sheetRect.center().y() - sheetRect.topLeft().y()); !m_printMode)
    {
        textRect.translate(0, -verticalAdjust - foldTextMargin - textRect.height() / 2.);
    }
    else
    {
        textRect.translate(0, -verticalAdjust + textRect.height() / 2.);
    }

    // painter->drawRect(textRect); // uncomment for debug

    return textRect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::PaintHorizontalFoldTextSVGFont(QPainter *painter, const QRectF &sheetRect) const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (layout.isNull())
    {
        return {};
    }

    QString const svgFontFamily = layout->TileFactory()->SVGFontFamily();

    const VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine const engine = db->FontEngine(svgFontFamily,
                                                 SVGFontStyle::Normal,
                                                 SVGFontWeight::Normal,
                                                 foldFontSize);

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        return PaintHorizontalFoldTextOutlineFont(painter, sheetRect);
    }

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    QPainterStateGuard const guard(painter);

    QPen pen = painter->pen();
    pen.setColor(tileColor);
    pen.setWidthF(settings->WidthHairLine());
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    QString const foldText = FoldText();

    QRectF textRect = engine.BoundingRect(foldText, pen.widthF());
    QPointF const textOffset(textRect.width() / 2.0, textRect.height() / 2.0);
    // QPointF const centerPoint = sheetRect.center();

    QTransform transform;
    transform.translate(sheetRect.center().x() - textOffset.x(), foldTextMargin);

    QPainterPath const path = engine.DrawPath(QPointF(), foldText, pen.widthF());

    painter->drawPath(transform.map(path));

    textRect.translate(sheetRect.center() - textRect.center());
    textRect.translate(0, -(sheetRect.center().y() - sheetRect.topLeft().y()) + textRect.height() / 2. + foldTextMargin);

    // painter->drawRect(textRect); // uncomment for debug
    return textRect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::PaintVerticalFoldTextOutlineFont(QPainter *painter, const QRectF &sheetRect) const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (layout.isNull())
    {
        return {};
    }

    QPainterStateGuard guard(painter);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    QString const foldText = FoldText();

    // Font setup
    QFont font = m_printMode && settings->GetSingleStrokeOutlineFont()
                     ? layout->TileFactory()->SingleStrokeOutlineFont()
                     : QApplication::font();
    font.setPointSize(foldFontSize);
    painter->setFont(font);

    const QFontMetrics fm(font);
    const int textDescent = fm.descent();
    QRectF const textRect = painter->fontMetrics().boundingRect(foldText);

    QRectF realSheetRect = sheetRect;
    if (m_printMode)
    {
        realSheetRect = GetMarginsRect();
        realSheetRect = QRectF(realSheetRect.topLeft(),
                               QPointF(realSheetRect.bottomRight().x() - painter->pen().widthF(),
                                       sheetRect.bottomRight().y()));
    }

    // Pen setup for print mode
    if (m_printMode)
    {
        QPen pen = painter->pen();
        pen.setColor(tileColor);
        painter->setPen(pen);

        guard.save();

        pen = painter->pen();
        pen.setCosmetic(true);
        pen.setWidthF(settings->WidthHairLine());
        if (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont())
        {
            pen.setDashPattern({12, 4});
        }
        else
        {
            pen.setStyle(Qt::DashLine);
        }
        painter->setPen(pen);

        // Draw fold edge
        painter->drawLine(QLineF(realSheetRect.topRight(), {realSheetRect.bottomRight()}));
        guard.restore();
    }

    const qreal centerX = realSheetRect.center().x();
    const qreal centerY = realSheetRect.center().y();
    const QPointF textPosition = m_printMode
                                     ? QPointF(centerX - textRect.width() / 2.,
                                               centerY - realSheetRect.width() / 2. + foldTextMargin + textRect.width())
                                     : QPointF(centerX - textRect.width() / 2.,
                                               centerY - realSheetRect.width() / 2. - foldTextMargin - textDescent);

    // Rendering
    if (m_printMode && settings->GetSingleStrokeOutlineFont())
    {
        static VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        QPainterPath path;
        int w = 0;
        for (auto c : foldText)
        {
            path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        guard.save();

        QPen pen = painter->pen();
        pen.setStyle(Qt::SolidLine);
        pen.setWidthF(settings->WidthHairLine());
        painter->setPen(pen);

        painter->translate(realSheetRect.center());
        painter->rotate(90);
        painter->translate(-realSheetRect.center());
        painter->translate(textPosition.x(), textPosition.y() - textRect.width() + textDescent);

        painter->drawPath(path);

        guard.restore();
    }
    else if (m_printMode && !settings->GetSingleStrokeOutlineFont())
    {
        guard.save();

        painter->translate(realSheetRect.center());
        painter->rotate(90);
        painter->translate(-realSheetRect.center());
        painter->translate(0, -textRect.height() / 2. - textDescent);

        painter->drawText(textPosition, foldText);

        guard.restore();
    }
    else if (!m_printMode && font.pointSizeF() * SceneScale(scene()) >= minTextFontSize)
    {
        guard.save();

        painter->translate(realSheetRect.center());
        painter->rotate(90);
        painter->translate(-realSheetRect.center());

        painter->drawText(textPosition, foldText);

        guard.restore();
    }
    else
    {
        QPainterPath path;
        path.addText(QPointF(), font, foldText);

        guard.save();

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QBrush(painter->pen().color(), Qt::SolidPattern));

        painter->translate(realSheetRect.center());
        painter->rotate(90);
        painter->translate(-realSheetRect.center());
        painter->translate(textPosition);

        painter->drawPath(path);

        guard.restore();
    }

    QRectF swappedRect = SwapRect(textRect);
    swappedRect.translate(realSheetRect.center() - swappedRect.center());

    if (m_printMode)
    {
        const qreal horizontalAdjust = (realSheetRect.center().x() - realSheetRect.topRight().x());
        swappedRect.translate(-horizontalAdjust - foldTextMargin - swappedRect.height() / 2., 0);
    }
    else
    {
        swappedRect.translate((sheetRect.topRight().x() - sheetRect.center().x()) + foldTextMargin
                                  + textRect.height() / 2.,
                              0);
    }

    // painter->drawRect(swappedRect); // uncomment for debug

    return swappedRect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::PaintVerticalFoldTextSVGFont(QPainter *painter, const QRectF &sheetRect) const -> QRectF
{
    VPLayoutPtr const layout = m_layout.toStrongRef();

    if (layout.isNull())
    {
        return {};
    }

    QString const svgFontFamily = layout->TileFactory()->SVGFontFamily();

    const VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine const engine = db->FontEngine(svgFontFamily,
                                                 SVGFontStyle::Normal,
                                                 SVGFontWeight::Normal,
                                                 foldFontSize);

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        return PaintHorizontalFoldTextOutlineFont(painter, sheetRect);
    }

    QRectF realSheetRect = sheetRect;
    if (m_printMode)
    {
        realSheetRect = GetMarginsRect();
        realSheetRect = QRectF(realSheetRect.topLeft(),
                               QPointF(realSheetRect.bottomRight().x() - painter->pen().widthF(),
                                       sheetRect.bottomRight().y()));
    }

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    QPainterStateGuard guard(painter);

    QPen pen = painter->pen();
    pen.setColor(tileColor);
    pen.setWidthF(settings->WidthHairLine());
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    guard.save();

    pen = painter->pen();
    pen.setCosmetic(true);
    pen.setWidthF(settings->WidthHairLine());
    if (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont())
    {
        pen.setDashPattern({12, 4});
    }
    else
    {
        pen.setStyle(Qt::DashLine);
    }
    painter->setPen(pen);

    // Draw fold edge
    painter->drawLine(QLineF(realSheetRect.topRight(), {realSheetRect.bottomRight()}));
    guard.restore();

    QString const foldText = FoldText();

    const QRectF textRect = engine.BoundingRect(foldText, pen.widthF());

    // Text position calculation
    const qreal centerX = realSheetRect.center().x();
    const qreal centerY = realSheetRect.center().y();
    const auto textPosition = QPointF(centerX - textRect.width() / 2.,
                                      centerY - realSheetRect.width() / 2. + foldTextMargin + textRect.height() / 2);

    guard.save();

    painter->translate(realSheetRect.center());
    painter->rotate(90);
    painter->translate(-realSheetRect.center());

    painter->translate(textPosition);

    QPainterPath const path = engine.DrawPath(QPointF(), foldText, pen.widthF());
    painter->drawPath(path);

    guard.restore();

    QRectF swappedRect = SwapRect(textRect);
    swappedRect.translate(realSheetRect.topRight().x() - textRect.width() / 2 - foldTextMargin - textRect.height(),
                          centerY - textRect.height() / 2);

    // painter->drawRect(swappedRect); // uncomment for debug
    return swappedRect;
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

    if (VPLayoutPtr const layout = m_layout.toStrongRef();
        not layout.isNull() && layout->LayoutSettings().IsCutOnFold())
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
