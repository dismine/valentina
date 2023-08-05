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

#include <QDoubleSpinBox>
#include <QMainWindow>
#include <QMessageBox>
#include <QPointer>

#include "../vlayout/dialogs/vabstractlayoutdialog.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vmisc/def.h"
#include "../vmisc/vlockguard.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "carousel/vpcarrousel.h"
#include "layout/vplayout.h"
#include "scene/vpmaingraphicsview.h"
#include "vpcommandline.h"

namespace Ui
{
class VPMainWindow;
}

class QFileSystemWatcher;
template <typename T> class QSharedPointer;
class DialogPuzzlePreferences;
struct VPExportData;
class WatermarkWindow;

class VPMainWindow : public VAbstractMainWindow
{
    Q_OBJECT // NOLINT

public:
    explicit VPMainWindow(const VPCommandLinePtr &cmd, QWidget *parent = nullptr);
    ~VPMainWindow() override;

    auto CurrentFile() const -> QString;

    /**
     * @brief LoadFile Loads the layout file of given path in m_layout.
     * This function doesn't update the gui.
     * @param path path to layout
     * @return true if success
     */
    auto LoadFile(const QString &path) -> bool;

    void LayoutWasSaved(bool saved);
    void SetCurrentFile(const QString &fileName);

    /**
     * @brief SaveLayout Saves the current layout to the layout file of given path
     * @param path path to layout file
     * @return true if success
     */
    auto SaveLayout(const QString &path, QString &error) -> bool;

    /**
     * @brief ImportRawLayouts The function imports the raw layouts of given paths
     * @param rawLayouts paths of the layouts to import
     */
    void ImportRawLayouts(const QStringList &rawLayouts);

    /**
     * @brief InitZoom Initialises the zoom to fit best
     */
    void InitZoom();

    void UpdateWindowTitle();

public slots:
    /**
     * @brief on_actionNew_triggered When the menu action File > New
     * is triggered
     */
    void on_actionNew_triggered();

    void ShowToolTip(const QString &toolTip) override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto RecentFileList() const -> QStringList override;

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
    bool on_actionSave_triggered(); // NOLINT(modernize-use-trailing-return-type)

    /**
     * @brief on_actionSaveAs_triggered When the menu action File > Save As
     * is triggered.
     * The slot is automatically connected through name convention.
     */
    bool on_actionSaveAs_triggered(); // NOLINT(modernize-use-trailing-return-type)

    /**
     * @brief on_actionImportRawLayout_triggered When the menu action
     * File > Import Raw Layout is triggered.
     * The slot is automatically connected through name convention.
     */
    void on_actionImportRawLayout_triggered();

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
     * @brief on_LayoutUnitChanged When the unit is changed in
     * the layout property tab.
     * The slot is automatically connected through name convention.
     * @param index the index of the selected unit
     */
    void on_LayoutUnitChanged(int index);

    /**
     * @brief on_SheetSizeChanged When the width or the length has been changed in
     * the sheet property tab
     */
    void on_SheetSizeChanged();

    /**
     * @brief on_SheetOrientationChanged When one of the radio boxes for the sheet
     * orientation has been clicked
     */
    void on_SheetOrientationChanged(bool checked);

    /**
     * @brief on_SheetMarginChanged When one of the margin values has been changed
     * in the sheet property tab.
     */
    void on_SheetMarginChanged();

    /**
     * @brief on_checkBoxSheetShowGrid_toggled When the checkbox "show grid" is
     * clicked
     * @param checked show grid
     */
    void on_checkBoxSheetShowGrid_toggled(bool checked);

    /**
     * @brief on_doubleSpinBoxSheetGridColWidth_valueChanged When the "grid placement
     * column width" value is changed in the layout property tab.
     * The slot is automatically connected through name convention.
     * @param value the new value of the grid placement column width
     */
    void on_doubleSpinBoxSheetGridColWidth_valueChanged(double value);

