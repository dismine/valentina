/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 10, 2016
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

#include "tst_buitinregexp.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/literals.h"
#include "../vpatterndb/vtranslatevars.h"

#include <QTranslator>
#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_BuitInRegExp::TST_BuitInRegExp(const QString &locale, QObject *parent)
  : TST_AbstractRegExp(locale, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::initTestCase()
{
    if (m_locale.isEmpty())
    {
        QFAIL("Empty locale code.");
    }

    const QStringList locales = SupportedLocales();

    if (not locales.contains(m_locale))
    {
        QFAIL("Unsupported locale code.");
    }

    if (LoadVariables(m_locale) != NoError)
    {
        const QString message = u"Couldn't load variables. Locale = %1"_s.arg(m_locale);
        QSKIP(qUtf8Printable(message));
    }

    QLocale::setDefault(QLocale(m_locale));

    InitTrMs(); // Very important do this after loading QM files.
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckNoEndLine_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckNoEndLine()
{
    CallTestCheckNoEndLine();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckRegExpNames_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckRegExpNames()
{
    CallTestCheckRegExpNames();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckIsNamesUnique_data()
{
    const QStringList originalNames = AllNames();
    QMultiMap<QString, QString> names;
    for (const auto &originalName : originalNames)
    {
        names.insert(m_trMs->VarToUser(originalName), originalName);
    }

    QTest::addColumn<QString>("translatedName");
    QTest::addColumn<QStringList>("originalNames");

    QList<QString> keys = names.uniqueKeys();
    for (const auto &key : keys)
    {
        const QString tag = u"Locale: '%1'. Name '%2'"_s.arg(m_locale, key);
        QTest::newRow(qUtf8Printable(tag)) << key << QStringList(names.values(key));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckIsNamesUnique()
{
    QFETCH(QString, translatedName);
    QFETCH(QStringList, originalNames);

    if (originalNames.size() > 1)
    {
        const QString message = u"Name is not unique. Translated name:'%1' also assosiated with: %2."_s.arg(
            translatedName, originalNames.join(", "));
        QFAIL(qUtf8Printable(message));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckNoOriginalNamesInTranslation_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckNoOriginalNamesInTranslation()
{
    CallTestCheckNoOriginalNamesInTranslation();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckUnderlineExists_data()
{
    QMap<QString, bool> data;

    data.insert(measurement_, true);
    data.insert(increment_, true);
    data.insert(line_, true);
    data.insert(angleLine_, true);
    data.insert(arc_, true);
    data.insert(elarc_, true);
    data.insert(spl_, true);
    data.insert(splPath, false);
    data.insert(radiusArc_, true);
    data.insert(radius1ElArc_, true);
    data.insert(radius2ElArc_, true);
    data.insert(angle1Arc_, true);
    data.insert(angle2Arc_, true);
    data.insert(angle1ElArc_, true);
    data.insert(angle2ElArc_, true);
    data.insert(angle1Spl_, true);
    data.insert(angle2Spl_, true);
    data.insert(angle1SplPath, false);
    data.insert(angle2SplPath, false);
    data.insert(seg_, true);
    data.insert(currentLength, false);
    data.insert(currentSeamAllowance, false);
    data.insert(c1LengthSpl_, true);
    data.insert(c2LengthSpl_, true);
    data.insert(c1LengthSplPath, false);
    data.insert(c2LengthSplPath, false);
    data.insert(rotationElArc_, true);
    data.insert(pieceArea_, true);
    data.insert(pieceSeamLineArea_, true);

    // Catch case when new internal variable appears.
    QCOMPARE(data.size(), BuilInVariables().size());

    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("exists");

    auto i = data.constBegin();
    while (i != data.constEnd())
    {
        const QString tag = u"Locale: '%1'. Name '%2'"_s.arg(m_locale, i.key());
        QTest::newRow(qUtf8Printable(tag)) << i.key() << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckUnderlineExists()
{
    QFETCH(QString, name);
    QFETCH(bool, exists);

    const QString translated = m_trMs->InternalVarToUser(name);
    if ((translated.right(1) == '_'_L1) != exists)
    {
        const QString message =
            u"String '%1' doesn't contain underline. Original string is '%2'"_s.arg(translated, name);
        QFAIL(qUtf8Printable(message));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckInternalVaribleRegExp_data()
{
    QTest::addColumn<QString>("var");
    QTest::addColumn<QString>("originalName");

    for (const auto &var : BuilInVariables())
    {
        const QString tag = u"Locale: '%1'. Var '%2'"_s.arg(m_locale, var);
        const QStringList originalNames = AllNames();
        for (const auto &str : originalNames)
        {
            QTest::newRow(qUtf8Printable(tag)) << var << str;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestCheckInternalVaribleRegExp()
{
    QFETCH(QString, var);
    QFETCH(QString, originalName);

    static const QString regex = QStringLiteral("(.){1,}_(.){1,}$");

    const QString sourceRegex = '^'_L1 + var + regex;
    const QRegularExpression sourceRe(sourceRegex);

    const QString translated = m_trMs->InternalVarToUser(var);
    const QString translationRegex = '^'_L1 + translated + regex;
    const QRegularExpression translationRe(translationRegex);

    {
        if (sourceRe.match(originalName).hasMatch() || translationRe.match(originalName).hasMatch())
        {
            const QString message = QStringLiteral("Invalid original string '%1'").arg(originalName);
            QFAIL(qUtf8Printable(message));
        }

        const QString translatedMessage = m_trMs->VarToUser(originalName);
        if (sourceRe.match(translatedMessage).hasMatch() || translationRe.match(translatedMessage).hasMatch())
        {
            const QString message = QStringLiteral("Invalid translation string '%1'").arg(translatedMessage);
            QFAIL(qUtf8Printable(message));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestForValidChars_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::TestForValidChars()
{
    CallTestForValidCharacters();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::cleanupTestCase()
{
    RemoveTrVariables(m_locale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_BuitInRegExp::PrepareData()
{
    static const QStringList originalNames = AllNames();

    QTest::addColumn<QString>("originalName");

    for (const auto &str : originalNames)
    {
        const QString tag = u"Locale: '%1'. Name '%2'"_s.arg(m_locale, str);
        QTest::newRow(qUtf8Printable(tag)) << str;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_BuitInRegExp::AllNames() -> QStringList
{
    return BuilInFunctions() + BuilInVariables();
}
