/************************************************************************
 **
 **  @file   vexceptionemptyparameter.h
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

#ifndef VEXCEPTIONEMPTYPARAMETER_H
#define VEXCEPTIONEMPTYPARAMETER_H

#include <QString>
#include <QtGlobal>

#include "../vmisc/exception/vexception.h"

class QDomElement;

/**
 * @brief The VExceptionEmptyParameter class for exception empty parameter
 */
class VExceptionEmptyParameter final : public VException
{
public:
    VExceptionEmptyParameter(const QString &what, const QString &name, const QDomElement &domElement)
        V_NOEXCEPT_EXPR(true);
    VExceptionEmptyParameter(const VExceptionEmptyParameter &e) V_NOEXCEPT_EXPR(true);
    auto operator=(const VExceptionEmptyParameter &e) V_NOEXCEPT_EXPR(true) -> VExceptionEmptyParameter &;
    ~VExceptionEmptyParameter() V_NOEXCEPT_EXPR(true) override = default;

    Q_NORETURN void raise() const override { throw *this; }

    Q_REQUIRED_RESULT auto clone() const -> VExceptionEmptyParameter * override
    {
        return new VExceptionEmptyParameter(*this);
    }

    auto ErrorMessage() const -> QString override;
    auto DetailedInformation() const -> QString override;
    auto Name() const -> QString;
    auto TagText() const -> QString;
    auto TagName() const -> QString;
    auto LineNumber() const -> qint32;

private:
    /** @brief name name attribute */
    QString name;

    /** @brief tagText tag text */
    QString tagText;

    /** @brief tagName tag name */
    QString tagName;

    /** @brief lineNumber line number */
    qint32 lineNumber;
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Name return name of attribute where error
 * @return name
 */
inline auto VExceptionEmptyParameter::Name() const -> QString
{
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TagText return tag text
 * @return tag text
 */
inline auto VExceptionEmptyParameter::TagText() const -> QString
{
    return tagText;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TagName return tag name
 * @return tag name
 */
inline auto VExceptionEmptyParameter::TagName() const -> QString
{
    return tagName;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LineNumber return line number of tag
 * @return line number
 */
inline auto VExceptionEmptyParameter::LineNumber() const -> qint32
{
    return lineNumber;
}

#endif // VEXCEPTIONEMPTYPARAMETER_H
