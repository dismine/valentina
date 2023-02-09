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

#ifndef QMUPARSERTOKENREADER_H
#define QMUPARSERTOKENREADER_H

#include <qcompilerdetection.h>
#include <QChar>
#include <QString>
#include <QtGlobal>
#include <list>
#include <locale>

#include "qmuparsercallback.h"
#include "qmuparserdef.h"
#include "qmuparsererror.h"
#include "qmuparsertoken.h"

/**
 * @file
 * @brief This file contains the parser token reader definition.
 */

namespace qmu
{
// Forward declaration
class QmuParserBase;

/**
 * @brief Token reader for the ParserBase class.
 *
 */
class QmuParserTokenReader
{
private:
    using token_type = QmuParserToken<qreal, QString>;
public:
    explicit QmuParserTokenReader(QmuParserBase *a_pParent);
    ~QmuParserTokenReader()=default;

    auto Clone(QmuParserBase *a_pParent) const -> QmuParserTokenReader*;

    void AddValIdent(identfun_type a_pCallback);
    void SetVarCreator(facfun_type a_pFactory, void *pUserData);
    void SetFormula(const QString &a_strFormula);
    void SetArgSep(char_type cArgSep);
    auto GetPos() const -> qmusizetype;
    auto GetExpr() const -> const QString&;
    auto GetUsedVar() -> varmap_type&;
    auto GetArgSep() const -> QChar;
    void IgnoreUndefVar(bool bIgnore);
    void ReInit();
    auto ReadNextToken(const QLocale &locale, bool cNumbers, const QChar &decimal, const QChar &thousand) -> token_type;
private:

    /**
     * @brief Syntax codes.
     *
     * The syntax codes control the syntax check done during the first time parsing of
     * the expression string. They are flags that indicate which tokens are allowed next
     * if certain tokens are identified.
     */
    enum ESynCodes
    {
        noBO      = 1 << 0,  ///< to avoid i.e. "cos(7)(" NOLINT(hicpp-signed-bitwise)
        noBC      = 1 << 1,  ///< to avoid i.e. "sin)" or "()" NOLINT(hicpp-signed-bitwise)
        noVAL     = 1 << 2,  ///< to avoid i.e. "tan 2" or "sin(8)3.14" NOLINT(hicpp-signed-bitwise)
        noVAR     = 1 << 3,  ///< to avoid i.e. "sin a" or "sin(8)a" NOLINT(hicpp-signed-bitwise)
        noARG_SEP = 1 << 4,  ///< to avoid i.e. ",," or "+," ... NOLINT(hicpp-signed-bitwise)
        noFUN     = 1 << 5,  ///< to avoid i.e. "sqrt cos" or "(1)sin" NOLINT(hicpp-signed-bitwise)
        noOPT     = 1 << 6,  ///< to avoid i.e. "(+)" NOLINT(hicpp-signed-bitwise)
        noPOSTOP  = 1 << 7,  ///< to avoid i.e. "(5!!)" "sin!" NOLINT(hicpp-signed-bitwise)
        noINFIXOP = 1 << 8,  ///< to avoid i.e. "++4" "!!4" NOLINT(hicpp-signed-bitwise)
        noEND     = 1 << 9,  ///< to avoid unexpected end of formula NOLINT(hicpp-signed-bitwise)
        noSTR     = 1 << 10, ///< to block numeric arguments on string functions NOLINT(hicpp-signed-bitwise)
        noASSIGN  = 1 << 11, ///< to block assignement to constant i.e. "4=7" NOLINT(hicpp-signed-bitwise)
        noIF      = 1 << 12, // NOLINT(hicpp-signed-bitwise)
        noELSE    = 1 << 13, // NOLINT(hicpp-signed-bitwise)
        sfSTART_OF_LINE = noOPT | noBC | noPOSTOP | noASSIGN | noIF | noELSE | noARG_SEP, // NOLINT(hicpp-signed-bitwise)
        noANY     = ~0       ///< All of he above flags set NOLINT(hicpp-signed-bitwise)
    };

