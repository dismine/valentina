/************************************************************************
 **
 **  @file   dialogseamallowance.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef DIALOGSEAMALLOWANCE_H
#define DIALOGSEAMALLOWANCE_H

#include "../dialogtool.h"
#include "../vgeometry/vplacelabelitem.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecepath.h"

namespace Ui
{
class DialogSeamAllowance;
class TabPaths;
class TabLabels;
class TabGrainline;
class TabPins;
class TabPassmarks;
class TabPlaceLabels;
} // namespace Ui

class VisPieceSpecialPoints;
class FancyTabBar;
class VUndoCommand;

class DialogSeamAllowance : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogSeamAllowance(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogSeamAllowance() override;

    void EnableApply(bool enable);

    auto GetPiece() const -> VPiece;
    void SetPiece(const VPiece &piece);

    auto GetFormulaSAWidth() const -> QString;

    auto UndoStack() -> QVector<QPointer<VUndoCommand>> &;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void ShowDialog(bool click) override;

protected:
    /** @brief SaveData Put dialog data in local variables */
    void SaveData() override;
    void CheckState() final;
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final;
    void SetPatternDoc(VAbstractPattern *doc) final;

private slots:
    void NameDetailChanged();
    void DetailUUIDChanged();
    void ShowMainPathContextMenu(const QPoint &pos);
    void ShowCustomSAContextMenu(const QPoint &pos);
    void ShowInternalPathsContextMenu(const QPoint &pos);
    void ShowPinsContextMenu(const QPoint &pos);
    void ShowPlaceLabelsContextMenu(const QPoint &pos);

    void ListChanged();
    void NodeChanged(int index);
    void PassmarkChanged(int index);
    void CSAStartPointChanged(int index);
    void CSAEndPointChanged(int index);
    void CSAIncludeTypeChanged(int index);
    void NodeAngleChanged(int index);
    void ReturnDefBefore();
    void ReturnDefAfter();
    void CustomSAChanged(int row);
    void PathDialogClosed(int result);
    void PlaceLabelDialogClosed(int result);
    void FancyTabChanged(int index);
    void TabChanged(int index);
    void PassmarkLineTypeChanged(int id);
    void PassmarkAngleTypeChanged(int id);
    void PassmarkShowSecondChanged(int state);
    void PassmarkClockwiseOrientationChanged(int state);

    void UpdateGrainlineValues();
    void UpdateDetailLabelValues();
    void UpdatePatternLabelValues();

    void EditGrainlineFormula();
    void EditDLFormula();
    void EditPLFormula();

    void DeployGrainlineRotation();
    void DeployGrainlineLength();

    void DeployDLWidth();
    void DeployDLHeight();
    void DeployDLAngle();

    void DeployPLWidth();
    void DeployPLHeight();
    void DeployPLAngle();

    void ResetGrainlineWarning();
    void ResetLabelsWarning();

    void EnabledGrainline();
    void EnabledDetailLabel();
    void EnabledPatternLabel();
    void EnabledManualPassmarkLength();
    void EnabledManualPassmarkWidth();
    void EnabledManualPassmarkAngle();

    void EvalWidth();
    void EvalWidthBefore();
    void EvalWidthAfter();
    void EvalPassmarkLength();
    void EvalPassmarkWidth();
    void EvalPassmarkAngle();

    void FXWidth();
    void FXWidthBefore();
    void FXWidthAfter();
    void FXPassmarkLength();
    void FXPassmarkWidth();
    void FXPassmarkAngle();

    void DeployWidthFormulaTextEdit();
    void DeployWidthBeforeFormulaTextEdit();
    void DeployWidthAfterFormulaTextEdit();
    void DeployPassmarkLength();
    void DeployPassmarkWidth();
    void DeployPassmarkAngle();

    void GrainlinePinPointChanged();
    void DetailPinPointChanged();
    void PatternPinPointChanged();

    void EditPieceLabel();
    void SetMoveControls();

    void PatternLabelDataChanged();
    void EditPatternLabel();
    void ManagePatternMaterials();

    void InsertGradationPlaceholder();

