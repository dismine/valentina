/************************************************************************
 **
 **  @file   vnestingoptionswidget.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 October, 2026
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
#ifndef VNESTINGOPTIONSWIDGET_H
#define VNESTINGOPTIONSWIDGET_H

#include "../vlayout/vnestingframe.h"

#include <memory>
#include <QWidget>

namespace Ui
{
class VNestingOptionsWidget;
}

/**
 * @brief The seconds the user types for timeBudgetS. Quick nest and Puzzle both use this widget.
 * The host dialog owns the rotations and reads Options().
 */
class VNestingOptionsWidget : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VNestingOptionsWidget(QWidget *parent = nullptr);
    ~VNestingOptionsWidget() override;

    auto Options() const -> VNestingOptions; // rotations and length are left at their defaults: the host sets them
    auto TimeBudgetS() const -> qreal;

protected:
    void changeEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(VNestingOptionsWidget) // NOLINT

    std::unique_ptr<Ui::VNestingOptionsWidget> ui;
};

#endif // VNESTINGOPTIONSWIDGET_H
