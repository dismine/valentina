/************************************************************************
 **
 **  @file   vbank.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 1, 2015
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

#ifndef VBANK_H
#define VBANK_H

#include <QHash>
#include <QLoggingCategory>
#include <QMap>
#include <QRectF>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/typedef.h"
#include "vlayoutdef.h"
#include "vlayoutpiece.h"

// An annoying char define, from the Windows team in <rpcndr.h>
// #define small char
// http://stuartjames.info/Journal/c--visual-studio-2012-vs2012--win8--converting-projects-up-some-conflicts-i-found.aspx
#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
#pragma push_macro("small")
#undef small
#endif

Q_DECLARE_LOGGING_CATEGORY(lBank) // NOLINT

class VBank
{
    Q_DECLARE_TR_FUNCTIONS(VBank) // NOLINT

public:
    VBank();
    ~VBank() = default;

    auto GetLayoutWidth() const -> qreal;
    void SetLayoutWidth(qreal value);

    auto GetManualPriority() const -> bool;
    void SetManualPriority(bool value);

    auto IsNestQuantity() const -> bool;
    void SetNestQuantity(bool value);

    void SetDetails(const QVector<VLayoutPiece> &details);
    auto GetNext() -> int;
    auto GetDetail(int i) const -> VLayoutPiece;

    void Arranged(int i);
    void NotArranged(int i);

    auto PrepareUnsorted() -> bool;
    auto PrepareDetails(bool togetherWithNotches) -> bool;
    void Reset();
    void SetCaseType(Cases caseType);

    auto AllDetailsCount() const -> vsizetype;
    auto LeftToArrange() const -> vsizetype;
    auto FailedToArrange() const -> vsizetype;

    auto GetBiggestDiagonal() const -> qreal;

    auto IsRotationNeeded() const -> bool;

private:
    Q_DISABLE_COPY_MOVE(VBank) // NOLINT
    QVector<VLayoutPiece> details{};

    QMap<uint, QHash<int, qint64>> unsorted{};
    QMap<uint, QHash<int, qint64>> big{};
    QMap<uint, QHash<int, qint64>> middle{};
    QMap<uint, QHash<int, qint64>> small{};
    QMap<uint, QMultiMap<qint64, int>> desc{};

    QVector<uint> groups{};
    QVector<vidtype> arranged{};

    qreal layoutWidth{0};

    Cases caseType{Cases::CaseDesc};
    bool prepare{false};
    qreal diagonal{0};
    bool m_nestQuantity{false};
    bool m_manualPriority{false};

    void PrepareGroup();

    void PrepareThreeGroups(uint priority);
    void PrepareTwoGroups(uint priority);
    void PrepareDescGroup(uint priority);

    auto GetNextThreeGroups(uint priority) const -> int;
    auto GetNextTwoGroups(uint priority) const -> int;
    auto GetNextDescGroup(uint priority) const -> int;

    void SqMaxMin(qint64 &sMax, qint64 &sMin, uint priority) const;

    auto ArrangedDetail(QMap<uint, QHash<int, qint64>> &container, int i) -> bool;
    auto ArrangedDetail(QMap<uint, QMultiMap<qint64, int>> &container, int i) -> bool;
};

#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
#pragma pop_macro("small")
#endif

#endif // VBANK_H
