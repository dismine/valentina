/************************************************************************
 **
 **  @file   dialogcurveintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#ifndef DIALOGCURVEINTERSECTAXIS_H
#define DIALOGCURVEINTERSECTAXIS_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogCurveIntersectAxis;
}

class DialogCurveIntersectAxis final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogCurveIntersectAxis(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogCurveIntersectAxis() override;

    auto GetPointName() const -> QString;
    void SetPointName(const QString &value);

    auto GetTypeLine() const -> QString;
    void SetTypeLine(const QString &value);

    auto GetAngle() const -> QString;
    void SetAngle(const QString &value);

    auto GetBasePointId() const -> quint32;
    void SetBasePointId(quint32 value);

    auto getCurveId() const -> quint32;
    void setCurveId(quint32 value);

    auto GetLineColor() const -> QString;
    void SetLineColor(const QString &value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetAliasSuffix1(const QString &alias);
    auto GetAliasSuffix1() const -> QString;

    void SetAliasSuffix2(const QString &alias);
    auto GetAliasSuffix2() const -> QString;

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void EvalAngle();
    void DeployAngleTextEdit();
    void FXAngle();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final;

private slots:
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogCurveIntersectAxis) // NOLINT
    Ui::DialogCurveIntersectAxis *ui;

    QString formulaAngle;
    int formulaBaseHeightAngle;

    QString pointName;

    bool m_firstRelease;

    QTimer *timerFormula;

    bool flagFormula;
    bool flagName;
    bool flagAlias1{true};
    bool flagAlias2{true};

    QString originAliasSuffix1{};
    QString originAliasSuffix2{};

    /** @brief number number of handled objects */
    qint32 number{0};

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogCurveIntersectAxis::IsValid() const -> bool
{
    return flagFormula && flagName && flagAlias1 && flagAlias2;
}

#endif // DIALOGCURVEINTERSECTAXIS_H
