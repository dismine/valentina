/************************************************************************
 **
 **  @file   vpiecearea.cpp
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
#include "vpiecearea.h"
#include "vpiecearea_p.h"
#include "../vpiece.h"
#include "../vcontainer.h"
#include "vincrement.h"

#include <QRegularExpression>

//---------------------------------------------------------------------------------------------------------------------
VPieceArea::VPieceArea()
    :d(new VPieceAreaData)
{
    SetType(VarType::PieceExternalArea);
}

//---------------------------------------------------------------------------------------------------------------------
VPieceArea::VPieceArea(PieceAreaType type, quint32 pieceId, const VPiece &piece, const VContainer *data, Unit unit)
    :d(new VPieceAreaData(pieceId))
{
    // cppcheck-suppress unknownMacro
    SCASSERT(data != nullptr)

    QString shortName = PieceShortName(piece);

    VContainer tempData = *data;
    auto *currentSA = new VIncrement(&tempData, currentSeamAllowance);
    currentSA->SetFormula(piece.GetSAWidth(), QString().setNum(piece.GetSAWidth()), true);

    tempData.AddVariable(currentSA);

    if (type == PieceAreaType::External)
    {
        SetType(VarType::PieceExternalArea);
        SetName(pieceArea_ + shortName);
        VInternalVariable::SetValue(FromPixel2(piece.ExternalArea(&tempData), unit));
    }
    else
    {
        SetType(VarType::PieceSeamLineArea);
        SetName(pieceSeamLineArea_ + shortName);
        VInternalVariable::SetValue(FromPixel2(piece.SeamLineArea(&tempData), unit));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceArea::operator=(const VPieceArea &var) -> VPieceArea &
{
    if ( &var == this )
    {
        return *this;
    }
    VInternalVariable::operator=(var);
    d = var.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPieceArea::VPieceArea(VPieceArea &&var) Q_DECL_NOTHROW
    :VInternalVariable(std::move(var)), d(std::move(var.d))
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceArea::~VPieceArea() // NOLINT(modernize-use-equals-default)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceArea::operator=(VPieceArea &&var) Q_DECL_NOTHROW -> VPieceArea &
{
    VInternalVariable::operator=(var);
    std::swap(d, var.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VPieceArea::SetValue(quint32 pieceId, const VPiece &piece, const VContainer *data, Unit unit)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(data != nullptr)
    d->m_pieceId = pieceId;
    VInternalVariable::SetValue(FromPixel2(GetType() == VarType::PieceExternalArea ? piece.ExternalArea(data)
                                                                                   : piece.SeamLineArea(data), unit));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceArea::GetPieceId() const -> quint32
{
    return d->m_pieceId;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceArea::PieceShortName(const VPiece &piece) -> QString
{
    QString shortName = piece.GetShortName();
    if (shortName.isEmpty())
    {
        shortName = piece.GetName().replace(QChar(QChar::Space), '_').left(25);
        if (shortName.isEmpty() || not QRegularExpression(VPiece::ShortNameRegExp()).match(shortName).hasMatch())
        {
            shortName = QObject::tr("Unknown", "piece area");
        }
    }

    return shortName;
}
