/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VPIECE_P_H
#define VPIECE_P_H

#include <QSharedData>
#include <QVector>

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/def.h"
#include "vpiecepath.h"
#include "floatItemData/vpiecelabeldata.h"
#include "floatItemData/vpatternlabeldata.h"
#include "floatItemData/vgrainlinedata.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPieceData : public QSharedData
{
public:
    explicit VPieceData(PiecePathType type)
        : m_path(type)
    {}

    VPieceData(const VPieceData &detail) = default;

    ~VPieceData() = default;

    /** @brief nodes list detail nodes. */
    VPiecePath m_path; // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_inLayout{true}; // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_united{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    QVector<CustomSARecord> m_customSARecords{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<quint32>        m_internalPaths{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<quint32>        m_pins{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<quint32>        m_placeLabels{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief Pattern piece data */
    VPieceLabelData m_ppData{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief Pattern info coordinates */
    VPatternLabelData m_piPatternInfo{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_glGrainline grainline geometry object*/
    VGrainlineData m_glGrainline{}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_formulaWidth{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_gradationLabel{}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_shortName{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPieceData) // NOLINT
};

QT_WARNING_POP

#endif // VPIECE_P_H

