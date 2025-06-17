/************************************************************************
 **
 **  @file   vmousewheelwidgetadjustmentguard.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2024 Valentina project
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
#include "vmousewheelwidgetadjustmentguard.h"

#include <QEvent>
#include <QWidget>

//---------------------------------------------------------------------------------------------------------------------
VMouseWheelWidgetAdjustmentGuard::VMouseWheelWidgetAdjustmentGuard(QObject *parent)
  : QObject{parent}
{
}

//---------------------------------------------------------------------------------------------------------------------
void VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(QWidget *w)
{
    if (w == nullptr)
    {
        return;
    }

    w->setFocusPolicy(Qt::StrongFocus);
    w->installEventFilter(new VMouseWheelWidgetAdjustmentGuard(w));
}

//---------------------------------------------------------------------------------------------------------------------
auto VMouseWheelWidgetAdjustmentGuard::eventFilter(QObject *o, QEvent *e) -> bool
{
    const QWidget *widget = qobject_cast<QWidget *>(o);
    if (e->type() == QEvent::Wheel && (widget != nullptr) && !widget->hasFocus())
    {
        e->ignore();
        return true;
    }

    return QObject::eventFilter(o, e);
}
