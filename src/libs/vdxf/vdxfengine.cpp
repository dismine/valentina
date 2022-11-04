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

#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QFlag>
#include <QFlags>
#include <QFont>
#include <QLineF>
#include <QList>
#include <QMessageLogger>
#include <QPaintEngineState>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QTextCodec>
#include <QTextItem>
#include <Qt>
#include <QtDebug>
#include <QtMath>

#include "../vmisc/def.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "dxiface.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpoint.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vlayoutplacelabel.h"

static const qreal AAMATextHeight = 2.5;

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer0, (UTF8STRING("0"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer1, (UTF8STRING("1"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer2, (UTF8STRING("2"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer3, (UTF8STRING("3"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer4, (UTF8STRING("4"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer5, (UTF8STRING("5"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer6, (UTF8STRING("6"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer7, (UTF8STRING("7"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer8, (UTF8STRING("8"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer9, (UTF8STRING("9"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer10, (UTF8STRING("10"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer11, (UTF8STRING("11"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer12, (UTF8STRING("12"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer13, (UTF8STRING("13"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer14, (UTF8STRING("14"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer15, (UTF8STRING("15"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer19, (UTF8STRING("19"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer26, (UTF8STRING("26"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer80, (UTF8STRING("80"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer81, (UTF8STRING("81"))) // NOLINT
//Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer82, (UTF8STRING("82"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer83, (UTF8STRING("83"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer84, (UTF8STRING("84"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer85, (UTF8STRING("85"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer86, (UTF8STRING("86"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const UTF8STRING, layer87, (UTF8STRING("87"))) // NOLINT

//---------------------------------------------------------------------------------------------------------------------
auto PieceOutline(const VLayoutPiece &detail) -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> outline;
    if (detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn())
    {
        outline = detail.GetMappedSeamAllowancePoints();
    }
    else
    {
        outline = detail.GetMappedContourPoints();
    }
    return outline;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
static inline auto svgEngineFeatures() -> QPaintEngine::PaintEngineFeatures
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wsign-conversion")
QT_WARNING_DISABLE_INTEL(68)

    return {QPaintEngine::AllFeatures
        & ~QPaintEngine::PatternBrush
        & ~QPaintEngine::PerspectiveTransform
        & ~QPaintEngine::ConicalGradientFill
        & ~QPaintEngine::PorterDuff};

QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
VDxfEngine::VDxfEngine()
    :QPaintEngine(svgEngineFeatures()),
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
        qWarning("VDxfEngine::begin(), the engine was alredy activated");
        return false;
    }

    if (not m_size.isValid())
    {
        qWarning()<<"VDxfEngine::begin(), size is not valid";
        return false;
    }

    m_input = QSharedPointer<dx_iface>(new dx_iface(GetFileNameForLocale(), m_version, m_varMeasurement,
                                                    m_varInsunits));
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

    for (const auto& polygon : subpaths)
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

            if (polygon.size() > 1 && ConstFirst<QPointF>(polygon) == ConstLast<QPointF>(polygon))
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
            if (polygon.size() > 1 && ConstFirst<QPointF>(polygon) == ConstLast<QPointF>(polygon))
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
        line->basePoint = DRW_Coord(FromPixel(p1.x(), m_varInsunits),
                                    FromPixel(GetSize().height() - p1.y(), m_varInsunits), 0);
        line->secPoint =  DRW_Coord(FromPixel(p2.x(), m_varInsunits),
                                    FromPixel(GetSize().height() - p2.y(), m_varInsunits), 0);
        line->layer = *layer0;
        line->color = GetPenColor();
        line->lWeight = DRW_LW_Conv::widthByLayer;
        line->lineType = GetPenStyle();

        m_input->AddEntity(line);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawLines(const QLine * lines, int lineCount)
{
    QPaintEngine::drawLines(lines, lineCount);
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

        if (pointCount > 1 && points[0] == points[pointCount]) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        {
            poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
        }

        poly->flags |= 0x80; // plinegen NOLINT(hicpp-signed-bitwise)

        for (int i = 0; i < pointCount; ++i)
        {
            const QPointF p = m_matrix.map(points[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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

        if (pointCount > 1 && points[0] == points[pointCount]) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
void VDxfEngine::drawPolygon(const QPoint *points, int pointCount, QPaintEngine::PolygonDrawMode mode)
{
    QPaintEngine::drawPolygon(points, pointCount, mode);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawEllipse(const QRectF & rect)
{
    const QRectF newRect = m_matrix.mapRect(rect);
    const double rotationAngle = atan(m_matrix.m12()/m_matrix.m11());

    double majorX, majorY; // distanse between center and endpoint of the major axis
    double ratio; // ratio of minor axis to major axis
    if(rect.width()<= rect.height())
    {
        majorX = (rect.top() - rect.center().y())*sin(rotationAngle)*m_matrix.m11()/cos(rotationAngle);
        // major axis * sin(rotation angle) * x-scale-factor
        majorY = (rect.top() - rect.center().y())*m_matrix.m22();
        // major axis * cos(rotation angle) * y-scale-factor, where y-scale-factor = matrix.m22()/cos(rotationAngle)
        ratio  = rect.width()/rect.height();
    }
    else
    {
        majorX = (rect.right() - rect.center().x())*m_matrix.m11();
        // major axis * cos(rotation angle) * x-scale-factor, where y-scale-factor = matrix.m22()/cos(rotationAngle)
        majorY = (rect.right() - rect.center().x())*sin(rotationAngle)*m_matrix.m22()/cos(rotationAngle);
        // major axis * sin(rotation angle) * y-scale-factor
        ratio  = rect.height()/rect.width();
    }

    auto *ellipse = new DRW_Ellipse();
    ellipse->basePoint = DRW_Coord(FromPixel(newRect.center().x(), m_varInsunits),
                                   FromPixel(GetSize().height() - newRect.center().y(), m_varInsunits), 0);
    ellipse->secPoint = DRW_Coord(FromPixel(majorX, m_varInsunits), FromPixel(majorY, m_varInsunits), 0);
    ellipse->ratio = ratio;
    ellipse->staparam = 0;
    ellipse->endparam = 2*M_PI;

    ellipse->layer = *layer0;
    ellipse->color = GetPenColor();
    ellipse->lWeight = DRW_LW_Conv::widthByLayer;
    ellipse->lineType = GetPenStyle();

    m_input->AddEntity(ellipse);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawEllipse(const QRect & rect)
{
    QPaintEngine::drawEllipse(rect);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::drawTextItem(const QPointF & p, const QTextItem & textItem)
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
    QColor color = state->pen().color();

    if(color == Qt::black)
    {
        return DRW::black;
    }

    if(color == Qt::white)
    {
        return DRW::white;
    }

    if(color == Qt::darkGray)
    {
        return DRW::gray;
    }

    if(color == Qt::gray)
    {
        return DRW::l_gray;
    }

    if(color == Qt::darkMagenta)
    {
        return DRW::magenta;
    }

    if(color == Qt::magenta)
    {
        return DRW::l_magenta;
    }

    if(color == Qt::cyan)
    {
        return DRW::l_cyan;
    }

    if(color == Qt::darkCyan)
    {
        return DRW::cyan;
    }

    if(color == Qt::blue)
    {
        return DRW::l_blue;
    }

    if(color == Qt::darkBlue)
    {
        return DRW::blue;
    }

    if(color == Qt::darkGreen)
    {
        return DRW::green;
    }

    if(color == Qt::green)
    {
        return DRW::l_green;
    }

    if(color == Qt::darkRed)
    {
        return DRW::red;
    }

    if(color == Qt::red)
    {
        return DRW::l_red;
    }

    if(color == Qt::yellow)
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
    m_yscale = yscale;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ErrorString() const -> QString
{
    return QString::fromStdString(m_input->ErrorString());
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

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
    }
    return pix;
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
    }
    return val;
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ExportToAAMA(const QVector<VLayoutPiece> &details) -> bool
{
    if (not m_size.isValid())
    {
        qWarning()<<"VDxfEngine::begin(), size is not valid";
        return false;
    }

    m_input = QSharedPointer<dx_iface>::create(GetFileNameForLocale(), m_version, m_varMeasurement, m_varInsunits);
    m_input->AddAAMAHeaderData();
    if (m_version > DRW::AC1009)
    {
        m_input->AddDefLayers();
    }
    m_input->AddAAMALayers();

    ExportStyleSystemText(m_input, details);

    for(auto detail : details)
    {

        auto detailBlock = QSharedPointer<dx_ifaceBlock>::create();

        QString blockName = detail.GetName();
        if (m_version <= DRW::AC1009)
        {
            blockName.replace(' ', '_');
        }

        detailBlock->name = blockName.toStdString();
        detailBlock->layer = *layer1;

        detail.Scale(m_xscale, m_yscale);

        ExportAAMAOutline(detailBlock, detail);
        ExportAAMADraw(detailBlock, detail);
        ExportAAMAIntcut(detailBlock, detail);
        ExportAAMANotch(detailBlock, detail);
        ExportAAMAGrainline(detailBlock, detail);
        ExportPieceText(detailBlock, detail);
        ExportAAMADrill(detailBlock, detail);

        m_input->AddBlock(detailBlock.get());

        QScopedPointer<DRW_Insert> insert(new DRW_Insert());
        insert->name = blockName.toStdString();
        insert->layer = *layer1;

        m_input->AddEntity(insert.take());
    }

    return m_input->fileExport(m_binary);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMAOutline(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<VLayoutPoint> points = PieceOutline(detail);

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
    if (detail.IsSeamAllowance() && not detail.IsHideMainPath() && not detail.IsSeamAllowanceBuiltIn())
    {
        QVector<VLayoutPoint> points = detail.GetMappedContourPoints();
        const UTF8STRING &layer = not detail.IsSewLineOnDrawing() ? *layer14 : *layer8;

        if (DRW_Entity *e = AAMAPolygon(points, layer, true))
        {
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, points);
        ExportCurvePoints(detailBlock, points);
    }

    const QVector<QVector<VLayoutPoint>> drawIntLine = detail.MappedInternalPathsForCut(false);
    for(const auto &intLine : drawIntLine)
    {
        if (DRW_Entity *e = AAMAPolygon(intLine, *layer8, false))
        {
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, intLine);
        ExportCurvePoints(detailBlock, intLine);
    }

    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();
    for(const auto &label : labels)
    {
        if (label.Type() != PlaceLabelType::Doubletree && label.Type() != PlaceLabelType::Button
                && label.Type() != PlaceLabelType::Circle)
        {
            PlaceLabelImg shape = detail.MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(label));
            for(const auto &points : shape)
            {
                if (DRW_Entity *e = AAMAPolygon(points, *layer8, false))
                {
                    detailBlock->ent.push_back(e);
                }

                ExportTurnPoints(detailBlock, points);
                ExportCurvePoints(detailBlock, points);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMAIntcut(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<QVector<VLayoutPoint>> drawIntCut = detail.MappedInternalPathsForCut(true);
    for(auto &intCut : drawIntCut)
    {
        if (DRW_Entity *e = AAMAPolygon(intCut, *layer11, false))
        {
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, intCut);
        ExportCurvePoints(detailBlock, intCut);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMANotch(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (detail.IsSeamAllowance())
    {
        const QVector<VLayoutPassmark> passmarks = detail.GetMappedPassmarks();
        for(const auto &passmark : passmarks)
        {
            for (const auto &line : passmark.lines)
            {
                if (DRW_Entity *e = AAMALine(line, *layer4))
                {
                    detailBlock->ent.push_back(e);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMAGrainline(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QVector<QPointF> grainline = detail.GetMappedGrainline();
    if (grainline.count() > 1)
    {
        if (DRW_Entity *e = AAMALine(QLineF(ConstFirst(grainline), ConstLast(grainline)), *layer7))
        {
            detailBlock->ent.push_back(e);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportPieceText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QStringList list = detail.GetPieceText();
    const QPointF startPos = detail.GetPieceTextPosition();

    for (int i = 0; i < list.size(); ++i)
    {
        QPointF pos(startPos.x(), startPos.y() - ToPixel(AAMATextHeight * m_yscale, m_varInsunits)*(list.size() - i-1));
        detailBlock->ent.push_back(AAMAText(pos, list.at(i), *layer1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportStyleSystemText(const QSharedPointer<dx_iface> &input, const QVector<VLayoutPiece> &details)
{
    for(const auto &detail : details)
    {
        const QStringList strings = detail.GetPatternText();
        if (not strings.isEmpty())
        {
            for (int j = 0; j < strings.size(); ++j)
            {
                QPointF pos(0, GetSize().height() -
                                   ToPixel(AAMATextHeight * m_yscale, m_varInsunits)*(strings.size() - j-1));
                input->AddEntity(AAMAText(pos, strings.at(j), *layer1));
            }
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportAAMADrill(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();

    for(const auto &label : labels)
    {
        if (label.Type() == PlaceLabelType::Doubletree || label.Type() == PlaceLabelType::Button
                || label.Type() == PlaceLabelType::Circle)
        {
            const QPointF center = detail.GetMatrix().map(label.Center());
            detailBlock->ent.push_back(AAMAPoint(center, *layer13));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::ExportToASTM(const QVector<VLayoutPiece> &details) -> bool
{
    if (not m_size.isValid())
    {
        qWarning()<<"VDxfEngine::begin(), size is not valid";
        return false;
    }

    m_input = QSharedPointer<dx_iface>(new dx_iface(GetFileNameForLocale(), m_version, m_varMeasurement,
                                                    m_varInsunits));

    m_input->AddAAMAHeaderData();
    if (m_version > DRW::AC1009)
    {
        m_input->AddDefLayers();
    }
    m_input->AddASTMLayers();

    ExportStyleSystemText(m_input, details);

    for(auto detail : details)
    {
        auto detailBlock = QSharedPointer<dx_ifaceBlock>::create();

        QString blockName = detail.GetName();
        if (m_version <= DRW::AC1009)
        {
            blockName.replace(' ', '_');
        }

        detailBlock->name = blockName.toStdString();
        detailBlock->layer = *layer1;

        detail.Scale(m_xscale, m_yscale);

        ExportASTMPieceBoundary(detailBlock, detail);
        ExportASTMSewLine(detailBlock, detail);
        ExportASTMInternalLine(detailBlock, detail);
        ExportASTMInternalCutout(detailBlock, detail);
        ExportASTMNotch(detailBlock, detail);
        ExportAAMAGrainline(detailBlock, detail);
        ExportPieceText(detailBlock, detail);
        ExportASTMDrill(detailBlock, detail);
        ExportASTMAnnotationText(detailBlock, detail);

        m_input->AddBlock(detailBlock.get());

        QScopedPointer<DRW_Insert> insert(new DRW_Insert());
        insert->name = blockName.toStdString();
        insert->layer = *layer1;

        m_input->AddEntity(insert.take());
    }

    return m_input->fileExport(m_binary);
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMPieceBoundary(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<VLayoutPoint> pieceBoundary = PieceOutline(detail);

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
    if (detail.IsSeamAllowance() && not detail.IsHideMainPath() && not detail.IsSeamAllowanceBuiltIn())
    {
        QVector<VLayoutPoint> sewLine = detail.GetMappedContourPoints();

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
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMInternalLine(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QVector<QVector<VLayoutPoint>> drawIntLine = detail.MappedInternalPathsForCut(false);
    for(const auto &intLine : drawIntLine)
    {
        // Internal line
        if (DRW_Entity *e = AAMAPolygon(intLine, *layer8, false))
        {
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, intLine);
        ExportCurvePoints(detailBlock, intLine);

        // Internal lines quality validation curves
        if (DRW_Entity *e = AAMAPolygon(intLine, *layer85, false))
        {
            detailBlock->ent.push_back(e);
        }
    }

    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();
    for(const auto &label : labels)
    {
        if (label.Type() != PlaceLabelType::Doubletree && label.Type() != PlaceLabelType::Button
            && label.Type() != PlaceLabelType::Circle)
        {
            PlaceLabelImg shape = detail.MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(label));
            for(const auto &p : shape)
            {
                // Internal line (placelabel)
                if (DRW_Entity *e = AAMAPolygon(p, *layer8, false))
                {
                    detailBlock->ent.push_back(e);
                }

                ExportTurnPoints(detailBlock, p);
                ExportCurvePoints(detailBlock, p);

                // Internal lines quality validation curves
                if (DRW_Entity *e = AAMAPolygon(p, *layer85, false))
                {
                    detailBlock->ent.push_back(e);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMInternalCutout(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QVector<QVector<VLayoutPoint>> drawIntCut = detail.MappedInternalPathsForCut(true);
    for(auto &intCut : drawIntCut)
    {
        // Internal cutout
        if (DRW_Entity *e = AAMAPolygon(intCut, *layer11, false))
        {
            detailBlock->ent.push_back(e);
        }

        ExportTurnPoints(detailBlock, intCut);
        ExportCurvePoints(detailBlock, intCut);

        // Internal cutouts quality validation curves
        if (DRW_Entity *e = AAMAPolygon(intCut, *layer86, false))
        {
            detailBlock->ent.push_back(e);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMAnnotationText(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    QString name = detail.GetName();
    QPointF textPos = detail.VLayoutPiece::DetailBoundingRect().center();

    QPointF pos(textPos.x(), textPos.y() - ToPixel(AAMATextHeight, m_varInsunits));
    detailBlock->ent.push_back(AAMAText(pos, name, *layer15));
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMDrill(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    const QVector<VLayoutPlaceLabel> labels = detail.GetPlaceLabels();

    for(const auto &label : labels)
    {
        if (label.Type() == PlaceLabelType::Doubletree || label.Type() == PlaceLabelType::Button
            || label.Type() == PlaceLabelType::Circle)
        {
            const QPointF center = detail.GetMatrix().map(label.Center());
            detailBlock->ent.push_back(AAMAPoint(center, *layer13));

            // TODO. Investigate drill category
//            QPointF pos(center.x(), center.y() - ToPixel(AAMATextHeight, varInsunits));
//            detailBlock->ent.push_back(AAMAText(pos, category, *layer13));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportASTMNotch(const QSharedPointer<dx_ifaceBlock> &detailBlock, const VLayoutPiece &detail)
{
    if (detail.IsSeamAllowance())
    {
        const QVector<VLayoutPassmark> passmarks = detail.GetMappedPassmarks();
        for(const auto &passmark : passmarks)
        {
            auto *notch = new DRW_ASTMNotch();
            const QPointF center = passmark.baseLine.p1();

            notch->basePoint = DRW_Coord(FromPixel(center.x(), m_varInsunits),
                                         FromPixel(GetSize().height() - center.y(), m_varInsunits),
                                         FromPixel(passmark.baseLine.length(), m_varInsunits));

            notch->angle = passmark.baseLine.angle();

            if (passmark.type == PassmarkLineType::OneLine || passmark.type == PassmarkLineType::TwoLines
                || passmark.type == PassmarkLineType::ThreeLines)
            { // Slit notch
                notch->layer = *layer4;
            }
            else if (passmark.type == PassmarkLineType::VMark || passmark.type == PassmarkLineType::VMark2)
            {
                QLineF boundaryLine(ConstFirst(passmark.lines).p2(), ConstLast(passmark.lines).p2());
                notch->thickness = FromPixel(boundaryLine.length(), m_varInsunits); // width

                notch->layer = *layer4;
            }
            else if (passmark.type == PassmarkLineType::TMark)
            {
                qreal width = FromPixel(ConstLast(passmark.lines).length(), m_varInsunits);
                notch->thickness = FromPixel(width, m_varInsunits);

                notch->layer = *layer80;
            }
            else if (passmark.type == PassmarkLineType::BoxMark)
            {
                QPointF start = ConstFirst(passmark.lines).p1();
                QPointF end = ConstLast(passmark.lines).p2();

                notch->layer = *layer81;

                notch->thickness = FromPixel(QLineF(start, end).length(), m_varInsunits);
            }
            else if (passmark.type == PassmarkLineType::UMark)
            {
                QPointF start = ConstFirst(passmark.lines).p1();
                QPointF end = ConstLast(passmark.lines).p2();

                notch->thickness = FromPixel(QLineF(start, end).length(), m_varInsunits);

                notch->layer = *layer83;
            }

            detailBlock->ent.push_back(notch);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDxfEngine::ExportTurnPoints(const QSharedPointer<dx_ifaceBlock> &detailBlock,
                                  const QVector<VLayoutPoint> &points) const
{
    for(const auto &p : qAsConst(points))
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
    for(const auto &p : qAsConst(points))
    {
        if (p.CurvePoint() && not p.TurnPoint())
        {
            detailBlock->ent.push_back(AAMAPoint(p, *layer3));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMAPolygon(const QVector<VLayoutPoint> &polygon, const UTF8STRING &layer,
                             bool forceClosed) -> DRW_Entity *
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
    lineEnt->secPoint =  DRW_Coord(FromPixel(line.p2().x(), m_varInsunits),
                                   FromPixel(GetSize().height() - line.p2().y(), m_varInsunits), 0);
    lineEnt->layer = layer;

    return lineEnt;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMAText(const QPointF &pos, const QString &text, const UTF8STRING &layer) -> DRW_Entity *
{
    auto *textLine = new DRW_Text();

    textLine->basePoint = DRW_Coord(FromPixel(pos.x(), m_varInsunits),
                                    FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    textLine->secPoint = DRW_Coord(FromPixel(pos.x(), m_varInsunits),
                                   FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    textLine->height = AAMATextHeight;
    textLine->layer = layer;
    textLine->text = text.toStdString();

    return textLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::AAMAPoint(const QPointF &pos, const UTF8STRING &layer) const -> DRW_Point *
{
    auto *point = new DRW_Point();
    point->basePoint = DRW_Coord(FromPixel(pos.x(), m_varInsunits),
                                 FromPixel(GetSize().height() - pos.y(), m_varInsunits), 0);
    point->layer = layer;
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::FromUnicodeToCodec(const QString &str, QTextCodec *codec) -> std::string
{
    return codec->fromUnicode(str).toStdString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDxfEngine::GetFileNameForLocale() const -> std::string
{
#if defined(Q_OS_WIN)
    return VDxfEngine::FromUnicodeToCodec(fileName, QTextCodec::codecForLocale());
#else
    return m_fileName.toStdString();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template<class P, class V, class C>
auto VDxfEngine::CreateAAMAPolygon(const QVector<C> &polygon, const UTF8STRING &layer,
                                   bool forceClosed) -> P *
{
    auto *poly = new P();
    poly->layer = layer;

    if (forceClosed)
    {
        poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
    }
    else
    {
        if (polygon.size() > 1 && ConstFirst<QPointF>(polygon) == ConstLast<QPointF>(polygon))
        {
            poly->flags |= 0x1; // closed NOLINT(hicpp-signed-bitwise)
        }
    }

    for (const auto &p : polygon)
    {
        poly->addVertex(V(FromPixel(p.x(), m_varInsunits),
                          FromPixel(GetSize().height() - p.y(), m_varInsunits)));
    }

    return poly;
}
