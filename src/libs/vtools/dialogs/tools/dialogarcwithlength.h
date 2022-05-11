/************************************************************************
 **
 **  @file   dialogarcwithlength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef DIALOGARCWITHLENGTH_H
#define DIALOGARCWITHLENGTH_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "defglobal.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogArcWithLength;
}

class DialogArcWithLength final : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogArcWithLength(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    ~DialogArcWithLength() override;

    auto GetCenter() const -> quint32;
    void SetCenter(const quint32 &value);

    auto GetRadius() const -> QString;
    void SetRadius(const QString &value);

    auto GetF1() const -> QString;
    void SetF1(const QString &value);

    auto GetLength() const -> QString;
    void SetLength(const QString &value);

    auto GetPenStyle() const -> QString;
    void SetPenStyle(const QString &value);

    auto GetColor() const -> QString;
    void SetColor(const QString &value);

    auto GetApproximationScale() const -> qreal;
    void SetApproximationScale(qreal value);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

    void SetAliasSuffix(const QString &alias);
    auto GetAliasSuffix() const -> QString;

    void ShowDialog(bool click) override;
public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    /**
     * @brief DeployFormulaTextEdit grow or shrink formula input
     */
    void DeployRadiusTextEdit();
    void DeployF1TextEdit();
    void DeployLengthTextEdit();

    void FXRadius();
    void FXF1();
    void FXLength();

protected:
    void  ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    void closeEvent(QCloseEvent *event) override;
    auto IsValid() const -> bool final;

private slots:
    void ValidateAlias();

private:
    Q_DISABLE_COPY_MOVE(DialogArcWithLength) // NOLINT
    Ui::DialogArcWithLength *ui;

    /** @brief flagRadius true if value of radius is correct */
    bool flagRadius{false};

    /** @brief flagF1 true if value of first angle is correct */
    bool flagF1{false};

    bool flagLength{false};

    bool flagAlias{true};

    /** @brief timerRadius timer of check formula of radius */
    QTimer *timerRadius;

    /** @brief timerF1 timer of check formula of first angle */
    QTimer *timerF1;

    QTimer *timerLength;

    /** @brief radius formula of radius */
    QString radius{};

    /** @brief f1 formula of first angle */
    QString f1{};

    QString length{};

    /** @brief formulaBaseHeight base height defined by dialogui */
    int formulaBaseHeightRadius{0};
    int formulaBaseHeightF1{0};
    int formulaBaseHeightLength{0};

    QString originAliasSuffix{};

    bool stageRadius{true};
    bool stageF1{false};

    bool m_firstRelease{false};

    void Radius();
    void Length();
    void EvalF();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogArcWithLength::IsValid() const -> bool
{
    return flagRadius && flagF1 && flagLength && flagAlias;
}

#endif // DIALOGARCWITHLENGTH_H
