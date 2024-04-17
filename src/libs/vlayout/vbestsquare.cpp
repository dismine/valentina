/************************************************************************
 **
 **  @file   vbestsquare.cpp
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

#include "vbestsquare.h"
#include "../vgeometry/vgeometrydef.h"
#include "vbestsquare_p.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto Square(QSizeF size) -> qint64
{
    return static_cast<qint64>(size.width() * size.height());
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
VBestSquare::VBestSquare()
  : d(new VBestSquareData())
{
}

//---------------------------------------------------------------------------------------------------------------------
VBestSquare::VBestSquare(QSizeF sheetSize, bool saveLength, bool isPortrait)
  : d(new VBestSquareData(sheetSize, saveLength, isPortrait))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VBestSquare)

//---------------------------------------------------------------------------------------------------------------------
VBestSquare::~VBestSquare() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::operator=(const VBestSquare &res) -> VBestSquare &
{
    if (&res == this)
    {
        return *this;
    }
    d = res.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VBestSquare::VBestSquare(VBestSquare &&res) noexcept
  : d(std::move(res.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::operator=(VBestSquare &&res) noexcept -> VBestSquare &
{
    std::swap(d, res.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
void VBestSquare::NewResult(const VBestSquareResData &data)
{
    auto SaveResult = [this, &data]()
    {
        d->valideResult = true;
        d->data = data;
    };

    const qint64 candidateSquare = Square(data.bestSize);

    if (candidateSquare > 0 && data.type >= d->data.type && candidateSquare <= Square(d->data.bestSize))
    {
        if (not HasValidResult())
        {
            SaveResult(); // First result
        }
        else
        {
            if (d->saveLength)
            {
                if ((VFuzzyOnAxis(data.depthPosition, d->data.depthPosition) &&
                     IsImprovedSidePosition(data.sidePosition)) ||
                    data.depthPosition < d->data.depthPosition)
                {
                    SaveResult();
                }
            }
            else
            {
                if (IsImprovedSidePosition(data.sidePosition) || VFuzzyOnAxis(data.sidePosition, d->data.sidePosition))
                {
                    SaveResult();
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBestSquare::NewResult(const VBestSquare &best)
{
    if (best.d->isValid && best.HasValidResult() && d->saveLength == best.IsSaveLength())
    {
        NewResult(best.BestResultData());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::BestSize() const -> QSizeF
{
    return d->data.bestSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::GContourEdge() const -> int
{
    return d->data.globalI;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::DetailEdge() const -> int
{
    return d->data.detJ;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::Matrix() const -> QTransform
{
    return d->data.resMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::HasValidResult() const -> bool
{
    return d->valideResult;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::Mirror() const -> bool
{
    return d->data.resMirror;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::Type() const -> BestFrom
{
    return d->data.type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::IsTerminatedByException() const -> bool
{
    return d->terminatedByException;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::ReasonTerminatedByException() const -> QString
{
    return d->exceptionReason;
}

//---------------------------------------------------------------------------------------------------------------------
void VBestSquare::TerminatedByException(const QString &reason)
{
    d->valideResult = false;
    d->terminatedByException = true;
    d->exceptionReason = reason;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::BestResultData() const -> VBestSquareResData
{
    return d->data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::IsSaveLength() const -> bool
{
    return d->saveLength;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::IsImprovedSidePosition(qreal sidePosition) const -> bool
{
    const bool lessThan = d->data.sidePosition < sidePosition;
    const bool greaterThan = d->data.sidePosition > sidePosition;

    return IsPortrait() ? greaterThan : lessThan;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBestSquare::IsPortrait() const -> bool
{
    return d->isPortrait;
}
