/************************************************************************
 **
 **  @file   vdrawtool.cpp
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

#include "vdrawtool.h"

#include <QDialog>
#include <QDomNode>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QUndoStack>
#include <QtDebug>

#include "../../undocommands/addtocalc.h"
#include "../../undocommands/savetooloptions.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../vabstracttool.h"
#include "../vpatterndb/vcontainer.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VDrawTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 */
VDrawTool::VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QString notes, QObject *parent)
  : VInteractiveTool(doc, data, id, parent),
    m_indexActivePatternBlock(doc->PatternBlockMapper()->GetActiveId()),
    m_lineType(TypeLineLine),
    m_notes(std::move(notes))
{
    connect(this->doc->PatternBlockMapper(),
            QOverload<int>::of(&VPatternBlockMapper::ChangedActivePatternBlock),
            this,
            &VDrawTool::Enable);
    connect(this->doc, &VAbstractPattern::ShowTool, this, &VDrawTool::ShowTool);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.
 * @param id object id in container.
 * @param enable enable or disable highlight.
 */
void VDrawTool::ShowTool(quint32 id, bool enable)
{
    Q_UNUSED(id)
    Q_UNUSED(enable)
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveDialogChange(const QString &undoText)
{
    Q_UNUSED(undoText)
    qCDebug(vTool, "Saving tool options after using dialog");
    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();
        QList<quint32> oldDependencies;
        QList<quint32> newDependencies;
        SaveDialog(newDomElement, oldDependencies, newDependencies);
        ApplyToolOptions(oldDependencies, newDependencies, oldDomElement, newDomElement);
    }
    else
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ApplyToolOptions(const QList<quint32> &oldDependencies, const QList<quint32> &newDependencies,
                                 const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    if (newDependencies != oldDependencies || not VDomDocument::Compare(newDomElement, oldDomElement))
    {
        auto *saveOptions =
            new SaveToolOptions(oldDomElement, newDomElement, oldDependencies, newDependencies, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        VAbstractApplication::VApp()->getUndoStack()->push(saveOptions);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VDrawTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOption(QSharedPointer<VGObject> &obj)
{
    qCDebug(vTool, "Saving tool options");
    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();

        SaveOptions(newDomElement, obj);

        auto *saveOptions =
            new SaveToolOptions(oldDomElement, newDomElement, QList<quint32>(), QList<quint32>(), doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        VAbstractApplication::VApp()->getUndoStack()->push(saveOptions);
    }
    else
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    Q_UNUSED(obj)

    doc->SetAttribute(tag, VDomDocument::AttrId, m_id);
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrNotes, m_notes,
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::MakeToolTip() const -> QString
{
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::UpdateNamePosition(quint32 id, const QPointF &pos)
{
    Q_UNUSED(id)
    Q_UNUSED(pos)
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadAttributes()
{
    const QDomElement domElement = doc->FindElementById(m_id, getTagName());
    if (domElement.isElement())
    {
        ReadToolAttributes(domElement);
    }
    else
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadToolAttributes(const QDomElement &domElement)
{
    m_notes = VDomDocument::GetParametrEmptyString(domElement, AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::EnableToolMove(bool move)
{
    Q_UNUSED(move)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SetDetailsMode(bool mode)
{
    Q_UNUSED(mode)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ChangeLabelVisibility(quint32 id, bool visible)
{
    Q_UNUSED(id)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves the geometric object type for the given item ID.
 * 
 * @param itemId The ID of the item to query
 * @return The GOType of the item, or GOType::Unknown if the item doesn't exist or was deleted
 */
auto VDrawTool::GetItemType(quint32 itemId) const -> GOType
{
    if (itemId == NULL_ID)
    {
        return GOType::Unknown;
    }

    try
    {
        return data.GetGObject(itemId)->getType();
    }
    catch (const VExceptionBadId &e)
    {
        // Possible case: Parent was deleted, but the node object still exists
        qWarning() << qUtf8Printable(e.ErrorMessage());
        return GOType::Unknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Creates the "Add to group" submenu with available groups.
 * 
 * @param menu The parent menu to add the submenu to
 * @param itemId The ID of the item that will be added to a group
 * @return Action group containing all "add to group" actions, or nullptr if no groups available
 */
auto VDrawTool::CreateAddToGroupMenu(QMenu &menu, quint32 itemId) -> QActionGroup *
{
    QMap<quint32, QString> groupsNotContainingItem = doc->GetGroupsContainingItem(this->getId(), itemId, false);

    auto *actionsAddToGroup = new QActionGroup(this);

    if (groupsNotContainingItem.empty())
    {
        return actionsAddToGroup;
    }

    QMenu *menuAddToGroup = menu.addMenu(FromTheme(VThemeIcon::ListAdd), tr("Add to group"));

    auto list = QStringList(groupsNotContainingItem.values());
    list.sort(Qt::CaseInsensitive);

    for (const auto &name : list)
    {
        QAction *actionAddToGroup = menuAddToGroup->addAction(name);
        actionsAddToGroup->addAction(actionAddToGroup);
        const quint32 groupId = groupsNotContainingItem.key(name);
        actionAddToGroup->setData(groupId);

        // Remove the processed group to handle duplicate names correctly
        groupsNotContainingItem.remove(groupId);
    }

    return actionsAddToGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::CreateRemoveFromGroupMenu(QMenu &menu, quint32 itemId) -> QActionGroup *
{
    QMap<quint32, QString> groupsContainingItem = doc->GetGroupsContainingItem(this->getId(), itemId, true);

    auto *actionsRemoveFromGroup = new QActionGroup(this);

    if (groupsContainingItem.empty())
    {
        return actionsRemoveFromGroup;
    }

    QMenu *menuRemoveFromGroup = menu.addMenu(FromTheme(VThemeIcon::ListRemove), tr("Remove from group"));

    auto list = QStringList(groupsContainingItem.values());
    list.sort(Qt::CaseInsensitive);

    for (const auto &name : list)
    {
        QAction *actionRemoveFromGroup = menuRemoveFromGroup->addAction(name);
        actionsRemoveFromGroup->addAction(actionRemoveFromGroup);
        const quint32 groupId = groupsContainingItem.key(name);
        actionRemoveFromGroup->setData(groupId);

        // Remove the processed group to handle duplicate names correctly
        groupsContainingItem.remove(groupId);
    }

    return actionsRemoveFromGroup;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Creates the "Show label" toggle action for point items.
 * 
 * @param menu The parent menu to add the action to
 * @param itemType The type of the geometric object
 * @param itemId The ID of the item
 * @return Checkable action for showing/hiding the label, or nullptr if not applicable
 */
auto VDrawTool::CreateShowLabelAction(QMenu &menu, GOType itemType, quint32 itemId) -> QAction *
{
    QAction *actionShowLabel = menu.addAction(tr("Show label"));
    actionShowLabel->setCheckable(true);

    if (itemType == GOType::Point)
    {
        actionShowLabel->setChecked(IsLabelVisible(itemId));
    }
    else
    {
        actionShowLabel->setVisible(false);
    }

    return actionShowLabel;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Creates the "Restore label position" action for point items.
 * 
 * @param menu The parent menu to add the action to
 * @param itemType The type of the geometric object
 * @return Action for restoring label position, visible only for points
 */
auto VDrawTool::CreateRestoreLabelAction(QMenu &menu, GOType itemType) -> QAction *
{
    QAction *actionRestoreLabelPosition = menu.addAction(VDrawTool::tr("Restore label position"));
    actionRestoreLabelPosition->setVisible(itemType == GOType::Point);
    return actionRestoreLabelPosition;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Creates the "Delete" action with appropriate status indication.
 * 
 * The action is enabled if the item is removable or blocked, and shows
 * a "Pending" indicator if deletion status is pending.
 * 
 * @param menu The parent menu to add the action to
 * @return Delete action with enabled state based on removal status
 */
auto VDrawTool::CreateRemoveAction(QMenu &menu) -> QAction *
{
    using namespace Qt::Literals::StringLiterals;

    QAction *actionRemove = menu.addAction(FromTheme(VThemeIcon::EditDelete), VDrawTool::tr("Delete"));
    RemoveStatus const status = IsRemovable();
    actionRemove->setEnabled(status == RemoveStatus::Removable || status == RemoveStatus::Blocked);

    if (status == RemoveStatus::Pending)
    {
        actionRemove->setText(actionRemove->text() + " ("_L1 + tr("Pending") + ')');
    }

    return actionRemove;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the remove/delete action, either deleting the tool or showing an error.
 * 
 * If the tool is removable, it proceeds with deletion. If the tool is blocked
 * due to dependencies, it displays a warning message explaining why deletion
 * cannot proceed.
 */
void VDrawTool::HandleRemoveAction()
{
    if (IsRemovable() == RemoveStatus::Removable)
    {
        qCDebug(vTool, "Deleting tool.");
        DeleteToolWithConfirm(); // do not catch exception here
        return;                  // Leave this method immediately after call!!!
    }

    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setWindowTitle(tr("Cannot Delete Object"));
    messageBox.setText(tr("This object cannot be deleted because it is being used by other items."));
    messageBox.setInformativeText(tr("Please resolve the dependencies before attempting to delete this object."));
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Adds the item to the selected group and updates the UI.
 * 
 * @param selectedAction The action containing the group ID as data
 * @param itemId The ID of the item to add to the group
 */
void VDrawTool::HandleAddToGroup(const QAction *selectedAction, quint32 itemId)
{
    quint32 const groupId = selectedAction->data().toUInt();
    QDomElement const item = doc->AddItemToGroup(this->getId(), itemId, groupId);

    auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
    SCASSERT(scene != nullptr)
    scene->clearSelection();

    auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
    SCASSERT(window != nullptr)

    auto *addItemToGroup = new AddItemToGroup(item, doc, groupId);
    connect(addItemToGroup, &AddItemToGroup::UpdateGroups, window, &VAbstractMainWindow::UpdateVisibilityGroups);
    VAbstractApplication::VApp()->getUndoStack()->push(addItemToGroup);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Removes the item from the selected group and updates the UI.
 * 
 * @param selectedAction The action containing the group ID as data
 * @param itemId The ID of the item to remove from the group
 */
void VDrawTool::HandleRemoveFromGroup(const QAction *selectedAction, quint32 itemId)
{
    quint32 const groupId = selectedAction->data().toUInt();
    QDomElement const item = doc->RemoveItemFromGroup(this->getId(), itemId, groupId);

    auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
    SCASSERT(window != nullptr)

    auto *removeItemFromGroup = new RemoveItemFromGroup(item, doc, groupId);
    connect(removeItemFromGroup,
            &RemoveItemFromGroup::UpdateGroups,
            window,
            &VAbstractMainWindow::UpdateVisibilityGroups);
    VAbstractApplication::VApp()->getUndoStack()->push(removeItemFromGroup);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToCalculation add tool to calculation tag in pattern file.
 * @param domElement tag in xml tree.
 */
void VDrawTool::AddToCalculation(const QDomElement &domElement)
{
    const QDomElement duplicate = doc->FindElementById(m_id);
    if (not duplicate.isNull())
    {
        throw VExceptionWrongId(tr("This id (%1) is not unique.").arg(m_id), duplicate);
    }

    auto *addToCal = new AddToCalc(domElement, doc);
    connect(addToCal, &AddToCalc::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(addToCal);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::AddDependence(QList<quint32> &list, quint32 objectId) const
{
    if (objectId != NULL_ID)
    {
        auto originPoint = VAbstractTool::data.GetGObject(objectId);
        list.append(originPoint->getIdTool());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::getLineType() const -> QString
{
    return m_lineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SetLineType(const QString &value)
{
    m_lineType = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::IsLabelVisible(quint32 id) const -> bool
{
    Q_UNUSED(id)
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::GetNotes() const -> QString
{
    return m_notes;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SetNotes(const QString &notes)
{
    m_notes = notes;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}
