/************************************************************************
 **
 **  @file   vpdialogoptimizeonline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
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

#include "vpdialogoptimizeonline.h"
#include "../vnesting/vnestingoptionswidget.h"
#include "ui_vpdialogoptimizeonline.h"

#include <QLocale>

//---------------------------------------------------------------------------------------------------------------------
VPDialogOptimizeOnline::VPDialogOptimizeOnline(int sheetPieces, int unplacedPieces, int groups, qreal fabricWidthMm,
                                               qreal gapMm, bool grainlineVertical, bool grainlineFixed, QWidget *parent)
  : QDialog(parent),
    ui(std::make_unique<Ui::VPDialogOptimizeOnline>())
{
    ui->setupUi(this);

    ui->comboBoxPieces->addItem(tr("Pieces on this sheet (%n)", "", sheetPieces),
                                static_cast<int>(NestingPieces::Sheet));
    ui->comboBoxPieces->addItem(tr("Pieces on this sheet and unplaced pieces (%n)", "", sheetPieces + unplacedPieces),
                                static_cast<int>(NestingPieces::SheetAndUnplaced));
    ui->comboBoxPieces->addItem(tr("Unplaced pieces around the pieces on this sheet (%n)", "", unplacedPieces),
                                static_cast<int>(NestingPieces::UnplacedAroundSheet));
    ui->comboBoxPieces->setEnabled(unplacedPieces > 0);
    ui->checkBoxGroupByGroup->setChecked(groups > 1); // spec 21.6
    ui->checkBoxGroupByGroup->setEnabled(groups > 1);
    ui->checkBoxNoBackfill->setEnabled(groups > 1); // a sub-option of the sequencing
    connect(ui->checkBoxGroupByGroup, &QCheckBox::toggled, ui->checkBoxNoBackfill, &QCheckBox::setEnabled);

    ui->comboBoxRotations->addItem(tr("0° and 180°"), static_cast<int>(NestingRotations::ZeroAnd180));
    ui->comboBoxRotations->addItem(tr("0° only (napped fabric)"), static_cast<int>(NestingRotations::ZeroOnly));
    ui->comboBoxRotations->addItem(tr("Any angle"), static_cast<int>(NestingRotations::Any));
    ui->comboBoxRotations->setCurrentIndex(grainlineFixed ? 0 : 2);


    const QLocale locale;
    ui->labelSummary->setText(tr("Fabric width %1 mm, gap between pieces %2 mm, grainline %3.")
                                  .arg(locale.toString(fabricWidthMm, 'f', 0), locale.toString(gapMm, 'f', 1),
                                       grainlineVertical ? tr("along the sheet height") : tr("along the sheet width")));
}

//---------------------------------------------------------------------------------------------------------------------
VPDialogOptimizeOnline::~VPDialogOptimizeOnline() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VPDialogOptimizeOnline::Mode() const -> NestingPieces
{
    return static_cast<NestingPieces>(ui->comboBoxPieces->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPDialogOptimizeOnline::GroupByGroup() const -> bool
{
    return ui->checkBoxGroupByGroup->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPDialogOptimizeOnline::Rotations() const -> NestingRotations
{
    return static_cast<NestingRotations>(ui->comboBoxRotations->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPDialogOptimizeOnline::Options() const -> VNestingOptions
{
    VNestingOptions options = ui->optionsWidget->Options();
    options.rotations = Rotations();
    options.groupByGroup = GroupByGroup();
    options.noBackfill = GroupByGroup() && ui->checkBoxNoBackfill->isChecked();
    return options;
}

//---------------------------------------------------------------------------------------------------------------------
void VPDialogOptimizeOnline::SetRequestBuilder(const std::function<QJsonObject()> &builder)
{
    ui->optionsWidget->SetRequestBuilder(builder);
}
