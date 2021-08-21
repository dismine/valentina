/************************************************************************
 **
 **  @file   layoutdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 8, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#ifndef LAYOUTDEF_H
#define LAYOUTDEF_H

#include <QPointF>

template <typename T> class QSharedPointer;
template <typename T> class QWeakPointer;

class VPLayout;
using VPLayoutPtr = QSharedPointer<VPLayout>;
using VPLayoutWeakPtr = QWeakPointer<VPLayout>;

class VPPiece;
using VPPiecePtr = QSharedPointer<VPPiece>;
using VPPieceWeakPtr = QWeakPointer<VPPiece>;

class VPSheet;
using VPSheetPtr = QSharedPointer<VPSheet>;
using VPSheetWeakPtr = QWeakPointer<VPSheet>;

enum class GrainlineType : qint8
{
    Vertical,
    Horizontal
};

struct VPTransformationOrigon
{
    QPointF origin{};
    bool    custom{false};
};

#endif // LAYOUTDEF_H
