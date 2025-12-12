#include "vptilefactory.h"

#include <QtSvg>

#include "../vformat/vsinglelineoutlinechar.h"
#include "../vlayout/vprintlayout.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/literals.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/svgfont/vsvghandler.h"
#include "../vmisc/vcommonsettings.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "layout/vplayout.h"
#include "layout/vppiece.h"
#include "layout/vpsheet.h"
#include "scene/scenedef.h"
#include "theme/vtheme.h"
#include "vpapplication.h"
#include "vtextmanager.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(QBrush, triangleBush, (QColor(200, 200, 200, 225))) // NOLINT

struct WatermarkScaledImageInfo
{
    qint64 scaledImageSize{-1};
    int width{0};
    int height{0};
};

using WatermarkScaledSize = QCache<QString, WatermarkScaledImageInfo>;
Q_GLOBAL_STATIC(WatermarkScaledSize, watermarkSizeCache) // NOLINT

QT_WARNING_POP

const int tileTextFontSize = 12; // Adjust as needed
constexpr qreal notchHeight = UnitConvertor(3, Unit::Mm, Unit::Px);
constexpr qreal shortNotchHeight = UnitConvertor(1.1, Unit::Mm, Unit::Px);

//---------------------------------------------------------------------------------------------------------------------
inline auto GenerateWatermarkScaledSizeCacheKey(const QString &watermarkPath, qreal xScale, qreal yScale) -> QString
{
    return QStringLiteral("path%1+xscale%2+yscale%3")
        .arg(watermarkPath, QString::number(xScale), QString::number(yScale));
}

