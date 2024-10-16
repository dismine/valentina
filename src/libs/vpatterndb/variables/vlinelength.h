/************************************************************************
 **
 **  @file   vlinelength.h
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

#ifndef VLINELENGTH_H
#define VLINELENGTH_H

#include <QSharedDataPointer>
#include <QTypeInfo>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vinternalvariable.h"

class VLengthLineData;
class VPointF;

class VLengthLine final : public VInternalVariable
{
public:
    VLengthLine();
    VLengthLine(const VPointF *p1, const quint32 &p1Id, const VPointF *p2, const quint32 &p2Id, Unit patternUnit);
    VLengthLine(const VLengthLine &var);
    ~VLengthLine() override;

    auto operator=(const VLengthLine &var) -> VLengthLine &;

    VLengthLine(VLengthLine &&var) noexcept;
    auto operator=(VLengthLine &&var) noexcept -> VLengthLine &;

    auto Filter(quint32 id) -> bool override;

    void SetValue(const VPointF *p1, const VPointF *p2);

    auto GetP1Id() const -> quint32;
    auto GetP2Id() const -> quint32;

private:
    QSharedDataPointer<VLengthLineData> d;
};

Q_DECLARE_TYPEINFO(VLengthLine, Q_MOVABLE_TYPE); // NOLINT

#endif // VLINELENGTH_H
