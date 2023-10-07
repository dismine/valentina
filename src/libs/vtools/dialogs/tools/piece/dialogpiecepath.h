/************************************************************************
 **
 **  @file   dialogpiecepath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#ifndef DIALOGPIECEPATH_H
#define DIALOGPIECEPATH_H

#include "../dialogtool.h"

namespace Ui
{
class DialogPiecePath;
}

class DialogPiecePath : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogPiecePath(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogPiecePath() override;

    void EnbleShowMode(bool disable);
    void HideVisibilityTrigger();

    auto GetPiecePath() const -> VPiecePath;
    void SetPiecePath(const VPiecePath &path);

    auto GetPieceId() const -> quint32;
    void SetPieceId(quint32 id);

    auto GetFormulaSAWidth() const -> QString;
    void SetFormulaSAWidth(const QString &formula);

    void SetPiecesList(const QVector<quint32> &list) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void ShowDialog(bool click) override;

protected:
    void CheckState() final;
    void ShowVisualization() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final;

private slots:
    void ShowContextMenu(const QPoint &pos);
    void ListChanged();
    void NameChanged();
    void NodeChanged(int index);
    void PassmarkChanged(int index);
    void ReturnDefBefore();
    void ReturnDefAfter();
    void PassmarkLineTypeChanged(int id);
    void PassmarkAngleTypeChanged(int id);
    void PassmarkShowSecondChanged(int state);
    void PassmarkClockwiseOrientationChanged(int state);

    void EvalWidth();
    void EvalWidthBefore();
    void EvalWidthAfter();
    void EvalVisible();
    void EvalPassmarkLength();
    void EvalPassmarkWidth();
    void EvalPassmarkAngle();

    void FXWidth();
    void FXWidthBefore();
    void FXWidthAfter();
    void FXVisible();
    void FXPassmarkLength();
    void FXPassmarkWidth();
    void FXPassmarkAngle();

    void DeployWidthFormulaTextEdit();
    void DeployWidthBeforeFormulaTextEdit();
    void DeployWidthAfterFormulaTextEdit();
    void DeployVisibleFormulaTextEdit();
    void DeployPassmarkLength();
    void DeployPassmarkWidth();
    void DeployPassmarkAngle();

    void SetMoveControls();

private:
    Q_DISABLE_COPY_MOVE(DialogPiecePath) // NOLINT
    Ui::DialogPiecePath *ui;
    bool m_showMode{false};
    qreal m_saWidth{0};

    QTimer *m_timerWidth;
    QTimer *m_timerWidthBefore;
    QTimer *m_timerWidthAfter;
    QTimer *m_timerVisible;
    QTimer *m_timerPassmarkLength;
    QTimer *m_timerPassmarkWidth;
    QTimer *m_timerPassmarkAngle;

    int m_formulaBaseWidth{0};
    int m_formulaBaseWidthBefore{0};
    int m_formulaBaseWidthAfter{0};
    int m_formulaBaseVisible{0};
    int m_formulaBasePassmarkLength{0};
    int m_formulaBasePassmarkWidth{0};
    int m_formulaBasePassmarkAngle{0};

    bool m_flagFormulaBefore{true};
    bool m_flagFormulaAfter{true};
    bool m_flagFormulaVisible{true};
    bool m_flagFormulaPassmarkLength{true};
    bool m_flagFormulaPassmarkWidth{true};
    bool m_flagFormulaPassmarkAngle{true};
    bool m_flagName{true}; // We have default name of piece.
    bool m_flagError{false};
    bool m_flagFormula{false};

    void InitPathTab();
    void InitSeamAllowanceTab();
    void InitPassmarksTab();
    void InitControlTab();
    void InitPathTypes();
    void InitNodesList();
    void InitPassmarksList();
    void NodeAngleChanged(int index);

    auto CreatePath() const -> VPiecePath;

    auto PathIsValid() const -> bool;
    void ValidObjects(bool value);
    void NewItem(const VPieceNode &node);

    auto GetType() const -> PiecePathType;
    void SetType(PiecePathType type);

    auto GetPenType() const -> Qt::PenStyle;
    void SetPenType(const Qt::PenStyle &type);

    auto IsCutPath() const -> bool;
    void SetCutPath(bool value);

    auto GetItemById(quint32 id) -> QListWidgetItem *;

    auto GetLastId() const -> quint32;

    void SetCurrentSABefore(const QString &formula);
    void SetCurrentSAAfter(const QString &formula);

    void UpdateNodeSABefore(const QString &formula);
    void UpdateNodeSAAfter(const QString &formula);
    void UpdateNodePassmarkLength(const QString &formula);
    void UpdateNodePassmarkWidth(const QString &formula);
    void UpdateNodePassmarkAngle(const QString &formula);

    void EnabledManualPassmarkLength();
    void EnabledManualPassmarkWidth();
    void EnabledManualPassmarkAngle();

    auto GetFormulaSAWidthBefore() const -> QString;
    auto GetFormulaSAWidthAfter() const -> QString;

    auto GetFormulaVisible() const -> QString;
    void SetFormulaVisible(const QString &formula);

    auto GetFormulaPassmarkLength() const -> QString;
    void SetFormulaPassmarkLength(const QString &formula);

    auto GetFormulaPassmarkWidth() const -> QString;
    void SetFormulaPassmarkWidth(const QString &formula);

    auto GetFormulaPassmarkAngle() const -> QString;
    void SetFormulaPassmarkAngle(const QString &formula);

    auto IsShowNotch() const -> bool;

    void RefreshPathList(const VPiecePath &path);

    void InitPassmarkLengthFormula(const VPieceNode &node);
    void InitPassmarkWidthFormula(const VPieceNode &node);
    void InitPassmarkAngleFormula(const VPieceNode &node);
    void InitPassmarkShapeType(const VPieceNode &node);
    void InitPassmarkAngleType(const VPieceNode &node);

    void InitIcons();

    auto InitContextMenu(QMenu *menu, const VPieceNode &rowNode) -> QHash<int, QAction *>;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPiecePath::IsValid() const -> bool
{
    return m_flagName && m_flagError && m_flagFormula && m_flagFormulaBefore && m_flagFormulaAfter &&
           m_flagFormulaVisible && m_flagFormulaPassmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPiecePath::IsShowNotch() const -> bool
{
    return m_showMode && GetType() == PiecePathType::CustomSeamAllowance;
}

#endif // DIALOGPIECEPATH_H
