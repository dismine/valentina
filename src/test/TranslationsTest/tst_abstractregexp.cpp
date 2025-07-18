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

#include "tst_abstractregexp.h"
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmuformulabase.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vtranslatevars.h"

#include <QTranslator>
#include <QtTest>

using namespace Qt::Literals::StringLiterals;

namespace
{
auto PrepareValidNameChars() -> QString
{
    qmu::QmuFormulaBase parser;
    parser.InitCharSets();
    parser.SetSepForTr(false, true);
    return parser.ValidNameChars();
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
TST_AbstractRegExp::TST_AbstractRegExp(const QString &locale, QObject *parent)
  : AbstractTest(parent),
    m_locale(locale),
    m_vTranslator(nullptr),
    m_trMs(nullptr)
{
}

//---------------------------------------------------------------------------------------------------------------------
TST_AbstractRegExp::~TST_AbstractRegExp()
{
    delete m_vTranslator.data();
    delete m_trMs;
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_AbstractRegExp::LoadVariables(const QString &checkedLocale) -> int
{
    const QString path = TranslationsPath();
    const QString file = u"valentina_%1.qm"_s.arg(checkedLocale);

    if (QFileInfo(path + '/'_L1 + file).size() <= 34)
    {
        qWarning("Translation variables for locale = %s is empty. \nFull path: %s/%s", qUtf8Printable(checkedLocale),
                 qUtf8Printable(path), qUtf8Printable(file));
        return ErrorSize;
    }

    m_vTranslator = new QTranslator(this);

    if (not m_vTranslator->load(file, path))
    {
        qWarning("Can't load translation variables for locale = %s. \nFull path: %s/%s", qUtf8Printable(checkedLocale),
                 qUtf8Printable(path), qUtf8Printable(file));
        delete m_vTranslator.data();
        return ErrorLoad;
    }

    if (not QCoreApplication::installTranslator(m_vTranslator))
    {
        qWarning("Can't install translation variables for locale = %s. \nFull path: %s/%s",
                 qUtf8Printable(checkedLocale), qUtf8Printable(path), qUtf8Printable(file));
        delete m_vTranslator.data();
        return ErrorInstall;
    }

    return NoError;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_AbstractRegExp::RemoveTrVariables(const QString &checkedLocale)
{
    if (not m_vTranslator.isNull())
    {
        const bool result = QCoreApplication::removeTranslator(m_vTranslator);

        if (result == false)
        {
            qWarning("Can't remove translation variables for locale = %s", qUtf8Printable(checkedLocale));
        }
        delete m_vTranslator.data();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_AbstractRegExp::InitTrMs()
{
    if (m_trMs != nullptr)
    {
        m_trMs->Retranslate();
    }
    else
    {
        m_trMs = new VTranslateVars();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_AbstractRegExp::CallTestCheckNoEndLine()
{
    QFETCH(QString, originalName);

    const QString translated = m_trMs->VarToUser(originalName);
    if (translated.endsWith('\n'_L1))
    {
        const QString message = u"Translated string '%1' shouldn't contain new line character."_s.arg(translated);
        QFAIL(qUtf8Printable(message));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_AbstractRegExp::CallTestCheckRegExpNames()
{
    QFETCH(QString, originalName);

    static const QRegularExpression re(NameRegExp());
    const QString translated = m_trMs->VarToUser(originalName);
    if (not re.match(translated).hasMatch())
    {
        const QString message = u"Original name:'%1', translated name:'%2'"_s.arg(originalName, translated);
        QFAIL(qUtf8Printable(message));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_AbstractRegExp::CallTestCheckNoOriginalNamesInTranslation()
{
    QFETCH(QString, originalName);

    static const QStringList originalNames = AllNames();
    static const auto names = ConvertToSet<QString>(originalNames);

    const QString translated = m_trMs->VarToUser(originalName);
    if (names.contains(translated) && originalName != translated)
    {
        const QString message = u"Translation repeat original name from other place. "
                                "Original name:'%1', translated name:'%2'"_s.arg(originalName, translated);
        QFAIL(qUtf8Printable(message));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_AbstractRegExp::CallTestForValidCharacters()
{
    QFETCH(QString, originalName);

    static const QString validNameChars = PrepareValidNameChars();

    if (QLocale() == QLocale(QStringLiteral("zh_CN")) || QLocale() == QLocale(QStringLiteral("he_IL"))
        || QLocale() == QLocale(QStringLiteral("ja_JP")))
    {
        const QString message =
            QStringLiteral("We do not support translation of variables for locale %1").arg(QLocale().name());
        QSKIP(qUtf8Printable(message));
    }

    const QString translated = m_trMs->VarToUser(originalName);
    const vsizetype pos = FindFirstNotOf(translated, validNameChars);
    if (pos != -1)
    {
        const auto message = QStringLiteral("Translated string '%1' contains invalid character '%2' at "
                                            "position '%3'.")
                                 .arg(translated)
                                 .arg(translated.at(pos))
                                 .arg(pos);
        QFAIL(qUtf8Printable(message));
    }
}