    QmuParserBase     *m_pParser;
    QString            m_strFormula{};
    qmusizetype        m_iPos{0};
    int                m_iSynFlags{0};
    bool               m_bIgnoreUndefVar{false};

    const funmap_type *m_pFunDef{nullptr};
    const funmap_type *m_pPostOprtDef{nullptr};
    const funmap_type *m_pInfixOprtDef{nullptr};
    const funmap_type *m_pOprtDef{nullptr};
    const valmap_type *m_pConstDef{nullptr};
    const strmap_type *m_pStrVarDef{nullptr};
    varmap_type       *m_pVarDef{nullptr};         ///< The only non const pointer to parser internals
    facfun_type        m_pFactory{nullptr};
    void              *m_pFactoryData{nullptr};
    std::list<identfun_type> m_vIdentFun{}; ///< Value token identification function
    varmap_type        m_UsedVar{};
    qreal              m_fZero{0};           ///< Dummy value of zero, referenced by undefined variables
    int                m_iBrackets{0};       ///< Keep count open brackets
    token_type         m_lastTok{};
    QChar              m_cArgSep{';'};         ///< The character used for separating function arguments

    QmuParserTokenReader(const QmuParserTokenReader &a_Reader);
    auto operator=(const QmuParserTokenReader &a_Reader) -> QmuParserTokenReader&;

    void Assign(const QmuParserTokenReader &a_Reader);

    void SetParent(QmuParserBase *a_pParent);
    auto ExtractToken(const QString &a_szCharSet, QString &a_strTok, qmusizetype a_iPos) const -> qmusizetype;
    auto ExtractOperatorToken(QString &a_sTok, qmusizetype a_iPos) const -> qmusizetype;

    auto IsBuiltIn(token_type &a_Tok) -> bool;
    auto IsArgSep(token_type &a_Tok) -> bool;
    auto IsEOF(token_type &a_Tok) -> bool;
    auto IsInfixOpTok(token_type &a_Tok) -> bool;
    auto IsFunTok(token_type &a_Tok) -> bool;
    auto IsPostOpTok(token_type &a_Tok) -> bool;
    auto IsOprt(token_type &a_Tok) -> bool;
    auto IsValTok(token_type &a_Tok, const QLocale &locale, bool cNumbers, const QChar &decimal,
                  const QChar &thousand) -> bool;
    auto IsVarTok(token_type &a_Tok) -> bool;
    auto IsStrVarTok(token_type &a_Tok) -> bool;
    auto IsUndefVarTok(token_type &a_Tok) -> bool;
    auto IsString(token_type &a_Tok) -> bool;
    void Q_NORETURN Error(EErrorCodes a_iErrc, qmusizetype a_iPos = -1, const QString &a_sTok = QString() ) const;

    auto SaveBeforeReturn(const token_type &tok) -> token_type&;
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Return the current position of the token reader in the formula string.
 *
 * @return #m_iPos
 * @throw nothrow
 */
inline auto QmuParserTokenReader::GetPos() const -> qmusizetype
{
    return m_iPos;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Return a reference to the formula.
 *
 * @return #m_strFormula
 * @throw nothrow
 */
inline auto QmuParserTokenReader::GetExpr() const -> const QString&
{
    return m_strFormula;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Return a map containing the used variables only.
 */
inline auto QmuParserTokenReader::GetUsedVar() -> varmap_type&
{
    return m_UsedVar;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Set Flag that contronls behaviour in case of undefined variables beeing found.
 *
 * If true, the parser does not throw an exception if an undefined variable is found. Otherwise it does. This variable
 * is used internally only! It supresses a "undefined variable" exception in GetUsedVar().
 * Those function should return a complete list of variables including
 * those the are not defined by the time of it's call.
 */
inline void QmuParserTokenReader::IgnoreUndefVar ( bool bIgnore )
{
    m_bIgnoreUndefVar = bIgnore;
}

//---------------------------------------------------------------------------------------------------------------------
inline void QmuParserTokenReader::SetArgSep ( char_type cArgSep )
{
    m_cArgSep = cArgSep;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto QmuParserTokenReader::GetArgSep() const -> QChar
{
    return m_cArgSep;
}
} // namespace qmu

#endif
