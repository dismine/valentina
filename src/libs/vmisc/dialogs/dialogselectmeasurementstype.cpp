/************************************************************************
 **
 **  @file   dialogselectmeasurementstype.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 9, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "dialogselectmeasurementstype.h"
#include "ui_dialogselectmeasurementstype.h"
#include "vmisc/def.h"

#include <QShowEvent>

//---------------------------------------------------------------------------------------------------------------------
DialogSelectMeasurementsType::DialogSelectMeasurementsType(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogSelectMeasurementsType)
{
    ui->setupUi(this);

    connect(ui->toolButtonIndividualMeasurements,
            &QToolButton::clicked,
            this,
            [this]()
            {
                m_type = MeasurementsType::Individual;
                accept();
            });

    connect(ui->toolButtonMultisizeMeasurements,
            &QToolButton::clicked,
            this,
            [this]()
            {
                m_type = MeasurementsType::Multisize;
                accept();
            });

    // QToolButton defaults to Fixed size policy, which makes the layout honour
    // sizeHint() exactly. The global custom stylesheet rule
    //   QDialog QToolButton[autoRaise="false"][text] { width: 1.2em; }
    // drives sizeHint() down to ~14 px.  Switching to Expanding lets the
    // HBoxLayout distribute available width equally once we clear maxWidth in
    // showEvent (after polishing has run).
    for (auto *btn : {ui->toolButtonIndividualMeasurements, ui->toolButtonMultisizeMeasurements})
    {
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

//---------------------------------------------------------------------------------------------------------------------
DialogSelectMeasurementsType::~DialogSelectMeasurementsType()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSelectMeasurementsType::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }

    // Polishing applies the global rule
    //   QDialog QToolButton[autoRaise="false"][text] { width: 1.2em; }
    // via setFixedWidth(~14 px), which also clamps maximumWidth to 14.
    // The Expanding policy (set in the constructor) can only take effect once
    // maxWidth is cleared; then the HBoxLayout distributes space equally.
    for (auto *btn : {ui->toolButtonIndividualMeasurements, ui->toolButtonMultisizeMeasurements})
    {
        btn->setMaximumWidth(QWIDGETSIZE_MAX);
    }
    layout()->invalidate();
    layout()->activate();

    setMaximumSize(size());
    setMinimumSize(size());

    m_isInitialized = true; // first show windows are held
}
