/************************************************************************
 **
 **  @file   dialogmove.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 9, 2016
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

#ifndef DIALOGMOVING_H
#define DIALOGMOVING_H

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "dialogoperationtool.h"

namespace Ui
{
class DialogMove;
}

class DialogMove final : public DialogOperationTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogMove(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogMove() override;

    auto GetAngle() const -> QString;
    void SetAngle(const QString &value);

    auto GetLength() const -> QString;
    void SetLength(const QString &value);

    auto GetRotationAngle() const -> QString;
    void SetRotationAngle(const QString &value);

    auto GetRotationOrigPointId() const -> quint32;
    void SetRotationOrigPointId(const quint32 &value);

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

private slots:
    /** @brief DeployAngleTextEdit grow or shrink formula input */
    void DeployAngleTextEdit();
    void DeployRotationAngleTextEdit();
    void DeployLengthTextEdit();

    void FXAngle();
    void FXRotationAngle();
    void FXLength();

protected:
    void ShowVisualization() override;

    /** @brief SaveData Put dialog data in local variables */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    auto IsValid() const -> bool override;

    void OnSourceObjectsSet() override;

    auto Widgets() const -> OperationWidgets override;

private:
    Q_DISABLE_COPY_MOVE(DialogMove) // NOLINT
    Ui::DialogMove *ui;

    /** @brief timerAngle timer of check formula of angle */
    QTimer *timerAngle;
    QTimer *timerRotationAngle;
    QTimer *timerLength;

    /** @brief angle formula of angle */
    QString formulaAngle{};
    QString formulaRotationAngle{};
    QString formulaLength{};

    /** @brief formulaBaseHeightAngle base height defined by dialogui */
    int formulaBaseHeightAngle{0};
    int formulaBaseHeightRotationAngle{0};
    int formulaBaseHeightLength{0};

    bool stage2{false};

    bool optionalRotationOrigin{false};

    /** @brief flagAngle true if value of angle is correct */
    bool flagAngle{false};
    bool flagRotationAngle{false};
    bool flagLength{false};

    void EvalAngle();
    void EvalRotationAngle();
    void EvalLength();

    void InitIcons();
};

#endif // DIALOGMOVING_H
