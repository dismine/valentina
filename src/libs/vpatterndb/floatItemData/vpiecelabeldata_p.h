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

#ifndef VPIECELABELDATA_P_H
#define VPIECELABELDATA_P_H

#include <QSharedData>
#include <QString>
#include <QVector>

#include "../ifc/ifcdef.h"
#include "../vmisc/defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPieceLabelDataPrivate : public QSharedData
{
public:
    VPieceLabelDataPrivate() = default;
    VPieceLabelDataPrivate(const VPieceLabelDataPrivate &data) = default;
    ~VPieceLabelDataPrivate() = default;

    /** @brief m_qsLetter Detail letter (should be no more than 3 characters) */
    QString m_qsLetter{};      // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_annotation{};    // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_orientation{};   // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_rotationWay{};   // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_tilt{};          // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_foldPosition{};  // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_areaShortName{}; // NOLINT(misc-non-private-member-variables-in-classes)

    quint16 m_quantity{1}; // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_onFold{false};  // NOLINT(misc-non-private-member-variables-in-classes)

    QVector<VLabelTemplateLine> m_lines{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPieceLabelDataPrivate) // NOLINT
};

QT_WARNING_POP

#endif // VPIECELABELDATA_P_H
