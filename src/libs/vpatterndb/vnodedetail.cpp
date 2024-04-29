/************************************************************************
 **
 **  @file   vnodedetail.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vnodedetail.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "vnodedetail_p.h"
#include "vpiecenode.h"
#include "vpiecepath.h"

#include <QLineF>
#include <QVector>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto IsOX(const QLineF &line) -> bool
{
    return VFuzzyComparePossibleNulls(line.angle(), 0) || VFuzzyComparePossibleNulls(line.angle(), 360) ||
           VFuzzyComparePossibleNulls(line.angle(), 180);
}

//---------------------------------------------------------------------------------------------------------------------
auto IsOY(const QLineF &line) -> bool
{
    return VFuzzyComparePossibleNulls(line.angle(), 90) || VFuzzyComparePossibleNulls(line.angle(), 270);
}

//---------------------------------------------------------------------------------------------------------------------
auto LocalWidth(const QLineF &line, const QLineF &movedLine) -> QString
{
    if (VFuzzyComparePossibleNulls(line.angle(), movedLine.angle()))
    {
        return QString().setNum(movedLine.length());
    }
    // different direction means value is negative
    return QChar('0');
}

//---------------------------------------------------------------------------------------------------------------------
void ConvertBefore(VPieceNode &node, const QLineF &line, qreal mX, qreal mY)
{
    if (not qFuzzyIsNull(mX) && IsOX(line))
    {
        const QLineF movedLine(line.p1().x(), line.p1().y(), line.p2().x() + mX, line.p2().y());
        node.SetFormulaSABefore(LocalWidth(line, movedLine));
    }
    else if (not qFuzzyIsNull(mY) && IsOY(line))
    {
        const QLineF movedLine(line.p1().x(), line.p1().y(), line.p2().x(), line.p2().y() + mY);
        node.SetFormulaSABefore(LocalWidth(line, movedLine));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ConvertAfter(VPieceNode &node, const QLineF &line, qreal mX, qreal mY)
{
    if (not qFuzzyIsNull(mX) && IsOX(line))
    {
        const QLineF movedLine(line.p1().x(), line.p1().y(), line.p2().x() + mX, line.p2().y());
        node.SetFormulaSAAfter(LocalWidth(line, movedLine));
    }
    else if (not qFuzzyIsNull(mY) && IsOY(line))
    {
        const QLineF movedLine(line.p1().x(), line.p1().y(), line.p2().x(), line.p2().y() + mY);
        node.SetFormulaSAAfter(LocalWidth(line, movedLine));
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VNodeDetail::VNodeDetail()
  : d(new VNodeDetailData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VNodeDetail::VNodeDetail(quint32 id, Tool typeTool, NodeDetail typeNode, qreal mx, qreal my, bool reverse)
  : d(new VNodeDetailData(id, typeTool, typeNode, mx, my, reverse))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VNodeDetail)

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::operator=(const VNodeDetail &node) -> VNodeDetail &
{
    if (&node == this)
    {
        return *this;
    }
    d = node.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VNodeDetail::VNodeDetail(VNodeDetail &&node) noexcept
  : d(std::move(node.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::operator=(VNodeDetail &&node) noexcept -> VNodeDetail &
{
    std::swap(d, node.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VNodeDetail::~VNodeDetail() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::getId() const -> quint32
{
    return d->id;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodeDetail::setId(const quint32 &value)
{
    d->id = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::getTypeTool() const -> Tool
{
    return d->typeTool;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VNodeDetail::setTypeTool(const Tool &value)
{
    d->typeTool = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::getTypeNode() const -> NodeDetail
{
    return d->typeNode;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VNodeDetail::setTypeNode(const NodeDetail &value)
{
    d->typeNode = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::getMx() const -> qreal
{
    return d->mx;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodeDetail::setMx(const qreal &value)
{
    d->mx = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::getMy() const -> qreal
{
    return d->my;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodeDetail::setMy(const qreal &value)
{
    d->my = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::getReverse() const -> bool
{
    if (getTypeTool() == Tool::NodePoint)
    {
        return false;
    }
    return d->reverse;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodeDetail::setReverse(bool reverse)
{
    if (getTypeTool() == Tool::NodePoint)
    {
        d->reverse = false;
    }
    else
    {
        d->reverse = reverse;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodeDetail::Convert(const VContainer *data, const QVector<VNodeDetail> &nodes, qreal width, bool closed)
    -> QVector<VPieceNode>
{
    if (width < 0)
    {
        width = 0;
    }

    VPiecePath path;
    for (const auto &node : nodes)
    {
        path.Append(VPieceNode(node.getId(), node.getTypeTool(), node.getReverse()));
    }

    if (path.PathPoints(data).size() > 2)
    {
        for (int i = 0; i < nodes.size(); ++i)
        {
            const VNodeDetail &node = nodes.at(i);
            if (node.getTypeTool() == Tool::NodePoint &&
                (not qFuzzyIsNull(node.getMx()) || not qFuzzyIsNull(node.getMy())))
            {
                const QPointF previosPoint = path.NodePreviousPoint(data, i);
                const QPointF nextPoint = path.NodeNextPoint(data, i);

                const QPointF point = data->GeometricObject<VPointF>(node.getId())->toQPointF();

                QLineF lineBefore(point, previosPoint);
                lineBefore.setAngle(lineBefore.angle() - 90);
                lineBefore.setLength(width);

                ConvertBefore(path[i], lineBefore, node.getMx(), node.getMy());

                QLineF lineAfter(point, nextPoint);
                lineAfter.setAngle(lineAfter.angle() + 90);
                lineAfter.setLength(width);

                ConvertAfter(path[i], lineAfter, node.getMx(), node.getMy());
            }
        }
    }

    if (not closed && path.CountNodes() > 1)
    {
        path[0].SetFormulaSABefore(QChar('0'));
        path[path.CountNodes() - 1].SetFormulaSAAfter(QChar('0'));
    }

    return path.GetNodes();
}
