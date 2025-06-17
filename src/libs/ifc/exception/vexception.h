/************************************************************************
 **
 **  @file   vexception.h
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

#ifndef VEXCEPTION_H
#define VEXCEPTION_H

#include <QCoreApplication>
#include <QException>
#include <QString>

#include "../ifcdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

/**
 * @brief The VException class parent for all exception. Could be use for abstract exception
 */
class VException : public QException
{
    Q_DECLARE_TR_FUNCTIONS(VException) // NOLINT

public:
    explicit VException(const QString &error) V_NOEXCEPT_EXPR(true);
    VException(const VException &e) V_NOEXCEPT_EXPR(true);
    auto operator=(const VException &e) V_NOEXCEPT_EXPR(true) -> VException &;
    ~VException() V_NOEXCEPT_EXPR(true) override = default;

    VException(VException &&) noexcept = default;
    auto operator=(VException &&) noexcept -> VException & = default;

    Q_NORETURN void raise() const override;

    // cppcheck-suppress unusedFunction
    Q_REQUIRED_RESULT auto clone() const -> VException * override;

    virtual auto ErrorMessage() const -> QString;
    virtual auto DetailedInformation() const -> QString;
    auto WhatUtf8() const V_NOEXCEPT_EXPR(true) -> QString;
    void AddMoreInformation(const QString &info);
    auto MoreInformation() const -> QString;

protected:
    /** @brief error string with error */
    QString error;

    /** @brief moreInfo more information about error */
    QString moreInfo{};

    auto MoreInfo(const QString &detInfo) const -> QString;
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief What return string with error
 * @return string with error
 */
inline auto VException::WhatUtf8() const V_NOEXCEPT_EXPR(true) -> QString
{
    return error;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MoreInformation return more information for error
 * @return information
 */
inline auto VException::MoreInformation() const -> QString
{
    return moreInfo;
}

// Want have special exception for catching unhadled deleting a tool
class VExceptionToolWasDeleted : public VException
{
    Q_DECLARE_TR_FUNCTIONS(VExceptionToolDeleted) // NOLINT

public:
    explicit VExceptionToolWasDeleted(const QString &error) V_NOEXCEPT_EXPR(true);
    VExceptionToolWasDeleted(const VExceptionToolWasDeleted &e) V_NOEXCEPT_EXPR(true);
    auto operator=(const VExceptionToolWasDeleted &e) V_NOEXCEPT_EXPR(true) -> VExceptionToolWasDeleted &;
    virtual ~VExceptionToolWasDeleted() V_NOEXCEPT_EXPR(true) = default;

    Q_NORETURN virtual void raise() const override;
    // cppcheck-suppress unusedFunction
    virtual auto clone() const -> VExceptionToolWasDeleted * override;
};

#endif // VEXCEPTION_H
