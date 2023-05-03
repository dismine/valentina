/************************************************************************
 **
 **  @file   movespline.h
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

#ifndef MOVESPLINE_H
#define MOVESPLINE_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vspline.h"
#include "vundocommand.h"

class QGraphicsScene;

class MoveSpline : public VUndoCommand
{
    Q_OBJECT // NOLINT
public:
    MoveSpline(VAbstractPattern *doc, const VSpline &oldSpl, const VSpline &newSpl, const quint32 &id,
               QUndoCommand *parent = nullptr);
    virtual ~MoveSpline() override;
    virtual void undo() override;
    virtual void redo() override;
    virtual auto mergeWith(const QUndoCommand *command) -> bool override;
    virtual auto id() const -> int override;
    auto getSplineId() const -> quint32;
    auto getNewSpline() const -> VSpline;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MoveSpline) // NOLINT
    VSpline  oldSpline;
    VSpline  newSpline;
    QGraphicsScene *scene;
    void         Do(const VSpline &spl);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSpline::getSplineId() const -> quint32
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto MoveSpline::getNewSpline() const -> VSpline
{
    return newSpline;
}

#endif // MOVESPLINE_H
