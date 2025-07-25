/***************************************************************************************************
 **
 **  Copyright (C) 2013 Ingo Berg
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

#include "qmuparsertokenreader.h"

#include <QList>
#include <QMessageLogger>
#include <QStringList>
#include <QtDebug>
#include <cassert>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "qmudef.h"
#include "qmuparserbase.h"

/**
 * @file
 * @brief This file contains the parser token reader implementation.
 */

namespace qmu
{

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Copy constructor.
 *
 * @sa Assign
 * @throw None No exceptions are thrown.
 */
QmuParserTokenReader::QmuParserTokenReader(const QmuParserTokenReader &a_Reader)
  : m_pParser(a_Reader.m_pParser),
    m_strFormula(a_Reader.m_strFormula),
    m_iPos(a_Reader.m_iPos),
    m_iSynFlags(a_Reader.m_iSynFlags),
    m_bIgnoreUndefVar(a_Reader.m_bIgnoreUndefVar),
    m_pFunDef(a_Reader.m_pFunDef),
    m_pPostOprtDef(a_Reader.m_pPostOprtDef),
    m_pInfixOprtDef(a_Reader.m_pInfixOprtDef),
    m_pOprtDef(a_Reader.m_pOprtDef),
    m_pConstDef(a_Reader.m_pConstDef),
    m_pStrVarDef(a_Reader.m_pStrVarDef),
    m_pVarDef(a_Reader.m_pVarDef),
    m_pFactory(a_Reader.m_pFactory),
    m_pFactoryData(a_Reader.m_pFactoryData),
    m_vIdentFun(a_Reader.m_vIdentFun),
    m_UsedVar(a_Reader.m_UsedVar),
    m_iBrackets(a_Reader.m_iBrackets),
    m_cArgSep(a_Reader.m_cArgSep)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Assignement operator.
 *
 * Self assignement will be suppressed otherwise #Assign is called.
 *
 * @param a_Reader Object to copy to this token reader.
 * @throw None No exceptions are thrown.
 */
auto QmuParserTokenReader::operator=(const QmuParserTokenReader &a_Reader) -> QmuParserTokenReader &
{
    if (&a_Reader != this)
    {
        Assign(a_Reader);
    }

    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Assign state of a token reader to this token reader.
 *
 * @param a_Reader Object from which the state should be copied.
 * @throw None No exceptions are thrown.
 */
void QmuParserTokenReader::Assign(const QmuParserTokenReader &a_Reader)
{
    m_pParser = a_Reader.m_pParser;
    m_strFormula = a_Reader.m_strFormula;
    m_iPos = a_Reader.m_iPos;
    m_iSynFlags = a_Reader.m_iSynFlags;

    m_UsedVar = a_Reader.m_UsedVar;
    m_pFunDef = a_Reader.m_pFunDef;
    m_pConstDef = a_Reader.m_pConstDef;
    m_pVarDef = a_Reader.m_pVarDef;
    m_pStrVarDef = a_Reader.m_pStrVarDef;
    m_pPostOprtDef = a_Reader.m_pPostOprtDef;
    m_pInfixOprtDef = a_Reader.m_pInfixOprtDef;
    m_pOprtDef = a_Reader.m_pOprtDef;
    m_bIgnoreUndefVar = a_Reader.m_bIgnoreUndefVar;
    m_vIdentFun = a_Reader.m_vIdentFun;
    m_pFactory = a_Reader.m_pFactory;
    m_pFactoryData = a_Reader.m_pFactoryData;
    m_iBrackets = a_Reader.m_iBrackets;
    m_cArgSep = a_Reader.m_cArgSep;

    m_fZero = 0;
    m_lastTok = token_type();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Constructor.
 *
 * Create a Token reader and bind it to a parser object.
 *
 * @pre [assert] a_pParser may not be NULL
 * @post #m_pParser==a_pParser
 * @param a_pParent Parent parser object of the token reader.
 */
QmuParserTokenReader::QmuParserTokenReader(QmuParserBase *a_pParent)
  : m_pParser(a_pParent)
{
    assert(m_pParser);
    SetParent(m_pParser);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create instance of a QParserTokenReader identical with this and return its pointer.
 *
 * This is a factory method the calling function must take care of the object destruction.
 *
 * @return A new QParserTokenReader object.
 * @throw None No exceptions are thrown.
 */
auto QmuParserTokenReader::Clone(QmuParserBase *a_pParent) const -> QmuParserTokenReader *
{
    std::unique_ptr<QmuParserTokenReader> ptr(new QmuParserTokenReader(*this));
    ptr->SetParent(a_pParent);
    return ptr.release();
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParserTokenReader::SaveBeforeReturn(const token_type &tok) -> QmuParserTokenReader::token_type &
{
    m_lastTok = tok;
    return m_lastTok;
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserTokenReader::AddValIdent(identfun_type a_pCallback)
{
    // Use push_front is used to give user defined callbacks a higher priority than
    // the built in ones. Otherwise reading hex numbers would not work
    // since the "0" in "0xff" would always be read first making parsing of
    // the rest impossible.
    // reference:
    // http://sourceforge.net/projects/muparser/forums/forum/462843/topic/4824956
    m_vIdentFun.push_front(a_pCallback);
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserTokenReader::SetVarCreator(facfun_type a_pFactory, void *pUserData)
{
    m_pFactory = a_pFactory;
    m_pFactoryData = pUserData;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the token Reader.
 *
 *   Sets the formula position index to zero and set Syntax flags to default for initial formula parsing.
 *   @pre [assert] triggered if a_szFormula==0
 */
void QmuParserTokenReader::SetFormula(const QString &a_strFormula)
{
    m_strFormula = a_strFormula;
    ReInit();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reset the token reader to the start of the formula.
 *
 * The syntax flags will be reset to a value appropriate for the start of a formula.
 * @post #m_iPos==0, #m_iSynFlags = noOPT | noBC | noPOSTOP | noSTR
 * @throw None No exceptions are thrown.
 * @sa ESynCodes
 */
void QmuParserTokenReader::ReInit()
{
    m_iPos = 0;
    m_iSynFlags = sfSTART_OF_LINE;
    m_iBrackets = 0;
    m_UsedVar.clear();
    m_lastTok = token_type();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Read the next token from the string.
 */
auto QmuParserTokenReader::ReadNextToken(const QLocale &locale, bool cNumbers, const QChar &decimal,
                                         const QChar &thousand) -> QmuParserTokenReader::token_type
{
    assert(m_pParser);

    token_type tok;

    // Ignore all non printable characters when reading the expression
    while (m_strFormula.size() > m_iPos && m_strFormula.at(m_iPos) <= QChar(0x20))
    {
        ++m_iPos;
    }

    if (IsEOF(tok))
    {
        return SaveBeforeReturn(tok); // Check for end of formula
    }
    if (IsOprt(tok))
    {
        return SaveBeforeReturn(tok); // Check for user defined binary operator
    }
    if (IsFunTok(tok))
    {
        return SaveBeforeReturn(tok); // Check for function token
    }
    if (IsBuiltIn(tok))
    {
        return SaveBeforeReturn(tok); // Check built in operators / tokens
    }
    if (IsArgSep(tok))
    {
        return SaveBeforeReturn(tok); // Check for function argument separators
    }
    if (IsValTok(tok, locale, cNumbers, decimal, thousand))
    {
        return SaveBeforeReturn(tok); // Check for values / constant tokens
    }
    if (IsVarTok(tok))
    {
        return SaveBeforeReturn(tok); // Check for variable tokens
    }
    if (IsStrVarTok(tok))
    {
        return SaveBeforeReturn(tok); // Check for string variables
    }
    if (IsString(tok))
    {
        return SaveBeforeReturn(tok); // Check for String tokens
    }
    if (IsInfixOpTok(tok))
    {
        return SaveBeforeReturn(tok); // Check for unary operators
    }
    if (IsPostOpTok(tok))
    {
        return SaveBeforeReturn(tok); // Check for unary operators
    }

    // Check String for undefined variable token. Done only if a
    // flag is set indicating to ignore undefined variables.
    // This is a way to conditionally avoid an error if
    // undefined variables occur.
    // (The GetUsedVar function must suppress the error for
    // undefined variables in order to collect all variable
    // names including the undefined ones.)
    if ((m_bIgnoreUndefVar || m_pFactory) && IsUndefVarTok(tok))
    {
        return SaveBeforeReturn(tok);
    }

    // Check for unknown token
    //
    // !!! From this point on there is no exit without an exception possible...
    //
    QString strTok;
    if (qmusizetype const iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos); iEnd != m_iPos)
    {
        Error(ecUNASSIGNABLE_TOKEN, m_iPos, strTok);
    }

    Error(ecUNASSIGNABLE_TOKEN, m_iPos, m_strFormula.mid(m_iPos));
    return {}; // never reached
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserTokenReader::SetParent(QmuParserBase *a_pParent)
{
    m_pParser = a_pParent;
    m_pFunDef = &a_pParent->m_FunDef;
    m_pOprtDef = &a_pParent->m_OprtDef;
    m_pInfixOprtDef = &a_pParent->m_InfixOprtDef;
    m_pPostOprtDef = &a_pParent->m_PostOprtDef;
    m_pVarDef = &a_pParent->m_VarDef;
    m_pStrVarDef = &a_pParent->m_StrVarDef;
    m_pConstDef = &a_pParent->m_ConstDef;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Extract all characters that belong to a certain charset.
 *
 * @param a_szCharSet [in] Const char array of the characters allowed in the token.
 * @param a_sTok [out]  The string that consists entirely of characters listed in a_szCharSet.
 * @param a_iPos [in] Position in the string from where to start reading.
 * @return The Position of the first character not listed in a_szCharSet.
 * @throw None No exceptions are thrown.
 */
QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4309)
auto QmuParserTokenReader::ExtractToken(const QString &a_szCharSet, QString &a_sTok, qmusizetype a_iPos) const
    -> qmusizetype
{
    qmusizetype iEnd = FindFirstNotOf(m_strFormula, a_szCharSet, a_iPos);

    if (iEnd == -1)
    {
        iEnd = m_strFormula.length();
    }

    // Assign token string if there was something found
    if (a_iPos != iEnd)
    {
        a_sTok = m_strFormula.mid(a_iPos, iEnd - a_iPos);
    }

    return iEnd;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check Expression for the presence of a binary operator token.
 *
 * Userdefined binary operator "++" gives inconsistent parsing result for the equations "a++b" and "a ++ b" if
 * alphabetic characters are allowed in operator tokens. To avoid this this function checks specifically
 * for operator tokens.
 */
auto QmuParserTokenReader::ExtractOperatorToken(QString &a_sTok, qmusizetype a_iPos) const -> qmusizetype
{
    qmusizetype iEnd = FindFirstNotOf(m_strFormula, m_pParser->ValidOprtChars(), a_iPos);

    if (iEnd == -1)
    {
        iEnd = m_strFormula.length();
    }

    // Assign token string if there was something found
    if (a_iPos != iEnd)
    {
        a_sTok = m_strFormula.mid(a_iPos, iEnd - a_iPos);
        return iEnd;
    }

    // There is still the chance of having to deal with an operator consisting exclusively
    // of alphabetic characters.
    return ExtractToken(QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), a_sTok, a_iPos);
}
QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check if a built in operator or other token can be found
 * @param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
 * @return true if an operator token has been found.
 */
auto QmuParserTokenReader::IsBuiltIn(token_type &a_Tok) -> bool
{
    const QStringList pOprtDef = QmuParserBase::GetOprtDef();

    // Compare token with function and operator strings
    // check string for operator/function
    for (int i = 0; i < pOprtDef.size(); ++i)
    {
        qmusizetype const len = pOprtDef.at(i).length();
        if (pOprtDef.at(i) == m_strFormula.mid(m_iPos, len))
        {
            if (i >= cmLE && i <= cmASSIGN)
            {
                // if (len!=sTok.length())
                //   continue;

                // The assignement operator need special treatment
                if (i == cmASSIGN && m_iSynFlags & noASSIGN)
                {
                    Error(ecUNEXPECTED_OPERATOR, m_iPos, pOprtDef.at(i));
                }

                if (not m_pParser->HasBuiltInOprt())
                {
                    continue;
                }
                if (m_iSynFlags & noOPT)
                {
                    // Maybe its an infix operator not an operator
                    // Both operator types can share characters in
                    // their identifiers
                    if (IsInfixOpTok(a_Tok))
                    {
                        return true;
                    }

                    Error(ecUNEXPECTED_OPERATOR, m_iPos, pOprtDef.at(i));
                }

                m_iSynFlags = noBC | noOPT | noARG_SEP | noPOSTOP | noASSIGN | noIF | noELSE;
                m_iSynFlags |= noEND;
            }
            else if (i == cmBO)
            {
                if (m_iSynFlags & noBO)
                {
                    Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef.at(i));
                }

                if (m_lastTok.GetCode() == cmFUNC)
                {
                    m_iSynFlags = noOPT | noEND | noARG_SEP | noPOSTOP | noASSIGN | noIF | noELSE;
                }
                else
                {
                    m_iSynFlags = noBC | noOPT | noEND | noARG_SEP | noPOSTOP | noASSIGN | noIF | noELSE;
                }

                ++m_iBrackets;
            }
            else if (i == cmBC)
            {
                if (m_iSynFlags & noBC)
                {
                    Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef.at(i));
                }

                m_iSynFlags = noBO | noVAR | noVAL | noFUN | noINFIXOP | noSTR | noASSIGN;

                if (--m_iBrackets < 0)
                {
                    Error(ecUNEXPECTED_PARENS, m_iPos, pOprtDef.at(i));
                }
            }
            else if (i == cmELSE)
            {
                if (m_iSynFlags & noELSE)
                {
                    Error(ecUNEXPECTED_CONDITIONAL, m_iPos, pOprtDef.at(i));
                }

                m_iSynFlags = noBC | noPOSTOP | noEND | noOPT | noIF | noELSE;
            }
            else if (i == cmIF)
            {
                if (m_iSynFlags & noIF)
                {
                    Error(ecUNEXPECTED_CONDITIONAL, m_iPos, pOprtDef.at(i));
                }

                m_iSynFlags = noBC | noPOSTOP | noEND | noOPT | noIF | noELSE;
            }
            else // The operator is listed in c_DefaultOprt, but not here. This is a bad thing...
            {
                Error(ecINTERNAL_ERROR);
            }

            m_iPos += len;
            a_Tok.Set(static_cast<ECmdCode>(i), pOprtDef.at(i));
            return true;
        } // if operator string found
    }     // end of for all operator strings

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParserTokenReader::IsArgSep(token_type &a_Tok) -> bool
{
    if (m_strFormula.at(m_iPos) == m_cArgSep)
    {
        // copy the separator into null terminated string
        QString const szSep(m_cArgSep);

        if (m_iSynFlags & noARG_SEP)
        {
            Error(ecUNEXPECTED_ARG_SEP, m_iPos, szSep);
        }

        m_iSynFlags = noBC | noOPT | noEND | noARG_SEP | noPOSTOP | noASSIGN;
        m_iPos++;
        a_Tok.Set(cmARG_SEP, szSep);
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check for End of Formula.
 *
 * @return true if an end of formula is found false otherwise.
 * @param a_Tok [out] If an eof is found the corresponding token will be stored there.
 * @sa IsOprt, IsFunTok, IsStrFunTok, IsValTok, IsVarTok, IsString, IsInfixOpTok, IsPostOpTok
 */
auto QmuParserTokenReader::IsEOF(token_type &a_Tok) -> bool
{
    // check for EOF
    if (m_iPos >= m_strFormula.size())
    {
        if (m_iSynFlags & noEND)
        {
            try
            {
                Error(ecUNEXPECTED_EOF, m_iPos);
            }
            catch (qmu::QmuParserError &e)
            {
                qDebug() << "  Code:" << e.GetCode() << "(" << e.GetMsg() << ")";
                throw;
            }
        }

        if (m_iBrackets > 0)
        {
            Error(ecMISSING_PARENS, m_iPos, QChar(')'));
        }

        m_iSynFlags = 0;
        a_Tok.Set(cmEND);
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check if a string position contains a unary infix operator.
 * @return true if a function token has been found false otherwise.
 */
auto QmuParserTokenReader::IsInfixOpTok(token_type &a_Tok) -> bool
{
    QString sTok;
    if (qmusizetype const iEnd = ExtractToken(m_pParser->ValidInfixOprtChars(), sTok, m_iPos); iEnd == m_iPos)
    {
        return false;
    }

    // iteraterate over all postfix operator strings
    auto it = m_pInfixOprtDef->rbegin();
    for (; it != m_pInfixOprtDef->rend(); ++it)
    {
        if (sTok.indexOf(it->first) == 0)
        {
            a_Tok.Set(it->second, it->first);
            m_iPos += static_cast<int>(it->first.length());

            if (m_iSynFlags & noINFIXOP)
            {
                Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.GetAsString());
            }

            m_iSynFlags = noPOSTOP | noINFIXOP | noOPT | noBC | noSTR | noASSIGN;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check whether the token at a given position is a function token.
 * @param a_Tok [out] If a value token is found it will be placed here.
 * @throw ParserException if Syntaxflags do not allow a function at a_iPos
 * @return true if a function token has been found false otherwise.
 * @pre [assert] m_pParser!=0
 */
auto QmuParserTokenReader::IsFunTok(token_type &a_Tok) -> bool
{
    QString strTok;
    qmusizetype const iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd == m_iPos)
    {
        return false;
    }

    auto item = m_pFunDef->find(strTok);
    if (item == m_pFunDef->end())
    {
        return false;
    }

    // Check if the next sign is an opening bracket
    if (m_strFormula.at(iEnd) != '(')
    {
        return false;
    }

    a_Tok.Set(item->second, strTok);

    m_iPos = iEnd;
    if (m_iSynFlags & noFUN)
    {
        Error(ecUNEXPECTED_FUN, m_iPos - static_cast<int>(a_Tok.GetAsString().length()), a_Tok.GetAsString());
    }

    m_iSynFlags = noANY ^ noBO;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check if a string position contains a binary operator.
 * @param a_Tok  [out] Operator token if one is found. This can either be a binary operator or an infix operator token.
 * @return true if an operator token has been found.
 */
auto QmuParserTokenReader::IsOprt(token_type &a_Tok) -> bool
{
    QString strTok;
    if (qmusizetype const iEnd = ExtractOperatorToken(strTok, m_iPos); iEnd == m_iPos)
    {
        return false;
    }

    // Check if the operator is a built in operator, if so ignore it here
    const QStringList &pOprtDef = QmuParserBase::GetOprtDef();
    for (auto constIterator = pOprtDef.constBegin();
         m_pParser->HasBuiltInOprt() && constIterator != pOprtDef.constEnd(); ++constIterator)
    {
        if ((*constIterator) == strTok)
        {
            return false;
        }
    }

    // Note:
    // All tokens in oprt_bin_maptype are have been sorted by their length
    // Long operators must come first! Otherwise short names (like: "add") that
    // are part of long token names (like: "add123") will be found instead
    // of the long ones.
    // Length sorting is done with ascending length so we use a reverse iterator here.
    auto it = m_pOprtDef->rbegin();
    for (; it != m_pOprtDef->rend(); ++it)
    {
        const QString &sID = it->first;
        if (sID == m_strFormula.mid(m_iPos, sID.length()))
        {
            a_Tok.Set(it->second, strTok);

            // operator was found
            if (m_iSynFlags & noOPT)
            {
                // An operator was found but is not expected to occur at
                // this position of the formula, maybe it is an infix
                // operator, not a binary operator. Both operator types
                // can share characters in their identifiers.
                if (IsInfixOpTok(a_Tok))
                {
                    return true;
                }

                // nope, no infix operator
                return false;
                // Error(ecUNEXPECTED_OPERATOR, m_iPos, a_Tok.GetAsString());
            }

            m_iPos += sID.length();
            m_iSynFlags = noBC | noOPT | noARG_SEP | noPOSTOP | noEND | noASSIGN;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check if a string position contains a unary post value operator.
 */
auto QmuParserTokenReader::IsPostOpTok(token_type &a_Tok) -> bool
{
    // <ibg 20110629> Do not check for postfix operators if they are not allowed at
    //                the current expression index.
    //
    //  This will fix the bug reported here:
    //
    //  http://sourceforge.net/tracker/index.php?func=detail&aid=3343891&group_id=137191&atid=737979
    //
    if (m_iSynFlags & noPOSTOP)
    {
        return false;
    }
    // </ibg>

    // Tricky problem with equations like "3m+5":
    //     m is a postfix operator, + is a valid sign for postfix operators and
    //     for binary operators parser detects "m+" as operator string and
    //     finds no matching postfix operator.
    //
    // This is a special case so this routine slightly differs from the other
    // token readers.

    // Test if there could be a postfix operator
    QString sTok;
    if (qmusizetype const iEnd = ExtractToken(m_pParser->ValidOprtChars(), sTok, m_iPos); iEnd == m_iPos)
    {
        return false;
    }

    // iteraterate over all postfix operator strings
    auto it = m_pPostOprtDef->rbegin();
    for (; it != m_pPostOprtDef->rend(); ++it)
    {
        if (sTok.indexOf(it->first) == 0)
        {
            a_Tok.Set(it->second, sTok);
            m_iPos += it->first.length();

            m_iSynFlags = noVAL | noVAR | noFUN | noBO | noPOSTOP | noSTR | noASSIGN;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check whether the token at a given position is a value token.
 *
 * Value tokens are either values or constants.
 *
 * @param a_Tok [out] If a value token is found it will be placed here.
 * @return true if a value token has been found.
 */
auto QmuParserTokenReader::IsValTok(token_type &a_Tok, const QLocale &locale, bool cNumbers, const QChar &decimal,
                                    const QChar &thousand) -> bool
{
    assert(m_pConstDef);
    assert(m_pParser);

    QString strTok;
    qreal fVal(0);
    qmusizetype iEnd(0);

    // 2.) Check for user defined constant
    // Read everything that could be a constant name
    iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd != m_iPos)
    {
        auto item = m_pConstDef->find(strTok);
        if (item != m_pConstDef->end())
        {
            m_iPos = iEnd;
            a_Tok.SetVal(item->second, strTok);

            if (m_iSynFlags & noVAL)
            {
                Error(ecUNEXPECTED_VAL, m_iPos - strTok.length(), strTok);
            }

            m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noSTR | noASSIGN;
            return true;
        }
    }

    // 3.call the value recognition functions provided by the user
    // Call user defined value recognition functions
    for (auto item = m_vIdentFun.begin(); item != m_vIdentFun.end(); ++item)
    {
        qmusizetype const iStart = m_iPos;
        if ((*item)(m_strFormula.mid(m_iPos), &m_iPos, &fVal, locale, cNumbers, decimal, thousand) == 1)
        {
            // 2013-11-27 Issue 2:  https://code.google.com/p/muparser/issues/detail?id=2
            strTok = m_strFormula.mid(iStart, m_iPos - iStart);
            if (m_iSynFlags & noVAL)
            {
                Error(ecUNEXPECTED_VAL, m_iPos - strTok.length(), strTok);
            }

            a_Tok.SetVal(fVal, strTok);
            m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noSTR | noASSIGN;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check wheter a token at a given position is a variable token.
 * @param a_Tok [out] If a variable token has been found it will be placed here.
 * @return true if a variable token has been found.
 */
auto QmuParserTokenReader::IsVarTok(token_type &a_Tok) -> bool
{
    if (m_pVarDef->empty())
    {
        return false;
    }

    QString strTok;
    qmusizetype iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd == m_iPos)
    {
        return false;
    }

    auto item = m_pVarDef->find(strTok);
    if (item == m_pVarDef->end())
    {
        return false;
    }

    if (m_iSynFlags & noVAR)
    {
        Error(ecUNEXPECTED_VAR, m_iPos, strTok);
    }

    m_pParser->OnDetectVar(m_strFormula, m_iPos, iEnd);

    m_iPos = iEnd;
    a_Tok.SetVar(item->second, strTok);
    m_UsedVar[item->first] = item->second; // Add variable to used-var-list

    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noSTR;

    //  Zur Info hier die SynFlags von IsVal():
    //    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noINFIXOP | noSTR | noASSIGN;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParserTokenReader::IsStrVarTok(token_type &a_Tok) -> bool
{
    if (m_pStrVarDef == nullptr || m_pStrVarDef->empty())
    {
        return false;
    }

    QString strTok;
    qmusizetype const iEnd = ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos);
    if (iEnd == m_iPos)
    {
        return false;
    }

    auto item = m_pStrVarDef->find(strTok);
    if (item == m_pStrVarDef->end())
    {
        return false;
    }

    if (m_iSynFlags & noSTR)
    {
        Error(ecUNEXPECTED_VAR, m_iPos, strTok);
    }

    m_iPos = iEnd;
    if (m_pParser->m_vStringVarBuf.empty())
    {
        Error(ecINTERNAL_ERROR);
    }

    a_Tok.SetString(m_pParser->m_vStringVarBuf[item->second], m_pParser->m_vStringVarBuf.size());

    m_iSynFlags = noANY ^ (noBC | noOPT | noEND | noARG_SEP);
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check wheter a token at a given position is an undefined variable.
 *
 * @param a_Tok [out] If a variable tom_pParser->m_vStringBufken has been found it will be placed here.
 * @return true if a variable token has been found.
 * @throw None No exceptions are thrown.
 */
auto QmuParserTokenReader::IsUndefVarTok(token_type &a_Tok) -> bool
{
    QString strTok;
    qmusizetype const iEnd(ExtractToken(m_pParser->ValidNameChars(), strTok, m_iPos));
    if (iEnd == m_iPos)
    {
        return false;
    }

    if (m_iSynFlags & noVAR)
    {
        // <ibg/> 20061021 added token string strTok instead of a_Tok.GetAsString() as the
        //                 token identifier.
        // related bug report:
        // http://sourceforge.net/tracker/index.php?func=detail&aid=1578779&group_id=137191&atid=737979
        Error(ecUNEXPECTED_VAR, m_iPos - a_Tok.GetAsString().length(), strTok);
    }

    // If a factory is available implicitely create new variables
    if (m_pFactory)
    {
        try
        {
            qreal *fVar = m_pFactory(strTok, m_pFactoryData);
            a_Tok.SetVar(fVar, strTok);

            // Do not use m_pParser->DefineVar( strTok, fVar );
            // in order to define the new variable, it will clear the
            // m_UsedVar array which will kill previousely defined variables
            // from the list
            // This is safe because the new variable can never override an existing one
            // because they are checked first!
            (*m_pVarDef)[strTok] = fVar;
            m_UsedVar[strTok] = fVar; // Add variable to used-var-list
        }
        catch (const qmu::QmuParserError &e)
        {
            Q_UNUSED(e)
            return false;
        }
    }
    else
    {
        a_Tok.SetVar(&m_fZero, strTok);
        m_UsedVar[strTok] = nullptr; // Add variable to used-var-list
    }

    m_iPos = iEnd;

    // Call the variable factory in order to let it define a new parser variable
    m_iSynFlags = noVAL | noVAR | noFUN | noBO | noPOSTOP | noINFIXOP | noSTR;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Check wheter a token at a given position is a string.
 * @param a_Tok [out] If a variable token has been found it will be placed here.
 * @return true if a string token has been found.
 * @sa IsOprt, IsFunTok, IsStrFunTok, IsValTok, IsVarTok, IsEOF, IsInfixOpTok, IsPostOpTok
 */
auto QmuParserTokenReader::IsString(token_type &a_Tok) -> bool
{
    if (m_strFormula[m_iPos] != '"')
    {
        return false;
    }

    QString strBuf(m_strFormula.mid(m_iPos + 1));
    qmusizetype iEnd(0), iSkip(0);

    // parser over escaped '\"' end replace them with '"'
    for (iEnd = strBuf.indexOf("\""); iEnd != 0 && iEnd != -1; iEnd = strBuf.indexOf("\"", iEnd))
    {
        if (strBuf[iEnd - 1] != '\\')
        {
            break;
        }
        strBuf.replace(iEnd - 1, 2, "\"");
        iSkip++;
    }

    if (iEnd == -1)
    {
        Error(ecUNTERMINATED_STRING, m_iPos, "\"");
    }

    QString const strTok = strBuf.mid(0, iEnd);

    if (m_iSynFlags & noSTR)
    {
        Error(ecUNEXPECTED_STR, m_iPos, strTok);
    }

    m_pParser->m_vStringBuf.push_back(strTok); // Store string in internal buffer
    a_Tok.SetString(strTok, m_pParser->m_vStringBuf.size());

    m_iPos += strTok.length() + 2 + iSkip; // +2 wg Anfhrungszeichen; +iSkip fr entfernte escape zeichen
    m_iSynFlags = noANY ^ (noARG_SEP | noBC | noOPT | noEND);

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create an error containing the parse error position.
 *
 * This function will create an Parser Exception object containing the error text and its position.
 *
 * @param a_iErrc [in] The error code of type #EErrorCodes.
 * @param a_iPos [in] The position where the error was detected.
 * @param a_sTok [in] The token string representation associated with the error.
 * @throw ParserException always throws thats the only purpose of this function.
 */
Q_NORETURN void QmuParserTokenReader::Error(EErrorCodes a_iErrc, qmusizetype a_iPos, const QString &a_sTok) const
{
    m_pParser->Error(a_iErrc, a_iPos, a_sTok);
}
} // namespace qmu
