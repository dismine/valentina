/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vtoolseamallowance.h"
#include "../dialogs/tools/piece/dialogduplicatedetail.h"
#include "../dialogs/tools/piece/dialogseamallowance.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/xml/vlabeltemplateconverter.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../qmuparser/qmutokenparser.h"
#include "../undocommands/addpiece.h"
#include "../undocommands/deletepiece.h"
#include "../undocommands/movepiece.h"
#include "../undocommands/savepieceoptions.h"
#include "../undocommands/togglepiecestate.h"
#include "../vformat/vlabeltemplate.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vplacelabelitem.h"
#include "../vgeometry/vpointf.h"
#include "../vlayout/vboundary.h"
#include "../vlayout/vfoldline.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vmisc/def.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vwidgets/vnobrushscalepathitem.h"
#include "../vwidgets/vpiecegrainline.h"
#include "nodeDetails/vnodearc.h"
#include "nodeDetails/vnodeellipticalarc.h"
#include "nodeDetails/vnodepoint.h"
#include "nodeDetails/vnodespline.h"
#include "nodeDetails/vnodesplinepath.h"
#include "nodeDetails/vtoolpiecepath.h"
#include "nodeDetails/vtoolpin.h"
#include "nodeDetails/vtoolplacelabel.h"
#include "qpainterpath.h"
#include "toolsdef.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainterPath>
#include <QTimer>
#include <QUuid>
#include <QtConcurrent/QtConcurrentRun>
#include <QtMath>
#include <chrono>
#include <memory>

using namespace std::chrono_literals;

// Current version of seam allowance tag need for backward compatibility
const quint8 VToolSeamAllowance::pieceVersion = 2;

const QString VToolSeamAllowance::TagCSA = QStringLiteral("csa");                 // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::TagRecord = QStringLiteral("record");           // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::TagIPaths = QStringLiteral("iPaths");           // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::TagPins = QStringLiteral("pins");               // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::TagPlaceLabels = QStringLiteral("placeLabels"); // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::TagMirrorLine = QStringLiteral("mirrorLine");   // NOLINT(cert-err58-cpp)

const QString VToolSeamAllowance::AttrSeamAllowance = QStringLiteral("seamAllowance"); // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrHideMainPath = QStringLiteral("hideMainPath");   // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrShowFullPiece = QStringLiteral("showFullPiece"); // NOLINT(cert-err58-cpp)
// NOLINTNEXTLINE(cert-err58-cpp)
const QString VToolSeamAllowance::AttrSeamAllowanceBuiltIn = QStringLiteral("seamAllowanceBuiltIn");
const QString VToolSeamAllowance::AttrUnited = QStringLiteral("united");                 // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrFont = QStringLiteral("fontSize");                 // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrTopLeftPin = QStringLiteral("topLeftPin");         // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrBottomRightPin = QStringLiteral("bottomRightPin"); // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrCenterPin = QStringLiteral("centerPin");           // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrTopPin = QStringLiteral("topPin");                 // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrBottomPin = QStringLiteral("bottomPin");           // NOLINT(cert-err58-cpp)
const QString VToolSeamAllowance::AttrPiecePriority = QStringLiteral("priority");        // NOLINT(cert-err58-cpp)

