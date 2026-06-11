/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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
#include <QMessageBox>
#include <QScopeGuard>

#include "../ifc/xml/vpatterngraph.h"
#include "../undocommands/deltool.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "toolsdef.h"
#include "vinteractivetool.h"

#include "../vmisc/vabstractvalapplication.h"
#include "../vwidgets/vabstractmainwindow.h"

#include <QLoggingCategory>

//---------------------------------------------------------------------------------------------------------------------
VInteractiveTool::VInteractiveTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent)
  : VAbstractTool(doc, data, id, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief BlockUndoRedoWhileDialogOpen block undo/redo while the tool options dialog (m_dialog) is open.
 *
 * Running undo/redo emits VPattern::UndoCommand, which triggers a full reparse that destroys this tool and
 * the pattern container while the dialog is still alive. Accepting the now-stale dialog afterwards
 * dereferences freed geometry (use-after-free). Call this right after creating and connecting m_dialog,
 * before showing it. The dialog is parented to the main window and outlives the tool, so the state is
 * restored on the dialog's destruction rather than via the tool, which may already be gone.
 */
void VInteractiveTool::BlockUndoRedoWhileDialogOpen()
{
    SCASSERT(not m_dialog.isNull())

    auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
    if (window == nullptr)
    {
        return;
    }

    const quint32 toolId = getId();
    window->SetToolOptionsDialogVisible(true);
    connect(m_dialog.data(), &QObject::destroyed, window,
            [window, toolId]()
            {
                qCDebug(vTool, "Options dialog for tool id=%u closed.", toolId);
                window->SetToolOptionsDialogVisible(false);
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VInteractiveTool::DialogLinkDestroy()
{
    if (!m_dialog.isNull())
    {
        m_dialog->deleteLater();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VInteractiveTool::DeleteToolWithConfirm(bool ask)
{
    qCDebug(vTool, "Deleting abstract tool.");
    if (IsRemovable() == RemoveStatus::Removable)
    {
        qCDebug(vTool, "No dependencies.");
        emit VAbstractValApplication::VApp()->getSceneView()->itemClicked(nullptr);
        if (ask)
        {
            qCDebug(vTool, "Asking.");
            if (ConfirmDeletion() == QMessageBox::No)
            {
                qCDebug(vTool, "User said no.");
                return;
            }
        }

        PerformDelete();

        // Throw exception, this will help prevent case when we forget to immediately quit function.
        throw VExceptionToolWasDeleted(QStringLiteral("Tool was used after deleting."));
    }

    qCDebug(vTool, "Can't delete, tool has dependencies.");
}

//---------------------------------------------------------------------------------------------------------------------
void VInteractiveTool::PerformDelete()
{
    qCDebug(vTool, "Begin deleting.");
    auto *delTool = new DelTool(doc, m_id);
    connect(delTool, &DelTool::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(delTool);
}

//---------------------------------------------------------------------------------------------------------------------
auto VInteractiveTool::IsRemovable() const -> RemoveStatus
{
    if (!doc->IsPatternGraphComplete())
    {
        return RemoveStatus::Pending; // Data not ready yet
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    auto Filter = [](const auto &node) -> auto
    { return node.type != VNodeType::MODELING_TOOL && node.type != VNodeType::MODELING_OBJECT; };

    auto const dependecies = patternGraph->TryGetDependentNodes(m_id, 1000, Filter);

    if (!dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    return dependecies->isEmpty() ? RemoveStatus::Removable : RemoveStatus::Blocked;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromGuiOk refresh tool data after change in options.
 * @param result keep result working dialog.
 */
void VInteractiveTool::FullUpdateFromGuiOk(int result)
{
    if (result == QDialog::Accepted)
    {
        SaveDialogChange();
    }
    DialogLinkDestroy();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromGuiApply refresh tool data after change in options but do not delete dialog
 */
void VInteractiveTool::FullUpdateFromGuiApply()
{
    SaveDialogChange();
}
