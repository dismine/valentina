/************************************************************************
 **
 **  @file   vabstractarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vabstractarc.h"

#include <QLineF>

#include "../vmisc/def.h"
#include "vabstractarc_p.h"
#include "vpointf.h"
#include "vsplinepath.h"

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(const GOType &type, const quint32 &idObject, const Draw &mode)
  : VAbstractCurve(type, idObject, mode),
    d(new VAbstractArcData())
{
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(const GOType &type, const VPointF &center, qreal f1, const QString &formulaF1, qreal f2,
                           const QString &formulaF2, quint32 idObject, Draw mode)
  : VAbstractCurve(type, idObject, mode),
    d(new VAbstractArcData(center, f1, formulaF1, f2, formulaF2))
{
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(const GOType &type, const VPointF &center, qreal f1, qreal f2, quint32 idObject, Draw mode)
  : VAbstractCurve(type, idObject, mode),
    d(new VAbstractArcData(center, f1, f2))
{
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(const GOType &type, const QString &formulaLength, const VPointF &center, qreal f1,
                           const QString &formulaF1, quint32 idObject, Draw mode)
  : VAbstractCurve(type, idObject, mode),
    d(new VAbstractArcData(formulaLength, center, f1, formulaF1))
{
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(const GOType &type, const VPointF &center, qreal f1, quint32 idObject, Draw mode)
  : VAbstractCurve(type, idObject, mode),
    d(new VAbstractArcData(center, f1))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VAbstractArc, VAbstractCurve)

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::operator=(const VAbstractArc &arc) -> VAbstractArc &
{
    if (&arc == this)
    {
        return *this;
    }
    VAbstractCurve::operator=(arc);
    d = arc.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(VAbstractArc &&arc) noexcept
  : VAbstractCurve(std::move(arc)),
    d(std::move(arc.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::operator=(VAbstractArc &&arc) noexcept -> VAbstractArc &
{
    VAbstractCurve::operator=(arc);
    std::swap(d, arc.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::~VAbstractArc() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetFormulaF1() const -> QString
{
    return d->formulaF1;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFormulaF1(const QString &formula, qreal value)
{
    d->formulaF1 = formula;
    d->f1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetStartAngle() const -> qreal
{
    return d->f1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetFormulaF2() const -> QString
{
    return d->formulaF2;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFormulaF2(const QString &formula, qreal value)
{
    d->formulaF2 = formula;
    d->f2 = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetEndAngle() const -> qreal
{
    return d->f2;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetCenter() const -> VPointF
{
    return d->center;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetCenter(const VPointF &point)
{
    d->center = point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetFormulaLength() const -> QString
{
    return d->formulaLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFormulaLength(const QString &formula, qreal value)
{
    d->formulaLength = formula;
    FindF2(value);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::setId(const quint32 &id)
{
    VAbstractCurve::setId(id);
    CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::NameForHistory(const QString &toolName) const -> QString
{
    QString const name = toolName + GetMainNameForHistory();

    QString alias;

    if (not GetAliasSuffix().isEmpty())
    {
        alias = QStringLiteral("%1 %2").arg(toolName, GetAliasSuffix());
    }

    return not alias.isEmpty() ? QStringLiteral("%1 (%2)").arg(alias, name) : name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetMainNameForHistory() const -> QString
{
    QString name = QStringLiteral(" %1").arg(GetCenter().name());

    if (VAbstractCurve::id() != NULL_ID)
    {
        name += QStringLiteral("_%1").arg(VAbstractCurve::id());
    }

    if (GetDuplicate() > 0)
    {
        name += QStringLiteral("_%1").arg(GetDuplicate());
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::IsFlipped() const -> bool
{
    return d->isFlipped;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::AngleArc() const -> qreal
{
    return AngleArc(GetStartAngle(), GetEndAngle());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::GetPath() const -> QPainterPath
{
    QPainterPath path;

    if (const QVector<QPointF> points = GetPoints(); points.count() >= 2)
    {
        path.addPolygon(QPolygonF(points));
    }
    else
    {
        QPointF const center = GetCenter().toQPointF();
        auto rec = QRectF(center.x(), center.y(), accuracyPointOnLine * 2, accuracyPointOnLine * 2);
        rec.translate(-rec.center().x(), -rec.center().y());
        path.addEllipse(rec);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::CutArc(qreal length, VAbstractArc *arc1, VAbstractArc *arc2, const QString &pointName) const
    -> QPointF
{
    return DoCutArc(length, arc1, arc2, pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::CutArc(qreal length, const QString &pointName) const -> QPointF
{
    return DoCutArcByLength(length, pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::Offset(qreal distance, const QString &name) const -> VSplinePath
{
    VSplinePath splPath = ToSplinePath().Offset(IsFlipped() ? -distance : distance, name);
    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetApproximationScale(GetApproximationScale());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::Outline(const QVector<qreal> &distances, const QString &name) const -> VSplinePath
{
    QVector<qreal> tmpDistances = distances;
    if (IsFlipped())
    {
        for (auto &dist : tmpDistances)
        {
            dist = -dist;
        }
    }

    VSplinePath splPath = ToSplinePath().Outline(tmpDistances, name);
    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetApproximationScale(GetApproximationScale());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFlipped(bool value)
{
    d->isFlipped = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetAllowEmpty(bool value)
{
    d->isAllowEmpty = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::IsAllowEmpty() const -> bool
{
    return d->isAllowEmpty;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFormulaLength(const QString &formula)
{
    d->formulaLength = formula;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::AngleArc(qreal startAngle, qreal endAngle) const -> qreal
{
    {
        const qreal angleDiff = qAbs(startAngle - endAngle);
        if (VFuzzyComparePossibleNulls(angleDiff, 0) || VFuzzyComparePossibleNulls(angleDiff, 360))
        {
            return !d->isAllowEmpty ? 360 : 0;
        }
    }
    QLineF l1(0, 0, 100, 0);
    l1.setAngle(startAngle);
    QLineF l2(0, 0, 100, 0);
    l2.setAngle(endAngle);

    qreal ang = l1.angleTo(l2);

    if (IsFlipped())
    {
        ang = 360 - ang;
    }

    return ang;
}
