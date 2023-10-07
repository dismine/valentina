/************************************************************************
 **
 **  @file   tst_measurementregexp.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2015
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

#include "tst_measurementregexp.h"

#include "../vpatterndb/measurements.h"
#include "../vpatterndb/vtranslatevars.h"
#include "abstracttest.h"

#include <QTranslator>
#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
auto InitializePmSystems() noexcept -> QStringList
{
    return {"p0",  "p1",  "p2",  "p3",  "p4",  "p5",  "p6",  "p7",  "p8",  "p9",  "p10", "p11", "p12", "p13",
            "p14", "p15", "p16", "p17", "p18", "p19", "p20", "p21", "p22", "p23", "p24", "p25", "p26", "p27",
            "p28", "p29", "p30", "p31", "p32", "p33", "p34", "p35", "p36", "p37", "p38", "p39", "p40", "p41",
            "p42", "p43", "p44", "p45", "p46", "p47", "p48", "p49", "p50", "p51", "p52", "p53", "p54", "p998"};
}
} // namespace

const QStringList TST_MeasurementRegExp::pmSystems = InitializePmSystems();

//---------------------------------------------------------------------------------------------------------------------
TST_MeasurementRegExp::TST_MeasurementRegExp(const QString &systemCode, const QString &locale, QObject *parent)
  : TST_AbstractRegExp(locale, parent),
    m_systemCode(systemCode),
    m_pmsTranslator(nullptr)
{
}

//---------------------------------------------------------------------------------------------------------------------
TST_MeasurementRegExp::~TST_MeasurementRegExp()
{
    delete m_pmsTranslator;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::initTestCase()
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

    TestCombinations(locales);

    const int res = LoadMeasurements(m_systemCode, m_locale);
    if (res != NoError)
    {
        const QString message = u"Couldn't load measurements. System = %1, locale = %2"_s.arg(m_systemCode, m_locale);

        if (res == ErrorMissing)
        {
            QFAIL(qUtf8Printable(message));
        }
    }

    if (LoadVariables(m_locale) != NoError)
    {
        const QString message = u"Couldn't load variables. System = %1, locale = %2"_s.arg(m_systemCode, m_locale);
        QSKIP(qUtf8Printable(message));
    }

    QLocale::setDefault(QLocale(m_locale));

    InitTrMs(); // Very important do this after loading QM files.
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoEndLine_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoEndLine()
{
    CallTestCheckNoEndLine();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckRegExpNames_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckRegExpNames()
{
    CallTestCheckRegExpNames();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckIsNamesUnique_data()
{
    QTest::addColumn<QString>("translatedName");
    QTest::addColumn<QStringList>("originalNames");

    const QStringList originalNames = AllNames();
    QMultiMap<QString, QString> names;
    for (const auto &originalName : originalNames)
    {
        names.insert(m_trMs->VarToUser(originalName), originalName);
    }

    QList<QString> keys = names.uniqueKeys();
    for (const auto &key : keys)
    {
        const QString tag = u"System: '%1', locale: '%2'. Name '%3'"_s.arg(m_systemCode, m_locale, key);
        QTest::newRow(qUtf8Printable(tag)) << key << QStringList(names.values(key));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckIsNamesUnique()
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
void TST_MeasurementRegExp::TestCheckNoOriginalNamesInTranslation_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoOriginalNamesInTranslation()
{
    CallTestCheckNoOriginalNamesInTranslation();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestForValidChars_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestForValidChars()
{
    CallTestForValidCharacters();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::cleanupTestCase()
{
    RemoveTrMeasurements(m_systemCode, m_locale);
    RemoveTrVariables(m_locale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCombinations(const QStringList &locales) const
{
    const vsizetype combinations = TST_MeasurementRegExp::pmSystems.size() * locales.size();

    QDir dir(TranslationsPath());
    const QStringList fileNames = dir.entryList(QStringList("measurements_p*_*.qm"));

    // cppcheck-suppress unreadVariable
    const QString error = u"Unexpected count of files. Excpected %1, got %2."_s.arg(combinations).arg(fileNames.size());
    QVERIFY2(combinations == fileNames.size(), qUtf8Printable(error));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::PrepareData()
{
    static const QStringList originalNames = AllNames();

    QTest::addColumn<QString>("originalName");

    for (const auto &str : originalNames)
    {
        const QString tag = u"System: '%1', locale: '%2'. Name '%3'"_s.arg(m_systemCode, m_locale, str);
        QTest::newRow(qUtf8Printable(tag)) << str;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_MeasurementRegExp::AllNames() -> QStringList
{
    return AllGroupNames();
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_MeasurementRegExp::LoadMeasurements(const QString &checkedSystem, const QString &checkedLocale) -> int
{
    const QString path = TranslationsPath();
    const QString file = QStringLiteral("measurements_%1_%2.qm").arg(checkedSystem, checkedLocale);

    QFileInfo info(path + '/'_L1 + file);

    if (not info.exists())
    {
        const QString message =
            u"File for translation for system = %1 and locale = %2 doesn't exists. \nFull path: %3/%4"_s.arg(
                checkedSystem, checkedLocale, path, file);
        QWARN(qUtf8Printable(message));

        return ErrorMissing;
    }

    if (QFileInfo(path + '/'_L1 + file).size() <= 34)
    {
        const QString message = u"Translation for system = %1 and locale = %2 is empty. \nFull path: %3/%4"_s.arg(
            checkedSystem, checkedLocale, path, file);
        QWARN(qUtf8Printable(message));

        return ErrorSize;
    }

    delete m_pmsTranslator;
    m_pmsTranslator = new QTranslator(this);

    if (not m_pmsTranslator->load(file, path))
    {
        const QString message = u"Can't load translation for system = %1 and locale = %2. \nFull path: %3/%4"_s.arg(
            checkedSystem, checkedLocale, path, file);
        QWARN(qUtf8Printable(message));

        delete m_pmsTranslator;

        return ErrorLoad;
    }

    if (not QCoreApplication::installTranslator(m_pmsTranslator))
    {
        const QString message = u"Can't install translation for system = %1 and locale = %2. \nFull path: %3/%4"_s.arg(
            checkedSystem, checkedLocale, path, file);
        QWARN(qUtf8Printable(message));

        delete m_pmsTranslator;

        return ErrorInstall;
    }

    return NoError;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::RemoveTrMeasurements(const QString &checkedSystem, const QString &checkedLocale)
{
    if (not m_pmsTranslator.isNull())
    {
        const bool result = QCoreApplication::removeTranslator(m_pmsTranslator);

        if (result == false)
        {
            const QString message =
                u"Can't remove translation for system = %1 and locale = %2"_s.arg(checkedSystem, checkedLocale);
            QWARN(qUtf8Printable(message));
        }
        delete m_pmsTranslator;
    }
}
