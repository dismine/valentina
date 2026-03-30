/************************************************************************
 **
 **  @file   movesegmentlabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 3, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#include "movesegmentlabel.h"

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
struct SegmentAttrs
{
    QString mx;
    QString my;
};

//---------------------------------------------------------------------------------------------------------------------
auto AttrsForSegment(SegmentLabel segment) -> SegmentAttrs
{
    switch (segment)
    {
        case SegmentLabel::Segment1:
            return {AttrSegment1Mx, AttrSegment1My};
        case SegmentLabel::Segment2:
            return {AttrSegment2Mx, AttrSegment2My};
        case SegmentLabel::Segment3:
            return {AttrSegment3Mx, AttrSegment3My};
        case SegmentLabel::Segment4:
            return {AttrSegment4Mx, AttrSegment4My};
        default:
            break;
    }
    Q_UNREACHABLE();
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadLabelPos(VAbstractPattern *doc, quint32 id, SegmentLabel segment) -> QPointF
{
    const QDomElement el = doc->FindElementById(id, VAbstractPattern::TagPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "MoveSegmentLabel: point id=%u not found; using (0,0) as old position", id);
        return {};
    }

    const auto [mxAttr, myAttr] = AttrsForSegment(segment);
    return {VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, mxAttr, *defPos)),
            VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(el, myAttr, *defPos))};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
MoveSegmentLabel::MoveSegmentLabel(
    VAbstractPattern *doc, const QPointF &newPos, quint32 id, SegmentLabel segment, QUndoCommand *parent)
  : MoveAbstractLabel(doc, id, ReadLabelPos(doc, id, segment), newPos, parent),
    m_segment(segment)
{
    setText(tr("move segment label"));
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSegmentLabel::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *other = dynamic_cast<const MoveSegmentLabel *>(command);
    if ((other == nullptr) || other->ElementId() != ElementId() || other->m_segment != m_segment)
    {
        return false;
    }

    qCDebug(vUndo, "Merging: new position (%f;%f)", other->GetNewPos().x(), other->GetNewPos().y());
    SetNewPos(other->GetNewPos());
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSegmentLabel::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveSegmentLabel);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSegmentLabel::ReadCurrentPos() const -> QPointF
{
    return ReadLabelPos(Doc(), ElementId(), m_segment);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSegmentLabel::WritePos(const QPointF &pos)
{
    QDomElement el = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPoint);
    if (!el.isElement())
    {
        qCWarning(vUndo, "MoveSegmentLabel: cannot find point id=%u to write position", ElementId());
        return;
    }

    const auto [mxAttr, myAttr] = AttrsForSegment(m_segment);
    Doc()->SetAttribute(el, mxAttr, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.x())));
    Doc()->SetAttribute(el, myAttr, QString::number(VAbstractValApplication::VApp()->fromPixel(pos.y())));

    if (auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(ElementId())))
    {
        tool->ChangeSegmentLabelPosition(m_segment, pos);
    }
}
