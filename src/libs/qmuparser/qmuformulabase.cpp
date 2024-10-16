/***************************************************************************************************
 **
 **  Copyright (C) 2015 Roman Telezhynskyi
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#include "qmuformulabase.h"

#include <QChar>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QStringList>

#include "qmudef.h"

namespace qmu
{

//---------------------------------------------------------------------------------------------------------------------
QmuFormulaBase::QmuFormulaBase()
  : QmuParser()
{
}

//---------------------------------------------------------------------------------------------------------------------
QmuFormulaBase::~QmuFormulaBase()
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief InitCharSets init character set for parser.
 *
 * QMuParser require setting character set for legal characters. Because we try make our expresion language independent
 * we set all posible unique characters from all alphabets.
 *
 */
void QmuFormulaBase::InitCharSets()
{
    QString symbolsStr;
    {
        // String with all unique symbols for supported alphabets.
        // See script alphabets.py for generation and more information.
        // Note. MSVC doesn't support normal string concatenation for long string. That's why we use QStringList in this
        // case.
        const QStringList symbols{
            QStringLiteral(
                "ցЀĆЈVӧĎАғΕĖӅИқΝĞơРңњΥĦШҫ̆έجگĮаҳѕεشԶиһνԾрÃυلՆӝшËύՎїPÓՖXӛӟŞãզhëҔծpóӞնxßվāŁЃֆĉЋΊCŬđҐГΒęҘЛΚŘġҠУGا"),
            QStringLiteral("հЫΪŪدԱҰгβطԹõлκKՁÀуςهՉÈыvیՑÐSOřӘћաőcӐթèkàѓżűðsķչøӥӔĀփӣІĈΏЎґĐΗЖҙĘȚΟОҡĠآΧЦتίЮұİزηжԸغẽοоÁՀقχц"),
            QStringLiteral(
                "ÉՈيюÑՐђӋіәťӆўáŠĺѐfөըnñŰӤӨӹոľЁրăЉŭċΌБӸēłΔҖЙŤěΜӜDСձģΤӰЩīņحάҮбưԳصδHйԻŇμӴсՃمτƠщՋόєLQŹՓŕÖYśÞaգĽ"),
            QStringLiteral(
                "æiŽիӓîqճöyջþĂօЄӦΉĊЌΑĒДҗјΙȘĚМΡéĵĢФūӚΩبĪЬүќαذԲдҷιظԺмρՂфÇωوՊьÏՒTŚĻJբdçժlïӪղtպӫAւąЇΆčŃЏΎĕӯЗΖEțŮ"),
            QStringLiteral(
                "ĝПΞأĥĹЧΦثÆӳЯήIسŲԵзζԽпξكՅÄчφNMՍӌяώӢӲՕÔWÎŝÜџёźեägխoӒյôwĶBžսüЂĄև̈ЊČƏљΓВҕĔӮΛКĜΣТҥĤکΫЪƯخγвŅԴϊضλкԼ"),
            QStringLiteral(
                "ĴσтÅՄنϋъÍՌRӕՔZÝŜbåդﻩjíլļrӵմӱzýռپêЅքćچΈЍďΐҒЕůėژșΘØҚНğńءΠFҢХħΨҪЭųįҶرΰҲеԷňعθҺнԿفπÂхՇψÊэšՏÒUəÚѝ"),
            QStringLiteral("ŻşҤӑâeէŐımկòuշÕúտŔ")};

        symbolsStr = symbols.join(QString());
    }

    INIT_LOCALE_VARIABLES(m_locale);
    Q_UNUSED(expUpper)
    Q_UNUSED(expLower)
    Q_UNUSED(decimalPoint)
    Q_UNUSED(groupSeparator)

    // Together with localized symbols, we always must include canonical symbols
    auto AddNotCanonical = [](QString &chars, const QChar &c, const QChar &canonical)
    {
        if (c != canonical)
        {
            chars += c;
        }
    };

    // Defining identifier character sets
    auto nameChars = QStringLiteral("0123456789\\_@#'") + symbolsStr;

    AddNotCanonical(nameChars, sign0, '0');
    AddNotCanonical(nameChars, sign1, '1');
    AddNotCanonical(nameChars, sign2, '2');
    AddNotCanonical(nameChars, sign3, '3');
    AddNotCanonical(nameChars, sign4, '4');
    AddNotCanonical(nameChars, sign5, '5');
    AddNotCanonical(nameChars, sign6, '6');
    AddNotCanonical(nameChars, sign7, '7');
    AddNotCanonical(nameChars, sign8, '8');
    AddNotCanonical(nameChars, sign9, '9');

    DefineNameChars(nameChars);

    const auto opChars = QStringLiteral("+-*^/?<>=!$%&|~'_");

    QString oprtChars = symbolsStr + opChars;
    AddNotCanonical(oprtChars, positiveSign, '+');
    AddNotCanonical(oprtChars, negativeSign, '-');

    DefineOprtChars(oprtChars);

    QString infixOprtChars = opChars;

    AddNotCanonical(infixOprtChars, positiveSign, '+');
    AddNotCanonical(infixOprtChars, negativeSign, '-');

    DefineInfixOprtChars(infixOprtChars);
}

//---------------------------------------------------------------------------------------------------------------------
// Factory function for creating new parser variables
// This could as well be a function performing database queries.
auto QmuFormulaBase::AddVariable(const QString &a_szName, void *a_pUserData) -> qreal *
{
    Q_UNUSED(a_szName)
    Q_UNUSED(a_pUserData)

    static qreal value = 0;
    return &value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSepForTr set separators for translation expression.
 * @param fromUser true if expression come from user (from dialog).
 */
void QmuFormulaBase::SetSepForTr(bool osSeparator, bool fromUser)
{
    if (fromUser)
    {
        const QLocale loc;
        setLocale(loc);
        SetArgSep(';');
        setCNumbers(not osSeparator);
        if (osSeparator)
        {
            setDecimalPoint(LocaleDecimalPoint(loc));
            setThousandsSeparator(LocaleGroupSeparator(loc));
            return;
        }
    }

    SetSepForEval(); // Same separators (internal) as for eval.
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSepForEval set separators for eval. Each expression eval in internal (C) locale.
 */
void QmuFormulaBase::SetSepForEval()
{
    SetArgSep(';');
    setThousandsSeparator(',');
    setDecimalPoint('.');
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveAll remove token from token list.
 *
 * Standard Qt class QMap doesn't have method RemoveAll.
 * Example: remove "-" from tokens list if exist. If don't do that unary minus operation will broken.
 *
 * @param map map with tokens
 * @param val token that need delete
 */
void QmuFormulaBase::RemoveAll(QMap<qmusizetype, QString> &map, const QString &val)
{
    const QList<qmusizetype> listKeys = map.keys(val); // Take all keys that contain token.
    for (auto key : listKeys)
    {
        map.remove(key);
    }
}

} // namespace qmu
