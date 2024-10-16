/************************************************************************
 **
 **  @file   toggledetailinlayout.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2016
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

#ifndef TOGGLEDETAILINLAYOUT_H
#define TOGGLEDETAILINLAYOUT_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class TogglePieceInLayout : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    TogglePieceInLayout(quint32 id, bool state, VContainer *data, VAbstractPattern *doc,
                        QUndoCommand *parent = nullptr);
    ~TogglePieceInLayout() override = default;

    void undo() override;
    void redo() override;

signals:
    void Toggled(quint32 id);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(TogglePieceInLayout) // NOLINT
    quint32 m_id;
    VContainer *m_data;
    bool m_oldState;
    bool m_newState;

    void Do(bool state);
};

class ToggleHideMainPath : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    ToggleHideMainPath(quint32 id, bool state, VContainer *data, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    ~ToggleHideMainPath() override = default;

    void undo() override;
    void redo() override;

signals:
    void Toggled(quint32 id);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(ToggleHideMainPath) // NOLINT
    quint32 m_id;
    VContainer *m_data;
    bool m_oldState;
    bool m_newState;

    void Do(bool state);
};

class ToggleShowFullPiece : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    ToggleShowFullPiece(quint32 id, bool state, VContainer *data, VAbstractPattern *doc,
                        QUndoCommand *parent = nullptr);
    ~ToggleShowFullPiece() override = default;

    void undo() override;
    void redo() override;

signals:
    void Toggled(quint32 id);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(ToggleShowFullPiece) // NOLINT
    quint32 m_id;
    VContainer *m_data;
    bool m_oldState;
    bool m_newState;

    void Do(bool state);
};

enum class ForceForbidFlippingType : qint8
{
    ForceFlipping,
    ForbidFlipping
};

class TogglePieceForceForbidFlipping : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    TogglePieceForceForbidFlipping(quint32 id, bool state, ForceForbidFlippingType type, VContainer *data,
                                   VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    ~TogglePieceForceForbidFlipping() override = default;
    void undo() override;
    void redo() override;

private:
    Q_DISABLE_COPY_MOVE(TogglePieceForceForbidFlipping) // NOLINT
    quint32 m_id;
    VContainer *m_data;
    ForceForbidFlippingType m_type;
    bool m_oldForceState{false};
    bool m_newForceState{false};
    bool m_oldForbidState{false};
    bool m_newForbidState{false};
};

#endif // TOGGLEDETAILINLAYOUT_H
