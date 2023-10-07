/************************************************************************
 **
 **  @file   vsvgpathtokenizer.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vsvgpathtokenizer.h"
#include "qpainterpath.h"

#include <QMutex>
#include <QSet>
#include <QtMath>

#include "../compatibility.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QMutex, svgPathTokenizerMutex) // NOLINT

QT_WARNING_POP

const qreal V_PI = static_cast<qreal>(M_PI); // pi

//---------------------------------------------------------------------------------------------------------------------
auto IsCommand(const QChar &ch) -> bool
{
    static QSet<QChar> commandCharacterSet{'m', 'M', 'z', 'Z', 'l', 'L', 'h', 'H', 'v', 'V',
                                           'c', 'C', 's', 'S', 'q', 'Q', 't', 'T', 'a', 'A'};
    return commandCharacterSet.contains(ch);
}

//---------------------------------------------------------------------------------------------------------------------
auto IsSeparator(const QString &currentToken, const QChar &ch) -> bool
{
    if (ch.isSpace() || ch == ',')
    {
        return true;
    }

    if (ch == '.' && currentToken.contains(ch))
    {
        return true;
    }

    if (ch == '-' && !currentToken.isEmpty() && Back(currentToken) != 'e' && Back(currentToken) != 'E')
    {
        return true;
    }

    if (ch == '+' && !currentToken.isEmpty() && Back(currentToken) != 'e' && Back(currentToken) != 'E')
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void AddArgumentIfNotEmpty(VSVGPathCommand &currentCommand, const QString &currentToken)
{
    if (!currentToken.isEmpty() && !currentCommand.m_command.isNull())
    {
        bool ok = false;
        qreal val = currentToken.toDouble(&ok);
        if (ok)
        {
            currentCommand.m_arguments.push_back(val);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PathArcSegment(QPainterPath &path, qreal xc, qreal yc, qreal th0, qreal th1, qreal rx, qreal ry,
                    qreal xAxisRotation)
{
    qreal sinTh, cosTh;
    qreal a00, a01, a10, a11;
    qreal x1, y1, x2, y2, x3, y3;
    qreal t;
    qreal thHalf;
    sinTh = qSin(xAxisRotation * (V_PI / 180.0));
    cosTh = qCos(xAxisRotation * (V_PI / 180.0));
    a00 = cosTh * rx;
    a01 = -sinTh * ry;
    a10 = sinTh * rx;
    a11 = cosTh * ry;
    thHalf = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * qSin(thHalf * 0.5) * qSin(thHalf * 0.5) / qSin(thHalf);
    x1 = xc + qCos(th0) - t * qSin(th0);
    y1 = yc + qSin(th0) + t * qCos(th0);
    x3 = xc + qCos(th1);
    y3 = yc + qSin(th1);
    x2 = x3 + t * qSin(th1);
    y2 = y3 - t * qCos(th1);
    path.cubicTo(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1, a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                 a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

//---------------------------------------------------------------------------------------------------------------------
// the arc handling code underneath is from XSVG (BSD license)
/*
 * Copyright  2002 USC/Information Sciences Institute
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Information Sciences Institute not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  Information Sciences Institute
 * makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * INFORMATION SCIENCES INSTITUTE DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL INFORMATION SCIENCES
 * INSTITUTE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
void PathArc(QPainterPath &path, qreal rx, qreal ry, qreal x_axis_rotation, int large_arc_flag, int sweep_flag, qreal x,
             qreal y, qreal curx, qreal cury)
{
    const qreal Pr1 = rx * rx;
    const qreal Pr2 = ry * ry;
    if (qFuzzyIsNull(Pr1) || qFuzzyIsNull(Pr2))
    {
        return;
    }
    qreal sin_th, cos_th;
    qreal a00, a01, a10, a11;
    qreal x0, y0, x1, y1, xc, yc;
    qreal d, sfactor, sfactor_sq;
    qreal th0, th1, th_arc;
    int i, n_segs;
    qreal dx, dy, dx1, dy1, Px, Py, check;
    rx = qAbs(rx);
    ry = qAbs(ry);
    sin_th = qSin(x_axis_rotation * (V_PI / 180.0));
    cos_th = qCos(x_axis_rotation * (V_PI / 180.0));
    dx = (curx - x) / 2.0;
    dy = (cury - y) / 2.0;
    dx1 = cos_th * dx + sin_th * dy;
    dy1 = -sin_th * dx + cos_th * dy;
    Px = dx1 * dx1;
    Py = dy1 * dy1;
    /* Spec : check if radii are large enough */
    check = Px / Pr1 + Py / Pr2;
    if (check > 1)
    {
        rx = rx * qSqrt(check);
        ry = ry * qSqrt(check);
    }
    a00 = cos_th / rx;
    a01 = sin_th / rx;
    a10 = -sin_th / ry;
    a11 = cos_th / ry;
    x0 = a00 * curx + a01 * cury;
    y0 = a10 * curx + a11 * cury;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
       (x1, y1) is new point in transformed coordinate space.
       The arc fits a unit-radius circle in this space.
    */
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    if (qFuzzyIsNull(d))
    {
        return;
    }
    sfactor_sq = 1.0 / d - 0.25;
    if (sfactor_sq < 0)
    {
        sfactor_sq = 0;
    }
    sfactor = qSqrt(sfactor_sq);
    if (sweep_flag == large_arc_flag)
    {
        sfactor = -sfactor;
    }
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */
    th0 = qAtan2(y0 - yc, x0 - xc);
    th1 = qAtan2(y1 - yc, x1 - xc);
    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag)
    {
        th_arc += 2 * V_PI;
    }
    else if (th_arc > 0 && !sweep_flag)
    {
        th_arc -= 2 * V_PI;
    }
    n_segs = qCeil(qAbs(th_arc / (V_PI * 0.5 + 0.001)));
    for (i = 0; i < n_segs; i++)
    {
        PathArcSegment(path, xc, yc, th0 + i * th_arc / n_segs, th0 + (i + 1) * th_arc / n_segs, rx, ry,
                       x_axis_rotation);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VSVGPathTokenizer::VSVGPathTokenizer(const QString &path)
{
    QMutexLocker locker(svgPathTokenizerMutex());

    TokenizePathString(path);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSVGPathTokenizer::ToPainterPath(QPainterPath &path) const -> bool
{
    QMutexLocker locker(svgPathTokenizerMutex());

    // starting point
    m_x0 = 0;
    m_y0 = 0;

    // current point
    m_x = 0;
    m_y = 0;

    m_lastMode = 0;

    for (const auto &c : m_commands)
    {
        m_pathElem = c.m_command;
        m_num = c.m_arguments.data();
        m_count = static_cast<int>(c.m_arguments.size());

        while (m_count > 0)
        {
            m_offsetX = m_x; // correction offsets
            m_offsetY = m_y; // for relative commands

            switch (m_pathElem.unicode())
            {
                case 'm':
                    Command_m(path);
                    break;
                case 'M':
                    Command_M(path);
                    break;
                case 'z':
                case 'Z':
                    Command_z(path);
                    break;
                case 'l':
                    Command_l(path);
                    break;
                case 'L':
                    Command_L(path);
                    break;
                case 'h':
                    Command_h(path);
                    break;
                case 'H':
                    Command_H(path);
                    break;
                case 'v':
                    Command_v(path);
                    break;
                case 'V':
                    Command_V(path);
                    break;
                case 'c':
                    Command_c(path);
                    break;
                case 'C':
                    Command_C(path);
                    break;
                case 's':
                    Command_s(path);
                    break;
                case 'S':
                    Command_S(path);
                    break;
                case 'q':
                    Command_q(path);
                    break;
                case 'Q':
                    Command_Q(path);
                    break;
                case 't':
                    Command_t(path);
                    break;
                case 'T':
                    Command_T(path);
                    break;
                case 'a':
                    Command_a(path);
                    break;
                case 'A':
                    Command_A(path);
                    break;
                default:
                    return false;
            }
            m_lastMode = m_pathElem.toLatin1();
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::TokenizePathString(const QString &path)
{
    m_commands.clear();

    VSVGPathCommand currentCommand;
    QString currentToken;

    for (auto currentChar : path)
    {
        if (IsCommand(currentChar))
        {
            AddArgumentIfNotEmpty(currentCommand, currentToken);

            if (AddCommandIfNotNull(currentCommand))
            {
                currentCommand = VSVGPathCommand();
            }

            currentCommand.m_command = currentChar;
            currentToken.clear();
        }
        else if (IsSeparator(currentToken, currentChar))
        {
            AddArgumentIfNotEmpty(currentCommand, currentToken);

            currentToken.clear();
            if (!currentChar.isSpace() && currentChar != ',')
            {
                currentToken += currentChar;
            }
        }
        else
        {
            currentToken += currentChar;
        }
    }

    AddArgumentIfNotEmpty(currentCommand, currentToken);
    AddCommandIfNotNull(currentCommand);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSVGPathTokenizer::AddCommandIfNotNull(VSVGPathCommand command) -> bool // clazy:exclude=function-args-by-ref
{
    if (!command.m_command.isNull())
    {
        if (command.m_command == 'z' || command.m_command == 'Z')
        {
            command.m_arguments = {0}; // dummy
        }

        m_commands.append(command);
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_m(QPainterPath &path) const
{
    if (m_count < 2)
    {
        m_num++;
        m_count--;
        return;
    }

    m_x = m_x0 = m_num[0] + m_offsetX;
    m_y = m_y0 = m_num[1] + m_offsetY;
    m_num += 2;
    m_count -= 2;
    path.moveTo(m_x0, m_y0);
    // As per 1.2  spec 8.3.2 The "moveto" commands
    // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
    // the subsequent pairs shall be treated as implicit 'lineto' commands.
    m_pathElem = 'l'_L1;
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_M(QPainterPath &path) const
{
    if (m_count < 2)
    {
        m_num++;
        m_count--;
        return;
    }
    m_x = m_x0 = m_num[0];
    m_y = m_y0 = m_num[1];
    m_num += 2;
    m_count -= 2;
    path.moveTo(m_x0, m_y0);
    // As per 1.2  spec 8.3.2 The "moveto" commands
    // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
    // the subsequent pairs shall be treated as implicit 'lineto' commands.
    m_pathElem = 'L'_L1;
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_z(QPainterPath &path) const
{
    m_x = m_x0;
    m_y = m_y0;
    m_count--; // skip dummy
    m_num++;
    if (!m_singlePath)
    {
        path.closeSubpath();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_l(QPainterPath &path) const
{
    if (m_count < 2)
    {
        m_num++;
        m_count--;
        return;
    }
    m_x = m_num[0] + m_offsetX;
    m_y = m_num[1] + m_offsetY;
    m_num += 2;
    m_count -= 2;
    path.lineTo(m_x, m_y);
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_L(QPainterPath &path) const
{
    if (m_count < 2)
    {
        m_num++;
        m_count--;
        return;
    }
    m_x = m_num[0];
    m_y = m_num[1];
    m_num += 2;
    m_count -= 2;
    path.lineTo(m_x, m_y);
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_h(QPainterPath &path) const
{
    m_x = m_num[0] + m_offsetX;
    m_num++;
    m_count--;
    path.lineTo(m_x, m_y);
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_H(QPainterPath &path) const
{
    m_x = m_num[0];
    m_num++;
    m_count--;
    path.lineTo(m_x, m_y);
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_v(QPainterPath &path) const
{
    m_y = m_num[0] + m_offsetY;
    m_num++;
    m_count--;
    path.lineTo(m_x, m_y);
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_V(QPainterPath &path) const
{
    m_y = m_num[0];
    m_num++;
    m_count--;
    path.lineTo(m_x, m_y);
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_c(QPainterPath &path) const
{
    if (m_count < 6)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }
    QPointF c1(m_num[0] + m_offsetX, m_num[1] + m_offsetY);
    QPointF c2(m_num[2] + m_offsetX, m_num[3] + m_offsetY);
    QPointF e(m_num[4] + m_offsetX, m_num[5] + m_offsetY);
    m_num += 6;
    m_count -= 6;
    path.cubicTo(c1, c2, e);
    m_ctrlPt = c2;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_C(QPainterPath &path) const
{
    if (m_count < 6)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }
    QPointF c1(m_num[0], m_num[1]);
    QPointF c2(m_num[2], m_num[3]);
    QPointF e(m_num[4], m_num[5]);
    m_num += 6;
    m_count -= 6;
    path.cubicTo(c1, c2, e);
    m_ctrlPt = c2;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_s(QPainterPath &path) const
{
    if (m_count < 4)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }
    QPointF c1;
    if (m_lastMode == 'c' || m_lastMode == 'C' || m_lastMode == 's' || m_lastMode == 'S')
    {
        c1 = QPointF(2 * m_x - m_ctrlPt.x(), 2 * m_y - m_ctrlPt.y());
    }
    else
    {
        c1 = QPointF(m_x, m_y);
    }
    QPointF c2(m_num[0] + m_offsetX, m_num[1] + m_offsetY);
    QPointF e(m_num[2] + m_offsetX, m_num[3] + m_offsetY);
    m_num += 4;
    m_count -= 4;
    path.cubicTo(c1, c2, e);
    m_ctrlPt = c2;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_S(QPainterPath &path) const
{
    if (m_count < 4)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }

    QPointF c1;
    if (m_lastMode == 'c' || m_lastMode == 'C' || m_lastMode == 's' || m_lastMode == 'S')
    {
        c1 = QPointF(2 * m_x - m_ctrlPt.x(), 2 * m_y - m_ctrlPt.y());
    }
    else
    {
        c1 = QPointF(m_x, m_y);
    }

    QPointF c2(m_num[0], m_num[1]);
    QPointF e(m_num[2], m_num[3]);
    m_num += 4;
    m_count -= 4;
    path.cubicTo(c1, c2, e);
    m_ctrlPt = c2;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_q(QPainterPath &path) const
{
    if (m_count < 4)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }

    QPointF c(m_num[0] + m_offsetX, m_num[1] + m_offsetY);
    QPointF e(m_num[2] + m_offsetX, m_num[3] + m_offsetY);
    m_num += 4;
    m_count -= 4;
    path.quadTo(c, e);
    m_ctrlPt = c;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_Q(QPainterPath &path) const
{
    if (m_count < 4)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }

    QPointF c(m_num[0], m_num[1]);
    QPointF e(m_num[2], m_num[3]);
    m_num += 4;
    m_count -= 4;
    path.quadTo(c, e);
    m_ctrlPt = c;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_t(QPainterPath &path) const
{
    if (m_count < 2)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }

    QPointF e(m_num[0] + m_offsetX, m_num[1] + m_offsetY);
    m_num += 2;
    m_count -= 2;
    QPointF c;
    if (m_lastMode == 'q' || m_lastMode == 'Q' || m_lastMode == 't' || m_lastMode == 'T')
    {
        c = QPointF(2 * m_x - m_ctrlPt.x(), 2 * m_y - m_ctrlPt.y());
    }
    else
    {
        c = QPointF(m_x, m_y);
    }
    path.quadTo(c, e);
    m_ctrlPt = c;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_T(QPainterPath &path) const
{
    if (m_count < 2)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }

    QPointF e(m_num[0], m_num[1]);
    m_num += 2;
    m_count -= 2;
    QPointF c;
    if (m_lastMode == 'q' || m_lastMode == 'Q' || m_lastMode == 't' || m_lastMode == 'T')
    {
        c = QPointF(2 * m_x - m_ctrlPt.x(), 2 * m_y - m_ctrlPt.y());
    }
    else
    {
        c = QPointF(m_x, m_y);
    }
    path.quadTo(c, e);
    m_ctrlPt = c;
    m_x = e.x();
    m_y = e.y();
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_a(QPainterPath &path) const
{
    if (m_count < 7)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }

    qreal rx = (*m_num++);
    qreal ry = (*m_num++);
    qreal xAxisRotation = (*m_num++);
    qreal largeArcFlag = (*m_num++);
    qreal sweepFlag = (*m_num++);
    qreal ex = (*m_num++) + m_offsetX;
    qreal ey = (*m_num++) + m_offsetY;
    m_count -= 7;
    qreal curx = m_x;
    qreal cury = m_y;
    PathArc(path, rx, ry, xAxisRotation, static_cast<int>(largeArcFlag), static_cast<int>(sweepFlag), ex, ey, curx,
            cury);
    m_x = ex;
    m_y = ey;
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::Command_A(QPainterPath &path) const
{
    if (m_count < 7)
    {
        m_num += m_count;
        m_count = 0;
        return;
    }
    qreal rx = (*m_num++);
    qreal ry = (*m_num++);
    qreal xAxisRotation = (*m_num++);
    qreal largeArcFlag = (*m_num++);
    qreal sweepFlag = (*m_num++);
    qreal ex = (*m_num++);
    qreal ey = (*m_num++);
    m_count -= 7;
    qreal curx = m_x;
    qreal cury = m_y;
    PathArc(path, rx, ry, xAxisRotation, static_cast<int>(largeArcFlag), static_cast<int>(sweepFlag), ex, ey, curx,
            cury);
    m_x = ex;
    m_y = ey;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSVGPathTokenizer::GetCommands() const -> QList<VSVGPathCommand>
{
    QMutexLocker locker(svgPathTokenizerMutex());

    return m_commands;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSVGPathTokenizer::GetSinglePath() const -> bool
{
    return m_singlePath;
}

//---------------------------------------------------------------------------------------------------------------------
void VSVGPathTokenizer::SetSinglePath(bool newSinglePath)
{
    m_singlePath = newSinglePath;
}
