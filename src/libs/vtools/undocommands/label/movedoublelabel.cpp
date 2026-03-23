/************************************************************************
 **
 **  @file   movedoublelabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "movedoublelabel.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vtools/tools/drawTools/vdrawtool.h"
#include "../vundocommand.h"
#include "moveabstractlabel.h"

using namespace Qt::Literals::StringLiterals;

namespace
{

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// Better to use global variables because repeating QStringLiteral blows up code size
Q_GLOBAL_STATIC_WITH_ARGS(const QString, defPos, ("0.0"_L1)) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
struct LabelAttrs
{
    QString mx;
    QString my;
};

//---------------------------------------------------------------------------------------------------------------------
auto AttrsForLabel(MoveDoublePoint type) -> LabelAttrs
{
    switch (type)
    {
        case MoveDoublePoint::FirstPoint:
            return {AttrMx1, AttrMy1};
        case MoveDoublePoint::SecondPoint:
            return {AttrMx2, AttrMy2};
        default:
            break;
    }
    Q_UNREACHABLE();
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadLabelPos(VAbstractPattern *doc, quint32 id, MoveDoublePoint type) -> QPointF
{
    const QDomElement el = doc->FindElementById(id, VAbstractPattern::TagPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "MoveDoubleLabel: point id=%u not found; using (0,0) as old position", id);
        return {};
    }

    const auto [mxAttr, myAttr] = AttrsForLabel(type);
    return {VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, mxAttr, *defPos)),
            VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, myAttr, *defPos))};
}

} // namespace

//---------------------------------------------------------------------------------------------------------------------
MoveDoubleLabel::MoveDoubleLabel(VAbstractPattern *doc,
                                 const QPointF &pos,
                                 MoveDoublePoint type,
                                 quint32 toolId,
                                 quint32 pointId,
                                 QUndoCommand *parent)
  : MoveAbstractLabel(doc, pointId, ReadLabelPos(doc, toolId, type), pos, parent),
    m_type(type),
    m_idTool(toolId)
{
    if (m_type == MoveDoublePoint::FirstPoint)
    {
        setText(tr("move the first dart label"));
    }
    else
    {
        setText(tr("move the second dart label"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveDoubleLabel::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *other = dynamic_cast<const MoveDoubleLabel *>(command);

    if ((other == nullptr) || other->ElementId() != ElementId() || other->m_type != m_type
        || other->m_idTool != m_idTool)
    {
        return false;
    }

    qCDebug(vUndo, "Merging: new position (%f;%f)", other->GetNewPos().x(), other->GetNewPos().y());
    SetNewPos(other->GetNewPos());
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveDoubleLabel::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveDoubleLabel);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveDoubleLabel::ReadCurrentPos() const -> QPointF
{
    return ReadLabelPos(Doc(), m_idTool, m_type);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveDoubleLabel::WritePos(const QPointF &pos)
{
    QDomElement el = Doc()->FindElementById(m_idTool, VAbstractPattern::TagPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "MoveDoubleLabel: cannot find point id=%u to write position", m_idTool);
        return;
    }

    const auto [mxAttr, myAttr] = AttrsForLabel(m_type);
    Doc()->SetAttribute(el, mxAttr, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.x())));
    Doc()->SetAttribute(el, myAttr, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.y())));

    if (auto *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
    {
        tool->ChangeLabelPosition(ElementId(), pos);
    }
}
