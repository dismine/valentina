/************************************************************************
 **
 **  @file   movetool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 1, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#include "movetool.h"
#include "typedef.h"
#include <qdom.h>

//---------------------------------------------------------------------------------------------------------------------
MoveToolUp::MoveToolUp(VAbstractPattern *doc, vidtype currentId, vidtype topId, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_currentId(currentId),
    m_topId(topId)
{
    SCASSERT(m_currentId > 0)
    SCASSERT(m_topId > 0)

    setText(tr("move tool up in history"));

    if (QDomElement const currentElement = doc->FindElementById(m_currentId); currentElement.isElement())
    {
        QDomNode previousSibling = currentElement.previousSibling();
        while (!previousSibling.isNull() && !previousSibling.isElement())
        {
            previousSibling = previousSibling.previousSibling();
        }

        if (previousSibling.isElement())
        {
            if (QDomElement const prevElement = previousSibling.toElement();
                prevElement.hasAttribute(VAbstractPattern::AttrId))
            {
                m_originalPreviousId = VDomDocument::GetParametrId(prevElement);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MoveToolUp::undo()
{
    QDomElement const currentElement = Doc()->FindElementById(m_currentId);
    if (!currentElement.isElement())
    {
        return;
    }

    QDomNode currentParent = currentElement.parentNode();
    if (currentParent.isNull())
    {
        return;
    }

    QDomNode res = currentParent.removeChild(currentElement);
    if (res.isNull())
    {
        return;
    }

    // Restore to original position
    if (m_originalPreviousId != NULL_ID)
    {
        if (QDomElement const previousElement = Doc()->FindElementById(m_originalPreviousId);
            previousElement.isElement())
        {
            QDomNode previousParent = previousElement.parentNode();
            res = previousParent.insertAfter(currentElement, previousElement);
        }
    }
    else
    {
        // Was first element, insert as first child
        res = currentParent.insertBefore(currentElement, currentParent.firstChild());
    }

    if (res.isNull())
    {
        // Try to recover
        // Restore to original position
        if (m_topId != NULL_ID)
        {
            if (QDomElement const topElement = Doc()->FindElementById(m_topId); topElement.isElement())
            {
                QDomNode topParent = topElement.parentNode();
                topParent.insertAfter(currentElement, topElement);
            }
        }
        else
        {
            // Was first element, insert as first child
            currentParent.appendChild(currentElement);
        }

        return;
    }

    Doc()->RefreshElementIdCache();

    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
void MoveToolUp::redo()
{
    QDomElement const currentElement = Doc()->FindElementById(m_currentId);
    if (!currentElement.isElement())
    {
        return;
    }

    QDomNode currentParent = currentElement.parentNode();
    if (currentParent.isNull())
    {
        return;
    }

    QDomElement const topElement = Doc()->FindElementById(m_topId);
    if (!topElement.isElement())
    {
        return;
    }

    QDomNode topParent = topElement.parentNode();
    if (topParent.isNull())
    {
        return;
    }

    QDomNode removedCurrentElement = currentParent.removeChild(currentElement);
    if (removedCurrentElement.isNull())
    {
        return;
    }

    QDomNode insertedElement = topParent.insertAfter(removedCurrentElement, topElement);
    if (insertedElement.isNull())
    {
        // Try to recover
        // Restore to original position
        if (m_originalPreviousId != NULL_ID)
        {
            if (QDomElement const previousElement = Doc()->FindElementById(m_originalPreviousId);
                previousElement.isElement())
            {
                QDomNode previousParent = previousElement.parentNode();
                previousParent.insertAfter(currentElement, previousElement);
            }
        }
        else
        {
            // Was first element, insert as first child
            currentParent.appendChild(currentElement);
        }

        return;
    }

    Doc()->RefreshElementIdCache();

    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveToolUp::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const MoveToolUp *>(command);

    if (moveCommand->m_currentId != m_currentId)
    {
        return false;
    }

    m_topId = moveCommand->m_topId;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveToolUp::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveToolUp);
}

// ============================================================================
// MoveToolDown
// ============================================================================

//---------------------------------------------------------------------------------------------------------------------
MoveToolDown::MoveToolDown(VAbstractPattern *doc, vidtype currentId, vidtype bottomId, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_currentId(currentId),
    m_bottomId(bottomId)
{
    SCASSERT(m_currentId > 0)

    setText(tr("move tool down in history"));

    if (QDomElement const currentElement = doc->FindElementById(m_currentId); currentElement.isElement())
    {
        QDomNode nextSibling = currentElement.nextSibling();
        while (!nextSibling.isNull() && !nextSibling.isElement())
        {
            nextSibling = nextSibling.nextSibling();
        }

        if (nextSibling.isElement())
        {
            if (QDomElement const nextElement = nextSibling.toElement();
                nextElement.hasAttribute(VAbstractPattern::AttrId))
            {
                m_originalNextId = VDomDocument::GetParametrId(nextElement);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MoveToolDown::undo()
{
    QDomElement const currentElement = Doc()->FindElementById(m_currentId);
    if (!currentElement.isElement())
    {
        return;
    }

    QDomNode currentParent = currentElement.parentNode();
    if (currentParent.isNull())
    {
        return;
    }

    QDomNode const currentRemovedElement = currentParent.removeChild(currentElement);
    if (currentRemovedElement.isNull())
    {
        return;
    }

    QDomNode res;
    // Restore to original position
    if (m_originalNextId != NULL_ID)
    {
        if (QDomElement const nextElement = Doc()->FindElementById(m_originalNextId); nextElement.isElement())
        {
            QDomNode nextParent = nextElement.parentNode();
            res = nextParent.insertBefore(currentRemovedElement, nextElement);
        }
    }
    else
    {
        // Was first element, insert as first child
        res = currentParent.appendChild(currentRemovedElement);
    }

    if (res.isNull())
    {
        // Try to recover
        // Restore to original position
        if (m_bottomId != NULL_ID)
        {
            if (QDomElement const bottomElement = Doc()->FindElementById(m_bottomId); bottomElement.isElement())
            {
                QDomNode bottomParent = bottomElement.parentNode();
                bottomParent.insertBefore(currentElement, bottomElement);
            }
        }
        else
        {
            // Was first element, insert as first child
            currentParent.appendChild(currentElement);
        }

        return;
    }

    Doc()->RefreshElementIdCache();

    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
void MoveToolDown::redo()
{
    QDomElement const currentElement = Doc()->FindElementById(m_currentId);
    if (!currentElement.isElement())
    {
        return;
    }

    QDomNode currentParent = currentElement.parentNode();
    if (currentParent.isNull())
    {
        return;
    }

    QDomElement bottomElement;

    if (m_bottomId != NULL_ID)
    {
        bottomElement = Doc()->FindElementById(m_bottomId);
        if (!bottomElement.isElement())
        {
            return;
        }
    }

    QDomNode const currentRemovedElement = currentParent.removeChild(currentElement);
    if (currentRemovedElement.isNull())
    {
        return;
    }

    QDomNode res;
    if (m_bottomId != NULL_ID)
    {
        res = currentParent.insertBefore(currentRemovedElement, bottomElement);
    }
    else
    {
        res = currentParent.appendChild(currentElement);
    }

    if (res.isNull())
    {
        // Try to recover
        // Restore to original position
        if (m_originalNextId != NULL_ID)
        {
            if (QDomElement const nextElement = Doc()->FindElementById(m_originalNextId); nextElement.isElement())
            {
                QDomNode nextParent = nextElement.parentNode();
                nextParent.insertBefore(currentElement, nextElement);
            }
        }
        else
        {
            // Was first element, insert as first child
            currentParent.appendChild(currentElement);
        }

        return;
    }

    Doc()->RefreshElementIdCache();

    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveToolDown::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const MoveToolDown *>(command);

    if (moveCommand->m_currentId != m_currentId)
    {
        return false;
    }

    m_bottomId = moveCommand->m_bottomId;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveToolDown::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveToolDown);
}
