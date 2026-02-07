/************************************************************************
 **
 **  @file   dialogsplinepath.h
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

#ifndef DIALOGSPLINEPATH_H
#define DIALOGSPLINEPATH_H

#include <QMetaObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vgeometry/vsplinepath.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogSplinePath;
}

/**
 * @brief The DialogSplinePath class dialog for ToolSplinePath. Help create spline path and edit option.
 */
class DialogSplinePath final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogSplinePath(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogSplinePath() override;

    auto GetPath() const -> VSplinePath;
    void SetPath(const VSplinePath &value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetDefPenStyle(const QString &value);
    void SetDefColor(const QString &value);

    void CheckDependencyTreeComplete() override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void ShowDialog(bool click) override;
    void PathUpdated(const VSplinePath &path);

protected:
    void ShowVisualization() override;
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void PointChanged(int row);
    void currentPointChanged(int index);
    void NewPointChanged();

    void AddPoint();
    void RemovePoint();

    void MoveTop();
    void MoveUp();
    void MoveDown();
    void MoveBottom();

    void DeployAngle1TextEdit();
    void DeployAngle2TextEdit();
    void DeployLength1TextEdit();
    void DeployLength2TextEdit();

    void Angle1Changed();
    void Angle2Changed();
    void Length1Changed();
    void Length2Changed();

    void FXAngle1();
    void FXAngle2();
    void FXLength1();
    void FXLength2();

    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogSplinePath) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogSplinePath *ui;

    /** @brief path spline path */
    VSplinePath path{};

    qint32 newDuplicate{-1};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int formulaBaseHeightAngle1{0};
    int formulaBaseHeightAngle2{0};
    int formulaBaseHeightLength1{0};
    int formulaBaseHeightLength2{0};

    /** @brief flagAngle1 true if value of first angle is correct */
    QVector<bool> flagAngle1{};
    QVector<bool> flagAngle2{};
    QVector<bool> flagLength1{};
    QVector<bool> flagLength2{};
    bool flagError{false};
    bool flagAlias{true};

    QString originAliasSuffix{};

    bool m_dependencyReady{true};

    void EvalAngle1();
    void EvalAngle2();
    void EvalLength1();
    void EvalLength2();

    void NewItem(const VSplinePoint &point);
    void DataPoint(const VSplinePoint &p);
    void SavePath();
    auto AllIds() const -> QSet<quint32>;
    auto IsPathValid() const -> bool;
    auto ExtractPath() const -> VSplinePath;
    void ShowPointIssue(const QString &pName);

    void InitIcons();

    void MoveControls();
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPath return spline path
 * @return path
 */
inline auto DialogSplinePath::GetPath() const -> VSplinePath
{
    return path;
}

#endif // DIALOGSPLINEPATH_H
