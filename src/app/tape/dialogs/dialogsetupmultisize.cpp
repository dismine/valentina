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

namespace
{
void InitDimensionTitle(QGroupBox *group, const MeasurementDimension_p &dimension)
{
    SCASSERT(group != nullptr)
    group->setTitle(QStringLiteral("%1 (%2)").arg(dimension->Name(), dimension->Axis()));
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogSetupMultisize::DialogSetupMultisize(Unit unit, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogSetupMultisize),
    m_xDimension(QSharedPointer<VXMeasurementDimension>::create(unit)),
    m_yDimension(QSharedPointer<VYMeasurementDimension>::create(unit)),
    m_wDimension(QSharedPointer<VWMeasurementDimension>::create(unit)),
    m_zDimension(QSharedPointer<VZMeasurementDimension>::create(unit))
{
    ui->setupUi(this);

    ui->labelError->clear();

    InitXDimension();
    InitYDimension();
    InitWDimension();
    InitZDimension();

    // height
    connect(ui->doubleSpinBoxXDimensionMinValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMinValueChanged(value, ui->doubleSpinBoxXDimensionMaxValue, ui->comboBoxXDimensionStep,
                                         ui->comboBoxXDimensionBase, m_xDimension);
            });
    connect(ui->doubleSpinBoxXDimensionMaxValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMaxValueChanged(value, ui->doubleSpinBoxXDimensionMinValue, ui->comboBoxXDimensionStep,
                                         ui->comboBoxXDimensionBase, m_xDimension);
            });
    connect(ui->comboBoxXDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index)
            { DimensionStepChanged(index, ui->comboBoxXDimensionStep, ui->comboBoxXDimensionBase, m_xDimension); });
    connect(ui->comboBoxXDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { DimensionBaseChanged(index, ui->comboBoxXDimensionBase, m_xDimension); });

    // size
    connect(ui->doubleSpinBoxYDimensionMinValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMinValueChanged(value, ui->doubleSpinBoxYDimensionMaxValue, ui->comboBoxYDimensionStep,
                                         ui->comboBoxYDimensionBase, m_yDimension);
            });
    connect(ui->doubleSpinBoxYDimensionMaxValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMaxValueChanged(value, ui->doubleSpinBoxYDimensionMinValue, ui->comboBoxYDimensionStep,
                                         ui->comboBoxYDimensionBase, m_yDimension);
            });
    connect(ui->comboBoxYDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index)
            { DimensionStepChanged(index, ui->comboBoxYDimensionStep, ui->comboBoxYDimensionBase, m_yDimension); });
    connect(ui->comboBoxYDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { DimensionBaseChanged(index, ui->comboBoxYDimensionBase, m_yDimension); });

    // hip
    connect(ui->doubleSpinBoxWDimensionMinValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMinValueChanged(value, ui->doubleSpinBoxWDimensionMaxValue, ui->comboBoxWDimensionStep,
                                         ui->comboBoxWDimensionBase, m_wDimension);
            });
    connect(ui->doubleSpinBoxWDimensionMaxValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMaxValueChanged(value, ui->doubleSpinBoxWDimensionMinValue, ui->comboBoxWDimensionStep,
                                         ui->comboBoxWDimensionBase, m_wDimension);
            });
    connect(ui->comboBoxWDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index)
            { DimensionStepChanged(index, ui->comboBoxWDimensionStep, ui->comboBoxWDimensionBase, m_wDimension); });
    connect(ui->comboBoxWDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { DimensionBaseChanged(index, ui->comboBoxWDimensionBase, m_wDimension); });

    // waist
    connect(ui->doubleSpinBoxZDimensionMinValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMinValueChanged(value, ui->doubleSpinBoxZDimensionMaxValue, ui->comboBoxZDimensionStep,
                                         ui->comboBoxZDimensionBase, m_zDimension);
            });
    connect(ui->doubleSpinBoxZDimensionMaxValue, QOverload<qreal>::of(&QDoubleSpinBox::valueChanged), this,
            [this](qreal value)
            {
                DimensionMaxValueChanged(value, ui->doubleSpinBoxZDimensionMinValue, ui->comboBoxZDimensionStep,
                                         ui->comboBoxZDimensionBase, m_zDimension);
            });
    connect(ui->comboBoxZDimensionStep, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index)
            { DimensionStepChanged(index, ui->comboBoxZDimensionStep, ui->comboBoxZDimensionBase, m_zDimension); });
    connect(ui->comboBoxZDimensionBase, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { DimensionBaseChanged(index, ui->comboBoxZDimensionBase, m_zDimension); });

    connect(ui->groupBoxXDimension, &QGroupBox::clicked, this, [this]() { CheckState(); });
    connect(ui->groupBoxYDimension, &QGroupBox::clicked, this, [this]() { CheckState(); });
    connect(ui->groupBoxWDimension, &QGroupBox::clicked, this, [this]() { CheckState(); });
    connect(ui->groupBoxZDimension, &QGroupBox::clicked, this, [this]() { CheckState(); });

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(ui->checkBoxFullCircumference, &QCheckBox::checkStateChanged, this,
            &DialogSetupMultisize::ShowFullCircumference);

    connect(ui->checkBoxXDimensionBodyMeasurement, &QCheckBox::checkStateChanged, this,
            &DialogSetupMultisize::XDimensionBodyMeasurementChanged);
    connect(ui->checkBoxYDimensionBodyMeasurement, &QCheckBox::checkStateChanged, this,
            &DialogSetupMultisize::YDimensionBodyMeasurementChanged);
    connect(ui->checkBoxWDimensionBodyMeasurement, &QCheckBox::checkStateChanged, this,
            &DialogSetupMultisize::WDimensionBodyMeasurementChanged);
    connect(ui->checkBoxZDimensionBodyMeasurement, &QCheckBox::checkStateChanged, this,
            &DialogSetupMultisize::ZDimensionBodyMeasurementChanged);
