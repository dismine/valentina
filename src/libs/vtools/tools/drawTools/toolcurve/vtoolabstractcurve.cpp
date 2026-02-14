/************************************************************************
 **
 **  @file   vtoolabstractcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 3, 2014
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

#include "vtoolabstractcurve.h"

#include <QColor>
#include <QFlags>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineF>
#include <QPen>
#include <QSharedPointer>
#include <QUndoStack>

#include "../../../undocommands/renameobject.h"
#include "../../../undocommands/savetooloptions.h"
#include "../../../visualization/line/visline.h"
#include "../../vabstracttool.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vdrawtool.h"
#include "../vgeometry/vabstractarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VToolAbstractCurve::VToolAbstractCurve(
    VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes, QGraphicsItem *parent)
  : VDrawTool(doc, data, id, notes),
    QGraphicsPathItem(parent),
    m_detailsMode(VAbstractApplication::VApp()->Settings()->IsShowCurveDetails())
{
    InitDefShape();
    setAcceptHoverEvents(m_acceptHoverEvents);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::shape() const -> QPainterPath
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    const QVector<QPointF> points = curve->GetPoints();

    QPainterPath path;
    for (qint32 i = 0; i < points.count() - 1; ++i)
    {
        path.moveTo(points.at(i));
        path.lineTo(points.at(i + 1));
    }

    if (m_isHovered || m_detailsMode)
    {
        path.addPath(VAbstractCurve::ShowDirection(curve->DirectionArrows(),
                                                   ScaleWidth(VAbstractCurve::LengthCurveDirectionArrow(),
                                                              SceneScale(scene()))));
    }
    path.setFillRule(Qt::WindingFill);
    return ItemShapeFromPath(path, pen());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal width = ScaleWidth(m_isHovered ? VAbstractApplication::VApp()->Settings()->WidthMainLine()
                                               : VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                   SceneScale(scene()));

    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    QColor const penColor =
        VSceneStylesheet::CorrectToolColor(this, VSceneStylesheet::CorrectToolColorForDarkTheme(curve->GetColor()));
    setPen(QPen(penColor, width, LineStyleToPenStyle(curve->GetPenStyle()), Qt::RoundCap));

    auto PaintSpline = [this, curve](QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) -> void
    {
        if (m_isHovered || m_detailsMode)
        {
            QPainterStateGuard const guard(painter);

            QPen arrowPen(pen());
            arrowPen.setStyle(Qt::SolidLine);

            painter->setPen(arrowPen);
            painter->setBrush(brush());

            painter->drawPath(VAbstractCurve::ShowDirection(
                curve->DirectionArrows(),
                ScaleWidth(VAbstractCurve::LengthCurveDirectionArrow(), SceneScale(scene()))));
        }

        PaintWithFixItemHighlightSelected<QGraphicsPathItem>(this, painter, option, widget);
    };

    PaintSpline(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::getTagName() const -> QString
{
    return VAbstractPattern::TagSpline;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::ShowHandles(bool show)
{
    Q_UNUSED(show);
    update(); // Show direction
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolAbstractCurve::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::Enable()
{
    const bool enabled = m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId();
    setEnabled(enabled);
    emit setEnabledPoint(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::SetDetailsMode(bool mode)
{
    m_detailsMode = mode;
    RefreshGeometry();
    ShowHandles(m_detailsMode);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::AllowHover(bool enabled)
{
    // Manually handle hover events. Need for setting cursor for not selectable paths.
    SetAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::MakeToolTip() const -> QString
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);

    const QString toolTip =
        u"<table>"
        u"<tr> <td><b>%4:</b> %5</td> </tr>"
        u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
        u"</table>"_s.arg(tr("Length"))
            .arg(VAbstractValApplication::VApp()->fromPixel(curve->GetLength()))
            .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Label"), curve->ObjectName());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::RefreshGeometry()
{
    InitDefShape();
    RefreshCtrlPoints();
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
// cppcheck-suppress unusedFunction
void VToolAbstractCurve::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    setToolTip(MakeToolTip());
    QGraphicsPathItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
// cppcheck-suppress unusedFunction
void VToolAbstractCurve::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    QGraphicsPathItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange hadle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
auto VToolAbstractCurve::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        emit ChangedToolSelection(value.toBool(), m_id, m_id);
    }

    return QGraphicsPathItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key release events.
 * @param event key release event.
 */
