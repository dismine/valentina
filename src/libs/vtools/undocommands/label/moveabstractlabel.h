/************************************************************************
 **
 **  @file   moveabstractlabel.h
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

#ifndef MOVEABSTRACTLABEL_H
#define MOVEABSTRACTLABEL_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vundocommand.h"

class QGraphicsScene;

class MoveAbstractLabel : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    MoveAbstractLabel(VAbstractPattern *doc,
                      quint32 pointId,
                      const QPointF &oldPos,
                      const QPointF &newPos,
                      QUndoCommand *parent = nullptr);
    ~MoveAbstractLabel() override = default;

    void undo() final;
    void redo() final;

    auto GetNewPos() const noexcept -> QPointF;
    auto GetOldPos() const noexcept -> QPointF;

protected:
    virtual auto ReadCurrentPos() const -> QPointF = 0;
    virtual void WritePos(const QPointF &pos) = 0;

    void SetNewPos(const QPointF &pos);

private:
    Q_DISABLE_COPY_MOVE(MoveAbstractLabel) // NOLINT

    QPointF m_oldPos;
    QPointF m_newPos;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveAbstractLabel::GetNewPos() const noexcept -> QPointF
{
    return m_newPos;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveAbstractLabel::GetOldPos() const noexcept -> QPointF
{
    return m_oldPos;
}

//---------------------------------------------------------------------------------------------------------------------
inline void MoveAbstractLabel::SetNewPos(const QPointF &pos)
{
    m_newPos = pos;
}

#endif // MOVEABSTRACTLABEL_H
