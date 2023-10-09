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
#include "../../../undocommands/deltool.h"
#include "../../../undocommands/label/operationmovelabel.h"
#include "../../../undocommands/label/operationshowlabel.h"
#include "../../../undocommands/savetooloptions.h"
#include "../../../undocommands/undogroup.h"
#include "../vgeometry/vpointf.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

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
        catch (const VExceptionBadId &)
        {
            // ignore
        }
    }

    return groupData;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::getTagName() const -> QString
{
    return VAbstractPattern::TagOperation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::Suffix() const -> QString
{
    return suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SetSuffix(const QString &suffix)
{
    // Don't know if need check name here.
    this->suffix = suffix;
    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOption(obj);
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
    if (operatedObjects.contains(object))
    {
        VAbstractSimple *obj = operatedObjects.value(object);
        if (obj && obj->GetType() == GOType::Point)
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
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(obj);
            SCASSERT(item != nullptr)
            QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
            point->setMx(pos.x());
            point->setMy(pos.y());
            item->RefreshPointGeometry(*(point.data()));

            if (QGraphicsScene *sc = item->scene())
            {
                VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), item);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::IsLabelVisible(quint32 id) const -> bool
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            return VAbstractTool::data.GeometricObject<VPointF>(id)->IsShowLabel();
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SetLabelVisible(quint32 id, bool visible)
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
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
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement dataElement = nodeList.at(i).toElement();
        if (not dataElement.isNull() && dataElement.tagName() == TagSource)
        {
            const QDomNodeList srcList = dataElement.childNodes();
            for (qint32 j = 0; j < srcList.size(); ++j)
            {
                const QDomElement element = srcList.at(j).toElement();
                if (not element.isNull())
                {
                    SourceItem item;
                    item.id = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR);
                    item.alias = VDomDocument::GetParametrEmptyString(element, AttrAlias);
                    item.penStyle = VDomDocument::GetParametrString(element, AttrPenStyle, TypeLineDefault);
                    item.color = VDomDocument::GetParametrString(element, AttrColor, ColorDefault);
                    source.append(item);
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
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement dataElement = nodeList.at(i).toElement();
        if (not dataElement.isNull() && dataElement.tagName() == TagDestination)
        {
            const QDomNodeList srcList = dataElement.childNodes();
            for (qint32 j = 0; j < srcList.size(); ++j)
            {
                const QDomElement element = srcList.at(j).toElement();
                if (not element.isNull())
                {
                    DestinationItem d;
                    d.id = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR);
                    d.mx = VAbstractValApplication::VApp()->toPixel(
                        VDomDocument::GetParametrDouble(element, AttrMx, QChar('1')));
                    d.my = VAbstractValApplication::VApp()->toPixel(
                        VDomDocument::GetParametrDouble(element, AttrMy, QChar('1')));
                    d.showLabel = VDomDocument::GetParametrBool(element, AttrShowLabel, trueStr);
                    destination.append(d);
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
    list.insert(ColorDefault, '<' + QCoreApplication::translate("VAbstractOperation", "default") + '>');
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::FullUpdateFromFile()
{
    ReadAttributes();
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
void VAbstractOperation::Disable(bool disable, const QString &namePP)
{
    const bool enabled = !CorrectDisable(disable, namePP);
    setEnabled(enabled);

    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    if (operatedObjects.contains(labelId))
    {
        VAbstractSimple *obj = operatedObjects.value(labelId);
        if (obj && obj->GetType() == GOType::Point)
        {
            auto *item = qobject_cast<VSimplePoint *>(obj);
            SCASSERT(item != nullptr)
            UpdateNamePosition(labelId, pos - item->pos());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractOperation::VAbstractOperation(const VAbstractOperationInitData &initData, QGraphicsItem *parent)
  : VDrawTool(initData.doc, initData.data, initData.id, initData.notes),
    QGraphicsLineItem(parent),
    suffix(initData.suffix),
    source(initData.source),
    destination(initData.destination)
{
    connect(initData.doc, &VAbstractPattern::UpdateToolTip, this,
            [this]()
            {
                QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            auto dItem = std::find_if(destination.begin(), destination.end(),
                                      [id](const DestinationItem &dItem) { return dItem.id == id; });
            if (dItem != destination.end())
            {
                dItem->showLabel = visible;
            }
            VAbstractApplication::VApp()->getUndoStack()->push(new OperationShowLabel(doc, m_id, id, visible));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ApplyToolOptions(const QList<quint32> &oldDependencies, const QList<quint32> &newDependencies,
                                          const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    bool updateToolOptions =
        newDependencies != oldDependencies || not VDomDocument::Compare(newDomElement, oldDomElement);
    bool updateVisibilityOptions = NeedUpdateVisibilityGroup();

    if (updateToolOptions && updateVisibilityOptions)
    {
        VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("operation options"));
    }

    if (updateToolOptions)
    {
        auto *saveOptions =
            new SaveToolOptions(oldDomElement, newDomElement, oldDependencies, newDependencies, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        VAbstractApplication::VApp()->getUndoStack()->push(saveOptions);
    }

    if (updateVisibilityOptions)
    {
        vidtype group = doc->GroupLinkedToTool(m_id);

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
    }

    if (updateToolOptions && updateVisibilityOptions)
    {
        VAbstractApplication::VApp()->getUndoStack()->endMacro();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::PerformDelete()
{
    vidtype group = doc->GroupLinkedToTool(m_id);
    bool deleteGroup = group != null_id;

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
    suffix = doc->GetParametrString(domElement, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrSuffix, suffix);

    SaveSourceDestination(tag);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::UpdateNamePosition(quint32 id, const QPointF &pos)
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            auto dItem = std::find_if(destination.begin(), destination.end(),
                                      [id](const DestinationItem &dItem) { return dItem.id == id; });
            if (dItem != destination.end())
            {
                dItem->mx = pos.x();
                dItem->my = pos.y();
            }
            VAbstractApplication::VApp()->getUndoStack()->push(new OperationMoveLabel(m_id, doc, pos, id));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveSourceDestination(QDomElement &tag)
{
    VAbstractPattern::RemoveAllChildren(tag);

    QDomElement tagObjects = doc->createElement(TagSource);
    for (const auto &sItem : qAsConst(source))
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, sItem.id);
        doc->SetAttributeOrRemoveIf<QString>(item, AttrAlias, sItem.alias,
                                             [](const QString &alias) noexcept { return alias.isEmpty(); });
        doc->SetAttributeOrRemoveIf<QString>(item, AttrPenStyle, sItem.penStyle,
                                             [](const QString &penStyle) noexcept
                                             { return penStyle == TypeLineDefault; });
        doc->SetAttributeOrRemoveIf<QString>(item, AttrColor, sItem.color,
                                             [](const QString &color) noexcept { return color == ColorDefault; });
        tagObjects.appendChild(item);
    }
    tag.appendChild(tagObjects);

    tagObjects = doc->createElement(TagDestination);
    for (auto dItem : qAsConst(destination))
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, dItem.id);

        VAbstractSimple *obj = operatedObjects.value(dItem.id, nullptr);

        doc->SetAttributeOrRemoveIf<double>(item, AttrMx, VAbstractValApplication::VApp()->fromPixel(dItem.mx),
                                            [obj](double value) noexcept
                                            {
                                                Q_UNUSED(value)
                                                return obj && obj->GetType() != GOType::Point;
                                            });

        doc->SetAttributeOrRemoveIf<double>(item, AttrMy, VAbstractValApplication::VApp()->fromPixel(dItem.my),
                                            [obj](double value) noexcept
                                            {
                                                Q_UNUSED(value)
                                                return obj && obj->GetType() != GOType::Point;
                                            });

        doc->SetAttributeOrRemoveIf<bool>(item, AttrShowLabel, dItem.showLabel,
                                          [](bool showLabel) noexcept { return showLabel; });

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
    connect(curve, &VSimpleCurve::ShowContextMenu, this,
            [this](QGraphicsSceneContextMenuEvent *event, quint32 id) { ShowContextMenu(event, id); });
    connect(curve, &VSimpleCurve::Choosed, this, [this, sceneType](quint32 id) { emit ChoosedTool(id, sceneType); });
    connect(curve, &VSimpleCurve::Delete, this, &VAbstractOperation::DeleteFromLabel);
    curve->RefreshGeometry(VAbstractTool::data.GeometricObject<VAbstractCurve>(id));
    operatedObjects.insert(id, curve);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowCurveHover(bool enabled, GOType type)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
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
    vidtype group = doc->GroupLinkedToTool(m_id);

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
    for (auto object : qAsConst(destination))
    {
        const QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(object.id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wswitch-default")
        switch (static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                auto *point = new VSimplePoint(object.id, VColorRole::PatternColor);
                point->setParentItem(this);
                point->SetType(GOType::Point);
                point->setToolTip(ComplexPointToolTip(object.id));
                connect(point, &VSimplePoint::Choosed, this,
                        [this](quint32 id) { emit ChoosedTool(id, SceneObject::Point); });
                connect(point, &VSimplePoint::Selected, this, &VAbstractOperation::ObjectSelected);
                connect(point, &VSimplePoint::ShowContextMenu, this,
                        [this](QGraphicsSceneContextMenuEvent *event, quint32 id) { ShowContextMenu(event, id); });
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
                            u"%6"
                            u"</table>"_s.arg(tr("Label"), curve->ObjectName(), tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curve->GetLength()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), MakeToolTip());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractOperation::VisibilityGroupToolTip() const -> QString
{
    vidtype group = doc->GroupLinkedToTool(m_id);
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
    vidtype groupId = initData.data->getNextId();
    const QDomElement group = initData.doc->CreateGroup(groupId, initData.visibilityGroupName,
                                                        initData.visibilityGroupTags, groupData, initData.id);
    if (not group.isNull())
    {
        auto *addGroup = new AddGroup(group, initData.doc);
        connect(addGroup, &AddGroup::UpdateGroups, initData.doc, &VAbstractPattern::UpdateVisiblityGroups);
        VAbstractApplication::VApp()->getUndoStack()->push(addGroup);
    }
}
