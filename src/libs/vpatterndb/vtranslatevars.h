/************************************************************************
 **
 **  @file   vtranslatevars.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VTRANSLATEVARS_H
#define VTRANSLATEVARS_H

#include <QtGlobal>

#include "../vmisc/defglobal.h"
#include "vtranslatemeasurements.h"

class VTranslateVars : public VTranslateMeasurements
{
public:
    explicit VTranslateVars();
    ~VTranslateVars() override = default;

    auto InternalVarToUser(const QString &var) const -> QString;

    auto VarToUser(const QString &var) const -> QString;
    auto VarFromUser(const QString &var) const -> QString;

    auto PMSystemName(const QString &code) const -> QString;
    auto PMSystemAuthor(const QString &code) const -> QString;
    auto PMSystemBook(const QString &code) const -> QString;

    auto FormulaFromUser(const QString &formula, bool osSeparator) const -> QString;
    static auto TryFormulaFromUser(const QString &formula, bool osSeparator) -> QString;

    auto FormulaToUser(const QString &formula, bool osSeparator) const -> QString;
    static auto TryFormulaToUser(const QString &formula, bool osSeparator) -> QString;

    void Retranslate() override;

    auto GetTranslatedFunctions() const -> QMap<QString, QString>;
    auto GetFunctions() const -> QMap<QString, qmu::QmuTranslation>;
    auto GetFunctionsDescriptions() const -> QMap<QString, qmu::QmuTranslation>;
    auto GetFunctionsArguments() const -> QMap<QString, QString>;

    static void BiasTokens(vsizetype position, vsizetype bias, QMap<vsizetype, QString> &tokens);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VTranslateVars) // NOLINT
    QMap<QString, qmu::QmuTranslation> PMSystemNames{};
    QMap<QString, qmu::QmuTranslation> PMSystemAuthors{};
    QMap<QString, qmu::QmuTranslation> PMSystemBooks{};
    QMap<QString, qmu::QmuTranslation> variables{};
    QMap<QString, qmu::QmuTranslation> functions{};
    QMap<QString, qmu::QmuTranslation> functionsDescriptions{};
    QMap<QString, QString> functionsArguments{};
    QMap<QString, qmu::QmuTranslation> stDescriptions{};
    QMap<QString, QString> translatedFunctions{};

    void InitPatternMakingSystems();
    void InitVariables();
    void InitFunctions();

    auto VariablesFromUser(QString &newFormula, vsizetype position, const QString &token, vsizetype &bias) const
        -> bool;
    auto FunctionsFromUser(QString &newFormula, vsizetype position, const QString &token, vsizetype &bias) const
        -> bool;
    auto VariablesToUser(QString &newFormula, vsizetype position, const QString &token, vsizetype &bias) const -> bool;

    void PrepareFunctionTranslations();

    void InitSystem(const QString &code, const qmu::QmuTranslation &name, const qmu::QmuTranslation &author,
                    const qmu::QmuTranslation &book);

    static void CorrectionsPositions(vsizetype position, vsizetype bias, QMap<vsizetype, QString> &tokens,
                                     QMap<vsizetype, QString> &numbers);

    void TranslateVarsFromUser(QString &newFormula, QMap<vsizetype, QString> &tokens,
                               QMap<vsizetype, QString> &numbers) const;
    static void TranslateNumbersFromUser(QString &newFormula, QMap<vsizetype, QString> &tokens,
                                         QMap<vsizetype, QString> &numbers, bool osSeparator);

    void TranslateVarsToUser(QString &newFormula, QMap<vsizetype, QString> &tokens,
                             QMap<vsizetype, QString> &numbers) const;
    static void TranslateNumbersToUser(QString &newFormula, QMap<vsizetype, QString> &tokens,
                                       QMap<vsizetype, QString> &numbers, bool osSeparator);
};

#endif // VTRANSLATEVARS_H
