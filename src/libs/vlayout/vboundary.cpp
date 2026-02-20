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
#include "../vgeometry/vgobject.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "vabstractpiece.h"
#include "vlayoutpiecepath.h"

#include <QPoint>
#include <QtDebug>
#include <algorithm>

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

    return {.type = type, .item = QVariant::fromValue(countur)};
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

        FillSequance({.type = VBoundarySequenceItem::Passmark, .item = QVariant::fromValue(line1)}, notchSequence);
    }

    if (passmark.lines.size() > 1)
    {
        VLayoutPassmark line2 = passmark;

        line2.lines = {passmark.lines.constLast()};
        line2.type = PassmarkLineType::OneLine;
        line2.baseLine = passmark.lines.constLast();

        FillSequance({.type = VBoundarySequenceItem::Passmark, .item = QVariant::fromValue(line2)}, notchSequence);
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

        FillSequance({.type = VBoundarySequenceItem::Passmark, .item = QVariant::fromValue(line1)}, notchSequence);
    }

    if (passmark.lines.size() > 1)
    {
        VLayoutPassmark line2 = passmark;

        line2.lines = {passmark.lines.at(1)};
        line2.type = PassmarkLineType::OneLine;
        line2.baseLine = passmark.lines.at(1);

        FillSequance({.type = VBoundarySequenceItem::Passmark, .item = QVariant::fromValue(line2)}, notchSequence);
    }

    if (passmark.lines.size() > 2)
    {
        VLayoutPassmark line3 = passmark;

        line3.lines = {passmark.lines.constLast()};
        line3.type = PassmarkLineType::OneLine;
        line3.baseLine = passmark.lines.constLast();

        FillSequance({.type = VBoundarySequenceItem::Passmark, .item = QVariant::fromValue(line3)}, notchSequence);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto BoundaryLength(const QVector<VLayoutPoint> &path) -> qreal
{
    if (path.size() < 2)
    {
        return 0;
    }

    QPainterPath splinePath;
    splinePath.moveTo(path.at(0));
    for (qint32 i = 1; i < path.count(); ++i)
    {
        splinePath.lineTo(path.at(i));
    }
    return splinePath.length();
}

//---------------------------------------------------------------------------------------------------------------------
// Helper to insert boundary items into the sequence
void InsertBoundaryItems(QList<VBoundarySequenceItemData> &sequence,
                         int index,
                         const QVector<VLayoutPoint> &startSub,
                         const QVector<VLayoutPoint> &subShape,
                         const QVector<VLayoutPoint> &endSub,
                         bool drawMode)
{
    if (!endSub.isEmpty())
    {
        sequence.insert(index, PrepareSequenceItem(endSub, drawMode, VBoundarySequenceItem::Boundary));
    }

    sequence.insert(index, PrepareSequenceItem(subShape, drawMode, VBoundarySequenceItem::PassmarkShape));

    if (!startSub.isEmpty())
    {
        sequence.insert(index, PrepareSequenceItem(startSub, drawMode, VBoundarySequenceItem::Boundary));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Helper to optimize the sequence by merging connected boundaries
void OptimizeBoundarySequence(QList<VBoundarySequenceItemData> &sequence)
{
    if (sequence.size() > 1 && sequence.constFirst().type == VBoundarySequenceItem::Boundary
        && sequence.constLast().type == VBoundarySequenceItem::Boundary)
    {
        auto firstBoundary = sequence.constFirst().item.value<VLayoutPiecePath>().Points();
        auto lastBoundaryPath = sequence.constLast().item.value<VLayoutPiecePath>();
        auto lastBoundary = lastBoundaryPath.Points();

        if (!firstBoundary.isEmpty() && !lastBoundary.isEmpty()
            && lastBoundary.constLast() == firstBoundary.constFirst())
        {
            // Merge boundaries
            firstBoundary.removeFirst();
            lastBoundary += firstBoundary;

            sequence.removeFirst(); // Remove the first boundary

            lastBoundaryPath.SetPoints(lastBoundary);
            sequence.last().item = QVariant::fromValue(lastBoundaryPath);
        }
    }
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
        if (SkipPassmark(passmark, layoutAllowance))
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

        FillSequance({.type = VBoundarySequenceItem::Passmark, .item = QVariant::fromValue(passmark)}, notchSequence);
    }

    QList<VBoundarySequenceItemData> sequence;
    sequence.append(PrepareSequenceItem(m_boundary, drawMode, VBoundarySequenceItem::Boundary));

    for (auto &item : notchSequence)
    {
        if (item.type == VBoundarySequenceItem::Passmark)
        {
            InsertPassmark(item, sequence, drawMode);

            if (!m_mirrorLine.isNull())
            {
                if (auto passmark = item.item.value<VLayoutPassmark>();
                    !IsPointOnLineviaPDP(passmark.baseLine.p1(), m_mirrorLine.p1(), m_mirrorLine.p2()))
                {
                    item.mirror = true;
                    InsertPassmark(item, sequence, drawMode);
                }
            }
        }
    }

    return sequence;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::SkipPassmark(const VLayoutPassmark &passmark, bool layoutAllowance) const -> bool
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
        if (const VBoundarySequenceItemData &itemData = sequence.at(i); itemData.type != VBoundarySequenceItem::Boundary)
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
            pieceName = QCoreApplication::translate("VBoundary", "Piece '%1'.").arg(m_pieceName) + ' '_L1;
        }

        QString errorMsg;
        if (!passmark.label.isEmpty())
        {
            errorMsg =
                pieceName +
                QCoreApplication::translate("VBoundary", "Unable to insert notch for point '%1'.").arg(passmark.label);
        }
        else
        {
            errorMsg = pieceName + QCoreApplication::translate("VBoundary", "Unable to insert notch.");
        }

        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::InsertDisconnect(QList<VBoundarySequenceItemData> &sequence, int i,
                                 const VBoundarySequenceItemData &item, bool drawMode) const -> bool
{
    auto passmark = item.item.value<VLayoutPassmark>();
    bool inserted = false;

    const auto boundary = sequence.at(i).item.value<VLayoutPiecePath>().Points();

    QPointF connectionPoint = passmark.baseLine.p1();
    if (item.mirror && !m_mirrorLine.isNull())
    {
        const QTransform matrix = VGObject::FlippingMatrix(m_mirrorLine);
        connectionPoint = matrix.map(connectionPoint);
    }

    QVector<VLayoutPoint> sub1;
    QVector<VLayoutPoint> sub2;
    if (!VAbstractPiece::SubdividePath(boundary, connectionPoint, sub1, sub2))
    {
        return false;
    }

    sequence.removeAt(i);

    if (not sub2.isEmpty())
    {
        sequence.insert(i, PrepareSequenceItem(sub2, drawMode, VBoundarySequenceItem::Boundary));
    }

    QVector<QVector<VLayoutPoint>> const shape = PreparePassmarkShape(passmark, drawMode, item.mirror);
    for (const auto &subShape : shape)
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
auto VBoundary::InsertCutOut(QList<VBoundarySequenceItemData> &sequence, int i, const VBoundarySequenceItemData &item,
                             bool drawMode) const -> bool
{
    // Extract passmark and prepare shape
    auto passmark = item.item.value<VLayoutPassmark>();
    QVector<QVector<VLayoutPoint>> const shape = PreparePassmarkShape(passmark, drawMode, item.mirror);
    if (shape.isEmpty() || shape.constFirst().size() < 2)
    {
        return false; // Invalid shape or insufficient points
    }

    const QVector<VLayoutPoint> &subShape = shape.constFirst();
    const auto boundary = sequence.at(i).item.value<VLayoutPiecePath>().Points();

    // Subdivide the boundary at the first and last points of the subShape
    QVector<VLayoutPoint> startSub1;
    QVector<VLayoutPoint> startSub2;
    QVector<VLayoutPoint> endSub1;
    QVector<VLayoutPoint> endSub2;
    if (!VAbstractPiece::SubdividePath(boundary, subShape.constFirst(), startSub1, startSub2)
        || !VAbstractPiece::SubdividePath(boundary, subShape.constLast(), endSub1, endSub2))
    {
        return false; // Subdivision failed
    }

    // Compare subdivisions and modify the sequence accordingly
    sequence.removeAt(i);
    if (BoundaryLength(startSub1) <= BoundaryLength(endSub1))
    {
        InsertBoundaryItems(sequence, i, startSub1, subShape, endSub2, drawMode);
    }
    else
    {
        // Recalculate subdivisions for consistency
        if (!VAbstractPiece::SubdividePath(startSub1, subShape.constLast(), endSub1, endSub2))
        {
            return false;
        }
        InsertBoundaryItems(sequence, i, {}, subShape, endSub2, drawMode);
    }

    // Optimize boundaries to merge connected segments
    OptimizeBoundarySequence(sequence);

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::PreparePassmarkShape(const VLayoutPassmark &passmark, bool drawMode, bool mirrorNotch) const
    -> QVector<QVector<VLayoutPoint>>
{
    switch (passmark.type)
    {
        case PassmarkLineType::OneLine:
        case PassmarkLineType::InternalVMark:
        case PassmarkLineType::BoxMark:
        case PassmarkLineType::CheckMark:
            return PrepareNoneBreakingPassmarkShape(passmark, mirrorNotch);
        case PassmarkLineType::ExternalVMark:
            return PrepareExternalVPassmarkShape(passmark, drawMode, mirrorNotch);
        case PassmarkLineType::TMark:
            return PrepareTPassmarkShape(passmark, drawMode, mirrorNotch);
        case PassmarkLineType::UMark:
            return PrepareUPassmarkShape(passmark, mirrorNotch);
        default:
            break;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::PrepareNoneBreakingPassmarkShape(const VLayoutPassmark &passmark, bool mirrorNotch) const
    -> QVector<QVector<VLayoutPoint>>
{
    auto TurnPoint = [](QPointF point)
    {
        VLayoutPoint p(point);
        p.SetTurnPoint(true);
        return p;
    };

    QVector<QLineF> lines = passmark.lines;
    if (mirrorNotch && !m_mirrorLine.isNull())
    {
        const QTransform matrix = VGObject::FlippingMatrix(m_mirrorLine);
        lines = VAbstractPiece::MapVector(lines, matrix);
    }

    QVector<VLayoutPoint> shape;
    shape.reserve(lines.size() + 1);
    for (int i = 0; i < lines.size(); ++i)
    {
        const QLineF &line = lines.at(i);
        shape.append(TurnPoint(line.p1()));

        if (lines.size() - 1 == i)
        {
            shape.append(TurnPoint(line.p2()));
        }
    }

    if (mirrorNotch && !m_mirrorLine.isNull() &&
        (passmark.type == PassmarkLineType::InternalVMark || passmark.type == PassmarkLineType::BoxMark ||
         passmark.type == PassmarkLineType::CheckMark))
    {
        std::reverse(shape.begin(), shape.end());
    }

    return {shape};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::PrepareExternalVPassmarkShape(const VLayoutPassmark &passmark, bool drawMode, bool mirrorNotch) const
    -> QVector<QVector<VLayoutPoint>>
{
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

    QVector<QLineF> lines = passmark.lines;
    if (mirrorNotch && !m_mirrorLine.isNull())
    {
        const QTransform matrix = VGObject::FlippingMatrix(m_mirrorLine);
        lines = VAbstractPiece::MapVector(lines, matrix);
    }

    QLineF const line1 = lines.constFirst();
    QVector<VLayoutPoint> shape;

    if (!drawMode)
    {
        shape.append(TurnPoint(line1.p2()));
    }

    shape.append(TurnPoint(line1.p1()));
    shape.append(TurnPoint(line1.p2()));

    if (lines.size() <= 1)
    {
        return {shape};
    }

    const QLineF &line2 = lines.constLast();

    shape.append(TurnPoint(line2.p1()));
    shape.append(TurnPoint(line2.p2()));

    if (!drawMode)
    {
        shape.append(TurnPoint(line2.p1()));

        if (mirrorNotch && !m_mirrorLine.isNull())
        {
            std::reverse(shape.begin(), shape.end());
        }
    }

    return {shape};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::PrepareTPassmarkShape(const VLayoutPassmark &passmark, bool drawMode, bool mirrorNotch) const
    -> QVector<QVector<VLayoutPoint>>
{
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

    QVector<QLineF> lines = passmark.lines;
    if (mirrorNotch && !m_mirrorLine.isNull())
    {
        const QTransform matrix = VGObject::FlippingMatrix(m_mirrorLine);
        lines = VAbstractPiece::MapVector(lines, matrix);
    }

    QLineF const line1 = lines.constFirst();
    QVector<VLayoutPoint> shape1;
    shape1.append(TurnPoint(line1.p1()));
    shape1.append(TurnPoint(line1.p2()));

    if (lines.size() <= 1)
    {
        return {shape1};
    }

    const QLineF &line2 = lines.constLast();

    if (!drawMode)
    {
        shape1.append(TurnPoint(line2.p1()));
        shape1.append(TurnPoint(line2.p2()));
        shape1.append(TurnPoint(line1.p2()));

        if (mirrorNotch && !m_mirrorLine.isNull())
        {
            std::reverse(shape1.begin(), shape1.end());
        }

        return {shape1};
    }

    QVector<VLayoutPoint> shape2;
    shape2.append(TurnPoint(line2.p1()));
    shape2.append(TurnPoint(line2.p2()));
    return {shape1, shape2};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBoundary::PrepareUPassmarkShape(const VLayoutPassmark &passmark, bool mirrorNotch) const
    -> QVector<QVector<VLayoutPoint>>
{
    auto LayoutPoint = [](QPointF point, bool turnPoint, bool curvePoint)
    {
        VLayoutPoint p(point);
        p.SetTurnPoint(turnPoint);
        p.SetCurvePoint(curvePoint);
        return p;
    };

    if (passmark.lines.isEmpty())
    {
        return {};
    }

    qreal const radius = QLineF(passmark.baseLine.p1(), passmark.lines.constFirst().p1()).length();

    QVector<QLineF> lines = passmark.lines;
    if (mirrorNotch && !m_mirrorLine.isNull())
    {
        const QTransform matrix = VGObject::FlippingMatrix(m_mirrorLine);
        lines = VAbstractPiece::MapVector(lines, matrix);
    }

    if (passmark.baseLine.length() - radius > accuracyPointOnLine)
    {
        if (lines.size() < 3)
        {
            return {};
        }

        QLineF const line1 = lines.takeFirst();

        QVector<VLayoutPoint> shape;
        shape.reserve(4 + passmark.lines.size() + 1);

        shape.append(LayoutPoint(line1.p1(), true, false));
        shape.append(LayoutPoint(line1.p2(), true, true));

        QLineF const line2 = lines.takeLast();

        for (int i = 0; i < lines.size(); ++i)
        {
            const QLineF &line = lines.at(i);
            shape.append(LayoutPoint(line.p1(), false, true));

            if (lines.size() - 1 == i)
            {
                shape.append(LayoutPoint(line.p2(), false, true));
            }
        }

        shape.append(LayoutPoint(line2.p1(), true, true));
        shape.append(LayoutPoint(line2.p2(), true, false));

        if (mirrorNotch && !m_mirrorLine.isNull())
        {
            std::reverse(shape.begin(), shape.end());
        }

        return {shape};
    }

    QVector<VLayoutPoint> shape;
    shape.reserve(lines.size() + 1);
    for (int i = 0; i < lines.size(); ++i)
    {
        const QLineF &line = lines.at(i);
        shape.append(LayoutPoint(line.p1(), false, true));

        if (lines.size() - 1 == i)
        {
            shape.append(LayoutPoint(line.p2(), false, true));
        }
    }

    if (!shape.isEmpty())
    {
        shape.first().SetTurnPoint(true);
        shape.last().SetTurnPoint(true);
    }

    if (mirrorNotch && !m_mirrorLine.isNull())
    {
        std::reverse(shape.begin(), shape.end());
    }

    return {shape};
}
