/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
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

#include "vpiecenode.h"
#include "../vmisc/vabstractvalapplication.h"
#include "vcontainer.h"
#include "vformula.h"
#include "vpiecenode_p.h"

#include <QDataStream>
#include <QtNumeric>

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::VPieceNode()
  : d(new VPieceNodeData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::VPieceNode(quint32 id, Tool typeTool, bool reverse)
  : d(new VPieceNodeData(id, typeTool, reverse))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VPieceNode)

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::operator=(const VPieceNode &node) -> VPieceNode &
{
    if (&node == this)
    {
        return *this;
    }
    d = node.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPieceNode::VPieceNode(VPieceNode &&node) noexcept
  : d(std::move(node.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::operator=(VPieceNode &&node) noexcept -> VPieceNode &
{
    std::swap(d, node.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::~VPieceNode() = default;

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &out, const VPieceNode &p) -> QDataStream &
{
    out << *p.d;
    return out;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &in, VPieceNode &p) -> QDataStream &
{
    in >> *p.d;
    return in;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetId() const -> quint32
{
    return d->m_id;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetId(quint32 id)
{
    d->m_id = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetTypeTool() const -> Tool
{
    return d->m_typeTool;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetTypeTool(Tool value)
{
    d->m_typeTool = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetReverse() const -> bool
{
    return d->m_reverse;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetReverse(bool reverse)
{
    if (d->m_typeTool != Tool::NodePoint)
    {
        d->m_reverse = reverse;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetSABefore(const VContainer *data) const -> qreal
{
    if (d->m_formulaWidthBefore == currentSeamAllowance)
    {
        return -1;
    }

    VFormula formula(d->m_formulaWidthBefore, data);
    formula.setCheckZero(false);
    formula.Eval();

    if (formula.error())
    {
        QString nodeName;
        try
        {
            nodeName = data->GetGObject(d->m_id)->name();
        }
        catch (const VExceptionBadId &)
        {
        }

        const QString errorMsg = QObject::tr("Cannot calculate seam allowance before for point '%1'. Reason: %2.")
                                     .arg(nodeName, formula.Reason());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return -1;
    }
    return formula.getDoubleValue();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetSABefore(const VContainer *data, Unit unit) const -> qreal
{
    if (d->m_formulaWidthBefore == currentSeamAllowance)
    {
        return -1;
    }

    VFormula formula(d->m_formulaWidthBefore, data);
    formula.setCheckZero(false);
    formula.Eval();

    if (formula.error())
    {
        QString nodeName;
        try
        {
            nodeName = data->GetGObject(d->m_id)->name();
        }
        catch (const VExceptionBadId &)
        {
        }

        const QString errorMsg = QObject::tr("Cannot calculate seam allowance before for point '%1'. Reason: %2.")
                                     .arg(nodeName, formula.Reason());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return -1;
    }

    qreal value = formula.getDoubleValue();
    if (value >= 0)
    {
        value = ToPixel(value, unit);
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetFormulaSABefore() const -> QString
{
    return d->m_formulaWidthBefore;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaSABefore(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_formulaWidthBefore = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetSAAfter(const VContainer *data) const -> qreal
{
    if (d->m_formulaWidthAfter == currentSeamAllowance)
    {
        return -1;
    }

    VFormula formula(d->m_formulaWidthAfter, data);
    formula.setCheckZero(false);
    formula.Eval();

    if (formula.error())
    {
        QString nodeName;
        try
        {
            nodeName = data->GetGObject(d->m_id)->name();
        }
        catch (const VExceptionBadId &)
        {
        }

        const QString errorMsg = QObject::tr("Cannot calculate seam allowance after for point '%1'. Reason: %2.")
                                     .arg(nodeName, formula.Reason());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return -1;
    }

    return formula.getDoubleValue();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetSAAfter(const VContainer *data, Unit unit) const -> qreal
{
    if (d->m_formulaWidthAfter == currentSeamAllowance)
    {
        return -1;
    }

    VFormula formula(d->m_formulaWidthAfter, data);
    formula.setCheckZero(false);
    formula.Eval();

    if (formula.error())
    {
        QString nodeName;
        try
        {
            nodeName = data->GetGObject(d->m_id)->name();
        }
        catch (const VExceptionBadId &)
        {
        }

        const QString errorMsg = QObject::tr("Cannot calculate seam allowance after for point '%1'. Reason: %2.")
                                     .arg(nodeName, formula.Reason());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return -1;
    }

    qreal value = formula.getDoubleValue();

    if (value >= 0)
    {
        value = ToPixel(value, unit);
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetFormulaSAAfter() const -> QString
{
    return d->m_formulaWidthAfter;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaSAAfter(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_formulaWidthAfter = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetFormulaPassmarkLength() const -> QString
{
    return d->m_formulaPassmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaPassmarkLength(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_formulaPassmarkLength = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetFormulaPassmarkWidth() const -> QString
{
    return d->m_formulaPassmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaPassmarkWidth(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_formulaPassmarkWidth = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetFormulaPassmarkAngle() const -> QString
{
    return d->m_formulaPassmarkAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaPassmarkAngle(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_formulaPassmarkAngle = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetPassmarkLength(const VContainer *data, Unit unit) const -> qreal
{
    if (d->m_manualPassmarkLength)
    {
        VFormula formula(d->m_formulaPassmarkLength, data);
        formula.setCheckZero(true);
        formula.setCheckLessThanZero(true);
        formula.Eval();

        if (formula.error())
        {
            QString nodeName;
            try
            {
                nodeName = data->GetGObject(d->m_id)->name();
            }
            catch (const VExceptionBadId &)
            {
            }

            const QString errorMsg = QObject::tr("Cannot calculate passmark length for point '%1'. Reason: %2.")
                                         .arg(nodeName, formula.Reason());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            return VSAPoint::maxPassmarkLength;
        }

        return ToPixel(formula.getDoubleValue(), unit);
    }
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetPassmarkWidth(const VContainer *data, Unit unit) const -> qreal
{
    if (d->m_manualPassmarkWidth)
    {
        VFormula formula(d->m_formulaPassmarkWidth, data);
        formula.setCheckZero(true);
        formula.setCheckLessThanZero(false);
        formula.Eval();

        if (formula.error())
        {
            QString nodeName;
            try
            {
                nodeName = data->GetGObject(d->m_id)->name();
            }
            catch (const VExceptionBadId &)
            {
            }

            const QString errorMsg = QObject::tr("Cannot calculate passmark width for point '%1'. Reason: %2.")
                                         .arg(nodeName, formula.Reason());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            return 0;
        }

        return ToPixel(formula.getDoubleValue(), unit);
    }
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetPassmarkAngle(const VContainer *data) const -> qreal
{
    if (d->m_manualPassmarkAngle)
    {
        VFormula formula(d->m_formulaPassmarkAngle, data);
        formula.setCheckZero(false);
        formula.setCheckLessThanZero(false);
        formula.Eval();

        if (formula.error())
        {
            QString nodeName;
            try
            {
                nodeName = data->GetGObject(d->m_id)->name();
            }
            catch (const VExceptionBadId &)
            {
            }

            const QString errorMsg = QObject::tr("Cannot calculate passmark angle for point '%1'. Reason: %2.")
                                         .arg(nodeName, formula.Reason());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            return 0;
        }

        return formula.getDoubleValue();
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetAngleType() const -> PieceNodeAngle
{
    return d->m_angleType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetAngleType(PieceNodeAngle type)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_angleType = type;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsPassmark() const -> bool
{
    return d->m_isPassmark;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetPassmark(bool passmark)
{
    if (GetTypeTool() == Tool::NodePoint)
    {
        d->m_isPassmark = passmark;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsMainPathNode() const -> bool
{
    return d->m_isMainPathNode;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetMainPathNode(bool value)
{
    d->m_isMainPathNode = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetPassmarkLineType() const -> PassmarkLineType
{
    return d->m_passmarkLineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetPassmarkLineType(PassmarkLineType lineType)
{
    d->m_passmarkLineType = lineType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::GetPassmarkAngleType() const -> PassmarkAngleType
{
    return d->m_passmarkAngleType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetPassmarkAngleType(PassmarkAngleType angleType)
{
    d->m_passmarkAngleType = angleType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsShowSecondPassmark() const -> bool
{
    return d->m_isShowSecondPassmark;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetShowSecondPassmark(bool value)
{
    d->m_isShowSecondPassmark = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsPassmarkClockwiseOpening() const -> bool
{
    return d->m_isPassmarkClockwiseOpening;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetPassmarkClockwiseOpening(bool value)
{
    d->m_isPassmarkClockwiseOpening = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsCheckUniqueness() const -> bool
{
    return d->m_checkUniqueness;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetCheckUniqueness(bool value)
{
    d->m_checkUniqueness = (d->m_typeTool == Tool::NodePoint ? value : true);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsManualPassmarkLength() const -> bool
{
    return d->m_manualPassmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetManualPassmarkLength(bool value)
{
    d->m_manualPassmarkLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsManualPassmarkWidth() const -> bool
{
    return d->m_manualPassmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetManualPassmarkWidth(bool value)
{
    d->m_manualPassmarkWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsManualPassmarkAngle() const -> bool
{
    return d->m_manualPassmarkAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetManualPassmarkAngle(bool value)
{
    d->m_manualPassmarkAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsTurnPoint() const -> bool
{
    return d->m_typeTool == Tool::NodePoint ? d->m_turnPoint : false;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetTurnPoint(bool value)
{
    d->m_turnPoint = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceNode::IsExcluded() const -> bool
{
    return d->m_excluded;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetExcluded(bool exclude)
{
    d->m_excluded = exclude;
}
