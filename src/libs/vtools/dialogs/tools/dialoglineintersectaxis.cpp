/************************************************************************
 **
 **  @file   dialoglineintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
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

#include "dialoglineintersectaxis.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QToolButton>
#include <new>

#include "../../visualization/line/vistoollineintersectaxis.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
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
#include "ui_dialoglineintersectaxis.h"

//---------------------------------------------------------------------------------------------------------------------
DialogLineIntersectAxis::DialogLineIntersectAxis(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                                 QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogLineIntersectAxis),
    formulaAngle(),
    formulaBaseHeightAngle(0),
    pointName(),
    m_firstRelease(false),
    timerFormula(new QTimer(this)),
    flagFormula(false),
    flagError(true),
    flagName(true)
{
    ui->setupUi(this);

    InitIcons();

    timerFormula->setSingleShot(true);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    this->formulaBaseHeightAngle = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxAxisPoint);
    FillComboBoxPoints(ui->comboBoxFirstLinePoint);
    FillComboBoxPoints(ui->comboBoxSecondLinePoint);
    FillComboBoxTypeLine(ui->comboBoxLineType, LineStylesPics(ui->comboBoxLineType->palette().color(QPalette::Base),
                                                              ui->comboBoxLineType->palette().color(QPalette::Text)));
    FillComboBoxLineColors(ui->comboBoxLineColor);

    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogLineIntersectAxis::FXAngle);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
                CheckState();
            });
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this,
            [this]() { timerFormula->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowLengthAngle, &QPushButton::clicked, this, &DialogLineIntersectAxis::DeployAngleTextEdit);
    connect(timerFormula, &QTimer::timeout, this, &DialogLineIntersectAxis::EvalAngle);
    connect(ui->comboBoxFirstLinePoint, &QComboBox::currentTextChanged, this,
            &DialogLineIntersectAxis::PointNameChanged);
    connect(ui->comboBoxSecondLinePoint, &QComboBox::currentTextChanged, this,
            &DialogLineIntersectAxis::PointNameChanged);
    connect(ui->comboBoxAxisPoint, &QComboBox::currentTextChanged, this, &DialogLineIntersectAxis::PointNameChanged);

    vis = new VisToolLineIntersectAxis(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogLineIntersectAxis::~DialogLineIntersectAxis()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetPointName() const -> QString
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetTypeLine() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxLineType, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetTypeLine(const QString &value)
{
    ChangeCurrentData(ui->comboBoxLineType, value);
    vis->SetLineStyle(LineStyleToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetAngle(const QString &value)
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

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetAngle(formulaAngle);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetBasePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxAxisPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetBasePointId(quint32 value)
{
    setCurrentPointId(ui->comboBoxAxisPoint, value);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetAxisPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetFirstPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxFirstLinePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetFirstPointId(quint32 value)
{
    setCurrentPointId(ui->comboBoxFirstLinePoint, value);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetPoint1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetSecondPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxSecondLinePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetSecondPointId(quint32 value)
{
    setCurrentPointId(ui->comboBoxSecondLinePoint, value);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetPoint2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetLineColor() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxLineColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetLineColor(const QString &value)
{
    ChangeCurrentData(ui->comboBoxLineColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::ShowDialog(bool click)
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

        auto *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
        SCASSERT(line != nullptr)
        SetAngle(line->Angle()); // Show in dialog angle what user choose

        line->SetMode(Mode::Show);
        emit ToolTip(QString());
        DialogAccepted(); // Just set default values and don't show dialog
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false) // After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            auto *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
            SCASSERT(line != nullptr)

            switch (number)
            {
                case (0):
                    if (SetObject(id, ui->comboBoxFirstLinePoint, tr("Select second point of line")))
                    {
                        number++;
                        line->VisualMode(id);
                        auto *window =
                            qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
                        SCASSERT(window != nullptr)
                        connect(line, &VisToolLineIntersectAxis::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
                    }
                    break;
                case (1):
                    if (getCurrentObjectId(ui->comboBoxFirstLinePoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxSecondLinePoint, tr("Select axis point")))
                        {
                            number++;
                            line->SetPoint2Id(id);
                            line->RefreshGeometry();
                        }
                    }
                    break;
                case (2):
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxFirstLinePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxSecondLinePoint));
                    set.insert(id);

                    if (set.size() == 3)
                    {
                        if (SetObject(id, ui->comboBoxAxisPoint, QString()))
                        {
                            line->SetAxisPointId(id);
                            line->RefreshGeometry();
                            prepare = true;

                            if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                            {
                                vis->SetMode(Mode::Show);
                                emit ToolTip(QString());
                                show();
                            }
                        }
                    }
                }
                break;
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::EvalAngle()
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
void DialogLineIntersectAxis::DeployAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLengthAngle, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::PointNameChanged()
{
    QSet<quint32> set;
    set.insert(getCurrentObjectId(ui->comboBoxFirstLinePoint));
    set.insert(getCurrentObjectId(ui->comboBoxSecondLinePoint));
    set.insert(getCurrentObjectId(ui->comboBoxAxisPoint));

    QColor color;
    if (set.size() != 3)
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelFirstLinePoint, color);
    ChangeColor(ui->labelSecondLinePoint, color);
    ChangeColor(ui->labelAxisPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::FXAngle()
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
void DialogLineIntersectAxis::ShowVisualization()
{
    AddVisualization<VisToolLineIntersectAxis>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formulaAngle = ui->plainTextEditFormula->toPlainText();

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)

    line->SetPoint1Id(GetFirstPointId());
    line->SetPoint2Id(GetSecondPointId());
    line->SetAxisPointId(GetBasePointId());
    line->SetAngle(formulaAngle);
    line->SetLineStyle(LineStyleToPenStyle(GetTypeLine()));
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::changeEvent(QEvent *event)
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
void DialogLineIntersectAxis::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label_3->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogLineIntersectAxis::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
