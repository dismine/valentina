/************************************************************************
 **
 **  @file   renameobject.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 1, 2026
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
#ifndef RENAMEOBJECT_H
#define RENAMEOBJECT_H

#include "../ifc/xml/vpatterngraphnode.h"
#include "vundocommand.h"

class RenameLabel : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    RenameLabel(QString oldLabel, QString newLabel, VAbstractPattern *doc, quint32 id, QUndoCommand *parent = nullptr);
    ~RenameLabel() override = default;

    void undo() override;
    void redo() override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(RenameLabel) // NOLINT
    QString m_oldLabel;
    QString m_newLabel;
    QVector<VNode> m_dependencies{};

    void RenameLabelInFormulas(const QString &oldLabel, const QString &newLabel);
};

#endif // RENAMEOBJECT_H
