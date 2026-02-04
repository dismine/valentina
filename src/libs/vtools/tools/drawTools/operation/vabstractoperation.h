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

#ifndef VABSTRACTOPERATION_H
#define VABSTRACTOPERATION_H

#include <QGraphicsLineItem>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../../toolsdef.h"
#include "../vdrawtool.h"
#include "../vwidgets/vsimplecurve.h"

struct DestinationItem
{
    quint32 id{NULL_ID};
    qreal mx{1};
    qreal my{1};
    bool showLabel{true};
};

struct VAbstractOperationInitData : VDrawToolInitData
{
    using VDrawToolInitData::VDrawToolInitData;

    QString suffix{};
    QVector<SourceItem> source{};
    QVector<DestinationItem> destination{};
    QString visibilityGroupName{};
    QStringList visibilityGroupTags{};
    bool hasLinkedVisibilityGroup{false};
};

// FIXME. I don't know how to use QGraphicsItem properly, so just took first available finished class.
// QGraphicsItem itself produce case where clicking on empty space produce call to QGraphicsItem.
// And i don't know how to fix it.
class VAbstractOperation : public VDrawTool, public QGraphicsLineItem
{
    Q_OBJECT // NOLINT
    // Fix warning "Class implements the interface QGraphicsItem but does not list it
    // in Q_INTERFACES. qobject_cast to QGraphicsItem will not work!"
    Q_INTERFACES(QGraphicsItem)

public:
    ~VAbstractOperation() override = default;

    static const QString TagItem;
    static const QString TagSource;
    static const QString TagDestination;

    auto getTagName() const -> QString override;

    auto IsRemovable() const -> RemoveStatus override;

    void SetNotes(const QString &notes) override;

    auto SourceItems() const -> QVector<SourceItem>;

    void GroupVisibility(quint32 object, bool visible) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void ChangeLabelPosition(quint32 id, const QPointF &pos) override;

    auto IsLabelVisible(quint32 id) const -> bool override;
    void SetLabelVisible(quint32 id, bool visible) override;

    static void ExtractData(const QDomElement &domElement, VAbstractOperationInitData &initData);
    static auto ExtractSourceData(const QDomElement &domElement) -> QVector<SourceItem>;
    static auto ExtractDestinationData(const QDomElement &domElement) -> QVector<DestinationItem>;

    static auto OperationColorsList() -> QMap<QString, QString>;

public slots:
    void FullUpdateFromFile() override;

    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;
    void EnableToolMove(bool move) override;

    void AllowPointHover(bool enabled);
    void AllowPointSelecting(bool enabled);

    void AllowPointLabelHover(bool enabled);
    void AllowPointLabelSelecting(bool enabled);

    void AllowSplineHover(bool enabled);
    void AllowSplineSelecting(bool enabled);

    void AllowSplinePathHover(bool enabled);
    void AllowSplinePathSelecting(bool enabled);

    void AllowArcHover(bool enabled);
    void AllowArcSelecting(bool enabled);

    void AllowElArcHover(bool enabled);
    void AllowElArcSelecting(bool enabled);

    void ToolSelectionType(const SelectionType &type) override;
    void Enable() override;
    void ObjectSelected(bool selected, quint32 objId);
    void DeleteFromLabel();
    void LabelChangePosition(const QPointF &pos, quint32 labelId);

protected:
    QVector<SourceItem> source;
    QVector<DestinationItem> destination;

    QMap<quint32, VAbstractSimple *> operatedObjects{};

    bool hasLinkedGroup{false};
    QString groupName{};
    QStringList groupTags{};

    explicit VAbstractOperation(const VAbstractOperationInitData &initData, QGraphicsItem *parent = nullptr);

    void AddToFile() override;
    void ChangeLabelVisibility(quint32 id, bool visible) override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;
    void PerformDelete() override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;

    void UpdateNamePosition(quint32 id, const QPointF &pos) override;
    void SaveSourceDestination(QDomElement &tag);

    template <typename T> void ShowToolVisualization(bool show);

    template <typename T> void SetDialogVisibilityGroupData(QPointer<T> dialogTool);

    template <typename T> void SaveVisibilityGroupData(QPointer<T> dialogTool);

    void InitCurve(quint32 id, VContainer *data, GOType curveType, SceneObject sceneType);

    template <typename T> static void InitOperationToolConnections(VMainGraphicsScene *scene, T *tool);

    void InitOperatedObjects();

    auto ComplexPointToolTip(quint32 itemId) const -> QString;
    auto ComplexCurveToolTip(quint32 itemId) const -> QString;
    auto VisibilityGroupToolTip() const -> QString;

    static void CreateVisibilityGroup(const VAbstractOperationInitData &initData);

    static void PrepareNames(VAbstractOperationInitData &initData);

    void ProcessOperationToolOptions(const QDomElement &oldDomElement,
                                     const QDomElement &newDomElement,
                                     const QVector<SourceItem> &newSource);

private:
    Q_DISABLE_COPY_MOVE(VAbstractOperation) // NOLINT

    void AllowCurveHover(bool enabled, GOType type);
    void AllowCurveSelecting(bool enabled, GOType type);

    auto NeedUpdateVisibilityGroup() const -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VAbstractOperation::SetDialogVisibilityGroupData(QPointer<T> dialogTool)
{
    SCASSERT(not dialogTool.isNull())

    vidtype const group = doc->GroupLinkedToTool(m_id);
    dialogTool->SetGroupCategories(doc->GetGroupCategories());
    if (group != null_id)
    {
        dialogTool->SetHasLinkedVisibilityGroup(true);
        dialogTool->SetVisibilityGroupName(doc->GetGroupName(group));
        dialogTool->SetVisibilityGroupTags(doc->GetGroupTags(group));
    }
    else
    {
        dialogTool->SetHasLinkedVisibilityGroup(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VAbstractOperation::SaveVisibilityGroupData(QPointer<T> dialogTool)
{
    SCASSERT(not dialogTool.isNull())

    // Save for later use.
    hasLinkedGroup = dialogTool->HasLinkedVisibilityGroup();
    groupName = dialogTool->GetVisibilityGroupName();
    groupTags = dialogTool->GetVisibilityGroupTags();
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VAbstractOperation::ShowToolVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<T>();
            SetVisualization();
        }
        else
        {
            if (T *visual = qobject_cast<T *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis.data();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VAbstractOperation::InitOperationToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);

    QObject::connect(scene, &VMainGraphicsScene::EnablePointItemHover, tool, &T::AllowPointHover);
    QObject::connect(scene, &VMainGraphicsScene::EnablePointItemSelection, tool, &T::AllowPointSelecting);
    QObject::connect(scene, &VMainGraphicsScene::EnableLabelItemHover, tool, &T::AllowPointLabelHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableLabelItemSelection, tool, &T::AllowPointLabelSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemHover, tool, &T::AllowSplineHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemSelection, tool, &T::AllowSplineSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemHover, tool, &T::AllowSplinePathHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemSelection, tool, &T::AllowSplinePathSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemHover, tool, &T::AllowArcHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemSelection, tool, &T::AllowArcSelecting);

    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemHover, tool, &T::AllowElArcHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemSelection, tool, &T::AllowElArcSelecting);
}

#endif // VABSTRACTOPERATION_H
