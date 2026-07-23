/************************************************************************
 **
 **  @file   vslowop.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 7, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef VSLOWOP_H
#define VSLOWOP_H

#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(vTiming) // NOLINT

/**
 * @brief VSlowOp reports a scope that took longer than it should have.
 *
 * Users report freezes we cannot reproduce, and the log currently jumps straight from one action to the next with the
 * stall hidden in between. Wrap a suspect scope in this and anything slower than the threshold names itself in the log,
 * with its duration, on the machine that actually freezes.
 *
 * Quiet by design: a scope that stays under the threshold logs nothing, so this can sit on hot paths.
 */
class VSlowOp
{
public:
    explicit VSlowOp(QString what, qint64 thresholdMs = 200);
    ~VSlowOp();

    VSlowOp(const VSlowOp &) = delete;
    auto operator=(const VSlowOp &) -> VSlowOp & = delete;
    VSlowOp(VSlowOp &&) = delete;
    auto operator=(VSlowOp &&) -> VSlowOp & = delete;

private:
    QString m_what;
    qint64 m_thresholdMs;
    QElapsedTimer m_timer{};
};

#endif // VSLOWOP_H
