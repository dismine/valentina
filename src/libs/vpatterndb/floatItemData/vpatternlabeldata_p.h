/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VPATTERNLABELDATA_P_H
#define VPATTERNLABELDATA_P_H

#include <QPointF>
#include <QSharedData>
#include <QString>

#include "../vmisc/defglobal.h"
#include "../vmisc/typedef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPatternLabelDataPrivate : public QSharedData
{
public:
    VPatternLabelDataPrivate() = default;
    VPatternLabelDataPrivate(const VPatternLabelDataPrivate &data) = default;
    ~VPatternLabelDataPrivate() = default;

    /** @brief m_dLabelWidth formula to calculate the width of label */
    QString m_dLabelWidth{}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_dLabelHeight formula to calculate the height of label */
    QString m_dLabelHeight{}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_dLabelAngle formula to calculate the rotation angle of label */
    QString m_dLabelAngle{}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_iFontSize label text base font size */
    int m_iFontSize{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_centerPin center pin id */
    quint32 m_centerPin{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_topLeftPin top left corner pin id */
    quint32 m_topLeftPin{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_bottomRightPin bottom right corner pin id */
    quint32 m_bottomRightPin{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPatternLabelDataPrivate) // NOLINT
};

QT_WARNING_POP

#endif // VPATTERNLABELDATA_P_H
