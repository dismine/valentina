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
#include <QMessageBox>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/exception/vexceptionbadid.h"
#include "../toolsdef.h"
#include "../vdatatool.h"
#include "../vinteractivetool.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"

struct VDrawToolInitData : VAbstractToolInitData
{
    QString notes{};
};

/**
 * @brief The VDrawTool abstract class for all draw tool.
 */
class VDrawTool : public VInteractiveTool
{
    Q_OBJECT // NOLINT

public:
    VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QString notes, QObject *parent = nullptr);
    ~VDrawTool() override = default;

    auto getLineType() const -> QString;
    virtual void SetLineType(const QString &value);

    virtual auto IsLabelVisible(quint32 id) const -> bool;

    auto GetNotes() const -> QString;
    virtual void SetNotes(const QString &notes);

signals:
    void ChangedToolSelection(bool selected, quint32 object, quint32 tool);

public slots:
    virtual void Enable() = 0;
    virtual void EnableToolMove(bool move);
    virtual void SetDetailsMode(bool mode);

protected slots:
    virtual void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) = 0;

protected:
    /** @brief m_indexActivePatternBlock index of active patetrn block */
    int m_indexActivePatternBlock;

    /** @brief typeLine line type. */
    QString m_lineType;

    QString m_notes{};

    void AddToCalculation(const QDomElement &domElement);

    /** @brief SaveDialog save options into file after change in dialog. */
    virtual void SaveDialog(QDomElement &domElement) = 0;
    void SaveDialogChange(const QString &undoText = QString()) final;
    virtual void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement);
    void AddToFile() override;
    void SaveOption(QSharedPointer<VGObject> &obj);
    virtual void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj);
    virtual auto MakeToolTip() const -> QString;
    virtual void UpdateNamePosition(quint32 id, const QPointF &pos);

    void ReadAttributes();
    virtual void ReadToolAttributes(const QDomElement &domElement);
    virtual void ChangeLabelVisibility(quint32 id, bool visible);

    template<class Dialog>
    void ContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 itemId = NULL_ID);

    template <class T> auto ObjectName(quint32 id) const -> QString;

    template <class T> auto ObjectAliasSuffix(quint32 id) const -> QString;

    template <class T> static void InitDrawToolConnections(VMainGraphicsScene *scene, T *tool);

private:
    Q_DISABLE_COPY_MOVE(VDrawTool) // NOLINT

    auto GetItemType(quint32 itemId) const -> GOType;

    auto CreateAddToGroupMenu(QMenu &menu, quint32 itemId) -> QActionGroup *;
    auto CreateRemoveFromGroupMenu(QMenu &menu, quint32 itemId) -> QActionGroup *;
    auto CreateShowLabelAction(QMenu &menu, GOType itemType, quint32 itemId) -> QAction *;
    auto CreateRestoreLabelAction(QMenu &menu, GOType itemType) -> QAction *;
    auto CreateRemoveAction(QMenu &menu) -> QAction *;

    template<class Dialog>
    void HandleMenuSelection(QAction const *selectedAction,
                             QAction const *actionOption,
                             QAction *actionRemove,
                             QAction *actionShowLabel,
                             QAction *actionRestoreLabelPosition,
                             QActionGroup *actionsAddToGroup,
                             QActionGroup *actionsRemoveFromGroup,
                             quint32 itemId);

    template<class Dialog>
    void ShowOptionsDialog();

    void HandleRemoveAction();
    void HandleAddToGroup(QAction const *selectedAction, quint32 itemId);
    void HandleRemoveFromGroup(QAction const *selectedAction, quint32 itemId);
};

template<class Dialog>
/**
 * @brief Displays a context menu for the drawing tool with options to edit, group, 
 *        show/hide labels, and delete the tool or its items.
 * 
 * The menu provides the following functionality:
 * - Options dialog for tool configuration
 * - Add/remove items to/from groups
 * - Show/hide point labels (points only)
 * - Restore label positions (points only)
 * - Delete tool or item
 * 
 * @param event The context menu event containing position and triggering information
 * @param itemId The ID of the specific item (e.g., point) being clicked, or 0 for the tool itself
 */
