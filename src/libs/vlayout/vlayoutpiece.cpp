/************************************************************************
 **
 **  @file   vlayoutdetail.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 1, 2015
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

#include "vlayoutpiece.h"

#include <QBrush>
#include <QFlags>
#include <QFont>
#include <QFontMetrics>
#include <QFuture>
#include <QGraphicsPathItem>
#include <QLine>
#include <QList>
#include <QMessageLogger>
#include <QPainterPath>
#include <QPoint>
#include <QPolygon>
#include <QPolygonF>
#include <QTransform>
#include <QUuid>
#include <QtConcurrent/QtConcurrentRun>
#include <QtDebug>
#include <QtMath>

#include "../vformat/vsinglelineoutlinechar.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vgeometry/vplacelabelitem.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/literals.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/testpath.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vpassmark.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vwidgets/vpiecegrainline.h"
#include "vabstractpiece.h"
#include "vboundary.h"
#include "vcommonsettings.h"
#include "vfoldline.h"
#include "vgraphicsfillitem.h"
#include "vlayoutpiece_p.h"
#include "vtextmanager.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ConvertInternalPaths(const VPiece &piece, const VContainer *pattern) -> QVector<VLayoutPiecePath>
{
    SCASSERT(pattern != nullptr)

    piece.TestInternalPaths(pattern);

    QVector<VLayoutPiecePath> paths;
    const QVector<quint32> pathsId = piece.GetInternalPaths();
    const QVector<QPointF> cuttingPath = piece.CuttingPathPoints(pattern);
    paths.reserve(pathsId.size());
    for (auto id : pathsId)
    {
        const VPiecePath path = pattern->GetPiecePath(id);
        if (path.GetType() == PiecePathType::InternalPath && path.IsVisible(pattern->DataVariables()))
        {
            VLayoutPiecePath convertedPath(path.PathPoints(pattern, cuttingPath));
            convertedPath.SetCutPath(path.IsCutPath());
            convertedPath.SetPenStyle(path.GetPenType());
            convertedPath.SetNotMirrored(path.IsNotMirrored());
            paths.append(convertedPath);
        }
    }
    return paths;
}

//---------------------------------------------------------------------------------------------------------------------
auto FindLabelGeometry(const VPatternLabelData &labelData, const VContainer *pattern, qreal &rotationAngle,
                       qreal &labelWidth, qreal &labelHeight, QPointF &pos) -> bool
{
    SCASSERT(pattern != nullptr)

    try
    {
        Calculator cal1;
        rotationAngle = cal1.EvalFormula(pattern->DataVariables(), labelData.GetRotation());
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return false;
    }

    const quint32 topLeftPin = labelData.TopLeftPin();
    const quint32 bottomRightPin = labelData.BottomRightPin();

    if (topLeftPin != NULL_ID && bottomRightPin != NULL_ID)
    {
        try
        {
            const auto topLeftPinPoint = pattern->GeometricObject<VPointF>(topLeftPin);
            const auto bottomRightPinPoint = pattern->GeometricObject<VPointF>(bottomRightPin);

            const auto labelRect =
                QRectF(static_cast<QPointF>(*topLeftPinPoint), static_cast<QPointF>(*bottomRightPinPoint));
            labelWidth = qAbs(labelRect.width());
            labelHeight = qAbs(labelRect.height());

            pos = labelRect.topLeft();

            return true;
        }
        catch (const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    try
    {
        Calculator cal1;
        labelWidth = cal1.EvalFormula(pattern->DataVariables(), labelData.GetLabelWidth());
        labelWidth = ToPixel(labelWidth, *pattern->GetPatternUnit());

        Calculator cal2;
        labelHeight = cal2.EvalFormula(pattern->DataVariables(), labelData.GetLabelHeight());
        labelHeight = ToPixel(labelHeight, *pattern->GetPatternUnit());
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return false;
    }

    const quint32 centerPin = labelData.CenterPin();
    if (centerPin != NULL_ID)
    {
        try
        {
            const auto centerPinPoint = pattern->GeometricObject<VPointF>(centerPin);
            pos = static_cast<QPointF>(*centerPinPoint) - QRectF(0, 0, labelWidth, labelHeight).center();
        }
        catch (const VExceptionBadId &)
        {
            pos = labelData.GetPos();
        }
    }
    else
    {
        pos = labelData.GetPos();
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VLayoutDetail::RotatePoint rotates a point around the center for given angle
 * @param ptCenter center around which the point is rotated
 * @param pt point, which is rotated around the center
 * @param dAng angle of rotation
 * @return position of point pt after rotating it around the center for dAng radians
 */
auto RotatePoint(const QPointF &ptCenter, const QPointF &pt, qreal dAng) -> QPointF
{
    QPointF ptDest;
    QPointF const ptRel = pt - ptCenter;
    ptDest.setX(cos(dAng) * ptRel.x() - sin(dAng) * ptRel.y());
    ptDest.setY(sin(dAng) * ptRel.x() + cos(dAng) * ptRel.y());

    return ptDest + ptCenter;
}

//---------------------------------------------------------------------------------------------------------------------
auto PieceLabelText(const QVector<QPointF> &labelShape, const VTextManager &tm) -> QStringList
{
    if (labelShape.count() <= 2)
    {
        return {};
    }

    QStringList text;
    auto sourceCount = tm.GetSourceLinesCount();
    text.reserve(sourceCount);
    for (int i = 0; i < sourceCount; ++i)
    {
        text.append(tm.GetSourceLine(i).qsText);
    }

    return text;
}

