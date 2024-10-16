/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "tst_vtranslatevars.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vtranslatevars.h"
#include "testvapplication.h"

#include <QtTest>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VTranslateVars::TST_VTranslateVars(QObject *parent)
  : QObject(parent),
    m_trMs(nullptr),
    m_systemLocale(QLocale::system())
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    m_translateFomula = settings->IsTranslateFormula();
    settings->SetTranslateFormula(true);
}

//---------------------------------------------------------------------------------------------------------------------
TST_VTranslateVars::~TST_VTranslateVars()
{
    VAbstractApplication::VApp()->Settings()->SetTranslateFormula(m_translateFomula);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::initTestCase()
{
    m_trMs = new VTranslateVars();
    TestVApplication::VApp()->SetTrVars(m_trMs);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::TestFormulaFromUser_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<QLocale>("locale");

    const QList<QLocale> allLocales =
        QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const auto &locale : allLocales)
    {
        if (not SupportedLocale(locale))
        {
            continue;
        }

        PrepareValFromUser(1000.5, locale);
        PrepareValFromUser(-1000.5, locale);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::TestFormulaFromUser()
{
    QFETCH(QString, input);
    QFETCH(QString, output);
    QFETCH(QLocale, locale);

    QLocale::setDefault(locale);

    const QString result = VTranslateVars::TryFormulaFromUser(input, true);

    QCOMPARE(result, output);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::TestFormulaToUser_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<QLocale>("locale");

    const QList<QLocale> allLocales =
        QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const auto &locale : allLocales)
    {
        if (not SupportedLocale(locale))
        {
            continue;
        }

        PrepareValToUser(1000.5, locale);
        PrepareValToUser(-1000.5, locale);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::TestFormulaToUser()
{
    QFETCH(QString, input);
    QFETCH(QString, output);
    QFETCH(QLocale, locale);

    QLocale::setDefault(locale);

    const QString result = VTranslateVars::TryFormulaToUser(input, true);

    QCOMPARE(result, output);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::cleanupTestCase()
{
    delete m_trMs;
    QLocale::setDefault(m_systemLocale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::PrepareValFromUser(double d, const QLocale &locale)
{
    const QString formulaToSystem = QLocale::c().toString(d);
    const QString formulaFromUser = locale.toString(d);

    PrepareVal(formulaFromUser, formulaToSystem, locale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::PrepareValToUser(double d, const QLocale &locale)
{
    const QString formulaFromSystem = QLocale::c().toString(d);
    QString formulaToUser = locale.toString(d);
    if (LocaleGroupSeparator(locale).isSpace())
    {
        formulaToUser.replace(LocaleGroupSeparator(locale), QString());
    }

    PrepareVal(formulaFromSystem, formulaToUser, locale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTranslateVars::PrepareVal(const QString &inputFormula, const QString &outputFormula, const QLocale &locale)
{
    QString inputString = inputFormula;
    QString outputString = outputFormula;

    auto PREPARE_CASE = [locale](const QString &inputString, const QString &outputString)
    {
        auto const tag = QStringLiteral("%1. String '%2'").arg(locale.name(), inputString);
        QTest::newRow(qUtf8Printable(tag)) << inputString << outputString << locale;
    };

    PREPARE_CASE(inputString, outputString);

    inputString = inputFormula + '+'_L1 + inputFormula;
    outputString = outputFormula + '+'_L1 + outputFormula;
    PREPARE_CASE(inputString, outputString);

    inputString = inputFormula + "+a"_L1;
    outputString = outputFormula + "+a"_L1;
    PREPARE_CASE(inputString, outputString);
}
