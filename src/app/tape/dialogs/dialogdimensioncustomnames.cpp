/************************************************************************
 **
 **  @file   dialogdimensioncustomnames.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 2, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "dialogdimensioncustomnames.h"
#include "ui_dialogdimensioncustomnames.h"

//---------------------------------------------------------------------------------------------------------------------
DialogDimensionCustomNames::DialogDimensionCustomNames(
    const QMap<MeasurementDimension, MeasurementDimension_p> &dimensions, QWidget *parent)
  : QDialog(parent),
    ui(std::make_unique<Ui::DialogDimensionCustomNames>())
{
    ui->setupUi(this);

    InitTable(dimensions);
}

//---------------------------------------------------------------------------------------------------------------------
DialogDimensionCustomNames::~DialogDimensionCustomNames() = default;

//---------------------------------------------------------------------------------------------------------------------
auto DialogDimensionCustomNames::CustomNames() const -> QMap<MeasurementDimension, QString>
{
    QMap<MeasurementDimension, QString> names;

    const int rows = ui->tableWidget->rowCount();
    for (int row = 0; row < rows; ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 1);
        if (item != nullptr)
        {
            names.insert(static_cast<MeasurementDimension>(item->data(Qt::UserRole).toInt()), item->text());
        }
    }

    return names;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDimensionCustomNames::InitTable(const QMap<MeasurementDimension, MeasurementDimension_p> &dimensions)
{
    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->clearContents();

    ui->tableWidget->setRowCount(static_cast<int>(dimensions.size()));

    int row = 0;
    QMap<MeasurementDimension, MeasurementDimension_p>::const_iterator i = dimensions.constBegin();
    while (i != dimensions.constEnd())
    {
        {
            auto const name = QStringLiteral("%1 (%2)").arg(
                VAbstartMeasurementDimension::DimensionName(i.value()->Type()), i.value()->Axis());
            auto *itemValue = new QTableWidgetItem(name);
            itemValue->setTextAlignment(Qt::AlignHCenter | Qt::AlignCenter);

            // set the item non-editable (view only), and non-selectable
            Qt::ItemFlags flags = itemValue->flags();
            flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
            itemValue->setFlags(flags);

            ui->tableWidget->setItem(row, 0, itemValue);
        }

        {
            auto *itemLabel = new QTableWidgetItem(i.value()->CustomName());
            itemLabel->setData(Qt::UserRole, static_cast<int>(i.value()->Type()));
            itemLabel->setTextAlignment(Qt::AlignHCenter | Qt::AlignCenter);

            ui->tableWidget->setItem(row, 1, itemLabel);
        }

        ++row;
        ++i;
    }

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}
