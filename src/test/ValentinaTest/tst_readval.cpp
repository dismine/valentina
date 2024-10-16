/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 12, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "tst_readval.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/compatibility.h"

#include <QtTest>
#include <limits>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_ReadVal::TST_ReadVal(QObject *parent)
  : QObject(parent),
    m_systemLocale(QLocale::system())
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::TestReadVal_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<int>("expCount");
    QTest::addColumn<qreal>("expVal");
    QTest::addColumn<QLocale>("locale");

    const QList<QLocale> allLocales =
        QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const auto &locale : allLocales)
    {
        if (not SupportedLocale(locale))
        {
            continue;
        }

        PrepareVal(1., locale);
        PrepareVal(1.0, locale);
        PrepareVal(-1.0, locale);
        PrepareVal(1.5, locale);
        PrepareVal(-1.5, locale);
        PrepareVal(1000.0, locale);
        PrepareVal(-1000.0, locale);
        PrepareVal(1000.5, locale);
        PrepareVal(-1000.5, locale);
        PrepareVal(std::numeric_limits<double>::max(), locale);
        PrepareVal(-std::numeric_limits<double>::max(), locale);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::TestReadVal()
{
    TestVal();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::TestInvalidData_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<int>("expCount");
    QTest::addColumn<qreal>("expVal");
    QTest::addColumn<QLocale>("locale");

    // Test invalid values
    const QLocale locale = QLocale::c();
    PrepareString(QString(), locale);
    PrepareString(u"-1.000.5"_s, locale);
    PrepareString(u"1.000.5"_s, locale);
    PrepareString(u"-1.000,5"_s, locale);
    PrepareString(u"1.000,5"_s, locale);
    PrepareString(u"-1.0.00,5"_s, locale);
    PrepareString(u"1.0.00,5"_s, locale);
    PrepareString(u"7,5"_s, locale);
    PrepareString(u"-7,5"_s, locale);
    PrepareString(u"- 7,5"_s, locale);
    PrepareString(u"- 7.5"_s, locale);
    PrepareString(u"1,0,00.5"_s, locale);
    PrepareString(u"1,,000.5"_s, locale);
    PrepareString(u",5"_s, locale);
    PrepareString(QChar(','), locale);
    PrepareString(QChar('.'), locale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::TestInvalidData()
{
    TestVal();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::cleanupTestCase()
{
    QLocale::setDefault(m_systemLocale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::PrepareVal(qreal val, const QLocale &locale)
{
    const QString string = locale.toString(val);
    bool ok = false;
    const double d = locale.toDouble(string, &ok);
    PrepareString(string, locale, d, string.size());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::PrepareString(const QString &str, const QLocale &locale, qreal val, vsizetype count)
{
    const auto tag = QStringLiteral("%1. String '%2'").arg(locale.name(), str);
    QTest::newRow(qUtf8Printable(tag)) << str << count << val << locale;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ReadVal::TestVal()
{
    QFETCH(QString, formula);
    QFETCH(int, expCount);
    QFETCH(qreal, expVal);
    QFETCH(QLocale, locale);

    qreal resVal = 0;
    QLocale::setDefault(locale);

    const vsizetype resCount =
        ReadVal(formula, resVal, locale, LocaleDecimalPoint(locale), LocaleGroupSeparator(locale));

    // cppcheck-suppress unreadVariable
    auto errorMsg = QStringLiteral("Conversion failed. Locale: '%1'.").arg(locale.name());
    QVERIFY2(resCount == expCount, qUtf8Printable(errorMsg));

    if (resCount != -1)
    {
        // cppcheck-suppress unreadVariable
        errorMsg = QStringLiteral("Unexpected result. Locale: '%1'.").arg(locale.name());
        QVERIFY2(QmuFuzzyComparePossibleNulls(resVal, expVal), qUtf8Printable(errorMsg));
    }
}
