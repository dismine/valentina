/************************************************************************
 **
 **  @file   vdrawtool.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VDRAWTOOL_H
#define VDRAWTOOL_H

#include <QAction>
#include <QActionGroup>
#include <QByteArray>
#include <QColor>
#include <QDomElement>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMenu>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/exception/vexceptionbadid.h"
#include "../toolsdef.h"
#include "../vdatatool.h"
#include "../vinteractivetool.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/undocommands/undogroup.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"

struct VDrawToolInitData : VAbstractToolInitData
{
    VDrawToolInitData() = default;

    QString notes{};
};

/**
 * @brief The VDrawTool abstract class for all draw tool.
 */
class VDrawTool : public VInteractiveTool
{
    Q_OBJECT // NOLINT

public:
    VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes, QObject *parent = nullptr);
    ~VDrawTool() override = default;

    auto getLineType() const -> QString;
    virtual void SetLineType(const QString &value);

    virtual auto IsLabelVisible(quint32 id) const -> bool;

    auto GetNotes() const -> QString;
    virtual void SetNotes(const QString &notes);

signals:
    void ChangedToolSelection(bool selected, quint32 object, quint32 tool);

public slots:
    virtual void ShowTool(quint32 id, bool enable);
    virtual void ChangedActivDraw(const QString &newName);
    void ChangedNameDraw(const QString &oldName, const QString &newName);
    virtual void EnableToolMove(bool move);
    virtual void Disable(bool disable, const QString &namePP) = 0;
    virtual void SetDetailsMode(bool mode);
protected slots:
    virtual void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) = 0;

protected:
    enum class RemoveOption : bool
    {
        Disable = false,
        Enable = true
    };
    enum class Referens : bool
    {
        Follow = true,
        Ignore = false
    };

    /** @brief nameActivDraw name of tool's pattern peace. */
    QString nameActivDraw;

    /** @brief typeLine line type. */
    QString m_lineType;

    QString m_notes{};

    void AddToCalculation(const QDomElement &domElement);
    void AddDependence(QList<quint32> &list, quint32 objectId) const;

    /** @brief SaveDialog save options into file after change in dialog. */
    virtual void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                            QList<quint32> &newDependencies) = 0;
    void SaveDialogChange(const QString &undoText = QString()) final;
    virtual void ApplyToolOptions(const QList<quint32> &oldDependencies, const QList<quint32> &newDependencies,
                                  const QDomElement &oldDomElement, const QDomElement &newDomElement);
    void AddToFile() override;
    void SaveOption(QSharedPointer<VGObject> &obj);
    virtual void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj);
    virtual auto MakeToolTip() const -> QString;
    virtual void UpdateNamePosition(quint32 id, const QPointF &pos);

    auto CorrectDisable(bool disable, const QString &namePP) const -> bool;

    void ReadAttributes();
    virtual void ReadToolAttributes(const QDomElement &domElement);
    virtual void ChangeLabelVisibility(quint32 id, bool visible);

    template <class Dialog>
    void ContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 itemId = NULL_ID,
                     const RemoveOption &showRemove = RemoveOption::Enable, const Referens &ref = Referens::Follow);

    template <class Item> void ShowItem(Item *item, quint32 id, bool enable);

    template <class T> auto ObjectName(quint32 id) const -> QString;

    template <class T> auto ObjectAliasSuffix(quint32 id) const -> QString;

    template <class T> static void InitDrawToolConnections(VMainGraphicsScene *scene, T *tool);

private:
    Q_DISABLE_COPY_MOVE(VDrawTool) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template <class Dialog>
/**
 * @brief ContextMenu show context menu for tool.
 * @param event context menu event.
 * @param itemId id of point. 0 if not a point
 * @param showRemove true - tool enable option delete.
 * @param ref true - do not ignore referens value.
 */
