/************************************************************************
 **
 **  @file   vexceptionconversionerror.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VEXCEPTIONCONVERSIONERROR_H
#define VEXCEPTIONCONVERSIONERROR_H

#include <QString>

#include "../ifcdef.h"
#include "vexception.h"

/**
 * @brief The VExceptionConversionError class for exception of conversion error
 */
class VExceptionConversionError final : public VException
{
public:
    VExceptionConversionError(const QString &error, const QString &str) V_NOEXCEPT_EXPR(true);
    VExceptionConversionError(const VExceptionConversionError &e) V_NOEXCEPT_EXPR(true);
    auto operator=(const VExceptionConversionError &e) V_NOEXCEPT_EXPR(true) -> VExceptionConversionError &;
    virtual ~VExceptionConversionError() V_NOEXCEPT_EXPR(true) = default;

    Q_NORETURN virtual void raise() const override { throw *this; }

    Q_REQUIRED_RESULT virtual auto clone() const -> VExceptionConversionError * override
    {
        return new VExceptionConversionError(*this);
    }

    virtual auto ErrorMessage() const -> QString override;
    auto String() const -> QString;

private:
    /** @brief str string, where happend error */
    QString str;
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief String return string, where happend error
 * @return string
 */
inline auto VExceptionConversionError::String() const -> QString
{
    return str;
}

#endif // VEXCEPTIONCONVERSIONERROR_H
