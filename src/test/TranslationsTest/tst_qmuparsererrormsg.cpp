/************************************************************************
 **
 **  @file   tst_qmuparsererrormsg.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 10, 2015
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

#include "tst_qmuparsererrormsg.h"
#include "../vmisc/def.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_QmuParserErrorMsg::TST_QmuParserErrorMsg(const QString &locale, QObject *parent)
  : AbstractTest(parent),
    m_locale(locale),
    appTranslator(nullptr),
    msg(nullptr)
{
}

//---------------------------------------------------------------------------------------------------------------------
TST_QmuParserErrorMsg::~TST_QmuParserErrorMsg()
{
    delete appTranslator.data();
    delete msg;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuParserErrorMsg::initTestCase()
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

    QDir const dir(TranslationsPath());
    const QStringList fileNames = dir.entryList(QStringList("valentina_*.qm"));
    QVERIFY2(locales.size() == fileNames.size(), "Unexpected count of files.");

    if (LoadTranslation(m_locale) != NoError)
    {
        const QString message = u"Couldn't load variables. Locale = %1"_s.arg(m_locale);
        QSKIP(qUtf8Printable(message));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuParserErrorMsg::TestEErrorCodes_data()
{
    QTest::addColumn<int>("code");
    QTest::addColumn<bool>("tok");
    QTest::addColumn<bool>("pos");

    AddCase(qmu::ecUNASSIGNABLE_TOKEN, true, true);
    AddCase(qmu::ecINTERNAL_ERROR, false, false);
    AddCase(qmu::ecINVALID_NAME, true, false);
    AddCase(qmu::ecINVALID_BINOP_IDENT, true, false);
    AddCase(qmu::ecINVALID_INFIX_IDENT, true, false);
    AddCase(qmu::ecINVALID_POSTFIX_IDENT, true, false);
    AddCase(qmu::ecINVALID_FUN_PTR, false, false);
    AddCase(qmu::ecEMPTY_EXPRESSION, false, false);
    AddCase(qmu::ecINVALID_VAR_PTR, false, false);
    AddCase(qmu::ecUNEXPECTED_OPERATOR, true, true);
    AddCase(qmu::ecUNEXPECTED_EOF, false, true);
    AddCase(qmu::ecUNEXPECTED_ARG_SEP, false, true);
    AddCase(qmu::ecUNEXPECTED_PARENS, true, true);
    AddCase(qmu::ecUNEXPECTED_FUN, true, true);
    AddCase(qmu::ecUNEXPECTED_VAL, true, true);
    AddCase(qmu::ecUNEXPECTED_VAR, true, true);
    AddCase(qmu::ecUNEXPECTED_ARG, false, true);
    AddCase(qmu::ecMISSING_PARENS, false, false);
    AddCase(qmu::ecTOO_MANY_PARAMS, true, true);
    AddCase(qmu::ecTOO_FEW_PARAMS, true, true);
    AddCase(qmu::ecDIV_BY_ZERO, false, false);
    AddCase(qmu::ecDOMAIN_ERROR, false, false);
    AddCase(qmu::ecNAME_CONFLICT, false, false);
    AddCase(qmu::ecOPT_PRI, false, false);
    AddCase(qmu::ecBUILTIN_OVERLOAD, true, false);
    AddCase(qmu::ecUNEXPECTED_STR, false, true);
    AddCase(qmu::ecUNTERMINATED_STRING, false, true);
    AddCase(qmu::ecSTRING_EXPECTED, false, false);
    AddCase(qmu::ecVAL_EXPECTED, false, false);
    AddCase(qmu::ecOPRT_TYPE_CONFLICT, true, true);
    AddCase(qmu::ecSTR_RESULT, false, false);
    AddCase(qmu::ecGENERIC, false, false);
    AddCase(qmu::ecLOCALE, false, false);
    AddCase(qmu::ecUNEXPECTED_CONDITIONAL, true, false);
    AddCase(qmu::ecMISSING_ELSE_CLAUSE, false, false);
    AddCase(qmu::ecMISPLACED_COLON, false, true);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuParserErrorMsg::TestEErrorCodes()
{
    QFETCH(int, code);
    QFETCH(bool, tok);
    QFETCH(bool, pos);

    const QString translated = (*msg)[code];
    // cppcheck-suppress unreadVariable
    const auto message = QStringLiteral("String: '%1'.").arg(translated);

    QVERIFY2((translated.indexOf("$TOK$"_L1) != -1) == tok, qUtf8Printable(message));
    QVERIFY2((translated.indexOf("$POS$"_L1) != -1) == pos, qUtf8Printable(message));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuParserErrorMsg::cleanupTestCase()
{
    RemoveTranslation();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuParserErrorMsg::AddCase(int code, bool tok, bool pos)
{
    const QString tag = u"Check translation code=%1 in file valentina_%2.qm"_s.arg(code).arg(m_locale);
    QTest::newRow(qUtf8Printable(tag)) << code << tok << pos;
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_QmuParserErrorMsg::LoadTranslation(const QString &checkedLocale) -> int
{
    const QString path = TranslationsPath();
    const QString file = u"valentina_%1.qm"_s.arg(checkedLocale);

    if (QFileInfo(path + '/'_L1 + file).size() <= 34)
    {
        qWarning("Translation for locale = %s is empty. \nFull path: %s/%s", qUtf8Printable(checkedLocale),
                 qUtf8Printable(path), qUtf8Printable(file));
        return ErrorSize;
    }

    appTranslator = new QTranslator(this);

    if (not appTranslator->load(file, path))
    {
        qWarning("Can't load translation for locale = %s. \nFull path: %s/%s", qUtf8Printable(checkedLocale),
                 qUtf8Printable(path), qUtf8Printable(file));
        delete appTranslator.data();
        return ErrorLoad;
    }

    if (not QCoreApplication::installTranslator(appTranslator))
    {
        qWarning("Can't install translation for locale = %s. \nFull path: %s/%s", qUtf8Printable(checkedLocale),
                 qUtf8Printable(path), qUtf8Printable(file));
        delete appTranslator.data();
        return ErrorInstall;
    }

    delete msg;
    msg = new qmu::QmuParserErrorMsg(); // Very important do it after load QM file.

    return NoError;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_QmuParserErrorMsg::RemoveTranslation()
{
    if (not appTranslator.isNull())
    {
        const bool result = QCoreApplication::removeTranslator(appTranslator);

        if (result == false)
        {
            qWarning("Can't remove translation for locale = %s", qUtf8Printable(m_locale));
        }
        delete appTranslator.data();
    }
}
