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

#include <QMenu>
#include <QTableWidgetItem>

#include "../vpatterndb/variables/vmeasurement.h"
#include "../vwidgets/vdecorationaligningdelegate.h"


namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FilterByMinimum(const QVector<qreal> &base, qreal restriction) -> QVector<qreal>
{
    if (restriction <= 0)
    {
        return base;
    }

    QVector<qreal> filtered;
    filtered.reserve(base.size());
    for(const auto &b : base)
    {
        if (b >= restriction)
        {
            filtered.append(b);
        }
    }
    return filtered;
}

//---------------------------------------------------------------------------------------------------------------------
auto FilterByMaximum(const QVector<qreal> &base, qreal restriction) -> QVector<qreal>
{
    if (restriction <= 0)
    {
        return base;
    }

    QVector<qreal> filtered;
    filtered.reserve(base.size());
    for(const auto &b : base)
    {
        if (b <= restriction)
        {
            filtered.append(b);
        }
    }
    return filtered;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
DialogRestrictDimension::DialogRestrictDimension(const QList<MeasurementDimension_p> &dimensions,
                                                 const QMap<QString, VDimensionRestriction> &restrictions,
                                                 RestrictDimension restrictionType, bool fullCircumference,
                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRestrictDimension),
    m_restrictionType(restrictionType),
    m_fullCircumference(fullCircumference),
    m_dimensions(dimensions),
    m_restrictions(restrictions)
{
    ui->setupUi(this);

    ui->tableWidget->setItemDelegate(
        new VDecorationAligningDelegate(Qt::AlignHCenter | Qt::AlignCenter, ui->tableWidget));
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &DialogRestrictDimension::RowSelected);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this,
            &DialogRestrictDimension::CellContextMenu);

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

                name->setText(dimension->Name()+QChar(':'));
                name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_fullCircumference));

                InitDimensionGradation(dimension, control);
            }
        };

        if (m_restrictionType == RestrictDimension::Third)
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

    if (item != nullptr && (item->flags() & Qt::ItemIsEnabled) != 0U)
    {
        qreal base1 = 0;
        qreal base2 = 0;
        MeasurementDimension_p dimension;

        if (m_restrictionType == RestrictDimension::Second)
        {
            base1 = item->data(Qt::UserRole).toDouble();

            if (m_dimensions.size() > 1)
            {
                dimension = m_dimensions.at(1);
            }
        }
        else
        {
            base1 = ui->comboBoxDimensionA->currentData().toDouble();
            base2 = item->data(Qt::UserRole).toDouble();

            if (m_dimensions.size() > 2)
            {
                dimension = m_dimensions.at(2);
            }
        }

        VDimensionRestriction restriction = m_restrictions.value(VMeasurement::CorrectionHash(base1, base2));

        if (dimension.isNull())
        {
            return;
        }

        const QVector<qreal> bases = dimension->ValidBases();

        ui->comboBoxMin->blockSignals(true);
        ui->comboBoxMin->clear();
        QVector<qreal> filtered = FilterByMaximum(bases, restriction.GetMax());
        FillBases(filtered, dimension, ui->comboBoxMin);
        int index = ui->comboBoxMin->findData(restriction.GetMin());
        ui->comboBoxMin->setCurrentIndex(index != -1 ? index : 0);
        ui->comboBoxMin->blockSignals(false);

        ui->comboBoxMax->blockSignals(true);
        ui->comboBoxMax->clear();
        filtered = FilterByMinimum(bases, restriction.GetMin());
        FillBases(FilterByMinimum(bases, restriction.GetMin()), dimension, ui->comboBoxMax);
        index = ui->comboBoxMax->findData(restriction.GetMax());
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

    if (item != nullptr)
    {
        qreal base1 = 0;
        qreal base2 = 0;

        if (m_restrictionType == RestrictDimension::Second)
        {
            base1 = item->data(Qt::UserRole).toDouble();
        }
        else
        {
            base1 = ui->comboBoxDimensionA->currentData().toDouble();
            base2 = item->data(Qt::UserRole).toDouble();
        }

        const QString coordinates = VMeasurement::CorrectionHash(base1, base2);
        VDimensionRestriction restriction = m_restrictions.value(coordinates);

        restriction.SetMin(ui->comboBoxMin->currentData().toDouble());
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

    if (item != nullptr)
    {
        qreal base1 = 0;
        qreal base2 = 0;

        if (m_restrictionType == RestrictDimension::Second)
        {
            base1 = item->data(Qt::UserRole).toDouble();
        }
        else
        {
            base1 = ui->comboBoxDimensionA->currentData().toDouble();
            base2 = item->data(Qt::UserRole).toDouble();
        }

        const QString coordinates = VMeasurement::CorrectionHash(base1, base2);
        VDimensionRestriction restriction = m_restrictions.value(coordinates);

        restriction.SetMax(ui->comboBoxMax->currentData().toDouble());
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
void DialogRestrictDimension::CellContextMenu(QPoint pos)
{
    QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
    if (item != nullptr && (item->flags() & Qt::ItemIsEnabled) != 0U)
    {
        qreal columnValue = 0;
        QString coordinates;
        MeasurementDimension_p dimension;

        if (m_restrictionType == RestrictDimension::First)
        {
            if (not m_dimensions.empty())
            {
                columnValue = m_dimensions.at(0)->ValidBases().at(item->column());
                coordinates = QChar('0');
            }
            else
            {
                return;
            }
        }
        else if (m_restrictionType == RestrictDimension::Second)
        {
            if (m_dimensions.size() >= 2)
            {
                dimension = m_dimensions.at(1);
                columnValue = dimension->ValidBases().at(item->column());
                qreal base1 = m_dimensions.at(0)->ValidBases().at(item->row());
                coordinates = VMeasurement::CorrectionHash(base1);
            }
            else
            {
                return;
            }
        }
        else if (m_restrictionType == RestrictDimension::Third)
        {
            if (m_dimensions.size() >= 3)
            {
                dimension = m_dimensions.at(2);
                columnValue = dimension->ValidBases().at(item->column());
                qreal base1 = ui->comboBoxDimensionA->currentData().toDouble();
                qreal base2 = m_dimensions.at(1)->ValidBases().at(item->row());
                coordinates = VMeasurement::CorrectionHash(base1, base2);
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }

        VDimensionRestriction restriction = m_restrictions.value(coordinates);
        bool exclude = not restriction.GetExcludeValues().contains(columnValue);
        QScopedPointer<QMenu> menu(new QMenu());
        QAction *actionExclude = menu->addAction(exclude ? tr("Exclude") : tr("Include"));

        if (m_restrictionType == RestrictDimension::Second || m_restrictionType == RestrictDimension::Third)
        {
            if (dimension != nullptr)
            {
                qreal min = restriction.GetMin();
                if (qFuzzyIsNull(min))
                {
                    min = dimension->MinValue();
                }

                qreal max = restriction.GetMax();
                if (qFuzzyIsNull(max))
                {
                    max = dimension->MaxValue();
                }

                actionExclude->setEnabled(columnValue >= min && columnValue <= max);
            }
            else
            {
                return;
            }
        }

        QAction *selectedAction = menu->exec(ui->tableWidget->viewport()->mapToGlobal(pos));
        if (selectedAction == actionExclude)
        {
            QSet<qreal> list = restriction.GetExcludeValues();
            if (exclude)
            {
                list.insert(columnValue);
                item->setIcon(QIcon(QStringLiteral("://icon/24x24/close.png")));
            }
            else
            {
                list.remove(columnValue);
                item->setIcon(QIcon(QStringLiteral("://icon/24x24/star.png")));
            }
            restriction.SetExcludeValues(list);
            m_restrictions[coordinates] = restriction;
        }
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
            name->setText(dimension->Name()+QChar(':'));
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_fullCircumference));

            InitDimensionGradation(dimension, control);
        }
    };

    if (m_restrictionType == RestrictDimension::First)
    {
        ui->labelDimensionA->setVisible(false);
        ui->comboBoxDimensionA->setVisible(false);
        ui->groupBoxRestriction->setVisible(false);
    }
    else if (m_restrictionType == RestrictDimension::Second)
    {
        ui->labelDimensionA->setVisible(false);
        ui->comboBoxDimensionA->setVisible(false);
    }
    else if (m_restrictionType == RestrictDimension::Third)
    {
        if (not m_dimensions.empty())
        {
            DimensionsBaseValue(0, ui->labelDimensionA, ui->comboBoxDimensionA);
        }
        else
        {
            ui->labelDimensionA->setVisible(false);
            ui->comboBoxDimensionA->setVisible(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::InitDimensionGradation(const MeasurementDimension_p &dimension, QComboBox *control)
{
    SCASSERT(control != nullptr)

    qreal current = -1;
    if (control->currentIndex() != -1)
    {
        current = control->currentData().toDouble();
    }

    control->blockSignals(true);
    control->clear();

    FillBases(DimensionRestrictedValues(dimension), dimension, control);

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

    auto InitVerticalHeaderForDimension = [this](int index)
    {
        if (m_dimensions.size() > index)
        {
            MeasurementDimension_p dimension = m_dimensions.at(index);
            const QVector<qreal> bases = dimension->ValidBases();
            ui->tableWidget->setRowCount(bases.size());
            ui->tableWidget->setVerticalHeaderLabels(DimensionLabels(bases, dimension));
        }
    };

    auto InitHorizontalHeaderForDimension = [this](int index)
    {
        if (m_dimensions.size() > index)
        {
            MeasurementDimension_p dimension = m_dimensions.at(index);
            const QVector<qreal> bases = dimension->ValidBases();
            ui->tableWidget->setColumnCount(bases.size());
            ui->tableWidget->setHorizontalHeaderLabels(DimensionLabels(bases, dimension));
        }
    };

    if (m_restrictionType == RestrictDimension::First)
    {
        InitHorizontalHeaderForDimension(0);
        ui->tableWidget->setRowCount(1);
    }
    else if (m_restrictionType == RestrictDimension::Second)
    {
        InitVerticalHeaderForDimension(0);
        InitHorizontalHeaderForDimension(1);
    }
    else if (m_restrictionType == RestrictDimension::Third)
    {
        InitVerticalHeaderForDimension(1);
        InitHorizontalHeaderForDimension(2);
    }

    ui->tableWidget->blockSignals(false);

    RefreshTable();
    ui->tableWidget->selectRow(StartRow());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::RefreshTable()
{
    QVector<qreal> basesRow;
    QVector<qreal> basesColumn;

    if (m_restrictionType == RestrictDimension::First)
    {
        if (not m_dimensions.empty())
        {
            MeasurementDimension_p dimensionA = m_dimensions.at(0);
            basesColumn = dimensionA->ValidBases();
        }
        else
        {
            return;
        }
    }
    else if (m_restrictionType == RestrictDimension::Second)
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
    else if (m_restrictionType == RestrictDimension::Third)
    {
        if (m_dimensions.size() >= 3)
        {
            MeasurementDimension_p dimensionB = m_dimensions.at(1);
            basesRow = dimensionB->ValidBases();

            MeasurementDimension_p dimensionC = m_dimensions.at(2);
            basesColumn = dimensionC->ValidBases();
        }
        else
        {
            return;
        }
    }

    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();

    if (m_restrictionType == RestrictDimension::First)
    {
        for(int column=0; column < basesColumn.size(); ++column)
        {
            AddCell(0, column, 0, basesColumn.at(column));
        }
    }
    else
    {
        for(int row=0; row < basesRow.size(); ++row)
        {
            for(int column=0; column < basesColumn.size(); ++column)
            {
                AddCell(row, column, basesRow.at(row), basesColumn.at(column));
            }
        }
    }

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (m_restrictionType != RestrictDimension::First)
    {
        ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    ui->tableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRestrictDimension::AddCell(int row, int column, qreal rowValue, qreal columnValue)
{
    auto *item = new QTableWidgetItem();
    item->setData(Qt::UserRole, rowValue);

    if (m_restrictionType == RestrictDimension::First)
    {
        VDimensionRestriction restriction = m_restrictions.value(QChar('0'));
        if (restriction.GetExcludeValues().contains(columnValue))
        {
            item->setIcon(QIcon(QStringLiteral("://icon/24x24/close.png")));
        }
        else
        {
            item->setIcon(QIcon(QStringLiteral("://icon/24x24/star.png")));
        }
    }
    else
    {
        qreal base1 = 0;
        qreal base2 = 0;
        MeasurementDimension_p dimension;
        QVector<qreal> bases;
        QVector<qreal> validRows;

        if (m_restrictionType == RestrictDimension::Second)
        {
            base1 = rowValue;

            if (m_dimensions.size() >= 2)
            {
                validRows = DimensionRestrictedValues(m_dimensions.at(0));
                dimension = m_dimensions.at(1);
                bases = dimension->ValidBases();
            }
        }
        else if (m_restrictionType == RestrictDimension::Third)
        {
            base1 = ui->comboBoxDimensionA->currentData().toDouble();
            base2 = rowValue;

            if (m_dimensions.size() >= 3)
            {
                validRows = DimensionRestrictedValues(m_dimensions.at(1));
                dimension = m_dimensions.at(2);
                bases = dimension->ValidBases();
            }
        }

        VDimensionRestriction restriction = m_restrictions.value(VMeasurement::CorrectionHash(base1, base2));
        qreal min = INT32_MIN;
        qreal max = INT32_MAX;

        if (not dimension.isNull())
        {
            min = bases.indexOf(restriction.GetMin()) != -1 ? restriction.GetMin() : dimension->MinValue();
            max = bases.indexOf(restriction.GetMax()) != -1 ? restriction.GetMax() : dimension->MaxValue();

            if (max < min)
            {
                min = dimension->MinValue();
                max = dimension->MaxValue();
            }
        }

        if (validRows.contains(rowValue))
        {
            const bool leftRestriction = columnValue >= min;
            const bool rightRestriction = columnValue <= max;

            if (leftRestriction && rightRestriction)
            {
                if (restriction.GetExcludeValues().contains(columnValue))
                {
                    item->setIcon(QIcon(QStringLiteral("://icon/24x24/close.png")));
                }
                else
                {
                    item->setIcon(QIcon(QStringLiteral("://icon/24x24/star.png")));
                }
            }
            else
            {
                item->setIcon(QIcon(QStringLiteral("://icon/24x24/close.png")));
            }
        }
        else
        {
            item->setIcon(QIcon(QStringLiteral("://icon/24x24/close.png")));

            Qt::ItemFlags flags = item->flags();
            flags &= ~(Qt::ItemIsEnabled);
            item->setFlags(flags);
        }
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
void DialogRestrictDimension::FillBases(const QVector<qreal> &bases, const MeasurementDimension_p &dimension,
                                        QComboBox *control) const
{
    SCASSERT(control != nullptr)

    const DimesionLabels labels = dimension->Labels();
    const QString units = UnitsToStr(dimension->Units(), true);

    if (dimension->Type() == MeasurementDimension::X)
    {
        for(auto base : bases)
        {
            if (labels.contains(base) && not labels.value(base).isEmpty())
            {
                control->addItem(labels.value(base), base);
            }
            else
            {
                control->addItem(QStringLiteral("%1 %2").arg(base).arg(units), base);
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::Y)
    {
        for(auto base : bases)
        {
            if (labels.contains(base) && not labels.value(base).isEmpty())
            {
                control->addItem(labels.value(base), base);
            }
            else
            {
                if (dimension->IsCircumference())
                {
                    control->addItem(QStringLiteral("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units), base);
                }
                else
                {
                    control->addItem(QString::number(base), base);
                }
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::W || dimension->Type() == MeasurementDimension::Z)
    {
        for(auto base : bases)
        {
            if (labels.contains(base) && not labels.value(base).isEmpty())
            {
                control->addItem(labels.value(base), base);
            }
            else
            {
                control->addItem(QStringLiteral("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units), base);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRestrictDimension::DimensionLabels(const QVector<qreal> &bases,
                                              const MeasurementDimension_p &dimension) const -> QStringList
{
    const bool showUnits = dimension->IsCircumference() || dimension->Type() == MeasurementDimension::X;
    const QString units = showUnits ? UnitsToStr(dimension->Units(), true) : QString();
    const DimesionLabels dimensionLabels = dimension->Labels();

    QStringList labels;

    if (dimension->Type() == MeasurementDimension::X)
    {
        for(auto base : bases)
        {
            if (dimensionLabels.contains(base) && not dimensionLabels.value(base).isEmpty())
            {
                labels.append(dimensionLabels.value(base));
            }
            else
            {
                labels.append(QStringLiteral("%1 %2").arg(base).arg(units));
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::Y)
    {
        for(auto base : bases)
        {
            if (dimensionLabels.contains(base) && not dimensionLabels.value(base).isEmpty())
            {
                labels.append(dimensionLabels.value(base));
            }
            else
            {
                if (dimension->IsCircumference())
                {
                    labels.append(QStringLiteral("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units));
                }
                else
                {
                    labels.append(QString::number(base));
                }
            }
        }
    }
    else if (dimension->Type() == MeasurementDimension::W || dimension->Type() == MeasurementDimension::Z)
    {
        for(auto base : bases)
        {
            if (dimensionLabels.contains(base) && not dimensionLabels.value(base).isEmpty())
            {
                labels.append(dimensionLabels.value(base));
            }
            else
            {
                labels.append(QStringLiteral("%1 %2").arg(m_fullCircumference ? base*2 : base).arg(units));
            }
        }
    }

    return labels;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRestrictDimension::DimensionRestrictedValues(const MeasurementDimension_p &dimension) const -> QVector<qreal>
{
    VDimensionRestriction restriction;

    if (m_restrictionType == RestrictDimension::First || m_restrictionType == RestrictDimension::Second)
    {
        restriction = m_restrictions.value(QChar('0'));
    }
    else if (m_restrictionType == RestrictDimension::Third)
    {
        qreal base1 = ui->comboBoxDimensionA->currentData().toDouble();
        restriction = m_restrictions.value(VMeasurement::CorrectionHash(base1));
    }

    const QVector<qreal> bases = dimension->ValidBases();

    qreal min = bases.indexOf(restriction.GetMin()) != -1 ? restriction.GetMin() : dimension->MinValue();
    qreal max = bases.indexOf(restriction.GetMax()) != -1 ? restriction.GetMax() : dimension->MaxValue();

    if (min > max)
    {
        min = dimension->MinValue();
        max = dimension->MaxValue();
    }

    return VAbstartMeasurementDimension::ValidBases(min, max, dimension->Step(), restriction.GetExcludeValues());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRestrictDimension::StartRow() const -> int
{
    if (m_restrictionType == RestrictDimension::Second)
    {
        return 0;
    }

    QVector<qreal> basesRow;

    if (m_dimensions.size() >= 3)
    {
        MeasurementDimension_p dimensionB = m_dimensions.at(1);
        basesRow = dimensionB->ValidBases();

        QVector<qreal> validRows = DimensionRestrictedValues(dimensionB);

        for(int i=0; i < basesRow.size(); ++i)
        {
            if (validRows.contains(basesRow.at(i)))
            {
                return i;
            }
        }
    }

    return 0;
}
