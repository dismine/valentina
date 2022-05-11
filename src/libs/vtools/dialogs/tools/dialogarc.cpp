/************************************************************************
 **
 **  @file   dialogarc.cpp
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

#include "dialogarc.h"

#include <climits>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <Qt>

#include "../ifc/xml/vdomdocument.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/vcontainer.h"
#include "../../visualization/path/vistoolarc.h"
#include "../../visualization/visualization.h"
#include "../support/dialogeditwrongformula.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogarc.h"
#include "../vgeometry/varc.h"
#include "../qmuparser/qmudef.h"
#include "../vwidgets/vabstractmainwindow.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogArc create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogArc::DialogArc(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogArc),
      timerRadius(new QTimer(this)),
      timerF1(new QTimer(this)),
      timerF2(new QTimer(this))
{
    ui->setupUi(this);

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    this->formulaBaseHeight = ui->plainTextEditFormula->height();
    this->formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->formulaBaseHeightF2 = ui->plainTextEditF2->height();

    ui->plainTextEditFormula->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditF2->installEventFilter(this);

    timerRadius->setSingleShot(true);
    connect(timerRadius, &QTimer::timeout, this, &DialogArc::EvalRadius);

    timerF1->setSingleShot(true);
    connect(timerF1, &QTimer::timeout, this, &DialogArc::EvalF);

    timerF2->setSingleShot(true);
    connect(timerF2, &QTimer::timeout, this, &DialogArc::EvalF);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxBasePoint);
    FillComboBoxLineColors(ui->comboBoxColor);
    FillComboBoxTypeLine(ui->comboBoxPenStyle, CurvePenStylesPics());

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogArc::FXRadius);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogArc::FXF1);
    connect(ui->toolButtonExprF2, &QPushButton::clicked, this, &DialogArc::FXF2);

    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerRadius->start(formulaTimerTimeout);
    });

    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerF1->start(formulaTimerTimeout);
    });

    connect(ui->plainTextEditF2, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerF2->start(formulaTimerTimeout);
    });

    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogArc::DeployFormulaTextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogArc::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthF2, &QPushButton::clicked, this, &DialogArc::DeployF2TextEdit);

    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogArc::ValidateAlias);

    vis = new VisToolArc(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::DeployFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::DeployF1TextEdit()
{
    DeployFormula(this, ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::DeployF2TextEdit()
{
    DeployFormula(this, ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
}

//---------------------------------------------------------------------------------------------------------------------
DialogArc::~DialogArc()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetCenter set id of center point
 * @param value id
 */
