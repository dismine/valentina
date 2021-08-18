#include "vpgraphicstilegrid.h"

#include "../vptilefactory.h"
#include "../layout/vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsTileGrid::VPGraphicsTileGrid(const VPLayoutPtr &layout, VPTileFactory *tileFactory, QGraphicsItem *parent):
    QGraphicsItem(parent),
    m_tileFactory(tileFactory),
    m_layout(layout)
{

}

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsTileGrid::~VPGraphicsTileGrid()
{

}

//---------------------------------------------------------------------------------------------------------------------
QRectF VPGraphicsTileGrid::boundingRect() const
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if(not layout.isNull() && layout->LayoutSettings().GetShowTiles())
    {
        return QRectF(0,
                   0,
                   m_tileFactory->getColNb()* m_tileFactory->getDrawingAreaWidth(),
                   m_tileFactory->getRowNb()* m_tileFactory->getDrawingAreaHeight()
                   );
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
        QPen pen(QColor(255,0,0,127), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setCosmetic(true);
        pen.setStyle(Qt::DashLine);
        QBrush noBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->setBrush(noBrush);

        for(int i=0;i<=m_tileFactory->getColNb();i++)
        {
           painter->drawLine(QPointF(
                                 i*m_tileFactory->getDrawingAreaWidth(),
                                 0),
                             QPointF(
                                 i*m_tileFactory->getDrawingAreaWidth(),
                                 m_tileFactory->getRowNb()*m_tileFactory->getDrawingAreaHeight()
                                 )
                             );
        }

        for(int j=0;j<=m_tileFactory->getRowNb();j++)
        {
            painter->drawLine(QPointF(
                                  0,
                                  j*m_tileFactory->getDrawingAreaHeight()
                                  ),
                              QPointF(
                                  m_tileFactory->getColNb()*m_tileFactory->getDrawingAreaWidth(),
                                  j*m_tileFactory->getDrawingAreaHeight()
                                  )
                              );
        }
    }
}
