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
VAbstractArc::VAbstractArc(const VAbstractArc &arc) = default;

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

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VAbstractArc::VAbstractArc(VAbstractArc &&arc) noexcept
  : VAbstractCurve(std::move(arc)),
    d(std::move(arc.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::operator=(VAbstractArc &&arc) noexcept -> VAbstractArc &
{
    VAbstractCurve::operator=(arc);
    std::swap(d, arc.d);
    return *this;
}
#endif

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
    QString name = toolName + QStringLiteral(" %1").arg(GetCenter().name());

    if (VAbstractCurve::id() != NULL_ID)
    {
        name += QStringLiteral("_%1").arg(VAbstractCurve::id());
    }

    if (GetDuplicate() > 0)
    {
        name += QStringLiteral("_%1").arg(GetDuplicate());
    }

    QString alias;

    if (not GetAliasSuffix().isEmpty())
    {
        alias = QStringLiteral("%1 %2").arg(toolName, GetAliasSuffix());
    }

    return not alias.isEmpty() ? QStringLiteral("%1 (%2)").arg(alias, name) : name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::IsFlipped() const -> bool
{
    return d->isFlipped;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractArc::AngleArc() const -> qreal
{
    {
        const qreal angleDiff = qAbs(GetStartAngle() - GetEndAngle());
        if (VFuzzyComparePossibleNulls(angleDiff, 0) || VFuzzyComparePossibleNulls(angleDiff, 360))
        {
            return 360;
        }
    }
    QLineF l1(0, 0, 100, 0);
    l1.setAngle(GetStartAngle());
    QLineF l2(0, 0, 100, 0);
    l2.setAngle(GetEndAngle());

    qreal ang = l1.angleTo(l2);

    if (IsFlipped())
    {
        ang = 360 - ang;
    }

    return ang;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFlipped(bool value)
{
    d->isFlipped = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractArc::SetFormulaLength(const QString &formula)
{
    d->formulaLength = formula;
}
