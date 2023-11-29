/************************************************************************
 **
 **  @file   vboundary.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 11, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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

#include "vboundary.h"
#include "../ifc/exception/vexception.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/vabstractapplication.h"
#include "vlayoutpiecepath.h"

#include <QPoint>
#include <QtDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void FillSequance(VBoundarySequenceItemData itemData, QList<VBoundarySequenceItemData> &sequence)
{
    if (itemData.number <= 0)
    {
        itemData.number = 0;
        sequence.append(itemData);
    }
    else if (sequence.isEmpty())
    {
        sequence.append(itemData);
    }
    else
    {
        for (int i = 0; i < sequence.size(); ++i)
        {
            if (sequence.at(i).number > itemData.number || sequence.at(i).number == 0)
            {
                sequence.insert(i, itemData);
                return;
            }
        }

        sequence.append(itemData);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareSequenceItem(const QVector<VLayoutPoint> &path, bool drawMode, VBoundarySequenceItem type)
    -> VBoundarySequenceItemData
{
    VLayoutPiecePath countur(path);
    countur.SetCutPath(!drawMode);

    VBoundarySequenceItemData itemData;
    itemData.item = QVariant::fromValue(countur);
    itemData.type = type;

    return itemData;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareTPassmarkShape(const VLayoutPassmark &passmark, bool drawMode) -> QVector<QVector<VLayoutPoint>>
{
    QVector<VLayoutPoint> shape1;
    if (passmark.lines.isEmpty())
    {
        return {};
    }

    auto TurnPoint = [](QPointF point)
    {
        VLayoutPoint p(point);
        p.SetTurnPoint(true);
        return p;
    };

    QLineF line1 = passmark.lines.constFirst();
    shape1.append(TurnPoint(line1.p1()));
    shape1.append(TurnPoint(line1.p2()));

    if (passmark.lines.size() <= 1)
    {
        return {shape1};
    }

    const QLineF &line2 = passmark.lines.constLast();

    if (!drawMode)
    {
        shape1.append(TurnPoint(line2.p1()));
        shape1.append(TurnPoint(line2.p2()));
        shape1.append(TurnPoint(line1.p2()));

        return {shape1};
    }

    QVector<VLayoutPoint> shape2;
    shape2.append(TurnPoint(line2.p1()));
    shape2.append(TurnPoint(line2.p2()));
    return {shape1, shape2};
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareNoneBreakingPassmarkShape(const VLayoutPassmark &passmark) -> QVector<QVector<VLayoutPoint>>
{
    auto TurnPoint = [](QPointF point)
    {
        VLayoutPoint p(point);
        p.SetTurnPoint(true);
        return p;
    };

    QVector<VLayoutPoint> shape;
    shape.reserve(passmark.lines.size() + 1);
    for (int i = 0; i < passmark.lines.size(); ++i)
    {
        const QLineF &line = passmark.lines.at(i);
        shape.append(TurnPoint(line.p1()));

        if (passmark.lines.size() - 1 == i)
        {
            shape.append(TurnPoint(line.p2()));
        }
    }

    return {shape};
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareUPassmarkShape(const VLayoutPassmark &passmark) -> QVector<QVector<VLayoutPoint>>
{
    auto LayoutPoint = [](QPointF point, bool turnPoint, bool curvePoint)
    {
        VLayoutPoint p(point);
        p.SetTurnPoint(turnPoint);
        p.SetCurvePoint(curvePoint);
        return p;
    };

    qreal radius = QLineF(passmark.baseLine.p1(), passmark.lines.constFirst().p1()).length();

    if (passmark.baseLine.length() - radius > accuracyPointOnLine)
    {
        QVector<QLineF> lines = passmark.lines;
        if (lines.size() < 3)
        {
            return {};
        }

        QLineF line1 = lines.takeFirst();

        QVector<VLayoutPoint> shape;
        shape.reserve(4 + passmark.lines.size() + 1);

        shape.append(LayoutPoint(line1.p1(), true, false));
        shape.append(LayoutPoint(line1.p2(), true, true));

        QLineF line2 = lines.takeLast();

        for (int i = 0; i < passmark.lines.size(); ++i)
        {
            const QLineF &line = passmark.lines.at(i);
            shape.append(LayoutPoint(line.p1(), false, true));

            if (passmark.lines.size() - 1 == i)
            {
                shape.append(LayoutPoint(line.p2(), false, true));
            }
        }

        shape.append(LayoutPoint(line2.p1(), true, true));
        shape.append(LayoutPoint(line2.p2(), true, false));

        return {shape};
    }

    QVector<VLayoutPoint> shape;
    shape.reserve(passmark.lines.size() + 1);
    for (int i = 0; i < passmark.lines.size(); ++i)
    {
        const QLineF &line = passmark.lines.at(i);
        shape.append(LayoutPoint(line.p1(), false, true));

        if (passmark.lines.size() - 1 == i)
        {
            shape.append(LayoutPoint(line.p2(), false, true));
        }
    }

    if (!shape.isEmpty())
    {
        shape.first().SetTurnPoint(true);
        shape.last().SetTurnPoint(true);
    }

    return {shape};
}

//---------------------------------------------------------------------------------------------------------------------
auto PreparePassmarkShape(const VLayoutPassmark &passmark, bool drawMode) -> QVector<QVector<VLayoutPoint>>
{
    switch (passmark.type)
    {
        case PassmarkLineType::OneLine:
        case PassmarkLineType::InternalVMark:
        case PassmarkLineType::ExternalVMark:
        case PassmarkLineType::BoxMark:
        case PassmarkLineType::CheckMark:
            return PrepareNoneBreakingPassmarkShape(passmark);
            break;
        case PassmarkLineType::TMark:
            return PrepareTPassmarkShape(passmark, drawMode);
            break;
        case PassmarkLineType::UMark:
            return PrepareUPassmarkShape(passmark);
            break;
        default:
            break;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void ConvertTwoLinesPassmark(const VLayoutPassmark &passmark, QList<VBoundarySequenceItemData> &notchSequence)
{
    if (!passmark.lines.isEmpty())
    {
        VLayoutPassmark line1 = passmark;

        line1.lines = {passmark.lines.constFirst()};
        line1.type = PassmarkLineType::OneLine;
        line1.baseLine = passmark.lines.constFirst();

        VBoundarySequenceItemData itemData;
        itemData.item = QVariant::fromValue(line1);
        itemData.type = VBoundarySequenceItem::Passmark;

        FillSequance(itemData, notchSequence);
    }

    if (passmark.lines.size() > 1)
    {
        VLayoutPassmark line2 = passmark;

        line2.lines = {passmark.lines.constLast()};
        line2.type = PassmarkLineType::OneLine;
        line2.baseLine = passmark.lines.constLast();

        VBoundarySequenceItemData itemData;
        itemData.item = QVariant::fromValue(line2);
        itemData.type = VBoundarySequenceItem::Passmark;

        FillSequance(itemData, notchSequence);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ConvertThreeLinesPassmark(const VLayoutPassmark &passmark, QList<VBoundarySequenceItemData> &notchSequence)
{
    if (!passmark.lines.isEmpty())
    {
        VLayoutPassmark line1 = passmark;

        line1.lines = {passmark.lines.constFirst()};
        line1.type = PassmarkLineType::OneLine;
        line1.baseLine = passmark.lines.constFirst();

        VBoundarySequenceItemData itemData;
        itemData.item = QVariant::fromValue(line1);
        itemData.type = VBoundarySequenceItem::Passmark;

        FillSequance(itemData, notchSequence);
    }

    if (passmark.lines.size() > 1)
    {
        VLayoutPassmark line2 = passmark;

        line2.lines = {passmark.lines.at(1)};
        line2.type = PassmarkLineType::OneLine;
        line2.baseLine = passmark.lines.at(1);

        VBoundarySequenceItemData itemData;
        itemData.item = QVariant::fromValue(line2);
        itemData.type = VBoundarySequenceItem::Passmark;

        FillSequance(itemData, notchSequence);
    }

    if (passmark.lines.size() > 2)
    {
        VLayoutPassmark line3 = passmark;

        line3.lines = {passmark.lines.constLast()};
        line3.type = PassmarkLineType::OneLine;
        line3.baseLine = passmark.lines.constLast();

        VBoundarySequenceItemData itemData;
        itemData.item = QVariant::fromValue(line3);
        itemData.type = VBoundarySequenceItem::Passmark;

        FillSequance(itemData, notchSequence);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SubdividePath(const QVector<VLayoutPoint> &boundary, const QPointF &p, QVector<VLayoutPoint> &sub1,
                   QVector<VLayoutPoint> &sub2) -> bool
{
    if (boundary.size() < 2)
    {
        return false;
    }

    bool found = false;
    sub1.clear();
    sub2.clear();

    auto BreakPoint = [p]()
    {
        VLayoutPoint breakPoint(p);
        breakPoint.SetTurnPoint(true);
        return breakPoint;
    };

    for (qint32 i = 0; i < boundary.count() - 1; ++i)
    {
        if (found)
        {
            sub2.append(boundary.at(i));

            if (i + 1 == boundary.count() - 1)
            {
                sub2.append(boundary.at(i + 1));
            }
            continue;
        }

        if (!VGObject::IsPointOnLineSegment(p, static_cast<QPointF>(boundary.at(i)),
                                            static_cast<QPointF>(boundary.at(i + 1))))
        {
            sub1.append(boundary.at(i));
            continue;
        }

        if (not VFuzzyComparePoints(boundary.at(i), p))
        {
            sub1.append(boundary.at(i));
            sub1.append(BreakPoint());
        }
        else
        {
            if (not sub1.isEmpty())
            {
                sub1.append(BreakPoint());
            }
        }

        if (not VFuzzyComparePoints(boundary.at(i + 1), p))
        {
            sub2.append(BreakPoint());

            if (i + 1 == boundary.count() - 1)
            {
                sub2.append(boundary.at(i + 1));
            }
        }

        found = true;
    }

    if (not found)
    {
        sub1.clear();
    }

    return found;
}

//---------------------------------------------------------------------------------------------------------------------
auto InsertDisconnect(QList<VBoundarySequenceItemData> &sequence, int i, const VBoundarySequenceItemData &item,
                      bool drawMode) -> bool
{
    auto passmark = item.item.value<VLayoutPassmark>();
    bool inserted = false;

    const auto boundary = sequence.at(i).item.value<VLayoutPiecePath>().Points();

    QVector<VLayoutPoint> sub1;
    QVector<VLayoutPoint> sub2;
    if (!SubdividePath(boundary, passmark.baseLine.p1(), sub1, sub2))
    {
        return false;
    }

    sequence.removeAt(i);

    if (not sub2.isEmpty())
    {
        sequence.insert(i, PrepareSequenceItem(sub2, drawMode, VBoundarySequenceItem::Boundary));
    }

    QVector<QVector<VLayoutPoint>> shape = PreparePassmarkShape(passmark, drawMode);
    for (auto &subShape : shape)
    {
        sequence.insert(i, PrepareSequenceItem(subShape, drawMode, VBoundarySequenceItem::PassmarkShape));
    }

    if (not sub1.isEmpty())
    {
        sequence.insert(i, PrepareSequenceItem(sub1, drawMode, VBoundarySequenceItem::Boundary));
    }

    inserted = true;

    return inserted;
}

//---------------------------------------------------------------------------------------------------------------------
auto InsertCutOut(QList<VBoundarySequenceItemData> &sequence, int i, const VBoundarySequenceItemData &item,
                  bool drawMode) -> bool
{
    auto passmark = item.item.value<VLayoutPassmark>();

    QVector<QVector<VLayoutPoint>> shape = PreparePassmarkShape(passmark, drawMode);
    if (shape.isEmpty())
    {
        return false;
    }

    const QVector<VLayoutPoint> &subShape = shape.constFirst();
    if (subShape.size() < 2)
    {
        return false;
    }

    const auto boundary = sequence.at(i).item.value<VLayoutPiecePath>().Points();

    QVector<VLayoutPoint> startSub1;
    QVector<VLayoutPoint> startSub2;
    if (!SubdividePath(boundary, subShape.constFirst(), startSub1, startSub2))
    {
        return false;
    }

    QVector<VLayoutPoint> endSub1;
    QVector<VLayoutPoint> endSub2;
    if (!SubdividePath(boundary, subShape.constLast(), endSub1, endSub2))
    {
        return false;
    }

    sequence.removeAt(i);

    if (not endSub2.isEmpty())
    {
        sequence.insert(i, PrepareSequenceItem(endSub2, drawMode, VBoundarySequenceItem::Boundary));
    }

    sequence.insert(i, PrepareSequenceItem(subShape, drawMode, VBoundarySequenceItem::PassmarkShape));

    if (not startSub1.isEmpty())
    {
        sequence.insert(i, PrepareSequenceItem(startSub1, drawMode, VBoundarySequenceItem::Boundary));
    }

    return true;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VBoundary::VBoundary(const QVector<VLayoutPoint> &boundary, bool seamAllowance, bool builtInSeamAllowance)
  : m_boundary(boundary),
    m_seamAllowance(seamAllowance),
    m_builtInSeamAllowance(builtInSeamAllowance)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::Combine(const QVector<VLayoutPassmark> &passmarks, bool drawMode, bool layoutAllowance) const
    -> QList<VBoundarySequenceItemData>
{
    QList<VBoundarySequenceItemData> notchSequence;

    for (const auto &passmark : passmarks)
    {
        if (SkipPassmark(passmark, drawMode, layoutAllowance))
        {
            continue;
        }

        if (passmark.type == PassmarkLineType::TwoLines)
        {
            ConvertTwoLinesPassmark(passmark, notchSequence);
            continue;
        }

        if (passmark.type == PassmarkLineType::ThreeLines)
        {
            ConvertThreeLinesPassmark(passmark, notchSequence);
            continue;
        }

        VBoundarySequenceItemData itemData;
        itemData.item = QVariant::fromValue(passmark);
        itemData.type = VBoundarySequenceItem::Passmark;

        FillSequance(itemData, notchSequence);
    }

    QList<VBoundarySequenceItemData> sequence;
    sequence.append(PrepareSequenceItem(m_boundary, drawMode, VBoundarySequenceItem::Boundary));

    for (auto &item : notchSequence)
    {
        if (item.type == VBoundarySequenceItem::Passmark)
        {
            InsertPassmark(item, sequence, drawMode);
        }
    }

    return sequence;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::SkipPassmark(const VLayoutPassmark &passmark, bool drawMode, bool layoutAllowance) const -> bool
{
    if (m_seamAllowance)
    {
        if ((m_builtInSeamAllowance && !passmark.isBuiltIn) || (!m_builtInSeamAllowance && passmark.isBuiltIn))
        {
            return true;
        }
    }
    else
    {
        if (!passmark.isBuiltIn)
        {
            return true;
        }
    }

    if (layoutAllowance && (passmark.type == PassmarkLineType::ExternalVMark ||
                            passmark.type == PassmarkLineType::OneLine || passmark.type == PassmarkLineType::TwoLines ||
                            passmark.type == PassmarkLineType::ThreeLines || passmark.type == PassmarkLineType::TMark))
    {
        return true;
    }

    if (!drawMode && passmark.type == PassmarkLineType::ExternalVMark)
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VBoundary::InsertPassmark(const VBoundarySequenceItemData &item, QList<VBoundarySequenceItemData> &sequence,
                               bool drawMode) const
{
    auto passmark = item.item.value<VLayoutPassmark>();

    bool inserted = false;
    for (int i = 0; i < sequence.size(); ++i)
    {
        const VBoundarySequenceItemData &itemData = sequence.at(i);
        if (itemData.type != VBoundarySequenceItem::Boundary)
        {
            continue;
        }

        switch (passmark.type)
        {
            case PassmarkLineType::OneLine:
            case PassmarkLineType::TMark:
            case PassmarkLineType::ExternalVMark:
                inserted = InsertDisconnect(sequence, i, item, drawMode);
                break;
            case PassmarkLineType::UMark:
            case PassmarkLineType::InternalVMark:
            case PassmarkLineType::BoxMark:
            case PassmarkLineType::CheckMark:
                inserted = InsertCutOut(sequence, i, item, drawMode);
                break;
            default:
                break;
        }

        if (inserted)
        {
            break;
        }
    }

    if (not inserted)
    {
        QString pieceName;
        if (!m_pieceName.isEmpty())
        {
            pieceName = tr("Piece '%1'.").arg(m_pieceName) + ' '_L1;
        }

        QString errorMsg;
        if (!passmark.label.isEmpty())
        {
            errorMsg = pieceName + tr("Unable to insert notch for point '%1'.").arg(passmark.label);
        }
        else
        {
            errorMsg = pieceName + tr("Unable to insert notch.");
        }

        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }
}
