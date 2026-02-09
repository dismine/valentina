/************************************************************************
 **
 **  @file   dialognewmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "dialognewmeasurements.h"
#include "ui_dialognewmeasurements.h"

#include <QShowEvent>

//---------------------------------------------------------------------------------------------------------------------
DialogNewMeasurements::DialogNewMeasurements(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogNewMeasurements),
    m_isInitialized(false)
{
    ui->setupUi(this);

    InitMTypes();
    InitUnits();
}

//---------------------------------------------------------------------------------------------------------------------
DialogNewMeasurements::~DialogNewMeasurements()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogNewMeasurements::Type() const -> MeasurementsType
{
    return static_cast<MeasurementsType>(ui->comboBoxMType->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogNewMeasurements::MUnit() const -> Unit
{
    return static_cast<Unit>(ui->comboBoxUnit->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNewMeasurements::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        InitMTypes();
        InitUnits();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNewMeasurements::showEvent(QShowEvent *event)
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
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNewMeasurements::InitMTypes()
{
    auto val = static_cast<int>(MeasurementsType::Unknown);
    if (ui->comboBoxMType->currentIndex() != -1)
    {
        val = ui->comboBoxMType->currentData().toInt();
    }

    {
        const QSignalBlocker blocker(ui->comboBoxMType);
        ui->comboBoxMType->clear();
        ui->comboBoxMType->addItem(tr("Individual"), static_cast<int>(MeasurementsType::Individual));
        ui->comboBoxMType->addItem(tr("Multisize"), static_cast<int>(MeasurementsType::Multisize));
    }

    int const index = ui->comboBoxMType->findData(val);
    if (index != -1)
    {
        ui->comboBoxMType->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNewMeasurements::InitUnits()
{
    auto val = static_cast<int>(Unit::Cm);
    if (ui->comboBoxUnit->currentIndex() != -1)
    {
        val = ui->comboBoxUnit->currentData().toInt();
    }

    {
        const QSignalBlocker blocker(ui->comboBoxUnit);
        ui->comboBoxUnit->clear();

        ui->comboBoxUnit->addItem(tr("Centimeters"), static_cast<int>(Unit::Cm));
        ui->comboBoxUnit->addItem(tr("Millimiters"), static_cast<int>(Unit::Mm));
        ui->comboBoxUnit->addItem(tr("Inches"), static_cast<int>(Unit::Inch));

        ui->comboBoxUnit->setCurrentIndex(-1);
    }

    int index = ui->comboBoxUnit->findData(val);
    if (index != -1)
    {
        ui->comboBoxUnit->setCurrentIndex(index);
    }
    else
    {
        index = ui->comboBoxUnit->findData(static_cast<int>(Unit::Cm));
        ui->comboBoxUnit->setCurrentIndex(index);
    }
}
