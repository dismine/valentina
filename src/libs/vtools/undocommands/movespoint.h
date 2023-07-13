/************************************************************************
 **
 **  @file   movespoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2014
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

#ifndef MOVESPOINT_H
#define MOVESPOINT_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class QGraphicsScene;

class MoveSPoint : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    MoveSPoint(VAbstractPattern *doc, const double &x, const double &y, const quint32 &id, QGraphicsScene *scene,
               QUndoCommand *parent = nullptr);
    ~MoveSPoint() override;

    void undo() override;
    void redo() override;

    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

    auto getSPointId() const -> quint32;
    auto getNewX() const -> double;
    auto getNewY() const -> double;

    void Do(double x, double y);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MoveSPoint) // NOLINT
    double oldX;
    double oldY;
    double newX;
    double newY;
    QGraphicsScene *scene;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSPoint::getSPointId() const -> quint32
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSPoint::getNewX() const -> double
{
    return newX;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSPoint::getNewY() const -> double
{
    return newY;
}

#endif // MOVESPOINT_H