void DialogArc::SetCenter(const quint32 &value)
{
    ChangeCurrentData(ui->comboBoxBasePoint, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetF2 set formula second angle of arc
 * @param value formula
 */
void DialogArc::SetF2(const QString &value)
{
    f2 = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f2.length() > 80)
    {
        this->DeployF2TextEdit();
    }
    ui->plainTextEditF2->setPlainText(f2);

    auto *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF2(f2);

    MoveCursorToEnd(ui->plainTextEditF2);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArc::GetPenStyle() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SetPenStyle(const QString &value)
{
    ChangeCurrentData(ui->comboBoxPenStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArc::GetColor() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SetColor(const QString &value)
{
    ChangeCurrentData(ui->comboBoxColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArc::GetApproximationScale() const -> qreal
{
    return ui->doubleSpinBoxApproximationScale->value();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SetApproximationScale(qreal value)
{
    ui->doubleSpinBoxApproximationScale->setValue(value);

    auto *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
            path->setApproximationScale(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArc::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SetAliasSuffix(const QString &alias)
{
    originAliasSuffix = alias;
    ui->lineEditAlias->setText(originAliasSuffix);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArc::GetAliasSuffix() const -> QString
{
    return ui->lineEditAlias->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::ShowDialog(bool click)
{
    if (prepare)
    {
        auto *arcVis = qobject_cast<VisToolArc *>(vis);
        SCASSERT(arcVis != nullptr)

        auto FinishCreating = [this, arcVis]()
        {
            arcVis->SetMode(Mode::Show);
            arcVis->RefreshGeometry();

            emit ToolTip(QString());

            setModal(true);
            show();
        };

        if (click)
        {
            // The check need to ignore first release of mouse button.
            // User can select point by clicking on a label.
            if (not m_firstRelease)
            {
                m_firstRelease = true;
                return;
            }

            /*We will ignore click if pointer is in point circle*/
            auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
            SCASSERT(scene != nullptr)
            const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(GetCenter());
            QLineF line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

            auto Angle = [line]()
            {
                if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
                {
                    QLineF correction = line;
                    correction.setAngle(VisToolArc::CorrectAngle(correction.angle()));
                    return correction.angle();
                }

                return line.angle();
            };

            if (stageRadius)
            {
                //Radius of point circle, but little bigger. Need handle with hover sizes.
                if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*1.5)
                {
                    return;
                }

                SetRadius(QString::number(VAbstractValApplication::VApp()->fromPixel(line.length())));
                arcVis->RefreshGeometry();
                stageRadius = false;
                stageF1 = true;
            }
            else if (stageF1)
            {
                SetF1(QString::number(Angle()));
                arcVis->RefreshGeometry();
                stageF1 = false;
            }
            else
            {
                SetF2(QString::number(arcVis->StickyEnd(Angle())));

                FinishCreating();
            }
            return;
        }

        FinishCreating();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetF1 set formula first angle of arc
 * @param value formula
 */
void DialogArc::SetF1(const QString &value)
{
    f1 = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(f1);

    auto *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF1(f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRadius set formula of radius
 * @param value formula
 */
void DialogArc::SetRadius(const QString &value)
{
    radius = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (radius.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(radius);

    auto *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius(radius);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogArc::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            if (SetObject(id, ui->comboBoxBasePoint, QString()))
            {
                if (vis != nullptr)
                {
                    auto *window = qobject_cast<VAbstractMainWindow *>(
                                VAbstractValApplication::VApp()->getMainWindow());
                    SCASSERT(window != nullptr)
                    connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

                    vis->VisualMode(id);
                    vis->RefreshToolTip();
                }

                prepare = true;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::ShowVisualization()
{
    AddVisualization<VisToolArc>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SaveData()
{
    radius = ui->plainTextEditFormula->toPlainText();
    f1 = ui->plainTextEditF1->toPlainText();
    f2 = ui->plainTextEditF2->toPlainText();

    auto *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(GetCenter());
    path->setRadius(radius);
    path->setF1(f1);
    path->setF2(f2);
    path->setApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditF2->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());
    VArc arc;
    arc.SetAliasSuffix(GetAliasSuffix());
    if (not GetAliasSuffix().isEmpty() &&
        (not rx.match(arc.GetAlias()).hasMatch() ||
         (originAliasSuffix != GetAliasSuffix() && not data->IsUnique(arc.GetAlias()))))
    {
        flagAlias = false;
        ChangeColor(ui->labelAlias, errorColor);
    }
    else
    {
        flagAlias = true;
        ChangeColor(ui->labelAlias, OkColor(this));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::FXRadius()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit radius"));
    dialog->SetFormula(GetRadius());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::FXF1()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit first angle"));
    dialog->SetFormula(GetF1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetF1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::FXF2()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit second angle"));
    dialog->SetFormula(GetF2());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetF2(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalRadius calculate value of radius
 */
void DialogArc::EvalRadius()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormula->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditRadius;
    formulaData.labelResult = ui->labelResultRadius;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

    const qreal radius = Eval(formulaData, flagRadius);

    if (radius < 0)
    {
        flagRadius = false;
        ChangeColor(ui->labelEditRadius, errorColor);
        ui->labelResultRadius->setText(tr("Error"));
        ui->labelResultRadius->setToolTip(tr("Radius can't be negative"));

        DialogArc::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalF1 calculate value of angle
 */
void DialogArc::EvalF()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditF1->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditF1;
    formulaData.labelResult = ui->labelResultF1;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    angleF1 = Eval(formulaData, flagF1);

    formulaData.formula = ui->plainTextEditF2->toPlainText();
    formulaData.labelEditFormula = ui->labelEditF2;
    formulaData.labelResult = ui->labelResultF2;

    angleF2 = Eval(formulaData, flagF2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetCenter return id of center point
 * @return id id
 */
auto DialogArc::GetCenter() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxBasePoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius return formula of radius
 * @return formula
 */
auto DialogArc::GetRadius() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(radius, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetF1 return formula first angle of arc
 * @return formula
 */
auto DialogArc::GetF1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(f1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetF2 return formula second angle of arc
 * @return formula
 */
auto DialogArc::GetF2() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(f2, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}
