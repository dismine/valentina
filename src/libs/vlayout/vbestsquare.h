/************************************************************************
 **
 **  @file   vbestsquare.h
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

#ifndef VBESTSQUARE_H
#define VBESTSQUARE_H

#include <QSizeF>
#include <QTransform>
#include <QtGlobal>
#include <QSharedDataPointer>
#include <QTypeInfo>

#include "vlayoutdef.h"

class VBestSquareData;

class VBestSquare final
{
public:
    VBestSquare();
    VBestSquare(QSizeF sheetSize, bool saveLength, bool isPortrait);
    VBestSquare(const VBestSquare &res);
    ~VBestSquare();

    auto operator=(const VBestSquare &res) -> VBestSquare &;
#ifdef Q_COMPILER_RVALUE_REFS
    VBestSquare(VBestSquare &&res) noexcept;
    auto operator=(VBestSquare &&res) noexcept->VBestSquare &;
#endif

    void NewResult(const VBestSquareResData &data);
    void NewResult(const VBestSquare &best);

    auto BestSize() const -> QSizeF;
    auto GContourEdge() const -> int;
    auto DetailEdge() const -> int;
    auto Matrix() const -> QTransform;
    auto HasValidResult() const -> bool;
    auto Mirror() const -> bool;
    auto Type() const -> BestFrom;
    auto IsTerminatedByException() const -> bool;
    auto ReasonTerminatedByException() const -> QString;
    void       TerminatedByException(const QString &reason);

    auto BestResultData() const -> VBestSquareResData;

    auto IsSaveLength() const -> bool;
    auto IsImprovedSidePosition(qreal sidePosition) const -> bool;
    auto IsPortrait() const -> bool;

private:
    QSharedDataPointer<VBestSquareData> d;

};

Q_DECLARE_TYPEINFO(VBestSquare, Q_MOVABLE_TYPE); // NOLINT

#endif // VBESTSQUARE_H
