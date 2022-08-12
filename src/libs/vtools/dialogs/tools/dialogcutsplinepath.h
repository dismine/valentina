/************************************************************************
 **
 **  @file   dialogcutsplinrpath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 12, 2013
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

#ifndef DIALOGCUTSPLINEPATH_H
#define DIALOGCUTSPLINEPATH_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogCutSplinePath;
}

/**
 * @brief The DialogCutSplinePath class dialog for ToolCutSplinePath.
 */
class DialogCutSplinePath final : public DialogTool
{
    Q_OBJECT // NOLINT
public:
    DialogCutSplinePath(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    virtual ~DialogCutSplinePath() override;

    QString      GetPointName() const;
    void         SetPointName(const QString &value);

    QString      GetFormula() const;
    void         SetFormula(const QString &value);

    quint32      getSplinePathId() const;
    void         setSplinePathId(quint32 value);

    void    SetNotes(const QString &notes);
    QString GetNotes() const;

    void    SetAliasSuffix1(const QString &alias);
    QString GetAliasSuffix1() const;

    void    SetAliasSuffix2(const QString &alias);
    QString GetAliasSuffix2() const;
public slots:
    virtual void ChosenObject(quint32 id, const SceneObject &type) override;
    /**
     * @brief DeployFormulaTextEdit grow or shrink formula input
     */
    void         DeployFormulaTextEdit();
    void         FXLength();
    void         EvalFormula();
protected:
    virtual void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    virtual void SaveData() override;
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool IsValid() const final;
private slots:
    void SplinePathChanged();
    void ValidateAlias();
private:
    Q_DISABLE_COPY_MOVE(DialogCutSplinePath) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogCutSplinePath *ui;

    /** @brief formula string with formula */
    QString formula;
    QString pointName;

    /** @brief formulaBaseHeight base height defined by dialogui */
    int formulaBaseHeight;

    QTimer *timerFormula;

    bool flagFormula;
    bool flagName;
    bool flagAlias1{true};
    bool flagAlias2{true};

    QString originAliasSuffix1{};
    QString originAliasSuffix2{};
};

//---------------------------------------------------------------------------------------------------------------------
inline bool DialogCutSplinePath::IsValid() const
{
    return flagFormula && flagAlias1 && flagAlias2 && flagName;
}

#endif // DIALOGCUTSPLINEPATH_H
