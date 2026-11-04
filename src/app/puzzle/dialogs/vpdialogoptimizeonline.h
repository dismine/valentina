/************************************************************************
 **
 **  @file   vpdialogoptimizeonline.h
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

#ifndef VPDIALOGOPTIMIZEONLINE_H
#define VPDIALOGOPTIMIZEONLINE_H

#include "../vlayout/vnestingframe.h"

#include <QDialog>
#include <memory>

namespace Ui
{
class VPDialogOptimizeOnline;
}

// Spec 21.6: which pieces a request takes, and whether the sheet's pieces stay where they are
enum class NestingPieces
{
    Sheet,
    SheetAndUnplaced,
    UnplacedAroundSheet
};

class VPDialogOptimizeOnline : public QDialog
{
    Q_OBJECT // NOLINT

public:
    VPDialogOptimizeOnline(int sheetPieces, int unplacedPieces, qreal fabricWidthMm, qreal gapMm, Unit unit,
                           bool grainlineVertical, bool grainlineFixed, QWidget *parent = nullptr);
    ~VPDialogOptimizeOnline() override;

    auto Mode() const -> NestingPieces;
    auto Rotations() const -> NestingRotations;
    auto Options() const -> VNestingOptions; // rotations and the seconds the user typed

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPDialogOptimizeOnline) // NOLINT
    std::unique_ptr<Ui::VPDialogOptimizeOnline> ui;
};

#endif // VPDIALOGOPTIMIZEONLINE_H
