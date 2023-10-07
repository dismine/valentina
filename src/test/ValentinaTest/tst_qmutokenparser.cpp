/************************************************************************
 **
 **  @file   tst_qmutokenparser.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2015
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

#include "tst_qmutokenparser.h"
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmutokenparser.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_QmuTokenParser::TST_QmuTokenParser(QObject *parent)
  : QObject(parent),
    m_systemLocale(QLocale::system())
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuTokenParser::IsSingle_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<bool>("result");

    QTest::newRow("Empty string") << QString() << false;
    QTest::newRow("Single value") << "15.5" << true;
    QTest::newRow("Two digits") << "2+2" << false;
    QTest::newRow("Two digits") << "2-2" << false;
    QTest::newRow("Negative single value") << "-2" << true;
    QTest::newRow("Digit and variable") << "2+a" << false;
    QTest::newRow("One variable twice") << "a+a" << false;
    QTest::newRow("Two variables") << "a+b" << false;
    QTest::newRow("Empty string") << QString() << false;
    QTest::newRow("Several spaces") << "   " << false;
    QTest::newRow("Invalid formula") << "2*)))" << false;
    QTest::newRow("Invalid formula") << "2*" << false;
    QTest::newRow("Incorrect thousand separator 15 500") << "15 500" << false;
    QTest::newRow("Correct C locale 15500") << "15500" << true;
    QTest::newRow("Correct C locale 15,500") << "15,500" << true;
    QTest::newRow("Correct C locale 15,500.1") << "15,500.1" << true;
    QTest::newRow("Correct C locale 15500.1") << "15500.1" << true;
    QTest::newRow("Not C locale 15,5") << "15,5" << false;
    QTest::newRow("Not C locale 15.500,1") << "15.500,1" << false;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuTokenParser::IsSingle()
{
    QFETCH(QString, formula);
    QFETCH(bool, result);

    QCOMPARE(qmu::QmuTokenParser::IsSingle(formula), result);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuTokenParser::TokenFromUser_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<bool>("result");
    QTest::addColumn<QLocale>("locale");

    const QList<QLocale> allLocales =
        QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const auto &locale : allLocales)
    {
        if (not SupportedLocale(locale))
        {
            continue;
        }
        PrepareVal(1000.5, locale);
        PrepareVal(-1000.5, locale);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuTokenParser::TokenFromUser()
{
    QFETCH(QString, formula);
    QFETCH(bool, result);
    QFETCH(QLocale, locale);

    QLocale::setDefault(locale);

    QCOMPARE(IsSingleFromUser(formula), result);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuTokenParser::cleanupTestCase()
{
    QLocale::setDefault(m_systemLocale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuTokenParser::PrepareVal(qreal val, const QLocale &locale)
{
    const QString formula = locale.toString(val);
    QString string = formula;
    QString tag = u"%1. String '%2'"_s.arg(locale.name(), string);
    QTest::newRow(qUtf8Printable(tag)) << string << true << locale;

    string = formula + '+'_L1;
    tag = u"%1. String '%2'"_s.arg(locale.name(), string);
    QTest::newRow(qUtf8Printable(tag)) << string << false << locale;

    string = formula + '+'_L1 + formula;
    tag = u"%1. String '%2'"_s.arg(locale.name(), string);
    QTest::newRow(qUtf8Printable(tag)) << string << false << locale;

    string = formula + u"+б"_s;
    tag = u"%1. String '%2'"_s.arg(locale.name(), string);
    QTest::newRow(qUtf8Printable(tag)) << string << false << locale;
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_QmuTokenParser::IsSingleFromUser(const QString &formula) -> bool
{
    if (formula.isEmpty())
    {
        return false; // if don't know say no
    }

    QMap<qmusizetype, QString> tokens;
    QMap<qmusizetype, QString> numbers;

    try
    {
        QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, true, true));
        tokens = cal->GetTokens();   // Tokens (variables, measurements)
        numbers = cal->GetNumbers(); // All numbers in expression
    }
    catch (const qmu::QmuParserError &e)
    {
        Q_UNUSED(e)
        return false; // something wrong with formula, say no
    }

    // Remove "-" from tokens list if exist. If don't do that unary minus operation will broken.
    qmu::QmuFormulaBase::RemoveAll(tokens, LocaleNegativeSign(QLocale()));

    return tokens.isEmpty() && numbers.size() == 1;
}
