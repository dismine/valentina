/************************************************************************
 **
 **  @file   toggledetailinlayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2016
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

#include "togglepiecestate.h"

#include <QDomElement>
#include <QHash>
#include <QMessageLogger>
#include <QtDebug>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vtools/tools/vtoolseamallowance.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
TogglePieceInLayout::TogglePieceInLayout(quint32 id, bool state, VContainer *data, VAbstractPattern *doc,
                                         QUndoCommand *parent)
  : VUndoCommand(QDomElement(), doc, parent),
    m_id(id),
    m_data(data),
    m_oldState(not state),
    m_newState(state)
{
    setText(tr("detail in layout list"));
    m_oldState = m_data->DataPieces()->value(m_id).IsInLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceInLayout::undo()
{
    qCDebug(vUndo, "TogglePieceState::undo().");
    Do(m_oldState);
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceInLayout::redo()
{
    qCDebug(vUndo, "TogglePieceState::redo().");
    Do(m_newState);
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceInLayout::Do(bool state)
{
    QDomElement detail = doc->FindElementById(m_id, VAbstractPattern::TagDetail);
    if (detail.isElement())
    {
        doc->SetAttributeOrRemoveIf<bool>(detail, AttrInLayout, state, [](bool state) noexcept { return state; });

        VPiece det = m_data->DataPieces()->value(m_id);
        det.SetInLayout(state);
        m_data->UpdatePiece(m_id, det);
        emit Toggled(m_id);
    }
    else
    {
        qDebug("Can't get detail by id = %u.", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
TogglePieceForceForbidFlipping::TogglePieceForceForbidFlipping(quint32 id, bool state, ForceForbidFlippingType type,
                                                               VContainer *data, VAbstractPattern *doc,
                                                               QUndoCommand *parent)
  : VUndoCommand(QDomElement(), doc, parent),
    m_id(id),
    m_data(data),
    m_type(type)
{
    setText(tr("piece flipping"));

    VPiece det = m_data->DataPieces()->value(m_id);

    m_oldForceState = det.IsForceFlipping();
    m_oldForbidState = det.IsForbidFlipping();

    if (m_type == ForceForbidFlippingType::ForceFlipping)
    {
        m_newForceState = state;
        det.SetForbidFlipping(state);
        m_newForbidState = det.IsForbidFlipping();
    }
    else
    {
        m_newForbidState = state;
        det.SetForceFlipping(state);
        m_newForceState = det.IsForceFlipping();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceForceForbidFlipping::undo()
{
    QDomElement detail = doc->FindElementById(m_id, VAbstractPattern::TagDetail);
    if (detail.isElement())
    {
        VPiece det = m_data->DataPieces()->value(m_id);
        if (m_type == ForceForbidFlippingType::ForceFlipping)
        {
            det.SetForceFlipping(m_oldForceState);
            det.SetForbidFlipping(m_oldForbidState);
        }
        else
        {
            det.SetForbidFlipping(m_oldForbidState);
            det.SetForceFlipping(m_oldForceState);
        }
        m_data->UpdatePiece(m_id, det);

        // Probably overkill, but will help to avoid mistakes
        VToolSeamAllowance::AddAttributes(doc, detail, m_id, det);
    }
    else
    {
        qDebug("Can't get detail by id = %u.", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceForceForbidFlipping::redo()
{
    QDomElement detail = doc->FindElementById(m_id, VAbstractPattern::TagDetail);
    if (detail.isElement())
    {
        VPiece det = m_data->DataPieces()->value(m_id);
        if (m_type == ForceForbidFlippingType::ForceFlipping)
        {
            det.SetForceFlipping(m_newForceState);
        }
        else
        {
            det.SetForbidFlipping(m_newForbidState);
        }
        m_data->UpdatePiece(m_id, det);

        // Probably overkill, but will help to avoid mistakes
        VToolSeamAllowance::AddAttributes(doc, detail, m_id, det);
    }
    else
    {
        qDebug("Can't get detail by id = %u.", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
ToggleHideMainPath::ToggleHideMainPath(quint32 id, bool state, VContainer *data, VAbstractPattern *doc,
                                       QUndoCommand *parent)
  : VUndoCommand(QDomElement(), doc, parent),
    m_id(id),
    m_data(data),
    m_oldState(not state),
    m_newState(state)
{
    setText(tr("detail hide main path"));
    m_oldState = m_data->DataPieces()->value(m_id).IsHideMainPath();
}

//---------------------------------------------------------------------------------------------------------------------
void ToggleHideMainPath::undo()
{
    qCDebug(vUndo, "ToggleHideMainPath::undo().");
    Do(m_oldState);
}

//---------------------------------------------------------------------------------------------------------------------
void ToggleHideMainPath::redo()
{
    qCDebug(vUndo, "ToggleHideMainPath::redo().");
    Do(m_newState);
}

//---------------------------------------------------------------------------------------------------------------------
void ToggleHideMainPath::Do(bool state)
{
    QDomElement detail = doc->FindElementById(m_id, VAbstractPattern::TagDetail);
    if (detail.isElement())
    {
        doc->SetAttribute(detail, VToolSeamAllowance::AttrHideMainPath, state);

        VPiece det = m_data->DataPieces()->value(m_id);
        det.SetHideMainPath(state);
        m_data->UpdatePiece(m_id, det);
        emit Toggled(m_id);
    }
    else
    {
        qDebug("Can't get detail by id = %u.", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
ToggleShowFullPiece::ToggleShowFullPiece(quint32 id, bool state, VContainer *data, VAbstractPattern *doc,
                                         QUndoCommand *parent)
  : VUndoCommand(QDomElement(), doc, parent),
    m_id(id),
    m_data(data),
    m_oldState(not state),
    m_newState(state)
{
    setText(tr("show full piece"));
    m_oldState = m_data->DataPieces()->value(m_id).IsShowFullPiece();
}

//---------------------------------------------------------------------------------------------------------------------
void ToggleShowFullPiece::undo()
{
    qCDebug(vUndo, "ToggleShowFullPiece::undo().");
    Do(m_oldState);
}

//---------------------------------------------------------------------------------------------------------------------
void ToggleShowFullPiece::redo()
{
    qCDebug(vUndo, "ToggleShowFullPiece::redo().");
    Do(m_newState);
}

//---------------------------------------------------------------------------------------------------------------------
void ToggleShowFullPiece::Do(bool state)
{
    QDomElement detail = doc->FindElementById(m_id, VAbstractPattern::TagDetail);
    if (detail.isElement())
    {
        doc->SetAttribute(detail, VToolSeamAllowance::AttrShowFullPiece, state);

        VPiece det = m_data->DataPieces()->value(m_id);
        det.SetShowFullPiece(state);
        m_data->UpdatePiece(m_id, det);
        emit Toggled(m_id);
    }
    else
    {
        qDebug("Can't get detail by id = %u.", m_id);
    }
}