void VToolAbstractCurve::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            try
            {
                DeleteToolWithConfirm();
            }
            catch (const VExceptionToolWasDeleted &e)
            {
                Q_UNUSED(e)
                return; // Leave this method immediately!!!
            }
            break;
        default:
            break;
    }
    QGraphicsPathItem::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    QGraphicsPathItem::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    event->accept(); // Special for not selectable item first need to call standard mousePressEvent then accept event
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent  handle mouse release events.
 * @param event mouse release event.
 */
void VToolAbstractCurve::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (IsSelectedByReleaseEvent(this, event))
    {
        emit ChoosedTool(m_id, sceneType);
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    const QSharedPointer<VAbstractCurve> curve = qSharedPointerCast<VAbstractCurve>(obj);
    doc->SetAttribute(tag, AttrColor, curve->GetColor());
    doc->SetAttribute(tag, AttrPenStyle, curve->GetPenStyle());
    doc->SetAttribute(tag, AttrAScale, curve->GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrAlias,
                                         curve->GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::RefreshCtrlPoints()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    ShowContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::CorrectedSpline(const VSpline &spline, const SplinePointPosition &position, const QPointF &pos)
    -> VSpline
{
    VSpline spl;
    if (position == SplinePointPosition::FirstPoint)
    {
        QLineF line(static_cast<QPointF>(spline.GetP1()), pos);
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            line.setAngle(VisLine::CorrectAngle(line.angle()));
        }

        qreal newAngle1 = line.angle();
        QString newAngle1F = QString().setNum(newAngle1);

        qreal newLength1 = line.length();
        QString newLength1F = QString().setNum(VAbstractValApplication::VApp()->fromPixel(newLength1));

        if (not qmu::QmuTokenParser::IsSingle(spline.GetStartAngleFormula()))
        {
            newAngle1 = spline.GetStartAngle();
            newAngle1F = spline.GetStartAngleFormula();
        }

        if (not qmu::QmuTokenParser::IsSingle(spline.GetC1LengthFormula()))
        {
            newLength1 = spline.GetC1Length();
            newLength1F = spline.GetC1LengthFormula();
        }

        spl = VSpline(spline.GetP1(), spline.GetP4(), newAngle1, newAngle1F, spline.GetEndAngle(),
                      spline.GetEndAngleFormula(), newLength1, newLength1F, spline.GetC2Length(),
                      spline.GetC2LengthFormula());
    }
    else
    {
        QLineF line(static_cast<QPointF>(spline.GetP4()), pos);
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            line.setAngle(VisLine::CorrectAngle(line.angle()));
        }

        qreal newAngle2 = line.angle();
        QString newAngle2F = QString().setNum(newAngle2);

        qreal newLength2 = line.length();
        QString newLength2F = QString().setNum(VAbstractValApplication::VApp()->fromPixel(newLength2));

        if (not qmu::QmuTokenParser::IsSingle(spline.GetEndAngleFormula()))
        {
            newAngle2 = spline.GetEndAngle();
            newAngle2F = spline.GetEndAngleFormula();
        }

        if (not qmu::QmuTokenParser::IsSingle(spline.GetC2LengthFormula()))
        {
            newLength2 = spline.GetC2Length();
            newLength2F = spline.GetC2LengthFormula();
        }
        spl = VSpline(spline.GetP1(), spline.GetP4(), spline.GetStartAngle(), spline.GetStartAngleFormula(), newAngle2,
                      newAngle2F, spline.GetC1Length(), spline.GetC1LengthFormula(), newLength2, newLength2F);
    }

    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::InitDefShape()
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    this->setPath(curve->GetPath());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::GetLineColor() const -> QString
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    return curve->GetColor();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::SetLineColor(const QString &value)
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->SetColor(value);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::GetPenStyle() const -> QString
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    return curve->GetPenStyle();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::SetPenStyle(const QString &value)
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->SetPenStyle(value);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::name() const -> QString
{
    return ObjectName<VAbstractCurve>(m_id);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::GetApproximationScale() const -> qreal
{
    return VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id)->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::GetDuplicate() const -> quint32
{
    return VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id)->GetDuplicate();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractCurve::GetAliasSuffix() const -> QString
{
    return ObjectAliasSuffix<VAbstractCurve>(m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractCurve::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    setVisible(visible);
}

// VToolAbstractArc
//---------------------------------------------------------------------------------------------------------------------
VToolAbstractArc::VToolAbstractArc(
    VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes, QGraphicsItem *parent)
  : VToolAbstractCurve(doc, data, id, notes, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractArc::CenterPointName() const -> QString
{
    QSharedPointer<VAbstractArc> const arc = VAbstractTool::data.GeometricObject<VAbstractArc>(m_id);
    SCASSERT(arc.isNull() == false)

    return VAbstractTool::data.GetGObject(arc->GetCenter().id())->name();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractArc::SetAliasSuffix(const QString &alias)
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);

    if (!alias.isEmpty())
    {
        const QString newAlias = curve->GetTypeHead() + alias;

        if (QRegularExpression const rx(NameRegExp()); !rx.match(newAlias).hasMatch())
        {
            return; // Invalid format
        }

        if (!VAbstractTool::data.IsUnique(newAlias))
        {
            return; // Not unique in data
        }
    }

    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (!oldDomElement.isElement())
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
        return;
    }

    ToolChanges const changes = {.oldCenterLabel = CenterPointName(),
                                 .newCenterLabel = CenterPointName(),
                                 .oldAliasSuffix = curve->GetAliasSuffix(),
                                 .newAliasSuffix = alias};

    curve->SetAliasSuffix(alias);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);

    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);

    ProcessArcToolOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractArc::ProcessArcToolOptions(const QDomElement &oldDomElement,
                                             const QDomElement &newDomElement,
                                             const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VDrawTool::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    const QSharedPointer<VAbstractArc> arc = VAbstractTool::data.GeometricObject<VAbstractArc>(m_id);

    if (changes.CenterLabelChanged())
    {
        const RenameArcType arcType = arc->getType() == GOType::Arc ? RenameArcType::Arc : RenameArcType::ElArc;

        // Share the same center point as a base arc
        auto *renameArc = new RenameArc(arcType, changes.oldCenterLabel, changes.newCenterLabel, 0, doc, m_id, newGroup);
        if (!changes.AliasSuffixChanged())
        {
            connect(renameArc, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffixChanged())
    {
        const CurveAliasType arcType = arc->getType() == GOType::Arc ? CurveAliasType::Arc : CurveAliasType::ElArc;

        auto *renameAlias = new RenameAlias(arcType, changes.oldAliasSuffix, changes.newAliasSuffix, doc, m_id, newGroup);
        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
VToolAbstractOffsetCurve::VToolAbstractOffsetCurve(VAbstractPattern *doc,
                                                   VContainer *data,
                                                   quint32 id,
                                                   quint32 originCurveId,
                                                   QString name,
                                                   const QString &notes,
                                                   QGraphicsItem *parent)
  : VToolAbstractCurve(doc, data, id, notes, parent),
    m_originCurveId(originCurveId),
    m_name(std::move(name))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractOffsetCurve::GetApproximationScale() const -> qreal
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    SCASSERT(curve.isNull() == false)

    return curve->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->SetApproximationScale(value);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::SetName(const QString &name)
{
    UpdateNameField(VToolAbstractOffsetCurveField::Name, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::SetAliasSuffix(const QString &alias)
{
    UpdateNameField(VToolAbstractOffsetCurveField::AliasSuffix, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractOffsetCurve::CurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(m_originCurveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractCurve::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrCurve, m_originCurveId);
    doc->SetAttribute(tag, AttrName, m_name);

    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (!m_name.isEmpty() && tag.hasAttribute(AttrSuffix))
    {
        tag.removeAttribute(AttrSuffix);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::ReadToolAttributes(const QDomElement &domElement)
{
    VToolAbstractCurve::ReadToolAttributes(domElement);

    m_originCurveId = VDomDocument::GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
    m_name = VDomDocument::GetParametrString(domElement, AttrName);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::ProcessOffsetCurveToolOptions(const QDomElement &oldDomElement,
                                                             const QDomElement &newDomElement,
                                                             const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VToolAbstractCurve::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    if (changes.NameChanged())
    {
        auto *renameName
            = new RenameAlias(CurveAliasType::SplinePath, changes.oldName, changes.newName, doc, m_id, newGroup);
        if (!changes.AliasSuffixChanged())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffixChanged())
    {
        auto *renameAlias = new RenameAlias(CurveAliasType::SplinePath,
                                            changes.oldAliasSuffix,
                                            changes.newAliasSuffix,
                                            doc,
                                            m_id,
                                            newGroup);
        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractOffsetCurve::UpdateNameField(VToolAbstractOffsetCurveField field, const QString &value)
{
    // Validation - name field requires non-empty value
    if (field == VToolAbstractOffsetCurveField::Name && value.isEmpty())
    {
        return; // Name is required
    }

    // Get curve for alias validation
    QSharedPointer<VAbstractCurve> curve;
    if (field == VToolAbstractOffsetCurveField::AliasSuffix)
    {
        curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    }

    // Validate format and uniqueness for non-empty values
    if (!value.isEmpty())
    {
        // For alias, prepend type head; for name, use as-is
        const QString fullName = (field == VToolAbstractOffsetCurveField::AliasSuffix) ? curve->GetTypeHead() + value
                                                                                       : value;

        if (QRegularExpression const rx(NameRegExp()); !rx.match(fullName).hasMatch())
        {
            return; // Invalid format
        }

        if (!VAbstractTool::data.IsUnique(fullName))
        {
            return; // Not unique in data
        }

        // Check conflicts
        if (HasConflict(value, field))
        {
            return; // Conflicts with other identifiers
        }
    }

    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (!oldDomElement.isElement())
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
        return;
    }

    // Build changes struct
    ToolChanges const changes = {.oldName = GetName(),
                                 .newName = (field == VToolAbstractOffsetCurveField::Name) ? value : GetName(),
                                 .oldAliasSuffix = GetAliasSuffix(),
                                 .newAliasSuffix = (field == VToolAbstractOffsetCurveField::AliasSuffix)
                                                       ? value
                                                       : GetAliasSuffix()};

    // Update the appropriate member variable or curve property
    QSharedPointer<VGObject> obj;
    if (field == VToolAbstractOffsetCurveField::Name)
    {
        m_name = value;
        obj = VAbstractTool::data.GetGObject(m_id);
    }
    else // AliasSuffix
    {
        curve->SetAliasSuffix(value);
        obj = qSharedPointerCast<VGObject>(curve);
    }

    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);
    ProcessOffsetCurveToolOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolAbstractOffsetCurve::HasConflict(const QString &value, VToolAbstractOffsetCurveField currentField) const
    -> bool
{
    if (value.isEmpty())
    {
        return false;
    }

    const QString currentName = GetName();
    const QString currentAlias = GetAliasSuffix();

    // Check conflict based on which field is being updated
    if (currentField == VToolAbstractOffsetCurveField::Name)
    {
        // Name cannot conflict with non-empty alias
        return !currentAlias.isEmpty() && value == currentAlias;
    }
    // AliasSuffix
    {
        // Alias cannot conflict with name
        return value == currentName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
VToolAbstractBezier::VToolAbstractBezier(
    VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes, QGraphicsItem *parent)
  : VToolAbstractCurve(doc, data, id, notes, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractBezier::SetAliasSuffix(const QString &alias)
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);

    if (!alias.isEmpty())
    {
        const QString newAlias = curve->GetTypeHead() + alias;

        if (QRegularExpression const rx(NameRegExp()); !rx.match(newAlias).hasMatch())
        {
            return; // Invalid format
        }

        if (!VAbstractTool::data.IsUnique(newAlias))
        {
            return; // Not unique in data
        }
    }

    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (!oldDomElement.isElement())
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
        return;
    }

    GOType const curveType = curve->getType();
    ToolChanges changes;
    if (curveType == GOType::CubicBezier || curveType == GOType::Spline)
    {
        QSharedPointer<VAbstractCubicBezier> const bezierCurve = qSharedPointerCast<VAbstractCubicBezier>(curve);
        changes = {.oldP1Label = bezierCurve->GetP1().name(),
                   .newP1Label = bezierCurve->GetP1().name(),
                   .oldP4Label = bezierCurve->GetP4().name(),
                   .newP4Label = bezierCurve->GetP4().name(),
                   .oldAliasSuffix = bezierCurve->GetAliasSuffix(),
                   .newAliasSuffix = alias};
    }
    else if (curveType == GOType::CubicBezierPath || curveType == GOType::SplinePath)
    {
        QSharedPointer<VAbstractCubicBezierPath> const path = qSharedPointerCast<VAbstractCubicBezierPath>(curve);
        changes = {.oldP1Label = path->FirstPoint().name(),
                   .newP1Label = path->FirstPoint().name(),
                   .oldP4Label = path->LastPoint().name(),
                   .newP4Label = path->LastPoint().name(),
                   .oldAliasSuffix = path->GetAliasSuffix(),
                   .newAliasSuffix = alias};
    }

    curve->SetAliasSuffix(alias);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);

    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);

    if (curveType == GOType::CubicBezier || curveType == GOType::Spline)
    {
        ProcessSplineToolOptions(oldDomElement, newDomElement, changes);
    }
    else if (curveType == GOType::CubicBezierPath || curveType == GOType::SplinePath)
    {
        ProcessSplinePathToolOptions(oldDomElement, newDomElement, changes);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractBezier::ProcessSplineToolOptions(const QDomElement &oldDomElement,
                                                   const QDomElement &newDomElement,
                                                   const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VDrawTool::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }
    const auto oldSpline = VAbstractTool::data.GeometricObject<VAbstractCubicBezier>(m_id);

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    if (changes.P1LabelChanged() || changes.P4LabelChanged())
    {
        auto *renamePair = RenamePair::CreateForSpline(std::make_pair(changes.oldP1Label, changes.oldP4Label),
                                                       std::make_pair(changes.newP1Label, changes.newP4Label),
                                                       oldSpline->GetDuplicate(),
                                                       doc,
                                                       m_id,
                                                       newGroup);
        if (!changes.AliasSuffixChanged())
        {
            connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffixChanged())
    {
        auto *renameAlias = new RenameAlias(CurveAliasType::Spline,
                                            changes.oldAliasSuffix,
                                            changes.newAliasSuffix,
                                            doc,
                                            m_id,
                                            newGroup);
        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolAbstractBezier::ProcessSplinePathToolOptions(const QDomElement &oldDomElement,
                                                       const QDomElement &newDomElement,
                                                       const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VDrawTool::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    const auto oldSplinePath = VAbstractTool::data.GeometricObject<VAbstractCubicBezierPath>(m_id);

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    if (changes.P1LabelChanged() || changes.P4LabelChanged())
    {
        auto *renamePair = RenamePair::CreateForSplinePath(std::make_pair(changes.oldP1Label, changes.oldP4Label),
                                                           std::make_pair(changes.newP1Label, changes.newP4Label),
                                                           oldSplinePath->GetDuplicate(),
                                                           doc,
                                                           m_id,
                                                           newGroup);
        if (!changes.AliasSuffixChanged())
        {
            connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffixChanged())
    {
        auto *renameAlias = new RenameAlias(CurveAliasType::SplinePath,
                                            changes.oldAliasSuffix,
                                            changes.newAliasSuffix,
                                            doc,
                                            m_id,
                                            newGroup);
        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}
