/************************************************************************
 **
 **  @file   puzzlepreferenceslayoutpage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#ifndef PUZZLEPREFERENCESLAYOUTPAGE_H
#define PUZZLEPREFERENCESLAYOUTPAGE_H

#include <QWidget>
#include <memory>

#include "../vlayout/dialogs/vabstractlayoutdialog.h"
#include "../vmisc/def.h"

namespace Ui
{
class PuzzlePreferencesLayoutPage;
}

class PuzzlePreferencesLayoutPage : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit PuzzlePreferencesLayoutPage(QWidget *parent = nullptr);
    ~PuzzlePreferencesLayoutPage() override;

    auto Apply() -> QStringList;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void ConvertPaperSize();
    void LayoutSheetIgnoreMargins(int state);
    void LayoutTileIgnoreMargins(int state);
    void FindSheetTemplate();
    void FindTileTemplate();
    void CorrectMaxMargins();
    void SwapSheetOrientation(bool checked);
    void SwapTileOrientation(bool checked);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(PuzzlePreferencesLayoutPage) // NOLINT
    std::unique_ptr<Ui::PuzzlePreferencesLayoutPage> ui{};
    Unit m_oldLayoutUnit{Unit::Mm};
    bool m_settingsChanged{false};

    void InitLayoutUnits();

    auto Template(VAbstractLayoutDialog::PaperSizeTemplate t) const -> QSizeF;
    auto SheetTemplate() const -> QSizeF;
    auto TileTemplate() const -> QSizeF;

    void MinimumSheetPaperSize();
    void MinimumTilePaperSize();

    auto LayoutUnit() const -> Unit;
    void SetLayoutUnit(Unit unit);

    void SheetSize(const QSizeF &size);
    void TileSize(const QSizeF &size);

    void CorrectPaperDecimals();

    void SheetPaperSizeChanged();
    void TilePaperSizeChanged();

    auto GetSheetMargins() const -> QMarginsF;
    void SetSheetMargins(const QMarginsF &value);

    auto GetTileMargins() const -> QMarginsF;
    void SetTileMargins(const QMarginsF &value);

    void SetPieceGap(qreal gap);

    void ReadSettings();

    void FindTemplate(QComboBox *box, qreal width, qreal height);
};

#endif // PUZZLEPREFERENCESLAYOUTPAGE_H
