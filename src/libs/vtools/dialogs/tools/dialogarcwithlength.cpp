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

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <climits>

#include "../../visualization/path/vistoolarcwithlength.h"
#include "../../visualization/visualization.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/varc.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogarcwithlength.h"

//---------------------------------------------------------------------------------------------------------------------
DialogArcWithLength::DialogArcWithLength(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogArcWithLength),
    m_timerRadius(new QTimer(this)),
    m_timerF1(new QTimer(this)),
    m_timerLength(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    this->m_formulaBaseHeightRadius = ui->plainTextEditRadius->height();
    this->m_formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->m_formulaBaseHeightLength = ui->plainTextEditLength->height();

    ui->plainTextEditRadius->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditLength->installEventFilter(this);

    m_timerRadius->setSingleShot(true);
    connect(m_timerRadius, &QTimer::timeout, this, &DialogArcWithLength::Radius);

    m_timerF1->setSingleShot(true);
    connect(m_timerF1, &QTimer::timeout, this, &DialogArcWithLength::EvalF);

    m_timerLength->setSingleShot(true);
    connect(m_timerLength, &QTimer::timeout, this, &DialogArcWithLength::Length);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxCenter);
    FillComboBoxLineColors(ui->comboBoxColor);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         CurvePenStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                            ui->comboBoxPenStyle->palette().color(QPalette::Text)));

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogArcWithLength::FXRadius);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogArcWithLength::FXF1);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogArcWithLength::FXLength);

    connect(ui->plainTextEditRadius, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerRadius->start(formulaTimerTimeout); });

    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerF1->start(formulaTimerTimeout); });

    connect(ui->plainTextEditLength, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerLength->start(formulaTimerTimeout); });

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
    return VTranslateVars::TryFormulaFromUser(m_radius, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetRadius(const QString &value)
{
    m_radius = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_radius.length() > 80)
    {
        this->DeployRadiusTextEdit();
    }
    ui->plainTextEditRadius->setPlainText(m_radius);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetRadius(m_radius);

    MoveCursorToEnd(ui->plainTextEditRadius);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetF1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_f1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetF1(const QString &value)
{
    m_f1 = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(m_f1);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetF1(m_f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogArcWithLength::GetLength() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_length, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetLength(const QString &value)
{
    m_length = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_length.length() > 80)
    {
        this->DeployLengthTextEdit();
    }
    ui->plainTextEditLength->setPlainText(m_length);

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetLength(m_length);

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
    m_originAliasSuffix = alias;
    ui->lineEditAlias->setText(m_originAliasSuffix);
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

            if (m_stageRadius)
            {
                // Radius of point circle, but little bigger. Need handle with hover sizes.
                if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
                {
                    return;
                }

                SetRadius(QString::number(VAbstractValApplication::VApp()->fromPixel(line.length())));
                vis->RefreshGeometry();

                m_stageRadius = false;
                m_stageF1 = true;
            }
            else if (m_stageF1)
            {

                SetF1(QString::number(Angle()));
                vis->RefreshGeometry();

                m_stageF1 = false;
            }
            else
            {
                const qreal r = Visualization::FindLengthFromUser(m_radius, data->DataVariables());
                const qreal angle1 = Visualization::FindValFromUser(m_f1, data->DataVariables());
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
    if (not prepare) // After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            if (SetObject(id, ui->comboBoxCenter, QString()))
            {
                if (vis != nullptr)
                {
                    auto *window =
                        qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
                    SCASSERT(window != nullptr)
                    connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

                    vis->VisualMode(id);
                    vis->RefreshToolTip();
                }

                prepare = true;

                if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                {
                    FinishCreating();
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployRadiusTextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius, ui->pushButtonGrowLengthArcLength, m_formulaBaseHeightRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployF1TextEdit()
{
    DeployFormula(this, ui->plainTextEditF1, ui->pushButtonGrowLengthF1, m_formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployLengthTextEdit()
{
    DeployFormula(this, ui->plainTextEditLength, ui->pushButtonGrowLengthArcLength, m_formulaBaseHeightLength);
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
    m_radius = ui->plainTextEditRadius->toPlainText();
    m_f1 = ui->plainTextEditF1->toPlainText();
    m_length = ui->plainTextEditLength->toPlainText();

    auto *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)

    path->SetCenterId(GetCenter());
    path->SetRadius(m_radius);
    path->SetF1(m_f1);
    path->SetLength(m_length);
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
void DialogArcWithLength::changeEvent(QEvent *event)
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
void DialogArcWithLength::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());
    VArc arc;
    arc.SetAliasSuffix(GetAliasSuffix());
    if (not GetAliasSuffix().isEmpty() &&
        (not rx.match(arc.GetAlias()).hasMatch() ||
         (m_originAliasSuffix != GetAliasSuffix() && not data->IsUnique(arc.GetAlias()))))
    {
        m_flagAlias = false;
        ChangeColor(ui->labelAlias, errorColor);
    }
    else
    {
        m_flagAlias = true;
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

    Eval(formulaData, m_flagRadius);
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

    Eval(formulaData, m_flagLength);
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

    Eval(formulaData, m_flagF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    const QString fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprRadius->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprF1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprLength->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const QString equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_6->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_9->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_10->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}
