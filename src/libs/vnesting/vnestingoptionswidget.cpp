/************************************************************************
 **
 **  @file   vnestingoptionswidget.cpp
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
#include "vnestingoptionswidget.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_vnestingoptionswidget.h"

#include <QEvent>

//---------------------------------------------------------------------------------------------------------------------
VNestingOptionsWidget::VNestingOptionsWidget(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::VNestingOptionsWidget>())
{
    ui->setupUi(this);
    const qreal saved = VAbstractApplication::VApp()->Settings()->GetNestingTimeBudgetS();
    ui->spinBoxTimeBudget->setValue(qMax(1, qRound(saved)));
    connect(ui->spinBoxTimeBudget, qOverload<int>(&QSpinBox::valueChanged), this, [](int seconds)
            { VAbstractApplication::VApp()->Settings()->SetNestingTimeBudgetS(seconds); });
}

//---------------------------------------------------------------------------------------------------------------------
VNestingOptionsWidget::~VNestingOptionsWidget() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VNestingOptionsWidget::Options() const -> VNestingOptions
{
    VNestingOptions options;
    options.timeBudgetS = TimeBudgetS();
    return options;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestingOptionsWidget::TimeBudgetS() const -> qreal
{
    return ui->spinBoxTimeBudget->value();
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
