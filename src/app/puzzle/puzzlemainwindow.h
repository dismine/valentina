/************************************************************************
 **
 **  @file   puzzlemainwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
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
#ifndef PUZZLEMAINWINDOW_H
#define PUZZLEMAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDoubleSpinBox>

#include "../vmisc/def.h"
#include "vpiececarrousel.h"
#include "vpuzzlelayout.h"
#include "vpuzzlepiece.h"

namespace Ui
{
    class PuzzleMainWindow;
}

class PuzzleMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PuzzleMainWindow(QWidget *parent = nullptr);
    virtual ~PuzzleMainWindow();

    bool LoadFile(const QString &path);

    void ImportRawLayouts(const QStringList &layouts);

public slots:
    void New();

private:
    Q_DISABLE_COPY(PuzzleMainWindow)
    Ui::PuzzleMainWindow *ui;
    VPieceCarrousel *pieceCarrousel;

    VPuzzleLayout *m_layout;

    VPuzzlePiece *m_selectedPiece;


    void InitMenuBar();
    void InitProperties();
    void InitPropertyTabCurrentPiece();
    void InitPropertyTabLayout();
    void InitPropertyTabTiles();
    void InitPropertyTabLayers();
    void InitPieceCarrousel();


    /**
     * @brief SetPropertiesData Sets the values of UI elements
     * in all the property tabs to the values saved in m_layout
     */
    void SetPropertiesData();

    /**
     * @brief SetPropertyTabCurrentPieceData Sets the values of UI elements
     * in the Current Piece Tab to the values saved in m_layout
     */
    void SetPropertyTabCurrentPieceData();

    /**
     * @brief SetPropertyTabLayoutData Sets the values of UI elements
     * in the Layout Tab to the values saved in m_layout
     */
    void SetPropertyTabLayoutData();

    /**
     * @brief SetPropertyTabTilesData Sets the values of UI elements
     * in the Tiles Tab to the values saved in m_layout
     */
    void SetPropertyTabTilesData();

    /**
     * @brief SetPropertyTabLayersData Sets the values of UI elements
     * in the Layers Tab to the values saved in m_layout
     */
    void SetPropertyTabLayersData();

    /**
     * @brief SetDoubleSpinBoxValue sets the given spinbox to the given value.
     * the signals are blocked before changing the value and unblocked after
     * @param spinbox
     * @param value
     */
    void SetDoubleSpinBoxValue(QDoubleSpinBox *spinBox, qreal value);

    /**
     * @brief SetCheckBoxValue sets the given checkbox to the given value.
     * the signals are blocked before changing the value and unblocked after
     * @param checkbox
     * @param value
     */
    void SetCheckBoxValue(QCheckBox *checkbox, bool value);

private slots:
    void Open();
    void Save();
    void SaveAs();
    void ImportRawLayout();
    void CloseLayout();

    void AboutQt();
    void AboutPuzzle();

    void LayoutUnitChanged(int index);
    void LayoutTemplateChanged(int index);
    void LayoutSizeChanged();
    void LayoutOrientationChanged();
    void LayoutRemoveUnusedLength();
    void LayoutMarginChanged();
    void LayoutFollowGrainlineChanged();
    void LayoutPiecesGapChanged(double value);
    void LayoutWarningPiecesSuperpositionChanged(bool checked);
    void LayoutWarningPiecesOutOfBoundChanged(bool checked);
    void LayoutStickyEdgesChanged(bool checked);
    void LayoutExport();

    void CurrentPieceShowSeamlineChanged(bool checked);
    void CurrentPieceMirrorPieceChanged(bool checked);
    void CurrentPieceAngleChanged(double value);
    void CurrentPiecePositionChanged();

    void PieceCarrouselLocationChanged(Qt::DockWidgetArea area);

};

#endif // PUZZLEMAINWINDOW_H