void VDrawTool::ContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 itemId)
{
    SCASSERT(event != nullptr)

    if (m_suppressContextMenu)
    {
        return;
    }

    GOType const itemType = GetItemType(itemId);

    qCDebug(vTool, "Creating tool context menu.");
    QMenu menu;

    // Build menu structure
    QAction const *actionOption = menu.addAction(FromTheme(VThemeIcon::PreferencesOther), VDrawTool::tr("Options"));

    // add the menu "add to group" to the context menu
    auto *actionsAddToGroup = CreateAddToGroupMenu(menu, itemId);

    // add the menu "remove from group" to the context menu
    auto *actionsRemoveFromGroup = CreateRemoveFromGroupMenu(menu, itemId);

    QAction *actionShowLabel = CreateShowLabelAction(menu, itemType, itemId);
    QAction *actionRestoreLabelPosition = CreateRestoreLabelAction(menu, itemType);
    QAction *actionRemove = CreateRemoveAction(menu);

    // Execute menu and handle selection
    QAction const *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == nullptr)
    {
        return;
    }

    HandleMenuSelection<Dialog>(selectedAction,
                                actionOption,
                                actionRemove,
                                actionShowLabel,
                                actionRestoreLabelPosition,
                                actionsAddToGroup,
                                actionsRemoveFromGroup,
                                itemId);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Dialog>
/**
 * @brief Handles the user's menu selection and executes the corresponding action.
 * 
 * @param selectedAction The action selected by the user
 * @param actionOption The options dialog action
 * @param actionRemove The delete action
 * @param actionShowLabel The show/hide label action
 * @param actionRestoreLabelPosition The restore label position action
 * @param actionsAddToGroup Action group for adding to groups
 * @param actionsRemoveFromGroup Action group for removing from groups
 * @param itemId The ID of the item being operated on
 */
void VDrawTool::HandleMenuSelection(const QAction *selectedAction,
                                    const QAction *actionOption,
                                    QAction *actionRemove,
                                    QAction *actionShowLabel,
                                    QAction *actionRestoreLabelPosition,
                                    QActionGroup *actionsAddToGroup,
                                    QActionGroup *actionsRemoveFromGroup,
                                    quint32 itemId)
{
    if (selectedAction == actionOption)
    {
        ShowOptionsDialog<Dialog>();
    }
    else if (selectedAction == actionRemove)
    {
        HandleRemoveAction();
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
        HandleAddToGroup(selectedAction, itemId);
    }
    else if (selectedAction->actionGroup() == actionsRemoveFromGroup)
    {
        HandleRemoveFromGroup(selectedAction, itemId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Dialog>
/**
 * @brief Shows the options dialog for configuring the tool.
 * 
 * Creates and displays a modal dialog for tool configuration. The dialog
 * is connected to update handlers that apply changes when the user confirms
 * or applies the settings.
 */
void VDrawTool::ShowOptionsDialog()
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
        return {}; // Return empty string for property browser
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
        return {}; // Return empty string for property browser
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void VDrawTool::InitDrawToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    QObject::connect(tool, &T::ChoosedTool, scene, &VMainGraphicsScene::ChoosedItem);
    QObject::connect(tool, &T::ChangedToolSelection, scene, &VMainGraphicsScene::SelectedItem);
    QObject::connect(scene, &VMainGraphicsScene::EnableItem, tool, &T::Enable);
    QObject::connect(scene, &VMainGraphicsScene::EnableToolMove, tool, &T::EnableToolMove);
    QObject::connect(scene, &VMainGraphicsScene::CurveDetailsMode, tool, &T::SetDetailsMode);
    QObject::connect(scene, &VMainGraphicsScene::ItemSelection, tool, &T::ToolSelectionType);
}

#endif // VDRAWTOOL_H
