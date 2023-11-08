/************************************************************************
 **
 **  @file   tst_nameregexp.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 5, 2015
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

#include "tst_nameregexp.h"
#include "../qmuparser/qmudef.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_NameRegExp::TST_NameRegExp(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_NameRegExp::TestNameRegExp_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("result");

    const QList<QLocale> allLocales =
        QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);

    for (const auto &locale : allLocales)
    {
        INIT_LOCALE_VARIABLES(locale);
        Q_UNUSED(positiveSign)
        Q_UNUSED(expUpper)
        Q_UNUSED(expLower)

        const QString localeName = locale.name();

        QString tag = localeName + ". First character can't be "_L1 + sign0;
        QTest::newRow(qUtf8Printable(tag)) << sign0 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign1;
        QTest::newRow(qUtf8Printable(tag)) << sign1 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign2;
        QTest::newRow(qUtf8Printable(tag)) << sign2 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign3;
        QTest::newRow(qUtf8Printable(tag)) << sign3 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign4;
        QTest::newRow(qUtf8Printable(tag)) << sign4 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign5;
        QTest::newRow(qUtf8Printable(tag)) << sign5 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign6;
        QTest::newRow(qUtf8Printable(tag)) << sign6 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign7;
        QTest::newRow(qUtf8Printable(tag)) << sign7 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign8;
        QTest::newRow(qUtf8Printable(tag)) << sign8 + 'a'_L1 << false;

        tag = localeName + ". First character can't be "_L1 + sign9;
        QTest::newRow(qUtf8Printable(tag)) << sign9 + 'a'_L1 << false;

        tag = localeName + ". First character can't be \""_L1 + negativeSign + '\"'_L1;
        QTest::newRow(qUtf8Printable(tag)) << negativeSign + 'a'_L1 << false;

        tag = localeName + ". First character can't be \""_L1 + decimalPoint + '\"'_L1;
        QTest::newRow(qUtf8Printable(tag)) << decimalPoint + 'a'_L1 << false;

        tag = localeName + ". First character can't be \""_L1 + groupSeparator + '\"'_L1;
        QTest::newRow(qUtf8Printable(tag)) << groupSeparator + 'a'_L1 << false;

        tag = localeName + ". Any next character can't be \""_L1 + negativeSign + '\"'_L1;
        QTest::newRow(qUtf8Printable(tag)) << 'a'_L1 + negativeSign << false;

        tag = localeName + ". Any next character can't be \""_L1 + decimalPoint + '\"'_L1;
        QTest::newRow(qUtf8Printable(tag)) << 'a'_L1 + decimalPoint << false;

        if (groupSeparator != '\'')
        {
            tag = localeName + ". Any next character can't be \""_L1 + groupSeparator + '\"'_L1;
            QTest::newRow(qUtf8Printable(tag)) << 'a'_L1 + groupSeparator << false;
        }
    }

    QTest::newRow("First character can't be \"+\"") << "+a" << false;
    QTest::newRow("First character can't be \"*\"") << "*a" << false;
    QTest::newRow("First character can't be \"/\"") << "/a" << false;
    QTest::newRow("First character can't be \"^\"") << "^a" << false;
    QTest::newRow("First character can't be \"=\"") << "=a" << false;
    QTest::newRow("First character can't be whitespace") << " a" << false;
    QTest::newRow("First character can't be \"(\"") << "(a" << false;
    QTest::newRow("First character can't be \")\"") << ")a" << false;
    QTest::newRow("First character can't be \"?\"") << "?a" << false;
    QTest::newRow("First character can't be \":\"") << ":a" << false;
    QTest::newRow("First character can't be \";\"") << ";a" << false;
    QTest::newRow("First character can't be \"'\"") << "'a" << false;
    QTest::newRow("First character can't be \"\"\"") << "\"a" << false;
    QTest::newRow("First character can't be \"&\"") << "&a" << false;
    QTest::newRow("First character can't be \"|\"") << "|a" << false;
    QTest::newRow("First character can't be \"!\"") << "!a" << false;
    QTest::newRow("First character can't be \"<\"") << "<a" << false;
    QTest::newRow("First character can't be \">\"") << ">a" << false;

    QTest::newRow("First character can be \"\\\"") << "\\a" << true;

    QTest::newRow("Any next character can't be \"+\"") << "a+" << false;
    QTest::newRow("Any next character can't be \"*\"") << "a*" << false;
    QTest::newRow("Any next character can't be \"/\"") << "a/" << false;
    QTest::newRow("Any next character can't be \"^\"") << "a^" << false;
    QTest::newRow("Any next character can't be \"=\"") << "a=" << false;
    QTest::newRow("Any next character can't be whitespace") << "L bust" << false;
    QTest::newRow("Any next character can't be \"(\"") << "a(" << false;
    QTest::newRow("Any next character can't be \")\"") << "a)" << false;
    QTest::newRow("Any next character can't be \"?\"") << "a?" << false;
    QTest::newRow("Any next character can't be \":\"") << "a:" << false;
    QTest::newRow("Any next character can't be \";\"") << "a;" << false;
    QTest::newRow("Any next character can't be \"\"\"") << "a\"" << false;
    QTest::newRow("Any next character can't be \"&\"") << "a&" << false;
    QTest::newRow("Any next character can't be \"|\"") << "a|" << false;
    QTest::newRow("Any next character can't be \"!\"") << "a!" << false;
    QTest::newRow("Any next character can't be \"<\"") << "a<" << false;
    QTest::newRow("Any next character can't be \">\"") << "a>" << false;

    QTest::newRow("Any next character can be \"\\\"") << "a\\" << true;

    QTest::newRow("Good name \"p12\"") << "p12" << true;
    QTest::newRow("Good name \"height\"") << "height" << true;
    QTest::newRow("Good name \"A_1\"") << "A_1" << true;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_NameRegExp::TestNameRegExp()
{
    const QRegularExpression re(NameRegExp());

    QFETCH(QString, name);
    QFETCH(bool, result);

    QCOMPARE(re.match(name).hasMatch(), result);
}
