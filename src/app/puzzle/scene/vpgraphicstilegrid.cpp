#include "vpgraphicstilegrid.h"

#include "../vptilefactory.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "qnamespace.h"

#include <QFileInfo>
#include <QImageReader>
#include <QPixmapCache>
#include <QSvgRenderer>

namespace
{
constexpr qreal penWidth = 1;

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
                                     int flags = Qt::TextDontClip|Qt::TextWordWrap, double goalError =  0.01,
                                     int maxIterationNumber=10) -> QFont
{
    painter->save();

    QRect fontBoundRect;
    QFont font;
    double minError = std::numeric_limits<double>::max();
    double error = std::numeric_limits<double>::max();
    int iterationNumber=0;
    while((error > goalError) && (iterationNumber<maxIterationNumber))
    {
        iterationNumber++;
        fontBoundRect = painter->fontMetrics().boundingRect(drawRect.toRect(), flags, text);
        double xFactor = drawRect.width() / fontBoundRect.width();
        double yFactor = drawRect.height() / fontBoundRect.height();
        double factor;
        if (xFactor<1 && yFactor<1)
        {
            factor = std::min(xFactor, yFactor);
        }
        else if (xFactor>1 && yFactor>1)
        {
            factor = std::max(xFactor, yFactor);
        }
        else if (xFactor<1 && yFactor>1)
        {
            factor = xFactor;
        }
        else
        {
            factor = yFactor;
        }

        error = abs(factor-1);
        if (factor > 1 )
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
        font.setPointSizeF(font.pointSizeF()*factor);
        painter->setFont(font);
    }
    painter->restore();

    return font;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsTileGrid::VPGraphicsTileGrid(const VPLayoutPtr &layout, const QUuid &sheetUuid, QGraphicsItem *parent):
    QGraphicsItem(parent),
    m_layout(layout),
    m_sheetUuid(sheetUuid)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTileGrid::boundingRect() const -> QRectF
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if(not layout.isNull() && layout->LayoutSettings().GetShowTiles())
    {
        VPSheetPtr sheet = layout->GetSheet(m_sheetUuid);

        QMarginsF sheetMargins;
        if (not sheet.isNull() && not sheet->IgnoreMargins())
        {
            sheetMargins = sheet->GetSheetMargins();
        }

        qreal xScale = layout->LayoutSettings().HorizontalScale();
        qreal yScale = layout->LayoutSettings().VerticalScale();

        qreal width = layout->TileFactory()->DrawingAreaWidth() - VPTileFactory::tileStripeWidth;
        qreal height = layout->TileFactory()->DrawingAreaHeight() - VPTileFactory::tileStripeWidth;

        QRectF rect(sheetMargins.left(), sheetMargins.top(),
                    layout->TileFactory()->ColNb(sheet) * (width / xScale),
                    layout->TileFactory()->RowNb(sheet) * (height / yScale));

        constexpr qreal halfPenWidth = penWidth/2.;

        return rect.adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTileGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    VPLayoutPtr layout = m_layout.toStrongRef();
    if(layout.isNull() || not layout->LayoutSettings().GetShowTiles())
    {
        return;
    }

    QPen pen(QColor(255,0,0,127), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);
    pen.setStyle(Qt::DashLine);
    QBrush noBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->setBrush(noBrush);

    qreal xScale = layout->LayoutSettings().HorizontalScale();
    qreal yScale = layout->LayoutSettings().VerticalScale();

    const qreal width = (layout->TileFactory()->DrawingAreaWidth() - VPTileFactory::tileStripeWidth) / xScale;
    const qreal height = (layout->TileFactory()->DrawingAreaHeight() - VPTileFactory::tileStripeWidth) / yScale;

    VPSheetPtr sheet = layout->GetSheet(m_sheetUuid);
    QMarginsF sheetMargins = SheetMargins(sheet);

    const int nbCol = layout->TileFactory()->ColNb(sheet);
    const int nbRow = layout->TileFactory()->RowNb(sheet);

    QFont font = OptimizeFontSizeToFitTextInRect(painter,
                                                 QRectF(sheetMargins.left(), sheetMargins.top(), width/3., height/3.),
                                                 QString::number(nbRow * nbCol));

    VWatermarkData watermarkData = layout->TileFactory()->WatermarkData();

    auto PaintWatermark = [painter, layout, xScale, yScale, watermarkData]
            (const QRectF &img)
    {
        if (not layout->LayoutSettings().WatermarkPath().isEmpty() &&
                layout->LayoutSettings().GetShowWatermark() && watermarkData.opacity > 0)
        {
            if (watermarkData.showImage && not watermarkData.path.isEmpty())
            {
                VPTileFactory::PaintWatermarkImage(painter, img, watermarkData,
                                                   layout->LayoutSettings().WatermarkPath(),
                                                   xScale, yScale);
            }

            if (watermarkData.showText && not watermarkData.text.isEmpty())
            {
                VPTileFactory::PaintWatermarkText(painter, img, watermarkData);
            }
        }
    };

    auto PaintTileNumber = [painter, layout, nbCol, font]
            (const QRectF &img, int i, int j)
    {
        if (layout->LayoutSettings().GetShowTileNumber())
        {
            painter->save();

            painter->setFont(font);

            QPen pen = painter->pen();
            pen.setColor(Qt::black);
            painter->setPen(pen);

            painter->drawText(img, Qt::AlignCenter, QString::number(j*nbCol + i+1));

            painter->restore();
        }
    };

    for(int j=0;j<=nbRow;++j)
    {
        // horizontal lines
        painter->drawLine(QPointF(sheetMargins.left(), sheetMargins.top()+j*height),
                          QPointF(sheetMargins.left()+nbCol*width, sheetMargins.top()+j*height));

        for(int i=0;i<=nbCol;++i)
        {
            // vertical lines
            painter->drawLine(QPointF(sheetMargins.left()+i*width, sheetMargins.top()),
                              QPointF(sheetMargins.left()+i*width, sheetMargins.top() + nbRow*height));

            if (j < nbRow && i < nbCol)
            {
                QRectF img(sheetMargins.left()+i*width, sheetMargins.top()+j*height, width, height);

                PaintWatermark(img);
                PaintTileNumber(img, i, j);
            }
        }
    }
}
