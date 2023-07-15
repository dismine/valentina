/************************************************************************
 **
 **  @file   vformula.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
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

#ifndef VFORMULA_H
#define VFORMULA_H

#include <QCoreApplication>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QtGlobal>

enum class FormulaType : qint8
{
    ToUser = 0,
    FromUser = 1,
    ToSystem = FromUser,
    FromSystem = ToUser
};

class VContainer;
class VFormulaData;

class VFormula
{
    Q_DECLARE_TR_FUNCTIONS(VFormula) // NOLINT

public:
    VFormula();
    VFormula(const QString &formula, const VContainer *container);
    auto operator=(const VFormula &formula) -> VFormula &;
    VFormula(const VFormula &formula);
    ~VFormula();

    auto operator==(const VFormula &formula) const -> bool;
    auto operator!=(const VFormula &formula) const -> bool;

    auto GetFormula(FormulaType type = FormulaType::ToUser) const -> QString;
    void SetFormula(const QString &value, FormulaType type = FormulaType::FromSystem);

    auto getStringValue() const -> QString;
    auto getDoubleValue() const -> qreal;

    auto getCheckZero() const -> bool;
    void setCheckZero(bool value);

    auto getCheckLessThanZero() const -> bool;
    void setCheckLessThanZero(bool value);

    auto getData() const -> const VContainer *;
    void setData(const VContainer *value);

    auto getToolId() const -> quint32;
    void setToolId(quint32 value);

    auto getPostfix() const -> QString;
    void setPostfix(const QString &value);

    auto error() const -> bool;
    auto Reason() const -> QString;

    static auto FormulaTypeId() -> int;

    void Eval();

private:
    QSharedDataPointer<VFormulaData> d;

    void ResetState();
};
Q_DECLARE_METATYPE(VFormula)                  // NOLINT
Q_DECLARE_TYPEINFO(VFormula, Q_MOVABLE_TYPE); // NOLINT

#endif // VFORMULA_H
