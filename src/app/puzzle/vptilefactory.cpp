#include "vptilefactory.h"

#include <QtSvg>

#include "../vlayout/vprintlayout.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/literals.h"
#include "../vmisc/vcommonsettings.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "layout/vplayout.h"
#include "layout/vpsheet.h"
#include "theme/vtheme.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(QColor, tileColor, (180, 180, 180))            // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(QBrush, triangleBush, (QColor(200, 200, 200))) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto Grayscale(QImage image) -> QImage
{
    for (int ii = 0; ii < image.height(); ii++)
    {
        uchar *scan = image.scanLine(ii);
        int depth = 4;
        for (int jj = 0; jj < image.width(); jj++)
        {
            auto *rgbpixel = reinterpret_cast<QRgb *>(scan + jj * depth); // NOLINT
            int gray = qGray(*rgbpixel);
            *rgbpixel = QColor(gray, gray, gray, qAlpha(*rgbpixel)).rgba();
        }
    }

    return image;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkImageFromCache(const VWatermarkData &watermarkData, const QString &watermarkPath, qreal xScale,
                             qreal yScale, QString &error) -> QPixmap
{
    QPixmap pixmap;
    QString imagePath = AbsoluteMPath(watermarkPath, watermarkData.path);
    QString imageCacheKey =
        QStringLiteral("puzzle=path%1+rotation%3+grayscale%4+xscale%5+yxcale%6")
            .arg(imagePath, QString::number(watermarkData.imageRotation), watermarkData.grayscale ? trueStr : falseStr)
            .arg(xScale)
            .arg(yScale);

    if (not QPixmapCache::find(imageCacheKey, &pixmap))
    {
        QImageReader imageReader(imagePath);
        QImage watermark = imageReader.read();
        if (watermark.isNull())
        {
            error = imageReader.errorString();
            return pixmap;
        }

        if (watermarkData.grayscale)
        {
            watermark = Grayscale(watermark);
        }

        QTransform t;
        t.scale(1 / xScale, 1 / yScale);
        watermark = watermark.transformed(t);

        t = QTransform();
        t.translate(watermark.width() / 2., watermark.height() / 2.);
        t.rotate(-watermarkData.imageRotation);
        t.translate(-watermark.width() / 2., -watermark.height() / 2.);
        watermark = watermark.transformed(t);

        pixmap = QPixmap::fromImage(watermark);

        QPixmapCache::insert(imageCacheKey, pixmap);
    }
    return pixmap;
}

//---------------------------------------------------------------------------------------------------------------------
// ------------- prepare triangles for position marks
auto TriangleBasic() -> QPainterPath
{
    // ------------- prepare triangles for position marks
    QRectF rectBasic = QRectF(-UnitConvertor(0.5, Unit::Cm, Unit::Px), 0, UnitConvertor(1, Unit::Cm, Unit::Px),
                              UnitConvertor(0.5, Unit::Cm, Unit::Px));
    QPainterPath triangleBasic;
    triangleBasic.moveTo(rectBasic.topLeft());
    triangleBasic.lineTo(rectBasic.topRight());
    triangleBasic.lineTo(rectBasic.left() + (rectBasic.width() / 2.), rectBasic.bottom());
    triangleBasic.lineTo(rectBasic.topLeft());
    return triangleBasic;
}
} // namespace

// See https://stackoverflow.com/a/46719572/3045403
#if __cplusplus < 201703L                       // C++17
constexpr qreal VPTileFactory::tileStripeWidth; // NOLINT(readability-redundant-declaration)
#endif

