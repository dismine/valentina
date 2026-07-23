/************************************************************************
 **
 **  @file   vslowop.cpp
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
#include "vslowop.h"

#include <utility>

Q_LOGGING_CATEGORY(vTiming, "v.timing") // NOLINT

//---------------------------------------------------------------------------------------------------------------------
VSlowOp::VSlowOp(QString what, qint64 thresholdMs)
  : m_what(std::move(what)),
    m_thresholdMs(thresholdMs)
{
    m_timer.start();
}

//---------------------------------------------------------------------------------------------------------------------
VSlowOp::~VSlowOp()
{
    if (const qint64 elapsed = m_timer.elapsed(); elapsed >= m_thresholdMs)
    {
        // Info rather than warning on purpose: the message handler turns warnings into a modal dialog in GUI mode, and
        // popping one up mid-freeze would be worse than the freeze.
        qCInfo(vTiming, "%s", qUtf8Printable(QStringLiteral("SLOW: %1 took %2 ms.").arg(m_what).arg(elapsed)));
    }
}
