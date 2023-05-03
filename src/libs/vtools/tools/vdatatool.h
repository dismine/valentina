/************************************************************************
 **
 **  @file   vdatatool.h
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

#ifndef VDATATOOL_H
#define VDATATOOL_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QLoggingCategory>

#include "../vpatterndb/vcontainer.h"

Q_DECLARE_LOGGING_CATEGORY(vTool)

//We need QObject class because we use qobject_cast.
/**
 * @brief The VDataTool class need for getting access to data container of tool.
 */
class VDataTool : public QObject
{
    Q_OBJECT // NOLINT
public:
    explicit VDataTool(VContainer *data, QObject *parent = nullptr);
    virtual ~VDataTool() = default;
    auto getData() const -> VContainer;
    void            setData(const VContainer *value);
    virtual auto referens() const -> quint32;
    virtual void    incrementReferens();
    virtual void    decrementReferens();
    virtual void    GroupVisibility(quint32 object, bool visible)=0;
protected:
    /** @brief data container with data */
    VContainer            data;

    /** @brief _referens keep count tools what use this tool. If value more than 1 you can't delete tool. */
    quint32                _referens;
private:
    Q_DISABLE_COPY_MOVE(VDataTool) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getData return data container.
 * @return container.
 */
inline auto VDataTool::getData() const -> VContainer
{
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setData set data container.
 * @param value container.
 */
inline void VDataTool::setData(const VContainer *value)
{
    data = *value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief referens return count of referens.
 * @return count count of referens.
 */
inline auto VDataTool::referens() const -> quint32
{
    return _referens;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief incrementReferens increment referens.
 */
inline void VDataTool::incrementReferens()
{
    ++_referens;
}

#endif // VDATATOOL_H
