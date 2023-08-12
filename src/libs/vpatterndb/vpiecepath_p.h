/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#ifndef VPIECEPATH_P_H
#define VPIECEPATH_P_H

#include <QSharedData>
#include <QVector>

#include "vpiecenode.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPiecePathData : public QSharedData
{
public:
    VPiecePathData() = default;
    explicit VPiecePathData(PiecePathType type);
    VPiecePathData(const VPiecePathData &path) = default;
    ~VPiecePathData() = default;

    QVector<VPieceNode> m_nodes{};                // NOLINT(misc-non-private-member-variables-in-classes)
    PiecePathType m_type{PiecePathType::Unknown}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_name{};                             // NOLINT(misc-non-private-member-variables-in-classes)
    Qt::PenStyle m_penType{Qt::SolidLine};        // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_cut{false};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_visibilityTrigger{'1'};             // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_firstToCuttingContour{false};          // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_lastToCuttingContour{false};           // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPiecePathData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VPiecePathData::VPiecePathData(PiecePathType type)
  : m_type(type)
{
}

#endif // VPIECEPATH_P_H