void VDrawTool::ContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 itemId, const RemoveOption &showRemove,
                            const Referens &ref)
{
    SCASSERT(event != nullptr)

    if (m_suppressContextMenu)
    {
        return;
    }

    GOType itemType = GOType::Unknown;
    if (itemId != NULL_ID)
    {
        try
        {
            itemType = data.GetGObject(itemId)->getType();
        }
        catch (const VExceptionBadId &e)
        { // Possible case. Parent was deleted, but the node object is still here.
            qWarning() << qUtf8Printable(e.ErrorMessage());
        }
    }

    qCDebug(vTool, "Creating tool context menu.");
    QMenu menu;
    QAction *actionOption = menu.addAction(FromTheme(VThemeIcon::PreferencesOther), VDrawTool::tr("Options"));

    // add the menu "add to group" to the context menu
    QMap<quint32, QString> groupsNotContainingItem = doc->GetGroupsContainingItem(this->getId(), itemId, false);
    auto *actionsAddToGroup = new QActionGroup(this);
    if (not groupsNotContainingItem.empty())
    {
        QMenu *menuAddToGroup = menu.addMenu(FromTheme(VThemeIcon::ListAdd), VDrawTool::tr("Add to group"));

        auto list = QStringList(groupsNotContainingItem.values());
        list.sort(Qt::CaseInsensitive);

        for (int i = 0; i < list.count(); ++i)
        {
            QAction *actionAddToGroup = menuAddToGroup->addAction(list[i]);
            actionsAddToGroup->addAction(actionAddToGroup);
            const quint32 groupId = groupsNotContainingItem.key(list[i]);
            actionAddToGroup->setData(groupId);

            // removes the group we just treated, because we can have several group
            // with the same name. Otherwise the groupId would always be the same
            groupsNotContainingItem.remove(groupId);
        }
    }

    // add the menu "remove from group" to the context menu
    QMap<quint32, QString> groupsContainingItem = doc->GetGroupsContainingItem(this->getId(), itemId, true);
    auto *actionsRemoveFromGroup = new QActionGroup(this);
    if (not groupsContainingItem.empty())
    {
        QMenu *menuRemoveFromGroup =
            menu.addMenu(FromTheme(VThemeIcon::ListRemove), VDrawTool::tr("Remove from group"));

        auto list = QStringList(groupsContainingItem.values());
        list.sort(Qt::CaseInsensitive);

        for (int i = 0; i < list.count(); ++i)
        {
            QAction *actionRemoveFromGroup = menuRemoveFromGroup->addAction(list[i]);
            actionsRemoveFromGroup->addAction(actionRemoveFromGroup);
            const quint32 groupId = groupsContainingItem.key(list[i]);
            actionRemoveFromGroup->setData(groupId);
            groupsContainingItem.remove(groupId);
        }
    }

    QAction *actionShowLabel = menu.addAction(VDrawTool::tr("Show label"));
    actionShowLabel->setCheckable(true);

    if (itemType == GOType::Point)
    {
        actionShowLabel->setChecked(IsLabelVisible(itemId));
    }
    else
    {
        actionShowLabel->setVisible(false);
    }

    QAction *actionRestoreLabelPosition = menu.addAction(VDrawTool::tr("Restore label position"));
    actionRestoreLabelPosition->setVisible(itemType == GOType::Point);

    QAction *actionRemove = menu.addAction(FromTheme(VThemeIcon::EditDelete), VDrawTool::tr("Delete"));
    if (showRemove == RemoveOption::Enable)
    {
        if (ref == Referens::Follow)
        {
            if (_referens > 0)
            {
                qCDebug(vTool, "Remove disabled. Tool has childern.");
                actionRemove->setEnabled(false);
            }
            else
            {
                qCDebug(vTool, "Remove enabled. Tool has not childern.");
                actionRemove->setEnabled(true);
            }
        }
        else
        {
            qCDebug(vTool, "Remove enabled. Ignore referens value.");
            actionRemove->setEnabled(true);
        }
    }
    else
    {
        qCDebug(vTool, "Remove disabled.");
        actionRemove->setEnabled(false);
    }

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == nullptr)
    {
        return;
    }

    if (selectedAction == actionOption)
    {
        qCDebug(vTool, "Show options.");
        emit VAbstractValApplication::VApp()->getSceneView()->itemClicked(nullptr);
        m_dialog = QPointer<Dialog>(new Dialog(getData(), doc, m_id, VAbstractValApplication::VApp()->getMainWindow()));
        m_dialog->setModal(true);

        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &VDrawTool::FullUpdateFromGuiOk);
        connect(m_dialog.data(), &DialogTool::DialogApplied, this, &VDrawTool::FullUpdateFromGuiApply);

        this->SetDialog();

        m_dialog->show();
    }
    else if (selectedAction == actionRemove)
    {
        qCDebug(vTool, "Deleting tool.");
        DeleteToolWithConfirm(); // do not catch exception here
        return;                  // Leave this method immediately after call!!!
    }
    else if (selectedAction == actionShowLabel)
    {
        ChangeLabelVisibility(itemId, selectedAction->isChecked());
    }
    else if (selectedAction == actionRestoreLabelPosition)
    {
        UpdateNamePosition(itemId, QPointF(labelMX, labelMY));
    }
    else if (selectedAction->actionGroup() == actionsAddToGroup)
    {
        quint32 const groupId = selectedAction->data().toUInt();
        QDomElement const item = doc->AddItemToGroup(this->getId(), itemId, groupId);

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
        SCASSERT(window != nullptr)
        {
            auto *addItemToGroup = new AddItemToGroup(item, doc, groupId);
            connect(addItemToGroup, &AddItemToGroup::UpdateGroups, window,
                    &VAbstractMainWindow::UpdateVisibilityGroups);
            VAbstractApplication::VApp()->getUndoStack()->push(addItemToGroup);
        }
    }
    else if (selectedAction->actionGroup() == actionsRemoveFromGroup)
    {
        quint32 const groupId = selectedAction->data().toUInt();
        QDomElement const item = doc->RemoveItemFromGroup(this->getId(), itemId, groupId);

        auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
        SCASSERT(window != nullptr)
        {
            auto *removeItemFromGroup = new RemoveItemFromGroup(item, doc, groupId);
            connect(removeItemFromGroup, &RemoveItemFromGroup::UpdateGroups, window,
                    &VAbstractMainWindow::UpdateVisibilityGroups);
            VAbstractApplication::VApp()->getUndoStack()->push(removeItemFromGroup);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
/**
 * @brief ShowItem highlight tool.
 * @param item tool.
 * @param id object id in container.
 * @param enable enable or disable highlight.
 */
void VDrawTool::ShowItem(Item *item, quint32 id, bool enable)
{
    SCASSERT(item != nullptr)
    if (id == item->m_id)
    {
        ShowVisualization(enable);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
/**
 * @brief ObjectName get object (point, curve, arc) name.
 * @param id object id in container.
 */
auto VDrawTool::ObjectName(quint32 id) const -> QString
{
    try
    {
        return data.GeometricObject<T>(id)->ObjectName();
    }
    catch (const VExceptionBadId &e)
    {
        qCDebug(vTool, "Error! Couldn't get object name by id = %s. %s %s", qUtf8Printable(QString().setNum(id)),
                qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return QString(); // Return empty string for property browser
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
/**
 * @brief ObjectAlias get object (point, curve, arc) alias.
 * @param id object id in container.
 */
auto VDrawTool::ObjectAliasSuffix(quint32 id) const -> QString
{
    try
    {
        return data.GeometricObject<T>(id)->GetAliasSuffix();
    }
    catch (const VExceptionBadId &e)
    {
        qCDebug(vTool, "Error! Couldn't get object alias suffix by id = %s. %s %s",
                qUtf8Printable(QString().setNum(id)), qUtf8Printable(e.ErrorMessage()),
                qUtf8Printable(e.DetailedInformation()));
        return QString(); // Return empty string for property browser
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void VDrawTool::InitDrawToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    QObject::connect(tool, &T::ChoosedTool, scene, &VMainGraphicsScene::ChoosedItem);
    QObject::connect(tool, &T::ChangedToolSelection, scene, &VMainGraphicsScene::SelectedItem);
    QObject::connect(scene, &VMainGraphicsScene::DisableItem, tool, &T::Disable);
    QObject::connect(scene, &VMainGraphicsScene::EnableToolMove, tool, &T::EnableToolMove);
    QObject::connect(scene, &VMainGraphicsScene::CurveDetailsMode, tool, &T::SetDetailsMode);
    QObject::connect(scene, &VMainGraphicsScene::ItemSelection, tool, &T::ToolSelectionType);
}

#endif // VDRAWTOOL_H
