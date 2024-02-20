/************************************************************************
 **
 **  @file   calculator.cpp
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

#include "calculator.h"

#include <QSharedPointer>
#include <QStringList>
#include <QtDebug>

#include "../qmuparser/qmuparsererror.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "variables/vinternalvariable.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Calculator class wraper for QMuParser. Make easy initialization math parser.
 *
 * This constructor hide initialization variables, operators, character sets.
 * Use this constuctor for evaluation formula. All formulas must be converted to internal look.
 * Example:
 *
 * const QString formula = VAbstractApplication::VApp()->FormulaFromUser(edit->text());
 * Calculator *cal = new Calculator(data, patternType);
 * const qreal result = cal->EvalFormula(data->PlainVariables(), formula);
 * delete cal;
 *
 */
Calculator::Calculator()
{
    InitCharSets();

    // Parser doesn't know any variable on this stage. So, we just use variable factory that for each unknown variable
    // set value to 0.
    SetVarFactory(VarFactory, this);
    SetSepForEval();

    DefineFun(QStringLiteral("warning"), Warning);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief eval calculate formula.
 *
 * First we try eval expression without adding variables. If it fail, we take tokens from expression and add variables
 * to parser and try again.
 *
 * @param formula string of formula.
 * @return value of formula.
 */
auto Calculator::EvalFormula(const QHash<QString, QSharedPointer<VInternalVariable>> *vars, const QString &formula)
    -> qreal
{
    // Converting with locale is much faster in case of single numerical value.
    QLocale const c(QLocale::C);
    bool ok = false;
    qreal const result = c.toDouble(formula, &ok);
    if (ok)
    {
        return result;
    }

    SetSepForEval(); // Reset separators options
    m_vars = vars;
    SetExpr(formula);

    m_pTokenReader->IgnoreUndefVar(true);
    return Eval();
}

//---------------------------------------------------------------------------------------------------------------------
auto Calculator::VarFactory(const QString &a_szName, void *a_pUserData) -> qreal *
{
    Q_UNUSED(a_szName)
    Calculator *calc = static_cast<Calculator *>(a_pUserData);

    if (calc->m_vars != nullptr && calc->m_vars->contains(a_szName))
    {
        QSharedPointer<qreal> const val(new qreal(*calc->m_vars->value(a_szName)->GetValue()));
        calc->m_varsValues.append(val);
        return val.data();
    }

    if (a_szName.startsWith('#'))
    {
        QSharedPointer<qreal> const val(new qreal(std::numeric_limits<qreal>::quiet_NaN()));
        calc->m_varsValues.append(val);
        return val.data();
    }

    throw qmu::QmuParserError(qmu::ecUNASSIGNABLE_TOKEN);
}

//---------------------------------------------------------------------------------------------------------------------
auto Calculator::Warning(const QString &warningMsg, qreal value) -> qreal
{
    VAbstractApplication::VApp()->IsPedantic()
        ? throw qmu::QmuParserWarning(warningMsg)
        : qWarning() << VAbstractApplication::warningMessageSignature + warningMsg;

    return value;
}
