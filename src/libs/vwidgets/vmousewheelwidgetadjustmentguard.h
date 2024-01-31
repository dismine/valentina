/************************************************************************
 **
 **  @file   vmousewheelwidgetadjustmentguard.h
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
#ifndef VMOUSEWHEELWIDGETADJUSTMENTGUARD_H
#define VMOUSEWHEELWIDGETADJUSTMENTGUARD_H

#include "qtclasshelpermacros.h"
#include <QObject>

// Helps to prevent widget from stealing scroll event.
// Install event filter
// Example:
// VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBox);

class VMouseWheelWidgetAdjustmentGuard : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VMouseWheelWidgetAdjustmentGuard(QObject *parent = nullptr);
    ~VMouseWheelWidgetAdjustmentGuard() override = default;

    static void InstallEventFilter(QWidget *w);

protected:
    auto eventFilter(QObject *o, QEvent *e) -> bool override;

private:
    Q_DISABLE_COPY_MOVE(VMouseWheelWidgetAdjustmentGuard) // NOLINT
};

#endif // VMOUSEWHEELWIDGETADJUSTMENTGUARD_H
