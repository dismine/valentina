/************************************************************************
 **
 **  @file   dialogsetupmultisize.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 9, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "dialogsetupmultisize.h"
#include "ui_dialogsetupmultisize.h"

#include <QPushButton>
#include <QShowEvent>

#include "../mapplication.h"
#include "../vmisc/backport/qoverload.h"

//---------------------------------------------------------------------------------------------------------------------
DialogSetupMultisize::DialogSetupMultisize(Unit unit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetupMultisize),
    m_xDimension(QSharedPointer<VXMeasurementDimension>::create(unit)),
    m_yDimension(QSharedPointer<VYMeasurementDimension>::create(unit)),
    m_wDimension(QSharedPointer<VWMeasurementDimension>::create(unit)),
    m_zDimension(QSharedPointer<VZMeasurementDimension>::create(unit))
{
    ui->setupUi(this);

    ui->labelError->clear();
    ui->checkBoxYDimensionCircumference->setChecked(m_yDimension->IsCircumference());

    InitXDimension();
    InitYDimension();
    InitWDimension();
    InitZDimension();

    // height
    connect(ui->spinBoxXDimensionMinValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMinValueChanged(value, ui->spinBoxXDimensionMaxValue, ui->comboBoxXDimensionStep,
                                         ui->comboBoxXDimensionBase, m_xDimension);
            });
    connect(ui->spinBoxXDimensionMaxValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMaxValueChanged(value, ui->spinBoxXDimensionMinValue, ui->comboBoxXDimensionStep,
                                         ui->comboBoxXDimensionBase, m_xDimension);
            });
    connect(ui->comboBoxXDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionStepChanged(index, ui->comboBoxXDimensionStep, ui->comboBoxXDimensionBase, m_xDimension);
            });
    connect(ui->comboBoxXDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionBaseChanged(index, ui->comboBoxXDimensionBase, m_xDimension);
            });

    // size
    connect(ui->spinBoxYDimensionMinValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMinValueChanged(value, ui->spinBoxYDimensionMaxValue, ui->comboBoxYDimensionStep,
                                         ui->comboBoxYDimensionBase, m_yDimension);
            });
    connect(ui->spinBoxYDimensionMaxValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMaxValueChanged(value, ui->spinBoxYDimensionMinValue, ui->comboBoxYDimensionStep,
                                         ui->comboBoxYDimensionBase, m_yDimension);
            });
    connect(ui->comboBoxYDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionStepChanged(index, ui->comboBoxYDimensionStep, ui->comboBoxYDimensionBase, m_yDimension);
            });
    connect(ui->comboBoxYDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionBaseChanged(index, ui->comboBoxYDimensionBase, m_yDimension);
            });

    // hip
    connect(ui->spinBoxWDimensionMinValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMinValueChanged(value, ui->spinBoxWDimensionMaxValue, ui->comboBoxWDimensionStep,
                                         ui->comboBoxWDimensionBase, m_wDimension);
            });
    connect(ui->spinBoxWDimensionMaxValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMaxValueChanged(value, ui->spinBoxWDimensionMinValue, ui->comboBoxWDimensionStep,
                                         ui->comboBoxWDimensionBase, m_wDimension);
            });
    connect(ui->comboBoxWDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionStepChanged(index, ui->comboBoxWDimensionStep, ui->comboBoxWDimensionBase, m_wDimension);
            });
    connect(ui->comboBoxWDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionBaseChanged(index, ui->comboBoxWDimensionBase, m_wDimension);
            });

    // waist
    connect(ui->spinBoxZDimensionMinValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMinValueChanged(value, ui->spinBoxZDimensionMaxValue, ui->comboBoxZDimensionStep,
                                         ui->comboBoxZDimensionBase, m_zDimension);
            });
    connect(ui->spinBoxZDimensionMaxValue, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value)
            {
                DimensionMaxValueChanged(value, ui->spinBoxZDimensionMinValue, ui->comboBoxZDimensionStep,
                                         ui->comboBoxZDimensionBase, m_zDimension);
            });
    connect(ui->comboBoxZDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionStepChanged(index, ui->comboBoxZDimensionStep, ui->comboBoxZDimensionBase, m_zDimension);
            });
    connect(ui->comboBoxZDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index)
            {
                DimensionBaseChanged(index, ui->comboBoxZDimensionBase, m_zDimension);
            });


    connect(ui->groupBoxXDimension, &QGroupBox::clicked, this, [this](){CheckState();});
    connect(ui->groupBoxYDimension, &QGroupBox::clicked, this, [this](){CheckState();});
    connect(ui->groupBoxWDimension, &QGroupBox::clicked, this, [this](){CheckState();});
    connect(ui->groupBoxZDimension, &QGroupBox::clicked, this, [this](){CheckState();});

    connect(ui->checkBoxFullCircumference, &QCheckBox::stateChanged,
            this, &DialogSetupMultisize::ShowFullCircumference);
    connect(ui->checkBoxYDimensionCircumference, &QCheckBox::stateChanged,
            this, &DialogSetupMultisize::YDimensionCircumferenceChanged);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
DialogSetupMultisize::~DialogSetupMultisize()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<MeasurementDimension_p> DialogSetupMultisize::Dimensions() const
{
    QVector<MeasurementDimension_p> dimensions;

    if (ui->groupBoxXDimension->isChecked())
    {
        dimensions.append(m_xDimension);
    }

    if (ui->groupBoxYDimension->isChecked())
    {
        dimensions.append(m_yDimension);
    }

    if (ui->groupBoxWDimension->isChecked())
    {
        dimensions.append(m_wDimension);
    }

    if (ui->groupBoxZDimension->isChecked())
    {
        dimensions.append(m_zDimension);
    }

    return dimensions;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogSetupMultisize::FullCircumference() const
{
    return ui->checkBoxFullCircumference->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
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

    m_isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::ShowFullCircumference()
{
    auto ShowDimensionFullCircumference = [this](QSpinBox *spinboxMinValue, QSpinBox *spinboxMaxValue,
                                                 QComboBox *comboBoxStep, QComboBox *comboBoxBase,
                                                 const MeasurementDimension_p &dimension)
    {
        SCASSERT(spinboxMinValue != nullptr)
        SCASSERT(spinboxMaxValue != nullptr)
        SCASSERT(comboBoxStep != nullptr)
        SCASSERT(comboBoxBase != nullptr)

        InitDimension(spinboxMinValue, spinboxMaxValue, comboBoxStep, dimension);
        UpdateBase(comboBoxBase, dimension);

        comboBoxBase->blockSignals(true);
        comboBoxBase->setCurrentIndex(-1);
        comboBoxBase->blockSignals(false);
    };

    ShowDimensionFullCircumference(ui->spinBoxYDimensionMinValue, ui->spinBoxYDimensionMaxValue,
                                   ui->comboBoxYDimensionStep, ui->comboBoxYDimensionBase, m_yDimension);
    ShowDimensionFullCircumference(ui->spinBoxWDimensionMinValue, ui->spinBoxWDimensionMaxValue,
                                   ui->comboBoxWDimensionStep, ui->comboBoxWDimensionBase, m_wDimension);
    ShowDimensionFullCircumference(ui->spinBoxZDimensionMinValue, ui->spinBoxZDimensionMaxValue,
                                   ui->comboBoxZDimensionStep, ui->comboBoxZDimensionBase, m_zDimension);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::YDimensionCircumferenceChanged()
{
    bool checked = ui->checkBoxYDimensionCircumference->isChecked();
    m_yDimension->SetCircumference(checked);

    const bool c = m_yDimension->IsCircumference();
    const QString unitStr = c ? " " + UnitsToStr(m_yDimension->Units()) : QString();

    ui->spinBoxYDimensionMinValue->setSuffix(unitStr);
    ui->spinBoxYDimensionMaxValue->setSuffix(unitStr);

    InitDimension(ui->spinBoxYDimensionMinValue, ui->spinBoxYDimensionMaxValue, ui->comboBoxYDimensionStep,
                  m_yDimension);

    UpdateBase(ui->comboBoxYDimensionBase, m_yDimension);

    ui->comboBoxYDimensionBase->blockSignals(true);
    ui->comboBoxYDimensionBase->setCurrentIndex(-1);
    ui->comboBoxYDimensionBase->blockSignals(false);

    bool ok = false;
    const int base = ui->comboBoxYDimensionBase->currentData().toInt(&ok);
    m_yDimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::CheckState()
{
    ui->labelError->clear();

    bool xDimensionValid = true;
    bool yDimensionValid = true;
    bool wDimensionValid = true;
    bool zDimensionValid = true;

    int dimensions = 0;

    auto CheckDimension = [this](QGroupBox *group, bool &dimensionValid, int &dimensions,
                                 const MeasurementDimension_p &dimension)
    {
        SCASSERT(group != nullptr)

        if (group->isChecked())
        {
            dimensionValid = dimension->IsValid();
            ++dimensions;

            if (ui->labelError->text().isEmpty() && not dimensionValid)
            {
                ui->labelError->setText(tr("Please, provide correct data for dimensions"));
            }
        }
    };

    CheckDimension(ui->groupBoxXDimension, xDimensionValid, dimensions, m_xDimension);
    CheckDimension(ui->groupBoxYDimension, yDimensionValid, dimensions, m_yDimension);
    CheckDimension(ui->groupBoxWDimension, wDimensionValid, dimensions, m_wDimension);
    CheckDimension(ui->groupBoxZDimension, zDimensionValid, dimensions, m_zDimension);

    if (ui->labelError->text().isEmpty() && dimensions == 0)
    {
        ui->labelError->setText(tr("Please, select at least one dimension"));
    }
    else if (ui->labelError->text().isEmpty() && dimensions > 3)
    {
        ui->labelError->setText(tr("No more than 3 dimensions allowed"));
    }

    const bool enough = dimensions > 0 && dimensions <= 3;

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(enough && xDimensionValid && yDimensionValid && wDimensionValid && zDimensionValid);

    if (ui->labelError->text().isEmpty())
    {
        ui->labelError->setText(tr("Ready"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitDimensionMinMax(QSpinBox *spinboxMinValue, QSpinBox *spinboxMaxValue,
                                               const MeasurementDimension_p &dimension)
{
    SCASSERT(spinboxMinValue != nullptr)
    SCASSERT(spinboxMaxValue != nullptr)

    dimension->SetMinValue(dimension->RangeMin());
    dimension->SetMaxValue(dimension->RangeMax());

    const bool fc = ui->checkBoxFullCircumference->isChecked();
    const bool c = dimension->IsCircumference();

    spinboxMinValue->blockSignals(true);
    const QString unitStr = " " + UnitsToStr(dimension->Units());
    if (c || dimension->Type() == MeasurementDimension::X)
    {
        spinboxMinValue->setSuffix(unitStr);
    }

    spinboxMinValue->setMinimum(c && fc ? dimension->RangeMin()*2 : dimension->RangeMin());
    spinboxMinValue->setMaximum(c && fc ? dimension->MaxValue()*2 : dimension->MaxValue());
    spinboxMinValue->setValue(c && fc ? dimension->MinValue()*2 : dimension->MinValue());
    spinboxMinValue->blockSignals(false);

    spinboxMaxValue->blockSignals(true);
    if (c || dimension->Type() == MeasurementDimension::X)
    {
        spinboxMaxValue->setSuffix(unitStr);
    }
    spinboxMaxValue->setMinimum(c && fc ? dimension->MinValue()*2 : dimension->MinValue());
    spinboxMaxValue->setMaximum(c && fc ? dimension->RangeMax()*2 : dimension->RangeMax());
    spinboxMaxValue->setValue(c && fc ? dimension->RangeMax()*2 : dimension->RangeMax());
    spinboxMaxValue->setValue(c && fc ? dimension->MaxValue()*2 : dimension->MaxValue());
    spinboxMaxValue->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitDimensionStep(QComboBox *comboBoxStep,
                                             const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxStep != nullptr)

    const bool fc = ui->checkBoxFullCircumference->isChecked();
    const bool c = dimension->IsCircumference();
    const QString unitStr = " " + UnitsToStr(dimension->Units());

    dimension->SetStep(-1);

    comboBoxStep->blockSignals(true);
    const QVector<int> steps = dimension->ValidSteps();
    comboBoxStep->clear();
    for(auto step : steps)
    {
        comboBoxStep->addItem(QString("%1%2").arg(c && fc ? step*2 : step)
                                  .arg(c || dimension->Type() == MeasurementDimension::X ? unitStr : QString()), step);
    }

    comboBoxStep->setCurrentIndex(-1); // force a user to select
    comboBoxStep->blockSignals(false);

    bool ok = false;
    const int step = comboBoxStep->currentData().toInt(&ok);
    dimension->SetStep(ok ? step : -1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitDimension(QSpinBox *spinboxMinValue, QSpinBox *spinboxMaxValue, QComboBox *comboBoxStep,
                                         const MeasurementDimension_p &dimension)
{
    InitDimensionMinMax(spinboxMinValue, spinboxMaxValue, dimension);
    InitDimensionStep(comboBoxStep, dimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitXDimension()
{
    InitDimension(ui->spinBoxXDimensionMinValue, ui->spinBoxXDimensionMaxValue, ui->comboBoxXDimensionStep,
                  m_xDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitYDimension()
{
    InitDimension(ui->spinBoxYDimensionMinValue, ui->spinBoxYDimensionMaxValue, ui->comboBoxYDimensionStep,
                  m_yDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitWDimension()
{
    InitDimension(ui->spinBoxWDimensionMinValue, ui->spinBoxWDimensionMaxValue, ui->comboBoxWDimensionStep,
                  m_wDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitZDimension()
{
    InitDimension(ui->spinBoxZDimensionMinValue, ui->spinBoxZDimensionMaxValue, ui->comboBoxZDimensionStep,
                  m_zDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::DimensionMinValueChanged(int value, QSpinBox *spinboxMaxValue, QComboBox *comboBoxStep,
                                                    QComboBox *comboBoxBase,
                                                    const MeasurementDimension_p &dimension)
{
    SCASSERT(spinboxMaxValue != nullptr)
    SCASSERT(comboBoxStep != nullptr)
    SCASSERT(comboBoxBase != nullptr)

    const bool fc = ui->checkBoxFullCircumference->isChecked();
    const bool c = dimension->IsCircumference();

    dimension->SetMinValue(c && fc ? value / 2 : value);

    spinboxMaxValue->blockSignals(true);
    spinboxMaxValue->setMinimum(value);
    spinboxMaxValue->blockSignals(false);

    dimension->SetMaxValue(c && fc ? spinboxMaxValue->value() / 2 : spinboxMaxValue->value());

    UpdateSteps(comboBoxStep, dimension);
    UpdateBase(comboBoxBase, dimension);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::DimensionMaxValueChanged(int value, QSpinBox *spinboxMinValue, QComboBox *comboBoxStep,
                                                    QComboBox *comboBoxBase,
                                                    const MeasurementDimension_p &dimension)
{
    SCASSERT(spinboxMinValue != nullptr)
    SCASSERT(comboBoxStep != nullptr)
    SCASSERT(comboBoxBase != nullptr)

    const bool fc = ui->checkBoxFullCircumference->isChecked();
    const bool c = dimension->IsCircumference();

    dimension->SetMaxValue(c && fc ? value / 2 : value);

    spinboxMinValue->blockSignals(true);
    spinboxMinValue->setMaximum(value);
    spinboxMinValue->blockSignals(false);

    dimension->SetMinValue(c && fc ? spinboxMinValue->value() / 2 : spinboxMinValue->value());

    UpdateSteps(comboBoxStep, dimension);
    UpdateBase(comboBoxBase, dimension);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::DimensionStepChanged(int index, QComboBox *comboBoxStep, QComboBox *comboBoxBase,
                                                const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxStep != nullptr)
    SCASSERT(comboBoxBase != nullptr)

    bool ok = false;
    const int step = comboBoxStep->itemData(index).toInt(&ok);
    dimension->SetStep(ok ? step : -1);

    UpdateBase(comboBoxBase, dimension);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::DimensionBaseChanged(int index, QComboBox *comboBoxBase,
                                                const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxBase != nullptr)

    bool ok = false;
    const int base = comboBoxBase->itemData(index).toInt(&ok);
    dimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::UpdateSteps(QComboBox *comboBoxStep,
                                       const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxStep != nullptr)

    int oldStep = -1;
    if (comboBoxStep->currentIndex() != -1)
    {
        oldStep = comboBoxStep->currentData().toInt();
    }

    comboBoxStep->blockSignals(true);

    const QString unitStr = " " + UnitsToStr(dimension->Units());
    const QVector<int> steps = dimension->ValidSteps();
    comboBoxStep->clear();

    const bool fc = ui->checkBoxFullCircumference->isChecked();
    const bool c = dimension->IsCircumference();

    for(auto step : steps)
    {
        comboBoxStep->addItem(QString("%1%2").arg(c && fc ? step * 2 : step)
                                  .arg(c ? unitStr : QString()), step);
    }

    comboBoxStep->setCurrentIndex(comboBoxStep->findData(oldStep));
    comboBoxStep->blockSignals(false);

    bool ok = false;
    const int step = comboBoxStep->currentData().toInt(&ok);
    dimension->SetStep(ok ? step : -1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::UpdateBase(QComboBox *comboBoxBase,
                                      const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxBase != nullptr)

    int oldBase = -1;
    if (comboBoxBase->currentIndex() != -1)
    {
        oldBase = comboBoxBase->currentData().toInt();
    }

    comboBoxBase->blockSignals(true);

    const QString unitStr = " " + UnitsToStr(dimension->Units());
    const QVector<int> bases = dimension->ValidBases();
    comboBoxBase->clear();
    const bool fc = ui->checkBoxFullCircumference->isChecked();
    const bool c = dimension->IsCircumference();

    for(auto base : bases)
    {
        comboBoxBase->addItem(QString("%1%2").arg(c && fc ? base * 2 : base)
                                  .arg(c || dimension->Type() == MeasurementDimension::X ? unitStr : QString()), base);
    }

    comboBoxBase->setCurrentIndex(comboBoxBase->findData(oldBase));
    comboBoxBase->blockSignals(false);

    bool ok = false;
    const int base = comboBoxBase->currentData().toInt(&ok);
    dimension->SetBaseValue(ok ? base : -1);
}

