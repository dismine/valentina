/************************************************************************
 **
 **  @file   vcontour_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 1, 2015
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

#ifndef VCONTOUR_P_H
#define VCONTOUR_P_H

#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QSharedData>
#include <QVector>

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VContourData : public QSharedData
{
public:
    VContourData() = default;
    VContourData(int height, int width, qreal layoutWidth);
    VContourData(const VContourData &contour) = default;
    ~VContourData() = default;

    /** @brief globalContour list of global points contour. */
    QVector<QPointF> globalContour{}; // NOLINT (misc-non-private-member-variables-in-classes)

    /** @brief paperHeight height of paper in pixels*/
    int paperHeight{0}; // NOLINT (misc-non-private-member-variables-in-classes)

    /** @brief paperWidth width of paper in pixels*/
    int paperWidth{0}; // NOLINT (misc-non-private-member-variables-in-classes)

    qreal shift{0}; // NOLINT (misc-non-private-member-variables-in-classes)

    qreal layoutWidth{0}; // NOLINT (misc-non-private-member-variables-in-classes)

    vsizetype m_emptySheetEdgesCount{0}; // NOLINT (misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VContourData) // NOLINT
};

// cppcheck-suppress unknownMacro
QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VContourData::VContourData(int height, int width, qreal layoutWidth)
  : paperHeight(height),
    paperWidth(width),
    layoutWidth(layoutWidth)
{
}

#endif // VCONTOUR_P_H