//---------------------------------------------------------------------------------------------------------------------
void CalculateAndCacheWatermarkScaledImageSize(const VWatermarkData &watermarkData, const QString &watermarkPath,
                                               qreal xScale, qreal yScale)
{
    QString const imagePath = AbsoluteMPath(watermarkPath, watermarkData.path);
    QImageReader imageReader(imagePath);
    QImage const watermarkImage = imageReader.read();

    qint64 scaledImageSize = -1;
    if (!watermarkImage.isNull())
    {
        qint64 const fileSize = watermarkImage.sizeInBytes();
        qint64 const pixelSize = fileSize / watermarkImage.height() / watermarkImage.width();
        QSize const scaledSize(qRound(watermarkImage.width() / xScale), qRound(watermarkImage.height() / yScale));
        scaledImageSize = pixelSize * scaledSize.width() * scaledSize.height() / 1024;
    }

    auto *info = new WatermarkScaledImageInfo;
    info->scaledImageSize = scaledImageSize;
    info->width = watermarkImage.width();
    info->height = watermarkImage.height();

    // Insert into the cache
    watermarkSizeCache->insert(GenerateWatermarkScaledSizeCacheKey(watermarkPath, xScale, yScale), info);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetWatermarkScaledImageInfo(const VWatermarkData &watermarkData, const QString &watermarkPath, qreal xScale,
                                 qreal yScale) -> WatermarkScaledImageInfo
{
    // Check if the value is already cached
    const WatermarkScaledImageInfo *info =
        watermarkSizeCache->object(GenerateWatermarkScaledSizeCacheKey(watermarkPath, xScale, yScale));
    if (info != nullptr)
    {
        return *info;
    }

    // If not cached, calculate and cache the value
    CalculateAndCacheWatermarkScaledImageSize(watermarkData, watermarkPath, xScale, yScale);
    // Retrieve the calculated value from the cache
    info = watermarkSizeCache->object(GenerateWatermarkScaledSizeCacheKey(watermarkPath, xScale, yScale));
    if (info != nullptr)
    {
        return *info;
    }

    return {}; // Return an error value if caching failed
}

//---------------------------------------------------------------------------------------------------------------------
auto Grayscale(QImage image) -> QImage
{
    for (int ii = 0; ii < image.height(); ii++)
    {
        // Scanline data is at least 32-bit aligned.
        // https://doc.qt.io/qt-6/qimage.html#scanLine
        void *voidPtr = image.scanLine(ii);
        auto *scan = static_cast<QRgb *>(voidPtr);
        for (int jj = 0; jj < image.width(); jj++)
        {
            auto *rgbpixel = scan + jj;
            int const gray = qGray(*rgbpixel);
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
    QString const imagePath = AbsoluteMPath(watermarkPath, watermarkData.path);

    if (auto const imageCacheKey = QStringLiteral("puzzle=path%1+rotation%3+grayscale%4+xscale%5+yxcale%6")
                                       .arg(imagePath, QString::number(watermarkData.imageRotation),
                                            watermarkData.grayscale ? trueStr : falseStr)
                                       .arg(xScale)
                                       .arg(yScale);
        not QPixmapCache::find(imageCacheKey, &pixmap))
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
    auto const rectBasic = QRectF(-UnitConvertor(0.5, Unit::Cm, Unit::Px), 0, UnitConvertor(1, Unit::Cm, Unit::Px),
                                  UnitConvertor(0.5, Unit::Cm, Unit::Px));
    QPainterPath triangleBasic;
    triangleBasic.moveTo(rectBasic.topLeft());
    triangleBasic.lineTo(rectBasic.topRight());
    triangleBasic.lineTo(rectBasic.left() + (rectBasic.width() / 2.), rectBasic.bottom());
    triangleBasic.lineTo(rectBasic.topLeft());
    return triangleBasic;
}

//---------------------------------------------------------------------------------------------------------------------
void InitPrimaryCuttingLineStyle(QPen &pen)
{
    if (const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
        settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts())
    {
        pen.setDashPattern({24, 4});
    }
    else
    {
        pen.setStyle(Qt::DashLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InitSecondaryCuttingLineStyle(QPen &pen)
{
    if (const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
        settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts())
    {
        pen.setDashPattern({12, 4});
    }
    else
    {
        pen.setStyle(Qt::DotLine);
    }
}
} // namespace

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
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        QSizeF const tilesSize = layout->LayoutSettings().GetTilesSize();
        QMarginsF const tilesMargins = layout->LayoutSettings().GetTilesMargins();

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
    VPLayoutPtr const layout = m_layout.toStrongRef();
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
void VPTileFactory::drawTile(
    QPainter *painter, QPrinter *printer, const VPSheetPtr &sheet, int row, int col, bool showRuler)
{
    SCASSERT(painter != nullptr)
    SCASSERT(printer != nullptr)

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

    QPainterStateGuard const guard(painter);

    // add the tiles decorations (cutting and gluing lines, scissors, infos etc.)
    painter->setPen(PenTileInfos());

    const auto [xscale, yscale] = VPrintLayout::PrinterScaleDiff(printer);
    painter->scale(xscale, yscale);

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

    if (showRuler)
    {
        DrawRuler(painter, xscale, sheet->SceneData()->IsTextAsPaths());
    }
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
    DrawGridTextInformation(painter, row, col, sheet->SceneData()->IsTextAsPaths());
    DrawPageTextInformation(painter, row, col, nbRow, nbCol, sheet->GetName(), sheet->SceneData()->IsTextAsPaths());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::RowNb(const VPSheetPtr &sheet) const -> int
{
    if (sheet.isNull())
    {
        return 0;
    }

    qreal yScale = 1;
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); not layout.isNull())
    {
        yScale = layout->LayoutSettings().VerticalScale();
    }

    QRectF const sheetSize = sheet->GetMarginsRect();
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
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); not layout.isNull())
    {
        xScale = layout->LayoutSettings().HorizontalScale();
    }

    QRectF const sheetSize = sheet->GetMarginsRect();
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
void VPTileFactory::DrawRuler(QPainter *painter, qreal scale, bool testAsPaths) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QPen const rulePen(tileColor, 1, Qt::SolidLine);

    QPainterStateGuard const guard(painter);
    painter->setPen(rulePen);

    Unit const layoutUnits = layout->LayoutSettings().GetUnit();
    Unit const rulerUnits = layoutUnits == Unit::Inch ? layoutUnits : Unit::Cm;

    const qreal step = UnitConvertor(1, rulerUnits, Unit::Px);
    double const marksCount = (m_drawingAreaWidth - tileStripeWidth) / step;
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
            if (m_commonSettings->GetSingleLineFonts())
            {
                DrawRulerSVGFont(painter, scale, testAsPaths);
            }
            else
            {
                DrawRulerOutlineFont(painter, scale, testAsPaths);
            }
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRulerOutlineFont(QPainter *painter, qreal scale, bool testAsPaths) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QPainterStateGuard const guard(painter);

    Unit const layoutUnits = layout->LayoutSettings().GetUnit();
    Unit const rulerUnits = layoutUnits == Unit::Inch ? layoutUnits : Unit::Cm;
    const qreal step = UnitConvertor(1, rulerUnits, Unit::Px);

    QString const units = rulerUnits != Unit::Inch ? tr("cm", "unit") : tr("in", "unit");

    if (m_commonSettings->GetSingleStrokeOutlineFont())
    {
        QPen textPen = PenTileInfos();
        textPen.setStyle(Qt::SolidLine);
        painter->setPen(textPen);

        QFont font = m_font;
        const int size = qRound(10 / scale);
        size > 0 ? font.setPointSize(size) : font.setPointSize(10);

        VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(m_commonSettings->GetPathFontCorrections());
        }

        QFontMetrics const fm(font);

        QPainterPath path;

        int w = 0;
        for (auto c : qAsConst(units))
        {
            path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        const QRectF textRect = fm.boundingRect(units);
        qreal const unitsWidth = fm.horizontalAdvance(units);

        painter->translate(
            QPointF(step * 0.5 - unitsWidth * 0.6,
                    m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight * 2 - textRect.height()));

        painter->drawPath(path);
    }
    else if (!testAsPaths)
    {
        QFont fnt = painter->font();
        const int size = qRound(10 / scale);
        size > 0 ? fnt.setPointSize(size) : fnt.setPointSize(10);
        painter->setFont(fnt);

        QFontMetrics const fm(fnt);
        qreal const unitsWidth = fm.horizontalAdvance(units);

        painter->drawText(QPointF(step * 0.5 - unitsWidth * 0.6,
                                  m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight),
                          units);
    }
    else
    {
        QPen textPen = PenTileInfos();
        textPen.setStyle(Qt::SolidLine);
        textPen.setWidthF(0.1);
        painter->setPen(textPen);
        painter->setBrush(QBrush(textPen.color(), Qt::SolidPattern));

        QFont fnt = painter->font();
        const int size = qRound(10 / scale);
        size > 0 ? fnt.setPointSize(size) : fnt.setPointSize(10);
        painter->setFont(fnt);

        QFontMetrics const fm(fnt);
        qreal const unitsWidth = fm.horizontalAdvance(units);

        QPainterPath path;
        path.addText(QPointF(), fnt, units);

        painter->translate(QPointF(step * 0.5 - unitsWidth * 0.6,
                                   m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight));

        painter->drawPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRulerSVGFont(QPainter *painter, qreal scale, bool testAsPaths) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    int size = qRound(10 / scale);
    size = size > 0 ? size : 10;

    const VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    const VSvgFontEngine engine = db->FontEngine(m_svgFontFamily, SVGFontStyle::Normal, SVGFontWeight::Normal, size);

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        DrawRulerOutlineFont(painter, scale, testAsPaths);
        return;
    }

    QPainterStateGuard const guard(painter);

    QPen textPen = PenTileInfos();
    textPen.setStyle(Qt::SolidLine);
    painter->setPen(textPen);

    Unit const layoutUnits = layout->LayoutSettings().GetUnit();
    Unit const rulerUnits = layoutUnits == Unit::Inch ? layoutUnits : Unit::Cm;
    const qreal step = UnitConvertor(1, rulerUnits, Unit::Px);
    QString const units = rulerUnits != Unit::Inch ? tr("cm", "unit") : tr("in", "unit");
    qreal const unitsWidth = engine.TextWidth(units, textPen.widthF());
    const QRectF textRect = engine.BoundingRect(units, textPen.widthF());

    painter->translate(
        QPointF(step * 0.5 - unitsWidth * 0.6,
                m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight - textRect.height()));

    painter->drawPath(engine.DrawPath(QPointF(), units, textPen.widthF()));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawWatermark(QPainter *painter) const
{
    SCASSERT(painter != nullptr)

    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    if (m_watermarkData.opacity > 0)
    {
        QRectF const img(0, 0, m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight - tileStripeWidth);

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
    return {tileColor, m_commonSettings->WidthHairLine(), Qt::DashLine, Qt::RoundCap, Qt::RoundJoin};
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTilePageContent(QPainter *painter, const VPSheetPtr &sheet, int row, int col,
                                        QPrinter *printer) const
{
    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QMarginsF sheetMargins;
    if (not sheet->IgnoreMargins())
    {
        sheetMargins = sheet->GetSheetMargins();
    }

    auto const penTileDrawing =
        QPen(Qt::black, m_commonSettings->WidthMainLine(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(penTileDrawing);

    // paint the content of the page
    const qreal xScale = layout->LayoutSettings().HorizontalScale();
    const qreal yScale = layout->LayoutSettings().VerticalScale();
    auto const source = QRectF(sheetMargins.left() + col * (m_drawingAreaWidth - tileStripeWidth) / xScale,
                               sheetMargins.top() + row * (m_drawingAreaHeight - tileStripeWidth) / yScale,
                               m_drawingAreaWidth / xScale, m_drawingAreaHeight / yScale);

    auto const target = QRectF(0, 0, m_drawingAreaWidth, m_drawingAreaHeight);
    sheet->SceneData()->Scene()->render(painter, VPrintLayout::SceneTargetRect(printer, target), source,
                                        Qt::IgnoreAspectRatio);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTopTriangle(QPainter *painter) const
{
    QPainterPath const triangleTop = QTransform().translate(m_drawingAreaWidth / 2, 0).map(TriangleBasic());
    painter->fillPath(triangleTop, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawLeftTriangle(QPainter *painter) const
{
    QPainterPath const triangleLeft =
        QTransform().translate(0, m_drawingAreaHeight / 2).rotate(-90).map(TriangleBasic());
    painter->fillPath(triangleLeft, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawBottomTriangle(QPainter *painter) const
{
    QPainterPath const triangleBottom = QTransform()
                                            .translate(m_drawingAreaWidth / 2, m_drawingAreaHeight - tileStripeWidth)
                                            .rotate(180)
                                            .map(TriangleBasic());
    painter->fillPath(triangleBottom, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRightTriangle(QPainter *painter) const
{
    QPainterPath const triangleRight = QTransform()
                                           .translate(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight / 2)
                                           .rotate(90)
                                           .map(TriangleBasic());
    painter->fillPath(triangleRight, *triangleBush);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::ShowTopLine() const -> bool
{
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); !layout.isNull())
    {
        const VPLayoutSettings &settings = layout->LayoutSettings();
        if (settings.IgnoreTilesMargins())
        {
            return false;
        }

        if (QMarginsF const tilesMargins = settings.GetTilesMargins(); qFuzzyIsNull(tilesMargins.top()))
        {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTileFactory::ShowLeftLine() const -> bool
{
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); !layout.isNull())
    {
        const VPLayoutSettings &settings = layout->LayoutSettings();
        if (settings.IgnoreTilesMargins())
        {
            return false;
        }

        if (QMarginsF const tilesMargins = settings.GetTilesMargins(); qFuzzyIsNull(tilesMargins.left()))
        {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTopLineScissors(QPainter *painter) const
{
    if (!ShowTopLine())
    {
        return;
    }

    QPainterStateGuard guard(painter);
    QPen pen = painter->pen();
    pen.setColor(tileColor);
    pen.setWidthF(m_commonSettings->WidthHairLine());
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    if (const bool singleLineFont = settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts();
        !singleLineFont)
    {
        static QSvgRenderer svgRenderer(QStringLiteral("://puzzleicon/svg/icon_scissors_horizontal.svg"));
        svgRenderer.render(painter,
                           QRectF(m_drawingAreaWidth - tileStripeWidth,
                                  0,
                                  UnitConvertor(0.95, Unit::Cm, Unit::Px),
                                  UnitConvertor(0.56, Unit::Cm, Unit::Px)));
    }
    else
    {
        static VSvgHandler handler(QStringLiteral("://puzzleicon/svg/icon_scissors_plotter_horizontal.svg"));
        guard.save();
        painter->translate(m_drawingAreaWidth - tileStripeWidth, 0);
        handler.Render(painter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawLeftLineScissors(QPainter *painter) const
{
    if (!ShowLeftLine())
    {
        return;
    }

    QPainterStateGuard guard(painter);
    QPen pen = painter->pen();
    pen.setColor(tileColor);
    pen.setWidthF(m_commonSettings->WidthHairLine());
    painter->setPen(pen);

    const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    if (const bool singleLineFont = settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts();
        !singleLineFont)
    {
        static QSvgRenderer svgRenderer(QStringLiteral("://puzzleicon/svg/icon_scissors_vertical.svg"));
        svgRenderer.render(painter,
                           QRectF(0,
                                  m_drawingAreaHeight - tileStripeWidth,
                                  UnitConvertor(0.56, Unit::Cm, Unit::Px),
                                  UnitConvertor(0.95, Unit::Cm, Unit::Px)));
    }
    else
    {
        static VSvgHandler handler(QStringLiteral("://puzzleicon/svg/icon_scissors_plotter_vertical.svg"));
        guard.save();
        painter->translate(0, m_drawingAreaHeight - tileStripeWidth);
        handler.Render(painter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawTopCuttingLine(QPainter *painter) const
{
    if (!ShowTopLine())
    {
        return;
    }
    QPen penTileInfos = PenTileInfos();
    InitPrimaryCuttingLineStyle(penTileInfos);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(), QPointF(m_drawingAreaWidth, 0));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawLeftCuttingLine(QPainter *painter) const
{
    if (!ShowLeftLine())
    {
        return;
    }
    QPen penTileInfos = PenTileInfos();
    InitPrimaryCuttingLineStyle(penTileInfos);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(), QPointF(0, m_drawingAreaHeight));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawBottomCuttingLine(QPainter *painter) const
{
    QPen penTileInfos = PenTileInfos();
    InitSecondaryCuttingLineStyle(penTileInfos);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(0, m_drawingAreaHeight - tileStripeWidth),
                      QPointF(m_drawingAreaWidth, m_drawingAreaHeight - tileStripeWidth));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawRightCuttingLine(QPainter *painter) const
{
    QPen penTileInfos = PenTileInfos();
    InitSecondaryCuttingLineStyle(penTileInfos);
    painter->setPen(penTileInfos);
    painter->drawLine(QPointF(m_drawingAreaWidth - tileStripeWidth, 0),
                      QPointF(m_drawingAreaWidth - tileStripeWidth, m_drawingAreaHeight));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawSolidTopLine(QPainter *painter, int col, int nbCol) const
{
    if (!ShowTopLine())
    {
        return;
    }

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
    if (!ShowLeftLine())
    {
        return;
    }

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
void VPTileFactory::DrawGridTextInformation(QPainter *painter, int row, int col, bool testAsPaths) const
{
    if (const QString grid = tr("Grid ( %1 , %2 )").arg(row + 1).arg(col + 1); m_commonSettings->GetSingleLineFonts())
    {
        DrawGridTextInformationSVGFont(painter, grid, testAsPaths);
    }
    else
    {
        DrawGridTextInformationOutlineFont(painter, grid, testAsPaths);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawGridTextInformationOutlineFont(QPainter *painter, const QString &text, bool testAsPaths) const
{
    QPainterStateGuard guard(painter);

    QPen textPen = PenTileInfos();
    textPen.setStyle(Qt::SolidLine);
    painter->setPen(textPen);

    if (m_commonSettings->GetSingleStrokeOutlineFont())
    {
        // Setup painter font
        QFont font = m_font;
        font.setPointSize(tileTextFontSize);
        painter->setFont(font);

        VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(m_commonSettings->GetPathFontCorrections());
        }

        QPainterPath path;
        QFontMetrics const fm(font);

        int w = 0;
        for (auto c : qAsConst(text))
        {
            path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        // Compute position to center the text
        qreal const x = UnitConvertor(1, Unit::Cm, Unit::Px);
        qreal const y = m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight;
        qreal const centerX = (m_drawingAreaWidth - tileStripeWidth - UnitConvertor(1, Unit::Cm, Unit::Px)) / 2.0
                              - w / 2.0;

        painter->translate(x + centerX, y);

        painter->drawPath(path);
    }
    else if (!testAsPaths)
    {
        QTextDocument td;
        td.setPageSize(QSizeF(m_drawingAreaWidth - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaHeight));

        // paint the grid information
        const QString tileColorStr
            = QStringLiteral("%1,%2,%3").arg(tileColor.red()).arg(tileColor.green()).arg(tileColor.blue());

        td.setHtml(u"<table width='100%' style='color:rgb(%1);'>"
                   u"<tr>"
                   u"<td align='center'>%2</td>"
                   u"</tr>"
                   u"</table>"_s.arg(tileColorStr, text));
        guard.save();
        painter->translate(QPointF(UnitConvertor(1, Unit::Cm, Unit::Px), m_drawingAreaHeight - tileStripeWidth / 1.3));
        td.drawContents(painter);
    }
    else
    {
        // Setup painter font
        QFont font = m_font;
        font.setPointSize(tileTextFontSize);

        textPen.setStyle(Qt::SolidLine);
        textPen.setWidthF(0.1);
        painter->setPen(textPen);
        painter->setBrush(QBrush(textPen.color(), Qt::SolidPattern));

        const QFontMetrics fm(font);
        const QRectF textRect = fm.boundingRect(text);

        QPainterPath path;
        path.addText(QPointF(), font, text);

        // Compute position to center the text
        qreal const x = UnitConvertor(1, Unit::Cm, Unit::Px);
        qreal const y = m_drawingAreaHeight - tileStripeWidth / 1.3 + textRect.height();
        qreal const centerX = (m_drawingAreaWidth - tileStripeWidth - UnitConvertor(1, Unit::Cm, Unit::Px)) / 2.0
                              - textRect.width() / 2.0;

        painter->translate(x + centerX, y);

        painter->drawPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawGridTextInformationSVGFont(QPainter *painter, const QString &text, bool testAsPaths) const
{
    const VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    const VSvgFontEngine engine = db->FontEngine(m_svgFontFamily,
                                                 SVGFontStyle::Normal,
                                                 SVGFontWeight::Normal,
                                                 tileTextFontSize);

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        DrawGridTextInformationOutlineFont(painter, text, testAsPaths);
        return;
    }

    QPainterStateGuard const guard(painter);

    QPen textPen = PenTileInfos();
    textPen.setStyle(Qt::SolidLine);
    painter->setPen(textPen);

    // Compute position to center the text
    qreal const x = UnitConvertor(1, Unit::Cm, Unit::Px);
    qreal const y = m_drawingAreaHeight - tileStripeWidth + notchHeight + shortNotchHeight;
    qreal const centerX = (m_drawingAreaWidth - tileStripeWidth - UnitConvertor(1, Unit::Cm, Unit::Px)) / 2.0
                          - engine.TextWidth(text, textPen.widthF()) / 2.0;

    painter->translate(x + centerX, y);

    painter->drawPath(engine.DrawPath(QPointF(), text, textPen.widthF()));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawPageTextInformation(
    QPainter *painter, int row, int col, int nbRow, int nbCol, const QString &sheetName, bool testAsPaths) const
{
    if (const QString page = tr("Page %1 of %2").arg(row * nbCol + col + 1).arg(nbCol * nbRow);
        m_commonSettings->GetSingleLineFonts())
    {
        DrawPageTextInformationSVGFont(painter, page, sheetName, testAsPaths);
    }
    else
    {
        DrawPageTextInformationOutlineFont(painter, page, sheetName, testAsPaths);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawPageTextInformationOutlineFont(QPainter *painter,
                                                       const QString &text,
                                                       const QString &sheetName,
                                                       bool testAsPaths) const
{
    QPainterStateGuard guard(painter);

    QPen textPen = PenTileInfos();
    textPen.setStyle(Qt::SolidLine);
    painter->setPen(textPen);

    if (m_commonSettings->GetSingleStrokeOutlineFont())
    {
        // Setup painter font
        QFont font = m_font;
        font.setPointSize(tileTextFontSize);
        painter->setFont(font);

        VSingleLineOutlineChar const corrector(font);
        if (!corrector.IsPopulated())
        {
            corrector.LoadCorrections(m_commonSettings->GetPathFontCorrections());
        }

        auto const fm = QFontMetrics(font);
        int const maxWidth = fm.horizontalAdvance(QString().fill('z', 50));
        QString const clippedSheetName = fm.elidedText(sheetName, Qt::ElideMiddle, maxWidth);

        const QString page = text + " - "_L1 + clippedSheetName;

        QPainterPath path;

        int w = 0;
        for (auto c : qAsConst(page))
        {
            path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
            w += fm.horizontalAdvance(c);
        }

        painter->rotate(-90);

        // Compute position to center the text
        qreal const x = -m_drawingAreaHeight / 2.0 - w / 2.0;
        qreal const y = m_drawingAreaWidth - tileStripeWidth;

        painter->translate(QPointF(x, y));

        painter->drawPath(path);
    }
    else if (!testAsPaths)
    {
        const QString tileColorStr
            = QStringLiteral("%1,%2,%3").arg(tileColor.red()).arg(tileColor.green()).arg(tileColor.blue());

        // paint the page information
        QTextDocument td;
        td.setPageSize(QSizeF(m_drawingAreaHeight - UnitConvertor(2, Unit::Cm, Unit::Px), m_drawingAreaWidth));

        auto const metrix = QFontMetrics(td.defaultFont());
        int const maxWidth = metrix.horizontalAdvance(QString().fill('z', 50));
        QString const clippedSheetName = metrix.elidedText(sheetName, Qt::ElideMiddle, maxWidth);

        td.setHtml(QStringLiteral("<table width='100%' style='color:rgb(%1);'>"
                                  "<tr>"
                                  "<td align='center'>%2 - %3</td>"
                                  "</tr>"
                                  "</table>")
                       .arg(tileColorStr, text, clippedSheetName));
        guard.save();
        painter->rotate(-90);
        painter->translate(
            QPointF(-m_drawingAreaHeight + UnitConvertor(1, Unit::Cm, Unit::Px), m_drawingAreaWidth - tileStripeWidth));
        td.drawContents(painter);
    }
    else
    {
        // Setup painter font
        QFont font = m_font;
        font.setPointSize(tileTextFontSize);

        textPen.setStyle(Qt::SolidLine);
        textPen.setWidthF(0.1);
        painter->setPen(textPen);
        painter->setBrush(QBrush(textPen.color(), Qt::SolidPattern));

        auto const fm = QFontMetrics(font);
        int const maxWidth = fm.horizontalAdvance(QString().fill('z', 50));
        QString const clippedSheetName = fm.elidedText(sheetName, Qt::ElideMiddle, maxWidth);

        const QString page = text + " - "_L1 + clippedSheetName;

        const QRectF textRect = fm.boundingRect(page);

        QPainterPath path;
        path.addText(QPointF(), font, page);

        painter->rotate(-90);

        // Compute position to center the text
        qreal const x = -m_drawingAreaHeight / 2.0 - textRect.width() / 2.0;
        qreal const y = m_drawingAreaWidth - tileStripeWidth + textRect.height();

        painter->translate(QPointF(x, y));

        painter->drawPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::DrawPageTextInformationSVGFont(QPainter *painter,
                                                   const QString &text,
                                                   const QString &sheetName,
                                                   bool testAsPaths) const
{
    const VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    const VSvgFontEngine engine = db->FontEngine(m_svgFontFamily,
                                                 SVGFontStyle::Normal,
                                                 SVGFontWeight::Normal,
                                                 tileTextFontSize);

    if (VSvgFont const svgFont = engine.Font(); !svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        DrawPageTextInformationOutlineFont(painter, text, sheetName, testAsPaths);
        return;
    }

    QPainterStateGuard const guard(painter);

    QPen textPen = PenTileInfos();
    textPen.setStyle(Qt::SolidLine);
    painter->setPen(textPen);

    int const maxWidth = qFloor(engine.TextWidth(QString().fill('z', 50), textPen.widthF()));
    QString const clippedSheetName = engine.ElidedText(sheetName,
                                                       SVGTextElideMode::ElideRight,
                                                       maxWidth,
                                                       textPen.widthF());

    const QString page = text + " - "_L1 + clippedSheetName;

    painter->rotate(-90);

    // Compute position to center the text
    qreal const x = -m_drawingAreaHeight / 2.0 - engine.TextWidth(page, textPen.widthF()) / 2.0;
    qreal const y = m_drawingAreaWidth - tileStripeWidth + ToPixel(1, Unit::Mm);

    painter->translate(QPointF(x, y));

    painter->drawPath(engine.DrawPath(QPointF(), page, textPen.widthF()));
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::PaintWatermarkText(QPainter *painter, const QRectF &img, const VWatermarkData &watermarkData,
                                       qreal xScale, qreal yScale)
{
    SCASSERT(painter != nullptr)

    QPainterStateGuard const guard(painter);

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

    QPointF const center = img.center() - text.boundingRect().center();
    t = QTransform();
    t.translate(center.x(), center.y());

    text = t.map(text);

    painter->drawPath(text);
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::PaintWatermarkImage(QPainter *painter, const QRectF &img, const VWatermarkData &watermarkData,
                                        const QString &watermarkPath, bool folowColorScheme, qreal xScale, qreal yScale)
{
    SCASSERT(painter != nullptr)

    const qreal opacity = watermarkData.opacity / 100.;
    auto BrokenImage = [&img, &watermarkData, watermarkPath, opacity, folowColorScheme]()
    {
        auto colorScheme = QStringLiteral("light");

        if (folowColorScheme)
        {
            colorScheme =
                (VTheme::ColorSheme() == VColorSheme::Light ? QStringLiteral("light") : QStringLiteral("dark"));
        }

        QPixmap watermark;

        if (QString const imagePath =
                QStringLiteral("puzzle=colorScheme%1+path%2+opacity%3_broken")
                    .arg(colorScheme, AbsoluteMPath(watermarkPath, watermarkData.path), QString::number(opacity));
            not QPixmapCache::find(imagePath, &watermark))
        {
            QScopedPointer<QSvgRenderer> const svgRenderer(new QSvgRenderer());

            QRect const imageRect(0, 0, qRound(img.width() / 4.), qRound(img.width() / 4.));
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

    WatermarkScaledImageInfo const watermarkImageInfo =
        GetWatermarkScaledImageInfo(watermarkData, watermarkPath, xScale, yScale);

    if (watermarkImageInfo.scaledImageSize == -1)
    {
        if (QPixmap const watermarkPixmap = BrokenImage();
            watermarkPixmap.width() < img.width() && watermarkPixmap.height() < img.height())
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

    if (int const limit = QPixmapCache::cacheLimit();
        watermarkImageInfo.scaledImageSize > limit && (xScale < 1 || yScale < 1))
    {
        QScopedPointer<QSvgRenderer> const svgRenderer(new QSvgRenderer());

        QPainterStateGuard const guard(painter);
        painter->setOpacity(opacity);

        QString const grayscale = watermarkData.grayscale ? QStringLiteral("_grayscale") : QString();
        svgRenderer->load(QStringLiteral("://puzzleicon/svg/watermark_placeholder%1.svg").arg(grayscale));
        QRect imageRect(0, 0, qRound(watermarkImageInfo.width / xScale), qRound(watermarkImageInfo.height / yScale));
        imageRect.translate(img.center().toPoint() - imageRect.center());
        svgRenderer->render(painter, imageRect);
        return;
    }

    QFileInfo const f(AbsoluteMPath(watermarkPath, watermarkData.path));
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

    QPainterStateGuard const guard(painter);
    painter->setOpacity(watermarkData.opacity / 100.);

    QRect imagePosition(0, 0, watermark.width(), watermark.height());
    imagePosition.translate(img.center().toPoint() - imagePosition.center());

    if (watermark.width() < img.width() && watermark.height() < img.height())
    {
        painter->drawPixmap(imagePosition, watermark);
    }
    else
    {
        QRect const croppedRect = imagePosition.intersected(img.toRect());
        QPixmap const cropped = watermark.copy(croppedRect.translated(-imagePosition.x(), -imagePosition.y()));

        painter->drawPixmap(croppedRect, cropped);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::InitTileSingleStrokeOutlineFont()
{
    m_font = QFont();

    if (!m_commonSettings->GetSingleStrokeOutlineFont())
    {
        return;
    }

    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QList<VPPiecePtr> const allPieces = layout->GetPieces();
    if (allPieces.isEmpty())
    {
        return;
    }

    QMap<QString, QFont> fontMap;

    for (const auto &piece : allPieces)
    {
        if (QVector<QPointF> const labelShape = piece->GetPieceLabelRect(); labelShape.count() == 4)
        {
            const QFont font = piece->GetPieceLabelData().GetFont();
            fontMap.insert(font.family(), font);
        }

        if (QVector<QPointF> const labelShape = piece->GetPatternLabelRect(); labelShape.count() == 4)
        {
            const QFont font = piece->GetPatternLabelData().GetFont();
            fontMap.insert(font.family(), font);
        }
    }

    if (fontMap.size() == 1)
    {
        m_font = fontMap.first();
        return;
    }

    if (fontMap.size() > 1)
    {
        const QList<QString> sortedFamilies = fontMap.keys();
        const QString &selectedFamily = sortedFamilies.first();
        const QFont &selectedFont = fontMap[selectedFamily];

        // Step 2: Build list of font names for warning
        const QString warning = tr("Warning: Multiple single stroke outline fonts detected (%1). "
                                   "The font \"%2\" was selected.")
                                    .arg(sortedFamilies.join(", "_L1), selectedFamily);

        qWarning().noquote() << warning;

        m_font = selectedFont;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPTileFactory::InitTileSingleLineSVGFont()
{
    m_svgFontFamily.clear();

    if (!m_commonSettings->GetSingleLineFonts())
    {
        return;
    }

    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QList<VPPiecePtr> const allPieces = layout->GetPieces();
    if (allPieces.isEmpty())
    {
        return;
    }

    QSet<QString> fonts;

    for (const auto &piece : allPieces)
    {
        if (QVector<QPointF> const labelShape = piece->GetPieceLabelRect(); labelShape.count() == 4)
        {
            fonts.insert(piece->GetPieceLabelData().GetSVGFontFamily());
        }

        if (QVector<QPointF> const labelShape = piece->GetPatternLabelRect(); labelShape.count() == 4)
        {
            fonts.insert(piece->GetPatternLabelData().GetSVGFontFamily());
        }
    }

    if (fonts.size() == 1)
    {
        m_svgFontFamily = *fonts.constBegin();
        return;
    }

    if (fonts.size() > 1)
    {
        QList<QString> fontList = fonts.values();
        std::sort(fontList.begin(),
                  fontList.end(),
                  [](const QString &a, const QString &b) { return a.compare(b, Qt::CaseInsensitive) < 0; });

        const QString selectedFont = fontList.first();

        const QString warning = tr("Warning: Multiple single-line SVG fonts detected (%1). "
                                   "The font \"%2\" was selected.")
                                    .arg(fontList.join(", "_L1), selectedFont);

        qWarning().noquote() << warning;

        m_svgFontFamily = selectedFont;
    }
}
