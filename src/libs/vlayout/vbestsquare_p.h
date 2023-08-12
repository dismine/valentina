/************************************************************************
 **
 **  @file   vbestsquare_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 3, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VBESTSQUARE_P_H
#define VBESTSQUARE_P_H

#include <QSharedData>
#include <QTransform>

#include "../vmisc/defglobal.h"
#include "vlayoutdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VBestSquareData : public QSharedData
{
public:
    VBestSquareData() = default;
    VBestSquareData(const QSizeF &sheetSize, bool saveLength, bool isPortrait);

    VBestSquareData(const VBestSquareData &res) = default;
    ~VBestSquareData() = default;

    bool isValid{false};               // NOLINT (misc-non-private-member-variables-in-classes)
    QSizeF sheetSize{};                // NOLINT (misc-non-private-member-variables-in-classes)
    bool valideResult{false};          // NOLINT (misc-non-private-member-variables-in-classes)
    bool saveLength{false};            // NOLINT (misc-non-private-member-variables-in-classes)
    VBestSquareResData data{};         // NOLINT (misc-non-private-member-variables-in-classes)
    bool isPortrait{true};             // NOLINT (misc-non-private-member-variables-in-classes)
    bool terminatedByException{false}; // NOLINT (misc-non-private-member-variables-in-classes)
    QString exceptionReason{};         // NOLINT (misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VBestSquareData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VBestSquareData::VBestSquareData(const QSizeF &sheetSize, bool saveLength, bool isPortrait)
  : isValid(true),
    sheetSize(sheetSize),
    saveLength(saveLength),
    isPortrait(isPortrait)
{
    data.bestSize = QSizeF(sheetSize.width() + 10, sheetSize.height() + 10);
}

#endif // VBESTSQUARE_P_H
