/************************************************************************
 **
 **  @file   vtoolrecord.h
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

#ifndef VTOOLRECORD_H
#define VTOOLRECORD_H

#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../vmisc/typedef.h"

/**
 * @brief The VToolRecord class record about tool in history.
 */
class VToolRecord
{
public:
    VToolRecord() = default;
    VToolRecord(vidtype id, Tool toolType, int indexPatternBlock);
    VToolRecord(const VToolRecord &record) = default;
    auto operator=(const VToolRecord &record) -> VToolRecord & = default;
    ~VToolRecord() = default;

    VToolRecord(VToolRecord &&) = default;
    auto operator=(VToolRecord &&) -> VToolRecord & = default;

    auto operator==(const VToolRecord &record) const -> bool;

    auto GetId() const -> quint32;
    void SetId(const quint32 &value);

    auto GetToolType() const -> Tool;
    void SetToolType(const Tool &value);

    auto GetPatternBlockIndex() const -> int;
    void SetPatternBlockIndex(int index);

    auto IsMandatory() const -> bool;

private:
    /** @brief id tool id. */
    vidtype id{NULL_ID};

    /** @brief toolType tool type. */
    Tool toolType{Tool::LAST_ONE_DO_NOT_USE};

    /** @brief nameDraw pattern peace name. */
    int indexPatternBlock{-1};
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getId return tool id.
 * @return id.
 */
inline auto VToolRecord::GetId() const -> vidtype
{
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetId set tool id.
 * @param value id.
 */
inline void VToolRecord::SetId(const vidtype &value)
{
    id = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetToolType return tool type.
 * @return tool type.
 */
inline auto VToolRecord::GetToolType() const -> Tool
{
    return toolType;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetToolType set tool type.
 * @param value tool type.
 */
inline void VToolRecord::SetToolType(const Tool &value)
{
    toolType = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPatternBlockIndex return pattern block index.
 * @return pattern block index.
 */
inline auto VToolRecord::GetPatternBlockIndex() const -> int
{
    return indexPatternBlock;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPatternBlockIndex set pattern block index.
 * @param index pattern block index.
 */
inline void VToolRecord::SetPatternBlockIndex(int index)
{
    indexPatternBlock = index;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolRecord::operator==(const VToolRecord &record) const -> bool
{
    // Id should be enough
    return id == record.id;
}

#endif // VTOOLRECORD_H
