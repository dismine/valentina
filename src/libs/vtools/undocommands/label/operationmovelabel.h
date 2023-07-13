/************************************************************************
 **
 **  @file   operationmovelabel.h
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

#ifndef OPERATIONMOVELABEL_H
#define OPERATIONMOVELABEL_H

#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "moveabstractlabel.h"

class OperationMoveLabel : public MoveAbstractLabel
{
    Q_OBJECT // NOLINT

public:
    OperationMoveLabel(quint32 idTool, VAbstractPattern *doc, const QPointF &pos, quint32 idPoint,
                       QUndoCommand *parent = nullptr);
    ~OperationMoveLabel() override = default;

    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

    auto GetToolId() const -> quint32;

protected:
    void Do(const QPointF &pos) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(OperationMoveLabel) // NOLINT
    quint32 m_idTool;
    // Need for resizing scene rect
    QGraphicsScene *m_scene;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto OperationMoveLabel::GetToolId() const -> quint32
{
    return m_idTool;
}

#endif // OPERATIONMOVELABEL_H
