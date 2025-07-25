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

#include "qmuparserbytecode.h"

#include <assert.h>
#include <QMessageLogger>
#include <QStack>
#include <QString>
#include <QtDebug>
#include <QtMath>

#include "qmudef.h"
#include "qmuparsererror.h"

namespace qmu
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Bytecode default constructor.
 */
QmuParserByteCode::QmuParserByteCode()
{
    m_vRPN.reserve(50);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Copy constructor.
 *
 * Implemented in Terms of Assign(const QParserByteCode &a_ByteCode)
 */
QmuParserByteCode::QmuParserByteCode(const QmuParserByteCode &a_ByteCode)
    :m_iStackPos(a_ByteCode.m_iStackPos), m_iMaxStackSize(a_ByteCode.m_iMaxStackSize), m_vRPN(a_ByteCode.m_vRPN)
{
    Assign(a_ByteCode);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Assignment operator.
 *
 * Implemented in Terms of Assign(const QParserByteCode &a_ByteCode)
 */
auto QmuParserByteCode::operator=(const QmuParserByteCode &a_ByteCode) -> QmuParserByteCode&
{
    if (this != &a_ByteCode)
    {
        Assign(a_ByteCode);
    }
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Copy state of another object to this.
 *
 * @throw None No exceptions are thrown.
 */
void QmuParserByteCode::Assign(const QmuParserByteCode &a_ByteCode)
{
    if (this==&a_ByteCode)
    {
        return;
    }

    m_iStackPos = a_ByteCode.m_iStackPos;
    m_vRPN = a_ByteCode.m_vRPN;
    m_iMaxStackSize = a_ByteCode.m_iMaxStackSize;
    m_bEnableOptimizer = a_ByteCode.m_bEnableOptimizer;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add a Variable pointer to bytecode.
 * @param a_pVar Pointer to be added.
 * @throw None No exceptions are thrown.
 */
void QmuParserByteCode::AddVar(qreal *a_pVar)
{
    ++m_iStackPos;
    m_iMaxStackSize = qMax(m_iMaxStackSize, m_iStackPos);

    // optimization does not apply
    SToken tok; // NOLINT
    tok.Cmd       = cmVAR;
    tok.Val.ptr   = a_pVar;
    tok.Val.data  = 1;
    tok.Val.data2 = 0;
    m_vRPN.push_back(tok);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add a Variable pointer to bytecode.
 *
 * Value entries in byte code consist of:
 * <ul>
 * <li>value array position of the value</li>
 * <li>the operator code according to ParserToken::cmVAL</li>
 * <li>the value stored in #mc_iSizeVal number of bytecode entries.</li>
 * </ul>
 *
 * @param a_fVal Value to be added.
 * @throw None No exceptions are thrown.
 */
void QmuParserByteCode::AddVal(qreal a_fVal)
{
    ++m_iStackPos;
    m_iMaxStackSize = qMax(m_iMaxStackSize, m_iStackPos);

    // If optimization does not apply
    SToken tok; // NOLINT
    tok.Cmd = cmVAL;
    tok.Val.ptr   = nullptr;
    tok.Val.data  = 0;
    tok.Val.data2 = a_fVal;
    m_vRPN.push_back(tok);
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserByteCode::ConstantFolding(ECmdCode a_Oprt)
{
    qmusizetype const sz = m_vRPN.size();
    qreal &x = m_vRPN[sz-2].Val.data2,
          &y = m_vRPN[sz-1].Val.data2;
    switch (a_Oprt)
    {
        case cmLAND:
            // cppcheck-suppress assignBoolToFloat
            x = static_cast<int>(x) && static_cast<int>(y);
            m_vRPN.pop_back();
            break;
        case cmLOR:
            // cppcheck-suppress assignBoolToFloat
            x = static_cast<int>(x) || static_cast<int>(y);
            m_vRPN.pop_back();
            break;
        case cmLT:
            // cppcheck-suppress assignBoolToFloat
            x = x < y;
            m_vRPN.pop_back();
            break;
        case cmGT:
            // cppcheck-suppress assignBoolToFloat
            x = x > y;
            m_vRPN.pop_back();
            break;
        case cmLE:
            // cppcheck-suppress assignBoolToFloat
            x = x <= y;
            m_vRPN.pop_back();
            break;
        case cmGE:
            // cppcheck-suppress assignBoolToFloat
            x = x >= y;
            m_vRPN.pop_back();
            break;
        case cmNEQ:
            // cppcheck-suppress assignBoolToFloat
            x = not QmuFuzzyComparePossibleNulls(x, y);
            m_vRPN.pop_back();
            break;
        case cmEQ:
            // cppcheck-suppress assignBoolToFloat
            x = QmuFuzzyComparePossibleNulls(x, y);
            m_vRPN.pop_back();
            break;
        case cmADD:
            x = x + y;
            m_vRPN.pop_back();
            break;
        case cmSUB:
            x = x - y;
            m_vRPN.pop_back();
            break;
        case cmMUL:
            x = x * y;
            m_vRPN.pop_back();
            break;
        case cmDIV:
        #if defined(MUP_MATH_EXCEPTIONS)
            if (y==0)
            {
                throw qmuParserError(ecDIV_BY_ZERO, QLatin1Char('0'));
            }
        #endif
            x = x / y;
            m_vRPN.pop_back();
            break;
        case cmPOW:
            x = qPow(x, y);
            m_vRPN.pop_back();
            break;
        default:
            break;
        } // switch opcode
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserByteCode::OpPOW(qmusizetype sz, bool &bOptimized)
{
    if (m_vRPN.at(sz-2).Cmd == cmVAR && m_vRPN.at(sz-1).Cmd == cmVAL) //-V807
    {
        if (qFuzzyCompare(m_vRPN.at(sz-1).Val.data2, 2)) //-V807
        {
            m_vRPN[sz-2].Cmd = cmVARPOW2;
        }
        else if (qFuzzyCompare(m_vRPN.at(sz-1).Val.data2, 3))
        {
            m_vRPN[sz-2].Cmd = cmVARPOW3;
        }
        else if (qFuzzyCompare(m_vRPN.at(sz-1).Val.data2, 4))
        {
            m_vRPN[sz-2].Cmd = cmVARPOW4;
        }
        else
        {
            return;
        }
        m_vRPN.pop_back();
        bOptimized = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserByteCode::OpSUBADD(ECmdCode a_Oprt, qmusizetype sz, bool &bOptimized)
{
    if ( (m_vRPN.at(sz-1).Cmd == cmVAR    && m_vRPN.at(sz-2).Cmd == cmVAL)   ||
        (m_vRPN.at(sz-1).Cmd == cmVAL    && m_vRPN.at(sz-2).Cmd == cmVAR)    ||
        (m_vRPN.at(sz-1).Cmd == cmVAL    && m_vRPN.at(sz-2).Cmd == cmVARMUL) ||
        (m_vRPN.at(sz-1).Cmd == cmVARMUL && m_vRPN.at(sz-2).Cmd == cmVAL)    ||
        (m_vRPN.at(sz-1).Cmd == cmVAR    && m_vRPN.at(sz-2).Cmd == cmVAR &&
         m_vRPN.at(sz-2).Val.ptr == m_vRPN.at(sz-1).Val.ptr) || //-V807
        (m_vRPN.at(sz-1).Cmd == cmVAR && m_vRPN.at(sz-2).Cmd == cmVARMUL
         && m_vRPN.at(sz-2).Val.ptr == m_vRPN.at(sz-1).Val.ptr) ||
        (m_vRPN.at(sz-1).Cmd == cmVARMUL && m_vRPN.at(sz-2).Cmd == cmVAR &&
         m_vRPN.at(sz-2).Val.ptr == m_vRPN.at(sz-1).Val.ptr) ||
        (m_vRPN.at(sz-1).Cmd == cmVARMUL && m_vRPN.at(sz-2).Cmd == cmVARMUL &&
         m_vRPN.at(sz-2).Val.ptr == m_vRPN.at(sz-1).Val.ptr) )
    {
        assert( (m_vRPN.at(sz-2).Val.ptr==nullptr && m_vRPN.at(sz-1).Val.ptr!=nullptr) ||
               (m_vRPN.at(sz-2).Val.ptr!=nullptr && m_vRPN.at(sz-1).Val.ptr==nullptr) ||
               (m_vRPN.at(sz-2).Val.ptr == m_vRPN.at(sz-1).Val.ptr) );

        m_vRPN[sz-2].Cmd = cmVARMUL;
        m_vRPN[sz-2].Val.ptr = reinterpret_cast<qreal*>( //-V807 NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<qlonglong>(m_vRPN.at(sz-2).Val.ptr) | // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, hicpp-signed-bitwise)
            reinterpret_cast<qlonglong>(m_vRPN.at(sz-1).Val.ptr)); // variable NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        m_vRPN[sz-2].Val.data2 += ((a_Oprt==cmSUB) ? -1 : 1) * m_vRPN.at(sz-1).Val.data2; // offset
        m_vRPN[sz-2].Val.data  += ((a_Oprt==cmSUB) ? -1 : 1) * m_vRPN.at(sz-1).Val.data;  // multiplikatior
        m_vRPN.pop_back();
        bOptimized = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserByteCode::OpMUL(qmusizetype sz, bool &bOptimized)
{
    if ( (m_vRPN.at(sz-1).Cmd == cmVAR && m_vRPN.at(sz-2).Cmd == cmVAL) ||
        (m_vRPN.at(sz-1).Cmd == cmVAL && m_vRPN.at(sz-2).Cmd == cmVAR) )
    {
        m_vRPN[sz-2].Cmd        = cmVARMUL;
        m_vRPN[sz-2].Val.ptr    = reinterpret_cast<qreal*>( // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<qlonglong>(m_vRPN.at(sz-2).Val.ptr) | // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, hicpp-signed-bitwise)
            reinterpret_cast<qlonglong>(m_vRPN.at(sz-1).Val.ptr)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        m_vRPN[sz-2].Val.data   = m_vRPN.at(sz-2).Val.data2 + m_vRPN.at(sz-1).Val.data2;
        m_vRPN[sz-2].Val.data2  = 0;
        m_vRPN.pop_back();
        bOptimized = true;
    }
    else if ( (m_vRPN.at(sz-1).Cmd == cmVAL    && m_vRPN.at(sz-2).Cmd == cmVARMUL) ||
             (m_vRPN.at(sz-1).Cmd == cmVARMUL && m_vRPN.at(sz-2).Cmd == cmVAL) )
    {
        // Optimization: 2*(3*b+1) or (3*b+1)*2 -> 6*b+2
        m_vRPN[sz-2].Cmd     = cmVARMUL;
        m_vRPN[sz-2].Val.ptr = reinterpret_cast<qreal*>( // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<qlonglong>(m_vRPN.at(sz-2).Val.ptr) | // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast, hicpp-signed-bitwise)
            reinterpret_cast<qlonglong>(m_vRPN.at(sz-1).Val.ptr)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        if (m_vRPN.at(sz-1).Cmd == cmVAL)
        {
            m_vRPN[sz-2].Val.data  *= m_vRPN.at(sz-1).Val.data2;
            m_vRPN[sz-2].Val.data2 *= m_vRPN.at(sz-1).Val.data2;
        }
        else
        {
            m_vRPN[sz-2].Val.data  = m_vRPN.at(sz-1).Val.data  * m_vRPN.at(sz-2).Val.data2;
            m_vRPN[sz-2].Val.data2 = m_vRPN.at(sz-1).Val.data2 * m_vRPN.at(sz-2).Val.data2;
        }
        m_vRPN.pop_back();
        bOptimized = true;
    }
    else if (m_vRPN.at(sz-1).Cmd == cmVAR && m_vRPN.at(sz-2).Cmd == cmVAR &&
             m_vRPN.at(sz-1).Val.ptr == m_vRPN.at(sz-2).Val.ptr)
    {
        // Optimization: a*a -> a^2
        m_vRPN[sz-2].Cmd = cmVARPOW2;
        m_vRPN.pop_back();
        bOptimized = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParserByteCode::OpDIV(qmusizetype sz, bool &bOptimized)
{
    if (m_vRPN.at(sz-1).Cmd == cmVAL && m_vRPN.at(sz-2).Cmd == cmVARMUL &&
        not qFuzzyIsNull(m_vRPN.at(sz-1).Val.data2))
    {
        // Optimization: 4*a/2 -> 2*a
        m_vRPN[sz-2].Val.data  /= m_vRPN.at(sz-1).Val.data2;
        m_vRPN[sz-2].Val.data2 /= m_vRPN.at(sz-1).Val.data2;
        m_vRPN.pop_back();
        bOptimized = true;
    }
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_INTEL(1195)
QT_WARNING_DISABLE_MSVC(4826)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add an operator identifier to bytecode.
 *
 * Operator entries in byte code consist of:
 * <ul>
 * <li>value array position of the result</li>
 * <li>the operator code according to ParserToken::ECmdCode</li>
 * </ul>
 *
 * @sa  ParserToken::ECmdCode
 */
void QmuParserByteCode::AddOp(ECmdCode a_Oprt)
{
    auto WriteValue = [this, a_Oprt](bool bOptimized)
    {
        if (not bOptimized)
        {
            --m_iStackPos;
            SToken tok; // NOLINT
            tok.Cmd = a_Oprt;
            m_vRPN.push_back(tok);
        }
    };

    bool bOptimized = false;

    if (not m_bEnableOptimizer)
    {
        // If optimization can't be applied just write the value
        WriteValue(bOptimized);
        return;
    }

    // Check for foldable constants like:
    //   cmVAL cmVAL cmADD
    // where cmADD can stand fopr any binary operator applied to
    // two constant values.
    if (qmusizetype const sz = m_vRPN.size();
        sz >= 2 && m_vRPN.at(sz - 2).Cmd == cmVAL && m_vRPN.at(sz - 1).Cmd == cmVAL)
    {
        ConstantFolding(a_Oprt);
        bOptimized = true;
    }
    else
    {
        switch (a_Oprt)
        {
            case cmPOW:
                // Optimization for ploynomials of low order
                OpPOW(sz, bOptimized);
                break;
            case cmSUB:
            case cmADD:
                // Simple optimization based on pattern recognition for a shitload of different
                // bytecode combinations of addition/subtraction
                OpSUBADD(a_Oprt, sz, bOptimized);
                break;
            case cmMUL:
                OpMUL(sz, bOptimized);
                break;
            case cmDIV:
                OpDIV(sz, bOptimized);
                break;
            default:
                break;

        } // switch a_Oprt
    }

    // If optimization can't be applied just write the value
    WriteValue(bOptimized);
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
void QmuParserByteCode::AddIfElse(ECmdCode a_Oprt)
{
    SToken tok; // NOLINT
    tok.Cmd = a_Oprt;
    m_vRPN.push_back(tok);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add an assignement operator
 *
 * Operator entries in byte code consist of:
 * <ul>
 * <li>cmASSIGN code</li>
 * <li>the pointer of the destination variable</li>
 * </ul>
 *
 * @sa  ParserToken::ECmdCode
 */
void QmuParserByteCode::AddAssignOp(qreal *a_pVar)
{
    --m_iStackPos;

    SToken tok; // NOLINT
    tok.Cmd = cmASSIGN;
    tok.Oprt.ptr = a_pVar;
    m_vRPN.push_back(tok);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add function to bytecode.
 *
 * @param a_iArgc Number of arguments, negative numbers indicate multiarg functions.
 * @param a_pFun Pointer to function callback.
 */
void QmuParserByteCode::AddFun(generic_fun_type a_pFun, int a_iArgc)
{
    if (a_iArgc>=0)
    {
        m_iStackPos = m_iStackPos - static_cast<quint32>(a_iArgc) + 1;
    }
    else
    {
        // function with unlimited number of arguments
        m_iStackPos = static_cast<quint32>(static_cast<int>(m_iStackPos) + a_iArgc + 1);
    }
    m_iMaxStackSize = qMax(m_iMaxStackSize, m_iStackPos);

    SToken tok; // NOLINT
    tok.Cmd = cmFUNC;
    tok.Fun.argc = a_iArgc;
    tok.Fun.ptr = a_pFun;
    m_vRPN.push_back(tok);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add a bulk function to bytecode.
 *
 * @param a_iArgc Number of arguments, negative numbers indicate multiarg functions.
 * @param a_pFun Pointer to function callback.
 */
void QmuParserByteCode::AddBulkFun(generic_fun_type a_pFun, qmusizetype a_iArgc)
{
    m_iStackPos = static_cast<quint32>(static_cast<int>(m_iStackPos) - a_iArgc + 1);
    m_iMaxStackSize = qMax(m_iMaxStackSize, m_iStackPos);

    SToken tok; // NOLINT
    tok.Cmd = cmFUNC_BULK;
    tok.Fun.argc = a_iArgc;
    tok.Fun.ptr = a_pFun;
    m_vRPN.push_back(tok);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add Strung function entry to the parser bytecode.
 * @throw None No exceptions are thrown.
 *
 * A string function entry consists of the stack position of the return value, followed by a cmSTRFUNC code, the
 * function pointer and an index into the string buffer maintained by the parser.
 */
void QmuParserByteCode::AddStrFun(generic_fun_type a_pFun, int a_iArgc, qmusizetype a_iIdx)
{
    m_iStackPos = static_cast<quint32>(static_cast<int>(m_iStackPos) - a_iArgc + 1);

    SToken tok; // NOLINT
    tok.Cmd = cmFUNC_STR;
    tok.Fun.argc = a_iArgc;
    tok.Fun.idx = a_iIdx;
    tok.Fun.ptr = a_pFun;
    m_vRPN.push_back(tok);

    m_iMaxStackSize = qMax(m_iMaxStackSize, m_iStackPos);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add end marker to bytecode.
 *
 * @throw None No exceptions are thrown.
 */
void QmuParserByteCode::Finalize()
{
    SToken tok; // NOLINT
    tok.Cmd = cmEND;
    m_vRPN.push_back(tok);
    rpn_type(m_vRPN).swap(m_vRPN);     // shrink bytecode vector to fit

    // Determine the if-then-else jump offsets
    QStack<int> stIf, stElse;
    int idx;
    for (int i=0; i<m_vRPN.size(); ++i)
    {
        switch (m_vRPN.at(i).Cmd)
        {
            case cmIF:
                stIf.push(i);
                break;
            case cmELSE:
                stElse.push(i);
                idx = stIf.pop();
                m_vRPN[idx].Oprt.offset = i - idx;
                break;
            case cmENDIF:
                idx = stElse.pop();
                m_vRPN[idx].Oprt.offset = i - idx;
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParserByteCode::GetBase() const -> const SToken*
{
    if (m_vRPN.empty())
    {
        throw QmuParserError(ecINTERNAL_ERROR);
    }

    return &m_vRPN.at(0);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Delete the bytecode.
 *
 * @throw None No exceptions are thrown.
 *
 * The name of this function is a violation of my own coding guidelines but this way it's more in line with the STL
 * functions thus more intuitive.
 */
void QmuParserByteCode::clear()
{
    m_vRPN.clear();
    m_iStackPos = 0;
    m_iMaxStackSize = 0;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Dump bytecode (for debugging only!).
 */
void QmuParserByteCode::AsciiDump()
{
    if (m_vRPN.empty())
    {
        qDebug() << "No bytecode available\n";
        return;
    }

    qDebug() << "Number of RPN tokens:" << m_vRPN.size() << "\n";
    for (int i=0; i<m_vRPN.size() && m_vRPN.at(i).Cmd!=cmEND; ++i)
    {
        qDebug() << i << " : \t";
        switch (m_vRPN.at(i).Cmd)
        {
            case cmVAL:
                qDebug() << "VAL \t" << "[" << m_vRPN.at(i).Val.data2 << "]\n";
                break;
            case cmVAR:
                qDebug() << "VAR \t" << "[ADDR: 0x" << QString::number(*m_vRPN.at(i).Val.ptr, 'f', 16) << "]\n";
                break;
            case cmVARPOW2:
                qDebug() << "VARPOW2 \t" << "[ADDR: 0x" << QString::number(*m_vRPN.at(i).Val.ptr, 'f', 16) << "]\n";
                break;
            case cmVARPOW3:
                qDebug() << "VARPOW3 \t" << "[ADDR: 0x" << QString::number(*m_vRPN.at(i).Val.ptr, 'f', 16) << "]\n";
                break;
            case cmVARPOW4:
                qDebug() << "VARPOW4 \t" << "[ADDR: 0x" << QString::number(*m_vRPN.at(i).Val.ptr, 'f', 16) << "]\n";
                break;
            case cmVARMUL:
                qDebug() << "VARMUL \t" << "[ADDR: 0x" << QString::number(*m_vRPN.at(i).Val.ptr, 'f', 16) << "]" << " * [" //-V807
                         << m_vRPN.at(i).Val.data << "]" << " + [" << m_vRPN.at(i).Val.data2 << "]\n";
                break;
            case cmFUNC:
                qDebug() << "CALL\t" << "[ARG:" << m_vRPN.at(i).Fun.argc << "]" << "[ADDR: 0x" << m_vRPN.at(i).Fun.ptr << "]"
                         << "\n";
                break;
            case cmFUNC_STR:
                qDebug() << "CALL STRFUNC\t" << "[ARG:" << m_vRPN.at(i).Fun.argc << "]" << "[IDX:" << m_vRPN.at(i).Fun.idx //-V807
                         << "]" << "[ADDR: 0x" << m_vRPN.at(i).Fun.ptr << "]\n";
                break;
            case cmLT:
                qDebug() << "LT\n";
                break;
            case cmGT:
                qDebug() << "GT\n";
                break;
            case cmLE:
                qDebug() << "LE\n";
                break;
            case cmGE:
                qDebug() << "GE\n";
                break;
            case cmEQ:
                qDebug() << "EQ\n";
                break;
            case cmNEQ:
                qDebug() << "NEQ\n";
                break;
            case cmADD:
                qDebug() << "ADD\n";
                break;
            case cmLAND:
                qDebug() << "&&\n";
                break;
            case cmLOR:
                qDebug() << "||\n";
                break;
            case cmSUB:
                qDebug() << "SUB\n";
                break;
            case cmMUL:
                qDebug() << "MUL\n";
                break;
            case cmDIV:
                qDebug() << "DIV\n";
                break;
            case cmPOW:
                qDebug() << "POW\n";
                break;
            case cmIF:
                qDebug() << "IF\t" << "[OFFSET:" << m_vRPN.at(i).Oprt.offset << "]\n";
                break;
            case cmELSE:
                qDebug() << "ELSE\t" << "[OFFSET:" << m_vRPN.at(i).Oprt.offset << "]\n";
                break;
            case cmENDIF:
                qDebug() << "ENDIF\n"; break;
            case cmASSIGN:
                qDebug() << "ASSIGN\t" << "[ADDR: 0x" << QString::number(*m_vRPN.at(i).Oprt.ptr, 'f', 16) << "]\n";
                break;
            case cmBO:
            case cmBC:
            case cmARG_SEP:
            case cmPOW2:
            case cmFUNC_BULK:
            case cmSTRING:
            case cmOPRT_BIN:
            case cmOPRT_POSTFIX:
            case cmOPRT_INFIX:
            case cmEND:
            case cmUNKNOWN:
            default:
                qDebug() << "(unknown code: " << m_vRPN.at(i).Cmd << ")\n";
                break;
        } // switch cmdCode
    } // while bytecode

    qDebug() << "END";
}
} // namespace qmu
