/************************************************************************
 **
 **  @file   vpiecearea.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 11, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef VPIECEAREA_H
#define VPIECEAREA_H

#include "vinternalvariable.h"

class VPiece;
class VContainer;
class VPieceAreaData;

enum class PieceAreaType : qint8 { External, SeamLine };

class VPieceArea final :public VInternalVariable
{
public:
    VPieceArea();
    explicit VPieceArea(PieceAreaType type, quint32 pieceId, const VPiece &piece, const VContainer *data, Unit unit);
    VPieceArea(const VPieceArea &var) = default;
    ~VPieceArea() override;

    auto operator=(const VPieceArea &var) -> VPieceArea &;
#ifdef Q_COMPILER_RVALUE_REFS
    VPieceArea(VPieceArea &&var) Q_DECL_NOTHROW;
    auto operator=(VPieceArea &&var) Q_DECL_NOTHROW -> VPieceArea &;
#endif

    void SetValue(quint32 pieceId, const VPiece &piece, const VContainer *data, Unit unit);

    auto GetPieceId() const -> quint32;

    static auto PieceShortName(const VPiece &piece) -> QString;
private:
    QSharedDataPointer<VPieceAreaData> d;
};

Q_DECLARE_TYPEINFO(VPieceArea, Q_MOVABLE_TYPE); // NOLINT

#endif // VPIECEAREA_H
