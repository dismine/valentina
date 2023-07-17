/************************************************************************
 **
 **  @file   dxfdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 8, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef DXFDEF_H
#define DXFDEF_H

#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

enum class VarMeasurement : quint8
{
    English = 0,
    Metric = 1
};

// Default drawing units for AutoCAD DesignCenter blocks:
enum class VarInsunits : quint8
{
    Inches = 1,
    Millimeters = 4,
    Centimeters = 5
};

// Helps mark end of string. See VDxfEngine::drawTextItem for more details
extern const QString endStringPlaceholder;

Q_REQUIRED_RESULT static inline auto DL_FuzzyComparePossibleNulls(double p1, double p2) -> bool;
static inline auto DL_FuzzyComparePossibleNulls(double p1, double p2) -> bool
{
    if (qFuzzyIsNull(p1))
    {
        return qFuzzyIsNull(p2);
    }
    if (qFuzzyIsNull(p2))
    {
        return false;
    }
    else
    {
        return qFuzzyCompare(p1, p2);
    }
}

auto LocaleMap() -> QMap<QString, QString>;

#endif // DXFDEF_H
