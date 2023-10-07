/************************************************************************
 **
 **  @file   dialogplacelabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 10, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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
#ifndef DIALOGPLACELABEL_H
#define DIALOGPLACELABEL_H

#include "../dialogtool.h"

namespace Ui
{
class DialogPlaceLabel;
}

class VPlaceLabelItem;

class DialogPlaceLabel : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogPlaceLabel(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogPlaceLabel() override;

    void EnbleShowMode(bool disable);

    auto GetCenterPoint() const -> quint32;
    void SetCenterPoint(quint32 id);

    auto GetLabelType() const -> PlaceLabelType;
    void SetLabelType(PlaceLabelType type);

    auto GetWidth() const -> QString;
    void SetWidth(const QString &value);

    auto GetHeight() const -> QString;
    void SetHeight(const QString &value);

    auto GetAngle() const -> QString;
    void SetAngle(const QString &value);

    auto GetPieceId() const -> quint32;
    void SetPieceId(quint32 id);

    auto GetFormulaVisible() const -> QString;
    void SetFormulaVisible(const QString &formula);

    void SetPiecesList(const QVector<quint32> &list) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

protected:
    void ShowVisualization() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final;

private slots:
    void DeployFormulaWidthEdit();
    void DeployFormulaHeightEdit();
    void DeployFormulaAngleEdit();
    void DeployVisibleFormulaTextEdit();

    void EvalWidth();
    void EvalHeight();
    void EvalAngle();
    void EvalVisible();

    void FXWidth();
    void FXHeight();
    void FXAngle();
    void FXVisible();

private:
    Q_DISABLE_COPY_MOVE(DialogPlaceLabel) // NOLINT
    Ui::DialogPlaceLabel *ui;
    bool m_showMode;

    /** @brief formulaBaseHeight base height defined by dialogui */
    int m_formulaBaseHeightWidth;
    int m_formulaBaseHeightHeight;
    int m_formulaBaseHeightAngle;
    int m_formulaBaseVisible;

    QTimer *timerAngle;
    QTimer *timerWidth;
    QTimer *timerHeight;
    QTimer *m_timerVisible;

    bool m_flagPoint;
    bool m_flagWidth;
    bool m_flagHeight;
    bool m_flagAngle;
    bool m_flagFormulaVisible;
    bool m_flagError;

    void InitPlaceLabelTab();
    void InitControlTab();

    void FillPlaceLabelTypes();

    void CheckPieces();
    void CheckPoint();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogPlaceLabel::IsValid() const -> bool
{
    return m_flagPoint && m_flagError && m_flagWidth && m_flagHeight && m_flagAngle && m_flagFormulaVisible;
}

#endif // DIALOGPLACELABEL_H
