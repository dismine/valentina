#include "vpgraphicstilegrid.h"

#include "../vptilefactory.h"
#include "../layout/vplayout.h"

namespace
{
constexpr qreal penWidth = 1;
}

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

        qreal xScale = layout->LayoutSettings().HorizontalScale();
        qreal yScale = layout->LayoutSettings().VerticalScale();

        QRectF rect(0, 0,
                    layout->TileFactory()->ColNb(sheet) * (layout->TileFactory()->DrawingAreaWidth() / xScale),
                    layout->TileFactory()->RowNb(sheet) * (layout->TileFactory()->DrawingAreaHeight() / yScale));

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

        QPen pen(QColor(255,0,0,127), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setCosmetic(true);
        pen.setStyle(Qt::DashLine);
        QBrush noBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->setBrush(noBrush);

        qreal xScale = layout->LayoutSettings().HorizontalScale();
        qreal yScale = layout->LayoutSettings().VerticalScale();

        const qreal drawingAreaWidth = layout->TileFactory()->DrawingAreaWidth() / xScale;
        const qreal drawingAreaHeight = layout->TileFactory()->DrawingAreaHeight() / yScale;

        for(int i=0;i<=layout->TileFactory()->ColNb(sheet);i++)
        {
           painter->drawLine(QPointF(i*drawingAreaWidth, 0),
                             QPointF(i*drawingAreaWidth, layout->TileFactory()->RowNb(sheet)*drawingAreaHeight));
        }

        for(int j=0;j<=layout->TileFactory()->RowNb(sheet);j++)
        {
            painter->drawLine(QPointF(0, j*drawingAreaHeight),
                              QPointF(layout->TileFactory()->ColNb(sheet)*drawingAreaWidth, j*drawingAreaHeight));
        }
    }
}
