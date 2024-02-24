#include "vpgraphicstilegrid.h"

#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vptilefactory.h"
#include "../vwidgets/global.h"

#include <QFileInfo>
#include <QImageReader>
#include <QPixmapCache>
#include <QSvgRenderer>

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
auto OptimizeFontSizeToFitTextInRect(QPainter *painter, const QRectF &drawRect, const QString &text,
                                     int flags = Qt::TextDontClip | Qt::TextWordWrap, double goalError = 0.01,
                                     int maxIterationNumber = 10) -> QFont
{
    QFont font;

    if (not drawRect.isValid())
    {
        font.setPointSizeF(0.00000001);
        return font;
    }

    painter->save();

    double minError = std::numeric_limits<double>::max();
    double error = std::numeric_limits<double>::max();
    int iterationNumber = 0;
    while ((error > goalError) && (iterationNumber < maxIterationNumber))
    {
        iterationNumber++;
        QRect const fontBoundRect = painter->fontMetrics().boundingRect(drawRect.toRect(), flags, text);
        if (fontBoundRect.isNull())
        {
            font.setPointSizeF(0.00000001);
            break;
        }

        double const xFactor = drawRect.width() / fontBoundRect.width();
        double const yFactor = drawRect.height() / fontBoundRect.height();
        double factor;
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
        font = painter->font();
        qreal size = font.pointSizeF() * factor;
        if (size <= 0)
        {
            size = 0.00000001;
        }
        font.setPointSizeF(size);
        painter->setFont(font);
    }
    painter->restore();

    return font;
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
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (not layout.isNull() && layout->LayoutSettings().GetShowTiles())
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

    auto PaintWatermark = [painter, layout, xScale, yScale, &watermarkData](const QRectF &img)
    {
        if (not layout->LayoutSettings().WatermarkPath().isEmpty() && layout->LayoutSettings().GetShowWatermark() &&
            watermarkData.opacity > 0)
        {
            if (watermarkData.showImage && not watermarkData.path.isEmpty())
            {
                VPTileFactory::PaintWatermarkImage(painter, img, watermarkData,
                                                   layout->LayoutSettings().WatermarkPath(), true, xScale, yScale);
            }

            if (watermarkData.showText && not watermarkData.text.isEmpty())
            {
                VPTileFactory::PaintWatermarkText(painter, img, watermarkData);
            }
        }
    };

    const qreal width = (layout->TileFactory()->DrawingAreaWidth() - VPTileFactory::tileStripeWidth) / xScale;
    const qreal height = (layout->TileFactory()->DrawingAreaHeight() - VPTileFactory::tileStripeWidth) / yScale;

    VPSheetPtr const sheet = layout->GetSheet(m_sheetUuid);
    QMarginsF const sheetMargins = SheetMargins(sheet);

    const int nbCol = layout->TileFactory()->ColNb(sheet);
    const int nbRow = layout->TileFactory()->RowNb(sheet);

    QFont const font = OptimizeFontSizeToFitTextInRect(
        painter, QRectF(sheetMargins.left(), sheetMargins.top(), width / 3., height / 3.),
        QString::number(nbRow * nbCol));

    const qreal scale = SceneScale(scene());

    auto PaintTileNumber = [painter, layout, nbCol, font, scale](const QRectF &img, int i, int j)
    {
        if (layout->LayoutSettings().GetShowTileNumber() && font.pointSizeF() * scale >= minTextFontSize)
        {
            painter->save();

            painter->setFont(font);

            QPen pen = painter->pen();
            pen.setColor(VSceneStylesheet::ManualLayoutStyle().SheetTileNumberColor());
            painter->setPen(pen);

            painter->drawText(img, Qt::AlignCenter, QString::number(j * nbCol + i + 1));

            painter->restore();
        }
    };

    for (int j = 0; j <= nbRow; ++j)
    {
        // horizontal lines
        painter->drawLine(QPointF(sheetMargins.left(), sheetMargins.top() + j * height),
                          QPointF(sheetMargins.left() + nbCol * width, sheetMargins.top() + j * height));

        for (int i = 0; i <= nbCol; ++i)
        {
            // vertical lines
            painter->drawLine(QPointF(sheetMargins.left() + i * width, sheetMargins.top()),
                              QPointF(sheetMargins.left() + i * width, sheetMargins.top() + nbRow * height));

            if (j < nbRow && i < nbCol)
            {
                QRectF const img(sheetMargins.left() + i * width, sheetMargins.top() + j * height, width, height);

                PaintWatermark(img);
                PaintTileNumber(img, i, j);
            }
        }
    }
}