//---------------------------------------------------------------------------------------------------------------------
VPTileFactory::VPTileFactory(const VPLayoutPtr &layout, VCommonSettings *commonSettings, QObject *parent)
  : QObject(parent),
    m_layout(layout),
    m_commonSettings(commonSettings)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::RefreshTileInfos()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
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
void VPTileFactory::RefreshWatermarkData()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        m_watermarkData = layout->WatermarkData();
    }
    else
    {
        m_watermarkData = VWatermarkData();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::drawTile(QPainter *painter, QPrinter *printer, const VPSheetPtr &sheet, int row, int col)
{
    SCASSERT(painter != nullptr)
    SCASSERT(printer != nullptr)

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    if (sheet.isNull())
    {
        return;
    }

    const int nbCol = ColNb(sheet);
    const int nbRow = RowNb(sheet);

    if (row < 0 || row > nbRow || col < 0 || col > nbCol)
    {
        return;
    }

    DrawTilePageContent(painter, sheet, row, col, printer);

    // add the tiles decorations (cutting and gluing lines, scissors, infos etc.)
    painter->setPen(PenTileInfos());

    painter->save();

    const QPair<qreal, qreal> scale = VPrintLayout::PrinterScaleDiff(printer);
    painter->scale(scale.first, scale.second);

    if (row > 0)
    {
        // add top triangle
        DrawTopTriangle(painter);
        // scissors along the top line
        DrawTopLineScissors(painter);
        // dashed top line (for cutting)
        DrawTopCuttingLine(painter);
    }
    else
    {
        // solid top line stopping at the edge
        DrawSolidTopLine(painter, col, nbCol);
    }

    if (col > 0)
    {
        // add left triangle
        DrawLeftTriangle(painter);
        // scissors along the left line
        DrawLeftLineScissors(painter);
        // dashed left line (for cutting)
        DrawLeftCuttingLine(painter);
    }
    else
    {
        // solid left line at the edge
        DrawSolidLeftLine(painter, row, nbRow);
    }

    if (row < nbRow - 1)
    {
        // add bottom triangle
        DrawBottomTriangle(painter);
        // dotted bottom line (for glueing)
        DrawBottomCuttingLine(painter);
    }
    else
    {
        // solid bottom line at the edge
        DrawSolidBottomLine(painter, col, nbCol);
    }

    DrawRuler(painter, scale.first);
    DrawWatermark(painter);

    if (col < nbCol - 1)
    {
        // add right triangle
        DrawRightTriangle(painter);
        // dotted right line (for glueing)
        DrawRightCuttingLine(painter);
    }
    else
    {
        // solid right line at the edge
        DrawSolidRightLine(painter, row, nbRow);
    }

    // prepare the painting for the text information
    DrawTextInformation(painter, row, col, nbRow, nbCol, sheet->GetName());

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
    if (not layout.isNull())
    {
        yScale = layout->LayoutSettings().VerticalScale();
    }

    QRectF sheetSize = sheet->GetMarginsRect();
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
    if (not layout.isNull())
    {
        xScale = layout->LayoutSettings().HorizontalScale();
    }

    QRectF sheetSize = sheet->GetMarginsRect();
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

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::WatermarkData() const -> const VWatermarkData &
{
    return m_watermarkData;
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRuler(QPainter *painter, qreal scale) const
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QPen rulePen(*tileColor, 1, Qt::SolidLine);

    painter->save();
    painter->setPen(rulePen);

    const qreal notchHeight = UnitConvertor(3, Unit::Mm, Unit::Px);
    const qreal shortNotchHeight = UnitConvertor(1.1, Unit::Mm, Unit::Px);

    Unit layoutUnits = layout->LayoutSettings().GetUnit();
    Unit rulerUnits = layoutUnits == Unit::Inch ? layoutUnits : Unit::Cm;

    const qreal step = UnitConvertor(1, rulerUnits, Unit::Px);
    double marksCount = (m_drawingAreaWidth - tileStripeWidth) / step;
    int i = 0;
    while (i < marksCount)
    {
        if (i != 0)
        { // don't need 0 notch
            // middle ruler line
            painter->drawLine(QPointF(step * i - step / 2., m_drawingAreaHeight - tileStripeWidth),
                              QPointF(step * i - step / 2., m_drawingAreaHeight - tileStripeWidth + shortNotchHeight));

            // ruler line
            painter->drawLine(QPointF(step * i, m_drawingAreaHeight - tileStripeWidth),
                              QPointF(step * i, m_drawingAreaHeight - tileStripeWidth + notchHeight));
        }
        else
        {
            painter->save();

            QFont fnt = painter->font();
            const int size = qRound(10 / scale);
            size > 0 ? fnt.setPointSize(size) : fnt.setPointSize(10);
            painter->setFont(fnt);

            qreal unitsWidth = 0;
            QFontMetrics fm(fnt);
            QString units = rulerUnits != Unit::Inch ? tr("cm", "unit") : tr("in", "unit");
            unitsWidth = TextWidth(fm, units);
            painter->drawText(QPointF(step * 0.5 - unitsWidth * 0.6,
                                      m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight),
                              units);

            painter->restore();
        }
        ++i;
    }

    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawWatermark(QPainter *painter) const
{
    SCASSERT(painter != nullptr)

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    if (m_watermarkData.opacity > 0)
    {
        QRectF img(0, 0, m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight - tileStripeWidth);

        if (m_watermarkData.showImage && not m_watermarkData.path.isEmpty())
        {
            PaintWatermarkImage(painter, img, m_watermarkData, layout->LayoutSettings().WatermarkPath(), false);
        }

        if (m_watermarkData.showText && not m_watermarkData.text.isEmpty())
        {
            PaintWatermarkText(painter, img, m_watermarkData);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPTileFactory::PenTileInfos() const -> QPen
{
    return {*tileColor, m_commonSettings->WidthHairLine(), Qt::DashLine, Qt::RoundCap, Qt::RoundJoin};
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTilePageContent(QPainter *painter, const VPSheetPtr &sheet, int row, int col,
                                        QPrinter *printer) const
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QMarginsF sheetMargins;
    if (not sheet->IgnoreMargins())
    {
        sheetMargins = sheet->GetSheetMargins();
    }

    QPen penTileDrawing =
        QPen(Qt::black, m_commonSettings->WidthMainLine(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(penTileDrawing);

    // paint the content of the page
    const qreal xScale = layout->LayoutSettings().HorizontalScale();
    const qreal yScale = layout->LayoutSettings().VerticalScale();
    QRectF source = QRectF(sheetMargins.left() + col * (m_drawingAreaWidth - tileStripeWidth) / xScale,
                           sheetMargins.top() + row * (m_drawingAreaHeight - tileStripeWidth) / yScale,
                           m_drawingAreaWidth / xScale, m_drawingAreaHeight / yScale);

    QRectF target = QRectF(0, 0, m_drawingAreaWidth, m_drawingAreaHeight);
    sheet->SceneData()->Scene()->render(painter, VPrintLayout::SceneTargetRect(printer, target), source,
                                        Qt::IgnoreAspectRatio);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTopTriangle(QPainter *painter) const
{
    QPainterPath triangleTop = QTransform().translate(m_drawingAreaWidth / 2, 0).map(TriangleBasic());
    painter->fillPath(triangleTop, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawLeftTriangle(QPainter *painter) const
{
    QPainterPath triangleLeft = QTransform().translate(0, m_drawingAreaHeight / 2).rotate(-90).map(TriangleBasic());
    painter->fillPath(triangleLeft, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawBottomTriangle(QPainter *painter) const
{
    QPainterPath triangleBottom = QTransform()
                                      .translate(m_drawingAreaWidth / 2, m_drawingAreaHeight - tileStripeWidth)
                                      .rotate(180)
                                      .map(TriangleBasic());
    painter->fillPath(triangleBottom, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRightTriangle(QPainter *painter) const
{
    QPainterPath triangleRight = QTransform()
                                     .translate(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight / 2)
                                     .rotate(90)
                                     .map(TriangleBasic());
    painter->fillPath(triangleRight, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTopLineScissors(QPainter *painter) const
{
    QSvgRenderer svgRenderer(QStringLiteral("://puzzleicon/svg/icon_scissors_horizontal.svg"));
    svgRenderer.render(painter, QRectF(m_drawingAreaWidth - tileStripeWidth, 0, UnitConvertor(0.95, Unit::Cm, Unit::Px),
                                       UnitConvertor(0.56, Unit::Cm, Unit::Px)));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawLeftLineScissors(QPainter *painter) const
{
    QSvgRenderer svgRenderer(QStringLiteral("://puzzleicon/svg/icon_scissors_vertical.svg"));
    svgRenderer.render(painter,
                       QRectF(0, m_drawingAreaHeight - tileStripeWidth, UnitConvertor(0.56, Unit::Cm, Unit::Px),
                              UnitConvertor(0.95, Unit::Cm, Unit::Px)));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTopCuttingLine(QPainter *painter) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::DashLine);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth, 0));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawLeftCuttingLine(QPainter *painter) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::DashLine);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawBottomCuttingLine(QPainter *painter) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::DotLine);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                      QPointF(m_drawingAreaWidth, m_drawingAreaHeight - tileStripeWidth));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRightCuttingLine(QPainter *painter) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::DotLine);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(m_drawingAreaWidth - tileStripeWidth, 0),
                      QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawSolidTopLine(QPainter *painter, int col, int nbCol) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::SolidLine);
    painter->setPen(penTileInfos);

    if (col < nbCol - 1)
    {
        painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth, 0));
    }
    else
    {
        painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth - tileStripeWidth, 0));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawSolidLeftLine(QPainter *painter, int row, int nbRow) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::SolidLine);
    painter->setPen(penTileInfos);

    if (row < nbRow - 1)
    {
        painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight));
    }
    else
    {
        painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight - tileStripeWidth));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawSolidBottomLine(QPainter *painter, int col, int nbCol) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::SolidLine);
    painter->setPen(penTileInfos);

    if (col < nbCol - 1)
    {
        painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                          QPointF(m_drawingAreaWidth, m_drawingAreaHeight - tileStripeWidth));
    }
    else
    {
        painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                          QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight - tileStripeWidth));
        ;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawSolidRightLine(QPainter *painter, int row, int nbRow) const
{
    QPen penTileInfos = PenTileInfos();
    penTileInfos.setStyle(Qt::SolidLine);
    painter->setPen(penTileInfos);

    if (row < nbRow - 1)
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

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTextInformation(QPainter *painter, int row, int col, int nbRow, int nbCol,
                                        const QString &sheetName) const
{
    QTextDocument td;
    td.setPageSize(QSizeF(m_drawingAreaWidth - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaHeight));

    // paint the grid information
    const QString grid = tr("Grid ( %1 , %2 )").arg(row + 1).arg(col + 1);
    const QString tileColorStr =
        QStringLiteral("%1,%2,%3").arg(tileColor->red()).arg(tileColor->green()).arg(tileColor->blue());

    td.setHtml(u"<table width='100%' style='color:rgb(%1);'>"
               u"<tr>"
               u"<td align='center'>%2</td>"
               u"</tr>"
               u"</table>"_s.arg(tileColorStr, grid));
    painter->setPen(PenTileInfos());
    painter->save();
    painter->translate(QPointF(UnitConvertor(1, Unit::Cm, Unit::Px), m_drawingAreaHeight - tileStripeWidth / 1.3));
    td.drawContents(painter);
    painter->restore();

    // paint the page information
    const QString page = tr("Page %1 of %2").arg(row * nbCol + col + 1).arg(nbCol * nbRow);

    td.setPageSize(QSizeF(m_drawingAreaHeight - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaWidth));

    QFontMetrics metrix = QFontMetrics(td.defaultFont());
    int maxWidth = TextWidth(metrix, QString().fill('z', 50));
    QString clippedSheetName = metrix.elidedText(sheetName, Qt::ElideMiddle, maxWidth);

    td.setHtml(QStringLiteral("<table width='100%' style='color:rgb(%1);'>"
                              "<tr>"
                              "<td align='center'>%2 - %3</td>"
                              "</tr>"
                              "</table>")
                   .arg(tileColorStr, page, clippedSheetName));
    painter->save();
    painter->rotate(-90);
    painter->translate(
        QPointF(-(m_drawingAreaHeight) + UnitConvertor(1, Unit::Cm, Unit::Px), m_drawingAreaWidth - tileStripeWidth));
    td.drawContents(painter);
    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::PaintWatermarkText(QPainter *painter, const QRectF &img, const VWatermarkData &watermarkData,
                                       qreal xScale, qreal yScale)
{
    SCASSERT(painter != nullptr)

    painter->save();

    painter->setOpacity(watermarkData.opacity / 100.);

    QPen pen = painter->pen();
    pen.setWidth(1);
    pen.setColor(watermarkData.textColor);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    painter->setBrush(watermarkData.textColor);

    QTransform t;
    t.translate(img.center().x(), img.center().y());
    t.rotate(-watermarkData.textRotation);
    t.translate(-img.center().x(), -img.center().y());
    t.scale(xScale, yScale);

    QPainterPath text;
    text.addText(img.center(), watermarkData.font, watermarkData.text);

    text = t.map(text);

    QPointF center = img.center() - text.boundingRect().center();
    t = QTransform();
    t.translate(center.x(), center.y());

    text = t.map(text);

    painter->drawPath(text);

    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::PaintWatermarkImage(QPainter *painter, const QRectF &img, const VWatermarkData &watermarkData,
                                        const QString &watermarkPath, bool folowColorScheme, qreal xScale, qreal yScale)
{
    SCASSERT(painter != nullptr)

    const qreal opacity = watermarkData.opacity / 100.;
    auto BrokenImage = [img, watermarkData, watermarkPath, opacity, folowColorScheme]()
    {
        QString colorScheme = QStringLiteral("light");

        if (folowColorScheme)
        {
            colorScheme =
                (VTheme::ColorSheme() == VColorSheme::Light ? QStringLiteral("light") : QStringLiteral("dark"));
        }

        QPixmap watermark;
        QString imagePath =
            QStringLiteral("puzzle=colorScheme%1+path%2+opacity%3_broken")
                .arg(colorScheme, AbsoluteMPath(watermarkPath, watermarkData.path), QString::number(opacity));

        if (not QPixmapCache::find(imagePath, &watermark))
        {
            QScopedPointer<QSvgRenderer> svgRenderer(new QSvgRenderer());

            QRect imageRect(0, 0, qRound(img.width() / 4.), qRound(img.width() / 4.));
            watermark = QPixmap(imageRect.size());
            watermark.fill(Qt::transparent);

            QPainter imagePainter(&watermark);
            imagePainter.setOpacity(opacity);

            svgRenderer->load(QStringLiteral("://puzzleicon/svg/%1/no_watermark_image.svg").arg(colorScheme));
            svgRenderer->render(&imagePainter, imageRect);

            QPixmapCache::insert(imagePath, watermark);

            return watermark;
        }

        return watermark;
    };

    QString imagePath = AbsoluteMPath(watermarkPath, watermarkData.path);
    QFileInfo f(imagePath);

    QImageReader imageReader(imagePath);
    QImage watermarkImage = imageReader.read();

    if (watermarkImage.isNull())
    {
        QPixmap watermarkPixmap = BrokenImage();

        if (watermarkPixmap.width() < img.width() && watermarkPixmap.height() < img.height())
        {
            QRect imagePosition(0, 0, watermarkPixmap.width(), watermarkPixmap.height());
            imagePosition.translate(img.center().toPoint() - imagePosition.center());

            painter->drawPixmap(imagePosition, watermarkPixmap);
        }
        else
        {
            painter->drawPixmap(img.toRect(), watermarkPixmap);
        }
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    qint64 fileSize = watermarkImage.byteCount();
#else
    qint64 fileSize = watermarkImage.sizeInBytes();
#endif
    qint64 pixelSize = fileSize / watermarkImage.height() / watermarkImage.width();
    QSize scaledSize(qRound(watermarkImage.width() / xScale), qRound(watermarkImage.height() / yScale));
    qint64 scaledImageSize = pixelSize * scaledSize.width() * scaledSize.height() / 1024;
    int limit = QPixmapCache::cacheLimit();

    if (scaledImageSize > limit && (xScale < 1 || yScale < 1))
    {
        QScopedPointer<QSvgRenderer> svgRenderer(new QSvgRenderer());

        painter->save();
        painter->setOpacity(opacity);
        painter->restore();

        QString grayscale = watermarkData.grayscale ? QStringLiteral("_grayscale") : QString();
        svgRenderer->load(QStringLiteral("://puzzleicon/svg/watermark_placeholder%1.svg").arg(grayscale));
        QRect imageRect(0, 0, qRound(watermarkImage.width() / xScale), qRound(watermarkImage.height() / yScale));
        imageRect.translate(img.center().toPoint() - imageRect.center());
        svgRenderer->render(painter, imageRect);
        return;
    }

    QPixmap watermark;
    if (f.suffix() == "png"_L1 || f.suffix() == "jpg"_L1 || f.suffix() == "jpeg"_L1 || f.suffix() == "bmp"_L1)
    {
        QString error;
        watermark = WatermarkImageFromCache(watermarkData, watermarkPath, xScale, yScale, error);

        if (watermark.isNull())
        {
            watermark = BrokenImage();
        }
    }
    else
    {
        watermark = BrokenImage();
    }

    painter->save();
    painter->setOpacity(watermarkData.opacity / 100.);

    QRect imagePosition(0, 0, watermark.width(), watermark.height());
    imagePosition.translate(img.center().toPoint() - imagePosition.center());

    if (watermark.width() < img.width() && watermark.height() < img.height())
    {
        painter->drawPixmap(imagePosition, watermark);
    }
    else
    {
        QRect croppedRect = imagePosition.intersected(img.toRect());
        QPixmap cropped = watermark.copy(croppedRect.translated(-imagePosition.x(), -imagePosition.y()));

        painter->drawPixmap(croppedRect, cropped);
    }

    painter->restore();
}
