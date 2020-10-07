/************************************************************************
 **
 **  @file   dialogrestrictdimension.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 10, 2020
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
#include "dialogrestrictdimension.h"
#include "ui_dialogrestrictdimension.h"

#include <QTableWidgetItem>

#include "../vpatterndb/variables/vmeasurement.h"
#include "../vwidgets/vdecorationaligningdelegate.h"


namespace
{
//---------------------------------------------------------------------------------------------------------------------
QVector<int> FilterByMinimum(const QVector<int> &base, int restriction)
{
    if (restriction <= 0)
    {
        return base;
    }

    QVector<int> filtered;
    filtered.reserve(base.size());
    for(auto &b : base)
    {
        if (b >= restriction)
        {
            filtered.append(b);
        }
    }
    return filtered;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<int> FilterByMaximum(const QVector<int> &base, int restriction)
{
    if (restriction <= 0)
    {
        return base;
    }

    QVector<int> filtered;
    filtered.reserve(base.size());
    for(auto &b : base)
    {
        if (b <= restriction)
        {
            filtered.append(b);
        }
    }
    return filtered;
}
}

//---------------------------------------------------------------------------------------------------------------------
DialogRestrictDimension::DialogRestrictDimension(const QList<MeasurementDimension_p> &dimensions,
                                                 const QMap<QString, QPair<int, int>> &restrictions,
                                                 bool oneDimesionRestriction, bool fullCircumference,
                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRestrictDimension),
    m_oneDimesionRestriction(oneDimesionRestriction),
    m_fullCircumference(fullCircumference),
    m_dimensions(dimensions),
    m_restrictions(restrictions)
{
    ui->setupUi(this);

    ui->tableWidget->setItemDelegate(
        new VDecorationAligningDelegate(Qt::AlignHCenter | Qt::AlignCenter, ui->tableWidget));

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &DialogRestrictDimension::RowSelected);

    connect(ui->comboBoxDimensionA, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogRestrictDimension::DimensionAChanged);

    connect(ui->comboBoxMin, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogRestrictDimension::MinRestrictionChanged);
    connect(ui->comboBoxMax, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogRestrictDimension::MaxRestrictionChanged);

    InitDimensionsBaseValues();
    InitTable();
}

//---------------------------------------------------------------------------------------------------------------------
DialogRestrictDimension::~DialogRestrictDimension()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        auto RetranslateControls = [this](int index, QLabel *name, QComboBox *control)
        {
            SCASSERT(name != nullptr)
            SCASSERT(control != nullptr)

            if (m_dimensions.size() > index)
            {
                MeasurementDimension_p dimension = m_dimensions.at(index);

                name->setText(VAbstartMeasurementDimension::DimensionName(dimension->Type())+QChar(':'));
                name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension->Type(),
                                                                                dimension->IsCircumference(),
                                                                                m_fullCircumference));

                InitDimensionGradation(dimension, control);
            }
        };

        if (not m_oneDimesionRestriction)
        {
            RetranslateControls(0, ui->labelDimensionA, ui->comboBoxDimensionA);
        }
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::RowSelected()
{
    EnableRestrictionControls(false);

    QTableWidgetItem *item = ui->tableWidget->currentItem();

    if (item)
    {
        int base1 = 0;
        int base2 = 0;
        MeasurementDimension_p dimension;

        if (m_oneDimesionRestriction)
        {
            base1 = item->data(Qt::UserRole).toInt();

            if (m_dimensions.size() > 1)
            {
                dimension = m_dimensions.at(1);
            }
        }
        else
        {
            base1 = ui->comboBoxDimensionA->currentData().toInt();
            base2 = item->data(Qt::UserRole).toInt();

            if (m_dimensions.size() > 2)
            {
                dimension = m_dimensions.at(2);
            }
        }

        QPair<int, int> restriction = m_restrictions.value(VMeasurement::CorrectionHash(base1, base2),
                                                           QPair<int, int>(0, 0));

        if (dimension.isNull())
        {
            return;
        }

        const QVector<int> bases = dimension->ValidBases();

        ui->comboBoxMin->blockSignals(true);
        ui->comboBoxMin->clear();
        QVector<int> filtered = FilterByMaximum(bases, restriction.second);
        FillBases(filtered, dimension, ui->comboBoxMin);
        int index = ui->comboBoxMin->findData(restriction.first);
        ui->comboBoxMin->setCurrentIndex(index != -1 ? index : 0);
        ui->comboBoxMin->blockSignals(false);

        ui->comboBoxMax->blockSignals(true);
        ui->comboBoxMax->clear();
        filtered = FilterByMinimum(bases, restriction.first);
        FillBases(FilterByMinimum(bases, restriction.first), dimension, ui->comboBoxMax);
        index = ui->comboBoxMax->findData(restriction.second);
        ui->comboBoxMax->setCurrentIndex(index != -1 ? index : ui->comboBoxMax->count() - 1);
        ui->comboBoxMax->blockSignals(false);

        EnableRestrictionControls(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::DimensionAChanged()
{
    InitTable();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::MinRestrictionChanged()
{
    QTableWidgetItem *item = ui->tableWidget->currentItem();

    if (item)
    {
        int base1 = 0;
        int base2 = 0;

        if (m_oneDimesionRestriction)
        {
            base1 = item->data(Qt::UserRole).toInt();
        }
        else
        {
            base1 = ui->comboBoxDimensionA->currentData().toInt();
            base2 = item->data(Qt::UserRole).toInt();
        }

        const QString coordinates = VMeasurement::CorrectionHash(base1, base2);
        QPair<int, int> restriction = m_restrictions.value(coordinates, QPair<int, int>(0, 0));

        restriction.first = ui->comboBoxMin->currentData().toInt();
        m_restrictions.insert(coordinates, restriction);

        const int currentRow = ui->tableWidget->currentRow();
        RefreshTable();

        ui->tableWidget->blockSignals(true);
        ui->tableWidget->selectRow(currentRow);
        ui->tableWidget->blockSignals(false);

        RowSelected();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::MaxRestrictionChanged()
{
    QTableWidgetItem *item = ui->tableWidget->currentItem();

    if (item)
    {
        int base1 = 0;
        int base2 = 0;

        if (m_oneDimesionRestriction)
        {
            base1 = item->data(Qt::UserRole).toInt();
        }
        else
        {
            base1 = ui->comboBoxDimensionA->currentData().toInt();
            base2 = item->data(Qt::UserRole).toInt();
        }

        const QString coordinates = VMeasurement::CorrectionHash(base1, base2);
        QPair<int, int> restriction = m_restrictions.value(coordinates, QPair<int, int>(0, 0));

        restriction.second = ui->comboBoxMax->currentData().toInt();
        m_restrictions.insert(coordinates, restriction);

        const int currentRow = ui->tableWidget->currentRow();
        RefreshTable();

        ui->tableWidget->blockSignals(true);
        ui->tableWidget->selectRow(currentRow);
        ui->tableWidget->blockSignals(false);

        RowSelected();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::InitDimensionsBaseValues()
{
    auto DimensionsBaseValue = [this](int index, QLabel *name, QComboBox *control)
    {
        SCASSERT(name != nullptr)
        SCASSERT(control != nullptr)

        if (m_dimensions.size() > index)
        {
            MeasurementDimension_p dimension = m_dimensions.at(index);
            const QString unit = UnitsToStr(dimension->Units(), true);
            name->setText(VAbstartMeasurementDimension::DimensionName(dimension->Type())+QChar(':'));
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension->Type(),
                                                                            dimension->IsCircumference(),
                                                                            m_fullCircumference));

            InitDimensionGradation(dimension, control);
        }
    };

    if (not m_oneDimesionRestriction)
    {
        if (m_dimensions.size() > 0)
        {
            DimensionsBaseValue(0, ui->labelDimensionA, ui->comboBoxDimensionA);
        }
        else
        {
            ui->labelDimensionA->setVisible(false);
            ui->comboBoxDimensionA->setVisible(false);
        }
    }
    else
    {
        ui->labelDimensionA->setVisible(false);
        ui->comboBoxDimensionA->setVisible(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::InitDimensionGradation(const MeasurementDimension_p &dimension, QComboBox *control)
{
    SCASSERT(control != nullptr)

    int current = -1;
    if (control->currentIndex() != -1)
    {
        current = control->currentData().toInt();
    }

    control->blockSignals(true);
    control->clear();

    FillBases(dimension->ValidBases(), dimension, control);

    int i = control->findData(current);
    if (i != -1)
    {
        control->setCurrentIndex(i);
        control->blockSignals(false);
    }
    else
    {
        control->blockSignals(false);
        control->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::InitTable()
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clear();

    auto InitHeaders = [this](int index)
    {
        if (m_dimensions.size() > index)
        {
            MeasurementDimension_p dimensionA = m_dimensions.at(index-1);
            const QVector<int> basesA = dimensionA->ValidBases();
            ui->tableWidget->setRowCount(basesA.size());
            ui->tableWidget->setVerticalHeaderLabels(DimensionLabels(basesA, dimensionA));

            MeasurementDimension_p dimensionB = m_dimensions.at(index);
            const QVector<int> basesB = dimensionB->ValidBases();
            ui->tableWidget->setColumnCount(basesB.size());
            ui->tableWidget->setHorizontalHeaderLabels(DimensionLabels(basesB, dimensionB));
        }
    };

    InitHeaders(m_oneDimesionRestriction ? 1 : 2);
    ui->tableWidget->blockSignals(false);

    RefreshTable();
    ui->tableWidget->selectRow(0);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::RefreshTable()
{
    QVector<int> basesRow;
    QVector<int> basesColumn;

    if (m_oneDimesionRestriction)
    {
        if (m_dimensions.size() >= 2)
        {
            MeasurementDimension_p dimensionA = m_dimensions.at(0);
            basesRow = dimensionA->ValidBases();

            MeasurementDimension_p dimensionB = m_dimensions.at(1);
            basesColumn = dimensionB->ValidBases();
        }
        else
        {
            return;
        }
    }
    else
    {
        if (m_dimensions.size() >= 3)
        {
            MeasurementDimension_p dimensionA = m_dimensions.at(1);
            basesRow = dimensionA->ValidBases();

            MeasurementDimension_p dimensionB = m_dimensions.at(2);
            basesColumn = dimensionB->ValidBases();
        }
        else
        {
            return;
        }
    }

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();

    for(int row=0; row < basesRow.size(); ++row)
    {
        for(int column=0; column < basesColumn.size(); ++column)
        {
            AddCell(row, column, basesRow.at(row), basesColumn.at(column));
        }
    }

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::AddCell(int row, int column, int rowValue, int columnValue)
{
    auto *item = new QTableWidgetItem();
    item->setData(Qt::UserRole, rowValue);

    int base1 = 0;
    int base2 = 0;
    MeasurementDimension_p dimension;
    QVector<int> bases;

    if (m_oneDimesionRestriction)
    {
        base1 = rowValue;

        if (m_dimensions.size() >= 2)
        {
            dimension = m_dimensions.at(1);
            bases = dimension->ValidBases();
        }
    }
    else
    {
        base1 = ui->comboBoxDimensionA->currentData().toInt();
        base2 = rowValue;

        if (m_dimensions.size() >= 3)
        {
            dimension = m_dimensions.at(2);
            bases = dimension->ValidBases();
        }
    }

    QPair<int, int> restriction = m_restrictions.value(VMeasurement::CorrectionHash(base1, base2),
                                                       QPair<int, int>(0, 0));
    int min = INT32_MIN;
    int max = INT32_MAX;

    if (not dimension.isNull())
    {
        min = bases.indexOf(restriction.first) != -1 ? restriction.first : dimension->MinValue();
        max = bases.indexOf(restriction.second) != -1 ? restriction.second : dimension->MaxValue();

        if (max < min)
        {
            min = dimension->MinValue();
            max = dimension->MaxValue();
        }
    }

    const bool leftRestriction = columnValue >= min;
    const bool rightRestriction = columnValue <= max;

    if (leftRestriction && rightRestriction)
    {
        item->setIcon(QIcon("://icon/24x24/star.png"));
    }
    else
    {
        item->setIcon(QIcon("://icon/24x24/close.png"));
    }

    // set the item non-editable (view only), and non-selectable
    Qt::ItemFlags flags = item->flags();
    flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
    item->setFlags(flags);

    ui->tableWidget->setItem(row, column, item);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::EnableRestrictionControls(bool enable)
{
    if (not enable)
    {
        ui->comboBoxMin->blockSignals(true);
        ui->comboBoxMin->setCurrentIndex(-1);
        ui->comboBoxMin->blockSignals(false);

        ui->comboBoxMax->blockSignals(true);
        ui->comboBoxMax->setCurrentIndex(-1);
        ui->comboBoxMax->blockSignals(false);
    }

    ui->groupBoxRestriction->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::FillBases(const QVector<int> &bases, const MeasurementDimension_p &dimension,
                                        QComboBox *control)
{
    SCASSERT(control != nullptr)

    const QString units = UnitsToStr(dimension->Units(), true);

    if (dimension->Type() == MeasurementDimension::X)
    {
        for(auto base : bases)
        {
            control->addItem(QString("%1 %2").arg(base).arg(units), base);
        }
    }
    else if (dimension->Type() == MeasurementDimension::Y)
    {
        for(auto base : bases)
        {
            if (dimension->IsCircumference())
            {
                control->addItem(QString("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units), base);
            }
            else
            {
                control->addItem(QString::number(base), base);
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::W || dimension->Type() == MeasurementDimension::Z)
    {
        for(auto base : bases)
        {
            control->addItem(QString("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units), base);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList DialogRestrictDimension::DimensionLabels(const QVector<int> &bases, const MeasurementDimension_p &dimension)
{
    const bool showUnits = dimension->IsCircumference() || dimension->Type() == MeasurementDimension::X;
    const QString units = showUnits ? UnitsToStr(dimension->Units(), true) : QString();

    QStringList labels;

    if (dimension->Type() == MeasurementDimension::X)
    {
        for(auto base : bases)
        {
            labels.append(QString("%1 %2").arg(base).arg(units));
        }
    }
    else if (dimension->Type() == MeasurementDimension::Y)
    {
        for(auto base : bases)
        {
            if (dimension->IsCircumference())
            {
                labels.append(QString("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units));
            }
            else
            {
                labels.append(QString::number(base));
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::W || dimension->Type() == MeasurementDimension::Z)
    {
        for(auto base : bases)
        {
            labels.append(QString("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units));
        }
    }

    return labels;
}
