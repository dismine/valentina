/************************************************************************
 **
 **  @file   dialogshoulderpoint.h
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

#ifndef DIALOGSHOULDERPOINT_H
#define DIALOGSHOULDERPOINT_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogShoulderPoint;
}

/**
 * @brief The DialogShoulderPoint class dialog for ToolShoulderPoint. Help create point and edit option.
 */
class DialogShoulderPoint final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogShoulderPoint(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogShoulderPoint() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetTypeLine() const -> QString;
    void SetTypeLine(const QString &value);

    auto GetFormula() const -> QString;
    void SetFormula(const QString &value);

    auto GetP1Line() const -> quint32;
    void SetP1Line(const quint32 &value);

    auto GetP2Line() const -> quint32;
    void SetP2Line(const quint32 &value);

    auto GetP3() const -> quint32;
    void SetP3(const quint32 &value);

    auto GetLineColor() const -> QString;
    void SetLineColor(const QString &value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    /**
     * @brief DeployFormulaTextEdit grow or shrink formula input
     */
    void DeployFormulaTextEdit();
    void PointNameChanged() override;
    void FXLength();
    void EvalFormula();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool override;

private:
    Q_DISABLE_COPY_MOVE(DialogShoulderPoint) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogShoulderPoint *ui;

    /** @brief formula formula */
    QString m_formula{};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int m_formulaBaseHeight{0};

    QTimer *m_timerFormula;

    QString m_pointName{};

    bool m_flagFormula{false};
    bool m_flagName{true};
    bool m_flagError{true};

    bool m_firstRelease{false};

    /** @brief number number of handled objects */
    qint32 m_number{0};

    void ChosenThirdPoint(quint32 id);

    void FinishCreating();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogShoulderPoint::IsValid() const -> bool
{
    return m_flagFormula && m_flagName && m_flagError;
}

#endif // DIALOGSHOULDERPOINT_H