    /**
     * @brief on_doubleSpinBoxSheetGridRowHeight_valueChanged When the "grid placement
     * row height" value is changed in the layout property tab.
     * The slot is automatically connected through name convention.
     * @param value the new value of the grid placement row height
     */
    void on_doubleSpinBoxSheetGridRowHeight_valueChanged(double value);

    /**
     * @brief on_TilesSizeChanged When the width or the length has been changed in
     * the tiles property tab
     */
    void on_TilesSizeChanged();

    /**
     * @brief on_TilesOrientationChanged When one of the radio boxes for the tiles
     * orientation has been clicked
     */
    void on_TilesOrientationChanged(bool checked);

    /**
     * @brief on_TilesMarginChanged When one of the margin values has been changed
     * in the tiles property tab.
     */
    void on_TilesMarginChanged();

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
     * @brief on_ScaleChanged When the scale of the graphic view is changed
     */
    void on_ScaleChanged(qreal scale);

    /**
     * @brief mouseMove save mouse position and show user.
     * @param scenePos position mouse.
     */
    void on_MouseMoved(const QPointF &scenePos);

    void on_actionPreferences_triggered();

    void ShowWindow() const;

    void ToolBarStyles();

    void on_actionAddSheet_triggered();

    void on_ApplyPieceTransformation();

    void on_ResetPieceTransformationSettings();

    void on_RelativeTranslationChanged(bool checked);

    void on_ConvertPaperSize();

    void on_ExportLayout();
    void on_ExportSheet();

    void on_actionPrintLayout_triggered();
    void on_actionPrintPreviewLayout_triggered();
    void on_actionPrintTiledLayout_triggered();
    void on_actionPrintPreviewTiledLayout_triggered();

    void on_printLayoutSheets(QPrinter *printer);
    void on_printLayoutTiledPages(QPrinter *printer);

    void on_actionPrintSheet_triggered();
    void on_actionPrintPreviewSheet_triggered();
    void on_actionPrintTiledSheet_triggered();
    void on_actionPrintPreviewTiledSheet_triggered();

    void on_printLayoutSheet(QPrinter *printer);
    void on_printLayoutSheetTiledPages(QPrinter *printer);

    void CreateWatermark();
    void EditCurrentWatermark();
    void LoadWatermark();
    void RemoveWatermark();

#if defined(Q_OS_MAC)
    void AboutToShowDockMenu();
#endif // defined(Q_OS_MAC)

    void AskDefaultSettings();

    void HorizontalScaleChanged(double value);
    void VerticalScaleChanged(double value);

    void LayoutWarningPiecesSuperposition_toggled(bool checked);
    void LayoutWarningPiecesOutOfBound_toggled(bool checked);

private:
    Q_DISABLE_COPY_MOVE(VPMainWindow) // NOLINT
    Ui::VPMainWindow *ui;

    VPCarrousel *m_carrousel{nullptr};
    VPMainGraphicsView *m_graphicsView{nullptr};

    VPCommandLinePtr m_cmd;

    QUndoStack *m_undoStack;

    VPLayoutPtr m_layout;

    /**
     * @brief spin box with the scale factor of the graphic view
     */
    QPointer<QDoubleSpinBox> m_doubleSpinBoxScale{nullptr};

    /**
     * @brief mouseCoordinate pointer to label who show mouse coordinate.
     */
    QLabel *m_mouseCoordinate{nullptr};

    QLabel *m_statusLabel{nullptr};

    QString curFile{};

    bool isInitialized{false};

    QSharedPointer<VLockGuard<char>> lock{nullptr};

    QFileSystemWatcher *m_layoutWatcher;

    QAction *undoAction{nullptr};
    QAction *redoAction{nullptr};

    Unit m_oldPieceTranslationUnit{Unit::Mm};
    Unit m_oldLayoutUnit{Unit::Mm};

    bool m_scaleConnected{true};

    QList<QPointer<WatermarkWindow>> m_watermarkEditors{};

    QFileSystemWatcher *m_watermarkWatcher{nullptr};

    struct VPLayoutPrinterPage
    {
        VPSheetPtr sheet{};
        bool tilesScheme{false};
        int tileRow{-1};
        int tileCol{-1};
    };

