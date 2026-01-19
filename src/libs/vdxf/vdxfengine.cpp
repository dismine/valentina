/************************************************************************
**
**  @file   vdxfengine.cpp
**  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
**  @date   12 8, 2015
**
**  @brief
**  @copyright
**  This source code is part of the Valentina project, a pattern making
**  program, whose allow create and modeling patterns of clothing.
**  Copyright (C) 2013-2015 Valentina project
**  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
**
**  Valentina is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Valentina is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include "vdxfengine.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QFlag>
#include <QFlags>
#include <QFont>
#include <QLineF>
#include <QList>
#include <QLoggingCategory>
#include <QMessageLogger>
#include <QPaintEngineState>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QTextItem>
#include <QtDebug>
#include <QtMath>
#include <memory>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
#include "../vmisc/codecs/qtextcodec.h"
#else
#include "../vmisc/vtextcodec.h"
using QTextCodec = VTextCodec;
#endif // WITH_TEXTCODEC
#else
#include <QTextCodec>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vfoldline.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vlayoutpoint.h"
#include "../vlayout/vtextmanager.h"
#include "../vmisc/def.h"
#include "dxiface.h"
#include "libdxfrw/drw_entities.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vDxf, "v.undo") // NOLINT

QT_WARNING_POP

namespace
{
const qreal AAMATextHeight = 2.5;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer0, (UTF8STRING("0"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer1, (UTF8STRING("1"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer2, (UTF8STRING("2"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer3, (UTF8STRING("3"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer4, (UTF8STRING("4"))) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer5, (UTF8STRING("5"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer6, (UTF8STRING("6"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer7, (UTF8STRING("7"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer8, (UTF8STRING("8"))) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer9, (UTF8STRING("9"))) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer10, (UTF8STRING("10"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer11, (UTF8STRING("11"))) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer12, (UTF8STRING("12"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer13, (UTF8STRING("13"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer14, (UTF8STRING("14"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer15, (UTF8STRING("15"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer19, (UTF8STRING("19"))) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer26, (UTF8STRING("26"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer80, (UTF8STRING("80"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer81, (UTF8STRING("81"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer82, (UTF8STRING("82"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer83, (UTF8STRING("83"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer84, (UTF8STRING("84"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer85, (UTF8STRING("85"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer86, (UTF8STRING("86"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer87, (UTF8STRING("87"))) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline auto LineFont(const TextLine &tl, const QFont &base) -> QFont
{
    QFont fnt = base;
    fnt.setPointSize(qMax(base.pointSize() + tl.iFontSize, 1));
    if (!VAbstractApplication::VApp()->Settings()->GetSingleStrokeOutlineFont())
    {
        fnt.setBold(tl.bold);
    }
    fnt.setItalic(tl.italic);
    return fnt;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineAlign(const TextLine &tl, const QString &text, const QFontMetrics &fm, qreal width) -> qreal
{
    const int lineWidth = fm.horizontalAdvance(text);

    qreal dX = 0;
    if ((tl.eAlign & Qt::AlignHCenter) > 0)
    {
        dX = (width - lineWidth) / 2;
    }
    else if ((tl.eAlign & Qt::AlignRight) > 0)
    {
        dX = width - lineWidth;
    }

    return dX;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto svgEngineFeatures() -> QPaintEngine::PaintEngineFeatures
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wsign-conversion")
    QT_WARNING_DISABLE_INTEL(68)

    return {QPaintEngine::AllFeatures & ~QPaintEngine::PatternBrush & ~QPaintEngine::PerspectiveTransform
            & ~QPaintEngine::ConicalGradientFill & ~QPaintEngine::PorterDuff};

    QT_WARNING_POP
}
} // namespace


//---------------------------------------------------------------------------------------------------------------------
VDxfEngine::VDxfEngine()
  : QPaintEngine(svgEngineFeatures()),
    m_textBuffer(new DRW_Text())
{
}

//---------------------------------------------------------------------------------------------------------------------
VDxfEngine::~VDxfEngine()
{
    delete m_textBuffer;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::begin(QPaintDevice *pdev) -> bool
{
    Q_UNUSED(pdev)

    if (isActive())
    {
        qCWarning(vDxf) << qUtf8Printable("VDxfEngine::begin(), the engine was alredy activated"_L1);
        return false;
    }

    if (not m_size.isValid())
    {
        qCWarning(vDxf) << qUtf8Printable("VDxfEngine::begin(), size is not valid"_L1);
        return false;
    }

    m_input = QSharedPointer<dx_iface>(
        new dx_iface(m_fileName.toUtf8().toStdString(), m_version, m_varMeasurement, m_varInsunits));
    m_input->AddDefHeaderData();
    m_input->AddQtLTypes();
    m_input->AddDefLayers();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::end() -> bool
{
    return m_input->fileExport(m_binary);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VDxfEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    // always stream full gstate, which is not required, but...
    flags |= QPaintEngine::AllDirty;

    if (flags & QPaintEngine::DirtyTransform)
    {
        m_matrix = state.transform(); // Save new matrix for moving paths
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawPath(const QPainterPath &path)
{
    const QList<QPolygonF> subpaths = path.toSubpathPolygons(m_matrix);

    for (const auto &polygon : subpaths)
    {
        if (polygon.isEmpty())
        {
            continue;
        }

        if (m_version > DRW::AC1009)
        { // Use lwpolyline
            auto *poly = new DRW_LWPolyline();
            poly->layer = *layer0;
            poly->color = GetPenColor();
            poly->lWeight = DRW_LW_Conv::widthByLayer;
            poly->lineType = GetPenStyle();

            if (polygon.size() > 1 && polygon.constFirst() == polygon.constLast())
            {
                poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
            }

            poly->flags |= 0x80; // plinegen NOLINT(hicpp-signed-bitwise)

            for (auto p : polygon)
            {
                poly->addVertex(DRW_Vertex2D(FromPixel(p.x(), m_varInsunits),
                                             FromPixel(GetSize().height() - p.y(), m_varInsunits), 0));
            }

            m_input->AddEntity(poly);
        }
        else
        { // Use polyline
            auto *poly = new DRW_Polyline();
            poly->layer = *layer0;
            poly->color = GetPenColor();
            poly->lWeight = DRW_LW_Conv::widthByLayer;
            poly->lineType = GetPenStyle();
            if (polygon.size() > 1 && polygon.constFirst() == polygon.constLast())
            {
                poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
            }

            poly->flags |= 0x80; // plinegen NOLINT(hicpp-signed-bitwise)

            for (auto p : polygon)
            {
                poly->addVertex(DRW_Vertex(FromPixel(p.x(), m_varInsunits),
                                           FromPixel(GetSize().height() - p.y(), m_varInsunits), 0, 0));
            }

            m_input->AddEntity(poly);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawLines(const QLineF *lines, int lineCount)
{
    for (int i = 0; i < lineCount; ++i)
    {
        const QPointF p1 = m_matrix.map(lines[i].p1()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const QPointF p2 = m_matrix.map(lines[i].p2()); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        auto *line = new DRW_Line();
        line->basePoint =
            DRW_Coord(FromPixel(p1.x(), m_varInsunits), FromPixel(GetSize().height() - p1.y(), m_varInsunits), 0);
        line->secPoint =
            DRW_Coord(FromPixel(p2.x(), m_varInsunits), FromPixel(GetSize().height() - p2.y(), m_varInsunits), 0);
        line->layer = *layer0;
        line->color = GetPenColor();
        line->lWeight = DRW_LW_Conv::widthByLayer;
        line->lineType = GetPenStyle();

        m_input->AddEntity(line);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_UNUSED(mode)

    if (pointCount <= 0)
    {
        return;
    }

    if (m_version > DRW::AC1009)
    { // Use lwpolyline
        auto *poly = new DRW_LWPolyline();
        poly->layer = *layer0;
        poly->color = GetPenColor();
        poly->lWeight = DRW_LW_Conv::widthByLayer;
        poly->lineType = GetPenStyle();

        if (pointCount > 1 &&
            points[0] == points[pointCount]) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        {
            poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
        }

        poly->flags |= 0x80; // plinegen NOLINT(hicpp-signed-bitwise)

        for (int i = 0; i < pointCount; ++i)
        {
            const QPointF p = m_matrix.map(points[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            poly->addVertex(
                DRW_Vertex2D(FromPixel(p.x(), m_varInsunits), FromPixel(GetSize().height() - p.y(), m_varInsunits), 0));
        }

        m_input->AddEntity(poly);
    }
    else
    { // Use polyline
        auto *poly = new DRW_Polyline();
        poly->layer = *layer0;
        poly->color = GetPenColor();
        poly->lWeight = DRW_LW_Conv::widthByLayer;
        poly->lineType = GetPenStyle();

        if (pointCount > 1 &&
            points[0] == points[pointCount]) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        {
            poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
        }

        poly->flags |= 0x80; // plinegen NOLINT(hicpp-signed-bitwise)

        for (int i = 0; i < pointCount; ++i)
        {
            const QPointF p = m_matrix.map(points[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            poly->addVertex(DRW_Vertex(FromPixel(p.x(), m_varInsunits),
                                       FromPixel(GetSize().height() - p.y(), m_varInsunits), 0, 0));
        }

        m_input->AddEntity(poly);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawEllipse(const QRectF &rect)
{
    const QRectF newRect = m_matrix.mapRect(rect);
    const double rotationAngle = atan(m_matrix.m12() / m_matrix.m11());

    // distanse between center and endpoint of the major axis
    double majorX = 0;
    double majorY = 0;
    double ratio = 0; // ratio of minor axis to major axis
    if (rect.width() <= rect.height())
    {
        majorX = (rect.top() - rect.center().y()) * sin(rotationAngle) * m_matrix.m11() / cos(rotationAngle);
        // major axis * sin(rotation angle) * x-scale-factor
        majorY = (rect.top() - rect.center().y()) * m_matrix.m22();
        // major axis * cos(rotation angle) * y-scale-factor, where y-scale-factor = matrix.m22()/cos(rotationAngle)
        ratio = rect.width() / rect.height();
    }
    else
    {
        majorX = (rect.right() - rect.center().x()) * m_matrix.m11();
        // major axis * cos(rotation angle) * x-scale-factor, where y-scale-factor = matrix.m22()/cos(rotationAngle)
        majorY = (rect.right() - rect.center().x()) * sin(rotationAngle) * m_matrix.m22() / cos(rotationAngle);
        // major axis * sin(rotation angle) * y-scale-factor
        ratio = rect.height() / rect.width();
    }

    auto *ellipse = new DRW_Ellipse();
    ellipse->basePoint = DRW_Coord(FromPixel(newRect.center().x(), m_varInsunits),
                                   FromPixel(GetSize().height() - newRect.center().y(), m_varInsunits), 0);
    ellipse->secPoint = DRW_Coord(FromPixel(majorX, m_varInsunits), FromPixel(majorY, m_varInsunits), 0);
    ellipse->ratio = ratio;
    ellipse->staparam = 0;
    ellipse->endparam = 2 * M_PI;

    ellipse->layer = *layer0;
    ellipse->color = GetPenColor();
    ellipse->lWeight = DRW_LW_Conv::widthByLayer;
    ellipse->lineType = GetPenStyle();

    m_input->AddEntity(ellipse);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    if (m_textBuffer->text.empty())
    {
        const QPointF startPoint = m_matrix.map(p);
        const double rotationAngle = qRadiansToDegrees(qAtan2(m_matrix.m12(), m_matrix.m11()));

        const QFont f = textItem.font();
        const UTF8STRING fontStyle = m_input->AddFont(f);

        m_textBuffer->basePoint = DRW_Coord(FromPixel(startPoint.x(), m_varInsunits),
                                            FromPixel(GetSize().height() - startPoint.y(), m_varInsunits), 0);
        m_textBuffer->secPoint = DRW_Coord(FromPixel(startPoint.x(), m_varInsunits),
                                           FromPixel(GetSize().height() - startPoint.y(), m_varInsunits), 0);
        m_textBuffer->height = FromPixel(QFontMetrics(f).height(), m_varInsunits);

        m_textBuffer->style = fontStyle;
        m_textBuffer->angle = -rotationAngle;

        m_textBuffer->layer = *layer0;
        m_textBuffer->color = GetPenColor();
        m_textBuffer->lWeight = DRW_LW_Conv::widthByLayer;
        m_textBuffer->lineType = GetPenStyle();
    }

    /* Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark end of each string by adding
     * special placholder. */
    QString t = textItem.text();
    const bool foundEndOfString = t.contains(endStringPlaceholder);

    if (foundEndOfString)
    {
        t.replace(endStringPlaceholder, QString());
    }

    m_textBuffer->text += t.toStdString();

    if (foundEndOfString)
    {
        m_input->AddEntity(m_textBuffer);
        m_textBuffer = new DRW_Text();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::type() const -> QPaintEngine::Type
{
    return QPaintEngine::User;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VDxfEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_UNUSED(r)
    Q_UNUSED(pm)
    Q_UNUSED(sr)
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetSize() const -> QSize
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetSize(const QSize &value)
{
    Q_ASSERT(not isActive());
    m_size = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetResolution() const -> double
{
    return m_resolution;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetResolution(double value)
{
    Q_ASSERT(not isActive());
    m_resolution = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetFileName() const -> QString
{
    return m_fileName;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetFileName(const QString &value)
{
    Q_ASSERT(not isActive());
    m_fileName = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetVersion() const -> DRW::Version
{
    return m_version;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetVersion(DRW::Version version)
{
    Q_ASSERT(not isActive());
    m_version = version;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetBinaryFormat(bool binary)
{
    m_binary = binary;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::IsBinaryFormat() const -> bool
{
    return m_binary;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetPenStyle() -> std::string
{
    switch (state->pen().style())
    {
        case Qt::DashLine:
            return "DASHED";
        case Qt::DotLine:
            return "DOT";
        case Qt::DashDotLine:
            return "DASHDOT2";
        case Qt::DashDotDotLine:
            return "DIVIDE2";
        case Qt::SolidLine:
        default:
            return "BYLAYER";
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetPenColor() -> int
{
    QColor const color = state->pen().color();

    if (color == Qt::black)
    {
        return DRW::black;
    }

    if (color == Qt::white)
    {
        return DRW::white;
    }

    if (color == Qt::darkGray)
    {
        return DRW::gray;
    }

    if (color == Qt::gray)
    {
        return DRW::l_gray;
    }

    if (color == Qt::darkMagenta)
    {
        return DRW::magenta;
    }

    if (color == Qt::magenta)
    {
        return DRW::l_magenta;
    }

    if (color == Qt::cyan)
    {
        return DRW::l_cyan;
    }

    if (color == Qt::darkCyan)
    {
        return DRW::cyan;
    }

    if (color == Qt::blue)
    {
        return DRW::l_blue;
    }

    if (color == Qt::darkBlue)
    {
        return DRW::blue;
    }

    if (color == Qt::darkGreen)
    {
        return DRW::green;
    }

    if (color == Qt::green)
    {
        return DRW::l_green;
    }

    if (color == Qt::darkRed)
    {
        return DRW::red;
    }

    if (color == Qt::red)
    {
        return DRW::l_red;
    }

    if (color == Qt::yellow)
    {
        return DRW::yellow;
    }

    return DRW::ColorByLayer;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetMeasurement(const VarMeasurement &var)
{
    Q_ASSERT(not isActive());
    m_varMeasurement = var;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetInsunits(const VarInsunits &var)
{
    Q_ASSERT(not isActive());
    m_varInsunits = var;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetXScale() const -> qreal
{
    return m_xscale;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetXScale(const qreal &xscale)
{
    Q_ASSERT(not isActive());
    m_xscale = xscale;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetYScale() const -> qreal
{
    return m_yscale;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetYScale(const qreal &yscale)
{
    Q_ASSERT(not isActive());
    m_yscale = yscale;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetBoundaryTogetherWithNotches(bool value)
{
    Q_ASSERT(not isActive());
    m_togetherWithNotches = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::IsBoundaryTogetherWithNotches() const -> bool
{
    return m_togetherWithNotches;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::DxfApparelCompatibility() const -> DXFApparelCompatibility
{
    return m_compatibilityMode;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::SetDxfApparelCompatibility(DXFApparelCompatibility mode)
{
    m_compatibilityMode = mode;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ErrorString() const -> QString
{
    return QString::fromStdString(m_input->ErrorString());
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::FromPixel(double pix, const VarInsunits &unit) const -> double
{
    switch (unit)
    {
        case VarInsunits::Millimeters:
            return pix / m_resolution * 25.4;
        case VarInsunits::Centimeters:
            return pix / m_resolution * 25.4 / 10.0;
        case VarInsunits::Inches:
            return pix / m_resolution;
        default:
            return pix;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ToPixel(double val, const VarInsunits &unit) const -> double
{
    switch (unit)
    {
        case VarInsunits::Millimeters:
            return (val / 25.4) * m_resolution;
        case VarInsunits::Centimeters:
            return ((val * 10.0) / 25.4) * m_resolution;
        case VarInsunits::Inches:
            return val * m_resolution;
        default:
            return val;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ExportToAAMA(const QVector<VLayoutPiece> &details) -> bool
{
    if (not m_size.isValid())
    {
        qCWarning(vDxf) << qUtf8Printable("VDxfEngine::ExportToAAMA(), size is not valid"_L1);
        return false;
    }

    m_input = QSharedPointer<dx_iface>::create(m_fileName.toUtf8().toStdString(),
                                               m_version,
                                               m_varMeasurement,
                                               m_varInsunits);
    m_input->AddXSpaceBlock(false);
    m_input->AddAAMAHeaderData();
    if (m_version > DRW::AC1009)
    {
        m_input->AddDefLayers();
    }
    m_input->AddAAMALayers();

    ExportStyleSystemText(m_input, details);

    for (auto detail : details)
    {
        // Use custom deleter function to lose ownership after adding the block
        bool deleteBlock = true;
        auto NoOpDeleter = [&deleteBlock](dx_ifaceBlock *block)
        {
            if (deleteBlock)
            {
                delete block;
            }
        };

        auto detailBlock = QSharedPointer<dx_ifaceBlock>(new dx_ifaceBlock, NoOpDeleter);

        QString blockName = detail.GetName();
        if (m_version <= DRW::AC1009)
        {
            blockName.replace(' '_L1, '_'_L1);
        }

        detailBlock->name = blockName.toStdString();
        detailBlock->flags = 64;
        detailBlock->layer = *layer1;

        detail.Scale(m_xscale, m_yscale);

        ExportAAMAOutline(detailBlock, detail);
        ExportAAMADraw(detailBlock, detail);
        ExportAAMAIntcut(detailBlock, detail);
        ExportAAMANotch(detailBlock, detail);
        ExportAAMAGrainline(detailBlock, detail);
        ExportPieceText(detailBlock, detail);
        ExportAAMADrill(detailBlock, detail);
        ExportAnnotationText(detailBlock, detail, *layer19);

        m_input->AddBlock(detailBlock.data());

        auto insert = std::make_unique<DRW_Insert>();
        insert->name = blockName.toStdString();
        insert->layer = *layer1;

        m_input->AddEntity(insert.release());

        deleteBlock = false; // lose ownership
    }

    return m_input->fileExport(m_binary);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMAOutline(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<VLayoutPoint> points = PieceOutline(detail);

    if (m_togetherWithNotches)
    {
        points = VAbstractPiece::RemoveDublicates(points, false);
    }

    if (DRW_Entity *e = AAMAPolygon(points, *layer1, true))
    {
        detailBlock->ent.push_back(e);
    }

    ExportTurnPoints(detailBlock, points);
    ExportCurvePoints(detailBlock, points);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADraw(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    ExportAAMADrawSewLine(detailBlock, detail);
    ExportAAMADrawInternalPaths(detailBlock, detail);
    ExportAAMADrawPlaceLabels(detailBlock, detail);
    ExportAAMADrawFoldLine(detailBlock, detail);

    if (!detail.IsShowFullPiece())
    {
        return;
    }

    if (QLineF const mirrorLine = detail.GetMappedCorrectedMirrorLine(m_togetherWithNotches);
        not mirrorLine.isNull() && detail.IsShowMirrorLine())
    {
        if (DRW_Entity *e = AAMALine(mirrorLine, *layer8))
        {
            e->lineType = dx_iface::QtPenStyleToString(Qt::DashDotLine);
            detailBlock->ent.push_back(e);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADrawSewLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (!detail.IsSeamAllowance() || detail.IsHideMainPath() || detail.IsSeamAllowanceBuiltIn())
    {
        return;
    }

    QVector<VLayoutPoint> const points = detail.GetMappedFullContourPoints(m_togetherWithNotches, true, false);
    if (points.isEmpty())
    {
        return;
    }

    if (const UTF8STRING &layer = not detail.IsSewLineOnDrawing() ? *layer14 : *layer8;
        DRW_Entity *e = AAMAPolygon(points, layer, true))
    {
        detailBlock->ent.push_back(e);
    }

    ExportTurnPoints(detailBlock, points);
    ExportCurvePoints(detailBlock, points);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADrawInternalPaths(const QSharedPointer<dx_ifaceBlock> &detailBlock,
                                             const VLayoutPiece &detail)
{
    auto DrawPolygon = [this, detailBlock](const QVector<VLayoutPoint> &points, Qt::PenStyle style, bool forceClosed)
    {
        if (DRW_Entity *e = AAMAPolygon(points, *layer8, forceClosed))
        {
            e->lineType = dx_iface::QtPenStyleToString(style);
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, points);
        ExportCurvePoints(detailBlock, points);
    };

    const QVector<VLayoutPiecePath> drawIPaths = detail.MappedInternalPathsForCut(false);
    for (const auto &iPath : drawIPaths)
    {
        QVector<VLayoutPoint> points = iPath.Points();
        DrawPolygon(points, iPath.PenStyle(), false);

        if (!iPath.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
        {
            const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
            std::transform(points.begin(), points.end(), points.begin(),
                           [&matrix](const VLayoutPoint &point) { return VAbstractPiece::MapPoint(point, matrix); });
            DrawPolygon(points, iPath.PenStyle(), false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADrawPlaceLabels(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    auto DrawShape = [this, detailBlock](const PlaceLabelImg &shape, bool forceClosed)
    {
        for (const auto &points : shape)
        {
            if (DRW_Entity *e = AAMAPolygon(points, *layer8, forceClosed))
            {
                detailBlock->ent.push_back(e);
            }

            ExportTurnPoints(detailBlock, points);
            ExportCurvePoints(detailBlock, points);
        }
    };

    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();
    for (const auto &label : labels)
    {
        if (label.Type() != PlaceLabelType::Doubletree && label.Type() != PlaceLabelType::Button &&
            label.Type() != PlaceLabelType::Circle)
        {
            DrawShape(detail.MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(label)), false);

            if (!label.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
            {
                PlaceLabelImg shape = detail.MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(label));
                const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
                for (auto &points : shape)
                {
                    std::transform(points.begin(), points.end(), points.begin(),
                                   [&matrix](const VLayoutPoint &point)
                                   { return VAbstractPiece::MapPoint(point, matrix); });
                }

                DrawShape(shape, false);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMAIntcut(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    auto DrawPolygon = [this, detailBlock](const QVector<VLayoutPoint> &points, bool forceClosed)
    {
        if (DRW_Entity *e = AAMAPolygon(points, *layer11, forceClosed))
        {
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, points);
        ExportCurvePoints(detailBlock, points);
    };

    QVector<VLayoutPiecePath> const drawIntCut = detail.MappedInternalPathsForCut(true);
    for (const auto &intCut : drawIntCut)
    {
        QVector<VLayoutPoint> points = intCut.Points();
        DrawPolygon(points, false);

        if (!intCut.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
        {
            const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
            std::transform(points.begin(), points.end(), points.begin(),
                           [&matrix](const VLayoutPoint &point) { return VAbstractPiece::MapPoint(point, matrix); });
            DrawPolygon(points, false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMANotch(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    auto ExportNotch = [this, detailBlock](QPointF center, qreal length, qreal angle)
    {
        auto notch = std::make_unique<DRW_ASTMNotch>();

        notch->basePoint =
            DRW_Coord(FromPixel(center.x(), m_varInsunits), FromPixel(GetSize().height() - center.y(), m_varInsunits),
                      FromPixel(length, m_varInsunits));
        notch->angle = angle;
        notch->layer = *layer4;

        detailBlock->ent.push_back(notch.release());
    };

    if (detail.IsSeamAllowance() && !m_togetherWithNotches)
    {
        const QLineF mirrorLine = detail.GetMappedSeamMirrorLine();
        const QVector<VLayoutPassmark> passmarks = detail.GetMappedPassmarks();
        for (const auto &passmark : passmarks)
        {
            ExportNotch(passmark.baseLine.p1(), passmark.baseLine.length(), passmark.baseLine.angle());

            if (!mirrorLine.isNull() && detail.IsShowFullPiece()
                && !IsPointOnLineviaPDP(passmark.baseLine.p1(), mirrorLine.p1(), mirrorLine.p2()))
            {
                const QTransform matrix = VGObject::FlippingMatrix(mirrorLine);
                QLineF const baseLine = matrix.map(passmark.baseLine);
                ExportNotch(baseLine.p1(), baseLine.length(), baseLine.angle());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMAGrainline(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QLineF grainlineMainLine = detail.GetMappedGrainlineMainLine();
    if (not grainlineMainLine.isNull())
    {
        if (DRW_Entity *e = AAMALine(grainlineMainLine, *layer7))
        {
            detailBlock->ent.push_back(e);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportPieceText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<QPointF> const labelShape = detail.GetPieceLabelRect();
    if (labelShape.count() != 4)
    {
        if (m_compatibilityMode == DXFApparelCompatibility::STANDARD)
        {
            qCWarning(vDxf) << qUtf8Printable(
                QApplication::translate("VDxfEngine", "Piece '%1'. Piece System Text is missing.")
                    .arg(detail.GetName()));
        }
        return;
    }

    if (m_compatibilityMode == DXFApparelCompatibility::RPCADV08)
    {
        CheckLabelCompatibilityRPCADV08(detail);
    }
    else if (m_compatibilityMode == DXFApparelCompatibility::RPCADV09)
    {
        CheckLabelCompatibilityRPCADV09(detail);
    }

    const qreal scale = qMin(detail.GetXScale(), detail.GetYScale());
    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = QLineF(labelShape.at(0), labelShape.at(1)).angle();
    qreal dY = 0;

    VTextManager const tm = detail.GetPieceLabelData();
    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), tm.GetFont());

    for (const auto &tl : labelLines)
    {
        const QFont fnt = LineFont(tl, tm.GetFont());
        QFontMetrics const fm(fnt);

        if (dY + fm.height() * scale / 2 > dH)
        {
            break;
        }

        dY += fm.height() * scale / 2;

        const qreal dX = LineAlign(tl, tl.qsText, fm, dW);
        QTransform const lineMatrix = detail.LineMatrix(labelShape.at(0), angle, QPointF(dX, dY), dW);

        QPointF const pos = lineMatrix.map(QPointF());

        auto *textLine = new DRW_Text();
        textLine->basePoint =
            DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
        textLine->secPoint =
            DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
        textLine->height = FromPixel(fm.ascent() * scale / 2, m_varInsunits);
        textLine->layer = *layer1;
        textLine->text = tl.qsText.toStdString();
        textLine->style = m_input->AddFont(fnt);

        QLineF string(0, 0, 100, 0);
        string.setAngle(angle);
        string = lineMatrix.map(string);
        const qreal labelAngle = string.angle();

        if (detail.IsVerticallyFlipped() && detail.IsHorizontallyFlipped())
        {
            textLine->angle = labelAngle + 180;
        }
        else if (detail.IsVerticallyFlipped())
        {
            textLine->angle = -labelAngle;
        }
        else if (detail.IsHorizontallyFlipped())
        {
            textLine->angle = -labelAngle - 180;
        }
        else
        {
            textLine->angle = labelAngle;
        }

        detailBlock->ent.push_back(textLine);

        dY += MmToPixel(1.5);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAnnotationText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail,
                                      const UTF8STRING &layer)
{
    VFoldLine const fLine = detail.FoldLine();
    bool ok = false;
    FoldLabelPosData const labelData = fLine.LabelPosition(ok);

    if (not ok)
    {
        return;
    }

    QFontMetrics const fm(labelData.font);
    QPointF pos = labelData.pos;
    qreal const height = fm.height() * qMin(detail.GetXScale(), detail.GetYScale());
    qreal const width = fm.horizontalAdvance(labelData.label);

    QLineF base(pos, QPointF(pos.x() + 100, pos.y()));
    base.setAngle(base.angle() - 180);
    base.setLength(height);
    pos = base.p2();

    QTransform matrix;
    if ((detail.IsVerticallyFlipped() && !detail.IsHorizontallyFlipped()) ||
        (!detail.IsVerticallyFlipped() && detail.IsHorizontallyFlipped()))
    {
        matrix.translate(pos.x(), pos.y());
        matrix.rotate(180);
        matrix.translate(-pos.x(), -pos.y());
        matrix.translate(-width, fm.height());
    }

    matrix *= detail.GetMatrix();

    QLineF angleLine(QPointF(), QPointF(1000, 0));
    angleLine.setAngle(labelData.angle);
    angleLine = matrix.map(angleLine);

    pos = matrix.map(pos);

    auto *text = new DRW_Text();
    text->basePoint =
        DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    text->secPoint =
        DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    text->height = FromPixel(height / 2, m_varInsunits);
    text->layer = layer;
    text->text = labelData.label.toStdString();
    text->style = m_input->AddFont(labelData.font);
    text->angle = angleLine.angle();

    detailBlock->ent.push_back(text);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportStyleSystemText(const QSharedPointer<dx_iface> &input, const QVector<VLayoutPiece> &details)
{
    for (const auto &detail : details)
    {
        const QStringList strings = detail.GetPatternText();
        if (not strings.isEmpty())
        {
            for (int j = 0; j < strings.size(); ++j)
            {
                const qreal height = ToPixel(AAMATextHeight * m_yscale, m_varInsunits);
                QPointF const pos(0, GetSize().height() - height * (static_cast<int>(strings.size()) - j - 1));
                input->AddEntity(AAMAText(pos, strings.at(j), *layer1));
            }
            return;
        }
    }

    if (m_compatibilityMode == DXFApparelCompatibility::STANDARD)
    {
        // According to ASTM standard Style System Text is mandatory.
        // Some applications may refuse file without Style System Text.
        qCWarning(vDxf) << qUtf8Printable(QApplication::translate("VDxfEngine", "Style System Text is missing."));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADrill(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();

    for (const auto &label : labels)
    {
        if (label.Type() == PlaceLabelType::Doubletree || label.Type() == PlaceLabelType::Button ||
            label.Type() == PlaceLabelType::Circle)
        {
            QPointF center = detail.GetMatrix().map(label.Center());
            detailBlock->ent.push_back(AAMAPoint(center, *layer13));

            if (!label.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
            {
                const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
                center = VAbstractPiece::MapPoint(center, matrix);
                detailBlock->ent.push_back(AAMAPoint(center, *layer13));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADrawFoldLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (detail.IsShowFullPiece() && !detail.IsShowMirrorLine())
    {
        return;
    }

    VFoldLine const fLine = detail.FoldLine();
    QVector<QVector<QPointF>> points = fLine.FoldLineMarkPoints();
    if (points.isEmpty())
    {
        return;
    }

    switch (detail.GetFoldLineType())
    {
        case FoldLineType::TwoArrows:
        case FoldLineType::TwoArrowsTextAbove:
        case FoldLineType::TwoArrowsTextUnder:
            points.removeAt(1);
            AAMADrawFoldLineTwoArrows(points, detailBlock);
            break;
        case FoldLineType::ThreeDots:
            AAMADrawFoldLineThreeDots(points, detailBlock, fLine.ThreeDotsRadius());
            break;
        case FoldLineType::ThreeX:
            AAMADrawFoldLineThreeX(points, detailBlock);
            break;
        case FoldLineType::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        case FoldLineType::Text:
        case FoldLineType::None:
        default:
            break;
    };
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ExportToASTM(const QVector<VLayoutPiece> &details) -> bool
{
    if (not m_size.isValid())
    {
        qCWarning(vDxf) << qUtf8Printable("VDxfEngine::ExportToASTM(), size is not valid"_L1);
        return false;
    }

    m_input = QSharedPointer<dx_iface>(
        new dx_iface(m_fileName.toUtf8().toStdString(), m_version, m_varMeasurement, m_varInsunits));
    m_input->AddXSpaceBlock(false);
    m_input->AddAAMAHeaderData();
    if (m_version > DRW::AC1009)
    {
        m_input->AddDefLayers();
    }
    m_input->AddASTMLayers();

    ExportStyleSystemText(m_input, details);

    for (auto detail : details)
    {
        // Use custom deleter function to lose ownership after adding the block
        bool deleteBlock = true;
        auto NoOpDeleter = [&deleteBlock](dx_ifaceBlock *block)
        {
            if (deleteBlock)
            {
                delete block;
            }
        };
        auto detailBlock = QSharedPointer<dx_ifaceBlock>(new dx_ifaceBlock, NoOpDeleter);

        QString blockName = detail.GetName();
        if (m_version <= DRW::AC1009)
        {
            blockName.replace(' '_L1, '_'_L1);
        }

        detailBlock->name = blockName.toStdString();
        detailBlock->layer = *layer1;

        detail.Scale(m_xscale, m_yscale);

        ExportASTMPieceBoundary(detailBlock, detail);
        ExportASTMSewLine(detailBlock, detail);
        ExportASTMDrawInternalPaths(detailBlock, detail);
        ExportASTMDrawPlaceLabels(detailBlock, detail);
        ExportASTMInternalCutout(detailBlock, detail);
        ExportASTMNotches(detailBlock, detail);
        ExportAAMAGrainline(detailBlock, detail);
        ExportPieceText(detailBlock, detail);
        ExportASTMDrill(detailBlock, detail);
        ExportASTMAnnotationText(detailBlock, detail);
        ExportASTMMirrorLine(detailBlock, detail);
        ExportASTMDrawFoldLine(detailBlock, detail);
        ExportAnnotationText(detailBlock, detail, *layer15);

        m_input->AddBlock(detailBlock.data());

        auto insert = std::make_unique<DRW_Insert>();
        insert->name = blockName.toStdString();
        insert->layer = *layer1;

        m_input->AddEntity(insert.release());

        deleteBlock = false; // lose ownership
    }

    return m_input->fileExport(m_binary);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMPieceBoundary(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<VLayoutPoint> pieceBoundary = PieceOutline(detail);
    if (m_togetherWithNotches)
    {
        pieceBoundary = VAbstractPiece::RemoveDublicates(pieceBoundary, false);
    }

    // Piece boundary
    if (DRW_Entity *e = AAMAPolygon(pieceBoundary, *layer1, true))
    {
        detailBlock->ent.push_back(e);
    }

    ExportTurnPoints(detailBlock, pieceBoundary);
    ExportCurvePoints(detailBlock, pieceBoundary);

    // Piece boundary quality validation curves
    if (DRW_Entity *q = AAMAPolygon(pieceBoundary, *layer84, true))
    {
        detailBlock->ent.push_back(q);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMSewLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (!detail.IsSeamAllowance() || detail.IsHideMainPath() || detail.IsSeamAllowanceBuiltIn())
    {
        return;
    }

    QVector<VLayoutPoint> const sewLine = detail.GetMappedFullContourPoints(m_togetherWithNotches, true, false);

    // Sew lines
    if (DRW_Entity *e = AAMAPolygon(sewLine, *layer14, true))
    {
        detailBlock->ent.push_back(e);
    }

    ExportTurnPoints(detailBlock, sewLine);
    ExportCurvePoints(detailBlock, sewLine);

    // Sew lines quality validation curves
    if (DRW_Entity *e = AAMAPolygon(sewLine, *layer87, true))
    {
        detailBlock->ent.push_back(e);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMDrawInternalPaths(const QSharedPointer<dx_ifaceBlock> &detailBlock,
                                             const VLayoutPiece &detail)
{
    auto DrawPolygon =
        [this, detailBlock](const QVector<VLayoutPoint> &points, Qt::PenStyle style, bool notMirrored, bool forceClosed)
    {
        // Internal line
        if (DRW_Entity *e = AAMAPolygon(points, *layer8, forceClosed))
        {
            e->lineType = dx_iface::QtPenStyleToString(style);
            detailBlock->ent.push_back(e);
        }

        if (notMirrored && !points.isEmpty())
        {
            QPointF const pos(points.constFirst().x(),
                              points.constFirst().y() - ToPixel(AAMATextHeight, m_varInsunits));
            detailBlock->ent.push_back(AAMAText(pos, QStringLiteral("NM"), *layer8));
        }

        ExportTurnPoints(detailBlock, points);
        ExportCurvePoints(detailBlock, points);

        // Internal lines quality validation curves
        if (DRW_Entity *e = AAMAPolygon(points, *layer85, forceClosed))
        {
            detailBlock->ent.push_back(e);
        }
    };

    const QVector<VLayoutPiecePath> drawIPaths = detail.MappedInternalPathsForCut(false);
    for (const auto &iPath : drawIPaths)
    {
        QVector<VLayoutPoint> points = iPath.Points();
        DrawPolygon(points, iPath.PenStyle(), iPath.IsNotMirrored(), false);

        if (!iPath.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
        {
            const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
            std::transform(points.begin(), points.end(), points.begin(),
                           [&matrix](const VLayoutPoint &point) { return VAbstractPiece::MapPoint(point, matrix); });
            DrawPolygon(points, iPath.PenStyle(), iPath.IsNotMirrored(), false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMDrawPlaceLabels(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    auto DrawShape = [this, detailBlock](const PlaceLabelImg &shape, bool notMirrored, bool forceClosed)
    {
        for (const auto &points : shape)
        {
            // Internal line (placelabel)
            if (DRW_Entity *e = AAMAPolygon(points, *layer8, forceClosed))
            {
                detailBlock->ent.push_back(e);
            }

            if (notMirrored && !points.isEmpty())
            {
                QPointF const pos(points.constFirst().x(),
                                  points.constFirst().y() - ToPixel(AAMATextHeight, m_varInsunits));
                detailBlock->ent.push_back(AAMAText(pos, QStringLiteral("NM"), *layer8));
            }

            ExportTurnPoints(detailBlock, points);
            ExportCurvePoints(detailBlock, points);

            // Internal lines quality validation curves
            if (DRW_Entity *e = AAMAPolygon(points, *layer85, false))
            {
                detailBlock->ent.push_back(e);
            }
        }
    };

    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();
    for (const auto &label : labels)
    {
        if (label.Type() != PlaceLabelType::Doubletree && label.Type() != PlaceLabelType::Button &&
            label.Type() != PlaceLabelType::Circle)
        {
            PlaceLabelImg shape = detail.MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(label));
            DrawShape(shape, label.IsNotMirrored(), false);

            if (!label.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
            {
                const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
                for (auto &points : shape)
                {
                    std::transform(points.begin(), points.end(), points.begin(),
                                   [&matrix](const VLayoutPoint &point)
                                   { return VAbstractPiece::MapPoint(point, matrix); });
                }

                DrawShape(shape, label.IsNotMirrored(), false);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMInternalCutout(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    auto DrawPolygon = [this, detailBlock](const QVector<VLayoutPoint> &points, bool notMirrored, bool forceClosed)
    {
        // Internal cutout
        if (DRW_Entity *e = AAMAPolygon(points, *layer11, forceClosed))
        {
            detailBlock->ent.push_back(e);
        }

        if (notMirrored && !points.isEmpty())
        {
            QPointF const pos(points.constFirst().x(),
                              points.constFirst().y() - ToPixel(AAMATextHeight, m_varInsunits));
            detailBlock->ent.push_back(AAMAText(pos, QStringLiteral("NM"), *layer11));
        }

        ExportTurnPoints(detailBlock, points);
        ExportCurvePoints(detailBlock, points);

        // Internal cutouts quality validation curves
        if (DRW_Entity *e = AAMAPolygon(points, *layer86, forceClosed))
        {
            detailBlock->ent.push_back(e);
        }
    };

    QVector<VLayoutPiecePath> const drawIntCut = detail.MappedInternalPathsForCut(true);
    for (const auto &intCut : drawIntCut)
    {
        QVector<VLayoutPoint> points = intCut.Points();
        DrawPolygon(points, intCut.IsNotMirrored(), false);

        if (!intCut.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
        {
            const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
            std::transform(points.begin(), points.end(), points.begin(),
                           [&matrix](const VLayoutPoint &point) { return VAbstractPiece::MapPoint(point, matrix); });
            DrawPolygon(points, intCut.IsNotMirrored(), false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMAnnotationText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QString const name = detail.GetName();
    QPointF const textPos = detail.VLayoutPiece::MappedDetailBoundingRect().center();

    QPointF const pos(textPos.x(), textPos.y() - ToPixel(AAMATextHeight, m_varInsunits));
    detailBlock->ent.push_back(AAMAText(pos, name, *layer15));
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMDrill(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();

    auto ExportPoint = [this, detailBlock](QPointF center, qreal diameter)
    {
        auto point = std::make_unique<DRW_Point>();
        point->basePoint =
            DRW_Coord(FromPixel(center.x(), m_varInsunits), FromPixel(GetSize().height() - center.y(), m_varInsunits),
                      FromPixel(diameter, m_varInsunits));
        point->layer = *layer13;
        detailBlock->ent.push_back(point.release());

        // TODO. Investigate drill category
        // QPointF pos(center.x(), center.y() - ToPixel(AAMATextHeight, m_varInsunits));
        // detailBlock->ent.push_back(AAMAText(pos, category, *layer13));
    };

    for (const auto &label : labels)
    {
        if (label.Type() == PlaceLabelType::Doubletree || label.Type() == PlaceLabelType::Button ||
            label.Type() == PlaceLabelType::Circle)
        {
            QPointF center = detail.GetMatrix().map(label.Center());
            const QLineF diameter(label.Box().bottomLeft(), label.Box().topRight());

            ExportPoint(center, diameter.length());

            if (!label.IsNotMirrored() && detail.IsShowFullPiece() && !detail.GetMappedSeamMirrorLine().isNull())
            {
                const QTransform matrix = VGObject::FlippingMatrix(detail.GetMappedSeamMirrorLine());
                center = VAbstractPiece::MapPoint(center, matrix);
                ExportPoint(center, diameter.length());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMNotches(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (!detail.IsSeamAllowance() || m_togetherWithNotches)
    {
        return;
    }

    auto ExportPassmark = [this, detailBlock, &detail](const VLayoutPassmark &passmark)
    {
        DRW_ASTMNotch *notch = ExportASTMNotch(passmark);
        DRW_ATTDEF *attdef = ExportASTMNotchDataDependecy(passmark, notch->layer, detail);
        detailBlock->ent.push_back(notch);

        if (attdef != nullptr)
        {
            detailBlock->ent.push_back(attdef);
        }
    };

    const QVector<VLayoutPassmark> passmarks = detail.GetMappedPassmarks();
    for (const auto &passmark : passmarks)
    {
        ExportPassmark(passmark);

        const QLineF mirrorLine = detail.GetMappedSeamMirrorLine();
        if (!mirrorLine.isNull() && detail.IsShowFullPiece()
            && !IsPointOnLineviaPDP(passmark.baseLine.p1(), mirrorLine.p1(), mirrorLine.p2()))
        {
            const QTransform matrix = VGObject::FlippingMatrix(mirrorLine);
            const VLayoutPassmark mirroredPassmark = VLayoutPiece::MapPassmark(passmark, matrix, false);

            ExportPassmark(mirroredPassmark);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMMirrorLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (detail.IsShowFullPiece())
    {
        return;
    }

    QLineF const mirrorLine = detail.GetMappedCorrectedMirrorLine(m_togetherWithNotches);
    if (mirrorLine.isNull())
    {
        return;
    }

    const bool isShowFullPiece = detail.IsShowFullPiece();
    const auto layer = isShowFullPiece ? *layer8 : *layer6;

    if (DRW_Entity *e = AAMALine(mirrorLine, layer))
    {
        if (isShowFullPiece && detail.IsShowMirrorLine())
        {
            e->lineType = dx_iface::QtPenStyleToString(Qt::DashDotLine);
        }
        detailBlock->ent.push_back(e);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMDrawFoldLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (detail.IsShowFullPiece() && !detail.IsShowMirrorLine())
    {
        return;
    }

    VFoldLine const fLine = detail.FoldLine();
    QVector<QVector<QPointF>> points = fLine.FoldLineMarkPoints();
    if (points.isEmpty())
    {
        return;
    }

    switch (detail.GetFoldLineType())
    {
        case FoldLineType::TwoArrows:
        case FoldLineType::TwoArrowsTextAbove:
        case FoldLineType::TwoArrowsTextUnder:
            points.removeAt(1);
            ASTMDrawFoldLineTwoArrows(points, detailBlock);
            break;
        case FoldLineType::ThreeDots:
            AAMADrawFoldLineThreeDots(points, detailBlock, fLine.ThreeDotsRadius());
            break;
        case FoldLineType::ThreeX:
            AAMADrawFoldLineThreeX(points, detailBlock);
            break;
        case FoldLineType::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        case FoldLineType::Text:
        case FoldLineType::None:
        default:
            break;
    };
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ExportASTMNotch(const VLayoutPassmark &passmark) -> DRW_ASTMNotch *
{
    auto *notch = new DRW_ASTMNotch();
    const QPointF center = passmark.baseLine.p1();

    notch->basePoint =
        DRW_Coord(FromPixel(center.x(), m_varInsunits), FromPixel(GetSize().height() - center.y(), m_varInsunits),
                  FromPixel(passmark.baseLine.length(), m_varInsunits));

    notch->angle = passmark.baseLine.angle();

    PassmarkLineType type = passmark.type;
    if ((m_compatibilityMode == DXFApparelCompatibility::RPCADV08 ||
         m_compatibilityMode == DXFApparelCompatibility::RPCADV09 ||
         m_compatibilityMode == DXFApparelCompatibility::RPCADV10) &&
        (type == PassmarkLineType::ExternalVMark || type == PassmarkLineType::InternalVMark))
    {
        type = PassmarkLineType::CheckMark;
    }

    switch (type)
    {
        case PassmarkLineType::OneLine:
        case PassmarkLineType::TwoLines:
        case PassmarkLineType::ThreeLines:
            // Slit notch
            notch->layer = *layer4;
            break;
        case PassmarkLineType::ExternalVMark:
        case PassmarkLineType::InternalVMark:
        { // V-Notch
            const QLineF boundaryLine(passmark.lines.constFirst().p1(), passmark.lines.constLast().p2());
            notch->thickness = FromPixel(boundaryLine.length(), m_varInsunits); // width
            notch->layer = *layer4;
            break;
        }
        case PassmarkLineType::TMark:
            // T-Notch
            notch->thickness = FromPixel(passmark.lines.constLast().length(), m_varInsunits); // width
            notch->layer = *layer80;
            break;
        case PassmarkLineType::BoxMark:
        { // Castle Notch
            QPointF const start = passmark.lines.constFirst().p1();
            QPointF const end = passmark.lines.constLast().p2();

            notch->layer = *layer81;
            notch->thickness = FromPixel(QLineF(start, end).length(), m_varInsunits); // width
            break;
        }
        case PassmarkLineType::UMark:
        { // U-Notch
            QPointF const start = passmark.lines.constFirst().p1();
            QPointF const end = passmark.lines.constLast().p2();

            notch->thickness = FromPixel(QLineF(start, end).length(), m_varInsunits); // width

            notch->layer = *layer83;
            break;
        }
        case PassmarkLineType::CheckMark:
        { // Check Notch
            const QLineF &line1 = passmark.lines.constFirst();
            const QLineF &line2 = passmark.lines.constLast();

            qreal width = QLineF(line1.p1(), line2.p2()).length();

            if (not passmark.isClockwiseOpening)
            { // a counter clockwise opening
                width *= -1;
            }

            notch->thickness = FromPixel(width, m_varInsunits); // width
            notch->layer = *layer82;
            break;
        }
        case PassmarkLineType::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        default:
            break;
    };

    return notch;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ExportASTMNotchDataDependecy(const VLayoutPassmark &passmark, const UTF8STRING &notchLayer,
                                              const VLayoutPiece &detail) -> DRW_ATTDEF *
{
    QVector<VLayoutPoint> const boundary = not detail.IsSeamAllowanceBuiltIn() && !passmark.isBuiltIn
                                               ? detail.GetMappedSeamAllowancePoints()
                                               : detail.GetMappedContourPoints();

    const QPointF center = passmark.baseLine.p1();
    QPointF referencePoint;
    if (not NotchPrecedingPoint(boundary, center, referencePoint))
    {
        return nullptr;
    }

    auto *attdef = new DRW_ATTDEF();
    attdef->layer = not detail.IsSeamAllowanceBuiltIn() && !passmark.isBuiltIn ? *layer1 : *layer14;
    attdef->basePoint = DRW_Coord(FromPixel(referencePoint.x(), m_varInsunits),
                                  FromPixel(GetSize().height() - referencePoint.y(), m_varInsunits), 0);
    attdef->adjustmentPoint =
        DRW_Coord(FromPixel(center.x(), m_varInsunits), FromPixel(GetSize().height() - center.y(), m_varInsunits), 0);
    attdef->height = 3.0;
    attdef->text = "Link:" + notchLayer;
    attdef->name = "Dependency";
    attdef->flags |= 0x2;             // this is a constant attribute
    attdef->horizontalAdjustment = 3; // aligned (if vertical alignment = 0)

    return attdef;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportTurnPoints(const QSharedPointer<dx_ifaceBlock> &detailBlock,
                                  const QVector<VLayoutPoint> &points) const
{
    for (const auto &p : std::as_const(points))
    {
        if (p.TurnPoint())
        {
            detailBlock->ent.push_back(AAMAPoint(p, *layer2));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportCurvePoints(const QSharedPointer<dx_ifaceBlock> &detailBlock,
                                   const QVector<VLayoutPoint> &points) const
{
    for (const auto &p : std::as_const(points))
    {
        if (p.CurvePoint() && not p.TurnPoint())
        {
            detailBlock->ent.push_back(AAMAPoint(p, *layer3));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMAPolygon(const QVector<VLayoutPoint> &polygon, const UTF8STRING &layer, bool forceClosed)
    -> DRW_Entity *
{
    if (polygon.isEmpty())
    {
        return nullptr;
    }

    if (m_version > DRW::AC1009)
    { // Use lwpolyline
        return CreateAAMAPolygon<DRW_LWPolyline, DRW_Vertex2D>(polygon, layer, forceClosed);
    }

    // Use polyline
    return CreateAAMAPolygon<DRW_Polyline, DRW_Vertex>(polygon, layer, forceClosed);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMALine(const QLineF &line, const UTF8STRING &layer) -> DRW_Entity *
{
    auto *lineEnt = new DRW_Line();
    lineEnt->basePoint = DRW_Coord(FromPixel(line.p1().x(), m_varInsunits),
                                   FromPixel(GetSize().height() - line.p1().y(), m_varInsunits), 0);
    lineEnt->secPoint = DRW_Coord(FromPixel(line.p2().x(), m_varInsunits),
                                  FromPixel(GetSize().height() - line.p2().y(), m_varInsunits), 0);
    lineEnt->layer = layer;

    return lineEnt;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMAText(const QPointF &pos, const QString &text, const UTF8STRING &layer) -> DRW_Entity *
{
    auto *textLine = new DRW_Text();
    textLine->basePoint =
        DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    textLine->secPoint =
        DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    textLine->height = AAMATextHeight;
    textLine->layer = layer;
    textLine->text = text.toStdString();

    return textLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMAPoint(const QPointF &pos, const UTF8STRING &layer) const -> DRW_Point *
{
    auto *point = new DRW_Point();
    point->basePoint =
        DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    point->layer = layer;
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMACircle(const QPointF &pos, const std::string &layer, qreal radius) const -> DRW_Circle *
{
    auto *circle = new DRW_Circle();
    circle->basePoint =
        DRW_Coord(FromPixel(pos.x(), m_varInsunits), FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    circle->layer = layer;
    circle->radious = FromPixel(radius, m_varInsunits);
    return circle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::NotchPrecedingPoint(const QVector<VLayoutPoint> &boundary, QPointF notchBase, QPointF &point) -> bool
{
    if (boundary.count() < 2)
    {
        return false;
    }

    if (VFuzzyComparePoints(boundary.constFirst(), notchBase))
    {
        point = boundary.constFirst().ToQPointF();
        return true;
    }

    if (VFuzzyComparePoints(boundary.constLast(), notchBase))
    {
        point = boundary.constLast().ToQPointF();
        return true;
    }

    QPointF candidatePoint;
    qreal bestDistance = INT_MAX;
    bool found = false;

    for (qint32 i = 0; i < boundary.count() - 1; ++i)
    {
        const QPointF cPoint = VGObject::ClosestPoint(QLineF(boundary.at(i), boundary.at(i + 1)), notchBase);

        if (IsPointOnLineSegment(cPoint, boundary.at(i), boundary.at(i + 1)))
        {
            const qreal length = QLineF(notchBase, cPoint).length();
            if (length < bestDistance)
            {
                candidatePoint = boundary.at(i).ToQPointF();
                bestDistance = length;
                found = true;
            }
        }
    }

    if (found)
    {
        point = candidatePoint;
        return true;
    }

    return found;
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::AAMADrawFoldLineTwoArrows(const QVector<QVector<QPointF>> &points,
                                           const QSharedPointer<dx_ifaceBlock> &detailBlock)
{
    QVector<VLayoutPoint> shape;
    for (const auto &subShape : points)
    {
        for (const auto &point : subShape)
        {
            VLayoutPoint p(point);
            p.SetTurnPoint(true);
            shape.append(p);
        }
    }

    if (DRW_Entity *e = AAMAPolygon(shape, *layer8, false))
    {
        detailBlock->ent.push_back(e);
    }

    ExportTurnPoints(detailBlock, shape);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::AAMADrawFoldLineThreeDots(const QVector<QVector<QPointF>> &points,
                                           const QSharedPointer<dx_ifaceBlock> &detailBlock, qreal radius)
{
    if (points.isEmpty() || points.constFirst().size() != 3)
    {
        return;
    }

    QVector<QPointF> const &shape = points.constFirst();
    for (const auto &center : shape)
    {
        if (DRW_Entity *e = AAMACircle(center, *layer8, radius))
        {
            detailBlock->ent.push_back(e);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::AAMADrawFoldLineThreeX(const QVector<QVector<QPointF>> &points,
                                        const QSharedPointer<dx_ifaceBlock> &detailBlock)
{
    if (points.isEmpty())
    {
        return;
    }

    QVector<QPointF> const &shape = points.constFirst();
    for (int i = 0; i < shape.size() - 1; i += 2)
    {
        if (DRW_Entity *e = AAMALine(QLineF(shape.at(i), shape.at(i + 1)), *layer8))
        {
            detailBlock->ent.push_back(e);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ASTMDrawFoldLineTwoArrows(const QVector<QVector<QPointF>> &points,
                                           const QSharedPointer<dx_ifaceBlock> &detailBlock)
{
    QVector<VLayoutPoint> shape;
    for (const auto &subShape : points)
    {
        for (const auto &point : subShape)
        {
            VLayoutPoint p(point);
            p.SetTurnPoint(true);
            shape.append(p);
        }
    }

    if (DRW_Entity *e = AAMAPolygon(shape, *layer8, false))
    {
        detailBlock->ent.push_back(e);
    }

    if (!shape.isEmpty())
    {
        QPointF const pos(shape.constFirst().x(), shape.constFirst().y() - ToPixel(AAMATextHeight, m_varInsunits));
        detailBlock->ent.push_back(AAMAText(pos, QStringLiteral("NM"), *layer8));
    }

    ExportTurnPoints(detailBlock, shape);

    if (DRW_Entity *e = AAMAPolygon(shape, *layer85, false))
    {
        detailBlock->ent.push_back(e);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::CheckLabelCompatibilityRPCADV09(const VLayoutPiece &detail)
{
    // According to ASTM standard Piece System Text is mandatory.
    // Richpeace CAD V9. At least 'Piece Name:' or 'Size:' identifiers must be present.
    const QStringList strings = detail.GetPieceText();
    bool pieceNameFound = false;
    bool sizeFound = false;

    for (const QString &line : strings)
    {
        if (line.startsWith("Piece Name:"_L1))
        {
            pieceNameFound = true;
        }
        else if (line.startsWith("Size:"_L1))
        {
            sizeFound = true;
        }

        // Break the loop early if both conditions are met
        if (pieceNameFound || sizeFound)
        {
            break;
        }
    }

    if (!pieceNameFound && !sizeFound)
    {
        qCWarning(vDxf) << qUtf8Printable(
            QApplication::translate("VDxfEngine", "Piece '%1'. 'Piece Name:' or 'Size:' identifier is missing.")
                .arg(detail.GetName()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::CheckLabelCompatibilityRPCADV08(const VLayoutPiece &detail)
{
    // According to ASTM standard Piece System Text is mandatory.
    // Richpeace CAD V8. If present 'Piece Name:' identifier, 'Size:' identifier must also be present.
    const QStringList strings = detail.GetPieceText();
    bool pieceNameFound = false;
    bool sizeFound = false;

    for (const QString &line : strings)
    {
        if (line.startsWith("Piece Name:"_L1))
        {
            pieceNameFound = true;
        }
        else if (line.startsWith("Size:"_L1))
        {
            sizeFound = true;
        }

        // Break the loop early if both conditions are met
        if (pieceNameFound && sizeFound)
        {
            break;
        }
    }

    if (pieceNameFound && !sizeFound)
    {
        qCWarning(vDxf) << qUtf8Printable(
            QApplication::translate("VDxfEngine",
                                    "Piece '%1'. 'Piece Name:' identifier requires 'Size:' identifier to be present.")
                .arg(detail.GetName()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class P, class V, class C>
auto VDxfEngine::CreateAAMAPolygon(const QVector<C> &polygon, const UTF8STRING &layer, bool forceClosed) -> P *
{
    auto *poly = new P();
    poly->layer = layer;

    if (forceClosed)
    {
        poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
    }
    else
    {
        if (polygon.size() > 1 && polygon.constFirst() == polygon.constLast())
        {
            poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
        }
    }

    for (const auto &p : polygon)
    {
        V vertex(FromPixel(p.x(), m_varInsunits), FromPixel(GetSize().height() - p.y(), m_varInsunits));

        if constexpr (std::is_same_v<V, DRW_Vertex>)
        {
            if (p.CurvePoint())
            {
                vertex.flags |= 0x1;
            }
        }

        poly->addVertex(vertex);
    }

    return poly;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::PieceOutline(const VLayoutPiece &detail) const -> QVector<VLayoutPoint>
{
    if (detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn())
    {
        return detail.GetMappedFullSeamAllowancePoints(m_togetherWithNotches);
    }
    return detail.GetMappedFullContourPoints(m_togetherWithNotches);
}
