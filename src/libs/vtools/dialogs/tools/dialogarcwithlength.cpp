/************************************************************************
 **
 **  @file   dialogarcwithlength.cpp
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

#include "dialogarcwithlength.h"

#include <climits>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <Qt>

#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/vcontainer.h"
#include "../../visualization/path/vistoolarcwithlength.h"
#include "../support/dialogeditwrongformula.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../../visualization/visualization.h"
#include "ui_dialogarcwithlength.h"
#include "../vgeometry/varc.h"
#include "../qmuparser/qmudef.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/global.h"

//---------------------------------------------------------------------------------------------------------------------
DialogArcWithLength::DialogArcWithLength(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogArcWithLength),
      timerRadius(new QTimer(this)),
      timerF1(new QTimer(this)),
      timerLength(new QTimer(this))
{
    ui->setupUi(this);

    this->formulaBaseHeightRadius = ui->plainTextEditRadius->height();
    this->formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->formulaBaseHeightLength = ui->plainTextEditLength->height();

    ui->plainTextEditRadius->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditLength->installEventFilter(this);

    timerRadius->setSingleShot(true);
    connect(timerRadius, &QTimer::timeout, this, &DialogArcWithLength::Radius);

    timerF1->setSingleShot(true);
    connect(timerF1, &QTimer::timeout, this, &DialogArcWithLength::EvalF);

    timerLength->setSingleShot(true);
    connect(timerLength, &QTimer::timeout, this, &DialogArcWithLength::Length);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxCenter);
    FillComboBoxLineColors(ui->comboBoxColor);
    FillComboBoxTypeLine(ui->comboBoxPenStyle, CurvePenStylesPics());

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogArcWithLength::FXRadius);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogArcWithLength::FXF1);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogArcWithLength::FXLength);

    connect(ui->plainTextEditRadius, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerRadius->start(formulaTimerTimeout);
    });

    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerF1->start(formulaTimerTimeout);
    });

    connect(ui->plainTextEditLength, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerLength->start(formulaTimerTimeout);
    });

    connect(ui->pushButtonGrowLengthRadius, &QPushButton::clicked, this, &DialogArcWithLength::DeployRadiusTextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogArcWithLength::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthArcLength, &QPushButton::clicked, this, &DialogArcWithLength::DeployLengthTextEdit);

    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogArcWithLength::ValidateAlias);

    vis = new VisToolArcWithLength(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogArcWithLength::~DialogArcWithLength()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetCenter() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetCenter(const quint32 &value)
{
    ChangeCurrentData(ui->comboBoxCenter, value);
    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetCenterId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetRadius() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(radius, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetRadius(const QString &value)
{
    radius = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (radius.length() > 80)
    {
        this->DeployRadiusTextEdit();
    }
    ui->plainTextEditRadius->setPlainText(radius);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetRadius(radius);

    MoveCursorToEnd(ui->plainTextEditRadius);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetF1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(f1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetF1(const QString &value)
{
    f1 = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(f1);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetF1(f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetLength() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(length, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetLength(const QString &value)
{
    length = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (length.length() > 80)
    {
        this->DeployLengthTextEdit();
    }
    ui->plainTextEditLength->setPlainText(length);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetLength(length);

    MoveCursorToEnd(ui->plainTextEditLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetPenStyle() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetPenStyle(const QString &value)
{
    ChangeCurrentData(ui->comboBoxPenStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetColor() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetColor(const QString &value)
{
    ChangeCurrentData(ui->comboBoxColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetApproximationScale() const -> qreal
{
    return ui->doubleSpinBoxApproximationScale->value();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetApproximationScale(qreal value)
{
    ui->doubleSpinBoxApproximationScale->setValue(value);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
            path->SetApproximationScale(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetAliasSuffix(const QString &alias)
{
    originAliasSuffix = alias;
    ui->lineEditAlias->setText(originAliasSuffix);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetAliasSuffix() const -> QString
{
    return ui->lineEditAlias->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::ShowDialog(bool click)
{
    if (prepare)
    {
        auto FinishCreating = [this]()
        {
            vis->SetMode(Mode::Show);
            vis->RefreshGeometry();

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
                    correction.setAngle(VisToolArcWithLength::CorrectAngle(correction.angle()));
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
                vis->RefreshGeometry();

                stageRadius = false;
                stageF1 = true;
            }
            else if (stageF1)
            {

                SetF1(QString::number(Angle()));
                vis->RefreshGeometry();

                stageF1 = false;
            }
            else
            {
                const qreal r = Visualization::FindLengthFromUser(radius, data->DataVariables());
                const qreal angle1 = Visualization::FindValFromUser(f1, data->DataVariables());
                VArc arc(*point, r, angle1, line.angle());

                SetLength(QString::number(VAbstractValApplication::VApp()->fromPixel(arc.GetLength())));

                FinishCreating();
            }

            return;
        }

        FinishCreating();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            if (SetObject(id, ui->comboBoxCenter, QString()))
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
void DialogArcWithLength::DeployRadiusTextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius, ui->pushButtonGrowLengthArcLength, formulaBaseHeightRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployF1TextEdit()
{
    DeployFormula(this, ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployLengthTextEdit()
{
    DeployFormula(this, ui->plainTextEditLength, ui->pushButtonGrowLengthArcLength, formulaBaseHeightLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::FXRadius()
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
void DialogArcWithLength::FXF1()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit the first angle"));
    dialog->SetFormula(GetF1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetF1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::FXLength()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit the arc length"));
    dialog->SetFormula(GetLength());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetLength(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::ShowVisualization()
{
    AddVisualization<VisToolArcWithLength>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SaveData()
{
    radius = ui->plainTextEditRadius->toPlainText();
    f1 = ui->plainTextEditF1->toPlainText();
    length = ui->plainTextEditLength->toPlainText();

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)

    path->SetCenterId(GetCenter());
    path->SetRadius(radius);
    path->SetF1(f1);
    path->SetLength(length);
    path->SetApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::ValidateAlias()
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
void DialogArcWithLength::Radius()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditRadius->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditRadius;
    formulaData.labelResult = ui->labelResultRadius;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkLessThanZero = true;

    Eval(formulaData, flagRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::Length()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditLength->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditLength;
    formulaData.labelResult = ui->labelResultLength;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

    Eval(formulaData, flagLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::EvalF()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditF1->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditF1;
    formulaData.labelResult = ui->labelResultF1;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    Eval(formulaData, flagF1);
}
