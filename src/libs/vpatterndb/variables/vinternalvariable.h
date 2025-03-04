/************************************************************************
 **
 **  @file   vinternalvariable.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 7, 2014
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

#ifndef VINTERNALVARIABLE_H
#define VINTERNALVARIABLE_H

#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QtGlobal>

#include "../vmisc/def.h"

class VInternalVariableData;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VInternalVariable
{
public:
    VInternalVariable();
    VInternalVariable(const VInternalVariable &var);
    virtual ~VInternalVariable();

    auto operator=(const VInternalVariable &var) -> VInternalVariable &;

    VInternalVariable(VInternalVariable &&var) noexcept;
    auto operator=(VInternalVariable &&var) noexcept -> VInternalVariable &;

    virtual auto GetValue() const -> qreal;
    virtual auto GetValue() -> qreal *;

    auto GetName() const -> QString;
    void SetName(const QString &name);

    auto GetType() const -> VarType;

    void SetAlias(const QString &alias);
    auto GetAlias() const -> QString;

    virtual auto Filter(quint32 id) -> bool;

    virtual auto IsNotUsed() const -> bool;

protected:
    void StoreValue(qreal value);
    void SetType(const VarType &type);

private:
    QSharedDataPointer<VInternalVariableData> d;
};

QT_WARNING_POP

Q_DECLARE_TYPEINFO(VInternalVariable, Q_MOVABLE_TYPE); // NOLINT

#endif // VINTERNALVARIABLE_H
