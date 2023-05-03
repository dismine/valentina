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
    explicit DialogPiecePath(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    virtual ~DialogPiecePath();

    void EnbleShowMode(bool disable);
    void HideVisibilityTrigger();

    auto GetPiecePath() const -> VPiecePath;
    void       SetPiecePath(const VPiecePath &path);

    auto GetPieceId() const -> quint32;
    void    SetPieceId(quint32 id);

    auto GetFormulaSAWidth() const -> QString;
    void    SetFormulaSAWidth(const QString &formula);

    virtual void SetPiecesList(const QVector<quint32> &list) override;

public slots:
    virtual void ChosenObject(quint32 id, const SceneObject &type) override;
    virtual void ShowDialog(bool click) override;

protected:
    virtual void CheckState() final;
    virtual void ShowVisualization() override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual auto IsValid() const -> bool final;

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

    void EvalWidth();
    void EvalWidthBefore();
    void EvalWidthAfter();
    void EvalVisible();
    void EvalPassmarkLength();

    void FXWidth();
    void FXWidthBefore();
    void FXWidthAfter();
    void FXVisible();
    void FXPassmarkLength();

    void DeployWidthFormulaTextEdit();
    void DeployWidthBeforeFormulaTextEdit();
    void DeployWidthAfterFormulaTextEdit();
    void DeployVisibleFormulaTextEdit();
    void DeployPassmarkLength();

    void SetMoveControls();

private:
    Q_DISABLE_COPY_MOVE(DialogPiecePath) // NOLINT
    Ui::DialogPiecePath *ui;
    bool  m_showMode;
    qreal m_saWidth;

    QTimer *m_timerWidth;
    QTimer *m_timerWidthBefore;
    QTimer *m_timerWidthAfter;
    QTimer *m_timerVisible;
    QTimer *m_timerPassmarkLength;

    int m_formulaBaseWidth;
    int m_formulaBaseWidthBefore;
    int m_formulaBaseWidthAfter;
    int m_formulaBaseVisible;
    int m_formulaBasePassmarkLength{0};

    bool m_flagFormulaBefore;
    bool m_flagFormulaAfter;
    bool m_flagFormulaVisible;
    bool m_flagFormulaPassmarkLength{true};
    bool m_flagName;
    bool m_flagError;
    bool m_flagFormula;

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
    void          SetType(PiecePathType type);

    auto GetPenType() const -> Qt::PenStyle;
    void         SetPenType(const Qt::PenStyle &type);

    auto IsCutPath() const -> bool;
    void SetCutPath(bool value);

    auto GetItemById(quint32 id) -> QListWidgetItem *;

    auto GetLastId() const -> quint32;

    void SetCurrentSABefore(const QString &formula);
    void SetCurrentSAAfter(const QString &formula);

    void UpdateNodeSABefore(const QString &formula);
    void UpdateNodeSAAfter(const QString &formula);
    void UpdateNodePassmarkLength(const QString &formula);

    void EnabledManualPassmarkLength();

    auto GetFormulaSAWidthBefore() const -> QString;
    auto GetFormulaSAWidthAfter() const -> QString;

    auto GetFormulaVisible() const -> QString;
    void    SetFormulaVisible(const QString &formula);

    auto GetFormulaPassmarkLength() const -> QString;
    void    SetFormulaPassmarkLength(const QString &formula);

    auto IsShowNotch() const -> bool;

    void RefreshPathList(const VPiecePath &path);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPiecePath::IsValid() const -> bool
{
    return m_flagName && m_flagError && m_flagFormula && m_flagFormulaBefore && m_flagFormulaAfter
            && m_flagFormulaVisible && m_flagFormulaPassmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPiecePath::IsShowNotch() const -> bool
{
    return m_showMode && GetType() == PiecePathType::CustomSeamAllowance;
}

#endif // DIALOGPIECEPATH_H
