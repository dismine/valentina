/************************************************************************
 **
 **  @file   vpmainwindow.h
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
#ifndef VPMAINWINDOW_H
#define VPMAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QPointer>

#include "../vmisc/def.h"
#include "vpcarrousel.h"
#include "vpmaingraphicsview.h"
#include "vplayout.h"
#include "vppiece.h"
#include "../vlayout/vlayoutpiece.h"
#include "vpcommandline.h"
#include "../vlayout/vlayoutdef.h"

namespace Ui
{
    class VPMainWindow;
}

class VPMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VPMainWindow(const VPCommandLinePtr &cmd, QWidget *parent = nullptr);
    virtual ~VPMainWindow();

    /**
     * @brief LoadFile Loads the layout file of given path in m_layout.
     * This function doesn't update the gui.
     * @param path path to layout
     * @return true if success
     */
    bool LoadFile(QString path);

    /**
     * @brief SaveFile Saves the current layout to the layout file of given path
     * @param path path to layout file
     * @return true if success
     */
    bool SaveFile(const QString &path);

    /**
     * @brief ImportRawLayouts The function imports the raw layouts of given paths
     * @param rawLayouts paths of the layouts to import
     */
    void ImportRawLayouts(const QStringList &rawLayouts);

    /**
     * @brief InitZoom Initialises the zoom to fit best
     */
    void InitZoom();

public slots:
    /**
     * @brief on_actionNew_triggered When the menu action File > New
     * is triggered
     */
    void on_actionNew_triggered();

protected:
    enum { MaxRecentFiles = 5 };

    virtual void closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(VPMainWindow)
    Ui::VPMainWindow *ui;

    VPCarrousel *m_carrousel{nullptr};
    VPMainGraphicsView *m_graphicsView{nullptr};

    VPCommandLinePtr m_cmd;

    VPLayout *m_layout{nullptr};
    QList<VPPiece *>m_selectedPieces{QList<VPPiece *>()};

    /**
     * @brief spin box with the scale factor of the graphic view
     */
    QPointer<QDoubleSpinBox> m_doubleSpinBoxScale{nullptr};

    /**
     * @brief mouseCoordinate pointer to label who show mouse coordinate.
     */
    QLabel* m_mouseCoordinate{nullptr};

    /**
     * @brief CreatePiece creates a piece from the given VLayoutPiece data
     * @param rawPiece the raw piece data
     */
    VPPiece* CreatePiece(const VLayoutPiece &rawPiece);

    /**
     * @brief InitMenuBar Inits the menu bar (File, Edit, Help ...)
     */
    void InitMenuBar();

    /**
     * @brief InitProperties Init the properties
     */
    void InitProperties();

    /**
     * @brief InitPropertyTabCurrentPiece Inits the current piece tab in the properties
     */
    void InitPropertyTabCurrentPiece();

    /**
     * @brief InitPropertyTabCurrentSheet Inits the current sheet tab in the properties;
     */
    void InitPropertyTabCurrentSheet();

    /**
     * @brief InitPropertyTabLayout Inits the layout tab in the properties
     */
    void InitPropertyTabLayout();

    /**
     * @brief InitPropertyTabTiles Inits the tiles tab in the properties
     */
    void InitPropertyTabTiles();

    /**
     * @brief InitCarrousel Inits the carrousel
     */
    void InitCarrousel();

    /**
     * @brief InitMainGraphics Initialises the puzzle main graphics
     */
    void InitMainGraphics();

    /**
     * @brief InitToolBar Initialises the tool bar
     */
    void InitZoomToolBar();

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
     * @brief SetPropertyTabSheetData Sets the values of UI elements
     * in the Sheet Tab to the values saved in focused sheet
     */
    void SetPropertyTabSheetData();

    /**
     * @brief SetPropertyTabTilesData Sets the values of UI elements
     * in the Tiles Tab to the values saved in m_layout
     */
    void SetPropertyTabTilesData();

    /**
    * @brief SetPropertyTabLayoutData Sets the values of UI elements
    * in the Layout Tab to the values saved in m_layout
    */
    void SetPropertyTabLayoutData();

    /**
     * @brief SetDoubleSpinBoxValue sets the given spinbox to the given value.
     * the signals are blocked before changing the value and unblocked after
     * @param spinBox pointer to spinbox
     * @param value spinbox value
     */
    void SetDoubleSpinBoxValue(QDoubleSpinBox *spinBox, qreal value);

    /**
     * @brief SetCheckBoxValue sets the given checkbox to the given value.
     * the signals are blocked before changing the value and unblocked after
     * @param checkbox pointer to checkbox
     * @param value checkbox value
     */
    void SetCheckBoxValue(QCheckBox *checkbox, bool value);

    void ReadSettings();
    void WriteSettings();

    bool MaybeSave();


