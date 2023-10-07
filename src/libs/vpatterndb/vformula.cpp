/************************************************************************
 **
 **  @file   vformula.cpp
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

#include "vformula.h"
#include "vformula_p.h"

#include <QMessageLogger>
#include <QScopedPointer>
#include <QtDebug>
#include <limits>

#include "../qmuparser/qmuparsererror.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "calculator.h"
#include "vcontainer.h"
#include "vtranslatevars.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

// VFormula
//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula()
  : d(new VFormulaData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula(const QString &formula, const VContainer *container)
  : d(new VFormulaData(formula, container))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::operator=(const VFormula &formula) -> VFormula &
{
    if (&formula == this)
    {
        return *this;
    }
    d = formula.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VFormula)

//---------------------------------------------------------------------------------------------------------------------
VFormula::~VFormula() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::operator==(const VFormula &formula) const -> bool
{
    return d->formula == formula.GetFormula() && d->strValue == formula.getStringValue() &&
           d->checkZero == formula.getCheckZero() && d->checkLessThanZero == formula.getCheckLessThanZero() &&
           d->data == formula.getData() && d->toolId == formula.getToolId() && d->postfix == formula.getPostfix() &&
           d->error == formula.error() && VFuzzyComparePossibleNulls(d->dValue, formula.getDoubleValue());
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::operator!=(const VFormula &formula) const -> bool
{
    return !VFormula::operator==(formula);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::GetFormula(FormulaType type) const -> QString
{
    if (type == FormulaType::ToUser)
    {
        return VTranslateVars::TryFormulaToUser(d->formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    }

    return d->formula;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::SetFormula(const QString &value, FormulaType type)
{
    if (d->formula != value)
    {
        if (type == FormulaType::FromUser)
        {
            d->formula =
                VTranslateVars::TryFormulaFromUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        }
        else
        {
            d->formula = value;
        }

        ResetState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getStringValue() const -> QString
{
    return d->strValue;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getDoubleValue() const -> qreal
{
    return d->dValue;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getCheckZero() const -> bool
{
    return d->checkZero;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setCheckZero(bool value)
{
    d->checkZero = value;
    ResetState();
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getCheckLessThanZero() const -> bool
{
    return d->checkLessThanZero;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setCheckLessThanZero(bool value)
{
    d->checkLessThanZero = value;
    ResetState();
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getData() const -> const VContainer *
{
    return d->data;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setData(const VContainer *value)
{
    if (d->data != value && value != nullptr)
    {
        d->data = value;
        ResetState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getToolId() const -> quint32
{
    return d->toolId;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setToolId(quint32 value)
{
    d->toolId = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::getPostfix() const -> QString
{
    return d->postfix;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setPostfix(const QString &value)
{
    d->postfix = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::error() const -> bool
{
    return d->error;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::Reason() const -> QString
{
    return d->reason;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormula::FormulaTypeId() -> int
{
    return qMetaTypeId<VFormula>();
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::Eval()
{
    ResetState();

    if (d->data == nullptr)
    {
        d->reason = tr("Data container is empty");
        return;
    }

    qreal result = 0;

    if (not d->formula.isEmpty())
    {
        try
        {
            QScopedPointer<Calculator> cal(new Calculator());
            result = cal->EvalFormula(d->data->DataVariables(), d->formula);
        }
        catch (qmu::QmuParserError &e)
        {
            d->reason = tr("Math parser error: %1").arg(e.GetMsg());
            return;
        }

        d->dValue = result;

        if (qIsInf(result))
        {
            d->reason = tr("Result is infinite");
        }
        else if (qIsNaN(result))
        {
            d->reason = tr("Result is NaN");
        }
        else if (d->checkZero && qFuzzyIsNull(result))
        {
            d->reason = tr("Result is zero");
        }
        else if (d->checkLessThanZero && result < 0)
        {
            d->reason = tr("Result less than zero");
        }
        else
        {
            d->strValue = VAbstractApplication::VApp()->LocaleToString(result) + ' '_L1 + d->postfix;
            d->error = false;
        }
    }
    else
    {
        d->reason = tr("Formula is empty");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::ResetState()
{
    d->strValue = tr("Error");
    d->error = true;
    d->dValue = std::numeric_limits<qreal>::quiet_NaN();
    d->reason = tr("Not evaluated");
}