//---------------------------------------------------------------------------------------------------------------------
auto ConvertPlaceLabels(const VPiece &piece, const VContainer *pattern) -> QVector<VLayoutPlaceLabel>
{
    QVector<VLayoutPlaceLabel> labels;
    const auto placeLabels = piece.GetPlaceLabels();
    labels.reserve(placeLabels.size());
    for (const auto &placeLabel : placeLabels)
    {
        const auto label = pattern->GeometricObject<VPlaceLabelItem>(placeLabel);
        if (label->IsVisible())
        {
            QT_WARNING_PUSH
            QT_WARNING_DISABLE_GCC("-Wnoexcept")
            // noexcept-expression evaluates to 'false' because of a call to 'constexpr QPointF::QPointF()'

            labels.append(VLayoutPlaceLabel(*label));

            QT_WARNING_POP
        }
    }
    return labels;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareSAPassmark(const VPiece &piece, const VContainer *pattern, const VPassmark &passmark, PassmarkSide side,
                       bool &ok) -> VLayoutPassmark
{
    VPiecePassmarkData const pData = passmark.Data();
    const int nodeIndex = VPiecePath::indexOfNode(piece.GetUnitedPath(pattern), pData.id);
    if (nodeIndex == -1)
    {
        const QString errorMsg =
            QObject::tr("Passmark '%1' is not part of piece '%2'.").arg(pData.nodeName, piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        ok = false;
        return {};
    }

    QVector<QLineF> const baseLines = passmark.SAPassmarkBaseLine(piece, pattern, side);
    if (baseLines.isEmpty())
    {
        const QString errorMsg =
            QObject::tr("Cannot prepare passmark '%1' for piece '%2'. Passmark base line is empty.")
                .arg(pData.nodeName, piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        ok = false;
        return {};
    }

    const QVector<QLineF> lines = passmark.SAPassmark(piece, pattern, side);
    if (lines.isEmpty())
    {
        const QString errorMsg = QObject::tr("Cannot prepare passmark '%1' for piece '%2'. Passmark is empty.")
                                     .arg(pData.nodeName, piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        ok = false;
        return {};
    }

    QLineF baseLine;
    if (side == PassmarkSide::All || side == PassmarkSide::Left)
    {
        baseLine = baseLines.constFirst();
    }
    else if (side == PassmarkSide::Right)
    {
        baseLine = baseLines.constLast();
    }

    ok = true;
    return {.lines = lines,
            .type = pData.passmarkLineType,
            .baseLine = baseLine,
            .isClockwiseOpening = pData.passmarkSAPoint.IsPassmarkClockwiseOpening(),
            .label = pData.nodeName,
            .notMirrored = pData.notMirrored};
}

//---------------------------------------------------------------------------------------------------------------------
auto PreapreBuiltInSAPassmark(const VPiece &piece, const VContainer *pattern, const VPassmark &passmark, bool &ok)
    -> VLayoutPassmark
{
    VPiecePassmarkData const pData = passmark.Data();
    const QVector<VPieceNode> path = piece.GetUnitedPath(pattern);
    const int nodeIndex = VPiecePath::indexOfNode(path, pData.id);
    if (nodeIndex == -1)
    {
        const QString errorMsg =
            QObject::tr("Passmark '%1' is not part of piece '%2'.").arg(pData.nodeName, piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        ok = false;
        return {};
    }

    const QVector<QLineF> lines = passmark.BuiltInSAPassmark(piece, pattern);
    if (lines.isEmpty())
    {
        const QString errorMsg = QObject::tr("Cannot prepare builtin passmark '%1' for piece '%2'. Passmark is empty.")
                                     .arg(pData.nodeName, piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        ok = false;
        return {};
    }

    const QLineF mirrorLine = piece.SeamMirrorLine(pattern);
    const QVector<QLineF> baseLines = passmark.BuiltInSAPassmarkBaseLine(piece, mirrorLine);
    if (baseLines.isEmpty())
    {
        const QString errorMsg =
            QObject::tr("Cannot prepare builtin  passmark '%1' for piece '%2'. Passmark base line is "
                        "empty.")
                .arg(pData.nodeName, piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        ok = false;
        return {};
    }

    ok = true;
    return {.lines = lines,
            .type = pData.passmarkLineType,
            .baseLine = baseLines.constFirst(),
            .isBuiltIn = true,
            .isClockwiseOpening = pData.passmarkSAPoint.IsPassmarkClockwiseOpening(),
            .label = pData.nodeName,
            .notMirrored = pData.notMirrored};
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareGradationPlaceholders(const VContainer *data) -> QMap<QString, QString>
{
    SCASSERT(data != nullptr)

    QMap<QString, QString> placeholders;

    QString const heightValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHeight());
    placeholders.insert(pl_height, heightValue);
    placeholders.insert(pl_dimensionX, heightValue);

    QString const sizeValue = QString::number(VAbstractValApplication::VApp()->GetDimensionSize());
    placeholders.insert(pl_size, sizeValue);
    placeholders.insert(pl_dimensionY, sizeValue);

    QString const hipValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHip());
    placeholders.insert(pl_hip, hipValue);
    placeholders.insert(pl_dimensionZ, hipValue);

    QString const waistValue = QString::number(VAbstractValApplication::VApp()->GetDimensionWaist());
    placeholders.insert(pl_waist, waistValue);
    placeholders.insert(pl_dimensionW, waistValue);

    {
        QString label = VAbstractValApplication::VApp()->GetDimensionHeightLabel();
        placeholders.insert(pl_heightLabel, not label.isEmpty() ? label : heightValue);
        placeholders.insert(pl_dimensionXLabel, not label.isEmpty() ? label : heightValue);

        label = VAbstractValApplication::VApp()->GetDimensionSizeLabel();
        placeholders.insert(pl_sizeLabel, not label.isEmpty() ? label : sizeValue);
        placeholders.insert(pl_dimensionYLabel, not label.isEmpty() ? label : heightValue);

        label = VAbstractValApplication::VApp()->GetDimensionHipLabel();
        placeholders.insert(pl_hipLabel, not label.isEmpty() ? label : hipValue);
        placeholders.insert(pl_dimensionZLabel, not label.isEmpty() ? label : heightValue);

        label = VAbstractValApplication::VApp()->GetDimensionWaistLabel();
        placeholders.insert(pl_waistLabel, not label.isEmpty() ? label : waistValue);
        placeholders.insert(pl_dimensionWLabel, not label.isEmpty() ? label : heightValue);
    }

    {
        const QMap<QString, QSharedPointer<VMeasurement>> measurements = data->DataMeasurements();
        auto i = measurements.constBegin();
        while (i != measurements.constEnd())
        {
            placeholders.insert(pl_measurement + i.key(), QString::number(*i.value()->GetValue()));
            placeholders.insert(pl_measurement + i.key() + pl_valueAlias, i.value()->GetValueAlias());
            ++i;
        }
    }

    return placeholders;
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplacePlaceholders(const QMap<QString, QString> &placeholders, QString line) -> QString
{
    QChar const per('%');

    auto TestDimension = [per, placeholders, line](const QString &placeholder, const QString &errorMsg)
    {
        if (line.contains(per + placeholder + per) && placeholders.value(placeholder) == '0'_L1)
        {
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    };

    TestDimension(pl_height, QObject::tr("No data for the height dimension."));
    TestDimension(pl_size, QObject::tr("No data for the size dimension."));
    TestDimension(pl_hip, QObject::tr("No data for the hip dimension."));
    TestDimension(pl_waist, QObject::tr("No data for the waist dimension."));

    TestDimension(pl_dimensionX, QObject::tr("No data for the X dimension."));
    TestDimension(pl_dimensionY, QObject::tr("No data for the Y dimension."));
    TestDimension(pl_dimensionZ, QObject::tr("No data for the Z dimension."));
    TestDimension(pl_dimensionW, QObject::tr("No data for the W dimension."));

    auto i = placeholders.constBegin();
    while (i != placeholders.constEnd())
    {
        line.replace(per + i.key() + per, i.value());
        ++i;
    }
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareGradationId(const QString &label, const VContainer *pattern) -> QString
{
    const QMap<QString, QString> placeholders = PrepareGradationPlaceholders(pattern);
    return ReplacePlaceholders(placeholders, label);
}

//---------------------------------------------------------------------------------------------------------------------
void InitFoldLine(VLayoutPiece &det, const VPiece &piece, const VContainer *pattern)
{
    det.SetFoldLineType(piece.GetFoldLineType());

    if (piece.GetFoldLineType() == FoldLineType::None)
    {
        return;
    }

    det.SetFoldLineSvgFontSize(piece.GetFoldLineSvgFontSize());
    det.SetFoldLineLabelFontItalic(piece.IsFoldLineLabelFontItalic());
    det.SetFoldLineLabelFontBold(piece.IsFoldLineLabelFontBold());
    det.SetFoldLineLabel(piece.GetFoldLineLabel());
    det.SetFoldLineLabelAlignment(piece.GetFoldLineLabelAlignment());

    {
        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        QFont font = settings->GetLabelFont();
        font.setPointSize(static_cast<int>(piece.GetFoldLineSvgFontSize()));
        det.SetFoldLineOutlineFont(font);
        det.SetFoldLineSVGFontFamily(settings->GetLabelSVGFont());
    }

    if (piece.IsManualFoldHeight())
    {
        VFormula formula(piece.GetFormulaFoldHeight(), pattern);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (!formula.error())
        {
            det.SetFoldLineHeight(ToPixel(formula.getDoubleValue(), *pattern->GetPatternUnit()));
        }
    }

    if (piece.IsManualFoldWidth())
    {
        VFormula formula(piece.GetFormulaFoldWidth(), pattern);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (!formula.error())
        {
            det.SetFoldLineWidth(ToPixel(formula.getDoubleValue(), *pattern->GetPatternUnit()));
        }
    }

    if (piece.IsManualFoldCenter())
    {
        VFormula formula(piece.GetFormulaFoldCenter(), pattern);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (!formula.error())
        {
            det.SetFoldLineCenterPosition(formula.getDoubleValue());
        }
    }
}
} // namespace

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPoint &p) -> QDataStream &
{
    dataStream << p.ToQPointF();
    dataStream << p.TurnPoint();
    dataStream << p.CurvePoint();
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPoint &p) -> QDataStream &
{
    QPointF tmp;
    bool turnPointFlag = false;
    bool curvePointFlag = false;
    dataStream >> tmp;
    dataStream >> turnPointFlag;
    dataStream >> curvePointFlag;

    p = VLayoutPoint(tmp);
    p.SetTurnPoint(turnPointFlag);
    p.SetCurvePoint(curvePointFlag);

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPiece &piece) -> QDataStream &
{
    const VAbstractPiece &abstractPiece = piece;
    dataStream << abstractPiece;
    dataStream << *piece.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPiece &piece) -> QDataStream &
{
    dataStream >> static_cast<VAbstractPiece &>(piece);
    dataStream >> *piece.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wnoexcept")

VLayoutPiece::VLayoutPiece()
  : d(new VLayoutPieceData)
{
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VLayoutPiece, VAbstractPiece)

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::operator=(const VLayoutPiece &detail) -> VLayoutPiece &
{
    if (&detail == this)
    {
        return *this;
    }
    VAbstractPiece::operator=(detail);
    d = detail.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece::VLayoutPiece(VLayoutPiece &&detail) noexcept
  : VAbstractPiece(std::move(detail)),
    d(std::move(detail.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::operator=(VLayoutPiece &&detail) noexcept -> VLayoutPiece &
{
    VAbstractPiece::operator=(detail);
    std::swap(d, detail.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece::~VLayoutPiece() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::Create(const VPiece &piece, vidtype id, const VContainer *pattern) -> VLayoutPiece
{
    QFuture<QVector<VLayoutPoint>> const futureSeamAllowance = QtConcurrent::run(
        [piece, pattern]()
        {
            if (!piece.SeamMirrorLine(pattern).isNull())
            {
                VPiece tmp = piece;
                tmp.SetShowFullPiece(false);
                return tmp.SeamAllowancePoints(pattern);
            }
            return piece.SeamAllowancePoints(pattern);
        });
    QFuture<bool> const futureSeamAllowanceValid =
        QtConcurrent::run([piece, pattern]() { return piece.IsSeamAllowanceValid(pattern); });
    QFuture<QVector<VLayoutPoint>> const futureMainPath =
        QtConcurrent::run([piece, pattern]() { return piece.MainPathPoints(pattern); });
    QFuture<QVector<VLayoutPiecePath>> const futureInternalPaths =
        QtConcurrent::run(ConvertInternalPaths, piece, pattern);
    QFuture<QVector<VLayoutPassmark>> const futurePassmarks =
        QtConcurrent::run(VLayoutPiece::ConvertPassmarks, piece, pattern);
    QFuture<QVector<VLayoutPlaceLabel>> const futurePlaceLabels = QtConcurrent::run(ConvertPlaceLabels, piece, pattern);

    VLayoutPiece det;

    det.SetMx(piece.GetMx());
    det.SetMy(piece.GetMy());

    det.SetName(piece.GetName());
    det.SetUUID(piece.GetUUID());
    det.SetGradationId(PrepareGradationId(piece.GetGradationLabel(), pattern));

    det.SetSAWidth(VAbstractValApplication::VApp()->toPixel(piece.GetSAWidth()));
    det.SetForbidFlipping(piece.IsForbidFlipping());
    det.SetForceFlipping(piece.IsForceFlipping());
    det.SetSymmetricalCopy(piece.IsSymmetricalCopy());
    det.SetFollowGrainline(piece.IsFollowGrainline());
    det.SetSewLineOnDrawing(piece.IsSewLineOnDrawing());
    det.SetShowFullPiece(piece.IsShowFullPiece());
    det.SetSeamMirrorLine(piece.SeamMirrorLine(pattern));
    det.SetSeamAllowanceMirrorLine(piece.SeamAllowanceMirrorLine(pattern));
    det.SetShowMirrorLine(piece.IsShowMirrorLine());
    det.SetId(id);

    if (not futureSeamAllowanceValid.result())
    {
        const QString errorMsg = QObject::tr("Piece '%1'. Seam allowance is not valid.").arg(piece.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    det.SetContourPoints(futureMainPath.result(), settings->IsPieceShowMainPath() ? false : piece.IsHideMainPath());
    det.SetSeamAllowancePoints(futureSeamAllowance.result(), piece.IsSeamAllowance(), piece.IsSeamAllowanceBuiltIn());
    det.SetInternalPaths(futureInternalPaths.result());
    det.SetPassmarks(futurePassmarks.result());
    det.SetPlaceLabels(futurePlaceLabels.result());
    det.SetPriority(piece.GetPriority());

    // Very important to set main path first!
    if (det.MappedContourPath(false, false).isEmpty())
    {
        throw VException(tr("Piece %1 doesn't have shape.").arg(piece.GetName()));
    }

    const VPieceLabelData &data = piece.GetPieceLabelData();
    det.SetQuantity(data.GetQuantity());
    if (data.IsEnabled())
    {
        const VAbstractPattern *pDoc = VAbstractValApplication::VApp()->getCurrentDocument();
        det.SetPieceText(pDoc, piece.GetName(), data, settings->GetLabelFont(), settings->GetLabelSVGFont(), pattern);
    }

    if (const VPatternLabelData &geom = piece.GetPatternLabelData(); geom.IsEnabled())
    {
        const VAbstractPattern *pDoc = VAbstractValApplication::VApp()->getCurrentDocument();
        det.SetPatternInfo(pDoc, geom, settings->GetLabelFont(), settings->GetLabelSVGFont(), pattern);
    }

    if (const VGrainlineData &grainlineGeom = piece.GetGrainlineGeometry(); grainlineGeom.IsEnabled())
    {
        det.SetGrainline(grainlineGeom, pattern);
    }

    InitFoldLine(det, piece, pattern);

    return det;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetUniqueID() const -> QString
{
    QString id = VAbstractPiece::GetUniqueID();

    if (not d->m_gradationId.isEmpty())
    {
        id = id + '_'_L1 + d->m_gradationId;
    }

    return id;
}

//---------------------------------------------------------------------------------------------------------------------
template <> // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
auto VLayoutPiece::Map<VLayoutPassmark>(QVector<VLayoutPassmark> passmarks) const -> QVector<VLayoutPassmark>
{
    for (auto &passmark : passmarks)
    {
        passmark = MapPassmark(passmark, d->m_matrix, d->m_verticallyFlipped || d->m_horizontallyFlipped);
    }

    return passmarks;
}

//---------------------------------------------------------------------------------------------------------------------
template <> auto VLayoutPiece::Map<VLayoutPoint>(QVector<VLayoutPoint> points) const -> QVector<VLayoutPoint>
{
    std::transform(points.begin(), points.end(), points.begin(),
                   [this](const VLayoutPoint &point) { return MapPoint(point, d->m_matrix); });
    if (d->m_verticallyFlipped || d->m_horizontallyFlipped)
    {
        std::reverse(points.begin(), points.end());
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::Map(const GrainlineShape &shape) const -> GrainlineShape
{
    GrainlineShape mappedShape;
    mappedShape.reserve(shape.size());

    for (const auto &subShape : shape)
    {
        mappedShape.append(Map(subShape));
    }
    return mappedShape;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VLayoutPiece::GetMappedContourPoints() const -> QVector<VLayoutPoint>
{
    return Map(d->m_contour);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedFullContourPoints(bool togetherWithNotches, bool drawMode, bool layoutAllowance) const
    -> QVector<VLayoutPoint>
{
    return Map(GetFullContourPoints(togetherWithNotches, drawMode, layoutAllowance));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFullContourPoints(bool togetherWithNotches, bool drawMode, bool layoutAllowance) const
    -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> points;
    points.reserve(d->m_contour.size());
    if (!d->m_seamMirrorLine.isNull() && IsShowFullPiece())
    {
        points = VAbstractPiece::FullSeamPath(d->m_contour, d->m_seamMirrorLine, GetName());
        points = CheckLoops(CorrectEquidistantPoints(points)); // A path can contains loops
    }
    else
    {
        points = d->m_contour;
    }

    if (togetherWithNotches)
    {
        QVector<VLayoutPassmark> const passmarks = GetPassmarks();

        bool const seamAllowance = IsSeamAllowance() && IsSeamAllowanceBuiltIn();
        bool const builtInSeamAllowance = IsSeamAllowance() && IsSeamAllowanceBuiltIn();

        VBoundary boundary(points, seamAllowance, builtInSeamAllowance);
        boundary.SetPieceName(GetName());
        if (IsShowFullPiece())
        {
            boundary.SetMirrorLine(GetSeamMirrorLine());
        }
        const QList<VBoundarySequenceItemData> sequence = boundary.Combine(passmarks, drawMode, layoutAllowance);

        QVector<VLayoutPoint> combinedBoundary;
        for (const auto &item : sequence)
        {
            combinedBoundary += item.item.value<VLayoutPiecePath>().Points();
        }

        return combinedBoundary;
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetContourPoints() const -> QVector<VLayoutPoint>
{
    return d->m_contour;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetContourPoints(const QVector<VLayoutPoint> &points, bool hideMainPath)
{
    d->m_contour = RemoveDublicates(points, false);
    SetHideMainPath(hideMainPath);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VLayoutPiece::GetMappedSeamAllowancePoints() const -> QVector<VLayoutPoint>
{
    return Map(d->m_seamAllowance);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedFullSeamAllowancePoints(bool togetherWithNotches, bool drawMode, bool layoutAllowance) const
    -> QVector<VLayoutPoint>
{
    return Map(GetFullSeamAllowancePoints(togetherWithNotches, drawMode, layoutAllowance));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::CorrectSeamAllowanceMirrorLine() const -> QLineF
{
    QLineF correctedSeamAllowanceMirrorLine = d->m_seamAllowanceMirrorLine;

    QVector<QPointF> seamAllowancePoints;
    CastTo(d->m_seamAllowance, seamAllowancePoints);

    if (!VAbstractCurve::IsPointOnCurve(seamAllowancePoints, d->m_seamAllowanceMirrorLine.p1()) ||
        !VAbstractCurve::IsPointOnCurve(seamAllowancePoints, d->m_seamAllowanceMirrorLine.p2()))
    {
        correctedSeamAllowanceMirrorLine = SeamAllowanceMirrorLine(d->m_seamMirrorLine, seamAllowancePoints);
    }

    return correctedSeamAllowanceMirrorLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFullSeamAllowancePoints(bool togetherWithNotches, bool drawMode, bool layoutAllowance) const
    -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> points;
    points.reserve(d->m_seamAllowance.size());
    if (!d->m_seamAllowanceMirrorLine.isNull() && IsShowFullPiece())
    {
        // Trying to correct a seam allowance mirror line based on seam mirror line
        const QLineF seamAllowanceMirrorLine = CorrectSeamAllowanceMirrorLine();
        points = VAbstractPiece::FullSeamAllowancePath(d->m_seamAllowance, seamAllowanceMirrorLine, GetName());
        points = CheckLoops(CorrectEquidistantPoints(points)); // A path can contains loops
    }
    else
    {
        points = d->m_seamAllowance;
    }

    if (togetherWithNotches)
    {
        const QVector<VLayoutPassmark> passmarks = GetPassmarks();

        bool const seamAllowance = IsSeamAllowance() && !IsSeamAllowanceBuiltIn();
        bool const builtInSeamAllowance = IsSeamAllowance() && IsSeamAllowanceBuiltIn();

        VBoundary boundary(points, seamAllowance, builtInSeamAllowance);
        boundary.SetPieceName(GetName());
        if (IsShowFullPiece())
        {
            boundary.SetMirrorLine(GetSeamAllowanceMirrorLine());
        }
        const QList<VBoundarySequenceItemData> sequence = boundary.Combine(passmarks, drawMode, layoutAllowance);

        QVector<VLayoutPoint> combinedBoundary;
        for (const auto &item : sequence)
        {
            combinedBoundary += item.item.value<VLayoutPiecePath>().Points();
        }

        return combinedBoundary;
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetSeamAllowancePoints() const -> QVector<VLayoutPoint>
{
    return d->m_seamAllowance;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetSeamAllowancePoints(const QVector<VLayoutPoint> &points, bool seamAllowance,
                                          bool seamAllowanceBuiltIn)
{
    if (seamAllowance)
    {
        SetSeamAllowance(seamAllowance);
        SetSeamAllowanceBuiltIn(seamAllowanceBuiltIn);
        d->m_seamAllowance = points;
        if (not d->m_seamAllowance.isEmpty())
        {
            d->m_seamAllowance = RemoveDublicates(d->m_seamAllowance, false);
        }
        else if (not IsSeamAllowanceBuiltIn())
        {
            qWarning() << "Seam allowance is empty.";
            SetSeamAllowance(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedLayoutAllowancePoints() const -> QVector<QPointF>
{
    return Map(d->m_layoutAllowance);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetLayoutAllowancePoints() const -> QVector<QPointF>
{
    return d->m_layoutAllowance;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPieceText() const -> QStringList
{
    return PieceLabelText(d->m_detailLabel, d->m_tmDetail);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPieceText(const VAbstractPattern *pDoc, const QString &qsName, const VPieceLabelData &data,
                                const QFont &font, const QString &SVGFontFamily, const VContainer *pattern)
{
    QPointF ptPos;
    qreal labelWidth = 0;
    qreal labelHeight = 0;
    qreal labelAngle = 0;
    if (not FindLabelGeometry(data, pattern, labelAngle, labelWidth, labelHeight, ptPos))
    {
        return;
    }

    QVector<QPointF> v{ptPos, QPointF(ptPos.x() + labelWidth, ptPos.y()),
                       QPointF(ptPos.x() + labelWidth, ptPos.y() + labelHeight),
                       QPointF(ptPos.x(), ptPos.y() + labelHeight)};

    const qreal dAng = qDegreesToRadians(-labelAngle);
    const QPointF ptCenter(ptPos.x() + labelWidth / 2, ptPos.y() + labelHeight / 2);

    for (int i = 0; i < v.count(); ++i)
    {
        v[i] = RotatePoint(ptCenter, v.at(i), dAng);
    }

    QScopedPointer<QGraphicsItem> const item(GetExternalContourPathItem());
    d->m_detailLabel = CorrectPosition(item->boundingRect(), v);

    // generate text
    d->m_tmDetail.SetFont(font);
    d->m_tmDetail.SetSVGFontFamily(SVGFontFamily);

    int fntSize = data.GetFontSize();
    if (fntSize == 0)
    {
        fntSize = VAbstractApplication::VApp()->Settings()->GetPieceLabelFontPointSize();
    }
    d->m_tmDetail.SetFontSize(fntSize);
    d->m_tmDetail.SetSVGFontPointSize(fntSize);

    VPieceLabelInfo info = VTextManager::PrepareLabelInfo(pDoc, pattern, true);
    info.pieceName = qsName;
    info.labelData = data;
    d->m_tmDetail.UpdatePieceLabelInfo(info);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPieceLabelRect() const -> QVector<QPointF>
{
    return d->m_detailLabel;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPieceLabelRect(const QVector<QPointF> &rect)
{
    d->m_detailLabel = rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPieceLabelData() const -> VTextManager
{
    return d->m_tmDetail;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPieceLabelData(const VTextManager &data)
{
    d->m_tmDetail = data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPatternText() const -> QStringList
{
    return PieceLabelText(d->m_patternInfo, d->m_tmPattern);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPatternInfo(const VAbstractPattern *pDoc, const VPatternLabelData &geom, const QFont &font,
                                  const QString &SVGFontFamily, const VContainer *pattern)
{
    QPointF ptPos;
    qreal labelWidth = 0;
    qreal labelHeight = 0;
    qreal labelAngle = 0;
    if (not FindLabelGeometry(geom, pattern, labelAngle, labelWidth, labelHeight, ptPos))
    {
        return;
    }

    QVector<QPointF> v{ptPos, QPointF(ptPos.x() + labelWidth, ptPos.y()),
                       QPointF(ptPos.x() + labelWidth, ptPos.y() + labelHeight),
                       QPointF(ptPos.x(), ptPos.y() + labelHeight)};

    const qreal dAng = qDegreesToRadians(-labelAngle);
    const QPointF ptCenter(ptPos.x() + labelWidth / 2, ptPos.y() + labelHeight / 2);
    for (int i = 0; i < v.count(); ++i)
    {
        v[i] = RotatePoint(ptCenter, v.at(i), dAng);
    }
    QScopedPointer<QGraphicsItem> const item(GetExternalContourPathItem());
    d->m_patternInfo = CorrectPosition(item->boundingRect(), v);

    // Generate text
    d->m_tmPattern.SetFont(font);
    d->m_tmPattern.SetSVGFontFamily(SVGFontFamily);

    int fntSize = geom.GetFontSize();
    if (fntSize == 0)
    {
        fntSize = VAbstractApplication::VApp()->Settings()->GetPieceLabelFontPointSize();
    }
    d->m_tmPattern.SetFontSize(fntSize);
    d->m_tmPattern.SetSVGFontPointSize(fntSize);

    VPieceLabelInfo const info = VTextManager::PrepareLabelInfo(pDoc, pattern, false);
    d->m_tmPattern.UpdatePatternLabelInfo(info);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPatternLabelRect() const -> QVector<QPointF>
{
    return d->m_patternInfo;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPatternLabelRect(const QVector<QPointF> &rect)
{
    d->m_patternInfo = rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPatternLabelData() const -> VTextManager
{
    return d->m_tmPattern;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPatternLabelData(const VTextManager &data)
{
    d->m_tmPattern = data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedCorrectedMirrorLine(bool togetherWithNotches) const -> QLineF
{
    return d->m_matrix.map(GetCorrectedMirrorLine(togetherWithNotches));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetCorrectedMirrorLine(bool togetherWithNotches) const -> QLineF
{
    if (!IsShowFullPiece())
    {
        return {};
    }

    QLineF mirrorLine;
    QVector<VLayoutPoint> points;

    if (!IsSeamAllowance() || IsSeamAllowanceBuiltIn())
    {
        mirrorLine = GetSeamMirrorLine();
        points = GetFullContourPoints(togetherWithNotches);
    }
    else if (not IsSeamAllowanceBuiltIn())
    {
        mirrorLine = GetSeamAllowanceMirrorLine();
        points = GetFullSeamAllowancePoints(togetherWithNotches);
    }

    if (mirrorLine.isNull() || points.isEmpty())
    {
        return {};
    }

    QPointF p1 = mirrorLine.p1();
    QPointF p2 = mirrorLine.p2();

    QVector<QPointF> seam;
    CastTo(points, seam);

    if (QVector<QPointF> const intersectionPoints = VAbstractCurve::CurveIntersectLine(seam,
                                                                                       QLineF(mirrorLine.center(), p1));
        !intersectionPoints.isEmpty())
    {
        p1 = intersectionPoints.constFirst();
    }

    if (QVector<QPointF> const intersectionPoints = VAbstractCurve::CurveIntersectLine(seam,
                                                                                       QLineF(mirrorLine.center(), p2));
        !intersectionPoints.isEmpty())
    {
        p2 = intersectionPoints.constFirst();
    }

    return {p1, p2};
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedSeamMirrorLine() const -> QLineF
{
    return d->m_matrix.map(d->m_seamMirrorLine);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetSeamMirrorLine() const -> QLineF
{
    return d->m_seamMirrorLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetSeamMirrorLine(const QLineF &line)
{
    d->m_seamMirrorLine = line;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedSeamAllowanceMirrorLine() const -> QLineF
{
    return d->m_matrix.map(d->m_seamAllowanceMirrorLine);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetSeamAllowanceMirrorLine() const -> QLineF
{
    return d->m_seamAllowanceMirrorLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetSeamAllowanceMirrorLine(const QLineF &line)
{
    d->m_seamAllowanceMirrorLine = line;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFoldLineHeight() const -> qreal
{
    return d->m_foldLineHeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetFoldLineHeight(qreal height)
{
    d->m_foldLineHeight = height;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFoldLineWidth() const -> qreal
{
    return d->m_foldLineWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetFoldLineWidth(qreal width)
{
    d->m_foldLineWidth = width;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFoldLineCenterPosition() const -> qreal
{
    return d->m_foldLineCenter;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetFoldLineCenterPosition(qreal center)
{
    d->m_foldLineCenter = center;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFoldLineOutlineFont() const -> QFont
{
    return d->m_foldLineOutlineFont;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetFoldLineOutlineFont(const QFont &font)
{
    d->m_foldLineOutlineFont = font;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetFoldLineSVGFontFamily() const -> QString
{
    return d->m_foldLineSvgFontFamily;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetFoldLineSVGFontFamily(const QString &font)
{
    d->m_foldLineSvgFontFamily = font;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetGrainline(const VPieceGrainline &grainline)
{
    d->m_grainline = grainline;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetGrainline(const VGrainlineData &geom, const VContainer *pattern)
{
    QScopedPointer<QGraphicsItem> const item(GetExternalContourPathItem());

    QLineF const mainLine = GrainlineMainLine(geom, pattern, item->boundingRect());
    if (mainLine.isNull())
    {
        return;
    }
    d->m_grainline = VPieceGrainline(mainLine, geom.GetArrowType());
    d->m_grainline.SetVisible(geom.IsVisible());
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetGrainline() const -> VPieceGrainline
{
    return d->m_grainline;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetGrainline() -> VPieceGrainline &
{
    return d->m_grainline;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedGrainlineShape() const -> GrainlineShape
{
    return Map(d->m_grainline.Shape());
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetGrainlineShape() const -> GrainlineShape
{
    return d->m_grainline.Shape();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedGrainlineMainLine() const -> QLineF
{
    return d->m_matrix.map(d->m_grainline.GetMainLine());
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetGrainlineMainLine() const -> QLineF
{
    return d->m_grainline.GetMainLine();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::IsGrainlineEnabled() const -> bool
{
    return d->m_grainline.IsEnabled();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::IsGrainlineVisible() const -> bool
{
    return d->m_grainline.IsVisible();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMatrix() const -> QTransform
{
    return d->m_matrix;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetMatrix(const QTransform &matrix)
{
    d->m_matrix = matrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetLayoutWidth() const -> qreal
{
    return d->m_layoutWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetLayoutWidth(qreal value)
{
    d->m_layoutWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetQuantity() const -> quint16
{
    return d->m_quantity;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetQuantity(quint16 value)
{
    d->m_quantity = qMax(static_cast<quint16>(1), value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetId() const -> vidtype
{
    return d->m_id;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetId(vidtype id)
{
    d->m_id = id;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Translate(qreal dx, qreal dy)
{
    Translate(QPointF(dx, dy));
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Translate(const QPointF &p)
{
    QTransform m;
    m.translate(p.x(), p.y());
    d->m_matrix *= m;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Scale(qreal sx, qreal sy)
{
    d->m_xScale *= sx;
    d->m_yScale *= sy;

    QTransform m;
    m.scale(sx, sy);
    d->m_matrix *= m;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Rotate(const QPointF &originPoint, qreal degrees)
{
    QTransform m;
    m.translate(originPoint.x(), originPoint.y());
    m.rotate(-degrees);
    m.translate(-originPoint.x(), -originPoint.y());
    d->m_matrix *= m;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Mirror(const QLineF &edge)
{
    if (edge.isNull())
    {
        return;
    }

    const auto axis = QLineF(edge.x2(), edge.y2(), edge.x2() + 100, edge.y2()); // Ox axis

    const qreal angle = edge.angleTo(axis);
    const QPointF p2 = edge.p2();
    QTransform m;
    m.translate(p2.x(), p2.y());
    m.rotate(-angle);
    m.translate(-p2.x(), -p2.y());
    d->m_matrix *= m;

    m.reset();
    m.translate(p2.x(), p2.y());
    m.scale(m.m11(), m.m22() * -1);
    m.translate(-p2.x(), -p2.y());
    d->m_matrix *= m;

    m.reset();
    m.translate(p2.x(), p2.y());
    m.rotate(-(360 - angle));
    m.translate(-p2.x(), -p2.y());
    d->m_matrix *= m;

    d->m_verticallyFlipped = !d->m_verticallyFlipped;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Mirror()
{
    QTransform m;
    m.scale(-1, 1);
    d->m_matrix *= m;
    d->m_verticallyFlipped = !d->m_verticallyFlipped;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::DetailEdgesCount() const -> vsizetype
{
    return DetailPath().count();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::LayoutEdgesCount() const -> vsizetype
{
    const auto count = d->m_layoutAllowance.count();
    return count > 2 ? count : 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::LayoutEdge(int i) const -> QLineF
{
    return Edge(d->m_layoutAllowance, i);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::LayoutEdgeByPoint(const QPointF &p1) const -> EdgeIndex
{
    return EdgeByPoint(d->m_layoutAllowance, p1);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MappedDetailBoundingRect() const -> QRectF
{
    QVector<QPointF> points;
    CastTo(GetMappedExternalContourPoints(), points);
    return BoundingRect(points);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::DetailBoundingRect() const -> QRectF
{
    QVector<QPointF> points;
    CastTo(GetExternalContourPoints(), points);
    return BoundingRect(points);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MappedLayoutBoundingRect() const -> QRectF
{
    return BoundingRect(GetMappedLayoutAllowancePoints());
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::Diagonal() const -> qreal
{
    const QRectF rec = MappedLayoutBoundingRect();
    return qSqrt(pow(rec.height(), 2) + pow(rec.width(), 2));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::FoldLine() const -> VFoldLine
{
    QLineF foldLine = IsHideMainPath() ? GetSeamAllowanceMirrorLine() : GetSeamMirrorLine();
    VFoldLine fLine(foldLine, GetFoldLineType());
    fLine.SetLabelSvgFontSize(GetFoldLineSvgFontSize());
    fLine.SetLabelFontItalic(IsFoldLineLabelFontItalic());
    fLine.SetLabelFontBold(IsFoldLineLabelFontBold());
    fLine.SetLabel(GetFoldLineLabel());
    fLine.SetLabelAlignment(GetFoldLineLabelAlignment());
    fLine.SetHeight(GetFoldLineHeight());
    fLine.SetWidth(GetFoldLineWidth());
    fLine.SetCenterPosition(GetFoldLineCenterPosition());
    fLine.SetMatrix(GetMatrix());
    fLine.SetHorizontallyFlipped(IsHorizontallyFlipped());
    fLine.SetVerticallyFlipped(IsVerticallyFlipped());
    fLine.SetXScale(GetXScale());
    fLine.SetYScale(GetYScale());
    fLine.SetOutlineFont(GetFoldLineOutlineFont());
    fLine.SetSvgFont(GetFoldLineSVGFontFamily());

    return fLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::isNull() const -> bool
{
    if (not d->m_contour.isEmpty() && d->m_layoutWidth > 0)
    {
        return !IsSeamAllowance() || IsSeamAllowanceBuiltIn() || d->m_seamAllowance.isEmpty();
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::Square() const -> qint64
{
    return d->m_square;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetLayoutAllowancePoints(bool togetherWithNotches)
{
    d->m_square = 0;

    if (d->m_layoutWidth > 0)
    {
        QVector<VLayoutPoint> const pieceBoundary = IsSeamAllowance() && not IsSeamAllowanceBuiltIn()
                                                        ? GetMappedFullSeamAllowancePoints(togetherWithNotches,
                                                                                           false,
                                                                                           true)
                                                        : GetMappedFullContourPoints(togetherWithNotches, false, true);

        QVector<VSAPoint> pieceBoundaryPoints;
        CastTo(pieceBoundary, pieceBoundaryPoints);
        CastTo(Equidistant(pieceBoundaryPoints, d->m_layoutWidth, false, GetName()), d->m_layoutAllowance);
        if (not d->m_layoutAllowance.isEmpty())
        {
            d->m_layoutAllowance.removeLast();
        }

        QVector<QPointF> points;
        CastTo(IsSeamAllowance() && not IsSeamAllowanceBuiltIn() ? GetSeamAllowancePoints() : GetContourPoints(),
               points);
        d->m_square = qFloor(qAbs(SumTrapezoids(points) / 2.0));
    }
    else
    {
        d->m_layoutAllowance.clear();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedExternalContourPoints() const -> QVector<VLayoutPoint>
{
    return IsSeamAllowance() && not IsSeamAllowanceBuiltIn() ? GetMappedFullSeamAllowancePoints()
                                                             : GetMappedFullContourPoints();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetExternalContourPoints() const -> QVector<VLayoutPoint>
{
    return IsSeamAllowance() && not IsSeamAllowanceBuiltIn() ? GetFullSeamAllowancePoints() : GetFullContourPoints();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMappedPassmarks() const -> QVector<VLayoutPassmark>
{
    return Map(d->m_passmarks);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPassmarks() const -> QVector<VLayoutPassmark>
{
    return d->m_passmarks;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPassmarks(const QVector<VLayoutPassmark> &passmarks)
{
    if (IsSeamAllowance())
    {
        d->m_passmarks = passmarks;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetPlaceLabels() const -> QVector<VLayoutPlaceLabel>
{
    return d->m_placeLabels;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetPlaceLabels(const QVector<VLayoutPlaceLabel> &labels)
{
    d->m_placeLabels = labels;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MappedInternalPathsForCut(bool cut) const -> QVector<VLayoutPiecePath>
{
    QVector<VLayoutPiecePath> paths;
    paths.reserve(d->m_internalPaths.size());

    for (const auto &path : d->m_internalPaths)
    {
        if (path.IsCutPath() == cut)
        {
            VLayoutPiecePath iPath = path;
            iPath.SetPoints(Map(path.Points()));
            paths.append(iPath);
        }
    }

    return paths;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetInternalPaths() const -> QVector<VLayoutPiecePath>
{
    return d->m_internalPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetInternalPaths(const QVector<VLayoutPiecePath> &internalPaths)
{
    d->m_internalPaths = internalPaths;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MappedContourPath(bool togetherWithNotches, bool showLayoutAllowance) const -> QPainterPath
{
    return d->m_matrix.map(ContourPath(togetherWithNotches, showLayoutAllowance));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::ContourPath(bool togetherWithNotches, bool showLayoutAllowance) const -> QPainterPath
{
    QPainterPath path;

    // sew line
    if (not IsHideMainPath() || not IsSeamAllowance() || IsSeamAllowanceBuiltIn())
    {
        QVector<VLayoutPoint> points = GetFullContourPoints(togetherWithNotches, true, false);
        path = VGObject::PainterPath(points);
    }

    // seam allowance
    if (IsSeamAllowance())
    {
        if (not IsSeamAllowanceBuiltIn())
        {
            QVector<VLayoutPoint> points = GetFullSeamAllowancePoints(togetherWithNotches, true, false);
            if (points.constLast().toPoint() != points.constFirst().toPoint())
            {
                points.append(points.at(0)); // Should be always closed
            }

            path.addPath(VGObject::PainterPath(points));
        }

        if (!togetherWithNotches)
        {
            // Draw passmarks
            for (const auto &passmark : d->m_passmarks)
            {
                QPainterPath passmaksPath;
                for (const auto &line : passmark.lines)
                {
                    passmaksPath.moveTo(line.p1());
                    passmaksPath.lineTo(line.p2());
                }
                path.addPath(passmaksPath);

                if (!d->m_seamAllowanceMirrorLine.isNull() && IsShowFullPiece()
                    && !IsPointOnLineviaPDP(passmark.baseLine.p1(),
                                            d->m_seamAllowanceMirrorLine.p1(),
                                            d->m_seamAllowanceMirrorLine.p2())
                    && !passmark.notMirrored)
                {
                    QPainterPath mirroredPassmaksPath;
                    for (const auto &line : passmark.lines)
                    {
                        mirroredPassmaksPath.moveTo(line.p1());
                        mirroredPassmaksPath.lineTo(line.p2());
                    }
                    const QTransform matrix = VGObject::FlippingMatrix(d->m_seamAllowanceMirrorLine);
                    path.addPath(matrix.map(mirroredPassmaksPath));
                }
            }
        }

        path.setFillRule(Qt::WindingFill);
    }

    if (showLayoutAllowance)
    {
        path.addPath(VGObject::PainterPath(d->m_layoutAllowance));
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MappedLayoutAllowancePath() const -> QPainterPath
{
    return VGObject::PainterPath(GetMappedLayoutAllowancePoints());
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::DrawMiniature(QPainter &painter, bool togetherWithNotches) const
{
    QTransform m;

    if (IsForceFlipping())
    {
        QRectF rect = DetailBoundingRect();
        QPointF const center = rect.center();

        m.translate(center.x(), 0);
        m.scale(-1, 1);
        m.translate(-center.x(), 0);
    }

    painter.drawPath(m.map(ContourPath(togetherWithNotches, false)));

    for (const auto &path : d->m_internalPaths)
    {
        QPainterStateGuard const guard(&painter);

        QPen pen = painter.pen();
        pen.setStyle(path.PenStyle());
        painter.setPen(pen);

        painter.drawPath(m.map(path.GetPainterPath()));
    }

    for (const auto &label : d->m_placeLabels)
    {
        painter.drawPath(m.map(LabelShapePath(label)));
    }

    painter.drawPath(m.map(VLayoutPiece::GrainlinePath(GetGrainlineShape())));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetItem(bool textAsPaths, bool togetherWithNotches, bool showLayoutAllowance) const
    -> QGraphicsItem *
{
    QGraphicsPathItem *item = GetMainItem(togetherWithNotches, showLayoutAllowance);

    for (const auto &path : d->m_internalPaths)
    {
        auto *pathItem = new QGraphicsPathItem(item);

        QPainterPath p = d->m_matrix.map(path.GetPainterPath());

        if (!path.IsNotMirrored() && IsShowFullPiece() && !d->m_seamMirrorLine.isNull())
        {
            QVector<VLayoutPoint> points = path.Points();
            const QTransform matrix = VGObject::FlippingMatrix(d->m_seamMirrorLine);
            std::transform(points.begin(), points.end(), points.begin(),
                           [&matrix](const VLayoutPoint &point) { return VAbstractPiece::MapPoint(point, matrix); });
            QVector<QPointF> casted;
            CastTo(points, casted);
            p.addPath(d->m_matrix.map(VPiecePath::MakePainterPath(casted)));
        }

        pathItem->setPath(p);

        QPen pen = pathItem->pen();
        pen.setStyle(path.PenStyle());
        pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
        pathItem->setPen(pen);
    }

    for (const auto &label : d->m_placeLabels)
    {
        auto *pathItem = new QGraphicsPathItem(item);
        QPen pen = pathItem->pen();
        pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
        pathItem->setPen(pen);

        QPainterPath path;
        path.addPath(d->m_matrix.map(LabelShapePath(label)));

        if (!label.IsNotMirrored() && IsShowFullPiece() && !d->m_seamMirrorLine.isNull())
        {
            PlaceLabelImg shape = VAbstractPiece::PlaceLabelShape(label);
            const QTransform matrix = VGObject::FlippingMatrix(d->m_seamMirrorLine);
            for (auto &points : shape)
            {
                std::transform(points.begin(), points.end(), points.begin(),
                               [&matrix](const VLayoutPoint &point) { return MapPoint(point, matrix); });
            }

            path.addPath(d->m_matrix.map(LabelShapePath(shape)));
        }

        pathItem->setPath(path);
    }

    CreateLabelStrings(item, d->m_detailLabel, d->m_tmDetail, textAsPaths);
    CreateLabelStrings(item, d->m_patternInfo, d->m_tmPattern, textAsPaths);
    CreateGrainlineItem(item);
    CreateFoldLineItem(item, textAsPaths);

    if (IsShowFullPiece())
    {
        if (const QLineF mirrorLine = GetMappedCorrectedMirrorLine(togetherWithNotches); !mirrorLine.isNull())
        {
            QPainterPath mirrorLinePath;
            mirrorLinePath.moveTo(mirrorLine.p1());
            mirrorLinePath.lineTo(mirrorLine.p2());

            auto *mirrorLineItem = new QGraphicsPathItem(item);
            QPen pen = mirrorLineItem->pen();
            pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
            pen.setStyle(Qt::DashDotLine);
            mirrorLineItem->setPen(pen);
            mirrorLineItem->setPath(mirrorLinePath);
        }
    }

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::IsLayoutAllowanceValid(bool togetherWithNotches) const -> bool
{
    QVector<VLayoutPoint> base = (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
                                     ? GetFullSeamAllowancePoints(togetherWithNotches, false, true)
                                     : GetFullContourPoints(togetherWithNotches, false, true);

    if (togetherWithNotches)
    {
        base = VAbstractPiece::RemoveDublicates(base, false);
    }

    QVector<QPointF> points;
    CastTo(base, points);
    return VAbstractPiece::IsAllowanceValid(points, d->m_layoutAllowance);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::BiggestEdge() const -> qreal
{
    qreal edge = 0;

    if (LayoutEdgesCount() < 1)
    {
        return edge;
    }

    for (int i = 1; i < LayoutEdgesCount(); ++i)
    {
        const qreal length = LayoutEdge(i).length();
        if (length > edge)
        {
            edge = length;
        }
    }

    return edge;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MapPlaceLabelShape(PlaceLabelImg shape) const -> PlaceLabelImg
{
    for (auto &i : shape)
    {
        i = Map(i);
    }

    return shape;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::MapPassmark(VLayoutPassmark passmark, const QTransform &matrix, bool mirror) -> VLayoutPassmark
{
    passmark.lines = MapVector(passmark.lines, matrix, mirror);
    passmark.baseLine = matrix.map(passmark.baseLine);
    return passmark;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::LineMatrix(const QPointF &topLeft, qreal angle, const QPointF &linePos, qreal maxLineWidth) const
    -> QTransform
{
    QTransform labelMatrix;
    labelMatrix.translate(topLeft.x(), topLeft.y());

    if ((IsVerticallyFlipped() && IsHorizontallyFlipped()) || (!IsVerticallyFlipped() && !IsHorizontallyFlipped()))
    {
        labelMatrix.rotate(angle);
    }
    else if (IsVerticallyFlipped() || IsHorizontallyFlipped())
    {
        if (IsVerticallyFlipped() && !IsHorizontallyFlipped())
        {
            labelMatrix.scale(-1, 1);
            labelMatrix.rotate(-angle);
            labelMatrix.translate(-maxLineWidth, 0);
        }

        if (IsHorizontallyFlipped() && !IsVerticallyFlipped())
        {
            labelMatrix.scale(-1, 1);
            labelMatrix.rotate(-angle);
            labelMatrix.translate(-maxLineWidth, 0);
        }
    }

    labelMatrix.translate(linePos.x(), linePos.y()); // Each string has own position
    labelMatrix *= GetMatrix();

    return labelMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::BoundingRect(QVector<QPointF> points) -> QRectF
{
    points.append(points.constFirst());
    return QPolygonF(points).boundingRect();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GrainlinePath(const GrainlineShape &shape) -> QPainterPath
{
    QPainterPath shapePath;
    for (auto subShape : shape)
    {
        QPainterPath path;
        path.moveTo(subShape.at(0));
        for (auto p : std::as_const(subShape))
        {
            path.lineTo(p);
        }
        shapePath.addPath(path);
    }
    return shapePath;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::LabelStringsSVGFont(QGraphicsItem *parent, const QVector<QPointF> &labelShape,
                                       const VTextManager &tm, bool textAsPaths) const
{
    SCASSERT(parent != nullptr)

    if (labelShape.count() <= 2)
    {
        return;
    }

    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine =
        db->FontEngine(tm.GetSVGFontFamily(), SVGFontStyle::Normal, SVGFontWeight::Normal, tm.GetSVGFontPointSize());

    VSvgFont const svgFont = engine.Font();
    if (!svgFont.IsValid())
    {
        QString const errorMsg = tr("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        LabelStringsOutlineFont(parent, labelShape, tm, textAsPaths);
        return;
    }

    qreal const penWidth = VAbstractApplication::VApp()->Settings()->WidthHairLine();

    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = -QLineF(labelShape.at(0), labelShape.at(1)).angle();
    qreal dY = penWidth;

    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), svgFont, penWidth);

    for (const auto &tl : labelLines)
    {
        VSvgFont lineFont = svgFont;
        lineFont.SetPointSize(svgFont.PointSize() + tl.iFontSize);
        lineFont.SetBold(tl.bold);
        lineFont.SetItalic(tl.italic);

        engine = db->FontEngine(lineFont);

        if (dY + engine.FontHeight() + penWidth > dH)
        {
            break;
        }

        qreal dX = 0;
        if (tl.eAlign == 0 || (tl.eAlign & Qt::AlignLeft) > 0)
        {
            dX = 0;
        }
        else if ((tl.eAlign & Qt::AlignHCenter) > 0)
        {
            dX = (dW - engine.TextWidth(tl.qsText, penWidth)) / 2;
        }
        else if ((tl.eAlign & Qt::AlignRight) > 0)
        {
            dX = dW - engine.TextWidth(tl.qsText, penWidth);
        }

        // set up the rotation around top-left corner matrix
        QTransform labelMatrix;
        labelMatrix.translate(labelShape.at(0).x(), labelShape.at(0).y());
        if (d->m_verticallyFlipped)
        {
            labelMatrix.scale(-1, 1);
            labelMatrix.rotate(-angle);
            labelMatrix.translate(-dW, 0);
            labelMatrix.translate(dX, dY); // Each string has own position
        }
        else
        {
            labelMatrix.rotate(angle);
            labelMatrix.translate(dX, dY); // Each string has own position
        }

        labelMatrix *= d->m_matrix;

        auto *item = new QGraphicsPathItem(parent);

        QPen pen = item->pen();
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setWidthF(penWidth);
        item->setPen(pen);

        item->setPath(engine.DrawPath(QPointF(), tl.qsText));
        item->setBrush(QBrush(Qt::NoBrush));
        item->setTransform(labelMatrix);

        dY += engine.FontHeight() - penWidth * 2 + tm.GetSpacing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::LabelStringsOutlineFont(QGraphicsItem *parent, const QVector<QPointF> &labelShape,
                                           const VTextManager &tm, bool textAsPaths) const
{
    SCASSERT(parent != nullptr)

    if (labelShape.count() <= 2)
    {
        return;
    }

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = -QLineF(labelShape.at(0), labelShape.at(1)).angle();
    qreal const penWidth = settings->WidthHairLine();
    qreal dY = 0;

    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), tm.GetFont());

    if (settings->GetSingleStrokeOutlineFont())
    {
        textAsPaths = true;

        dY += penWidth;
    }

    for (const auto &tl : labelLines)
    {
        QFont fnt = tm.GetFont();
        fnt.setPointSize(qMax(tm.GetFont().pointSize() + tl.iFontSize, 1));
        if (!settings->GetSingleStrokeOutlineFont())
        {
            fnt.setBold(tl.bold);
        }
        fnt.setItalic(tl.italic);

        VSingleLineOutlineChar const corrector(fnt);
        if (settings->GetSingleStrokeOutlineFont() && !corrector.IsPopulated())
        {
            corrector.LoadCorrections(settings->GetPathFontCorrections());
        }

        QFontMetrics const fm(fnt);

        if (dY + fm.height() > dH)
        {
            break;
        }

        qreal dX = 0;
        if (tl.eAlign == 0 || (tl.eAlign & Qt::AlignLeft) > 0)
        {
            dX = 0;
        }
        else if ((tl.eAlign & Qt::AlignHCenter) > 0)
        {
            dX = (dW - fm.horizontalAdvance(tl.qsText)) / 2;
        }
        else if ((tl.eAlign & Qt::AlignRight) > 0)
        {
            dX = dW - fm.horizontalAdvance(tl.qsText);
        }

        // set up the rotation around top-left corner matrix
        QTransform labelMatrix;
        labelMatrix.translate(labelShape.at(0).x(), labelShape.at(0).y());
        if (d->m_verticallyFlipped)
        {
            labelMatrix.scale(-1, 1);
            labelMatrix.rotate(-angle);
            labelMatrix.translate(-qRound(dW), 0);
        }
        else
        {
            labelMatrix.rotate(angle);
        }

        labelMatrix.translate(dX, dY); // Each string has own position
        labelMatrix *= d->m_matrix;

        if (textAsPaths)
        {
            QPainterPath path;

            if (settings->GetSingleStrokeOutlineFont())
            {
                int w = 0;
                for (auto c : std::as_const(tl.qsText))
                {
                    path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
                    w += fm.horizontalAdvance(c);
                }
            }
            else
            {
                path.addText(0, static_cast<qreal>(fm.ascent()), fnt, tl.qsText);
            }

            auto *item = new QGraphicsPathItem(parent);
            item->setPath(path);

            QPen itemPen = item->pen();
            itemPen.setColor(Qt::black);
            itemPen.setCapStyle(Qt::RoundCap);
            itemPen.setJoinStyle(Qt::RoundJoin);
            itemPen.setWidthF(penWidth);
            item->setPen(itemPen);
            item->setBrush(settings->GetSingleStrokeOutlineFont() ? QBrush(Qt::NoBrush) : QBrush(Qt::black));
            item->setTransform(labelMatrix);

            dY += fm.height() + penWidth * 2 + MmToPixel(1.5) + tm.GetSpacing();
        }
        else
        {
            auto *item = new QGraphicsSimpleTextItem(parent);
            item->setFont(fnt);
            item->setText(tl.qsText);
            item->setTransform(labelMatrix);

            dY += (fm.height() + MmToPixel(1.5) + tm.GetSpacing());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::CreateLabelStrings(QGraphicsItem *parent, const QVector<QPointF> &labelShape, const VTextManager &tm,
                                      bool textAsPaths) const
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (settings->GetSingleLineFonts())
    {
        LabelStringsSVGFont(parent, labelShape, tm, textAsPaths);
    }
    else
    {
        LabelStringsOutlineFont(parent, labelShape, tm, textAsPaths);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::CreateGrainlineItem(QGraphicsItem *parent) const
{
    SCASSERT(parent != nullptr)

    if (!d->m_grainline.IsEnabled() || !d->m_grainline.IsVisible())
    {
        return;
    }

    auto *item = new VGraphicsFillItem(VLayoutPiece::GrainlinePath(GetMappedGrainlineShape()), parent);
    item->SetCustomPen(true);
    QPen pen = item->pen();
    pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    item->setPen(pen);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::CreateFoldLineItem(QGraphicsItem *parent, bool textAsPaths) const
{
    SCASSERT(parent != nullptr)

    if (GetFoldLineType() == FoldLineType::None || (IsShowFullPiece() && !IsShowMirrorLine()))
    {
        return;
    }

    VFoldLine const fLine = FoldLine();

    QVector<QPainterPath> const shape = fLine.FoldLinePath();

    if (shape.isEmpty())
    {
        return;
    }

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (!textAsPaths && !settings->GetSingleStrokeOutlineFont() && !settings->GetSingleLineFonts())
    {
        auto *item = new QGraphicsSimpleTextItem(parent);
        fLine.UpdateFoldLineLabel(item);
    }

    auto FoldLineMark = [parent](const QPainterPath &shape)
    {
        auto *item = new VGraphicsFoldLineItem(shape, parent);
        item->SetCustomPen(true);
        QPen pen = item->pen();
        pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        item->setPen(pen);
    };

    auto FoldLineLabel = [parent](const QPainterPath &shape)
    {
        auto *item = new QGraphicsPathItem(parent);
        item->setPath(shape);

        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

        QPen itemPen = item->pen();
        itemPen.setColor(Qt::black);
        itemPen.setCapStyle(Qt::RoundCap);
        itemPen.setJoinStyle(Qt::RoundJoin);
        itemPen.setWidthF(settings->WidthHairLine());
        item->setPen(itemPen);
        item->setBrush(settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts() ? QBrush(Qt::NoBrush)
                                                                                                : QBrush(Qt::black));
    };

    if (GetFoldLineType() == FoldLineType::ThreeDots || GetFoldLineType() == FoldLineType::ThreeX ||
        GetFoldLineType() == FoldLineType::TwoArrows)
    {
        FoldLineMark(shape.constFirst());
    }
    else if (GetFoldLineType() == FoldLineType::Text)
    {
        if (settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts())
        {
            FoldLineLabel(shape.constFirst());
        }
    }
    else
    {
        FoldLineMark(shape.constFirst());

        if (shape.size() > 1 && (settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts()))
        {
            FoldLineLabel(shape.constLast());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::DetailPath() const -> QVector<VLayoutPoint>
{
    if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
    {
        return d->m_seamAllowance;
    }

    return d->m_contour;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetMainItem(bool togetherWithNotches, bool showLayoutAllowance) const -> QGraphicsPathItem *
{
    auto *item = new QGraphicsPathItem();
    QPen pen = item->pen();
    pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
    item->setPen(pen);
    item->setPath(MappedContourPath(togetherWithNotches, showLayoutAllowance));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetExternalContourPathItem() const -> QGraphicsPathItem *
{
    auto *item = new QGraphicsPathItem();
    QPen pen = item->pen();
    pen.setWidthF(VAbstractApplication::VApp()->Settings()->WidthHairLine());
    item->setPen(pen);

    QPainterPath path;

    // contour
    QVector<VLayoutPoint> const points = GetMappedExternalContourPoints();

    path.moveTo(points.at(0));
    for (qint32 i = 1; i < points.count(); ++i)
    {
        path.lineTo(points.at(i));
    }
    path.lineTo(points.at(0));

    item->setPath(path);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::IsVerticallyFlipped() const -> bool
{
    return d->m_verticallyFlipped;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetVerticallyFlipped(bool value)
{
    d->m_verticallyFlipped = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::IsHorizontallyFlipped() const -> bool
{
    return d->m_horizontallyFlipped;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetHorizontallyFlipped(bool value)
{
    d->m_horizontallyFlipped = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetGradationId(const QString &id)
{
    d->m_gradationId = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetGradationId() const -> QString
{
    return d->m_gradationId;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetXScale() const -> qreal
{
    return d->m_xScale;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetXScale(qreal xs)
{
    d->m_xScale = xs;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::GetYScale() const -> qreal
{
    return d->m_yScale;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetYScale(qreal ys)
{
    d->m_yScale = ys;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::Edge(const QVector<QPointF> &path, int i) const -> QLineF
{
    if (i < 1)
    { // Doesn't exist such edge
        return {};
    }

    vsizetype i1, i2;
    if (i < path.count())
    {
        i1 = i - 1;
        i2 = i;
    }
    else
    {
        i1 = path.count() - 1;
        i2 = 0;
    }

    if (d->m_verticallyFlipped || d->m_horizontallyFlipped)
    {
        QVector<QPointF> const newPath = Map(path);
        return {newPath.at(i1), newPath.at(i2)};
    }
    return {d->m_matrix.map(path.at(i1)), d->m_matrix.map(path.at(i2))};
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::EdgeByPoint(const QVector<QPointF> &path, const QPointF &p1) const -> EdgeIndex
{
    if (p1.isNull() || path.count() < 3)
    {
        return {};
    }

    const QVector<QPointF> points = Map(path);
    auto posIter = std::find_if(points.cbegin(), points.cend(),
                                [&p1](const QPointF &point) { return VFuzzyComparePoints(point, p1); });
    if (posIter != points.cend())
    {
        return static_cast<int>(posIter - points.cbegin() + 1);
    }
    return {}; // Did not find edge
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto VLayoutPiece::Map(QVector<T> points) const -> QVector<T>
{
    return MapVector(points, d->m_matrix, d->m_verticallyFlipped || d->m_horizontallyFlipped);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiece::ConvertPassmarks(const VPiece &piece, const VContainer *pattern) -> QVector<VLayoutPassmark>
{
    const QVector<VPassmark> passmarks = piece.Passmarks(pattern);
    QVector<VLayoutPassmark> layoutPassmarks;
    layoutPassmarks.reserve(passmarks.size());

    for (const auto &passmark : passmarks)
    {
        if (passmark.IsNull())
        {
            continue;
        }

        auto AddPassmark = [&passmark, &piece, pattern, &layoutPassmarks](PassmarkSide side)
        {
            bool ok = false;
            VLayoutPassmark const layoutPassmark = PrepareSAPassmark(piece, pattern, passmark, side, ok);
            if (ok)
            {
                layoutPassmarks.append(layoutPassmark);
            }
        };

        auto AddBuiltInPassmark = [&passmark, &piece, pattern, &layoutPassmarks]()
        {
            bool ok = false;
            VLayoutPassmark const layoutPassmark = PreapreBuiltInSAPassmark(piece, pattern, passmark, ok);
            if (ok)
            {
                layoutPassmarks.append(layoutPassmark);
            }
        };

        if (piece.IsSeamAllowanceBuiltIn())
        {
            AddBuiltInPassmark();
            continue;
        }

        VPiecePassmarkData const pData = passmark.Data();

        switch (pData.passmarkAngleType)
        {
            case PassmarkAngleType::Straightforward:
            case PassmarkAngleType::Bisector:
                AddPassmark(PassmarkSide::All);
                break;
            case PassmarkAngleType::Intersection:
            case PassmarkAngleType::Intersection2:
                AddPassmark(PassmarkSide::Left);
                AddPassmark(PassmarkSide::Right);
                break;
            case PassmarkAngleType::IntersectionOnlyLeft:
            case PassmarkAngleType::Intersection2OnlyLeft:
                AddPassmark(PassmarkSide::Left);
                break;
            case PassmarkAngleType::IntersectionOnlyRight:
            case PassmarkAngleType::Intersection2OnlyRight:
                AddPassmark(PassmarkSide::Right);
                break;
            default:
                break;
        }

        if (VAbstractApplication::VApp()->Settings()->IsDoublePassmark() &&
            (VAbstractApplication::VApp()->Settings()->IsPieceShowMainPath() || not piece.IsHideMainPath()) &&
            pData.isMainPathNode && pData.passmarkAngleType != PassmarkAngleType::Intersection &&
            pData.passmarkAngleType != PassmarkAngleType::IntersectionOnlyLeft &&
            pData.passmarkAngleType != PassmarkAngleType::IntersectionOnlyRight &&
            pData.passmarkAngleType != PassmarkAngleType::Intersection2 &&
            pData.passmarkAngleType != PassmarkAngleType::Intersection2OnlyLeft &&
            pData.passmarkAngleType != PassmarkAngleType::Intersection2OnlyRight && pData.isShowSecondPassmark)
        {
            AddBuiltInPassmark();
        }
    }

    return layoutPassmarks;
}
