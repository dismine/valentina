#include "vptilefactory.h"

#include <QtSvg>

#include "../vwidgets/vmaingraphicsscene.h"
#include "layout/vpsheet.h"
#include "scene/vpmaingraphicsview.h"
#include "../vmisc/def.h"
#include "../vmisc/vcommonsettings.h"
#include "../vlayout/vprintlayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPTileFactory::VPTileFactory(const VPLayoutPtr &layout, VCommonSettings *commonSettings):
    m_layout(layout),
    m_commonSettings(commonSettings)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::refreshTileInfos()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if(not layout.isNull())
    {
        QSizeF tilesSize = layout->LayoutSettings().GetTilesSize();
        QMarginsF tilesMargins = layout->LayoutSettings().GetTilesMargins();

        // sets the drawing height
        m_drawingAreaHeight = tilesSize.height();

        if (not layout->LayoutSettings().IgnoreTilesMargins())
        {
            m_drawingAreaHeight -= tilesMargins.top() + tilesMargins.bottom();
        }

        // sets the drawing width
        m_drawingAreaWidth = tilesSize.width();

        if (not layout->LayoutSettings().IgnoreTilesMargins())
        {
            m_drawingAreaWidth -= tilesMargins.left() + tilesMargins.right();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::drawTile(QPainter *painter, QPrinter *printer, const VPSheetPtr &sheet, int row, int col)
{
    SCASSERT(painter != nullptr)
    SCASSERT(printer != nullptr)

    VPLayoutPtr layout = m_layout.toStrongRef();
    if(layout.isNull())
    {
        return;
    }

    if (sheet.isNull())
    {
        return;
    }

    QMarginsF sheetMargins;
    if (not sheet->IgnoreMargins())
    {
        sheetMargins = sheet->GetSheetMargins();
    }

    const int nbCol = ColNb(sheet);
    const int nbRow = RowNb(sheet);

    if (row < 0 || row > nbRow || col < 0 || col > nbCol)
    {
        return;
    }

    QMarginsF tilesMargins = layout->LayoutSettings().GetTilesMargins();
    QPen penTileInfos = QPen(QColor(180,180,180), m_commonSettings->WidthHairLine(), Qt::DashLine, Qt::RoundCap,
                             Qt::RoundJoin);
    QPen penTileDrawing = QPen(Qt::black, m_commonSettings->WidthMainLine(), Qt::SolidLine, Qt::RoundCap,
                               Qt::RoundJoin);

    painter->setPen(penTileDrawing);

    // paint the content of the page
    qreal xScale = layout->LayoutSettings().HorizontalScale();
    qreal yScale = layout->LayoutSettings().VerticalScale();
    QRectF source = QRectF(sheetMargins.left() + col*(m_drawingAreaWidth - tileStripeWidth) / xScale,
                           sheetMargins.top() + row*(m_drawingAreaHeight - tileStripeWidth) / yScale,
                           m_drawingAreaWidth / xScale,
                           m_drawingAreaHeight / yScale);

    QRectF target = QRectF(0,
                           0,
                           m_drawingAreaWidth,
                           m_drawingAreaHeight);
    sheet->SceneData()->Scene()->render(painter, VPrintLayout::SceneTargetRect(printer, target), source,
                                        Qt::IgnoreAspectRatio);

    QScopedPointer<QSvgRenderer> svgRenderer(new QSvgRenderer());

    // ------------- prepare triangles for position marks
    QRectF rectBasic = QRectF(-UnitConvertor(0.5, Unit::Cm, Unit::Px),
                         0,
                         UnitConvertor(1, Unit::Cm, Unit::Px),
                         UnitConvertor(0.5, Unit::Cm, Unit::Px)
                         );
    QPainterPath triangleBasic;
    triangleBasic.moveTo(rectBasic.topLeft());
    triangleBasic.lineTo(rectBasic.topRight());
    triangleBasic.lineTo(rectBasic.left() + (rectBasic.width() / 2), rectBasic.bottom());
    triangleBasic.lineTo(rectBasic.topLeft());

    QBrush triangleBush = QBrush(QColor(200,200,200));

    // add the tiles decorations (cutting and gluing lines, scissors, infos etc.)
    painter->setPen(penTileInfos);

    if(row > 0)
    {
        // add top triangle
        QPainterPath triangleTop =
                QTransform()
                .translate(m_drawingAreaWidth/2, 0)
                .map(triangleBasic);
        painter->fillPath(triangleTop, triangleBush);

        //  scissors along the top line
        svgRenderer->load(QStringLiteral("://puzzleicon/svg/icon_scissors_horizontal.svg"));
        svgRenderer->render(painter, QRectF(m_drawingAreaWidth - tileStripeWidth, 0,
                                            UnitConvertor(0.95, Unit::Cm, Unit::Px),
                                            UnitConvertor(0.56, Unit::Cm, Unit::Px)));

        // dashed top line (for cutting)
        penTileInfos.setStyle(Qt::DashLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth, 0));
    }
    else
    {
        // solid top line stopping at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);

        if(col < nbCol - 1)
        {
            painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth, 0));
        }
        else
        {
            painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth - tileStripeWidth, 0));
        }
    }

    if(col > 0)
    {
        // add left triangle
        QPainterPath triangleLeft =
                QTransform()
                .translate(0, m_drawingAreaHeight/2)
                .rotate(-90)
                .map(triangleBasic);
        painter->fillPath(triangleLeft, triangleBush);

        //  scissors along the left line
        svgRenderer->load(QStringLiteral("://puzzleicon/svg/icon_scissors_vertical.svg"));
        svgRenderer->render(painter, QRectF(0, m_drawingAreaHeight - tileStripeWidth,
                                            UnitConvertor(0.56, Unit::Cm, Unit::Px),
                                            UnitConvertor(0.95, Unit::Cm, Unit::Px)));

        // dashed left line (for cutting)
        penTileInfos.setStyle(Qt::DashLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight));
    }
    else
    {
        // solid left line at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);

        if(row < nbRow - 1)
        {
            painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight));
        }
        else
        {
            painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight - tileStripeWidth));
        }
    }

    if(row < nbRow-1)
    {
        // add bottom triangle
        QPainterPath triangleBottom =
                QTransform()
                .translate(m_drawingAreaWidth/2, m_drawingAreaHeight - tileStripeWidth)
                .rotate(180)
                .map(triangleBasic);

        painter->fillPath(triangleBottom, triangleBush);

        // dotted bottom line (for glueing)
        penTileInfos.setStyle(Qt::DotLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                          QPointF(m_drawingAreaWidth, m_drawingAreaHeight - tileStripeWidth));
    }
    else
    {
        // solid bottom line at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);

        if(col < nbCol - 1)
        {
            painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                              QPointF(m_drawingAreaWidth, m_drawingAreaHeight - tileStripeWidth));
        }
        else
        {
            painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                              QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight - tileStripeWidth));;
        }
    }

    if(col < nbCol-1)
    {
        // add right triangle
        QPainterPath triangleRight =
                QTransform()
                .translate(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight/2)
                .rotate(90)
                .map(triangleBasic);
        painter->fillPath(triangleRight, triangleBush);

        // dotted right line (for glueing)
        penTileInfos.setStyle(Qt::DotLine);
        painter->setPen(penTileInfos);
        painter->drawLine(QPointF(m_drawingAreaWidth - tileStripeWidth, 0),
                          QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight));
    }
    else
    {
        // solid right line at the edge
        penTileInfos.setStyle(Qt::SolidLine);
        painter->setPen(penTileInfos);

        if(row < nbRow - 1)
        {
            painter->drawLine(QPointF(m_drawingAreaWidth - tileStripeWidth, 0),
                              QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight));
        }
        else
        {
            painter->drawLine(QPointF(m_drawingAreaWidth - tileStripeWidth, 0),
                              QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight - tileStripeWidth));
        }
    }

    // prepare the painting for the text information
    QTextDocument td;
    td.setPageSize(QSizeF(m_drawingAreaWidth - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaHeight));

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
    painter->translate(QPointF(UnitConvertor(1, Unit::Cm, Unit::Px), m_drawingAreaHeight - tileStripeWidth));
    td.drawContents(painter);
    painter->restore();

    // paint the page information
    const QString page = tr("Page %1 of %2").arg(row*nbCol+col+1).arg(nbCol*nbRow);

    td.setPageSize(QSizeF(m_drawingAreaHeight - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaWidth));

    QFontMetrics metrix = QFontMetrics(td.defaultFont());
    QString clippedSheetName = metrix.elidedText(sheet->GetName(), Qt::ElideMiddle,
                                                 metrix.width(QString().fill('z', 50)));

    td.setHtml(QString("<table width='100%' style='color:rgb(180,180,180);'>"
                       "<tr>"
                       "<td align='center'>%1 - %2</td>"
                       "</tr>"
                       "</table>")
               .arg(page).arg(clippedSheetName));
    painter->save();
    painter->rotate(-90);
    painter->translate(QPointF(-(m_drawingAreaHeight) + UnitConvertor(1, Unit::Cm, Unit::Px),
                               m_drawingAreaWidth - tileStripeWidth));
    td.drawContents(painter);
    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::RowNb(const VPSheetPtr &sheet) const -> int
{
    if (sheet.isNull())
    {
        return 0;
    }

    qreal yScale = 1;
    VPLayoutPtr layout = m_layout.toStrongRef();
    if(not layout.isNull())
    {
        yScale = layout->LayoutSettings().VerticalScale();
    }

    QSizeF sheetSize = sheet->GetSheetSize();
    return qCeil(sheetSize.height() * yScale / (m_drawingAreaHeight - tileStripeWidth));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::ColNb(const VPSheetPtr &sheet) const -> int
{
    if (sheet.isNull())
    {
        return 0;
    }

    qreal xScale = 1;
    VPLayoutPtr layout = m_layout.toStrongRef();
    if(not layout.isNull())
    {
        xScale = layout->LayoutSettings().HorizontalScale();
    }

    QSizeF sheetSize = sheet->GetSheetSize();
    return qCeil(sheetSize.width() * xScale / (m_drawingAreaWidth - tileStripeWidth));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::DrawingAreaHeight() const -> qreal
{
    return m_drawingAreaHeight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::DrawingAreaWidth() const -> qreal
{
    return m_drawingAreaWidth;
}