    /**
     * @brief InitMenuBar Inits the menu bar (File, Edit, Help ...)
     */
    void SetupMenu();

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

    void InitPaperSizeData(const QString &suffix);
    void InitMarginsData(const QString &suffix);

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
     * @brief InitScaleToolBar Initialises the scale tool bar
     */
    void InitScaleToolBar();

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

    void ReadSettings();
    void WriteSettings();

    auto MaybeSave() -> bool;

    void CreateWindowMenu(QMenu *menu);

    auto IsLayoutReadOnly() const -> bool;

    void ConnectToPreferences(const QSharedPointer<DialogPuzzlePreferences> &preferences);

    auto SelectedPieces() const -> QList<VPPiecePtr>;

    auto TranslateUnit() const -> Unit;
    auto LayoutUnit() const -> Unit;

    auto Template(VAbstractLayoutDialog::PaperSizeTemplate t) const -> QSizeF;
    auto SheetTemplate() const -> QSizeF;
    auto TileTemplate() const -> QSizeF;

    void SheetSize(const QSizeF &size);
    void TileSize(const QSizeF &size);

    void CorrectPaperDecimals();

    void SheetPaperSizeChanged();
    void TilePaperSizeChanged();

    void MinimumSheetPaperSize();
    void MinimumTilePaperSize();

    void FindTemplate(QComboBox *box, qreal width, qreal height);
    void FindSheetTemplate();
    void FindTileTemplate();

    void CorrectTileMaxMargins();
    void CorrectSheetMaxMargins();
    void CorrectMaxMargins();

    void RotatePiecesToGrainline();

    void ExportData(const VPExportData &data);
    static void ExportApparelLayout(const VPExportData &data, const QVector<VLayoutPiece> &details, const QString &name,
                                    const QSize &size);
    void ExportFlatLayout(const VPExportData &data);
    void ExportScene(const VPExportData &data);
    static void ExportUnifiedPdfFile(const VPExportData &data);
    static void GenerateUnifiedPdfFile(const VPExportData &data, const QString &name);
    void ExportPdfTiledFile(const VPExportData &data);
    auto GeneratePdfTiledFile(const VPSheetPtr &sheet, const VPExportData &data, QPainter *painter,
                              const QSharedPointer<QPrinter> &printer, bool &firstPage) -> bool;

    void UpdateScaleConnection() const;

    void OpenWatermark(const QString &path = QString());
    void CleanWaterkmarkEditors();

    auto DrawTilesScheme(QPrinter *printer, QPainter *painter, const VPSheetPtr &sheet, bool firstPage) -> bool;

    auto AskLayoutIsInvalid(const QList<VPSheetPtr> &sheets) -> bool;
    auto CheckPiecesOutOfBound(const VPPiecePtr &piece, bool &outOfBoundChecked) -> bool;
    auto CheckSuperpositionOfPieces(const VPPiecePtr &piece, bool &pieceSuperpositionChecked) -> bool;

    void PrintLayoutSheets(QPrinter *printer, const QList<VPSheetPtr> &sheets);
    static auto PrintLayoutSheetPage(QPrinter *printer, QPainter &painter, const VPSheetPtr &sheet) -> bool;
    void PrintLayoutTiledSheets(QPrinter *printer, const QList<VPSheetPtr> &sheets);
    auto PrepareLayoutTilePages(const QList<VPSheetPtr> &sheets) -> QVector<VPLayoutPrinterPage>;
    auto PrintLayoutTiledSheetPage(QPrinter *printer, QPainter &painter, const VPLayoutPrinterPage &page,
                                   bool firstPage) -> bool;

    void ZValueMove(int move);

    auto ImportRawLayout(const QString &rawLayout) -> bool;
    auto AddLayoutPieces(const QVector<VLayoutPiece> &pieces) -> bool;

    void TranslatePieces();
    void TranslatePieceRelatively(const VPPiecePtr &piece, const QRectF &rect, vsizetype selectedPiecesCount, qreal dx,
                                  qreal dy);
    void RotatePieces();

    void InitIcons();
};

#endif // VPMAINWINDOW_H
