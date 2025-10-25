/************************************************************************
 **
 **  @file   mainwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../vformat/vdimensions.h"
#include "../vmisc/vabstractshortcutmanager.h"
#include "../vmisc/vlockguard.h"
#include "core/vcmdexport.h"
#include "mainwindowsnogui.h"

#include <QDoubleSpinBox>
#include <QPointer>

namespace Ui
{
class MainWindow;
}

class VToolOptionsPropertyBrowser;
class VMeasurements;
class QFileSystemWatcher;
class QLabel;
class DialogIncrements;
class DialogTool;
class DialogHistory;
class DialogFinalMeasurements;
class VWidgetGroups;
class VWidgetDetails;
class QToolButton;
class QProgressBar;
class WatermarkWindow;
class Quuid;
class VBackgroundImageItem;
class VBackgroundImageControls;
class VWidgetBackgroundImages;
namespace VPE
{
class QtColorPicker;
}

/**
 * @brief The MainWindow class main windows.
 */
class MainWindow final : public MainWindowsNoGUI
{
    Q_OBJECT // NOLINT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    auto LoadPattern(QString fileName, const QString &customMeasureFile = QString()) -> bool;

public slots:
    void ProcessCMD();
    void ShowToolTip(const QString &toolTip) override;
    void UpdateVisibilityGroups() override;
    void UpdateDetailsList() override;
    void ZoomFitBestCurrent() override;
    void PlaceBackgroundImage(const QPointF &pos, const QString &fileName);
    void RemoveBackgroundImage(const QUuid &id);

signals:
    void RefreshHistory();
    void EnableItemMove(bool move);
    void ItemsSelection(SelectionType type);

    void EnableLabelSelection(bool enable);
    void EnablePointSelection(bool enable);
    void EnableLineSelection(bool enable);
    void EnableArcSelection(bool enable);
    void EnableElArcSelection(bool enable);
    void EnableSplineSelection(bool enable);
    void EnableSplinePathSelection(bool enable);
    void EnableNodeLabelSelection(bool enable);
    void EnableNodePointSelection(bool enable);
    void EnableDetailSelection(bool enable);
    void EnableBackgroundImageSelection(bool enable);

    void EnableLabelHover(bool enable);
    void EnablePointHover(bool enable);
    void EnableLineHover(bool enable);
    void EnableArcHover(bool enable);
    void EnableElArcHover(bool enable);
    void EnableSplineHover(bool enable);
    void EnableSplinePathHover(bool enable);
    void EnableNodeLabelHover(bool enable);
    void EnableNodePointHover(bool enable);
    void EnableDetailHover(bool enable);
    void EnableImageBackgroundHover(bool enable);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void customEvent(QEvent *event) override;
    void CleanLayout() override;
    void PrepareSceneList(PreviewQuatilty quality) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
#if defined(Q_OS_MAC)
    void ToolBarStyle(QToolBar *bar) const override;
#endif
private slots:
    void ScaleChanged(qreal scale);
    void MouseMove(const QPointF &scenePos);
    void Clear();
    void PatternChangesWereSaved(bool saved);
    void LastUsedTool();
    void FullParseFile();
    void SetEnabledGUI(bool enabled);
    void GlobalChangePP(const QString &patternPiece);
    void PreviousPatternPiece();
    void NextPatternPiece();
    void ToolBarStyles();
    void ToolboxIconSize();
    void ShowPaper(int index);
    void Preferences();
#if defined(Q_OS_MAC)
    void CreateMeasurements();
#endif
    void ExportDrawAs(bool checked);
    void ExportLayoutAs(bool checked);
    void ExportDetailsAs(bool checked);

