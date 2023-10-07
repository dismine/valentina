/************************************************************************
 **
 **  @file   dialogcurveintersectaxis.cpp
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

#include "dialogcurveintersectaxis.h"

#include <QDialog>
#include <QLineEdit>
#include <QLineF>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QSharedPointer>
#include <QTimer>
#include <QToolButton>
#include <new>

#include "../../visualization/line/vistoolcurveintersectaxis.h"
#include "../../visualization/visualization.h"
#include "../dialogtoolbox.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogcurveintersectaxis.h"

//---------------------------------------------------------------------------------------------------------------------
DialogCurveIntersectAxis::DialogCurveIntersectAxis(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                                   QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogCurveIntersectAxis),
    formulaAngle(),
    formulaBaseHeightAngle(0),
    pointName(),
    m_firstRelease(false),
    timerFormula(new QTimer(this)),
    flagFormula(false),
    flagName(true)
{
    ui->setupUi(this);

    InitIcons();

    timerFormula->setSingleShot(true);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    formulaBaseHeightAngle = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxAxisPoint);
    FillComboBoxCurves(ui->comboBoxCurve);
    FillComboBoxTypeLine(ui->comboBoxLineType, LineStylesPics(ui->comboBoxLineType->palette().color(QPalette::Base),
                                                              ui->comboBoxLineType->palette().color(QPalette::Text)));
    FillComboBoxLineColors(ui->comboBoxLineColor);

    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogCurveIntersectAxis::FXAngle);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
                CheckState();
            });
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this,
            [this]() { timerFormula->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowLengthAngle, &QPushButton::clicked, this, &DialogCurveIntersectAxis::DeployAngleTextEdit);
    connect(timerFormula, &QTimer::timeout, this, &DialogCurveIntersectAxis::EvalAngle);
    connect(ui->lineEditAlias1, &QLineEdit::textEdited, this, &DialogCurveIntersectAxis::ValidateAlias);
    connect(ui->lineEditAlias2, &QLineEdit::textEdited, this, &DialogCurveIntersectAxis::ValidateAlias);

    vis = new VisToolCurveIntersectAxis(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCurveIntersectAxis::~DialogCurveIntersectAxis()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetPointName() const -> QString
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetTypeLine() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxLineType, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetTypeLine(const QString &value)
{
    ChangeCurrentData(ui->comboBoxLineType, value);
    vis->SetLineStyle(LineStyleToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetAngle(const QString &value)
{
    formulaAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formulaAngle.length() > 80)
    {
        this->DeployAngleTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formulaAngle);

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetAngle(formulaAngle);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetBasePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxAxisPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetBasePointId(quint32 value)
{
    setCurrentPointId(ui->comboBoxAxisPoint, value);

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->setAxisPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::getCurveId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCurve);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::setCurveId(quint32 value)
{
    setCurrentCurveId(ui->comboBoxCurve, value);

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetCurveId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetLineColor() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxLineColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetLineColor(const QString &value)
{
    ChangeCurrentData(ui->comboBoxLineColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ShowDialog(bool click)
{
    if (prepare)
    {
        if (click)
        {
            // The check need to ignore first release of mouse button.
            // User can select point by clicking on a label.
            if (not m_firstRelease)
            {
                m_firstRelease = true;
                return;
            }

            /*We will ignore click if poinet is in point circle*/
            auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
            SCASSERT(scene != nullptr)
            const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(GetBasePointId());
            QLineF line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

            // Radius of point circle, but little bigger. Need handle with hover sizes.
            if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
            {
                return;
            }
        }

        auto *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(line != nullptr)
        SetAngle(line->Angle()); // Show in dialog angle what user choose

        emit ToolTip(QString());
        DialogAccepted(); // Just set default values and don't show dialog
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false) // After first choose we ignore all objects
    {
        auto *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(line != nullptr)

        switch (number)
        {
            case (0):
                if (type == SceneObject::Spline || type == SceneObject::Arc || type == SceneObject::ElArc ||
                    type == SceneObject::SplinePath)
                {
                    if (SetObject(id, ui->comboBoxCurve, tr("Select axis point")))
                    {
                        number++;
                        line->VisualMode(id);
                        VAbstractMainWindow *window =
                            qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
                        SCASSERT(window != nullptr)
                        connect(line, &VisToolCurveIntersectAxis::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
                    }
                }
                break;
            case (1):
                if (type == SceneObject::Point)
                {
                    if (SetObject(id, ui->comboBoxAxisPoint, QString()))
                    {
                        line->setAxisPointId(id);
                        line->RefreshGeometry();
                        prepare = true;

                        if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                        {
                            emit ToolTip(QString());

                            setModal(true);
                            show();
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::EvalAngle()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormula->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormula;
    formulaData.labelResult = ui->labelResultCalculation;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    Eval(formulaData, flagFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::DeployAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLengthAngle, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::FXAngle()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit angle"));
    dialog->SetFormula(GetAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetAngle(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ShowVisualization()
{
    AddVisualization<VisToolCurveIntersectAxis>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formulaAngle = ui->plainTextEditFormula->toPlainText();

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)

    line->SetCurveId(getCurveId());
    line->setAxisPointId(GetBasePointId());
    line->SetAngle(formulaAngle);
    line->SetLineStyle(LineStyleToPenStyle(GetTypeLine()));
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
        InitDialogButtonBoxIcons(ui->buttonBox);
    }

    // remember to call base class implementation
    DialogTool::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(getCurveId());
    QPair<QString, QString> alias = SegmentAliases(curve->getType(), GetAliasSuffix1(), GetAliasSuffix2());

    if (not GetAliasSuffix1().isEmpty() &&
        (not rx.match(alias.first).hasMatch() ||
         (originAliasSuffix1 != GetAliasSuffix1() && not data->IsUnique(alias.first)) || alias.first == alias.second))
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
        (not rx.match(alias.second).hasMatch() ||
         (originAliasSuffix2 != GetAliasSuffix2() && not data->IsUnique(alias.second)) || alias.first == alias.second))
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
void DialogCurveIntersectAxis::InitIcons()
{
    QString resource = QStringLiteral("icon");

    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label_3->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetAliasSuffix1(const QString &alias)
{
    originAliasSuffix1 = alias;
    ui->lineEditAlias1->setText(originAliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetAliasSuffix1() const -> QString
{
    return ui->lineEditAlias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetAliasSuffix2(const QString &alias)
{
    originAliasSuffix2 = alias;
    ui->lineEditAlias2->setText(originAliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetAliasSuffix2() const -> QString
{
    return ui->lineEditAlias2->text();
}
