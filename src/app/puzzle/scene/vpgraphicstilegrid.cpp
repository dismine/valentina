#include "vpgraphicstilegrid.h"

#include "../vptilefactory.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"

#include <QFileInfo>
#include <QImageReader>
#include <QPixmapCache>
#include <QSvgRenderer>

namespace
{
constexpr qreal penWidth = 1;
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
    if(not layout.isNull() && layout->LayoutSettings().GetShowTiles())
    {
        VPSheetPtr sheet = layout->GetSheet(m_sheetUuid);

        QMarginsF sheetMargins;
        if (not sheet.isNull() && not sheet->IgnoreMargins())
        {
            sheetMargins = sheet->GetSheetMargins();
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

        const int nbCol = layout->TileFactory()->ColNb(sheet);
        const int nbRow = layout->TileFactory()->RowNb(sheet);

        VWatermarkData watermarkData = layout->TileFactory()->WatermarkData();

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
                    QRectF img(sheetMargins.left()+i*width, sheetMargins.top()+j*height,
                               width, height);

                    if (not layout->LayoutSettings().WatermarkPath().isEmpty() &&
                            layout->LayoutSettings().GetShowWatermark())
                    {
                        if (watermarkData.opacity > 0)
                        {
                            if (watermarkData.showImage && not watermarkData.path.isEmpty())
                            {
                                VPTileFactory::PaintWatermarkImage(painter, img, watermarkData,
                                                                   layout->LayoutSettings().WatermarkPath());
                            }

                            if (watermarkData.showText && not watermarkData.text.isEmpty())
                            {
                                VPTileFactory::PaintWatermarkText(painter, img, watermarkData);
                            }
                        }
                    }
                }
            }
        }
    }
}
