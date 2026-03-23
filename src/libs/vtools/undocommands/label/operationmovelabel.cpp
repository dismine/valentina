/************************************************************************
 **
 **  @file   moverotationlabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 5, 2016
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

#include "operationmovelabel.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/compatibility.h"
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
auto ReadLabelPos(VAbstractPattern *doc, quint32 m_idTool, quint32 idPoint) -> QPointF
{
    const QDomElement el = VUndoCommand::GetDestinationObject(doc, m_idTool, idPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "OperationMoveLabel: point id=%u not found; using (0,0) as old position", m_idTool);
        return {};
    }

    return {VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, AttrMx, *defPos)),
            VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, AttrMy, *defPos))};
}

} // namespace

//---------------------------------------------------------------------------------------------------------------------
OperationMoveLabel::OperationMoveLabel(
    quint32 idTool, VAbstractPattern *doc, const QPointF &pos, quint32 idPoint, QUndoCommand *parent)
  : MoveAbstractLabel(doc, idPoint, ReadLabelPos(doc, idTool, idPoint), pos, parent),
    m_idTool(idTool)
{
    setText(tr("move point label"));

    qCDebug(vUndo, "Tool id %u", m_idTool);
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationMoveLabel::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *other = dynamic_cast<const OperationMoveLabel *>(command);
    if ((other == nullptr) || other->ElementId() != ElementId() || other->m_idTool != m_idTool)
    {
        return false;
    }

    qCDebug(vUndo, "Merging: new position (%f;%f)", other->GetNewPos().x(), other->GetNewPos().y());
    SetNewPos(other->GetNewPos());
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationMoveLabel::id() const -> int
{
    return static_cast<int>(UndoCommand::RotationMoveLabel);
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationMoveLabel::ReadCurrentPos() const -> QPointF
{
    return ReadLabelPos(Doc(), m_idTool, ElementId());
}

//---------------------------------------------------------------------------------------------------------------------
void OperationMoveLabel::WritePos(const QPointF &pos)
{
    QDomElement el = VUndoCommand::GetDestinationObject(Doc(), m_idTool, ElementId());
    if (!el.isElement())
    {
        qCWarning(vUndo, "OperationMoveLabel: point id=%u not found; using (0,0) as old position", ElementId());
        return;
    }

    Doc()->SetAttribute(el, AttrMx, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.x())));
    Doc()->SetAttribute(el, AttrMy, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.y())));

    if (auto *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
    {
        tool->ChangeLabelPosition(ElementId(), pos);
    }
}
