/************************************************************************
 **
 **  @file   dialogspline.h
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

#ifndef DIALOGSPLINE_H
#define DIALOGSPLINE_H

#include <QMetaObject>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vspline.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogSpline;
}

/**
 * @brief The DialogSpline class dialog for ToolSpline. Help create spline and edit option.
 */
class DialogSpline : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogSpline(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogSpline() override;

    auto GetSpline() const -> VSpline;
    void SetSpline(const VSpline &spline);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void PointNameChanged() override;
    void ShowDialog(bool click) override;

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
    void DeployAngle1TextEdit();
    void DeployAngle2TextEdit();
    void DeployLength1TextEdit();
    void DeployLength2TextEdit();

    void FXAngle1();
    void FXAngle2();
    void FXLength1();
    void FXLength2();

    void EvalAngle1();
    void EvalAngle2();
    void EvalLength1();
    void EvalLength2();

    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogSpline) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogSpline *ui;

    /** @brief spl spline */
    VSpline spl{};

    qint32 newDuplicate{-1};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int formulaBaseHeightAngle1{0};
    int formulaBaseHeightAngle2{0};
    int formulaBaseHeightLength1{0};
    int formulaBaseHeightLength2{0};

    /** @brief timerAngle1 timer of check first angle formula */
    QTimer *timerAngle1;
    QTimer *timerAngle2;
    QTimer *timerLength1;
    QTimer *timerLength2;

    /** @brief flagAngle1 true if value of first angle is correct */
    bool flagAngle1{false};
    bool flagAngle2{false};
    bool flagLength1{false};
    bool flagLength2{false};
    bool flagError{false};
    bool flagAlias{true};

    QString originAliasSuffix{};

    /** @brief number number of handled objects */
    qint32 number{0};

    auto GetP1() const -> const QSharedPointer<VPointF>;
    auto GetP4() const -> const QSharedPointer<VPointF>;

    auto CurrentSpline() const -> VSpline;

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogSpline::IsValid() const -> bool
{
    return flagAngle1 && flagAngle2 && flagLength1 && flagLength2 && flagError && flagAlias;
}

#endif // DIALOGSPLINE_H
