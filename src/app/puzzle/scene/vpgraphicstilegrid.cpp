#include "vpgraphicstilegrid.h"

#include <cmath>

#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "../vformat/vsinglelineoutlinechar.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vabstractapplication.h"
#include "../vptilefactory.h"
#include "../vwidgets/global.h"
#include "scenedef.h"

#include <QFileInfo>
#include <QImageReader>
#include <QPixmapCache>
#include <QSvgRenderer>

#include <../vmisc/svgfont/vsvgfontdatabase.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

namespace
{
constexpr qreal penWidth = 1;
constexpr qreal minTextFontSize = 5.0;

//---------------------------------------------------------------------------------------------------------------------
auto SheetMargins(const VPSheetPtr &sheet) -> QMarginsF
{
    if (not sheet.isNull() && not sheet->IgnoreMargins())
    {
        return sheet->GetSheetMargins();
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto OptimizeOutlineFontSizeToFitTextInRect(QFont font,
                                            const QRectF &drawRect,
                                            const QString &text,
                                            int flags = Qt::TextDontClip | Qt::TextWordWrap,
                                            double goalError = 0.01,
                                            int maxIterationNumber = 10) -> QFont
{
    if (not drawRect.isValid())
    {
        font.setPointSizeF(0.00000001);
        return font;
    }

    double minError = std::numeric_limits<double>::max();
    double error = std::numeric_limits<double>::max();
    int iterationNumber = 0;
    while ((error > goalError) && (iterationNumber < maxIterationNumber))
    {
        iterationNumber++;
        QFontMetrics const fm(font);
        QRect const fontBoundRect = fm.boundingRect(drawRect.toRect(), flags, text);
        if (fontBoundRect.isNull())
        {
            font.setPointSizeF(0.00000001);
            break;
        }

        double const xFactor = drawRect.width() / fontBoundRect.width();
        double const yFactor = drawRect.height() / fontBoundRect.height();
        double factor = NAN;
        if (xFactor < 1 && yFactor < 1)
        {
            factor = std::min(xFactor, yFactor);
        }
        else if (xFactor > 1 && yFactor > 1)
        {
            factor = std::max(xFactor, yFactor);
        }
        else if (xFactor < 1 && yFactor > 1)
        {
            factor = xFactor;
        }
        else
        {
            factor = yFactor;
        }

        error = qFabs(factor - 1);
        if (factor > 1)
        {
            if (error < minError)
            {
                minError = error;
            }
            else
            {
                break;
            }
        }
        qreal size = font.pointSizeF() * factor;
        if (size <= 0)
        {
            size = 0.00000001;
        }
        font.setPointSizeF(size);
    }

    return font;
}

//---------------------------------------------------------------------------------------------------------------------
auto OptimizeSVGFontSizeToFitTextInRect(const QString &family,
                                        SVGFontStyle style,
                                        SVGFontWeight weight,
                                        const QRectF &drawRect,
                                        const QString &text,
                                        qreal penWidth = 0,
                                        double goalError = 0.01,
                                        int maxIterationNumber = 10) -> qreal
{
    qreal fontSize = 1;
    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine = db->FontEngine(family, style, weight, qRound(fontSize));

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        return 0.00000001;
    }

    double minError = std::numeric_limits<double>::max();
    double error = std::numeric_limits<double>::max();
    int iterationNumber = 0;
    while ((error > goalError) && (iterationNumber < maxIterationNumber))
    {
        iterationNumber++;
        QRectF const fontBoundRect = engine.BoundingRect(text, penWidth);
        if (fontBoundRect.isNull())
        {
            return 0.00000001;
        }

        double const xFactor = drawRect.width() / fontBoundRect.width();
        double const yFactor = drawRect.height() / fontBoundRect.height();
        double factor = NAN;
        if (xFactor < 1 && yFactor < 1)
        {
            factor = std::min(xFactor, yFactor);
        }
        else if (xFactor > 1 && yFactor > 1)
        {
            factor = std::max(xFactor, yFactor);
        }
        else if (xFactor < 1 && yFactor > 1)
        {
            factor = xFactor;
        }
        else
        {
            factor = yFactor;
        }

        error = qFabs(factor - 1);
        if (factor > 1)
        {
            if (error < minError)
            {
                minError = error;
            }
            else
            {
                break;
            }
        }
        qreal size = fontSize * factor;
        if (size <= 0)
        {
            size = 0.00000001;
        }
        fontSize = size;
        engine = db->FontEngine(family, style, weight, qRound(fontSize));
    }

    return fontSize;
}

//---------------------------------------------------------------------------------------------------------------------
void PaintWatermark(QPainter *painter,
                    const QRectF &img,
                    const VPLayoutPtr &layout,
                    const VWatermarkData &watermarkData,
                    qreal xScale,
                    qreal yScale)
{
    if (not layout->LayoutSettings().WatermarkPath().isEmpty() && layout->LayoutSettings().GetShowWatermark()
        && watermarkData.opacity > 0)
    {
        if (watermarkData.showImage && not watermarkData.path.isEmpty())
        {
            VPTileFactory::PaintWatermarkImage(painter,
                                               img,
                                               watermarkData,
                                               layout->LayoutSettings().WatermarkPath(),
                                               true,
                                               xScale,
                                               yScale);
        }

        if (watermarkData.showText && not watermarkData.text.isEmpty())
        {
            VPTileFactory::PaintWatermarkText(painter, img, watermarkData);
        }
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsTileGrid::VPGraphicsTileGrid(const VPLayoutPtr &layout, const QUuid &sheetUuid, QGraphicsItem *parent)
  : QGraphicsItem(parent),
    m_layout(layout),
    m_sheetUuid(sheetUuid)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTileGrid::boundingRect() const -> QRectF
{
    if (VPLayoutPtr const layout = m_layout.toStrongRef();
        not layout.isNull() && layout->LayoutSettings().GetShowTiles())
    {
        VPSheetPtr const sheet = layout->GetSheet(m_sheetUuid);

        QMarginsF sheetMargins;
        if (not sheet.isNull() && not sheet->IgnoreMargins())
        {
            sheetMargins = sheet->GetSheetMargins();
        }

        qreal const xScale = layout->LayoutSettings().HorizontalScale();
        qreal const yScale = layout->LayoutSettings().VerticalScale();

        qreal const width = layout->TileFactory()->DrawingAreaWidth() - VPTileFactory::tileStripeWidth;
        qreal const height = layout->TileFactory()->DrawingAreaHeight() - VPTileFactory::tileStripeWidth;

        QRectF const rect(sheetMargins.left(), sheetMargins.top(),
                          layout->TileFactory()->ColNb(sheet) * (width / xScale),
                          layout->TileFactory()->RowNb(sheet) * (height / yScale));

        constexpr qreal halfPenWidth = penWidth / 2.;

        return rect.adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTileGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull() || not layout->LayoutSettings().GetShowTiles())
    {
        return;
    }

    QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetTileGridColor(), penWidth, Qt::SolidLine, Qt::RoundCap,
             Qt::RoundJoin);
    pen.setCosmetic(true);
    pen.setStyle(Qt::DashLine);
    QBrush const noBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->setBrush(noBrush);

    qreal const xScale = layout->LayoutSettings().HorizontalScale();
    qreal const yScale = layout->LayoutSettings().VerticalScale();

    VWatermarkData const watermarkData = layout->TileFactory()->WatermarkData();

    const qreal width = (layout->TileFactory()->DrawingAreaWidth() - VPTileFactory::tileStripeWidth) / xScale;
    const qreal height = (layout->TileFactory()->DrawingAreaHeight() - VPTileFactory::tileStripeWidth) / yScale;

    VPSheetPtr const sheet = layout->GetSheet(m_sheetUuid);
    QMarginsF const sheetMargins = SheetMargins(sheet);

    const int nbCol = layout->TileFactory()->ColNb(sheet);
    const int nbRow = layout->TileFactory()->RowNb(sheet);

    QFont font;
    QString svgFontFamily;

    VSvgFontEngine engine;

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    if (m_printMode && settings->GetSingleLineFonts())
    {
        svgFontFamily = layout->TileFactory()->SVGFontFamily();
        VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
        qreal const fontSize = OptimizeSVGFontSizeToFitTextInRect(svgFontFamily,
                                                                  SVGFontStyle::Normal,
                                                                  SVGFontWeight::Normal,
                                                                  QRectF(sheetMargins.left(),
                                                                         sheetMargins.top(),
                                                                         width / 3.,
                                                                         height / 3.),
                                                                  QString::number(nbRow * nbCol),
                                                                  pen.widthF());
        engine = db->FontEngine(svgFontFamily, SVGFontStyle::Normal, SVGFontWeight::Normal, qRound(fontSize));
    }
    else
    {
        font = OptimizeOutlineFontSizeToFitTextInRect(m_printMode && settings->GetSingleStrokeOutlineFont()
                                                          ? layout->TileFactory()->SingleStrokeOutlineFont()
                                                          : font,
                                                      QRectF(sheetMargins.left(),
                                                             sheetMargins.top(),
                                                             width / 3.,
                                                             height / 3.),
                                                      QString::number(nbRow * nbCol));
    }

    DrawGrid(painter, nbRow, nbCol, sheetMargins, width, height);

    for (int j = 0; j <= nbRow; ++j)
    {
        for (int i = 0; i <= nbCol; ++i)
        {
            if (j < nbRow && i < nbCol)
            {
                QRectF const img(sheetMargins.left() + i * width, sheetMargins.top() + j * height, width, height);

                PaintWatermark(painter, img, layout, watermarkData, xScale, yScale);
                if (m_printMode && settings->GetSingleLineFonts())
                {
                    PaintTileNumberSVGFont(painter, img, i, j, layout, font, nbCol, engine);
                }
                else
                {
                    PaintTileNumberOutlineFont(painter, img, i, j, layout, font, nbCol);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTileGrid::PaintTileNumberOutlineFont(
    QPainter *painter, const QRectF &img, int i, int j, const VPLayoutPtr &layout, const QFont &font, int nbCol) const
{
    if (!layout->LayoutSettings().GetShowTileNumber())
    {
        return;
    }

    QPainterStateGuard const guard(painter);

    painter->setFont(font);

    QPen pen = painter->pen();
    if (!m_printMode)
    {
        pen.setColor(VSceneStylesheet::ManualLayoutStyle().SheetTileNumberColor());
    }
    else
    {
        pen.setColor(tileColor);
    }
    painter->setPen(pen);

    const qreal scale = SceneScale(scene());

    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        m_printMode && settings->GetSingleStrokeOutlineFont())
    {
        static VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        QPainterPath path;
        QFontMetrics const fm(font);

        const QString tileNumber = QString::number(j * nbCol + i + 1);

        int w = 0;
        for (auto c : tileNumber)
        {
            path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        pen = painter->pen();
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);

        QRectF const textRect = fm.boundingRect(tileNumber);
        QPointF const textOffset(textRect.width() / 2.0, textRect.height() / 2.0);
        QPointF const centerPoint = img.center();

        QTransform transform;
        transform.translate(centerPoint.x() - textOffset.x(), centerPoint.y() - textOffset.y());

        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawPath(transform.map(path));
    }
    else if (font.pointSizeF() * scale >= minTextFontSize)
    {
        painter->drawText(img, Qt::AlignCenter, QString::number(j * nbCol + i + 1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTileGrid::PaintTileNumberSVGFont(QPainter *painter,
                                                const QRectF &img,
                                                int i,
                                                int j,
                                                const VPLayoutPtr &layout,
                                                const QFont &font,
                                                int nbCol,
                                                const VSvgFontEngine &engine) const
{
    if (!layout->LayoutSettings().GetShowTileNumber())
    {
        return;
    }

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        PaintTileNumberOutlineFont(painter, img, i, j, layout, font, nbCol);
        return;
    }

    QPainterStateGuard const guard(painter);

    QPen pen = painter->pen();

    if (!m_printMode)
    {
        pen.setColor(VSceneStylesheet::ManualLayoutStyle().SheetTileNumberColor());
    }
    else
    {
        pen.setColor(tileColor);
    }

    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    const QString tileNumber = QString::number(j * nbCol + i + 1);

    QRectF const textRect = engine.BoundingRect(tileNumber, pen.widthF());
    QPointF const textOffset(textRect.width() / 2.0, textRect.height() / 2.0);
    QPointF const centerPoint = img.center();

    QTransform transform;
    transform.translate(centerPoint.x() - textOffset.x(), centerPoint.y() - textOffset.y());

    QPainterPath const path = engine.DrawPath(QPointF(), tileNumber, pen.widthF());

    painter->drawPath(transform.map(path));
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTileGrid::DrawGrid(
    QPainter *painter, int nbRow, int nbCol, const QMarginsF &margins, qreal width, qreal height) const
{
    QPainterStateGuard const guard(painter);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    const bool penPrinting = m_printMode && (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont());

    QPen pen = painter->pen();
    if (penPrinting)
    {
        pen.setDashPattern({12, 4});
        painter->setPen(pen);
    }

    const qreal left = margins.left();
    const qreal top = margins.top();
    const qreal penWidth = pen.widthF();
    const qreal totalWidth = nbCol * width;
    const qreal totalHeight = nbRow * height;

    // Draw horizontal lines
    for (int j = 0; j <= nbRow; ++j)
    {
        qreal y = top + j * height;

        // Apply pen margin only for the first line
        if (j == 0)
        {
            y += penWidth * 2;
        }

        painter->drawLine(QPointF(left, y), QPointF(left + totalWidth, y));
    }

    // Draw vertical lines
    for (int i = 0; i <= nbCol; ++i)
    {
        const qreal x = left + i * width;
        painter->drawLine(QPointF(x, top), QPointF(x, top + totalHeight));
    }
}
