/************************************************************************
 **
 **  @file   dialogcutsplinrpath.cpp
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

#include "dialogcutsplinepath.h"

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/vcontainer.h"
#include "../../visualization/path/vistoolcutsplinepath.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../support/dialogeditwrongformula.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogcutsplinepath.h"
#include "../vgeometry/vsplinepath.h"
#include "../qmuparser/qmudef.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogCutSplinePath create dialog.
 * @param data container with data
 * @param parent parent widget
 */
DialogCutSplinePath::DialogCutSplinePath(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogCutSplinePath),
      formula(),
      pointName(),
      formulaBaseHeight(0),
      timerFormula(new QTimer(this)),
      flagFormula(false),
      flagName(true)
{
    ui->setupUi(this);

    timerFormula->setSingleShot(true);
    connect(timerFormula, &QTimer::timeout, this, &DialogCutSplinePath::EvalFormula);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
                VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    this->formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    InitOkCancelApply(ui);

    FillComboBoxSplinesPath(ui->comboBoxSplinePath);

    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogCutSplinePath::FXLength);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this, [this]()
    {
        CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
        CheckState();
    });
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerFormula->start(formulaTimerTimeout);
    });
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogCutSplinePath::DeployFormulaTextEdit);
    connect(ui->comboBoxSplinePath, &QComboBox::currentTextChanged, this, &DialogCutSplinePath::SplinePathChanged);

    connect(ui->lineEditAlias1, &QLineEdit::textEdited, this, &DialogCutSplinePath::ValidateAlias);
    connect(ui->lineEditAlias2, &QLineEdit::textEdited, this, &DialogCutSplinePath::ValidateAlias);

    vis = new VisToolCutSplinePath(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCutSplinePath::~DialogCutSplinePath()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCutSplinePath::GetPointName() const
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name
 */
void DialogCutSplinePath::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFormula set string of formula
 * @param value formula
 */
void DialogCutSplinePath::SetFormula(const QString &value)
{
    formula = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formula.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formula);

    VisToolCutSplinePath *path = qobject_cast<VisToolCutSplinePath *>(vis);
    SCASSERT(path != nullptr)
    path->setLength(formula);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setSplineId set id spline
 * @param value id
 */
void DialogCutSplinePath::setSplinePathId(quint32 value)
{
    setCurrentSplinePathId(ui->comboBoxSplinePath, value);

    VisToolCutSplinePath *path = qobject_cast<VisToolCutSplinePath *>(vis);
    SCASSERT(path != nullptr)
    path->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogCutSplinePath::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::SplinePath)
        {
            if (SetObject(id, ui->comboBoxSplinePath, QString()))
            {
                if (vis != nullptr)
                {
                    vis->VisualMode(id);
                }
                prepare = true;
                this->setModal(true);
                this->show();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formula = ui->plainTextEditFormula->toPlainText();

    VisToolCutSplinePath *path = qobject_cast<VisToolCutSplinePath *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(getSplinePathId());
    path->setLength(formula);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::SplinePathChanged()
{
    CurrentCurveLength(getSplinePathId(), const_cast<VContainer *> (data));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());

    VSplinePath path1;
    path1.SetAliasSuffix(GetAliasSuffix1());

    VSplinePath path2;
    path2.SetAliasSuffix(GetAliasSuffix2());

    if (not GetAliasSuffix1().isEmpty() &&
        (not rx.match(path1.GetAlias()).hasMatch() ||
         (originAliasSuffix1 != GetAliasSuffix1() && not data->IsUnique(path1.GetAlias())) ||
         path1.GetAlias() == path2.GetAlias()))
    {
        flagAlias1 = false;
        ChangeColor(ui->labelAlias1, errorColor);
    }
    else
    {
        flagAlias1 = true;
        ChangeColor(ui->labelAlias1, OkColor(this));
    }

    if (not GetAliasSuffix2().isEmpty() &&
        (not rx.match(path2.GetAlias()).hasMatch() ||
         (originAliasSuffix2 != GetAliasSuffix2() && not data->IsUnique(path2.GetAlias())) ||
         path1.GetAlias() == path2.GetAlias()))
    {
        flagAlias2 = false;
        ChangeColor(ui->labelAlias2, errorColor);
    }
    else
    {
        flagAlias2 = true;
        ChangeColor(ui->labelAlias2, OkColor(this));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::DeployFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::FXLength()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit length"));
    dialog->SetFormula(GetFormula());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormula(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::EvalFormula()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormula->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormula;
    formulaData.labelResult = ui->labelResultCalculation;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkZero = false;

    Eval(formulaData, flagFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::ShowVisualization()
{
    AddVisualization<VisToolCutSplinePath>();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormula return string of formula
 * @return formula
 */
QString DialogCutSplinePath::GetFormula() const
{
    return VTranslateVars::TryFormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getSplineId return id base point of line
 * @return id
 */
quint32 DialogCutSplinePath::getSplinePathId() const
{
    return getCurrentObjectId(ui->comboBoxSplinePath);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCutSplinePath::GetNotes() const
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::SetAliasSuffix1(const QString &alias)
{
    originAliasSuffix1 = alias;
    ui->lineEditAlias1->setText(originAliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCutSplinePath::GetAliasSuffix1() const
{
    return ui->lineEditAlias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSplinePath::SetAliasSuffix2(const QString &alias)
{
    originAliasSuffix2 = alias;
    ui->lineEditAlias2->setText(originAliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCutSplinePath::GetAliasSuffix2() const
{
    return ui->lineEditAlias2->text();
}