    void ArrowTool(bool checked);
    void ToolEndLine(bool checked);
    void ToolLine(bool checked);
    void ToolAlongLine(bool checked);
    void ToolMidpoint(bool checked);
    void ToolShoulderPoint(bool checked);
    void ToolNormal(bool checked);
    void ToolBisector(bool checked);
    void ToolLineIntersect(bool checked);
    void ToolSpline(bool checked);
    void ToolCubicBezier(bool checked);
    void ToolCutSpline(bool checked);
    void ToolArc(bool checked);
    void ToolEllipticalArc(bool checked);
    void ToolSplinePath(bool checked);
    void ToolCubicBezierPath(bool checked);
    void ToolCutSplinePath(bool checked);
    void ToolPointOfContact(bool checked);
    void ToolDetail(bool checked);
    void ToolPiecePath(bool checked);
    void ToolPin(bool checked);
    void ToolPlaceLabel(bool checked);
    void ToolHeight(bool checked);
    void ToolTriangle(bool checked);
    void ToolPointOfIntersection(bool checked);
    void ToolUnionDetails(bool checked);
    void ToolDuplicateDetail(bool checked);
    void ToolGroup(bool checked);
    void ToolRotation(bool checked);
    void ToolFlippingByLine(bool checked);
    void ToolFlippingByAxis(bool checked);
    void ToolMove(bool checked);
    void ToolCutArc(bool checked);
    void ToolLineIntersectAxis(bool checked);
    void ToolCurveIntersectAxis(bool checked);
    void ToolArcIntersectAxis(bool checked);
    void ToolPointOfIntersectionArcs(bool checked);
    void ToolPointOfIntersectionCircles(bool checked);
    void ToolPointOfIntersectionCurves(bool checked);
    void ToolPointFromCircleAndTangent(bool checked);
    void ToolPointFromArcAndTangent(bool checked);
    void ToolArcWithLength(bool checked);
    void ToolTrueDarts(bool checked);
    void ToolInsertNode(bool checked);
    void ToolArcStart(bool checked);
    void ToolArcEnd(bool checked);

    void ActionDraw(bool checked);
    void ActionDetails(bool checked);
    void ActionLayout(bool checked);

    void on_actionNew_triggered();
    bool on_actionSaveAs_triggered(); // NOLINT(modernize-use-trailing-return-type)
    bool on_actionSave_triggered();   // NOLINT(modernize-use-trailing-return-type)
    void on_actionOpen_triggered();

    void on_actionOpenPuzzle_triggered();
    void on_actionCreateManualLayout_triggered();
    void on_actionUpdateManualLayout_triggered();

    void ActionAddBackgroundImage();

    void ActionExportFontCorrections();
    void ActionReloadLabels();

    /**
     * @brief Opens a file dialog to allow the user to select one or more
     * single-line font files for installation.
     * It supports TrueType (.ttf), OpenType (.otf), and SVG (.svg) font formats.
     */
    void ActionInstallSingleLineFont();

    void ClosedDialogUnionDetails(int result);
    void ClosedDialogDuplicateDetail(int result);
    void ClosedDialogGroup(int result);
    void ClosedDialogPiecePath(int result);
    void ClosedDialogPin(int result);
    void ClosedDialogPlaceLabel(int result);
    void ClosedDialogInsertNode(int result);

    void ConnectMeasurements();
    void UnloadMeasurements();
    void ShowMeasurements();
    void MeasurementsChanged(const QString &path);
    void SyncMeasurements();

    void CreateWatermark();
    void EditCurrentWatermark();
    void LoadWatermark();
    void RemoveWatermark();
#if defined(Q_OS_MAC)
    void OpenAt(QAction *where);
#endif // defined(Q_OS_MAC)

    void DimensionABaseChanged();
    void DimensionBBaseChanged();
    void DimensionCBaseChanged();

    void GradationChanged();

    void ShowProgress();
    void ClearPatternMessages();

    void AskDefaultSettings();

    void AddBackgroundImageItem(const QUuid &id);
    void DeleteBackgroundImageItem(const QUuid &id);
    void ShowBackgroundImageInExplorer(const QUuid &id);
    void SaveBackgroundImage(const QUuid &id);

    void ParseBackgroundImages();

    void ActionHistory_triggered(bool checked);
    void ActionExportRecipe_triggered();
    void ActionNewDraw_triggered();
    void ActionTable_triggered();
    void ActionFinalMeasurements_triggered();
    void ActionShowMainPath_triggered(bool checked);
    void ActionBoundaryTogetherWithNotches_triggered(bool checked);
    void ActionOpenTape_triggered();

