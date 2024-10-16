/************************************************************************
 **
 **  @file   vexceptionundo.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 6, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VEXCEPTIONUNDO_H
#define VEXCEPTIONUNDO_H

#include <QString>

#include "../ifcdef.h"
#include "vexception.h"

class VExceptionUndo : public VException
{
public:
    explicit VExceptionUndo(const QString &what) V_NOEXCEPT_EXPR(true);
    VExceptionUndo(const VExceptionUndo &e) V_NOEXCEPT_EXPR(true);
    ~VExceptionUndo() V_NOEXCEPT_EXPR(true) override = default;

    VExceptionUndo(VExceptionUndo &&) noexcept = default;
    auto operator=(VExceptionUndo &&) noexcept -> VExceptionUndo & = default;

    Q_NORETURN void raise() const override { throw *this; }

    Q_REQUIRED_RESULT auto clone() const -> VExceptionUndo * override { return new VExceptionUndo(*this); }
};

#endif // VEXCEPTIONUNDO_H
