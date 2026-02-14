/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#include "vabstractoperation.h"

#include <QScopeGuard>

#include "../../../undocommands/deltool.h"
#include "../../../undocommands/label/operationmovelabel.h"
#include "../../../undocommands/label/operationshowlabel.h"
#include "../../../undocommands/savetooloptions.h"
#include "../../../undocommands/undogroup.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vwidgets/vsimplepoint.h"
#include "vtools/undocommands/renameobject.h"

using namespace Qt::Literals::StringLiterals;

const QString VAbstractOperation::TagItem = QStringLiteral("item");
const QString VAbstractOperation::TagSource = QStringLiteral("source");
const QString VAbstractOperation::TagDestination = QStringLiteral("destination");

namespace
{
/**
 * @brief VisibilityGroupDataFromSource converts source list to visibility group list.
 * @param data container with pattern data
 * @param source list with source objects
 * @return visibility group data
 */
auto VisibilityGroupDataFromSource(const VContainer *data, const QVector<SourceItem> &source) -> QMap<quint32, quint32>
{
    QMap<quint32, quint32> groupData;

    for (const auto &sItem : source)
    {
        try
        {
            groupData.insert(sItem.id, data->GetGObject(sItem.id)->getIdTool());
        }
        catch (const VExceptionBadId &) // NOLINT(bugprone-empty-catch)
        {
            // ignore
        }
    }

    return groupData;
}

//---------------------------------------------------------------------------------------------------------------------
auto BuildNameMap(const QVector<SourceItem> &items) -> QMap<quint32, QString>
{
    QMap<quint32, QString> names;
    for (const auto &item : items)
    {
        if (item.id != NULL_ID)
        {
            names.insert(item.id, item.name);
        }
    }
    return names;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::getTagName() const -> QString
{
    return VAbstractPattern::TagOperation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::IsRemovable() const -> RemoveStatus
{
    if (!doc->IsPatternGraphComplete())
    {
        return RemoveStatus::Pending; // Data not ready yet
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    auto Filter = [](const auto &node) -> auto
    { return node.type != VNodeType::MODELING_TOOL && node.type != VNodeType::MODELING_OBJECT; };

    for (const auto &item : destination)
    {
        auto const dependecies = patternGraph->TryGetDependentNodes(item.id, 100, Filter);

        if (!dependecies)
        {
            return RemoveStatus::Pending; // Lock timeout
        }

        if (!dependecies->isEmpty())
        {
            return RemoveStatus::Blocked;
        }
    }

    return RemoveStatus::Removable;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SetNotes(const QString &notes)
{
    m_notes = notes;

    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::SourceItems() const -> QVector<SourceItem>
{
    return source;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::GroupVisibility(quint32 object, bool visible)
{
    if (!operatedObjects.contains(object))
    {
        return;
    }

    if (VAbstractSimple *obj = operatedObjects.value(object); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        auto *item = qobject_cast<VSimplePoint *>(obj);
        SCASSERT(item != nullptr)
        item->setVisible(visible);
    }
    else
    {
        auto *item = qobject_cast<VSimpleCurve *>(obj);
        SCASSERT(item != nullptr)
        item->setVisible(visible);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ChangeLabelPosition(quint32 id, const QPointF &pos)
{
    if (!operatedObjects.contains(id))
    {
        return;
    }

    if (VAbstractSimple *obj = operatedObjects.value(id); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        auto *item = qobject_cast<VSimplePoint *>(obj);
        SCASSERT(item != nullptr)
        QSharedPointer<VPointF> const point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->setMx(pos.x());
        point->setMy(pos.y());
        item->RefreshPointGeometry(*(point.data()));

        if (QGraphicsScene *sc = item->scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), item);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::IsLabelVisible(quint32 id) const -> bool
{
    if (!operatedObjects.contains(id))
    {
        return false;
    }

    if (const VAbstractSimple *obj = operatedObjects.value(id); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        return VAbstractTool::data.GeometricObject<VPointF>(id)->IsShowLabel();
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SetLabelVisible(quint32 id, bool visible)
{
    if (!operatedObjects.contains(id))
    {
        return;
    }

    if (VAbstractSimple *obj = operatedObjects.value(id); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        auto *item = qobject_cast<VSimplePoint *>(obj);
        SCASSERT(item != nullptr)
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->SetShowLabel(visible);
        item->RefreshPointGeometry(*point);
        if (QGraphicsScene *sc = item->scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), item);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ExtractData(const QDomElement &domElement, VAbstractOperationInitData &initData)
{
    initData.source = ExtractSourceData(domElement);
    initData.destination = ExtractDestinationData(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::ExtractSourceData(const QDomElement &domElement) -> QVector<SourceItem>
{
    QVector<SourceItem> source;
    const QDomNodeList nodeList = domElement.childNodes();
    QDOM_LOOP(nodeList, i)
    {
        if (const QDomElement dataElement = QDOM_ELEMENT(nodeList, i).toElement();
            not dataElement.isNull() && dataElement.tagName() == TagSource)
        {
            const QDomNodeList srcList = dataElement.childNodes();
            QDOM_LOOP(srcList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(srcList, j).toElement(); not element.isNull())
                {
                    // We no longer need to handle alias attribute here. The code can be removed.
                    // Name must be mandatory
                    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
                    const QString alias = VDomDocument::GetParametrEmptyString(element, AttrAlias);
                    QString name = VDomDocument::GetParametrEmptyString(element, AttrName);
                    if (name.isEmpty() && !alias.isEmpty())
                    {
                        name = alias;
                    }

                    source.append({.id = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR),
                                   .name = name,
                                   .penStyle = VDomDocument::GetParametrString(element, AttrPenStyle, TypeLineDefault),
                                   .color = VDomDocument::GetParametrString(element, AttrColor, ColorDefault)});
                }
            }
            return source;
        }
    }

    return source;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::ExtractDestinationData(const QDomElement &domElement) -> QVector<DestinationItem>
{
    QVector<DestinationItem> destination;
    const QDomNodeList nodeList = domElement.childNodes();
    QDOM_LOOP(nodeList, i)
    {
        if (const QDomElement dataElement = QDOM_ELEMENT(nodeList, i).toElement();
            not dataElement.isNull() && dataElement.tagName() == TagDestination)
        {
            const QDomNodeList srcList = dataElement.childNodes();
            QDOM_LOOP(srcList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(srcList, j).toElement(); not element.isNull())
                {
                    destination.append({.id = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR),
                                        .mx = VAbstractValApplication::VApp()->toPixel(
                                            VDomDocument::GetParametrDouble(element, AttrMx, QChar('1'))),
                                        .my = VAbstractValApplication::VApp()->toPixel(
                                            VDomDocument::GetParametrDouble(element, AttrMy, QChar('1'))),
                                        .showLabel = VDomDocument::GetParametrBool(element, AttrShowLabel, trueStr)});
                }
            }

            return destination;
        }
    }

    return destination;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::OperationColorsList() -> QMap<QString, QString>
{
    QMap<QString, QString> list = VAbstractTool::ColorsList();
    list.insert(ColorDefault, QCoreApplication::translate("VAbstractOperation", "default"));
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::FullUpdateFromFile()
{
    ReadAttributes();
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setToolTip(ComplexPointToolTip(i.key()));
            item->RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(i.key()));
        }
        else
        {
            auto *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setToolTip(ComplexCurveToolTip(i.key()));
            item->RefreshGeometry(VAbstractTool::data.GeometricObject<VAbstractCurve>(i.key()));
        }
    }
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowHover(bool enabled)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setAcceptHoverEvents(enabled);
        }
        else
        {
            auto *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setAcceptHoverEvents(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSelecting(bool enabled)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
        else
        {
            auto *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::EnableToolMove(bool move)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->EnableToolMove(move);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointHover(bool enabled)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setAcceptHoverEvents(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointSelecting(bool enabled)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointLabelHover(bool enabled)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->AllowLabelHover(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointLabelSelecting(bool enabled)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->AllowLabelSelecting(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplineHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::Spline);
    AllowCurveHover(enabled, GOType::CubicBezier);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplineSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::Spline);
    AllowCurveSelecting(enabled, GOType::CubicBezier);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplinePathHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::SplinePath);
    AllowCurveHover(enabled, GOType::CubicBezierPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplinePathSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::SplinePath);
    AllowCurveSelecting(enabled, GOType::CubicBezierPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowArcHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::Arc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowArcSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::Arc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowElArcHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::EllipticalArc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowElArcSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::EllipticalArc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ToolSelectionType(const SelectionType &type)
{
    VAbstractTool::ToolSelectionType(type);
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->ToolSelectionType(selectionType);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::Enable()
{
    const bool enabled = m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId();
    setEnabled(enabled);

    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->SetEnabled(enabled);
        }
        else
        {
            auto *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setEnabled(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ObjectSelected(bool selected, quint32 objId)
{
    emit ChangedToolSelection(selected, objId, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::DeleteFromLabel()
{
    try
    {
        DeleteToolWithConfirm();
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::LabelChangePosition(const QPointF &pos, quint32 labelId)
{
    if (!operatedObjects.contains(labelId))
    {
        return;
    }

    if (VAbstractSimple *obj = operatedObjects.value(labelId); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        auto *item = qobject_cast<VSimplePoint *>(obj);
        SCASSERT(item != nullptr)
        UpdateNamePosition(labelId, pos - item->pos());
    }
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractOperation::VAbstractOperation(const VAbstractOperationInitData &initData, QGraphicsItem *parent)
  : VDrawTool(initData.doc, initData.data, initData.id, initData.notes),
    QGraphicsLineItem(parent),
    source(initData.source),
    destination(initData.destination)
{
    connect(initData.doc,
            &VAbstractPattern::UpdateToolTip,
            this,
            [this]() -> void
            {
                QMapIterator i(operatedObjects);
                while (i.hasNext())
                {
                    i.next();
                    if (i.value()->GetType() == GOType::Point)
                    {
                        auto *item = qobject_cast<VSimplePoint *>(i.value());
                        SCASSERT(item != nullptr)
                        item->setToolTip(ComplexPointToolTip(i.key()));
                    }
                    else
                    {
                        auto *item = qobject_cast<VSimpleCurve *>(i.value());
                        SCASSERT(item != nullptr)
                        item->setToolTip(ComplexCurveToolTip(i.key()));
                    }
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ChangeLabelVisibility(quint32 id, bool visible)
{
    if (!operatedObjects.contains(id))
    {
        return;
    }

    if (const VAbstractSimple *obj = operatedObjects.value(id); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        if (auto dItem = std::find_if(destination.begin(),
                                      destination.end(),
                                      [id](const DestinationItem &dItem) -> bool { return dItem.id == id; });
            dItem != destination.end())
        {
            dItem->showLabel = visible;
        }
        VAbstractApplication::VApp()->getUndoStack()->push(new OperationShowLabel(doc, m_id, id, visible));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    bool const updateVisibilityOptions = NeedUpdateVisibilityGroup();

    if (updateVisibilityOptions)
    {
        VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("operation options"));
    }

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    VAbstractApplication::VApp()->getUndoStack()->push(saveOptions);

    if (updateVisibilityOptions)
    {
        vidtype const group = doc->GroupLinkedToTool(m_id);

        if (hasLinkedGroup)
        {
            if (group != null_id)
            {
                auto *groupOptions = new ChangeGroupOptions(doc, group, groupName, groupTags);
                connect(groupOptions, &ChangeGroupOptions::UpdateGroups, doc, &VAbstractPattern::UpdateVisiblityGroups);
                VAbstractApplication::VApp()->getUndoStack()->push(groupOptions);
            }
            else
            {
                VAbstractOperationInitData initData;
                initData.id = m_id;
                initData.hasLinkedVisibilityGroup = hasLinkedGroup;
                initData.visibilityGroupName = groupName;
                initData.visibilityGroupTags = groupTags;
                initData.data = &(VDataTool::data);
                initData.doc = doc;
                initData.source = source;

                VAbstractOperation::CreateVisibilityGroup(initData);
            }
        }
        else
        {
            auto *delGroup = new DelGroup(doc, group);
            connect(delGroup, &DelGroup::UpdateGroups, doc, &VAbstractPattern::UpdateVisiblityGroups);
            VAbstractApplication::VApp()->getUndoStack()->push(delGroup);
        }

        VAbstractApplication::VApp()->getUndoStack()->endMacro();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::PerformDelete()
{
    vidtype const group = doc->GroupLinkedToTool(m_id);
    bool const deleteGroup = group != null_id;

    qCDebug(vTool, "Begin deleting.");
    if (deleteGroup)
    {
        VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("delete operation"));

        qCDebug(vTool, "Deleting the linked group.");
        auto *delGroup = new DelGroup(doc, group);
        connect(delGroup, &DelGroup::UpdateGroups, doc, &VAbstractPattern::UpdateVisiblityGroups);
        VAbstractApplication::VApp()->getUndoStack()->push(delGroup);
    }

    qCDebug(vTool, "Deleting the tool.");
    auto *delTool = new DelTool(doc, m_id);
    connect(delTool, &DelTool::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(delTool);

    if (deleteGroup)
    {
        VAbstractApplication::VApp()->getUndoStack()->endMacro();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ReadToolAttributes(const QDomElement &domElement)
{
    VDrawTool::ReadToolAttributes(domElement);

    source = ExtractSourceData(domElement);
    destination = ExtractDestinationData(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (tag.hasAttribute(AttrSuffix))
    {
        tag.removeAttribute(AttrSuffix);
    }

    SaveSourceDestination(tag);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::UpdateNamePosition(quint32 id, const QPointF &pos)
{
    if (!operatedObjects.contains(id))
    {
        return;
    }

    if (const VAbstractSimple *obj = operatedObjects.value(id); (obj != nullptr) && obj->GetType() == GOType::Point)
    {
        if (auto dItem = std::find_if(destination.begin(),
                                      destination.end(),
                                      [id](const DestinationItem &dItem) -> bool { return dItem.id == id; });
            dItem != destination.end())
        {
            dItem->mx = pos.x();
            dItem->my = pos.y();
        }
        VAbstractApplication::VApp()->getUndoStack()->push(new OperationMoveLabel(m_id, doc, pos, id));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveSourceDestination(QDomElement &tag)
{
    VAbstractPattern::RemoveAllChildren(tag);

    QDomElement tagObjects = doc->createElement(TagSource);
    for (const auto &sItem : std::as_const(source))
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, sItem.id);
        doc->SetAttribute(item, AttrName, sItem.name);
        doc->SetAttributeOrRemoveIf<QString>(item,
                                             AttrPenStyle,
                                             sItem.penStyle,
                                             [](const QString &penStyle) noexcept -> bool
                                             { return penStyle == TypeLineDefault; });
        doc->SetAttributeOrRemoveIf<QString>(item,
                                             AttrColor,
                                             sItem.color,
                                             [](const QString &color) noexcept -> bool
                                             { return color == ColorDefault; });
        tagObjects.appendChild(item);
    }
    tag.appendChild(tagObjects);

    tagObjects = doc->createElement(TagDestination);
    for (auto dItem : std::as_const(destination))
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, dItem.id);

        VAbstractSimple const *obj = operatedObjects.value(dItem.id, nullptr);

        doc->SetAttributeOrRemoveIf<double>(item,
                                            AttrMx,
                                            VAbstractValApplication::VApp()->fromPixel(dItem.mx),
                                            [obj](double value) noexcept -> bool
                                            {
                                                Q_UNUSED(value)
                                                return obj && obj->GetType() != GOType::Point;
                                            });

        doc->SetAttributeOrRemoveIf<double>(item,
                                            AttrMy,
                                            VAbstractValApplication::VApp()->fromPixel(dItem.my),
                                            [obj](double value) noexcept -> bool
                                            {
                                                Q_UNUSED(value)
                                                return obj && obj->GetType() != GOType::Point;
                                            });

        doc->SetAttributeOrRemoveIf<bool>(item,
                                          AttrShowLabel,
                                          dItem.showLabel,
                                          [](bool showLabel) noexcept -> bool { return showLabel; });

        tagObjects.appendChild(item);
    }
    tag.appendChild(tagObjects);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::InitCurve(quint32 id, VContainer *data, GOType curveType, SceneObject sceneType)
{
    const QSharedPointer<VAbstractCurve> initCurve = data->GeometricObject<VAbstractCurve>(id);
    auto *curve = new VSimpleCurve(id, initCurve);
    curve->setParentItem(this);
    curve->SetType(curveType);
    curve->setToolTip(ComplexCurveToolTip(id));
    connect(curve, &VSimpleCurve::Selected, this, &VAbstractOperation::ObjectSelected);
    connect(curve,
            &VSimpleCurve::ShowContextMenu,
            this,
            [this](QGraphicsSceneContextMenuEvent *event, quint32 id) -> void { ShowContextMenu(event, id); });
    connect(curve,
            &VSimpleCurve::Choosed,
            this,
            [this, sceneType](quint32 id) -> void { emit ChoosedTool(id, sceneType); });
    connect(curve, &VSimpleCurve::Delete, this, &VAbstractOperation::DeleteFromLabel);
    curve->RefreshGeometry(VAbstractTool::data.GeometricObject<VAbstractCurve>(id));
    operatedObjects.insert(id, curve);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowCurveHover(bool enabled, GOType type)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() != GOType::Point)
        {
            auto *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            if (item->GetType() == type)
            {
                item->setAcceptHoverEvents(enabled);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowCurveSelecting(bool enabled, GOType type)
{
    QMapIterator i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() != GOType::Point)
        {
            auto *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            if (item->GetType() == type)
            {
                item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::NeedUpdateVisibilityGroup() const -> bool
{
    vidtype const group = doc->GroupLinkedToTool(m_id);

    if (hasLinkedGroup)
    {
        if (group != null_id)
        {
            if (groupName != doc->GetGroupName(group) || groupTags != doc->GetGroupTags(group))
            {
                return true; // rename group
            }
        }
        else
        {
            return true; // create group
        }
    }
    else
    {
        return group != null_id; // remove group
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::InitOperatedObjects()
{
    for (auto object : std::as_const(destination))
    {
        const QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(object.id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wswitch-default")
        QT_WARNING_DISABLE_CLANG("-Wswitch-default")

        switch (obj->getType())
        {
            case GOType::Point:
            {
                auto *point = new VSimplePoint(object.id, VColorRole::PatternColor);
                point->setParentItem(this);
                point->SetType(GOType::Point);
                point->setToolTip(ComplexPointToolTip(object.id));
                connect(point,
                        &VSimplePoint::Choosed,
                        this,
                        [this](quint32 id) -> void { emit ChoosedTool(id, SceneObject::Point); });
                connect(point, &VSimplePoint::Selected, this, &VAbstractOperation::ObjectSelected);
                connect(point,
                        &VSimplePoint::ShowContextMenu,
                        this,
                        [this](QGraphicsSceneContextMenuEvent *event, quint32 id) -> void
                        { ShowContextMenu(event, id); });
                connect(point, &VSimplePoint::Delete, this, &VAbstractOperation::DeleteFromLabel);
                connect(point, &VSimplePoint::NameChangedPosition, this, &VAbstractOperation::LabelChangePosition);
                point->RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(object.id));
                operatedObjects.insert(object.id, point);
                break;
            }
            case GOType::Arc:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::Arc);
                break;
            case GOType::EllipticalArc:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::ElArc);
                break;
            case GOType::Spline:
            case GOType::CubicBezier:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::Spline);
                break;
            case GOType::SplinePath:
            case GOType::CubicBezierPath:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::SplinePath);
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }

        QT_WARNING_POP
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::ComplexPointToolTip(quint32 itemId) const -> QString
{
    return QStringLiteral("<table>"
                          "<tr> <td><b>%1:</b> %2</td> </tr>"
                          "<tr> <td colspan=\"100%\"><hr/></td> </tr>"
                          "%3"
                          "</table>")
        .arg(tr("Label"), VAbstractTool::data.GetGObject(itemId)->name(), MakeToolTip());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::ComplexCurveToolTip(quint32 itemId) const -> QString
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(itemId);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%1:</b> %2</td> </tr>"
                            u"<tr> <td><b>%3:</b> %4 %5</td> </tr>"
                            u"<tr> <td colspan=\"100%\"><hr/></td> </tr>"
                            u"%6"
                            u"</table>"_s.arg(tr("Label"), curve->ObjectName(), tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curve->GetLength()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), MakeToolTip());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::VisibilityGroupToolTip() const -> QString
{
    vidtype const group = doc->GroupLinkedToTool(m_id);
    if (group != null_id)
    {
        return QStringLiteral("<tr> <td><b>%1:</b> %2</td> </tr>")
            .arg(tr("Visibility group"), doc->GetGroupName(group)); // 1, 2
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::CreateVisibilityGroup(const VAbstractOperationInitData &initData)
{
    if (not initData.hasLinkedVisibilityGroup && not initData.visibilityGroupName.isEmpty())
    {
        return;
    }

    const QMap<quint32, quint32> groupData = VisibilityGroupDataFromSource(initData.data, initData.source);
    vidtype const groupId = initData.data->getNextId();

    if (const QDomElement group = initData.doc->CreateGroup(groupId, initData.visibilityGroupName,
                                                            initData.visibilityGroupTags, groupData, initData.id);
        not group.isNull())
    {
        auto *addGroup = new AddGroup(group, initData.doc);
        connect(addGroup, &AddGroup::UpdateGroups, initData.doc, &VAbstractPattern::UpdateVisiblityGroups);
        VAbstractApplication::VApp()->getUndoStack()->push(addGroup);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::PrepareNames(VAbstractOperationInitData &initData)
{
    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (initData.suffix.isEmpty())
    {
        return;
    }

    for (int i = 0; i < initData.source.size(); ++i)
    {
        SourceItem &object = initData.source[i];

        if (!object.name.isEmpty())
        {
            continue;
        }

        if (const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id); obj->getType() == GOType::Point)
        {
            object.name = obj->name() + initData.suffix;
        }
        else
        {
            QSharedPointer<VAbstractCurve> const curve = initData.data->GeometricObject<VAbstractCurve>(object.id);
            object.name = curve->HeadlessName() + initData.suffix;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ProcessOperationToolOptions(const QDomElement &oldDomElement,
                                                     const QDomElement &newDomElement,
                                                     const QVector<SourceItem> &newSource)
{
    // Build maps for efficient lookup by id
    const QMap<quint32, QString> oldNames = BuildNameMap(source);
    const QMap<quint32, QString> newNames = BuildNameMap(newSource);

    // Check if there are any name changes
    if (oldNames == newNames)
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

    const QSet<quint32> oldIds = ConvertToSet<quint32>(oldNames.keys());
    const QSet<quint32> newIds = ConvertToSet<quint32>(newNames.keys());
    const QSet<quint32> commonIds = oldIds & newIds;

    // Collect all rename operations first
    QVector<QPair<quint32, QPair<QString, QString>>> renames;
    renames.reserve(commonIds.size());
    for (quint32 const id : std::as_const(commonIds))
    {
        const QString oldName = oldNames.value(id);
        const QString newName = newNames.value(id);

        if (oldName != newName)
        {
            renames.append(qMakePair(id, qMakePair(oldName, newName)));
        }
    }

    // Process rename operations, connecting signal only for the last one
    for (int i = 0; i < renames.size(); ++i)
    {
        const auto &rename = renames.at(i);

        if (const QSharedPointer<VGObject> obj = VDataTool::data.GetGObject(rename.first);
            obj->getType() == GOType::Point)
        {
            auto *renameLabel = new RenameLabel(rename.second.first, rename.second.second, doc, rename.first, newGroup);

            if (i == renames.size() - 1) // Last rename operation
            {
                connect(renameLabel, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            }
        }
        else
        {
            const CurveAliasType type = RenameAlias::CurveType(obj->getType());
            auto *renameName
                = new RenameAlias(type, rename.second.first, rename.second.second, doc, rename.first, newGroup);

            if (i == renames.size() - 1) // Last rename operation
            {
                connect(renameName, &RenameAlias::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            }
        }
    }

    undoStack->push(newGroup);
}