#else
    connect(ui->checkBoxFullCircumference, &QCheckBox::stateChanged, this,
            &DialogSetupMultisize::ShowFullCircumference);

    connect(ui->checkBoxXDimensionBodyMeasurement, &QCheckBox::stateChanged, this,
            &DialogSetupMultisize::XDimensionBodyMeasurementChanged);
    connect(ui->checkBoxYDimensionBodyMeasurement, &QCheckBox::stateChanged, this,
            &DialogSetupMultisize::YDimensionBodyMeasurementChanged);
    connect(ui->checkBoxWDimensionBodyMeasurement, &QCheckBox::stateChanged, this,
            &DialogSetupMultisize::WDimensionBodyMeasurementChanged);
    connect(ui->checkBoxZDimensionBodyMeasurement, &QCheckBox::stateChanged, this,
            &DialogSetupMultisize::ZDimensionBodyMeasurementChanged);
#endif

    connect(ui->lineEditCustomXDimensionName, &QLineEdit::textChanged, this,
            [this](const QString &text)
            {
                m_xDimension->SetCustomName(text);
                InitDimensionTitle(ui->groupBoxXDimension, m_xDimension);

                CheckState();
            });

    connect(ui->lineEditCustomYDimensionName, &QLineEdit::textChanged, this,
            [this](const QString &text)
            {
                m_yDimension->SetCustomName(text);
                InitDimensionTitle(ui->groupBoxYDimension, m_yDimension);

                CheckState();
            });

    connect(ui->lineEditCustomWDimensionName, &QLineEdit::textChanged, this,
            [this](const QString &text)
            {
                m_wDimension->SetCustomName(text);
                InitDimensionTitle(ui->groupBoxWDimension, m_wDimension);

                CheckState();
            });

    connect(ui->lineEditCustomZDimensionName, &QLineEdit::textChanged, this,
            [this](const QString &text)
            {
                m_zDimension->SetCustomName(text);
                InitDimensionTitle(ui->groupBoxZDimension, m_zDimension);

                CheckState();
            });

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
DialogSetupMultisize::~DialogSetupMultisize()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSetupMultisize::Dimensions() const -> QVector<MeasurementDimension_p>
{
    QVector<MeasurementDimension_p> dimensions;

    if (ui->groupBoxXDimension->isChecked())
    {
        if (not ui->groupBoxCustomXDimensionName->isChecked())
        {
            m_xDimension->SetCustomName(QString());
        }
        dimensions.append(m_xDimension);
    }

    if (ui->groupBoxYDimension->isChecked())
    {
        if (not ui->groupBoxCustomYDimensionName->isChecked())
        {
            m_yDimension->SetCustomName(QString());
        }
        dimensions.append(m_yDimension);
    }

    if (ui->groupBoxWDimension->isChecked())
    {
        if (not ui->groupBoxCustomWDimensionName->isChecked())
        {
            m_wDimension->SetCustomName(QString());
        }
        dimensions.append(m_wDimension);
    }

    if (ui->groupBoxZDimension->isChecked())
    {
        if (not ui->groupBoxCustomZDimensionName->isChecked())
        {
            m_zDimension->SetCustomName(QString());
        }
        dimensions.append(m_zDimension);
    }

    return dimensions;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSetupMultisize::FullCircumference() const -> bool
{
    return ui->checkBoxFullCircumference->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::showEvent(QShowEvent *event)
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
void DialogSetupMultisize::ShowFullCircumference()
{
    auto ShowDimensionFullCircumference = [this](QDoubleSpinBox *doubleSpinBoxMinValue,
                                                 QDoubleSpinBox *doubleSpinBoxMaxValue, QComboBox *comboBoxStep,
                                                 QComboBox *comboBoxBase, const MeasurementDimension_p &dimension)
    {
        SCASSERT(doubleSpinBoxMinValue != nullptr)
        SCASSERT(doubleSpinBoxMaxValue != nullptr)
        SCASSERT(comboBoxStep != nullptr)
        SCASSERT(comboBoxBase != nullptr)

        InitDimension(doubleSpinBoxMinValue, doubleSpinBoxMaxValue, comboBoxStep, dimension);
        UpdateBase(comboBoxBase, dimension);

        const QSignalBlocker blocker(comboBoxBase);
        comboBoxBase->setCurrentIndex(-1);
    };

    ShowDimensionFullCircumference(ui->doubleSpinBoxYDimensionMinValue, ui->doubleSpinBoxYDimensionMaxValue,
                                   ui->comboBoxYDimensionStep, ui->comboBoxYDimensionBase, m_yDimension);
    ShowDimensionFullCircumference(ui->doubleSpinBoxWDimensionMinValue, ui->doubleSpinBoxWDimensionMaxValue,
                                   ui->comboBoxWDimensionStep, ui->comboBoxWDimensionBase, m_wDimension);
    ShowDimensionFullCircumference(ui->doubleSpinBoxZDimensionMinValue, ui->doubleSpinBoxZDimensionMaxValue,
                                   ui->comboBoxZDimensionStep, ui->comboBoxZDimensionBase, m_zDimension);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::XDimensionBodyMeasurementChanged()
{
    bool const checked = ui->checkBoxXDimensionBodyMeasurement->isChecked();
    m_xDimension->SetBodyMeasurement(checked);

    InitDimension(ui->doubleSpinBoxXDimensionMinValue, ui->doubleSpinBoxXDimensionMaxValue, ui->comboBoxXDimensionStep,
                  m_xDimension);

    UpdateBase(ui->comboBoxXDimensionBase, m_xDimension);

    {
        const QSignalBlocker blocker(ui->comboBoxXDimensionBase);
        ui->comboBoxXDimensionBase->setCurrentIndex(-1);
    }

    bool ok = false;
    const qreal base = ui->comboBoxXDimensionBase->currentData().toDouble(&ok);
    m_xDimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::YDimensionBodyMeasurementChanged()
{
    bool const checked = ui->checkBoxYDimensionBodyMeasurement->isChecked();
    m_yDimension->SetBodyMeasurement(checked);

    InitDimension(ui->doubleSpinBoxYDimensionMinValue, ui->doubleSpinBoxYDimensionMaxValue, ui->comboBoxYDimensionStep,
                  m_yDimension);

    UpdateBase(ui->comboBoxYDimensionBase, m_yDimension);

    {
        const QSignalBlocker blocker(ui->comboBoxYDimensionBase);
        ui->comboBoxYDimensionBase->setCurrentIndex(-1);
    }

    bool ok = false;
    const qreal base = ui->comboBoxYDimensionBase->currentData().toDouble(&ok);
    m_yDimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::WDimensionBodyMeasurementChanged()
{
    bool const checked = ui->checkBoxWDimensionBodyMeasurement->isChecked();
    m_wDimension->SetBodyMeasurement(checked);

    InitDimension(ui->doubleSpinBoxWDimensionMinValue, ui->doubleSpinBoxWDimensionMaxValue, ui->comboBoxWDimensionStep,
                  m_wDimension);

    UpdateBase(ui->comboBoxWDimensionBase, m_wDimension);

    {
        const QSignalBlocker blocker(ui->comboBoxWDimensionBase);
        ui->comboBoxWDimensionBase->setCurrentIndex(-1);
    }

    bool ok = false;
    const qreal base = ui->comboBoxWDimensionBase->currentData().toDouble(&ok);
    m_wDimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::ZDimensionBodyMeasurementChanged()
{
    bool const checked = ui->checkBoxZDimensionBodyMeasurement->isChecked();
    m_zDimension->SetBodyMeasurement(checked);

    InitDimension(ui->doubleSpinBoxZDimensionMinValue, ui->doubleSpinBoxZDimensionMaxValue, ui->comboBoxZDimensionStep,
                  m_zDimension);

    UpdateBase(ui->comboBoxZDimensionBase, m_zDimension);

    {
        const QSignalBlocker blocker(ui->comboBoxZDimensionBase);
        ui->comboBoxZDimensionBase->setCurrentIndex(-1);
    }

    bool ok = false;
    const qreal base = ui->comboBoxZDimensionBase->currentData().toDouble(&ok);
    m_zDimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::CheckDimension(QGroupBox *group, QGroupBox *nameGroup, QLineEdit *lineEdit,
                                          bool &dimensionValid, int &dimensions,
                                          const MeasurementDimension_p &dimension)
{
    SCASSERT(group != nullptr)
    SCASSERT(nameGroup != nullptr)
    SCASSERT(lineEdit != nullptr)

    if (group->isChecked())
    {
        dimensionValid = dimension->IsValid();
        ++dimensions;

        if (ui->labelError->text().isEmpty() && not dimensionValid)
        {
            ui->labelError->setText(tr("Please, provide correct data for dimension %1").arg(dimension->Axis()));
            return;
        }

        if (nameGroup->isChecked() && lineEdit->text().isEmpty() && ui->labelError->text().isEmpty())
        {
            ui->labelError->setText(tr("Please, provide custom name for dimension %1").arg(dimension->Axis()));
            return;
        }
    }
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

    CheckDimension(ui->groupBoxXDimension, ui->groupBoxCustomXDimensionName, ui->lineEditCustomXDimensionName,
                   xDimensionValid, dimensions, m_xDimension);
    CheckDimension(ui->groupBoxYDimension, ui->groupBoxCustomYDimensionName, ui->lineEditCustomYDimensionName,
                   yDimensionValid, dimensions, m_yDimension);
    CheckDimension(ui->groupBoxWDimension, ui->groupBoxCustomWDimensionName, ui->lineEditCustomWDimensionName,
                   wDimensionValid, dimensions, m_wDimension);
    CheckDimension(ui->groupBoxZDimension, ui->groupBoxCustomZDimensionName, ui->lineEditCustomZDimensionName,
                   zDimensionValid, dimensions, m_zDimension);

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
void DialogSetupMultisize::InitDimensionMinMax(QDoubleSpinBox *doubleSpinBoxMinValue,
                                               QDoubleSpinBox *doubleSpinBoxMaxValue,
                                               const MeasurementDimension_p &dimension)
{
    SCASSERT(doubleSpinBoxMinValue != nullptr)
    SCASSERT(doubleSpinBoxMaxValue != nullptr)

    dimension->SetMinValue(dimension->RangeMin());
    dimension->SetMaxValue(dimension->RangeMax());

    const bool fc = dimension->Type() != MeasurementDimension::X ? ui->checkBoxFullCircumference->isChecked() : false;
    const bool m = dimension->IsBodyMeasurement();
    const QString unitStr = m ? " " + UnitsToStr(m_xDimension->Units()) : QString();

    {
    	const QSignalBlocker blocker(doubleSpinBoxMinValue);
        doubleSpinBoxMinValue->setSuffix(unitStr);
        doubleSpinBoxMinValue->setDecimals(dimension->Decimals());
        doubleSpinBoxMinValue->setMinimum(m && fc ? dimension->RangeMin() * 2 : dimension->RangeMin());
        doubleSpinBoxMinValue->setMaximum(m && fc ? dimension->MaxValue() * 2 : dimension->MaxValue());
        doubleSpinBoxMinValue->setValue(m && fc ? dimension->MinValue() * 2 : dimension->MinValue());
    }

    const QSignalBlocker blocker(doubleSpinBoxMaxValue);
    doubleSpinBoxMaxValue->setSuffix(unitStr);
    doubleSpinBoxMaxValue->setDecimals(dimension->Decimals());
    doubleSpinBoxMaxValue->setMinimum(m && fc ? dimension->MinValue() * 2 : dimension->MinValue());
    doubleSpinBoxMaxValue->setMaximum(m && fc ? dimension->RangeMax() * 2 : dimension->RangeMax());
    doubleSpinBoxMaxValue->setValue(m && fc ? dimension->RangeMax() * 2 : dimension->RangeMax());
    doubleSpinBoxMaxValue->setValue(m && fc ? dimension->MaxValue() * 2 : dimension->MaxValue());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitDimensionStep(QComboBox *comboBoxStep, const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxStep != nullptr)

    const bool fc = dimension->Type() != MeasurementDimension::X ? ui->checkBoxFullCircumference->isChecked() : false;
    const bool m = dimension->IsBodyMeasurement();
    const QString unitStr = m ? " " + UnitsToStr(dimension->Units()) : QString();

    dimension->SetStep(-1);

    {
        const QSignalBlocker blocker(comboBoxStep);
        const QVector<qreal> steps = dimension->ValidSteps();
        comboBoxStep->clear();
        for (auto step : steps)
        {
            comboBoxStep->addItem(QStringLiteral("%1%2").arg(m && fc ? step * 2 : step).arg(unitStr), step);
        }

        comboBoxStep->setCurrentIndex(-1); // force a user to select
    }

    bool ok = false;
    const qreal step = comboBoxStep->currentData().toDouble(&ok);
    dimension->SetStep(ok ? step : -1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitDimension(QDoubleSpinBox *doubleSpinBoxMinValue, QDoubleSpinBox *doubleSpinBoxMaxValue,
                                         QComboBox *comboBoxStep, const MeasurementDimension_p &dimension)
{
    InitDimensionMinMax(doubleSpinBoxMinValue, doubleSpinBoxMaxValue, dimension);
    InitDimensionStep(comboBoxStep, dimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitXDimension()
{
    InitDimensionTitle(ui->groupBoxXDimension, m_xDimension);
    InitDimension(ui->doubleSpinBoxXDimensionMinValue, ui->doubleSpinBoxXDimensionMaxValue, ui->comboBoxXDimensionStep,
                  m_xDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitYDimension()
{
    InitDimensionTitle(ui->groupBoxYDimension, m_yDimension);
    InitDimension(ui->doubleSpinBoxYDimensionMinValue, ui->doubleSpinBoxYDimensionMaxValue, ui->comboBoxYDimensionStep,
                  m_yDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitWDimension()
{
    InitDimensionTitle(ui->groupBoxWDimension, m_wDimension);
    InitDimension(ui->doubleSpinBoxWDimensionMinValue, ui->doubleSpinBoxWDimensionMaxValue, ui->comboBoxWDimensionStep,
                  m_wDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::InitZDimension()
{
    InitDimensionTitle(ui->groupBoxZDimension, m_zDimension);
    InitDimension(ui->doubleSpinBoxZDimensionMinValue, ui->doubleSpinBoxZDimensionMaxValue, ui->comboBoxZDimensionStep,
                  m_zDimension);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::DimensionMinValueChanged(qreal value, QDoubleSpinBox *doubleSpinBoxMaxValue,
                                                    QComboBox *comboBoxStep, QComboBox *comboBoxBase,
                                                    const MeasurementDimension_p &dimension)
{
    SCASSERT(doubleSpinBoxMaxValue != nullptr)
    SCASSERT(comboBoxStep != nullptr)
    SCASSERT(comboBoxBase != nullptr)

    const bool fc = dimension->Type() != MeasurementDimension::X ? ui->checkBoxFullCircumference->isChecked() : false;
    const bool m = dimension->IsBodyMeasurement();

    dimension->SetMinValue(m && fc ? value / 2 : value);

    {
        const QSignalBlocker blocker(doubleSpinBoxMaxValue);
        doubleSpinBoxMaxValue->setMinimum(value);
    }

    dimension->SetMaxValue(m && fc ? doubleSpinBoxMaxValue->value() / 2 : doubleSpinBoxMaxValue->value());

    UpdateSteps(comboBoxStep, dimension);
    UpdateBase(comboBoxBase, dimension);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::DimensionMaxValueChanged(qreal value, QDoubleSpinBox *doubleSpinBoxMinValue,
                                                    QComboBox *comboBoxStep, QComboBox *comboBoxBase,
                                                    const MeasurementDimension_p &dimension)
{
    SCASSERT(doubleSpinBoxMinValue != nullptr)
    SCASSERT(comboBoxStep != nullptr)
    SCASSERT(comboBoxBase != nullptr)

    const bool fc = dimension->Type() != MeasurementDimension::X ? ui->checkBoxFullCircumference->isChecked() : false;
    const bool m = dimension->IsBodyMeasurement();

    dimension->SetMaxValue(m && fc ? value / 2 : value);

    {
        const QSignalBlocker blocker(doubleSpinBoxMinValue);
        doubleSpinBoxMinValue->setMaximum(value);
    }

    dimension->SetMinValue(m && fc ? doubleSpinBoxMinValue->value() / 2 : doubleSpinBoxMinValue->value());

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
    const qreal step = comboBoxStep->itemData(index).toDouble(&ok);
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
    const qreal base = comboBoxBase->itemData(index).toDouble(&ok);
    dimension->SetBaseValue(ok ? base : -1);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::UpdateSteps(QComboBox *comboBoxStep, const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxStep != nullptr)

    qreal oldStep = -1;
    if (comboBoxStep->currentIndex() != -1)
    {
        oldStep = comboBoxStep->currentData().toDouble();
    }

    {
        const QSignalBlocker blocker(comboBoxStep);

        const QVector<qreal> steps = dimension->ValidSteps();
        comboBoxStep->clear();

        const bool fc = dimension->Type() != MeasurementDimension::X ? ui->checkBoxFullCircumference->isChecked()
                                                                     : false;
        const bool m = dimension->IsBodyMeasurement();
        const QString unitStr = m ? " " + UnitsToStr(dimension->Units()) : QString();

        for (auto step : steps)
        {
            comboBoxStep->addItem(QStringLiteral("%1%2").arg(m && fc ? step * 2 : step).arg(unitStr), step);
        }

        comboBoxStep->setCurrentIndex(comboBoxStep->findData(oldStep));
    }

    bool ok = false;
    const qreal step = comboBoxStep->currentData().toDouble(&ok);
    dimension->SetStep(ok ? step : -1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSetupMultisize::UpdateBase(QComboBox *comboBoxBase, const MeasurementDimension_p &dimension)
{
    SCASSERT(comboBoxBase != nullptr)

    qreal oldBase = -1;
    if (comboBoxBase->currentIndex() != -1)
    {
        oldBase = comboBoxBase->currentData().toDouble();
    }

    {
        const QSignalBlocker blocker(comboBoxBase);

        const QVector<qreal> bases = dimension->ValidBases();
        comboBoxBase->clear();
        const bool fc = dimension->Type() != MeasurementDimension::X ? ui->checkBoxFullCircumference->isChecked()
                                                                     : false;
        const bool m = dimension->IsBodyMeasurement();
        const QString unitStr = m ? " " + UnitsToStr(dimension->Units()) : QString();

        for (auto base : bases)
        {
            comboBoxBase->addItem(QStringLiteral("%1%2").arg(m && fc ? base * 2 : base).arg(unitStr), base);
        }

        comboBoxBase->setCurrentIndex(comboBoxBase->findData(oldBase));
    }

    bool ok = false;
    const qreal base = comboBoxBase->currentData().toDouble(&ok);
    dimension->SetBaseValue(ok ? base : -1);
}
