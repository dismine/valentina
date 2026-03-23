/************************************************************************
 **
 **  @file   movelabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 12, 2014
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

#include "movelabel.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vtools/tools/vabstracttool.h"
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
auto ReadLabelPos(VAbstractPattern *doc, quint32 id) -> QPointF
{
    const QDomElement el = doc->FindElementById(id, VAbstractPattern::TagPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "MoveLabel: point id=%u not found; using (0,0) as old position", id);
        return {};
    }
    return {VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, AttrMx, *defPos)),
            VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, AttrMy, *defPos))};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
MoveLabel::MoveLabel(VAbstractPattern *doc, const QPointF &newPos, const quint32 &id, QUndoCommand *parent)
  : MoveAbstractLabel(doc, id, ReadLabelPos(doc, id), newPos, parent)
{
    setText(tr("move point label"));
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveLabel::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *other = dynamic_cast<const MoveLabel *>(command);
    if ((other == nullptr) || other->ElementId() != ElementId())
    {
        return false;
    }

    qCDebug(vUndo, "Merging: new position (%f;%f)", other->GetNewPos().x(), other->GetNewPos().y());
    SetNewPos(other->GetNewPos());
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveLabel::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveLabel);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveLabel::ReadCurrentPos() const -> QPointF
{
    return ReadLabelPos(Doc(), ElementId());
}

//---------------------------------------------------------------------------------------------------------------------
void MoveLabel::WritePos(const QPointF &pos)
{
    QDomElement el = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "MoveLabel: cannot find point id=%u to write position", ElementId());
        return;
    }
    Doc()->SetAttribute(el, AttrMx, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.x())));
    Doc()->SetAttribute(el, AttrMy, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.y())));

    if (auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(ElementId())))
    {
        tool->ChangeLabelPosition(ElementId(), pos);
    }
}