private:
    Q_DISABLE_COPY_MOVE(DialogSeamAllowance) // NOLINT

    Ui::DialogSeamAllowance *ui;
    Ui::TabPaths *uiTabPaths;
    Ui::TabLabels *uiTabLabels;
    Ui::TabGrainline *uiTabGrainline;
    Ui::TabPins *uiTabPins;
    Ui::TabPassmarks *uiTabPassmarks;
    Ui::TabPlaceLabels *uiTabPlaceLabels;

    QWidget *m_tabPaths{nullptr};
    QWidget *m_tabLabels{nullptr};
    QWidget *m_tabGrainline{nullptr};
    QWidget *m_tabPins{nullptr};
    QWidget *m_tabPassmarks{nullptr};
    QWidget *m_tabPlaceLabels{nullptr};

    FancyTabBar *m_ftb{nullptr};

    bool applyAllowed{false}; // By default disabled
    bool flagGPin{false};
    bool flagDPin{false};
    bool flagPPin{false};
    bool flagGFormulas{true};
    bool flagDLAngle{true};
    bool flagDLFormulas{true};
    bool flagPLAngle{true};
    bool flagPLFormulas{true};
    bool flagFormulaBefore{true};
    bool flagFormulaAfter{true};
    bool flagFormulaPassmarkLength{true};
    bool flagFormulaPassmarkWidth{true};
    bool flagFormulaPassmarkAngle{true};
    bool flagMainPathIsValid{true};
    bool flagName{true}; // We have default name of piece.
    bool flagUUID{true};
    bool flagFormula{true};
    bool m_bAddMode{true};
    bool m_patternLabelDataChanged{false};
    bool m_askSavePatternLabelData{false};
    bool m_patternTemplateDataChanged{false};
    bool m_patternMaterialsChanged{false};

    QPointer<DialogTool> m_dialog{};
    QPointer<VisPieceSpecialPoints> m_visSpecialPoints{};

    QVector<VLabelTemplateLine> m_patternTemplateLines{};

    QMap<int, QString> m_patternMaterials{};

    int m_iRotBaseHeight{0};
    int m_iLenBaseHeight{0};
    int m_DLWidthBaseHeight{0};
    int m_DLHeightBaseHeight{0};
    int m_DLAngleBaseHeight{0};
    int m_PLWidthBaseHeight{0};
    int m_PLHeightBaseHeight{0};
    int m_PLAngleBaseHeight{0};
    int m_formulaBaseWidth{0};
    int m_formulaBaseWidthBefore{0};
    int m_formulaBaseWidthAfter{0};
    int m_formulaBasePassmarkLength{0};
    int m_formulaBasePassmarkWidth{0};
    int m_formulaBasePassmarkAngle{0};

    QTimer *m_timerWidth{nullptr};
    QTimer *m_timerWidthBefore{nullptr};
    QTimer *m_timerWidthAfter{nullptr};
    QTimer *m_timerPassmarkLength{nullptr};
    QTimer *m_timerPassmarkWidth{nullptr};
    QTimer *m_timerPassmarkAngle{nullptr};
    qreal m_saWidth{0};

    QVector<VLabelTemplateLine> m_templateLines{};

    QVector<QPointer<VUndoCommand>> m_undoStack{};
    QHash<quint32, VPlaceLabelItem> m_newPlaceLabels{};
    QHash<quint32, VPiecePath> m_newPaths{};

    QString m_defLabelValue{};

    QMenu *m_placeholdersMenu;

    QMap<QString, QPair<QString, QString>> m_gradationPlaceholders{};

    auto CreatePiece() const -> VPiece;

    void NewMainPathItem(const VPieceNode &node);
    auto GetPathName(quint32 path, bool reverse = false) const -> QString;
    auto MainPathIsValid() const -> bool;
    void ValidObjects(bool value);
    auto MainPathIsClockwise() const -> bool;
    void UpdateCurrentCustomSARecord();
    void UpdateCurrentInternalPathRecord();
    void UpdateCurrentPlaceLabelRecords();

    auto GetItemById(quint32 id) -> QListWidgetItem *;

    auto GetLastId() const -> quint32;

    void SetCurrentSABefore(const QString &formula);
    void SetCurrentSAAfter(const QString &formula);

    void UpdateNodeSABefore(const QString &formula);
    void UpdateNodeSAAfter(const QString &formula);
    void UpdateNodePassmarkLength(const QString &formula);
    void UpdateNodePassmarkWidth(const QString &formula);
    void UpdateNodePassmarkAngle(const QString &formula);

    void InitFancyTabBar();
    void InitMainPathTab();
    void InitPieceTab();
    void InitSeamAllowanceTab();
    void InitNodesList();
    void InitPassmarksList();
    void InitCSAPoint(QComboBox *box);
    void InitPinPoint(QComboBox *box);
    void InitSAIncludeType();
    void InitInternalPathsTab();
    void InitPatternPieceDataTab();
    void InitLabelsTab();
    void InitGrainlineTab();
    void InitPinsTab();
    void InitPassmarksTab();
    void InitPlaceLabelsTab();
    void InitAllPinComboboxes();
    void InitLabelFontSize(QComboBox *box);

    void SetFormulaSAWidth(const QString &formula);
    void SetFormulaPassmarkLength(const QString &formula);
    void SetFormulaPassmarkWidth(const QString &formula);
    void SetFormulaPassmarkAngle(const QString &formula);

    void SetGrainlineAngle(QString angleFormula);
    void SetGrainlineLength(QString lengthFormula);

    void SetDLWidth(QString widthFormula);
    void SetDLHeight(QString heightFormula);
    void SetDLAngle(QString angleFormula);

    void SetPLWidth(QString widthFormula);
    void SetPLHeight(QString heightFormula);
    void SetPLAngle(QString angleFormula);

    auto CurrentRect() const -> QRectF;
    void ShowPieceSpecialPointsWithRect(const QListWidget *list, bool showRect);

    auto CurrentPath(quint32 id) const -> VPiecePath;
    auto CurrentPlaceLabel(quint32 id) const -> VPlaceLabelItem;

    auto GetDefaultPieceName() const -> QString;

    void EnableGrainlineFormulaControls(bool enable);
    void EnableDetailLabelFormulaControls(bool enable);
    void EnablePatternLabelFormulaControls(bool enable);

    void SavePatternLabelData();
    void SavePatternTemplateData();
    void SavePatternMaterialData();

    void InitGradationPlaceholdersMenu();
    void InitGradationPlaceholders();

    void InitPassmarkLengthFormula(const VPieceNode &node);
    void InitPassmarkWidthFormula(const VPieceNode &node);
    void InitPassmarkAngleFormula(const VPieceNode &node);
    void InitPassmarkShapeType(const VPieceNode &node);
    void InitPassmarkAngleType(const VPieceNode &node);

    void InitIcons();

    auto InitMainPathContextMenu(QMenu *menu, const VPieceNode &rowNode) const -> QHash<int, QAction *>;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogSeamAllowance::IsValid() const -> bool
{
    return flagName && flagUUID && flagMainPathIsValid && flagFormula && flagFormulaBefore && flagFormulaAfter &&
           (flagGFormulas || flagGPin) && flagDLAngle && (flagDLFormulas || flagDPin) && flagPLAngle &&
           (flagPLFormulas || flagPPin) && flagFormulaPassmarkLength;
}

#endif // DIALOGSEAMALLOWANCE_H