namespace
{
//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto FixLabelPins(T itemData, const QMap<quint32, quint32> &mappedPins) -> T
{
    itemData.SetCenterPin(mappedPins.value(itemData.CenterPin(), NULL_ID));
    itemData.SetTopLeftPin(mappedPins.value(itemData.TopLeftPin(), NULL_ID));
    itemData.SetBottomRightPin(mappedPins.value(itemData.BottomRightPin(), NULL_ID));
    return itemData;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto FixGrainlinePins(T itemData, const QMap<quint32, quint32> &mappedPins) -> T
{
    itemData.SetCenterPin(mappedPins.value(itemData.CenterPin(), NULL_ID));
    itemData.SetTopPin(mappedPins.value(itemData.TopPin(), NULL_ID));
    itemData.SetBottomPin(mappedPins.value(itemData.BottomPin(), NULL_ID));
    return itemData;
}

//---------------------------------------------------------------------------------------------------------------------
auto DuplicatePins(const QVector<quint32> &pins, const VToolSeamAllowanceInitData &initData) -> QMap<quint32, quint32>
{
    QMap<quint32, quint32> newPins;
    for (auto p : pins)
    {
        QSharedPointer<VPointF> const pin = initData.data->GeometricObject<VPointF>(p);

        auto *tool = qobject_cast<VAbstractNode *>(VAbstractPattern::getTool(p));
        SCASSERT(tool != nullptr)

        VToolPinInitData initNodeData;
        initNodeData.id = initData.data->AddGObject(new VPointF(*pin));
        initNodeData.pointId = pin->getIdObject();
        initNodeData.idObject = NULL_ID; // piece id
        initNodeData.doc = initData.doc;
        initNodeData.data = initData.data;
        initNodeData.parse = Document::FullParse;
        initNodeData.typeCreation = Source::FromTool;
        initNodeData.drawName = initData.drawName;
        initNodeData.idTool = tool->GetIdTool();

        VToolPin::Create(initNodeData);
        newPins.insert(p, initNodeData.id);
    }
    return newPins;
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateLabelItem(VTextGraphicsItem *labelItem, QPointF pos, qreal labelAngle)
{
    SCASSERT(labelItem != nullptr)

    QRectF rectBB;
    rectBB.setTopLeft(pos);
    rectBB.setWidth(labelItem->boundingRect().width());
    rectBB.setHeight(labelItem->boundingRect().height());
    qreal dX;
    qreal dY;
    if (!labelItem->IsContained(rectBB, labelAngle, dX, dY))
    {
        pos.setX(pos.x() + dX);
        pos.setY(pos.y() + dY);
    }

    labelItem->setPos(pos);
    labelItem->setRotation(-labelAngle); // expects clockwise direction
    labelItem->Update();
    labelItem->GetTextLines() > 0 ? labelItem->show() : labelItem->hide();
}

//---------------------------------------------------------------------------------------------------------------------
auto SeamPoints(const VPiece &detail, bool combineTogether, const VContainer *data) -> QVector<VLayoutPoint>
{
    if (combineTogether)
    {
        const QVector<VLayoutPassmark> passmarks = VLayoutPiece::ConvertPassmarks(detail, data);
        const QVector<VLayoutPoint> points = detail.FullMainPathPoints(data);

        bool const seamAllowance = detail.IsSeamAllowance() && detail.IsSeamAllowanceBuiltIn();
        bool const builtInSeamAllowance = detail.IsSeamAllowance() && detail.IsSeamAllowanceBuiltIn();

        // DumpVector(points, QStringLiteral("points.json.XXXXXX")); // Uncomment for dumping test data
        // DumpVector(passmarks, QStringLiteral("passmarks.json.XXXXXX")); // Uncomment for dumping test data

        VBoundary boundary(points, seamAllowance, builtInSeamAllowance);
        boundary.SetPieceName(detail.GetName());

        if (QLineF const mirrorLine = detail.SeamMirrorLine(data); !mirrorLine.isNull() && detail.IsShowFullPiece())
        {
            boundary.SetMirrorLine(mirrorLine);
        }
        const QList<VBoundarySequenceItemData> sequence = boundary.Combine(passmarks, false, false);

        QVector<VLayoutPoint> combinedBoundary;
        for (const auto &item : sequence)
        {
            combinedBoundary += item.item.value<VLayoutPiecePath>().Points();
        }

        // DumpVector(combinedBoundary, QStringLiteral("boundary.json.XXXXXX")); // Uncomment for dumping test data
        return combinedBoundary;
    }

    return detail.FullMainPathPoints(data);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenderSeamPath(const VPiece &detail, bool combineTogether, const VContainer *data) -> QPainterPath
{
    QVector<VLayoutPoint> const points = SeamPoints(detail, combineTogether, data);
    QVector<QPointF> boundary;
    CastTo(points, boundary);

    if (combineTogether)
    {
        QPainterPath combinedPath;
        combinedPath.addPolygon(QPolygonF(boundary));
        combinedPath.closeSubpath();
        combinedPath.setFillRule(Qt::OddEvenFill);

        return combinedPath;
    }

    return VPiece::MainPathPath(boundary);
}

//---------------------------------------------------------------------------------------------------------------------
auto SeamAllowancePoints(const VPiece &detail, bool combineTogether, const VContainer *data) -> QVector<VLayoutPoint>
{
    if (combineTogether)
    {
        const QVector<VLayoutPassmark> passmarks = VLayoutPiece::ConvertPassmarks(detail, data);
        const QVector<VLayoutPoint> points = detail.FullSeamAllowancePoints(data);

        // DumpVector(points, QStringLiteral("points.json.XXXXXX"));       // Uncomment for dumping test data
        // DumpVector(passmarks, QStringLiteral("passmarks.json.XXXXXX")); // Uncomment for dumping test data

        VBoundary boundary(points, true);
        boundary.SetPieceName(detail.GetName());

        if (QLineF const mirrorLine = detail.SeamAllowanceMirrorLine(data);
            !mirrorLine.isNull() && detail.IsShowFullPiece())
        {
            boundary.SetMirrorLine(mirrorLine);
        }

        const QList<VBoundarySequenceItemData> sequence = boundary.Combine(passmarks, false, false);

        QVector<VLayoutPoint> combinedBoundary;
        for (const auto &item : sequence)
        {
            combinedBoundary += item.item.value<VLayoutPiecePath>().Points();
        }

        // DumpVector(combinedBoundary, QStringLiteral("boundary.json.XXXXXX")); // Uncomment for dumping test data
        return combinedBoundary;
    }

    return detail.FullSeamAllowancePoints(data);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenderSeamAllowancePath(const VPiece &detail, bool combineTogether, const VContainer *data) -> QPainterPath
{
    QVector<VLayoutPoint> const points = SeamAllowancePoints(detail, combineTogether, data);
    if (combineTogether)
    {
        QVector<QPointF> combinedBoundary;
        CastTo(points, combinedBoundary);

        QPainterPath combinedPath;
        combinedPath.addPolygon(QPolygonF(combinedBoundary));
        combinedPath.closeSubpath();
        combinedPath.setFillRule(Qt::OddEvenFill);

        return combinedPath;
    }

    return detail.SeamAllowancePath(points);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenderPassmarks(const VPiece &detail, const VContainer *data) -> QPainterPath
{
    if (const QLineF mirrorLine = detail.SeamMirrorLine(data); !mirrorLine.isNull() && detail.IsShowFullPiece())
    {
        if (!detail.IsSeamAllowance())
        {
            return {};
        }

        QPainterPath path;
        const QTransform matrix = VGObject::FlippingMatrix(mirrorLine);
        const QVector<VLayoutPassmark> passmarks = VLayoutPiece::ConvertPassmarks(detail, data);
        for (const auto &passmark : passmarks)
        {
            QPainterPath passmaksPath;
            for (const auto &line : passmark.lines)
            {
                passmaksPath.moveTo(line.p1());
                passmaksPath.lineTo(line.p2());
            }
            path.addPath(passmaksPath);

            if (!VGObject::IsPointOnLineviaPDP(passmark.baseLine.p1(), mirrorLine.p1(), mirrorLine.p2(),
                                               accuracyPointOnLine * 2))
            {
                QPainterPath mirroredPassmaksPath;
                for (const auto &line : passmark.lines)
                {
                    mirroredPassmaksPath.moveTo(line.p1());
                    mirroredPassmaksPath.lineTo(line.p2());
                }
                path.addPath(matrix.map(mirroredPassmaksPath));
            }
        }

        return path;
    }
    return detail.PassmarksPath(data);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenderFoldLine(const VPiece &detail, const VContainer *data) -> VFoldLine
{
    QLineF foldLine = detail.IsHideMainPath() ? detail.SeamAllowanceMirrorLine(data) : detail.SeamMirrorLine(data);
    VFoldLine fLine(foldLine, detail.GetFoldLineType());
    fLine.SetLabelSvgFontSize(detail.GetFoldLineSvgFontSize());
    fLine.SetLabelFontItalic(detail.IsFoldLineLabelFontItalic());
    fLine.SetLabelFontBold(detail.IsFoldLineLabelFontBold());
    fLine.SetLabel(detail.GetFoldLineLabel());
    fLine.SetLabelAlignment(detail.GetFoldLineLabelAlignment());

    {
        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        QFont font = settings->GetLabelFont();
        font.setPointSize(static_cast<int>(detail.GetFoldLineSvgFontSize()));
        fLine.SetOutlineFont(font);
        fLine.SetSvgFont(settings->GetLabelSVGFont());
    }

    if (detail.IsManualFoldHeight())
    {
        VFormula formula(detail.GetFormulaFoldHeight(), data);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (formula.error())
        {
            const QString errorMsg = QObject::tr("Cannot calculate fold line height for piece '%1'. Reason: %2.")
                                         .arg(detail.GetName(), formula.Reason());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
        else
        {
            fLine.SetHeight(ToPixel(formula.getDoubleValue(), *data->GetPatternUnit()));
        }
    }

    if (detail.IsManualFoldWidth())
    {
        VFormula formula(detail.GetFormulaFoldWidth(), data);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (formula.error())
        {
            const QString errorMsg = QObject::tr("Cannot calculate fold line width for piece '%1'. Reason: %2.")
                                         .arg(detail.GetName(), formula.Reason());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
        else
        {
            fLine.SetWidth(ToPixel(formula.getDoubleValue(), *data->GetPatternUnit()));
        }
    }

    if (detail.IsManualFoldCenter())
    {
        VFormula formula(detail.GetFormulaFoldCenter(), data);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (formula.error())
        {
            const QString errorMsg =
                QObject::tr("Cannot calculate fold line center position for piece '%1'. Reason: %2.")
                    .arg(detail.GetName(), formula.Reason());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
        else
        {
            fLine.SetCenterPosition(formula.getDoubleValue());
        }
    }

    return fLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto RenderMirrorLine(const VPiece &detail, const VContainer *data, bool combineTogether) -> QPainterPath
{
    if (!detail.IsShowFullPiece() || !detail.IsShowMirrorLine())
    {
        return {};
    }

    // Determine the appropriate seam line and points based on the seam allowance state
    QLineF mirrorLine;
    QVector<VLayoutPoint> points;

    if (!detail.IsSeamAllowance() || detail.IsSeamAllowanceBuiltIn())
    {
        mirrorLine = detail.SeamMirrorLine(data);
        // Return empty path if no valid mirror line is found
        if (mirrorLine.isNull())
        {
            return {};
        }

        if (combineTogether)
        {
            points = SeamPoints(detail, combineTogether, data);
        }
    }
    else
    {
        mirrorLine = detail.SeamAllowanceMirrorLine(data);
        // Return empty path if no valid mirror line is found
        if (mirrorLine.isNull())
        {
            return {};
        }

        if (combineTogether)
        {
            points = SeamAllowancePoints(detail, combineTogether, data);
        }
    }

    QPointF p1 = mirrorLine.p1();
    QPointF p2 = mirrorLine.p2();

    // Adjust points based on intersections, if required
    if (combineTogether && !points.isEmpty())
    {
        QVector<QPointF> seam;
        CastTo(points, seam);

        if (QVector<QPointF> const intersectionPoints = VAbstractCurve::CurveIntersectLine(seam,
                                                                                           QLineF(mirrorLine.center(),
                                                                                                  p1));
            !intersectionPoints.isEmpty())
        {
            p1 = intersectionPoints.constFirst();
        }

        if (QVector<QPointF> const intersectionPoints = VAbstractCurve::CurveIntersectLine(seam,
                                                                                           QLineF(mirrorLine.center(),
                                                                                                  p2));
            !intersectionPoints.isEmpty())
        {
            p2 = intersectionPoints.constFirst();
        }
    }

    // Construct and return the path
    QPainterPath path;
    path.moveTo(p1);
    path.lineTo(p2);
    return path;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VToolSeamAllowance::~VToolSeamAllowance()
{
    CancelLabelRendering();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                VContainer *data) -> VToolSeamAllowance *
{
    SCASSERT(not dialog.isNull());
    const QPointer<DialogSeamAllowance> dialogTool = qobject_cast<DialogSeamAllowance *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolSeamAllowanceInitData initData;
    initData.detail = dialogTool->GetPiece();
    initData.width = initData.detail.GetFormulaSAWidth();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;

    auto LoadLabelTemplate = [&initData](const QString &path)
    {
        if (not path.isEmpty())
        {
            try
            {
                VLabelTemplate ltemplate;
                ltemplate.setXMLContent(VLabelTemplateConverter(path).Convert());
                return ltemplate.ReadLines();
            }
            catch (VException &e)
            {
                const QString errorMsg = QObject::tr("Piece '%1'. Unable to load default piece label template.\n%2\n%3")
                                             .arg(initData.detail.GetName(), e.ErrorMessage(), e.DetailedInformation());
                VAbstractApplication::VApp()->IsPedantic()
                    ? throw VException(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            }
        }

        return QVector<VLabelTemplateLine>();
    };

    initData.detail.GetPieceLabelData().SetLabelTemplate(LoadLabelTemplate(doc->GetDefaultPieceLabelPath()));
    initData.detail.GetPath().SetNodes(PrepareNodes(initData.detail.GetPath(), scene, doc, data));

    VToolSeamAllowance *piece = Create(initData);

    if (piece != nullptr)
    {
        piece->m_dialog = dialog;
        piece->RefreshGeometry(true); // Refresh internal paths
    }
    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::Create(VToolSeamAllowanceInitData &initData) -> VToolSeamAllowance *
{
    if (initData.typeCreation == Source::FromGui || initData.typeCreation == Source::FromTool)
    {
        auto *currentSA = new VIncrement(initData.data, currentSeamAllowance);
        currentSA->SetFormula(initData.detail.GetSAWidth(), initData.width, true);
        currentSA->SetDescription(tr("Current seam allowance"));

        initData.data->AddVariable(currentSA);
        initData.id = initData.data->AddPiece(initData.detail);
    }
    else
    {
        const qreal calcWidth = CheckFormula(initData.id, initData.width, initData.data);
        initData.detail.SetFormulaSAWidth(initData.width, calcWidth);

        auto *currentSA = new VIncrement(initData.data, currentSeamAllowance);
        currentSA->SetFormula(calcWidth, initData.width, true);
        currentSA->SetDescription(tr("Current seam allowance"));

        initData.data->AddVariable(currentSA);

        initData.data->UpdatePiece(initData.id, initData.detail);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    VToolSeamAllowance *piece = nullptr;
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Piece, initData.doc);
        piece = new VToolSeamAllowance(initData);
        initData.scene->addItem(piece);
        VMainGraphicsView::NewSceneRect(initData.scene, VAbstractValApplication::VApp()->getSceneView(), piece);
        VAbstractPattern::AddTool(initData.id, piece);
    }
    // Very important to delete it. Only this tool need this special variable.
    initData.data->RemoveVariable(currentSeamAllowance);
    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::Duplicate(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc)
    -> VToolSeamAllowance *
{
    SCASSERT(not dialog.isNull());
    const QPointer<DialogDuplicateDetail> dialogTool = qobject_cast<DialogDuplicateDetail *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolSeamAllowanceInitData initData;
    initData.scene = scene;
    initData.doc = doc;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.drawName = doc->PieceDrawName(dialogTool->Duplicate());

    VContainer toolData = VAbstractPattern::getTool(dialogTool->Duplicate())->getData();
    initData.data = &toolData;

    VPiece detail = initData.data->GetPiece(dialogTool->Duplicate());
    detail.SetMx(dialogTool->MoveDuplicateX());
    detail.SetMy(dialogTool->MoveDuplicateY());
    initData.detail = detail;
    initData.width = initData.detail.GetFormulaSAWidth();

    VToolSeamAllowance *piece = Duplicate(initData);
    if (piece != nullptr)
    {
        piece->RefreshGeometry(true); // Refresh internal paths
    }
    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::Duplicate(VToolSeamAllowanceInitData &initData) -> VToolSeamAllowance *
{
    VPiece dupDetail = initData.detail;

    QMap<quint32, quint32> replacements;
    dupDetail.GetPath().SetNodes(DuplicateNodes(initData.detail.GetPath(), initData, replacements));
    dupDetail.SetCustomSARecords(
        DuplicateCustomSARecords(initData.detail.GetCustomSARecords(), initData, replacements));
    dupDetail.SetInternalPaths(DuplicateInternalPaths(initData.detail.GetInternalPaths(), initData));
    dupDetail.SetPlaceLabels(DuplicatePlaceLabels(initData.detail.GetPlaceLabels(), initData));
    dupDetail.SetUUID(QUuid::createUuid());

    const QMap<quint32, quint32> mappedPins = DuplicatePins(initData.detail.GetPins(), initData);
    dupDetail.SetPins(ConvertToVector(mappedPins.values()));
    dupDetail.SetPieceLabelData(FixLabelPins(initData.detail.GetPieceLabelData(), mappedPins));
    dupDetail.SetPatternLabelData(FixLabelPins(initData.detail.GetPatternLabelData(), mappedPins));
    dupDetail.SetGrainlineGeometry(FixGrainlinePins(initData.detail.GetGrainlineGeometry(), mappedPins));

    if (replacements.contains(dupDetail.GetMirrorLineStartPoint()))
    {
        dupDetail.SetMirrorLineStartPoint(replacements.value(dupDetail.GetMirrorLineStartPoint()));
    }

    if (replacements.contains(dupDetail.GetMirrorLineEndPoint()))
    {
        dupDetail.SetMirrorLineEndPoint(replacements.value(dupDetail.GetMirrorLineEndPoint()));
    }

    initData.detail = dupDetail;
    return VToolSeamAllowance::Create(initData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::RemoveWithConfirm(bool ask)
{
    try
    {
        DeleteToolWithConfirm(ask);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e);
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InsertNodes(const QVector<VPieceNode> &nodes, quint32 pieceId, VMainGraphicsScene *scene,
                                     VContainer *data, VAbstractPattern *doc)
{
    SCASSERT(scene != nullptr)
    SCASSERT(data != nullptr)
    SCASSERT(doc != nullptr)

    if (pieceId <= NULL_ID || nodes.isEmpty())
    {
        return;
    }

    VPiece oldDet;
    try
    {
        oldDet = data->GetPiece(pieceId);
    }
    catch (const VExceptionBadId &)
    {
        return;
    }

    VPiece newDet = oldDet;

    for (auto node : nodes)
    {
        const quint32 id = PrepareNode(node, scene, doc, data);
        if (id == NULL_ID)
        {
            return;
        }

        node.SetId(id);
        newDet.GetPath().Append(node);

        // Seam allowance tool already initializated and can't init the node
        auto *saTool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(pieceId));
        SCASSERT(saTool != nullptr);

        InitNode(node, scene, saTool);
    }

    VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, pieceId));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece)
{
    SCASSERT(doc != nullptr);

    doc->SetAttribute(domElement, VDomDocument::AttrId, id);
    doc->SetAttribute(domElement, AttrName, piece.GetName());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrShortName, piece.GetShortName(),
                                         [](const QString &name) noexcept { return name.isEmpty(); });
    doc->SetAttribute(domElement, AttrUUID, piece.GetUUID().toString());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrGradationLabel, piece.GetGradationLabel(),
                                         [](const QString &label) noexcept { return label.isEmpty(); });
    doc->SetAttribute(domElement, AttrVersion, QString().setNum(pieceVersion));
    doc->SetAttribute(domElement, AttrMx, VAbstractValApplication::VApp()->fromPixel(piece.GetMx()));
    doc->SetAttribute(domElement, AttrMy, VAbstractValApplication::VApp()->fromPixel(piece.GetMy()));
    doc->SetAttributeOrRemoveIf<bool>(domElement, AttrInLayout, piece.IsInLayout(),
                                      [](bool inLayout) noexcept { return inLayout; });
    doc->SetAttribute(domElement, AttrForbidFlipping, piece.IsForbidFlipping());
    doc->SetAttribute(domElement, AttrForceFlipping, piece.IsForceFlipping());
    doc->SetAttributeOrRemoveIf<bool>(domElement,
                                      AttrSymmetricalCopy,
                                      piece.IsSymmetricalCopy(),
                                      [](bool sCopy) noexcept { return not sCopy; });
    doc->SetAttribute(domElement, AttrFollowGrainline, piece.IsFollowGrainline());
    doc->SetAttribute(domElement, AttrSewLineOnDrawing, piece.IsSewLineOnDrawing());
    doc->SetAttributeOrRemoveIf<bool>(domElement, AttrSeamAllowance, piece.IsSeamAllowance(),
                                      [](bool seamAllowance) noexcept { return not seamAllowance; });
    doc->SetAttribute(domElement, AttrHideMainPath, piece.IsHideMainPath());
    doc->SetAttributeOrRemoveIf<bool>(domElement, AttrSeamAllowanceBuiltIn, piece.IsSeamAllowanceBuiltIn(),
                                      [](bool builtin) noexcept { return not builtin; });
    doc->SetAttributeOrRemoveIf<bool>(domElement, AttrShowFullPiece, piece.IsShowFullPiece(),
                                      [](bool show) noexcept { return show; });
    doc->SetAttribute(domElement, AttrWidth, piece.GetFormulaSAWidth());
    doc->SetAttributeOrRemoveIf<bool>(domElement, AttrUnited, piece.IsUnited(),
                                      [](bool united) noexcept { return not united; });
    doc->SetAttributeOrRemoveIf<uint>(domElement, AttrPiecePriority, piece.GetPriority(),
                                      [](uint priority) noexcept { return priority == 0; });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddCSARecord(VAbstractPattern *doc, QDomElement &domElement, CustomSARecord record)
{
    QDomElement recordNode = doc->createElement(VToolSeamAllowance::TagRecord);

    doc->SetAttribute(recordNode, VAbstractPattern::AttrStart, record.startPoint);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrPath, record.path);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrEnd, record.endPoint);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrNodeReverse, record.reverse);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrIncludeAs, static_cast<unsigned int>(record.includeType));

    domElement.appendChild(recordNode);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddCSARecords(VAbstractPattern *doc, QDomElement &domElement,
                                       const QVector<CustomSARecord> &records)
{
    if (not records.empty())
    {
        QDomElement csaRecordsElement = doc->createElement(VToolSeamAllowance::TagCSA);
        for (auto record : records)
        {
            AddCSARecord(doc, csaRecordsElement, record);
        }
        domElement.appendChild(csaRecordsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths)
{
    if (not paths.empty())
    {
        QDomElement iPathsElement = doc->createElement(VToolSeamAllowance::TagIPaths);
        for (auto path : paths)
        {
            QDomElement recordNode = doc->createElement(VToolSeamAllowance::TagRecord);
            doc->SetAttribute(recordNode, VAbstractPattern::AttrPath, path);
            iPathsElement.appendChild(recordNode);
        }
        domElement.appendChild(iPathsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPins(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &pins)
{
    AddPointRecords(doc, domElement, pins, VToolSeamAllowance::TagPins);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPlaceLabels(VAbstractPattern *doc, QDomElement &domElement,
                                        const QVector<quint32> &placeLabels)
{
    AddPointRecords(doc, domElement, placeLabels, VToolSeamAllowance::TagPlaceLabels);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPatternPieceData(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagData);
    const VPieceLabelData &data = piece.GetPieceLabelData();
    doc->SetAttribute(domData, VAbstractPattern::AttrLetter, data.GetLetter());
    doc->SetAttribute(domData, VAbstractPattern::AttrAnnotation, data.GetAnnotation());
    doc->SetAttribute(domData, VAbstractPattern::AttrOrientation, data.GetOrientation());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotationWay, data.GetRotationWay());
    doc->SetAttribute(domData, VAbstractPattern::AttrTilt, data.GetTilt());
    doc->SetAttribute(domData, VAbstractPattern::AttrFoldPosition, data.GetFoldPosition());
    doc->SetAttribute(domData, VAbstractPattern::AttrQuantity, data.GetQuantity());
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible, data.IsEnabled());
    doc->SetAttribute(domData, VAbstractPattern::AttrOnFold, data.IsOnFold());
    doc->SetAttribute(domData, AttrMx, data.GetPos().x());
    doc->SetAttribute(domData, AttrMy, data.GetPos().y());
    doc->SetAttribute(domData, AttrWidth, data.GetLabelWidth());
    doc->SetAttribute(domData, AttrHeight, data.GetLabelHeight());
    doc->SetAttributeOrRemoveIf<int>(domData, AttrFont, data.GetFontSize(),
                                     [](int size) noexcept { return size == 0; });
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, data.GetRotation());
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrCenterPin, data.CenterPin(),
                                         [](quint32 pin) noexcept { return pin == NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrTopLeftPin, data.TopLeftPin(),
                                         [](quint32 leftPin) noexcept { return leftPin == NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrBottomRightPin, data.BottomRightPin(),
                                         [](quint32 rightPin) noexcept { return rightPin == NULL_ID; });
    doc->SetLabelTemplate(domData, data.GetLabelTemplate());

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPatternInfo(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagPatternInfo);
    const VPatternLabelData &geom = piece.GetPatternLabelData();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible, geom.IsEnabled());
    doc->SetAttribute(domData, AttrMx, geom.GetPos().x());
    doc->SetAttribute(domData, AttrMy, geom.GetPos().y());
    doc->SetAttribute(domData, AttrWidth, geom.GetLabelWidth());
    doc->SetAttribute(domData, AttrHeight, geom.GetLabelHeight());
    doc->SetAttributeOrRemoveIf<int>(domData, AttrFont, geom.GetFontSize(),
                                     [](int size) noexcept { return size == 0; });
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, geom.GetRotation());
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrCenterPin, geom.CenterPin(),
                                         [](quint32 pin) noexcept { return pin <= NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrTopLeftPin, geom.TopLeftPin(),
                                         [](quint32 pin) noexcept { return pin <= NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrBottomRightPin, geom.BottomRightPin(),
                                         [](quint32 pin) noexcept { return pin <= NULL_ID; });

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    // grainline
    QDomElement domData = doc->createElement(VAbstractPattern::TagGrainline);
    const VGrainlineData &glGeom = piece.GetGrainlineGeometry();
    doc->SetAttribute(domData, VAbstractPattern::AttrEnabled, glGeom.IsEnabled());
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible, glGeom.IsVisible());
    doc->SetAttribute(domData, AttrMx, glGeom.GetPos().x());
    doc->SetAttribute(domData, AttrMy, glGeom.GetPos().y());
    doc->SetAttribute(domData, AttrLength, glGeom.GetLength());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, glGeom.GetRotation());
    doc->SetAttribute(domData, VAbstractPattern::AttrArrows, static_cast<int>(glGeom.GetArrowType()));
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrCenterPin, glGeom.CenterPin(),
                                         [](quint32 pin) noexcept { return pin <= NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrTopPin, glGeom.TopPin(),
                                         [](quint32 pin) noexcept { return pin <= NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, AttrBottomPin, glGeom.BottomPin(),
                                         [](quint32 pin) noexcept { return pin <= NULL_ID; });

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddMirrorLine(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VToolSeamAllowance::TagMirrorLine);
    doc->SetAttributeOrRemoveIf<quint32>(domData, VAbstractPattern::AttrMirrorLineP1, piece.GetMirrorLineStartPoint(),
                                         [](quint32 id) noexcept { return id <= NULL_ID; });
    doc->SetAttributeOrRemoveIf<quint32>(domData, VAbstractPattern::AttrMirrorLineP2, piece.GetMirrorLineEndPoint(),
                                         [](quint32 id) noexcept { return id <= NULL_ID; });
    doc->SetAttributeOrRemoveIf<bool>(domData, VAbstractPattern::AttrFoldLineManualHeight, piece.IsManualFoldHeight(),
                                      [](bool manual) noexcept { return not manual; });
    doc->SetAttributeOrRemoveIf<bool>(domData, VAbstractPattern::AttrFoldLineManualWidth, piece.IsManualFoldWidth(),
                                      [](bool manual) noexcept { return not manual; });
    doc->SetAttributeOrRemoveIf<bool>(domData, VAbstractPattern::AttrFoldLineManualCenter, piece.IsManualFoldCenter(),
                                      [](bool manual) noexcept { return not manual; });
    doc->SetAttributeOrRemoveIf<QString>(
        domData, VAbstractPattern::AttrFoldLineHeightFormula, piece.GetFormulaFoldHeight(),
        [piece](const QString &height) noexcept { return not piece.IsManualFoldHeight() || height.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(
        domData, VAbstractPattern::AttrFoldLineWidthFormula, piece.GetFormulaFoldWidth(),
        [piece](const QString &width) noexcept { return not piece.IsManualFoldWidth() || width.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(
        domData, VAbstractPattern::AttrFoldLineCenterFormula, piece.GetFormulaFoldCenter(),
        [piece](const QString &center) noexcept { return not piece.IsManualFoldCenter() || center.isEmpty(); });
    doc->SetAttribute(domData, VAbstractPattern::AttrFoldLineType, FoldLineTypeToString(piece.GetFoldLineType()));
    doc->SetAttributeOrRemoveIf<unsigned int>(domData, VAbstractPattern::AttrFoldLineFontSize,
                                              piece.GetFoldLineSvgFontSize(),
                                              [](unsigned int size) noexcept { return size == defFoldLineFontSize; });
    doc->SetAttributeOrRemoveIf<bool>(domData, VDomDocument::AttrItalic, piece.IsFoldLineLabelFontItalic(),
                                      [](bool italic) noexcept { return not italic; });
    doc->SetAttributeOrRemoveIf<bool>(domData, VDomDocument::AttrBold, piece.IsFoldLineLabelFontBold(),
                                      [](bool bold) noexcept { return not bold; });
    doc->SetAttributeOrRemoveIf<QString>(domData, VAbstractPattern::AttrFoldLineLabel, piece.GetFoldLineLabel(),
                                         [](const QString &label) noexcept { return label.isEmpty(); });
    doc->SetAttributeOrRemoveIf<int>(domData, VDomDocument::AttrAlignment, piece.GetFoldLineLabelAlignment(),
                                     [](int alignment) noexcept { return alignment == Qt::AlignHCenter; });
    doc->SetAttributeOrRemoveIf<bool>(domData, VAbstractPattern::AttrMirrorLineVisible, piece.IsShowMirrorLine(),
                                      [](bool visible) noexcept { return visible; });

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SetName(const QString &name)
{
    VPiece detail = VAbstractTool::data.GetPiece(m_id);
    detail.SetName(name);
    VAbstractTool::data.UpdatePiece(m_id, detail);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::Move(qreal x, qreal y)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    VPiece detail = VAbstractTool::data.GetPiece(m_id);
    detail.SetMx(x);
    detail.SetMy(y);
    VAbstractTool::data.UpdatePiece(m_id, detail);

    setPos(x, y);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::Update(const VPiece &piece)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    VAbstractTool::data.UpdatePiece(m_id, piece);
    RefreshGeometry();
    VMainGraphicsView::NewSceneRect(m_sceneDetails, VAbstractValApplication::VApp()->getSceneView(), this);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::DisconnectOutsideSignals()
{
    // If UnionDetails tool delete the detail this object will be deleted only after full parse.
    // Deleting inside UnionDetails cause crash.
    // Because this object should be inactive from no one we disconnect all signals that may cause a crash
    // KEEP THIS LIST ACTUALL!!!
    disconnect(doc, nullptr, this, nullptr);
    if (QGraphicsScene *toolScene = scene())
    {
        disconnect(toolScene, nullptr, this, nullptr);
    }
    disconnect(m_dataLabel, nullptr, this, nullptr);
    disconnect(m_patternInfo, nullptr, this, nullptr);
    disconnect(m_grainLine, nullptr, this, nullptr);
    disconnect(m_sceneDetails, nullptr, this, nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ConnectOutsideSignals()
{
    connect(m_dataLabel, &VTextGraphicsItem::SignalMoved, this, &VToolSeamAllowance::SaveMoveDetail);
    connect(m_dataLabel, &VTextGraphicsItem::SignalResized, this, &VToolSeamAllowance::SaveResizeDetail);
    connect(m_dataLabel, &VTextGraphicsItem::SignalRotated, this, &VToolSeamAllowance::SaveRotationDetail);

    connect(m_patternInfo, &VTextGraphicsItem::SignalMoved, this, &VToolSeamAllowance::SaveMovePattern);
    connect(m_patternInfo, &VTextGraphicsItem::SignalResized, this, &VToolSeamAllowance::SaveResizePattern);
    connect(m_patternInfo, &VTextGraphicsItem::SignalRotated, this, &VToolSeamAllowance::SaveRotationPattern);

    connect(m_grainLine, &VGrainlineItem::SignalMoved, this, &VToolSeamAllowance::SaveMoveGrainline);
    connect(m_grainLine, &VGrainlineItem::SignalResized, this, &VToolSeamAllowance::SaveResizeGrainline);
    connect(m_grainLine, &VGrainlineItem::SignalRotated, this, &VToolSeamAllowance::SaveRotateGrainline);

    connect(doc, &VAbstractPattern::UpdatePatternLabel, this, &VToolSeamAllowance::UpdatePatternInfo);
    connect(doc, &VAbstractPattern::UpdatePatternLabel, this, &VToolSeamAllowance::UpdateDetailLabel);
    connect(doc, &VAbstractPattern::CheckLayout, this, &VToolSeamAllowance::UpdateDetailLabel);
    connect(doc, &VAbstractPattern::CheckLayout, this, &VToolSeamAllowance::UpdatePatternInfo);
    connect(doc, &VAbstractPattern::CheckLayout, this, &VToolSeamAllowance::UpdateGrainline);

    connect(m_sceneDetails, &VMainGraphicsScene::EnableToolMove, this, &VToolSeamAllowance::EnableToolMove);
    connect(m_sceneDetails, &VMainGraphicsScene::ItemByMousePress, this, &VToolSeamAllowance::ResetChildren);
    connect(m_sceneDetails, &VMainGraphicsScene::DimensionsChanged, this, &VToolSeamAllowance::UpdateDetailLabel);
    connect(m_sceneDetails, &VMainGraphicsScene::DimensionsChanged, this, &VToolSeamAllowance::UpdatePatternInfo);
    connect(m_sceneDetails, &VMainGraphicsScene::LanguageChanged, this, &VToolSeamAllowance::retranslateUi);
    connect(m_sceneDetails, &VMainGraphicsScene::EnableDetailItemHover, this, &VToolSeamAllowance::AllowHover);
    connect(m_sceneDetails, &VMainGraphicsScene::EnableDetailItemSelection, this, &VToolSeamAllowance::AllowSelecting);
    connect(m_sceneDetails, &VMainGraphicsScene::HighlightDetail, this, &VToolSeamAllowance::Highlight);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ReinitInternals(const VPiece &detail, VMainGraphicsScene *scene)
{
    InitNodes(detail, scene);
    InitCSAPaths(detail);
    InitInternalPaths(detail);
    InitSpecialPoints(detail.GetPins());
    InitSpecialPoints(detail.GetPlaceLabels());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::getTagName() const -> QString
{
    return VAbstractPattern::TagDetail;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ShowVisualization(bool show)
{
    Q_UNUSED(show)
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object);
    Q_UNUSED(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::FullUpdateFromFile()
{
    // Do nothing. Refresh triggered outside
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::EnableToolMove(bool move)
{
    setFlag(QGraphicsItem::ItemIsMovable, move);

    m_dataLabel->setFlag(QGraphicsItem::ItemIsMovable, move);
    m_patternInfo->setFlag(QGraphicsItem::ItemIsMovable, move);
    m_grainLine->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AllowHover(bool enabled)
{
    // Manually handle hover events. Need for setting cursor for not selectable paths.
    m_acceptHoverEvents = enabled;
    m_dataLabel->setAcceptHoverEvents(enabled);
    m_patternInfo->setAcceptHoverEvents(enabled);
    m_grainLine->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ResetChildren(QGraphicsItem *pItem)
{
    const bool selected = isSelected();
    const VPiece detail = VAbstractTool::data.GetPiece(m_id);
    auto *pVGI = qgraphicsitem_cast<VTextGraphicsItem *>(pItem);
    if (pVGI != m_dataLabel && detail.GetPieceLabelData().IsEnabled())
    {
        m_dataLabel->Reset();
    }
    if (pVGI != m_patternInfo && detail.GetPatternLabelData().IsEnabled())
    {
        m_patternInfo->Reset();
    }

    if (const auto *pGLI = qgraphicsitem_cast<VGrainlineItem *>(pItem);
        pGLI != m_grainLine && detail.GetGrainlineGeometry().IsEnabled())
    {
        m_grainLine->Reset();
    }

    setSelected(selected);
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdateAll()
{
    m_sceneDetails->update();
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::retranslateUi()
{
    UpdateDetailLabel();
    UpdatePatternInfo();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::Highlight(quint32 id)
{
    setSelected(m_id == id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateLabel updates the text label, making it just big enough for the text to fit it
 */
void VToolSeamAllowance::UpdateDetailLabel()
{
    if (m_pieceUpdateInfoWatcher->isFinished())
    {
        VPiece detail = VAbstractTool::data.GetPiece(m_id);
        detail.SetPieceLabelData(detail.GetPieceLabelData()); // Refresh translation
        const VPieceLabelData &labelData = detail.GetPieceLabelData();
        const QVector<quint32> &pins = detail.GetPins();

        if (labelData.IsEnabled())
        {
            m_pieceLabelPos = QPointF();
            m_pieceLabelAngle = 0;
            if (PrepareLabelData(labelData, pins, m_dataLabel, m_pieceLabelPos, m_pieceLabelAngle))
            {

                VPieceLabelInfo info = VTextManager::PrepareLabelInfo(doc, getData(), true);
                info.pieceName = detail.GetName();
                info.labelData = detail.GetPieceLabelData();

                m_pieceLabelInfoStale = false;
                m_pieceUpdateInfoWatcher->setFuture(QtConcurrent::run(
                    [this, info, detail]()
                    {
                        m_dataLabel->SetPieceName(detail.GetName());
                        m_dataLabel->UpdatePieceLabelData(info);
                    }));
            }
        }
        else
        {
            m_dataLabel->hide();
        }
    }
    else
    {
        m_pieceLabelInfoStale = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdatePatternInfo updates the pattern info label
 */
void VToolSeamAllowance::UpdatePatternInfo()
{
    if (m_patternUpdateInfoWatcher->isFinished())
    {
        const VPiece detail = VAbstractTool::data.GetPiece(m_id);
        const VPatternLabelData &geom = detail.GetPatternLabelData();
        const QVector<quint32> &pins = detail.GetPins();

        if (geom.IsEnabled())
        {
            m_patternLabelPos = QPointF();
            m_patternLabelAngle = 0;
            if (PrepareLabelData(geom, pins, m_patternInfo, m_patternLabelPos, m_patternLabelAngle))
            {
                VPieceLabelInfo const info = VTextManager::PrepareLabelInfo(doc, getData(), false);

                m_patternLabelInfoStale = false;
                m_patternUpdateInfoWatcher->setFuture(QtConcurrent::run(
                    [this, info, detail]()
                    {
                        m_patternInfo->SetPieceName(detail.GetName());
                        m_patternInfo->UpdatePatternLabelData(info);
                    }));
            }
        }
        else
        {
            m_patternInfo->hide();
        }
    }
    else
    {
        m_patternLabelInfoStale = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdatePassmarks()
{
    const VPiece detail = VAbstractTool::data.GetPiece(m_id);
    m_passmarks->setPath(RenderPassmarks(detail, getData()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolDetail::UpdateGrainline updates the grain line item
 */
void VToolSeamAllowance::UpdateGrainline()
{
    const VPiece detail = VAbstractTool::data.GetPiece(m_id);
    const VGrainlineData &geom = detail.GetGrainlineGeometry();
    const QVector<quint32> &pins = detail.GetPins();

    if (geom.IsEnabled() && geom.IsVisible())
    {
        QPointF pos;
        qreal dRotation = 0;
        qreal dLength = 0;

        const VGrainlineItem::MoveTypes type = FindGrainlineGeometry(geom, pins, dLength, dRotation, pos);
        if ((type & VGrainlineItem::Error) != 0U)
        {
            m_grainLine->hide();
            return;
        }

        m_grainLine->SetMoveType(type);
        m_grainLine->UpdateGeometry(pos, dRotation, ToPixel(dLength, *VDataTool::data.GetPatternUnit()),
                                    geom.GetArrowType());
        m_grainLine->show();

        if (m_geometryIsReady && not IsGrainlinePositionValid())
        {
            const QString errorMsg = QObject::tr("Piece '%1'. Grainline is not valid.").arg(detail.GetName());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    }
    else
    {
        m_grainLine->hide();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveMoveDetail saves the move detail operation to the undo stack
 */
void VToolSeamAllowance::SaveMoveDetail(const QPointF &ptPos)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;
    newDet.GetPieceLabelData().SetPos(ptPos);

    auto *moveCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    moveCommand->setText(tr("move pattern piece label"));
    VAbstractApplication::VApp()->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveResizeDetail saves the resize detail label operation to the undo stack
 */
void VToolSeamAllowance::SaveResizeDetail(qreal dLabelW)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newDet.GetPieceLabelData().SetLabelWidth(QString().setNum(dLabelW));
    const qreal height = FromPixel(m_dataLabel->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newDet.GetPieceLabelData().SetLabelHeight(QString().setNum(height));

    auto *resizeCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    resizeCommand->setText(tr("resize pattern piece label"));
    VAbstractApplication::VApp()->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveRotationDetail saves the rotation detail label operation to the undo stack
 */
void VToolSeamAllowance::SaveRotationDetail(qreal dRot)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;
    newDet.GetPieceLabelData().SetPos(m_dataLabel->pos());

    // Tranform angle to anticlockwise
    QLineF line(0, 0, 100, 0);
    line.setAngle(-dRot);
    newDet.GetPieceLabelData().SetRotation(QString().setNum(line.angle()));

    auto *rotateCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    rotateCommand->setText(tr("rotate pattern piece label"));
    VAbstractApplication::VApp()->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveMovePattern saves the pattern label position
 */
void VToolSeamAllowance::SaveMovePattern(const QPointF &ptPos)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;
    newDet.GetPatternLabelData().SetPos(ptPos);

    auto *moveCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    moveCommand->setText(tr("move pattern info label"));
    VAbstractApplication::VApp()->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief: SaveResizePattern saves the pattern label width and font size
 */
void VToolSeamAllowance::SaveResizePattern(qreal dLabelW)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newDet.GetPatternLabelData().SetLabelWidth(QString().setNum(dLabelW));
    qreal const height = FromPixel(m_patternInfo->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newDet.GetPatternLabelData().SetLabelHeight(QString().setNum(height));

    auto *resizeCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    resizeCommand->setText(tr("resize pattern info label"));
    VAbstractApplication::VApp()->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveRotationPattern(qreal dRot)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    newDet.GetPatternLabelData().SetPos(m_patternInfo->pos());

    // Tranform angle to anticlockwise
    QLineF line(0, 0, 100, 0);
    line.setAngle(-dRot);
    newDet.GetPatternLabelData().SetRotation(QString().setNum(line.angle()));

    auto *rotateCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    rotateCommand->setText(tr("rotate pattern info label"));
    VAbstractApplication::VApp()->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveMoveGrainline(const QPointF &ptPos)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;
    newDet.GetGrainlineGeometry().SetPos(ptPos);
    qDebug() << "******* new grainline pos" << ptPos;

    auto *moveCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    moveCommand->setText(tr("move grainline"));
    VAbstractApplication::VApp()->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveResizeGrainline(qreal dLength)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    dLength = FromPixel(dLength, *VDataTool::data.GetPatternUnit());
    newDet.GetGrainlineGeometry().SetPos(m_grainLine->pos());
    newDet.GetGrainlineGeometry().SetLength(QString().setNum(dLength));

    auto *resizeCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    resizeCommand->setText(tr("resize grainline"));
    VAbstractApplication::VApp()->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveRotateGrainline(qreal dRot, const QPointF &ptPos)
{
    VPiece const oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    newDet.GetGrainlineGeometry().SetRotation(QString().setNum(qRadiansToDegrees(dRot)));
    newDet.GetGrainlineGeometry().SetPos(ptPos);

    auto *rotateCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    rotateCommand->setText(tr("rotate grainline"));
    VAbstractApplication::VApp()->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolDetail::paint draws a bounding box around detail, if one of its text or grainline items is not idle.
 */
void VToolSeamAllowance::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen toolPen = pen();
    toolPen.setWidthF(ScaleWidth(VAbstractApplication::VApp()->Settings()->WidthHairLine(), SceneScale(scene())));
    toolPen.setColor(VSceneStylesheet::PatternPieceStyle().PieceColor());

    setPen(toolPen);
    m_seamAllowance->setPen(toolPen);
    m_passmarks->setPen(toolPen);
    m_placeLabels->setPen(toolPen);
    m_foldLineMark->setPen(toolPen);
    m_foldLineLabel->setPen(toolPen);

    QPen mirrorLinePen = toolPen;
    mirrorLinePen.setStyle(Qt::DashDotLine);
    m_mirrorLine->setPen(mirrorLinePen);

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts())
    {
        m_foldLineLabel->setBrush(Qt::NoBrush);
    }
    else
    {
        m_foldLineLabel->setBrush(Qt::SolidPattern);
    }

    if ((not m_dataLabel->IsIdle() || not m_patternInfo->IsIdle() || not m_grainLine->IsIdle()) && not isSelected())
    {
        setSelected(true);
    }
    PaintWithFixItemHighlightSelected<QGraphicsPathItem>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::boundingRect() const -> QRectF
{
    if (m_pieceBoundingRect.isNull())
    {
        return QGraphicsPathItem::boundingRect();
    }

    return m_pieceBoundingRect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::shape() const -> QPainterPath
{
    if (m_mainPath == QPainterPath())
    {
        return QGraphicsPathItem::shape();
    }

    return ItemShapeFromPath(m_mainPath, pen());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::CancelLabelRendering()
{
    m_patternUpdateInfoWatcher->cancel();
    m_pieceUpdateInfoWatcher->cancel();

    m_patternUpdateInfoWatcher->waitForFinished();
    m_pieceUpdateInfoWatcher->waitForFinished();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::FullUpdateFromGuiApply()
{
    SaveDialogChange(tr("apply save detail options"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddToFile()
{
    const QDomElement duplicate = doc->FindElementById(m_id);
    if (not duplicate.isNull())
    {
        throw VExceptionWrongId(tr("This id (%1) is not unique.").arg(m_id), duplicate);
    }

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);

    QDomElement domElement = doc->createElement(VToolSeamAllowance::getTagName());

    AddAttributes(doc, domElement, m_id, piece);
    AddPatternPieceData(doc, domElement, piece);
    AddPatternInfo(doc, domElement, piece);
    AddGrainline(doc, domElement, piece);

    // nodes
    AddNodes(doc, domElement, piece);
    // custom seam allowance
    AddCSARecords(doc, domElement, piece.GetCustomSARecords());
    AddInternalPaths(doc, domElement, piece.GetInternalPaths());
    AddPins(doc, domElement, piece.GetPins());
    AddPlaceLabels(doc, domElement, piece.GetPlaceLabels());
    AddMirrorLine(doc, domElement, piece);

    VAbstractApplication::VApp()->getUndoStack()->push(
        new AddPiece(domElement, doc, VAbstractTool::data, m_sceneDetails, m_drawName));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::RefreshDataInFile()
{
    QDomElement domElement = doc->FindElementById(m_id, VToolSeamAllowance::getTagName());
    if (!domElement.isElement())
    {
        return;
    }

    // Refresh only parts that we possibly need to update

    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    const uint version = VAbstractPattern::GetParametrUInt(domElement, AttrVersion, QChar('1'));
    if (version == 1)
    {
        const VPiece piece = VAbstractTool::data.GetPiece(m_id);

        doc->SetAttribute(domElement, AttrVersion, QString().setNum(pieceVersion));

        VAbstractPattern::RemoveAllChildren(domElement); // Very important to clear before rewrite
        AddPatternPieceData(doc, domElement, piece);
        AddPatternInfo(doc, domElement, piece);
        AddGrainline(doc, domElement, piece);
        AddNodes(doc, domElement, piece);
        AddCSARecords(doc, domElement, piece.GetCustomSARecords());
        AddInternalPaths(doc, domElement, piece.GetInternalPaths());
        AddPins(doc, domElement, piece.GetPins());
        AddPlaceLabels(doc, domElement, piece.GetPlaceLabels());
        AddMirrorLine(doc, domElement, piece);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == ItemPositionChange && scene())
    {
        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool changeFinished = true;
        if (changeFinished)
        {
            changeFinished = false;

            // value - this is new position.
            const QPointF newPos = value.toPointF();

            VAbstractApplication::VApp()->getUndoStack()->push(
                new MovePiece(doc, newPos.x(), newPos.y(), m_id, scene()));

            const QList<QGraphicsView *> viewList = scene()->views();
            if (not viewList.isEmpty())
            {
                if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
                {
                    view->EnsureItemVisibleWithDelay(this, VMainGraphicsView::scrollDelay);
                }
            }

            changeFinished = true;
        }
    }

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (value == true)
        {
            // do stuff if selected
            this->setFocus();
        }
        else
        {
            // do stuff if not selected
        }
    }

    return QGraphicsPathItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    QGraphicsPathItem::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }

    if (selectionType == SelectionType::ByMouseRelease)
    {
        event
            ->accept(); // Special for not selectable item first need to call standard mousePressEvent then accept event
    }
    else
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            doc->SelectedDetail(m_id);
            emit ChoosedTool(m_id, SceneObject::Detail);
            event->accept();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && (flags() & QGraphicsItem::ItemIsMovable))
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }

    if (selectionType == SelectionType::ByMouseRelease && IsSelectedByReleaseEvent(this, event))
    {
        doc->SelectedDetail(m_id);
        emit ChoosedTool(m_id, SceneObject::Detail);
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_acceptHoverEvents)
    {
        if (flags() & QGraphicsItem::ItemIsMovable)
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
        else
        {
            setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
        }
        QGraphicsPathItem::hoverEnterEvent(event);
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_acceptHoverEvents)
    {
        QGraphicsPathItem::hoverLeaveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_suppressContextMenu)
    {
        return;
    }

    QMenu menu;
    QAction *actionOption = menu.addAction(FromTheme(VThemeIcon::PreferencesOther), tr("Options"));

    const VPiece detail = VAbstractTool::data.GetPiece(m_id);

    QAction *inLayoutOption = menu.addAction(tr("In layout"));
    inLayoutOption->setCheckable(true);
    inLayoutOption->setChecked(detail.IsInLayout());

    QAction *hideMainPathOption = menu.addAction(tr("Hide main path"));
    hideMainPathOption->setCheckable(true);
    hideMainPathOption->setChecked(detail.IsHideMainPath());

    QAction *forbidFlippingOption = menu.addAction(tr("Forbid flipping"));
    forbidFlippingOption->setCheckable(true);
    forbidFlippingOption->setChecked(detail.IsForbidFlipping());

    QAction *forceFlippingOption = menu.addAction(tr("Force flipping"));
    forceFlippingOption->setCheckable(true);
    forceFlippingOption->setChecked(detail.IsForceFlipping());

    QAction *showFullPiece = menu.addAction(tr("Show full piece"));
    showFullPiece->setCheckable(true);
    {
        const QLineF mirrorLine = detail.SeamAllowanceMirrorLine(&(VAbstractTool::data));
        showFullPiece->setEnabled(not mirrorLine.isNull());
        showFullPiece->setChecked(true);
        if (not mirrorLine.isNull())
        {
            showFullPiece->setChecked(detail.IsShowFullPiece());
        }
    }

    QAction *reseteLabelTemplateOption = menu.addAction(tr("Reset piece label template"));
    reseteLabelTemplateOption->setEnabled(not doc->GetDefaultPieceLabelPath().isEmpty());

    QAction *actionRemove = menu.addAction(FromTheme(VThemeIcon::EditDelete), tr("Delete"));
    actionRemove->setDisabled(_referens > 0);

    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == actionOption)
    {
        ShowOptions();
    }
    else if (selectedAction == inLayoutOption)
    {
        ToggleInLayout(selectedAction->isChecked());
    }
    else if (selectedAction == hideMainPathOption)
    {
        ToggleHideMainPath(selectedAction->isChecked());
    }
    else if (selectedAction == forbidFlippingOption)
    {
        ToggleForbidFlipping(selectedAction->isChecked());
    }
    else if (selectedAction == forceFlippingOption)
    {
        ToggleForceFlipping(selectedAction->isChecked());
    }
    else if (selectedAction == reseteLabelTemplateOption)
    {
        ResetPieceLabelTemplate();
    }
    else if (selectedAction == showFullPiece)
    {
        ToggleShowFullPiece(selectedAction->isChecked());
    }
    else if (selectedAction == actionRemove)
    {
        try
        {
            DeleteFromMenu();
        }
        catch (const VExceptionToolWasDeleted &e)
        {
            Q_UNUSED(e);
            return; // Leave this method immediately!!!
        }
        // Leave this method immediately after call!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            if (ConfirmDeletion() == QMessageBox::Yes)
            {
                const QList<VToolSeamAllowance *> toolList = SelectedTools();

                try
                {
                    if (not toolList.isEmpty())
                    {
                        VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("multi deletion"));

                        for (auto *tool : toolList)
                        {
                            tool->RemoveWithConfirm(false);
                        }
                    }
                    DeleteToolWithConfirm(false);
                }
                catch (const VExceptionToolWasDeleted &e)
                {
                    Q_UNUSED(e);
                    if (not toolList.isEmpty())
                    {
                        VAbstractApplication::VApp()->getUndoStack()->endMacro();
                    }
                    return; // Leave this method immediately!!!
                }
            }
            break;
        default:
            break;
    }

    QGraphicsPathItem::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SetDialog()
{
    SCASSERT(not m_dialog.isNull());
    const QPointer<DialogSeamAllowance> dialogTool = qobject_cast<DialogSeamAllowance *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->SetPiece(VAbstractTool::data.GetPiece(m_id));
    dialogTool->EnableApply(true);
}

//---------------------------------------------------------------------------------------------------------------------
VToolSeamAllowance::VToolSeamAllowance(const VToolSeamAllowanceInitData &initData, QGraphicsItem *parent)
  : VInteractiveTool(initData.doc, initData.data, initData.id),
    QGraphicsPathItem(parent),
    m_sceneDetails(initData.scene),
    m_drawName(initData.drawName),
    m_seamAllowance(new VNoBrushScalePathItem(this)),
    m_dataLabel(new VTextGraphicsItem(VTextGraphicsItem::ItemType::PieceLabel, this)),
    m_patternInfo(new VTextGraphicsItem(VTextGraphicsItem::ItemType::PatternLabel, this)),
    m_grainLine(new VGrainlineItem(VColorRole::PieceColor, this)),
    m_passmarks(new QGraphicsPathItem(this)),
    m_placeLabels(new QGraphicsPathItem(this)),
    m_mirrorLine(new QGraphicsPathItem(this)),
    m_foldLineMark(new QGraphicsPathItem(this)),
    m_foldLineLabel(new QGraphicsPathItem(this)),
    m_foldLineLabelText(new QGraphicsSimpleTextItem(this)),
    m_patternUpdateInfoWatcher(new QFutureWatcher<void>(this)),
    m_pieceUpdateInfoWatcher(new QFutureWatcher<void>(this))
{
    m_dataLabel->setVisible(false);
    m_patternInfo->setVisible(false);

    VPiece const detail = initData.data->GetPiece(initData.id);
    ReinitInternals(detail, m_sceneDetails);
    VToolSeamAllowance::AllowSelecting(true);
    EnableToolMove(true);
    VToolSeamAllowance::AllowHover(true);

    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    VToolSeamAllowance::ToolCreation(initData.typeCreation);
    setAcceptHoverEvents(m_acceptHoverEvents);

    connect(this, &VToolSeamAllowance::ChoosedTool, m_sceneDetails, &VMainGraphicsScene::ChoosedItem);
    connect(m_sceneDetails, &VMainGraphicsScene::EnableToolMove, this, &VToolSeamAllowance::EnableToolMove);
    connect(m_sceneDetails, &VMainGraphicsScene::ItemSelection, this, &VToolSeamAllowance::ToolSelectionType);
    connect(m_sceneDetails, &VMainGraphicsScene::UpdatePassmarks, this, &VToolSeamAllowance::UpdatePassmarks);
    connect(doc, &VAbstractPattern::CancelLabelRendering, this, &VToolSeamAllowance::CancelLabelRendering);

    ConnectOutsideSignals();

    m_foldLineMark->setBrush(Qt::SolidPattern);

    connect(qApp, &QCoreApplication::aboutToQuit, m_patternUpdateInfoWatcher,
            [this]()
            {
                m_patternUpdateInfoWatcher->cancel();
                m_patternUpdateInfoWatcher->waitForFinished();
            });
    connect(m_patternUpdateInfoWatcher, &QFutureWatcher<void>::finished, this,
            [this]()
            {
                if (m_patternUpdateInfoWatcher->isCanceled())
                {
                    return;
                }

                setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
                m_patternInfo->show();
                UpdateLabelItem(m_patternInfo, m_patternLabelPos, m_patternLabelAngle);
                setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
                if (m_patternLabelInfoStale)
                {
                    m_patternLabelInfoStale = false;
                    UpdatePatternInfo();
                }
            });
    connect(qApp, &QCoreApplication::aboutToQuit, m_pieceUpdateInfoWatcher,
            [this]()
            {
                m_pieceUpdateInfoWatcher->cancel();
                m_pieceUpdateInfoWatcher->waitForFinished();
            });
    connect(m_pieceUpdateInfoWatcher, &QFutureWatcher<void>::finished, this,
            [this]()
            {
                if (m_pieceUpdateInfoWatcher->isCanceled())
                {
                    return;
                }

                setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
                m_dataLabel->show();
                UpdateLabelItem(m_dataLabel, m_pieceLabelPos, m_pieceLabelAngle);
                setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
                if (m_pieceLabelInfoStale)
                {
                    m_pieceLabelInfoStale = false;
                    UpdateDetailLabel();
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdateExcludeState()
{
    const VPiece detail = VAbstractTool::data.GetPiece(m_id);
    const VPiecePath &path = detail.GetPath();
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        const VPieceNode &node = path.at(i);
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            auto *tool = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(node.GetId()));
            SCASSERT(tool != nullptr);

            tool->SetExluded(node.IsExcluded());
            tool->setVisible(not node.IsExcluded()); // Hide excluded point
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdateInternalPaths()
{
    VPiece piece = VAbstractTool::data.GetPiece(m_id);
    piece.TestInternalPaths(&(VAbstractTool::data));
    const QVector<quint32> paths = piece.GetInternalPaths();
    for (auto path : paths)
    {
        try
        {
            if (auto *tool = qobject_cast<VToolPiecePath *>(VAbstractPattern::getTool(path)))
            {
                tool->RefreshGeometry();
            }
        }
        catch (const VExceptionBadId &)
        {
            // ignore
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::RefreshGeometry(bool updateChildren)
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

    const VPiece detail = VAbstractTool::data.GetPiece(m_id);

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    const bool combineTogether = settings->IsBoundaryTogetherWithNotches();

    QFuture<QPainterPath> const futurePath = QtConcurrent::run(
        [this, detail, combineTogether]() { return RenderSeamPath(detail, combineTogether, getData()); });

    QFuture<QPainterPath> futurePassmarks;
    if (!combineTogether)
    {
        futurePassmarks = QtConcurrent::run([this, detail]() { return RenderPassmarks(detail, getData()); });
    }

    QFuture<VFoldLine> const futureFoldLine =
        QtConcurrent::run([this, detail]() { return RenderFoldLine(detail, getData()); });

    QFuture<QPainterPath> const futureMirrorLine = QtConcurrent::run(
        [this, detail, combineTogether]() { return RenderMirrorLine(detail, getData(), combineTogether); });

    QFuture<QPainterPath> futureSeamAllowance;
    QFuture<bool> futureSeamAllowanceValid;

    if (detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn())
    {
        futureSeamAllowance = QtConcurrent::run(
            [this, detail, combineTogether]() { return RenderSeamAllowancePath(detail, combineTogether, getData()); });
        futureSeamAllowanceValid =
            QtConcurrent::run([this, detail]() { return detail.IsSeamAllowanceValid(getData()); });
    }

    this->setPos(detail.GetMx(), detail.GetMy());

    QPainterPath path;

    if (VAbstractApplication::VApp()->Settings()->IsPieceShowMainPath() || not detail.IsHideMainPath() ||
        not detail.IsSeamAllowance() || detail.IsSeamAllowanceBuiltIn())
    {
        m_mainPath = QPainterPath();
        m_seamAllowance->setBrush(QBrush(VSceneStylesheet::PatternPieceStyle().PieceColor(), Qt::Dense7Pattern));
        path = futurePath.result();
    }
    else
    {
        m_seamAllowance->setBrush(QBrush(Qt::NoBrush)); // Disable if the main path was hidden
        // need for returning a bounding rect when main path is not visible
        m_mainPath = futurePath.result();
        path = QPainterPath();
    }

    this->setPath(path);

    m_placeLabels->setPath(detail.PlaceLabelPath(this->getData()));

    if (detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn())
    {
        if (not futureSeamAllowanceValid.result())
        {
            const QString errorMsg = QObject::tr("Piece '%1'. Seam allowance is not valid.").arg(detail.GetName());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
        path.addPath(futureSeamAllowance.result());
        path.setFillRule(Qt::OddEvenFill);
        m_seamAllowance->setPath(path);

        m_pieceBoundingRect = m_seamAllowance->path().controlPointRect();
    }
    else
    {
        m_seamAllowance->setPath(QPainterPath());

        m_pieceBoundingRect = m_mainPath.controlPointRect();
    }

    m_mirrorLine->setPath(futureMirrorLine.result());

    UpdateFoldLine(futureFoldLine.result());

    if (VAbstractApplication::VApp()->IsAppInGUIMode())
    {
        QTimer::singleShot(100ms, Qt::CoarseTimer, this,
                           [this, updateChildren]()
                           {
                               this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
                               UpdateDetailLabel();
                               UpdatePatternInfo();
                               UpdateGrainline();
                               UpdateExcludeState();
                               if (updateChildren)
                               {
                                   UpdateInternalPaths();
                               }
                               this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
                           });
    }
    else
    {
        UpdateDetailLabel();
        UpdatePatternInfo();
        UpdateGrainline();
        UpdateExcludeState();
        if (updateChildren)
        {
            UpdateInternalPaths();
        }
    }

    m_passmarks->setPath(!combineTogether ? futurePassmarks.result() : QPainterPath());

    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    // Now we can start checking validity of the grainline
    m_geometryIsReady = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveDialogChange(const QString &undoText)
{
    SCASSERT(not m_dialog.isNull());
    auto *dialogTool = qobject_cast<DialogSeamAllowance *>(m_dialog.data());
    SCASSERT(dialogTool != nullptr);
    const VPiece newDet = dialogTool->GetPiece();
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);

    CancelLabelRendering();

    QVector<QPointer<VUndoCommand>> &undocommands = dialogTool->UndoStack();
    const bool groupChange = not undocommands.isEmpty();

    auto *saveCommand = new SavePieceOptions(oldDet, newDet, doc, m_id);
    if (auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow()))
    { // Better not to crash here, just silently do not update list.
        connect(saveCommand, &SavePieceOptions::UpdateGroups, window, &VAbstractMainWindow::UpdateDetailsList);
    }

    if (groupChange)
    {
        VAbstractApplication::VApp()->getUndoStack()->beginMacro(undoText.isEmpty() ? saveCommand->text() : undoText);

        for (auto command : undocommands)
        {
            VAbstractApplication::VApp()->getUndoStack()->push(command);
            command.clear(); // To prevent double free memory
        }
        undocommands.clear();
    }

    VAbstractApplication::VApp()->getUndoStack()->push(saveCommand);

    if (groupChange)
    {
        VAbstractApplication::VApp()->getUndoStack()->endMacro();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ShowOptions()
{
    QPointer<DialogSeamAllowance> const dialog =
        new DialogSeamAllowance(getData(), doc, m_id, VAbstractValApplication::VApp()->getMainWindow());
    dialog->EnableApply(true);
    m_dialog = dialog;
    m_dialog->setModal(true);
    connect(m_dialog.data(), &DialogTool::DialogClosed, this, &VToolSeamAllowance::FullUpdateFromGuiOk);
    connect(m_dialog.data(), &DialogTool::DialogApplied, this, &VToolSeamAllowance::FullUpdateFromGuiApply);
    SetDialog();
    m_dialog->show();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleInLayout(bool checked)
{
    auto *toggleInLayout = new TogglePieceInLayout(m_id, checked, &(VAbstractTool::data), doc);
    connect(toggleInLayout, &TogglePieceInLayout::Toggled, doc, &VAbstractPattern::CheckInLayoutList);
    VAbstractApplication::VApp()->getUndoStack()->push(toggleInLayout);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleHideMainPath(bool checked)
{
    auto *toggleHideMainPath = new class ToggleHideMainPath(m_id, checked, &(VAbstractTool::data), doc);
    connect(toggleHideMainPath, &ToggleHideMainPath::Toggled, this, [this]() { RefreshGeometry(false); });
    VAbstractApplication::VApp()->getUndoStack()->push(toggleHideMainPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleForbidFlipping(bool checked)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new TogglePieceForceForbidFlipping(
        m_id, checked, ForceForbidFlippingType::ForbidFlipping, &(VAbstractTool::data), doc));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleForceFlipping(bool checked)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new TogglePieceForceForbidFlipping(
        m_id, checked, ForceForbidFlippingType::ForceFlipping, &(VAbstractTool::data), doc));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleShowFullPiece(bool checked)
{
    auto *toggleShowFullPiece = new class ToggleShowFullPiece(m_id, checked, &(VAbstractTool::data), doc);
    connect(toggleShowFullPiece, &ToggleShowFullPiece::Toggled, this, [this]() { RefreshGeometry(true); });
    VAbstractApplication::VApp()->getUndoStack()->push(toggleShowFullPiece);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::DeleteFromMenu()
{
    DeleteToolWithConfirm();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleExcludeState(quint32 id)
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    for (int i = 0; i < oldDet.GetPath().CountNodes(); ++i)
    {
        VPieceNode node = oldDet.GetPath().at(i);
        if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
        {
            node.SetExcluded(not node.IsExcluded());
            newDet.GetPath()[i] = node;

            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleTurnPointState(quint32 id)
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    for (int i = 0; i < oldDet.GetPath().CountNodes(); ++i)
    {
        VPieceNode node = oldDet.GetPath().at(i);
        if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
        {
            node.SetTurnPoint(not node.IsTurnPoint());
            newDet.GetPath()[i] = node;

            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleNodePointAngleType(quint32 id, PieceNodeAngle type)
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    for (int i = 0; i < oldDet.GetPath().CountNodes(); ++i)
    {
        VPieceNode node = oldDet.GetPath().at(i);
        if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
        {
            node.SetAngleType(type);
            newDet.GetPath()[i] = node;

            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToggleNodePointPassmark(quint32 id, bool toggle)
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    for (int i = 0; i < oldDet.GetPath().CountNodes(); ++i)
    {
        VPieceNode node = oldDet.GetPath().at(i);
        if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
        {
            node.SetPassmark(toggle);
            newDet.GetPath()[i] = node;

            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::TogglePassmarkAngleType(quint32 id, PassmarkAngleType type)
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    for (int i = 0; i < oldDet.GetPath().CountNodes(); ++i)
    {
        VPieceNode node = oldDet.GetPath().at(i);
        if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
        {
            node.SetPassmarkAngleType(type);
            newDet.GetPath()[i] = node;

            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::TogglePassmarkLineType(quint32 id, PassmarkLineType type)
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    for (int i = 0; i < oldDet.GetPath().CountNodes(); ++i)
    {
        VPieceNode node = oldDet.GetPath().at(i);
        if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
        {
            node.SetPassmarkLineType(type);
            newDet.GetPath()[i] = node;

            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ResetPieceLabelTemplate()
{
    const VPiece oldDet = VAbstractTool::data.GetPiece(m_id);
    VPiece newDet = oldDet;

    if (const QString path = doc->GetDefaultPieceLabelPath(); not path.isEmpty())
    {
        QVector<VLabelTemplateLine> lines;
        try
        {
            VLabelTemplate ltemplate;
            ltemplate.setXMLContent(VLabelTemplateConverter(path).Convert());
            lines = ltemplate.ReadLines();
            newDet.GetPieceLabelData().SetLabelTemplate(lines);
            VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_id));
        }
        catch (VException &e)
        {
            const QString errorMsg = QObject::tr("Piece '%1'. Unable to load default piece label template.\n%2\n%3")
                                         .arg(newDet.GetName(), e.ErrorMessage(), e.DetailedInformation());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::FindLabelGeometry(const VPatternLabelData &labelData, const QVector<quint32> &pins,
                                           qreal &rotationAngle, qreal &labelWidth, qreal &labelHeight, QPointF &pos)
    -> VPieceItem::MoveTypes
{

    VPieceItem::MoveTypes restrictions = VPieceItem::AllModifications;
    try
    {
        if (not qmu::QmuTokenParser::IsSingle(labelData.GetRotation()))
        {
            restrictions &= ~VPieceItem::IsRotatable;
        }

        Calculator cal1;
        rotationAngle = cal1.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetRotation());
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return VPieceItem::Error;
    }

    const quint32 topLeftPin = labelData.TopLeftPin();
    const quint32 bottomRightPin = labelData.BottomRightPin();

    if (topLeftPin != NULL_ID && pins.contains(topLeftPin) && bottomRightPin != NULL_ID &&
        pins.contains(bottomRightPin))
    {
        try
        {
            const auto topLeftPinPoint = VAbstractTool::data.GeometricObject<VPointF>(topLeftPin);
            const auto bottomRightPinPoint = VAbstractTool::data.GeometricObject<VPointF>(bottomRightPin);

            const auto labelRect =
                QRectF(static_cast<QPointF>(*topLeftPinPoint), static_cast<QPointF>(*bottomRightPinPoint));
            labelWidth = FromPixel(qAbs(labelRect.width()), *VDataTool::data.GetPatternUnit());
            labelHeight = FromPixel(qAbs(labelRect.height()), *VDataTool::data.GetPatternUnit());

            pos = labelRect.topLeft();

            restrictions &= ~VPieceItem::IsMovable;
            restrictions &= ~VPieceItem::IsResizable;

            return restrictions;
        }
        catch (const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    try
    {
        const bool widthIsSingle = qmu::QmuTokenParser::IsSingle(labelData.GetLabelWidth());

        Calculator cal1;
        labelWidth = cal1.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetLabelWidth());

        const bool heightIsSingle = qmu::QmuTokenParser::IsSingle(labelData.GetLabelHeight());

        Calculator cal2;
        labelHeight = cal2.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetLabelHeight());

        if (not widthIsSingle || not heightIsSingle)
        {
            restrictions &= ~VPieceItem::IsResizable;
        }
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return VPieceItem::Error;
    }

    const quint32 centerPin = labelData.CenterPin();
    if (centerPin != NULL_ID && pins.contains(centerPin))
    {
        try
        {
            const auto centerPinPoint = VAbstractTool::data.GeometricObject<VPointF>(centerPin);

            const qreal lWidth = ToPixel(labelWidth, *VDataTool::data.GetPatternUnit());
            const qreal lHeight = ToPixel(labelHeight, *VDataTool::data.GetPatternUnit());

            pos = static_cast<QPointF>(*centerPinPoint) - QRectF(0, 0, lWidth, lHeight).center();
            restrictions &= ~VPieceItem::IsMovable;
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

    return restrictions;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::FindGrainlineGeometry(const VGrainlineData &geom, const QVector<quint32> &pins, qreal &length,
                                               qreal &rotationAngle, QPointF &pos) -> VPieceItem::MoveTypes
{
    const quint32 topPin = geom.TopPin();

    if (const quint32 bottomPin = geom.BottomPin();
        topPin != NULL_ID && pins.contains(topPin) && bottomPin != NULL_ID && pins.contains(bottomPin))
    {
        try
        {
            const auto topPinPoint = VAbstractTool::data.GeometricObject<VPointF>(topPin);
            const auto bottomPinPoint = VAbstractTool::data.GeometricObject<VPointF>(bottomPin);

            QLineF grainline(static_cast<QPointF>(*bottomPinPoint), static_cast<QPointF>(*topPinPoint));
            length = FromPixel(grainline.length(), *VDataTool::data.GetPatternUnit());
            rotationAngle = grainline.angle();

            if (not VFuzzyComparePossibleNulls(rotationAngle, 0))
            {
                grainline.setAngle(0);
            }

            pos = grainline.p1();

            return VPieceItem::NotMovable;
        }
        catch (const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    VPieceItem::MoveTypes restrictions = VPieceItem::AllModifications;
    try
    {
        if (not qmu::QmuTokenParser::IsSingle(geom.GetRotation()))
        {
            restrictions &= ~VPieceItem::IsRotatable;
        }

        Calculator cal1;
        rotationAngle = cal1.EvalFormula(VAbstractTool::data.DataVariables(), geom.GetRotation());

        if (not qmu::QmuTokenParser::IsSingle(geom.GetLength()))
        {
            restrictions &= ~VPieceItem::IsResizable;
        }

        Calculator cal2;
        length = cal2.EvalFormula(VAbstractTool::data.DataVariables(), geom.GetLength());
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return VPieceItem::Error;
    }

    if (const quint32 centerPin = geom.CenterPin(); centerPin != NULL_ID && pins.contains(centerPin))
    {
        try
        {
            const auto centerPinPoint = VAbstractTool::data.GeometricObject<VPointF>(centerPin);

            const qreal cLength = ToPixel(length, *VDataTool::data.GetPatternUnit());
            QLineF grainline(centerPinPoint->x(), centerPinPoint->y(), centerPinPoint->x() - cLength / 2.0,
                             centerPinPoint->y());

            grainline.setAngle(rotationAngle);
            Swap(grainline);
            grainline.setLength(cLength);

            pos = grainline.p2();
            restrictions &= ~VPieceItem::IsMovable;
        }
        catch (const VExceptionBadId &)
        {
            pos = geom.GetPos();
        }
    }
    else
    {
        pos = geom.GetPos();
    }

    return restrictions;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitNodes(const VPiece &detail, VMainGraphicsScene *scene)
{
    const VPiecePath &path = detail.GetPath();
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        const VPieceNode &node = path.at(i);
        InitNode(node, scene, this);
        doc->IncrementReferens(VAbstractTool::data.GetGObject(node.GetId())->getIdTool());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitNode(const VPieceNode &node, VMainGraphicsScene *scene, VToolSeamAllowance *parent)
{
    SCASSERT(scene != nullptr)
    SCASSERT(parent != nullptr)

    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
        {
            auto *tool = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(node.GetId()));
            SCASSERT(tool != nullptr);

            if (tool->parent() != parent)
            {
                connect(tool, &VNodePoint::ShowOptions, parent, &VToolSeamAllowance::ShowOptions, Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleInLayout, parent, &VToolSeamAllowance::ToggleInLayout,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleForbidFlipping, parent, &VToolSeamAllowance::ToggleForbidFlipping,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleForceFlipping, parent, &VToolSeamAllowance::ToggleForceFlipping,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleShowFullPiece, parent, &VToolSeamAllowance::ToggleShowFullPiece,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::Delete, parent, &VToolSeamAllowance::DeleteFromMenu, Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleExcludeState, parent, &VToolSeamAllowance::ToggleExcludeState,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleTurnPointState, parent, &VToolSeamAllowance::ToggleTurnPointState,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ToggleSeamAllowanceAngleType, parent,
                        &VToolSeamAllowance::ToggleNodePointAngleType, Qt::UniqueConnection);
                connect(tool, &VNodePoint::TogglePassmark, parent, &VToolSeamAllowance::ToggleNodePointPassmark,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ChoosedTool, scene, &VMainGraphicsScene::ChoosedItem, Qt::UniqueConnection);
                connect(tool, &VNodePoint::TogglePassmarkAngleType, parent,
                        &VToolSeamAllowance::TogglePassmarkAngleType, Qt::UniqueConnection);
                connect(tool, &VNodePoint::TogglePassmarkLineType, parent, &VToolSeamAllowance::TogglePassmarkLineType,
                        Qt::UniqueConnection);
                connect(tool, &VNodePoint::ResetPieceLabelTemplate, parent,
                        &VToolSeamAllowance::ResetPieceLabelTemplate, Qt::UniqueConnection);
                tool->setParentItem(parent);
                tool->SetParentType(ParentType::Item);
                tool->SetExluded(node.IsExcluded());
            }
            tool->setVisible(not node.IsExcluded()); // Hide excluded point
            break;
        }
        case (Tool::NodeArc):
        case (Tool::NodeElArc):
        case (Tool::NodeSpline):
        case (Tool::NodeSplinePath):
            // Do nothing
            break;
        default:
            qDebug() << "Get wrong tool type. Ignore.";
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitCSAPaths(const VPiece &detail) const
{
    const QVector<CustomSARecord> records = detail.GetCustomSARecords();
    for (auto record : records)
    {
        doc->IncrementReferens(record.path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitInternalPaths(const VPiece &detail)
{
    const QVector<quint32> paths = detail.GetInternalPaths();
    for (auto path : paths)
    {
        auto *tool = qobject_cast<VToolPiecePath *>(VAbstractPattern::getTool(path));
        SCASSERT(tool != nullptr);

        if (tool->parent() != this)
        {
            tool->setParentItem(this);
            tool->SetParentType(ParentType::Item);
        }
        tool->show();
        doc->IncrementReferens(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitSpecialPoints(const QVector<quint32> &points) const
{
    for (auto point : points)
    {
        doc->IncrementReferens(point);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::DeleteToolWithConfirm(bool ask)
{
    if (ask && ConfirmDeletion() == QMessageBox::No)
    {
        return;
    }

    auto delDet = std::make_unique<DeletePiece>(doc, m_id, VAbstractTool::data, m_sceneDetails);
    VAbstractApplication::VApp()->getUndoStack()->push(delDet.release());

    // Throw exception, this will help prevent case when we forget to immediately quit function.
    throw VExceptionToolWasDeleted(tr("Tool was used after deleting."));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToolCreation(const Source &typeCreation)
{
    if (typeCreation == Source::FromGui || typeCreation == Source::FromTool)
    {
        VToolSeamAllowance::AddToFile();
    }
    else
    {
        VToolSeamAllowance::RefreshDataInFile();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::PrepareLabelData(const VPatternLabelData &labelData, const QVector<quint32> &pins,
                                          VTextGraphicsItem *labelItem, QPointF &pos, qreal &labelAngle) -> bool
{
    SCASSERT(labelItem != nullptr)

    qreal labelWidth = 0;
    qreal labelHeight = 0;
    const VTextGraphicsItem::MoveTypes type =
        FindLabelGeometry(labelData, pins, labelAngle, labelWidth, labelHeight, pos);
    if (type & VGrainlineItem::Error)
    {
        labelItem->hide();
        return false;
    }
    labelItem->SetMoveType(type);

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    QFont fnt = settings->GetLabelFont();
    {
        const int iFS = labelData.GetFontSize() < VCommonSettings::MinPieceLabelFontPointSize()
                            ? settings->GetPieceLabelFontPointSize()
                            : labelData.GetFontSize();
        fnt.setPointSize(qMax(iFS, 1));
        labelItem->SetSVGFontPointSize(iFS);
    }
    labelItem->SetFont(fnt);
    labelItem->SetSVGFontFamily(settings->GetLabelSVGFont());
    labelItem->SetSize(ToPixel(labelWidth, *VDataTool::data.GetPatternUnit()),
                       ToPixel(labelHeight, *VDataTool::data.GetPatternUnit()));

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::SelectedTools() const -> QList<VToolSeamAllowance *>
{
    QList<VToolSeamAllowance *> tools;
    if (m_sceneDetails)
    {
        if (const QList<QGraphicsItem *> list = m_sceneDetails->selectedItems(); not list.isEmpty())
        {
            tools.reserve(list.size());
            for (auto *item : list)
            {
                auto *tool = qgraphicsitem_cast<VToolSeamAllowance *>(item);
                if (tool != nullptr && tool->getId() != m_id)
                {
                    tools.append(tool);
                }
            }
        }
    }

    return tools;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::IsGrainlinePositionValid() const -> bool
{
    VPieceGrainline const grainLine = m_grainLine->Grainline();
    const VPiece detail = VAbstractTool::data.GetPiece(m_id);
    QVector<QPointF> contourPoints;
    detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn()
        ? CastTo(detail.FullSeamAllowancePoints(getData()), contourPoints)
        : CastTo(detail.FullMainPathPoints(getData()), contourPoints);

    return grainLine.IsPositionValid(contourPoints);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::IsFoldLinePositionValid(const QVector<QPainterPath> &shape, FoldLineType type) const -> bool
{
    if (type == FoldLineType::None || shape.isEmpty())
    {
        return true;
    }

    QPainterPath foldLinePath;
    foldLinePath.addPath(shape.constFirst());

    if (shape.size() > 1)
    {
        foldLinePath.addPath(shape.constLast());
    }

    const VPiece detail = VAbstractTool::data.GetPiece(m_id);
    QVector<QPointF> contourPoints;
    detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn()
        ? CastTo(detail.FullSeamAllowancePoints(getData()), contourPoints)
        : CastTo(detail.FullMainPathPoints(getData()), contourPoints);

    const QPainterPath contourPath = VGObject::PainterPath(contourPoints);
    return contourPath.contains(foldLinePath);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPointRecords(VAbstractPattern *doc, QDomElement &domElement,
                                         const QVector<quint32> &records, const QString &tag)
{
    if (not records.empty())
    {
        QDomElement pinsElement = doc->createElement(tag);
        for (auto record : records)
        {
            pinsElement.appendChild(
                doc->CreateElementWithText(VToolSeamAllowance::TagRecord, QString().setNum(record)));
        }
        domElement.appendChild(pinsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::DuplicateNode(const VPieceNode &node, const VToolSeamAllowanceInitData &initData) -> quint32
{
    SCASSERT(initData.scene != nullptr)
    SCASSERT(initData.doc != nullptr)
    SCASSERT(initData.data != nullptr)

    const QSharedPointer<VGObject> gobj = initData.data->GetGObject(node.GetId());
    auto *tool = qobject_cast<VAbstractNode *>(VAbstractPattern::getTool(node.GetId()));
    SCASSERT(tool != nullptr)

    VAbstractNodeInitData initNodeData;
    initNodeData.idObject = gobj->getIdObject();
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromGui;
    initNodeData.scene = initData.scene;
    initNodeData.drawName = initData.drawName;
    initNodeData.idTool = tool->GetIdTool();

    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
        {
            auto point = QSharedPointer<VPointF>(new VPointF(*qSharedPointerDynamicCast<VPointF>(gobj).data()));
            initNodeData.id = VAbstractTool::CreateNodePoint(initData.data, gobj->getIdObject(), point);
            VNodePoint::Create(initNodeData);
            break;
        }
        case (Tool::NodeArc):
            initNodeData.id = VAbstractTool::CreateNode<VArc>(initData.data, gobj->getIdObject());
            VNodeArc::Create(initNodeData);
            break;
        case (Tool::NodeElArc):
            initNodeData.id = VAbstractTool::CreateNode<VEllipticalArc>(initData.data, gobj->getIdObject());
            VNodeEllipticalArc::Create(initNodeData);
            break;
        case (Tool::NodeSpline):
            initNodeData.id = VAbstractTool::CreateNodeSpline(initData.data, gobj->getIdObject());
            VNodeSpline::Create(initNodeData);
            break;
        case (Tool::NodeSplinePath):
            initNodeData.id = VAbstractTool::CreateNodeSplinePath(initData.data, gobj->getIdObject());
            VNodeSplinePath::Create(initNodeData);
            break;
        default:
            qDebug() << "May be wrong tool type!!! Ignoring." << Q_FUNC_INFO;
            break;
    }
    return initNodeData.id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::DuplicatePiecePath(quint32 id, const VToolSeamAllowanceInitData &initData) -> quint32
{
    const VPiecePath path = initData.data->GetPiecePath(id);
    VPiecePath newPath = path;
    QMap<quint32, quint32> recordReplacements; // Not used
    newPath.SetNodes(DuplicateNodes(path, initData, recordReplacements));

    const quint32 idPath = initData.data->AddPiecePath(newPath);

    auto *tool = qobject_cast<VAbstractNode *>(VAbstractPattern::getTool(id));
    SCASSERT(tool != nullptr)

    VToolPiecePathInitData initNodeData;
    initNodeData.id = idPath;
    initNodeData.idObject = NULL_ID; // piece id
    initNodeData.scene = initData.scene;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.drawName = initData.drawName;
    initNodeData.idTool = tool->GetIdTool();
    initNodeData.path = newPath;

    VToolPiecePath::Create(initNodeData);
    return idPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::DuplicateCustomSARecords(const QVector<CustomSARecord> &records,
                                                  const VToolSeamAllowanceInitData &initData,
                                                  const QMap<quint32, quint32> &replacements) -> QVector<CustomSARecord>
{
    QVector<CustomSARecord> newRecords;
    newRecords.reserve(records.size());
    for (auto record : records)
    {
        record.path = DuplicatePiecePath(record.path, initData);
        record.startPoint = replacements.value(record.startPoint, NULL_ID);
        record.endPoint = replacements.value(record.endPoint, NULL_ID);
        newRecords.append(record);
    }
    return newRecords;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::DuplicateInternalPaths(const QVector<quint32> &iPaths,
                                                const VToolSeamAllowanceInitData &initData) -> QVector<quint32>
{
    QVector<quint32> newPaths;
    newPaths.reserve(iPaths.size());
    for (auto iPath : iPaths)
    {
        newPaths.append(DuplicatePiecePath(iPath, initData));
    }
    return newPaths;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::DuplicatePlaceLabels(const QVector<quint32> &placeLabels,
                                              const VToolSeamAllowanceInitData &initData) -> QVector<quint32>
{
    QVector<quint32> newPlaceLabels;
    newPlaceLabels.reserve(placeLabels.size());
    for (auto placeLabel : placeLabels)
    {
        QSharedPointer<VPlaceLabelItem> const label = initData.data->GeometricObject<VPlaceLabelItem>(placeLabel);
        auto *tool = qobject_cast<VAbstractNode *>(VAbstractPattern::getTool(placeLabel));
        SCASSERT(tool != nullptr)

        VToolPlaceLabelInitData initNodeData;
        initNodeData.idObject = NULL_ID; // piece id
        initNodeData.doc = initData.doc;
        initNodeData.data = initData.data;
        initNodeData.parse = Document::FullParse;
        initNodeData.typeCreation = Source::FromTool;
        initNodeData.drawName = initData.drawName;
        initNodeData.width = label->GetWidthFormula();
        initNodeData.height = label->GetHeightFormula();
        initNodeData.angle = label->GetAngleFormula();
        initNodeData.visibilityTrigger = label->GetVisibilityTrigger();
        initNodeData.type = label->GetLabelType();
        initNodeData.centerPoint = label->GetCenterPoint();
        initNodeData.notMirrored = label->IsNotMirrored();
        initNodeData.id = initNodeData.data->AddGObject(new VPlaceLabelItem(*label));
        initNodeData.idTool = tool->GetIdTool();

        VToolPlaceLabel::Create(initNodeData);
        newPlaceLabels.append(initNodeData.id);
    }
    return newPlaceLabels;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdateFoldLine(const VFoldLine &foldLine)
{
    const VPiece detail = VAbstractTool::data.GetPiece(m_id);

    if (detail.GetFoldLineType() == FoldLineType::None || (detail.IsShowFullPiece() && !detail.IsShowMirrorLine()))
    {
        m_foldLineLabelText->setVisible(false);
        m_foldLineMark->setPath(QPainterPath());
        m_foldLineLabel->setPath(QPainterPath());
        return;
    }

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    if (settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts())
    {
        m_foldLineLabelText->setVisible(false);
    }
    else
    {
        foldLine.UpdateFoldLineLabel(m_foldLineLabelText);
    }

    QVector<QPainterPath> const shape = foldLine.FoldLinePath();

    if (detail.GetFoldLineType() == FoldLineType::ThreeDots || detail.GetFoldLineType() == FoldLineType::ThreeX ||
        detail.GetFoldLineType() == FoldLineType::TwoArrows)
    {
        m_foldLineMark->setPath(!shape.isEmpty() ? shape.constFirst() : QPainterPath());
        m_foldLineLabel->setPath(QPainterPath());
    }
    else if (detail.GetFoldLineType() == FoldLineType::Text)
    {
        m_foldLineMark->setPath(QPainterPath());
        if (!shape.isEmpty() && (settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts()))
        {
            m_foldLineLabel->setPath(shape.constFirst());
        }
        else
        {
            m_foldLineLabel->setPath(QPainterPath());
        }
    }
    else
    {
        m_foldLineMark->setPath(!shape.isEmpty() ? shape.constFirst() : QPainterPath());

        if (shape.size() > 1 && (settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts()))
        {
            m_foldLineLabel->setPath(shape.constLast());
        }
        else
        {
            m_foldLineLabel->setPath(QPainterPath());
        }
    }

    if (m_geometryIsReady && not IsFoldLinePositionValid(shape, detail.GetFoldLineType()))
    {
        const QString errorMsg = QObject::tr("Piece '%1'. Fold line is not valid.").arg(detail.GetName());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSeamAllowance::DuplicateNodes(const VPiecePath &path, const VToolSeamAllowanceInitData &initData,
                                        QMap<quint32, quint32> &replacements) -> QVector<VPieceNode>
{
    QVector<VPieceNode> nodes;
    nodes.reserve(path.CountNodes());
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        VPieceNode nodeD = path.at(i);
        const quint32 oldId = nodeD.GetId();
        const quint32 id = DuplicateNode(nodeD, initData);
        if (id > NULL_ID)
        {
            nodeD.SetId(id);
            nodes.append(nodeD);
            replacements.insert(oldId, id);
        }
    }
    return nodes;
}
