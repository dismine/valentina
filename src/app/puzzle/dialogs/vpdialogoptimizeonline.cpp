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

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto LayoutDecimals(Unit unit) -> int
{
    switch (unit)
    {
        case Unit::Cm:
            return 2;
        case Unit::Mm:
            return 1;
        case Unit::Px:
            return 4;
        case Unit::Inch:
            return 5;
        default:
            return 2;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto FormatLayoutLength(qreal millimetres, Unit unit) -> QString
{
    const qreal value = UnitConvertor(millimetres, Unit::Mm, unit);
    return QLocale().toString(value, 'f', LayoutDecimals(unit)) + QStringLiteral(" ") + UnitsToStr(unit, true);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPDialogOptimizeOnline::VPDialogOptimizeOnline(int sheetPieces, int unplacedPieces, qreal fabricWidthMm, qreal gapMm,
                                               Unit unit, bool grainlineVertical, bool grainlineFixed,
                                               QWidget *parent)
  : QDialog(parent),
    ui(std::make_unique<Ui::VPDialogOptimizeOnline>())
{
    ui->setupUi(this);
    Q_UNUSED(grainlineVertical);
    Q_UNUSED(grainlineFixed);

    ui->comboBoxPieces->addItem(tr("Pieces on this sheet (%n)", "", sheetPieces),
                                static_cast<int>(NestingPieces::Sheet));
    ui->comboBoxPieces->addItem(tr("Pieces on this sheet and unplaced pieces (%n)", "", sheetPieces + unplacedPieces),
                                static_cast<int>(NestingPieces::SheetAndUnplaced));
    ui->comboBoxPieces->addItem(tr("Unplaced pieces around the pieces on this sheet (%n)", "", unplacedPieces),
                                static_cast<int>(NestingPieces::UnplacedAroundSheet));
    ui->comboBoxPieces->setEnabled(unplacedPieces > 0);

    ui->comboBoxRotations->addItem(tr("Follow piece grainline exactly"), static_cast<int>(NestingRotations::ZeroOnly));
    ui->comboBoxRotations->addItem(tr("Original Grainline and 180° rotation"),
                                   static_cast<int>(NestingRotations::ZeroAnd180));
    ui->comboBoxRotations->setCurrentIndex(0);


    ui->labelSummary->setText(tr("Fabric width %1, gap between pieces %2, grainline along the length.")
                                  .arg(FormatLayoutLength(fabricWidthMm, unit), FormatLayoutLength(gapMm, unit)));
}

//---------------------------------------------------------------------------------------------------------------------
VPDialogOptimizeOnline::~VPDialogOptimizeOnline() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VPDialogOptimizeOnline::Mode() const -> NestingPieces
{
    return static_cast<NestingPieces>(ui->comboBoxPieces->currentData().toInt());
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
    return options;
}
