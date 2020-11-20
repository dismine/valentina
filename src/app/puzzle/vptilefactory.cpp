#include "vptilefactory.h"

#include <QtSvg>

#include "../vwidgets/vmaingraphicsscene.h"
#include "vpsheet.h"
#include "vpmaingraphicsview.h"


//---------------------------------------------------------------------------------------------------------------------
VPTileFactory::VPTileFactory(VPLayout *layout, VCommonSettings *commonSettings):
    m_layout(layout),
    m_commonSettings(commonSettings)
{
    m_infoStripeWidth = UnitConvertor(1, Unit::Cm, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
VPTileFactory::~VPTileFactory()
{

}


//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::refreshTileInfos()
{
    if(m_layout != nullptr)
    {
        PageOrientation tilesOrientation = m_layout->GetTilesOrientation();
        QSizeF tilesSize =  m_layout->GetTilesSize();
        QMarginsF tilesMargins = m_layout->GetTilesMargins();

        // sets the drawing height
        m_drawingAreaHeight = (tilesOrientation == PageOrientation::Portrait)?
                        tilesSize.height() : tilesSize.width();
        m_drawingAreaHeight -=
                tilesMargins.top() + tilesMargins.bottom() + m_infoStripeWidth;

        // sets the drawing width
        m_drawingAreaWidth = (tilesOrientation == PageOrientation::Portrait)?
                    tilesSize.width() : tilesSize.height();
        m_drawingAreaWidth -=
                tilesMargins.left() + tilesMargins.right() + m_infoStripeWidth;


        QSizeF sheetSize = m_layout->GetFocusedSheet()->GetSheetSize();
        qreal totalDrawingWidth = 0;
        qreal totaldrawingHeight = 0;

        if(m_layout->GetFocusedSheet()->GetOrientation() == PageOrientation::Portrait)
        {
             totalDrawingWidth = sheetSize.width();
             totaldrawingHeight = sheetSize.height();
        }
        else
        {
            totalDrawingWidth = sheetSize.height();
            totaldrawingHeight = sheetSize.width();
        }

        m_nbCol = qCeil(totalDrawingWidth/m_drawingAreaWidth);
        m_nbRow = qCeil(totaldrawingHeight/m_drawingAreaHeight);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::drawTile(QPainter *painter, VPMainGraphicsView *graphicsView, int row, int col)
{
    QMarginsF tilesMargins = m_layout->GetTilesMargins();
    QPen penTileInfos = QPen(QColor(180,180,180), m_commonSettings->WidthHairLine(), Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    QPen penTileDrawing = QPen(Qt::black, m_commonSettings->WidthMainLine(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QSvgRenderer* svgRenderer = new QSvgRenderer();

    // FIXME here instead of creating 4 triangle, maybe create one and rotate it

    // ------------- prepare triangles for positioning

    // top triangle
    QRectF rectTop = QRectF(tilesMargins.left()+ m_drawingAreaWidth/2 - UnitConvertor(0.5, Unit::Cm, Unit::Px),
                         tilesMargins.top(),
                         UnitConvertor(1, Unit::Cm, Unit::Px),
                         UnitConvertor(0.5, Unit::Cm, Unit::Px)
                         );
    QPainterPath triangleTop;
    triangleTop.moveTo(rectTop.topLeft());
    triangleTop.lineTo(rectTop.topRight());
    triangleTop.lineTo(rectTop.left() + (rectTop.width() / 2), rectTop.bottom());
    triangleTop.lineTo(rectTop.topLeft());

    // left triangle
    QRectF rectLeft = QRectF(tilesMargins.left(),
                         tilesMargins.top() + m_drawingAreaHeight/2 - UnitConvertor(0.5, Unit::Cm, Unit::Px),
                         UnitConvertor(0.5, Unit::Cm, Unit::Px),
                         UnitConvertor(1, Unit::Cm, Unit::Px)
                         );
    QPainterPath triangleLeft;
    triangleLeft.moveTo(rectLeft.topLeft());
    triangleLeft.lineTo(rectLeft.right(), rectLeft.top() + (rectLeft.height() / 2));
    triangleLeft.lineTo(rectLeft.bottomLeft());
    triangleLeft.lineTo(rectLeft.topLeft());

    // bottom triangle
    QRectF rectBottom = QRectF(tilesMargins.left()+ m_drawingAreaWidth/2 - UnitConvertor(0.5, Unit::Cm, Unit::Px),
                         tilesMargins.top()+m_drawingAreaHeight - UnitConvertor(0.5, Unit::Cm, Unit::Px),
                         UnitConvertor(1, Unit::Cm, Unit::Px),
                         UnitConvertor(0.5, Unit::Cm, Unit::Px)
                         );
    QPainterPath triangleBottom;
    triangleBottom.moveTo(rectBottom.bottomLeft());
    triangleBottom.lineTo(rectBottom.left() + (rectBottom.width() / 2), rectBottom.top());
    triangleBottom.lineTo(rectBottom.bottomRight());
    triangleBottom.lineTo(rectBottom.bottomLeft());

    // right triangle
    QRectF rectRight = QRectF(tilesMargins.left() + m_drawingAreaWidth - UnitConvertor(0.5, Unit::Cm, Unit::Px),
                              tilesMargins.top() + m_drawingAreaHeight/2 - UnitConvertor(0.5, Unit::Cm, Unit::Px),
                              UnitConvertor(0.5, Unit::Cm, Unit::Px),
                              UnitConvertor(1, Unit::Cm, Unit::Px)
                              );
    QPainterPath triangleRight;
    triangleRight.moveTo(rectRight.topRight());
    triangleRight.lineTo(rectRight.bottomRight());
    triangleRight.lineTo(rectRight.left(), rectRight.top() + (rectRight.height() / 2));
    triangleRight.lineTo(rectRight.topRight());

    QBrush triangleBush = QBrush(QColor(200,200,200));

    // add the tiles decorations (cutting and gluing lines, scissors, infos etc.)
    painter->setPen(penTileInfos);

    if(row > 0)
    {
        // add top triangle
        painter->fillPath(triangleTop, triangleBush);

        //  scissors along the top line
        svgRenderer->load(QStringLiteral("://puzzleicon/svg/icon_scissors_horizontal.svg"));
        svgRenderer->render(painter, QRectF(tilesMargins.left() + m_drawingAreaWidth,
                                             tilesMargins.top(),
                                             UnitConvertor(1, Unit::Cm, Unit::Px),
                                             UnitConvertor(0.56, Unit::Cm, Unit::Px)
                                             ));

        // dashed top line (for cutting)
        penTileInfos.setStyle(Qt::DashLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left(),
                                 tilesMargins.top()),
                        QPointF(tilesMargins.left() + m_drawingAreaWidth + m_infoStripeWidth,
                                tilesMargins.top())
                         );
    }
    else
    {
        // solid top line stopping at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left(),
                                 tilesMargins.top()),
                        QPointF(tilesMargins.left() + m_drawingAreaWidth + ((col < m_nbCol-1)? m_infoStripeWidth : 0),
                                tilesMargins.top())
                         );
    }

    if(col > 0)
    {
        // add left triangle
        painter->fillPath(triangleLeft, triangleBush);

        //  scissors along the left line
        svgRenderer->load(QStringLiteral("://puzzleicon/svg/icon_scissors_vertical.svg"));
        svgRenderer->render(painter, QRectF(tilesMargins.left(),
                                             tilesMargins.top()+m_drawingAreaHeight,
                                             UnitConvertor(0.56, Unit::Cm, Unit::Px),
                                             UnitConvertor(1, Unit::Cm, Unit::Px)
                                             ));

        // dashed left line (for cutting)
        penTileInfos.setStyle(Qt::DashLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left(),
                                 tilesMargins.top()),
                        QPointF(tilesMargins.left(),
                                tilesMargins.top() + m_drawingAreaHeight + m_infoStripeWidth)
                         );
    }
    else
    {
        // solid left line at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left(),
                                 tilesMargins.top()),
                        QPointF(tilesMargins.left(),
                                tilesMargins.top() + m_drawingAreaHeight + ((row < m_nbRow-1)? m_infoStripeWidth : 0))
                         );
    }

    if(row < m_nbRow-1)
    {
        // add bottom triangle
        painter->fillPath(triangleBottom, triangleBush);

        // dotted bottom line (for glueing)
        penTileInfos.setStyle(Qt::DotLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left(),
                                 tilesMargins.top() + m_drawingAreaHeight),
                        QPointF(tilesMargins.left() + m_drawingAreaWidth + m_infoStripeWidth,
                                tilesMargins.top() + m_drawingAreaHeight)
                         );
    } else
    {
        // solid bottom line at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left(),
                                 tilesMargins.top() + m_drawingAreaHeight),
                        QPointF(tilesMargins.left() + m_drawingAreaWidth + ((col < m_nbCol-1)? m_infoStripeWidth : 0),
                                tilesMargins.top() + m_drawingAreaHeight)
                         );
    }

    if(col < m_nbCol-1)
    {
        // add right triangle
        painter->fillPath(triangleRight, triangleBush);

        // dotted right line (for glueing)
        penTileInfos.setStyle(Qt::DotLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left() + m_drawingAreaWidth,
                                 tilesMargins.top()),
                        QPointF(tilesMargins.left() + m_drawingAreaWidth,
                                tilesMargins.top()+ m_drawingAreaHeight + m_infoStripeWidth)
                         );
    }
    else
    {
        // solid right line at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(tilesMargins.left() + m_drawingAreaWidth,
                                 tilesMargins.top()),
                        QPointF(tilesMargins.left() + m_drawingAreaWidth,
                                tilesMargins.top()+ m_drawingAreaHeight + ((row < m_nbRow-1) ? m_infoStripeWidth : 0))
                         );
    }

    // paint the content of the page
    QRectF source = QRectF(col*m_drawingAreaWidth,
                           row*m_drawingAreaHeight,
                           m_drawingAreaWidth + m_infoStripeWidth,
                           m_drawingAreaHeight + m_infoStripeWidth
                           );
    QRectF target = QRectF(tilesMargins.left(),
                           tilesMargins.top(),
                           source.width(),
                           source.height()
                           );

    painter->setPen(penTileDrawing);
    graphicsView->GetScene()->render(painter, target, source, Qt::IgnoreAspectRatio);

    // prepare the painting for the text information
    QTextDocument td;
    td.setPageSize(QSizeF(
                       m_drawingAreaWidth - UnitConvertor(2, Unit::Cm, Unit::Px),
                       m_drawingAreaHeight
                       ));

    // paint the grid information
    const QString grid = tr("Grid ( %1 , %2 )").arg(row+1).arg(col+1);

    td.setHtml(QString("<table width='100%' style='color:rgb(180,180,180);'>"
                       "<tr>"
                       "<td align='center'>%1</td>"
                       "</tr>"
                       "</table>")
               .arg(grid));
    painter->setPen(penTileInfos);
    painter->save();
    painter->translate(QPointF(tilesMargins.left()+ UnitConvertor(1, Unit::Cm, Unit::Px),
                              m_drawingAreaHeight + tilesMargins.top()
                              ));
    td.drawContents(painter);
    painter->restore();

    // paint the page information
    const QString page = tr("Page %1 of %2").arg(row*m_nbCol+col+1).arg(m_nbCol*m_nbRow);

    td.setPageSize(QSizeF(m_drawingAreaHeight - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaWidth));
    td.setHtml(QString("<table width='100%' style='color:rgb(180,180,180);'>"
                       "<tr>"
                       "<td align='center'>%1 - %2</td>"
                       "</tr>"
                       "</table>")
               .arg(page).arg(m_layout->GetFocusedSheet()->GetName()));
    painter->save();
    painter->rotate(-90);
    painter->translate(QPointF(-(m_drawingAreaHeight+tilesMargins.top()) + UnitConvertor(1, Unit::Cm, Unit::Px),
                             m_drawingAreaWidth + tilesMargins.left()
                             ));
    td.drawContents(painter);
    painter->restore();

}

//---------------------------------------------------------------------------------------------------------------------
int VPTileFactory::getRowNb()
{
    return m_nbRow;
}

//---------------------------------------------------------------------------------------------------------------------
int VPTileFactory::getColNb()
{
    return m_nbCol;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPTileFactory::getDrawingAreaHeight()
{
    return m_drawingAreaHeight;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPTileFactory::getDrawingAreaWidth()
{
    return m_drawingAreaWidth;
}
