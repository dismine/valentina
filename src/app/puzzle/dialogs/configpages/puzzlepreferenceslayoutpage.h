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

#include "../vmisc/def.h"
#include "../vlayout/dialogs/vabstractlayoutdialog.h"

namespace Ui
{
    class PuzzlePreferencesLayoutPage;
}

class PuzzlePreferencesLayoutPage : public QWidget
{
    Q_OBJECT

public:
    explicit PuzzlePreferencesLayoutPage(QWidget *parent = nullptr);
    ~PuzzlePreferencesLayoutPage();

    QStringList Apply();

protected:
    virtual void changeEvent(QEvent* event) override;

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
    Q_DISABLE_COPY(PuzzlePreferencesLayoutPage)
    Ui::PuzzlePreferencesLayoutPage *ui;
    Unit m_oldLayoutUnit{Unit::Mm};

    void InitLayoutUnits();

    QSizeF Template(VAbstractLayoutDialog::PaperSizeTemplate t) const;
    QSizeF SheetTemplate() const;
    QSizeF TileTemplate() const;

    void MinimumSheetPaperSize();
    void MinimumTilePaperSize();

    Unit LayoutUnit() const;
    void SetLayoutUnit(Unit unit);

    void SheetSize(const QSizeF &size);
    void TileSize(const QSizeF &size);

    void CorrectPaperDecimals();

    void SheetPaperSizeChanged();
    void TilePaperSizeChanged();

    QMarginsF GetSheetMargins() const;
    void SetSheetMargins(const QMarginsF &value);

    QMarginsF GetTileMargins() const;
    void SetTileMargins(const QMarginsF &value);

    void SetPieceGap(qreal gap);

    void ReadSettings();

    void FindTemplate(QComboBox *box, qreal width, qreal height);
};

#endif // PUZZLEPREFERENCESLAYOUTPAGE_H
