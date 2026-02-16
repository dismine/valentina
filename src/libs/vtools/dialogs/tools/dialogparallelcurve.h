/************************************************************************
 **
 **  @file   dialogparallelcurve.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 11, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef DIALOGPARALLELCURVE_H
#define DIALOGPARALLELCURVE_H

#include "dialogtool.h"

namespace Ui
{
class DialogParallelCurve;
}

class DialogParallelCurve : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogParallelCurve(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogParallelCurve() override;

    auto GetOriginCurveId() const -> quint32;
    void SetOriginCurveId(const quint32 &value);

    auto GetFormulaWidth() const -> QString;
    void SetFormulaWidth(const QString &value);

    auto GetPenStyle() const -> QString;
    void SetPenStyle(const QString &value);

    auto GetColor() const -> QString;
    void SetColor(const QString &value);

    auto GetApproximationScale() const -> qreal;
    void SetApproximationScale(qreal value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetName(const QString &name);
    auto GetName() const -> QString;

    void SetAliasSuffix(const QString &alias);
    auto GetAliasSuffix() const -> QString;

    void ShowDialog(bool click) override;

    void CheckDependencyTreeComplete() override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    /**
     * @brief DeployWidthTextEdit grow or shrink formula input
     */
    void DeployWidthTextEdit();

    void FXWidth();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool override;

private slots:
    void ValidateName();
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogParallelCurve) // NOLINT
    Ui::DialogParallelCurve *ui;

    /** @brief m_flagWidth true if value of width is correct */
    bool m_flagWidth{false};
    bool m_flagName{true};
    bool m_flagAlias{true};

    QTimer *m_timerWidth;

    QString m_width{};

    int m_formulaBaseHeightWidth{0};

    QString m_originName{};
    QString m_originAliasSuffix{};

    bool m_firstRelease{false};

    void Width();

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogParallelCurve::IsValid() const -> bool
{
    return m_flagWidth && m_flagName && m_flagAlias;
}

#endif // DIALOGPARALLELCURVE_H
