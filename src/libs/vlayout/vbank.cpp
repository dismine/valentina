/************************************************************************
 **
 **  @file   vbank.cpp
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

#include "vbank.h"

#include <QLoggingCategory>
#include <climits>

#include "../ifc/exception/vexception.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractvalapplication.h"
#include "vlayoutdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(lBank, "layout.bank") // NOLINT

QT_WARNING_POP

// An annoying char define, from the Windows team in <rpcndr.h>
// #define small char
// http://stuartjames.info/Journal/c--visual-studio-2012-vs2012--win8--converting-projects-up-some-conflicts-i-found.aspx
#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
#pragma push_macro("small")
#undef small
#endif

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto HandleSymmetricalCopy(const VLayoutPiece &piece, int copyIndex) -> VLayoutPiece
{
    VLayoutPiece copy = piece;

    if (copyIndex % 2 == 1) // Flip every even-numbered copy
    {
        if (piece.IsForceFlipping())
        {
            copy.SetForbidFlipping(true);
        }
        else if (piece.IsForbidFlipping())
        {
            copy.SetForceFlipping(true);
        }
    }

    return copy;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareQuantity(const QVector<VLayoutPiece> &details) -> QVector<VLayoutPiece>
{
    QVector<VLayoutPiece> withQuantity;
    withQuantity.reserve(details.size());
    for (const auto &piece : details)
    {
        if (const int quantity = piece.GetQuantity(); quantity > 1)
        {
            for (int i = 0; i < quantity; ++i)
            {
                if (piece.IsSymmetricalCopy() && (piece.IsForceFlipping() || piece.IsForbidFlipping()))
                {
                    withQuantity.append(HandleSymmetricalCopy(piece, i));
                }
                else
                {
                    withQuantity.append(piece);
                }
            }
        }
        else
        {
            withQuantity.append(piece);
        }
    }

    return withQuantity;
}

//---------------------------------------------------------------------------------------------------------------------
void Insert(QMap<uint, QHash<int, qint64>> &container, uint key, int valKey, qint64 valValue)
{
    QHash<int, qint64> containerSequence = container.value(key);
    containerSequence.insert(valKey, valValue);
    container.insert(key, containerSequence);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto CountDetails(const T &container) -> vsizetype
{
    vsizetype count = 0;
    auto i = container.constBegin();
    while (i != container.constEnd())
    {
        count += i.value().count();
        ++i;
    }

    return count;
}

//---------------------------------------------------------------------------------------------------------------------
auto NotArrangedDetail(QMap<uint, QHash<int, qint64>> &container, QMap<uint, QHash<int, qint64>> &unsorted, int i)
    -> bool
{
    QMutableMapIterator iterator(container);
    while (iterator.hasNext())
    {
        iterator.next();
        auto containerGroup = container.value(iterator.key());
        if (containerGroup.contains(i))
        {
            Insert(unsorted, iterator.key(), i, containerGroup.value(i));
            containerGroup.remove(i);

            if (not containerGroup.isEmpty())
            {
                container.insert(iterator.key(), containerGroup);
            }
            else
            {
                container.remove(iterator.key());
            }
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto NotArrangedDetail(QMap<uint, QMultiMap<qint64, int>> &container, QMap<uint, QHash<int, qint64>> &unsorted, int i)
    -> bool
{
    QMutableMapIterator iterator(container);
    while (iterator.hasNext())
    {
        iterator.next();
        auto containerGroup = container.value(iterator.key());
        auto detailIterator = std::find(containerGroup.begin(), containerGroup.end(), i);
        if (detailIterator != containerGroup.end())
        {
            Insert(unsorted, iterator.key(), i, detailIterator.key());
            containerGroup.erase(detailIterator);

            if (not containerGroup.isEmpty())
            {
                container.insert(iterator.key(), containerGroup);
            }
            else
            {
                container.remove(iterator.key());
            }
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto TakeFirstForPriority(const QMap<uint, QHash<int, qint64>> &container, uint priority) -> int
{
    if (const QHash<int, qint64> priorityGroup = container.value(priority); not priorityGroup.isEmpty())
    {
        return priorityGroup.constBegin().key();
    }

    return -1;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VBank::VBank()
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetLayoutWidth() const -> qreal
{
    return layoutWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::SetLayoutWidth(qreal value)
{
    layoutWidth = value;
    Reset();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetManualPriority() const -> bool
{
    return m_manualPriority;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::SetManualPriority(bool value)
{
    m_manualPriority = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::IsNestQuantity() const -> bool
{
    return m_nestQuantity;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::SetNestQuantity(bool value)
{
    m_nestQuantity = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::SetDetails(const QVector<VLayoutPiece> &details)
{
    this->details = details;
    Reset();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetNext() -> int
{
    if (not prepare)
    {
        return -1;
    }

    if (LeftToArrange() == 0)
    {
        if (CountDetails(unsorted) == 0)
        {
            return -1;
        }
        else
        {
            PrepareGroup();
        }
    }

    auto GetNextInGroup = [this](uint group)
    {
        switch (caseType)
        {
            case Cases::CaseThreeGroup:
                return GetNextThreeGroups(group);
            case Cases::CaseTwoGroup:
                return GetNextTwoGroups(group);
            case Cases::CaseDesc:
                return GetNextDescGroup(group);
            default:
                return -1;
        }
    };

    for (auto &group : groups)
    {
        int next = -1;
        if (group != 0) // Group 0 must go last
        {
            next = GetNextInGroup(group);
        }

        if (next != -1)
        {
            return next;
        }
    }

    if (groups.contains(0U))
    {
        return GetNextInGroup(0);
    }

    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetDetail(int i) const -> VLayoutPiece
{
    if (i >= 0 && i < details.size())
    {
        return details.at(i);
    }
    else
    {
        return VLayoutPiece();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::Arranged(int i)
{
    if (ArrangedDetail(big, i))
    {
        return;
    }

    if (ArrangedDetail(middle, i))
    {
        return;
    }

    if (ArrangedDetail(small, i))
    {
        return;
    }

    ArrangedDetail(desc, i);
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::NotArranged(int i)
{
    if (NotArrangedDetail(big, unsorted, i))
    {
        return;
    }

    if (NotArrangedDetail(middle, unsorted, i))
    {
        return;
    }

    if (NotArrangedDetail(small, unsorted, i))
    {
        return;
    }

    NotArrangedDetail(desc, unsorted, i);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::PrepareUnsorted() -> bool
{
    QSet<uint> uniqueGroup;

    for (int i = 0; i < details.size(); ++i)
    {
        const qint64 square = details.at(i).Square();
        if (square <= 0)
        {
            qCCritical(lBank)
                << VAbstractValApplication::warningMessageSignature +
                       tr("Error of preparing data for layout: Detail '%1' square <= 0").arg(details.at(i).GetName());
            prepare = false;
            return prepare;
        }
        const uint group = m_manualPriority ? details.at(i).GetPriority() : 0;
        uniqueGroup.insert(group);
        Insert(unsorted, group, i, square);
    }

    groups = ConvertToVector(uniqueGroup);
    std::sort(groups.begin(), groups.end());

    PrepareGroup();

    prepare = true;
    return prepare;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::PrepareDetails(bool togetherWithNotches) -> bool
{
    if (layoutWidth <= 0)
    {
        qCCritical(lBank) << VAbstractValApplication::warningMessageSignature +
                                 tr("Error of preparing data for layout: Layout paper sheet <= 0");
        prepare = false;
        return prepare;
    }

    if (details.isEmpty())
    {
        qCCritical(lBank) << VAbstractValApplication::warningMessageSignature +
                                 tr("Error of preparing data for layout: List of details is empty");
        prepare = false;
        return prepare;
    }

    if (m_nestQuantity)
    {
        details = PrepareQuantity(details);
    }

    diagonal = 0;

    for (auto &detail : details)
    {
        detail.SetLayoutWidth(layoutWidth);
        detail.SetLayoutAllowancePoints(togetherWithNotches);

        if (not detail.IsLayoutAllowanceValid(togetherWithNotches))
        {
            const QString errorMsg = QObject::tr("Piece '%1' has invalid layout allowance. Please, check seam allowance"
                                                 " to check how seam allowance behave.")
                                         .arg(detail.GetName());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }

        const qreal d = detail.Diagonal();
        if (d > diagonal)
        {
            diagonal = d;
        }
    }

    prepare = true;
    return prepare;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::Reset()
{
    prepare = false;
    unsorted.clear();
    big.clear();
    middle.clear();
    small.clear();
    desc.clear();
    groups.clear();
    arranged.clear();
    diagonal = 0;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::SetCaseType(Cases caseType)
{
    this->caseType = caseType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::AllDetailsCount() const -> vsizetype
{
    return CountDetails(unsorted) + CountDetails(big) + CountDetails(middle) + CountDetails(small) + CountDetails(desc);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::LeftToArrange() const -> vsizetype
{
    return CountDetails(big) + CountDetails(middle) + CountDetails(small) + CountDetails(desc);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::FailedToArrange() const -> vsizetype
{
    return CountDetails(unsorted);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetBiggestDiagonal() const -> qreal
{
    return diagonal;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::PrepareGroup()
{
    QMutableMapIterator i(unsorted);
    while (i.hasNext())
    {
        i.next();
        switch (caseType)
        {
            case Cases::CaseThreeGroup:
                PrepareThreeGroups(i.key());
                break;
            case Cases::CaseTwoGroup:
                PrepareTwoGroups(i.key());
                break;
            case Cases::CaseDesc:
                PrepareDescGroup(i.key());
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::PrepareThreeGroups(uint priority)
{
    qint64 sMax = LLONG_MIN;
    qint64 sMin = LLONG_MAX;

    SqMaxMin(sMax, sMin, priority);

    const qint64 s1 = sMax - (sMax - sMin) / 3;
    const qint64 s2 = sMin + (sMax - sMin) / 3;

    const QHash<int, qint64> usortedGroup = unsorted.value(priority);
    QHash<int, qint64>::const_iterator i = usortedGroup.constBegin();
    while (i != usortedGroup.constEnd())
    {
        if (i.value() > s1)
        {
            Insert(big, priority, i.key(), i.value());
        }
        else if (s1 >= i.value() && i.value() > s2)
        {
            Insert(middle, priority, i.key(), i.value());
        }
        else
        {
            Insert(small, priority, i.key(), i.value());
        }
        ++i;
    }
    unsorted.remove(priority);
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::PrepareTwoGroups(uint priority)
{
    qint64 sMax = LLONG_MIN;
    qint64 sMin = LLONG_MAX;

    SqMaxMin(sMax, sMin, priority);

    const qint64 s = (sMax + sMin) / 2;
    const QHash<int, qint64> usortedGroup = unsorted.value(priority);
    QHash<int, qint64>::const_iterator i = usortedGroup.constBegin();
    while (i != usortedGroup.constEnd())
    {
        if (i.value() >= s)
        {
            Insert(big, priority, i.key(), i.value());
        }
        else
        {
            Insert(small, priority, i.key(), i.value());
        }
        ++i;
    }
    unsorted.remove(priority);
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::PrepareDescGroup(uint priority)
{
    QMultiMap<qint64, int> descGroup;
    const QHash<int, qint64> usortedGroup = unsorted.value(priority);
    QHash<int, qint64>::const_iterator i = usortedGroup.constBegin();
    while (i != usortedGroup.constEnd())
    {
        descGroup.insert(i.value(), i.key());
        ++i;
    }
    desc.insert(priority, descGroup);
    unsorted.remove(priority);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetNextThreeGroups(uint priority) const -> int
{
    int next = TakeFirstForPriority(big, priority);
    if (next != -1)
    {
        return next;
    }

    next = TakeFirstForPriority(middle, priority);
    if (next != -1)
    {
        return next;
    }

    next = TakeFirstForPriority(small, priority);
    return next;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetNextTwoGroups(uint priority) const -> int
{
    int next = TakeFirstForPriority(big, priority);
    if (next != -1)
    {
        return next;
    }

    next = TakeFirstForPriority(small, priority);
    return next;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::GetNextDescGroup(uint priority) const -> int
{
    auto descGroup = desc.value(priority);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMultiMapIterator i(descGroup);
#else
    QMapIterator i(descGroup);
#endif
    i.toBack();
    if (i.hasPrevious())
    {
        return i.previous().value();
    }

    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
void VBank::SqMaxMin(qint64 &sMax, qint64 &sMin, uint priority) const
{
    sMax = LLONG_MIN;
    sMin = LLONG_MAX;

    const QHash<int, qint64> usortedGroup = unsorted.value(priority);
    QHash<int, qint64>::const_iterator i = usortedGroup.constBegin();
    while (i != usortedGroup.constEnd())
    {
        if (i.value() < sMin)
        {
            sMin = i.value();
        }

        if (i.value() > sMax)
        {
            sMax = i.value();
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::ArrangedDetail(QMap<uint, QHash<int, qint64>> &container, int i) -> bool
{
    QMutableMapIterator iterator(container);
    while (iterator.hasNext())
    {
        iterator.next();
        auto containerGroup = container.value(iterator.key());
        if (containerGroup.contains(i))
        {
            arranged.append(details.at(i).GetId());
            containerGroup.remove(i);
            if (not containerGroup.isEmpty())
            {
                container.insert(iterator.key(), containerGroup);
            }
            else
            {
                container.remove(iterator.key());
            }
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::ArrangedDetail(QMap<uint, QMultiMap<qint64, int>> &container, int i) -> bool
{
    QMutableMapIterator iterator(container);
    while (iterator.hasNext())
    {
        iterator.next();
        auto containerGroup = container.value(iterator.key());
        auto detailIterator = std::find(containerGroup.begin(), containerGroup.end(), i);
        if (detailIterator != containerGroup.end())
        {
            arranged.append(details.at(detailIterator.value()).GetId());
            containerGroup.erase(detailIterator);

            if (not containerGroup.isEmpty())
            {
                container.insert(iterator.key(), containerGroup);
            }
            else
            {
                container.remove(iterator.key());
            }
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBank::IsRotationNeeded() const -> bool
{
    for (const auto &piece : details)
    {
        if (not piece.IsGrainlineEnabled())
        {
            return true;
        }
    }
    return false;
}

#if defined(Q_OS_WIN) && defined(Q_CC_MSVC)
#pragma pop_macro("small")
#endif
