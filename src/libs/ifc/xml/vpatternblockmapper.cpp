/************************************************************************
 **
 **  @file   vpatternblockmapper.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 12, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "vpatternblockmapper.h"
#include "../ifcdef.h"

//---------------------------------------------------------------------------------------------------------------------
VPatternBlockMapper::VPatternBlockMapper(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetId(const QString &blockName) -> int
{
    auto it = m_nameToId.find(blockName);
    if (it != m_nameToId.end())
    {
        return it.value();
    }
    // New name - assign next ID
    int const id = m_nextId++;
    m_nameToId[blockName] = id;
    m_idToName[id] = blockName;
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::AddBlock(const QString &blockName, const QDomElement &element) -> int
{
    int const id = GetId(blockName);
    m_idToElement[id] = element;
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::FindId(const QString &blockName) const -> int
{
    return m_nameToId.value(blockName, -1); // Returns -1 if not found
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::HasId(int id) const -> bool
{
    return m_idToName.contains(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::FindName(int id) const -> QString
{
    return m_idToName.value(id, QString()); // Returns empty string if not found
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetBlockNames() const -> QStringList
{
    return m_nameToId.keys();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetBlockIds() const -> QList<int>
{
    return m_idToName.keys();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::SetElement(const QString &blockName, const QDomElement &element) -> bool
{
    auto it = m_nameToId.find(blockName);
    if (it == m_nameToId.end())
    {
        return false;
    }
    m_idToElement[it.value()] = element;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::SetElementById(int id, const QDomElement &element) -> bool
{
    if (!m_idToName.contains(id))
    {
        return false;
    }
    m_idToElement[id] = element;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetElement(const QString &blockName) const -> QDomElement
{
    int const id = FindId(blockName);
    if (id == -1)
    {
        return {}; // Return null element
    }
    return m_idToElement.value(id, QDomElement());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetElementById(int id) const -> QDomElement
{
    return m_idToElement.value(id, QDomElement());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetActiveElement() const -> QDomElement
{
    if (m_activeBlock.isEmpty())
    {
        return {};
    }
    return GetElement(m_activeBlock);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::HasElement(const QString &blockName) const -> bool
{
    int const id = FindId(blockName);
    return id != -1 && m_idToElement.contains(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::HasElementById(int id) const -> bool
{
    return m_idToElement.contains(id);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::Rename(const QString &oldName, const QString &newName) -> bool
{
    Q_ASSERT_X(not newName.isEmpty(), Q_FUNC_INFO, "new name is empty");
    Q_ASSERT_X(not oldName.isEmpty(), Q_FUNC_INFO, "old name is empty");

    auto it = m_nameToId.find(oldName);
    if (it == m_nameToId.end())
    {
        return false; // Old name doesn't exist
    }

    int const id = it.value();

    QDomElement oldNameElement = GetElementById(id);

    m_nameToId.remove(oldName);
    m_nameToId[newName] = id;
    m_idToName[id] = newName;

    // Update active block if it was renamed
    if (m_activeBlock == oldName)
    {
        m_activeBlock = newName;
    }

    if (!oldNameElement.isNull())
    {
        oldNameElement.setAttribute(AttrName, newName);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::Contains(const QString &blockName) const -> bool
{
    return m_nameToId.contains(blockName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::SetActive(const QString &blockName) -> bool
{
    if (!m_nameToId.contains(blockName))
    {
        return false; // Block doesn't exist
    }

    if (m_activeBlock != blockName)
    {
        m_activeBlock = blockName;

        emit ChangedActivePatternBlock(m_nameToId[m_activeBlock]);
        emit ChangedActivePatternBlock(m_activeBlock);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::SetActiveById(int id) -> bool
{
    if (!m_idToName.contains(id))
    {
        return false; // Block with this ID doesn't exist
    }

    if (m_activeBlock != m_idToName[id])
    {
        m_activeBlock = m_idToName[id];

        emit ChangedActivePatternBlock(id);
        emit ChangedActivePatternBlock(m_activeBlock);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetActive() const -> QString
{
    return m_activeBlock;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::GetActiveId() const -> int
{
    if (m_activeBlock.isEmpty())
    {
        return -1;
    }
    return FindId(m_activeBlock);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::HasActive() const -> bool
{
    return !m_activeBlock.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternBlockMapper::ClearActive()
{
    m_activeBlock.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternBlockMapper::Clear()
{
    m_nameToId.clear();
    m_idToName.clear();
    m_idToElement.clear();
    m_nextId = 0;
    m_activeBlock.clear();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternBlockMapper::Size() const -> vsizetype
{
    return m_nameToId.size();
}
