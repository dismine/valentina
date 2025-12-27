/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VINTERACTIVETOOL_H
#define VINTERACTIVETOOL_H

#include "vabstracttool.h"
#include "../dialogs/tools/dialogtool.h"

#include <QtCore/qglobal.h>

enum class RemoveStatus : quint8
{
    Removable, // Tool can be removed
    Blocked,   // Tool has dependencies which prevent removing
    Locked,    // Tool cannot be removed
    Pending    // Collecting data about dependecies
};

class VInteractiveTool : public VAbstractTool
{
    Q_OBJECT // NOLINT
public:
    VInteractiveTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent = nullptr);
    ~VInteractiveTool() override = default;

    void DialogLinkDestroy();

    virtual auto IsRemovable() const -> RemoveStatus;

public slots:
    virtual void FullUpdateFromGuiOk(int result);
    virtual void FullUpdateFromGuiApply();

protected:
    /** @brief m_dialog tool's dialog options.*/
    QPointer<DialogTool> m_dialog{}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    /** @brief setDialog set dialog when user want change tool option. */
    virtual void SetDialog()
    { /*do nothing by default*/
    }
    virtual void SaveDialogChange(const QString &undoText = QString())=0;

private:
    Q_DISABLE_COPY_MOVE(VInteractiveTool) // NOLINT
};

#endif // VINTERACTIVETOOL_H