private slots:
    /**
     * @brief on_actionOpen_triggered When the menu action File > Open is
     * triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionOpen_triggered();

    /**
     * @brief on_actionSave_triggered When the menu action File > Save is
     * triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionSave_triggered();

    /**
     * @brief on_actionSaveAs_triggered When the menu action File > Save As
     * is triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionSaveAs_triggered();

    /**
     * @brief on_actionImportRawLayout_triggered When the menu action
     * File > Import Raw Layout is triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionImportRawLayout_triggered();

    /**
     * @brief on_actionCloseLayout_triggered When the menu action
     * File > Close Layout is triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionCloseLayout_triggered();

    /**
     * @brief on_actionAboutQt_triggered When the menu action Help > About Qt
     * is triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionAboutQt_triggered();

    /**
     * @brief on_actionAboutPuzzle_triggered When the menu action Help > About Puzzle
     * is triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionAboutPuzzle_triggered();

    /**
     * @brief on_comboBoxLayoutUnit_currentIndexChanged When the unit is changed in
     * the layout property tab.
     * The slot is automatically connected through name convention.
     * @param index the index of the selected unit
     */
    void on_comboBoxLayoutUnit_currentIndexChanged(int index);

    /**
     * @brief on_comboBoxLayoutTemplate_currentIndexChanged When the template is
     * changed in the layout property tab.
     * The slot is automatically connected through name convention.
     * @param index the index of the selected templated
     */
    void on_comboBoxSheetTemplate_currentIndexChanged(int index);

    /**
     * @brief LayoutSizeChanged When the width or the length has been changed in
     * the layout property tab
     */
    void on_SheetSizeChanged();

    /**
     * @brief LayoutOrientationChanged When one of the radio boxes for the layout
     * orientation has been clicked
     */
    void on_SheetOrientationChanged();

    /**
     * @brief on_pushButtonLayoutRemoveUnusedLength_clicked When the button
     * "Remove unused length" in the layout property tab is clicked.
     * The slot is automatically connected through name convention.
     */
    void on_pushButtonSheetRemoveUnusedLength_clicked();

    /**
     * @brief on_LayoutMarginChanged When one of the margin values has been changed
     * in the layout property tab.
     */
    void on_SheetMarginChanged();

    /**
     * @brief LayoutFollowGrainlineChanged When one of the radio boxes for the
     * "Follow grainline" has been clicked in the layout property tab.
     */
    void on_SheetFollowGrainlineChanged();

    /**
     * @brief on_doubleSpinBoxLayoutPiecesGap_valueChanged When the "pieces gap"
     * value is changed in the layout property tab.
     * The slot is automatically connected through name convention.
     * @param value the new value of the pieces gap
     */
    void on_doubleSpinBoxSheetPiecesGap_valueChanged(double value);

    /**
     * @brief on_checkBoxLayoutWarningPiecesSuperposition_toggled When the
     * "Warning when pieces superposition" checkbox value in the layout
     * property tab is toggled.
     * The slot is automatically connected through name convention.
     * @param checked the new checked value
     */
    void on_checkBoxLayoutWarningPiecesSuperposition_toggled(bool checked);

    /**
     * @brief on_checkBoxLayoutWarningPiecesOutOfBound_toggled When the
     * "Warning when pieces out of bound" checkbox value in the layout property
     * tab is toggled.
     * The slot is automatically connected through name convention.
     * @param checked the new checked value
     */
    void on_checkBoxLayoutWarningPiecesOutOfBound_toggled(bool checked);

    /**
     * @brief on_checkBoxLayoutStickyEdges_toggled When the "Sticky edges"
     * checkbox value in the layout property tab is toggled.
     * The slot is automatically connected through name convention.
     * @param checked the new checked value
     */
    void on_checkBoxSheetStickyEdges_toggled(bool checked);

    /**
     * @brief on_pushButtonLayoutExport_clicked When the button
     * "Export layout" in the layout property is clicked.
     * The slot is automatically connected through name convention.
     */
    void on_pushButtonSheetExport_clicked();

    /**
     * @brief on_checkBoxCurrentPieceShowSeamline_toggled When the
     * "Show seamline" checkbox value in the current piece tab is toggled.
     * The slot is automatically connected through name convention.
     * @param checked the new checked value
     */
    void on_checkBoxCurrentPieceShowSeamline_toggled(bool checked);

    /**
     * @brief on_checkBoxCurrentPieceMirrorPiece_toggled When the
     * "Mirror piece" checkbox in the current piece tab is toggled.
     * The slot is automatically connected through name convention.
     * @param checked the new checked value
     */
    void on_checkBoxCurrentPieceMirrorPiece_toggled(bool checked);

    /**
     * @brief on_doubleSpinBoxCurrentPieceAngle_valueChanged When the
     * "Current Piece Angle" value in the current piece property is changed
     * The slot is automatically connected through name convention.
     * @param value the new angle value
     */
    void on_doubleSpinBoxCurrentPieceAngle_valueChanged(double value);

    /**
     * @brief on_CurrentPiecePositionChanged When the positionX or the positionY
     * is changed in the current piece tab
     */
    void on_CurrentPiecePositionEdited();

    /**
     * @brief CarrouselLocationChanged When the piece carrousel's location
     * has been changed
     * @param area The new area where the piece carrousel has been placed
     */
    void on_CarrouselLocationChanged(Qt::DockWidgetArea area);

    /**
     * @brief on_PieceSelectionChanged When the piece selection has changed
     */
    void on_PieceSelectionChanged();

    /**
     * @brief on_PiecePositionChanged When the current piece position has changed
     */
    void on_PiecePositionChanged();

    /**
     * @brief on_PieceRotationChanged When the current piece rotation has changed
     */
    void on_PieceRotationChanged();

    /**
     * @brief on_ScaleChanged When the scale of the graphic view is changed
     */
    void on_ScaleChanged(qreal scale);

    /**
     * @brief mouseMove save mouse position and show user.
     * @param scenePos position mouse.
     */
    void on_MouseMoved(const QPointF &scenePos);
};

#endif // VPMAINWINDOW_H
