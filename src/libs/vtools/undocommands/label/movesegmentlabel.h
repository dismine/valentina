/************************************************************************
 **
 **  @file   movesegmentlabel.h
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
#ifndef MOVESEGMENTLABEL_H
#define MOVESEGMENTLABEL_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../../tools/toolsdef.h"
#include "moveabstractlabel.h"

class MoveSegmentLabel : public MoveAbstractLabel
{
    Q_OBJECT // NOLINT

public:
    MoveSegmentLabel(
        VAbstractPattern *doc, const QPointF &newPos, quint32 id, SegmentLabel segment, QUndoCommand *parent = nullptr);
    ~MoveSegmentLabel() override = default;

    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

protected:
    auto ReadCurrentPos() const -> QPointF override;
    void WritePos(const QPointF &pos) override;

private:
    Q_DISABLE_COPY_MOVE(MoveSegmentLabel) // NOLINT

    SegmentLabel m_segment;
};

#endif // MOVESEGMENTLABEL_H
