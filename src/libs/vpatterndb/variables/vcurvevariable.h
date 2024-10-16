/************************************************************************
 **
 **  @file   vcurvelength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#ifndef VCURVEVARIABLE_H
#define VCURVEVARIABLE_H

#include <QSharedDataPointer>
#include <QTypeInfo>
#include <QtGlobal>

#include "vinternalvariable.h"

class VCurveVariableData;

class VCurveVariable : public VInternalVariable
{
public:
    VCurveVariable();
    VCurveVariable(const quint32 &id, const quint32 &parentId);
    VCurveVariable(const VCurveVariable &var);
    ~VCurveVariable() override;

    auto operator=(const VCurveVariable &var) -> VCurveVariable &;

    VCurveVariable(VCurveVariable &&var) noexcept;
    auto operator=(VCurveVariable &&var) noexcept -> VCurveVariable &;

    auto Filter(quint32 id) -> bool override;

    auto GetId() const -> quint32;
    void SetId(const quint32 &id);

    auto GetParentId() const -> quint32;
    void SetParentId(const quint32 &value);

private:
    QSharedDataPointer<VCurveVariableData> d;
};

Q_DECLARE_TYPEINFO(VCurveVariable, Q_MOVABLE_TYPE); // NOLINT

#endif // VCURVEVARIABLE_H
