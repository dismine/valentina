/************************************************************************
 **
 **  @file   movesplinepath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 6, 2014
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

#ifndef MOVESPLINEPATH_H
#define MOVESPLINEPATH_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vsplinepath.h"
#include "vundocommand.h"

class QGraphicsScene;

class MoveSplinePath : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    MoveSplinePath(VAbstractPattern *doc, const VSplinePath &oldSplPath, const VSplinePath &newSplPath,
                   const quint32 &id, QUndoCommand *parent = nullptr);
    ~MoveSplinePath() = default;

    void undo() override;
    void redo() override;

    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

    auto getSplinePathId() const -> quint32;
    auto getNewSplinePath() const -> VSplinePath;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MoveSplinePath) // NOLINT
    VSplinePath oldSplinePath;
    VSplinePath newSplinePath;
    QGraphicsScene *scene;
    void Do(const VSplinePath &splPath);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSplinePath::getSplinePathId() const -> quint32
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSplinePath::getNewSplinePath() const -> VSplinePath
{
    return newSplinePath;
}

#endif // MOVESPLINEPATH_H
