/************************************************************************
**
**  @file   vlayoutpaper_p.h
**  @author Roman Telezhynskyi <dismine(at)gmail.com>
**  @date   8 1, 2015
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

#ifndef VLAYOUTPAPER_P_H
#define VLAYOUTPAPER_P_H

#include <QPointF>
#include <QSharedData>
#include <QVector>

#include "vcontour.h"
#include "vlayoutpiece.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VLayoutPaperData : public QSharedData
{
public:
    VLayoutPaperData() = default;
    VLayoutPaperData(int height, int width, qreal layoutWidth);
    VLayoutPaperData(const VLayoutPaperData &paper) = default;
    ~VLayoutPaperData() = default;

    /** @brief details list of arranged details. */
    QVector<VLayoutPiece> details{}; // NOLINT (misc-non-private-member-variables-in-classes)

    QVector<VCachedPositions> positionsCache{}; // NOLINT (misc-non-private-member-variables-in-classes)

    /** @brief globalContour list of global points contour. */
    VContour globalContour{}; // NOLINT (misc-non-private-member-variables-in-classes)

    quint32 paperIndex{0};             // NOLINT (misc-non-private-member-variables-in-classes)
    qreal layoutWidth{0};              // NOLINT (misc-non-private-member-variables-in-classes)
    bool globalRotate{true};           // NOLINT (misc-non-private-member-variables-in-classes)
    bool localRotate{true};            // NOLINT (misc-non-private-member-variables-in-classes)
    int globalRotationNumber{2};       // NOLINT (misc-non-private-member-variables-in-classes)
    int localRotationNumber{2};        // NOLINT (misc-non-private-member-variables-in-classes)
    bool saveLength{false};            // NOLINT (misc-non-private-member-variables-in-classes)
    bool followGrainline{false};       // NOLINT (misc-non-private-member-variables-in-classes)
    bool originPaperOrientation{true}; // NOLINT (misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VLayoutPaperData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VLayoutPaperData::VLayoutPaperData(int height, int width, qreal layoutWidth)
  : globalContour(VContour(height, width, layoutWidth)),
    layoutWidth(layoutWidth)
{
}

#endif // VLAYOUTPAPER_P_H
