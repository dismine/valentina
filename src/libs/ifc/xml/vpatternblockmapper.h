/************************************************************************
 **
 **  @file   vpatternblockmapper.h
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
#ifndef VPATTERNBLOCKMAPPER_H
#define VPATTERNBLOCKMAPPER_H

#include <QDomElement>
#include <QHash>
#include <QObject>

#include "../vmisc/defglobal.h"
#include <qtmetamacros.h>

class VPatternBlockMapper : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VPatternBlockMapper(QObject *parent = nullptr);
    ~VPatternBlockMapper() override = default;

    // Add a block name and get its unique ID (or get existing ID)
    auto GetId(const QString &blockName) -> int;

    // Add a block with its DOM element
    auto AddBlock(const QString &blockName, const QDomElement &element) -> int;

    // Check if a block name exists and get its ID
    auto FindId(const QString &blockName) const -> int;

    // Check if a block with given ID exists
    auto HasId(int id) const -> bool;

    // Get block name by ID
    auto FindName(int id) const -> QString;

    // Get list of all known block names
    auto GetBlockNames() const -> QStringList;

    // Get list of all block IDs
    auto GetBlockIds() const -> QList<int>;

    // Set DOM element for a block by name
    auto SetElement(const QString &blockName, const QDomElement &element) -> bool;

    // Set DOM element for a block by ID
    auto SetElementById(int id, const QDomElement &element) -> bool;

    // Get DOM element by block name
    auto GetElement(const QString &blockName) const -> QDomElement;

    // Get DOM element by ID
    auto GetElementById(int id) const -> QDomElement;

    // Get active block's DOM element
    auto GetActiveElement() const -> QDomElement;

    // Check if block has cached element by name
    auto HasElement(const QString &blockName) const -> bool;

    // Check if block has cached element by ID
    auto HasElementById(int id) const -> bool;

    // Replace old block name with new name (keeps same ID)
    auto Rename(const QString &oldName, const QString &newName) -> bool;

    // Check if name exists
    auto Contains(const QString &blockName) const -> bool;

    // Set active block
    auto SetActive(const QString &blockName) -> bool;

    // Set active block by ID
    auto SetActiveById(int id) -> bool;

    // Get active block name
    auto GetActive() const -> QString;

    // Get active block ID
    auto GetActiveId() const -> int;

    // Check if there's an active block
    auto HasActive() const -> bool;

    // Clear active block
    void ClearActive();

    // Clear all mappings and reset ID counter
    void Clear();

    // Get current number of mapped blocks
    auto Size() const -> vsizetype;

signals:
    /**
     * @brief ChangedActivePatternBlock change active pattern block.
     * @param id new pattern peace id.
     */
    void ChangedActivePatternBlock(int id);

    /**
     * @brief ChangedActivePatternBlock change active pattern block.
     * @param name new pattern peace name.
     */
    void ChangedActivePatternBlock(const QString &name);

private:
    Q_DISABLE_COPY_MOVE(VPatternBlockMapper) // NOLINT

    QHash<QString, int> m_nameToId{};
    QHash<int, QString> m_idToName{}; // Reverse lookup
    QHash<int, QDomElement> m_idToElement{};
    int m_nextId{0};
    QString m_activeBlock{};
};

#endif // VPATTERNBLOCKMAPPER_H
