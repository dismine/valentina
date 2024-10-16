/************************************************************************
 **
 **  @file   calculator.h
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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QHash>
#include <QMap>
#include <QString>
#include <QtGlobal>

#include "../qmuparser/qmuformulabase.h"

class VInternalVariable;

/**
 * @brief The Calculator class for calculation formula.
 *
 * Main purpose make easy evaluate value of formula and get tokens.
 * Note. If created to many parser for different purpes in the same time parser can work wrong.
 * Example:
 * DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data);
 * dialog->SetFormula(formula);
 * if (dialog->exec() == QDialog::Accepted)
 * {
 *     formula = dialog->GetFormula();
 *     //Need delete dialog here because parser in dialog don't allow use correct separator for parsing here.
 *     //Don't know why.
 *     delete dialog;
 *     QScopedPointer<Calculator> cal(new Calculator());
 *     result = cal->EvalFormula(data->PlainVariables(), formula);
 * }
 */
class Calculator final : public qmu::QmuFormulaBase
{
public:
    Calculator();
    ~Calculator() override = default;

    auto EvalFormula(const QHash<QString, QSharedPointer<VInternalVariable>> *vars, const QString &formula) -> qreal;

private:
    Q_DISABLE_COPY_MOVE(Calculator) // NOLINT
    QVector<QSharedPointer<qreal>> m_varsValues{};
    const QHash<QString, QSharedPointer<VInternalVariable>> *m_vars{nullptr};

    static auto VarFactory(const QString &a_szName, void *a_pUserData) -> qreal *;
    static auto Warning(const QString &warningMsg, qreal value) -> qreal;
};

#endif // CALCULATOR_H