    void UpdateShortcuts();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MainWindow) // NOLINT
    /** @brief ui keeps information about user interface */
    Ui::MainWindow *ui;

    /** @brief tool current tool */
    Tool m_currentTool{Tool::Arrow};

    /** @brief tool last used tool */
    Tool m_lastUsedTool{Tool::Arrow};

    /** @brief sceneDraw draw scene. */
    VMainGraphicsScene *m_sceneDraw{nullptr};

    /** @brief sceneDetails details scene. */
    VMainGraphicsScene *m_sceneDetails{nullptr};

    /** @brief isInitialized true after first show window. */
    bool m_isInitialized{false};

    /** @brief mChanges true if measurement file was changed. */
    bool m_mChanges{false};
    bool m_mChangesAsked{true};

    bool m_patternReadOnly{false};

    QPointer<DialogIncrements> m_dialogTable;
    QPointer<DialogTool> m_dialogTool{};
    QPointer<DialogHistory> m_dialogHistory;
    QPointer<DialogFinalMeasurements> m_dialogFMeasurements;

    /** @brief comboBoxDraws comboc who show name of pattern peaces. */
    QComboBox *m_comboBoxDraws{nullptr};
    QLabel *m_patternPieceLabel{nullptr};

    QComboBox *m_comboBoxPenStyle{nullptr};
    QLabel *m_globalPenStyleLabel{nullptr};

    VPE::QtColorPicker *m_pushButtonColor{nullptr};
    QLabel *m_globalColorLabel{nullptr};

    /** @brief currentDrawIndex save current selected pattern peace. */
    qint32 m_currentDrawIndex{0};

    /** @brief drawMode true if we current draw scene. */
    bool m_drawMode{true};

    QLabel *m_leftGoToStage{nullptr};
    QLabel *m_rightGoToStage{nullptr};
    QTimer *m_autoSaveTimer{nullptr};
    QTimer *m_measurementsSyncTimer;
    bool m_guiEnabled{true};
    QPointer<QComboBox> m_dimensionA{nullptr};
    QPointer<QComboBox> m_dimensionB{nullptr};
    QPointer<QComboBox> m_dimensionC{nullptr};
    QPointer<QLabel> m_dimensionALabel{nullptr};
    QPointer<QLabel> m_dimensionBLabel{nullptr};
    QPointer<QLabel> m_dimensionCLabel{nullptr};
    QPointer<QLabel> m_zoomScale{nullptr};
    QPointer<QDoubleSpinBox> m_doubleSpinBoxScale{nullptr};
    VToolOptionsPropertyBrowser *m_toolOptions{nullptr};
    VWidgetGroups *m_groupsWidget{nullptr};
    VWidgetDetails *m_detailsWidget{nullptr};
    VWidgetBackgroundImages *m_backgroundImagesWidget{nullptr};
    QSharedPointer<VLockGuard<char>> m_lock{nullptr};

    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    QList<QPointer<WatermarkWindow>> m_watermarkEditors{};

    qreal m_currentDimensionA{0};
    qreal m_currentDimensionB{0};
    qreal m_currentDimensionC{0};

    QSharedPointer<VMeasurements> m_m{};

    QTimer *m_gradation;

    QMap<QUuid, VBackgroundImageItem *> m_backgroundImages{};
    VBackgroundImageControls *m_backgroudcontrols{nullptr};

    bool m_groupsActive{false};
    bool m_toolOptionsActive{false};
    bool m_patternMessagesActive{false};
    bool m_backgroundImagesActive{false};

    QMultiHash<VShortcutAction, QAction *> m_shortcutActions{};

    void InitDimensionControls();
    void InitDimensionGradation(int index, const MeasurementDimension_p &dimension, const QPointer<QComboBox> &control);
    static void InitDimensionXGradation(const QVector<qreal> &bases, const DimesionLabels &labels,
                                        const QPointer<QComboBox> &control);
    void InitDimensionYWZGradation(const QVector<qreal> &bases, const DimesionLabels &labels,
                                   const QPointer<QComboBox> &control, bool bodyMeasurement);

    void ToolBarOption();
    void ToolBarStages();
    void ToolBarDraws();
    void ToolBarPenOptions();
    void ToolBarTools();
    void ToolBarDrawTools();
    void InitToolButtons();
    void InitActionShortcuts();
    void CancelTool();

    void SetupDrawToolsIcons();

    void SetEnableWidgets(bool enable);
    void SetEnableTool(bool enable);
    void SetLayoutModeActions();

    void SaveCurrentScene();
    void RestoreCurrentScene();
    void MinimumScrollBar();

    template <typename Dialog, typename Func>
    void SetToolButton(bool checked, Tool t, const QString &cursor, const QString &toolTip, Func closeDialogSlot);
    template <typename Dialog, typename Func, typename Func2>
    void SetToolButtonWithApply(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                                Func closeDialogSlot, Func2 applyDialogSlot);
    template <typename DrawTool> void ClosedDialog(int result);

    template <typename DrawTool> void ClosedDialogWithApply(int result, VMainGraphicsScene *scene);
    template <typename DrawTool> void ApplyDialog(VMainGraphicsScene *scene);
    template <typename DrawTool> void ClosedDrawDialogWithApply(int result);
    template <typename DrawTool> void ApplyDrawDialog();
    template <typename DrawTool> void ClosedDetailsDialogWithApply(int result);
    template <typename DrawTool> void ApplyDetailsDialog();

    auto SavePattern(const QString &fileName, QString &error) -> bool;
    void AutoSavePattern();
    void setCurrentFile(const QString &fileName);

    void ReadSettings();
    void WriteSettings();

    auto MaybeSave() -> bool;
    void CreateMenus();
    //---------------------------------------------------------------------------------------------------------------------
    void CreateActions();
    void InitAutoSave();
    auto PatternPieceName(QString &name) -> bool;
    auto CheckPathToMeasurements(const QString &patternPath, const QString &path) -> QString;
    void ChangePP(int index, bool zoomBestFit = true);
    /**
     * @brief EndVisualization try show dialog after and working with tool visualization.
     */
    void EndVisualization(bool click = false);
    void ZoomFirstShow();

    void AddDocks();
    void InitDocksContain();
    auto OpenNewValentina(const QString &fileName = QString()) const -> bool;
    void FileClosedCorrect();
    static auto GetUnlokedRestoreFileList() -> QStringList;

    void AddPP(const QString &PPName);
    auto StartPositionNewPP() const -> QPointF;

    void InitScenes();

    auto LoadMeasurements(const QString &patternPath, QString &path) -> bool;
    auto UpdateMeasurements(const QString &patternPath, QString &path, qreal baseA, qreal baseB, qreal baseC) -> bool;
    auto UpdateMeasurements(const QSharedPointer<VMeasurements> &mFile, qreal baseA, qreal baseB, qreal baseC) -> bool;

    void ReadMeasurements(qreal baseA, qreal baseB, qreal baseC);

    void ReopenFilesAfterCrash(QStringList &args);
    auto DoExport(const VCommandLinePtr &expParams) -> bool;
    auto DoFMExport(const VCommandLinePtr &expParams) -> bool;

    auto SetDimensionA(int value) -> bool;
    auto SetDimensionB(int value) -> bool;
    auto SetDimensionC(int value) -> bool;

    static auto GetPatternFileName() -> QString;
    auto GetMeasurementFileName() -> QString;

    void UpdateWindowTitle();

    void ToolSelectPoint();
    void ToolSelectPointByPress();
    void ToolSelectPointByRelease();
    void ToolSelectSpline();
    void ToolSelectSplinePath();
    void ToolSelectArc();
    void ToolSelectPointArc();
    void ToolSelectCurve();
    void ToolSelectAllDrawObjects();
    void ToolSelectOperationObjects();
    void ToolSelectGroupObjects();
    void ToolSelectDetail();

    void PrintPatternMessage(QEvent *event);

    void OpenWatermark(const QString &path = QString());
    void CleanWaterkmarkEditors();

    void StoreMultisizeMDimensions();
    void StoreIndividualMDimensions();

    void StoreMultisizeMDimension(const QList<MeasurementDimension_p> &dimensions, int index, qreal currentBase);
    void StoreIndividualMDimension(const QMap<QString, QSharedPointer<VMeasurement>> &measurements, IMD type);

    auto DimensionRestrictedValues(int index, const MeasurementDimension_p &dimension) -> QVector<qreal>;
    void SetDimensionBases();

    void StoreDimensions();

    void ExportDraw(const QString &fileName);

    void NewBackgroundImageItem(const VBackgroundPatternImage &image);
    auto InitBackgroundImageItem(const VBackgroundPatternImage &image) -> VBackgroundImageItem *;

    auto SavePatternAs(const QString &fileName) -> bool;

    auto FullParsePattern() -> bool;

    void RefreshPieceGeometry();
};

#endif // MAINWINDOW_H
