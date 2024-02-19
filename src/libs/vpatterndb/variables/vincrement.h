/************************************************************************
 **
 **  @file   vincrementtablerow.h
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

#ifndef VINCREMENTTABLEROW_H
#define VINCREMENTTABLEROW_H

#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QtGlobal>

#include "vvariable.h"

class VIncrementData;
class VContainer;

/**
 * @brief The VIncrement class keep data row of increment table
 */
class VIncrement : public VVariable
{
public:
    VIncrement();
    VIncrement(VContainer *data, const QString &name, IncrementType incrType = IncrementType::Increment);
    VIncrement(const VIncrement &incr);

    ~VIncrement() override;

    auto operator=(const VIncrement &incr) -> VIncrement &;

    VIncrement(VIncrement &&incr) noexcept;
    auto operator=(VIncrement &&incr) noexcept -> VIncrement &;

    void SetFormula(qreal base, const QString &formula, bool ok);
    auto GetFormula() const -> QString;
    auto IsFormulaOk() const -> bool;

    void SetIndex(quint32 index);
    auto GetIndex() const -> quint32;

    auto GetData() -> VContainer *;
    auto GetIncrementType() const -> IncrementType;

    auto IsPreviewCalculation() const -> bool;
    void SetPreviewCalculation(bool value);

    auto IsSpecialUnits() const -> bool;
    void SetSpecialUnits(bool special);

private:
    QSharedDataPointer<VIncrementData> d;
};

Q_DECLARE_TYPEINFO(VIncrement, Q_MOVABLE_TYPE); // NOLINT

#endif // VINCREMENTTABLEROW_H
