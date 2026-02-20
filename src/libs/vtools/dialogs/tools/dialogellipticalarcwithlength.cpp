/************************************************************************
 **
 **  @file   dialogellipticalarcwithlength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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

#include "dialogellipticalarcwithlength.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

#include "../../visualization/path/vistoolellipticalarcwithlength.h"
#include "../../visualization/visualization.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogellipticalarcwithlength.h"

//---------------------------------------------------------------------------------------------------------------------
DialogEllipticalArcWithLength::DialogEllipticalArcWithLength(const VContainer *data,
                                                             VAbstractPattern *doc,
                                                             quint32 toolId,
                                                             QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogEllipticalArcWithLength),
    m_timerRadius1(new QTimer(this)),
    m_timerRadius2(new QTimer(this)),
    m_timerF1(new QTimer(this)),
    m_timerLength(new QTimer(this)),
    m_timerRotationAngle(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    this->m_formulaBaseHeightRadius1 = ui->plainTextEditRadius1->height();
    this->m_formulaBaseHeightRadius2 = ui->plainTextEditRadius2->height();
    this->m_formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->m_formulaBaseHeightLength = ui->plainTextEditLength->height();
    this->m_formulaBaseHeightRotationAngle = ui->plainTextEditRotationAngle->height();

    ui->plainTextEditRadius1->installEventFilter(this);
    ui->plainTextEditRadius2->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditLength->installEventFilter(this);
    ui->plainTextEditRotationAngle->installEventFilter(this);

    m_timerRadius1->setSingleShot(true);
    connect(m_timerRadius1, &QTimer::timeout, this, &DialogEllipticalArcWithLength::Radius1);

    m_timerRadius2->setSingleShot(true);
    connect(m_timerRadius2, &QTimer::timeout, this, &DialogEllipticalArcWithLength::Radius2);

    m_timerF1->setSingleShot(true);
    connect(m_timerF1, &QTimer::timeout, this, &DialogEllipticalArcWithLength::EvalF);

    m_timerLength->setSingleShot(true);
    connect(m_timerLength, &QTimer::timeout, this, &DialogEllipticalArcWithLength::Length);

    m_timerRotationAngle->setSingleShot(true);
    connect(m_timerRotationAngle, &QTimer::timeout, this, &DialogEllipticalArcWithLength::RotationAngle);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxCenter);
    InitColorPicker(ui->pushButtonColor, VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         CurvePenStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                            ui->comboBoxPenStyle->palette().color(QPalette::Text)));

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    connect(ui->toolButtonExprRadius1, &QPushButton::clicked, this, &DialogEllipticalArcWithLength::FXRadius1);
    connect(ui->toolButtonExprRadius2, &QPushButton::clicked, this, &DialogEllipticalArcWithLength::FXRadius2);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogEllipticalArcWithLength::FXF1);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogEllipticalArcWithLength::FXLength);
    connect(ui->toolButtonExprRotationAngle,
            &QPushButton::clicked,
            this,
            &DialogEllipticalArcWithLength::FXRotationAngle);

    connect(ui->plainTextEditRadius1,
            &QPlainTextEdit::textChanged,
            this,
            [this]() { m_timerRadius1->start(formulaTimerTimeout); });
    connect(ui->plainTextEditRadius2,
            &QPlainTextEdit::textChanged,
            this,
            [this]() { m_timerRadius2->start(formulaTimerTimeout); });

    connect(ui->plainTextEditF1,
            &QPlainTextEdit::textChanged,
            this,
            [this]() { m_timerF1->start(formulaTimerTimeout); });

    connect(ui->plainTextEditLength,
            &QPlainTextEdit::textChanged,
            this,
            [this]() { m_timerLength->start(formulaTimerTimeout); });

    connect(ui->plainTextEditRotationAngle,
            &QPlainTextEdit::textChanged,
            this,
            [this]() { m_timerRotationAngle->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowLengthRadius1,
            &QPushButton::clicked,
            this,
            &DialogEllipticalArcWithLength::DeployRadius1TextEdit);
    connect(ui->pushButtonGrowLengthRadius2,
            &QPushButton::clicked,
            this,
            &DialogEllipticalArcWithLength::DeployRadius2TextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogEllipticalArcWithLength::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthArcLength,
            &QPushButton::clicked,
            this,
            &DialogEllipticalArcWithLength::DeployLengthTextEdit);
    connect(ui->pushButtonGrowLengthRotationAngle,
            &QPushButton::clicked,
            this,
            &DialogEllipticalArcWithLength::DeployRotationAngleTextEdit);

    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogEllipticalArcWithLength::ValidateAlias);

    vis = new VisToolEllipticalArcWithLength(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogEllipticalArcWithLength::~DialogEllipticalArcWithLength()
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetCenter() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetCenter(const quint32 &value)
{
    ChangeCurrentData(ui->comboBoxCenter, value);
    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetCenterId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetRadius1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_radius1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetRadius1(const QString &value)
{
    m_radius1 = VAbstractApplication::VApp()
                    ->TrVars()
                    ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_radius1.length() > 80)
    {
        this->DeployRadius1TextEdit();
    }
    ui->plainTextEditRadius1->setPlainText(m_radius1);

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetRadius1(m_radius1);

    MoveCursorToEnd(ui->plainTextEditRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetRadius2() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_radius2, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetRadius2(const QString &value)
{
    m_radius2 = VAbstractApplication::VApp()
                    ->TrVars()
                    ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_radius2.length() > 80)
    {
        this->DeployRadius2TextEdit();
    }
    ui->plainTextEditRadius2->setPlainText(m_radius2);

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetRadius2(m_radius2);

    MoveCursorToEnd(ui->plainTextEditRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetF1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_f1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetF1(const QString &value)
{
    m_f1 = VAbstractApplication::VApp()
               ->TrVars()
               ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(m_f1);

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetF1(m_f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetLength() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_length, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetLength(const QString &value)
{
    m_length = VAbstractApplication::VApp()
                   ->TrVars()
                   ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_length.length() > 80)
    {
        this->DeployLengthTextEdit();
    }
    ui->plainTextEditLength->setPlainText(m_length);

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetLength(m_length);

    MoveCursorToEnd(ui->plainTextEditLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetRotationAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_rotationAngle,
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetRotationAngle(const QString &value)
{
    m_rotationAngle = VAbstractApplication::VApp()
                          ->TrVars()
                          ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_rotationAngle.length() > 80)
    {
        this->DeployRotationAngleTextEdit();
    }
    ui->plainTextEditRotationAngle->setPlainText(m_rotationAngle);

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetRotationAngle(m_rotationAngle);

    MoveCursorToEnd(ui->plainTextEditRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetPenStyle() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetPenStyle(const QString &value)
{
    ChangeCurrentData(ui->comboBoxPenStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetColor() const -> QString
{
    return ui->pushButtonColor->currentColor().name();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetColor(const QString &value)
{
    ui->pushButtonColor->setCurrentColor(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetApproximationScale() const -> qreal
{
    return ui->doubleSpinBoxApproximationScale->value();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetApproximationScale(qreal value)
{
    ui->doubleSpinBoxApproximationScale->setValue(value);

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->SetApproximationScale(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SetAliasSuffix(const QString &alias)
{
    m_originAliasSuffix = alias;
    ui->lineEditAlias->setText(m_originAliasSuffix);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArcWithLength::GetAliasSuffix() const -> QString
{
    return ui->lineEditAlias->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::ShowDialog(bool click)
{
    if (!prepare)
    {
        return;
    }

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
        const QSharedPointer<VPointF> center = data->GeometricObject<VPointF>(GetCenter());
        auto line = QLineF(static_cast<QPointF>(*center), scene->getScenePos());

        auto Angle = [&line]()
        {
            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                QLineF correction = line;
                correction.setAngle(VisToolEllipticalArcWithLength::CorrectAngle(correction.angle()));
                return correction.angle();
            }

            return line.angle();
        };

        if (m_stage == 0) // radius 1
        {
            // Radius of point circle, but little bigger. Need to handle with hover sizes.
            if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
            {
                return;
            }

            SetRadius1(QString::number(FromPixel(line.length(), *data->GetPatternUnit())));
            vis->RefreshGeometry();
            ++m_stage;
            return;
        }

        if (m_stage == 1) // radius 2
        {
            QLineF const radius2Line(center->x(), center->y(), center->x(), center->y() - 100);
            QPointF const p = VGObject::ClosestPoint(radius2Line, scene->getScenePos());
            line = QLineF(static_cast<QPointF>(*center), p);

            // Radius of point circle, but little bigger. Need to handle with hover sizes.
            if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
            {
                return;
            }

            SetRadius2(QString::number(FromPixel(line.length(), *data->GetPatternUnit())));
            vis->RefreshGeometry();
            ++m_stage;
            return;
        }

        if (m_stage == 2) // f1
        {
            SetF1(QString::number(Angle()));
            vis->RefreshGeometry();
            ++m_stage;
            return;
        }

        if (m_stage == 3) // length
        {
            const qreal r1 = Visualization::FindLengthFromUser(m_radius1, data->DataVariables());
            const qreal r2 = Visualization::FindLengthFromUser(m_radius2, data->DataVariables());
            const qreal angle1 = Visualization::FindValFromUser(m_f1, data->DataVariables());
            VEllipticalArc const arc(*center, r1, r2, angle1, line.angle(), 0);

            SetLength(QString::number(VAbstractValApplication::VApp()->fromPixel(arc.GetLength())));
            vis->RefreshGeometry();
            ++m_stage;
            return;
        }

        auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
        SCASSERT(path != nullptr)

        SetRotationAngle(QString::number(Angle() - path->StartingRotationAngle()));
    }

    FinishCreating();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::CheckDependencyTreeComplete()
{
    const bool ready = m_doc->IsPatternGraphComplete();
    ui->comboBoxCenter->setEnabled(ready);
    ui->lineEditAlias->setEnabled(ready);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if (type != SceneObject::Point)
    {
        return;
    }

    if (SetObject(id, ui->comboBoxCenter, QString()))
    {
        if (vis != nullptr)
        {
            auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
            SCASSERT(window != nullptr)
            connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

            vis->VisualMode(id);
        }
        prepare = true;

        if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
        {
            FinishCreating();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::DeployRadius1TextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, m_formulaBaseHeightRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::DeployRadius2TextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, m_formulaBaseHeightRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::DeployF1TextEdit()
{
    DeployFormula(this, ui->plainTextEditF1, ui->pushButtonGrowLengthF1, m_formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::DeployLengthTextEdit()
{
    DeployFormula(this, ui->plainTextEditLength, ui->pushButtonGrowLengthArcLength, m_formulaBaseHeightLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::DeployRotationAngleTextEdit()
{
    DeployFormula(this,
                  ui->plainTextEditRotationAngle,
                  ui->pushButtonGrowLengthRotationAngle,
                  m_formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::FXRadius1()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit radius1"));
    dialog->SetFormula(GetRadius1());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRadius1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::FXRadius2()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit radius2"));
    dialog->SetFormula(GetRadius2());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRadius2(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::FXF1()
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
void DialogEllipticalArcWithLength::FXLength()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit the elliptical arc length"));
    dialog->SetFormula(GetLength());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetLength(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::FXRotationAngle()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit rotation angle"));
    dialog->SetFormula(GetRotationAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRotationAngle(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::ShowVisualization()
{
    AddVisualization<VisToolEllipticalArcWithLength>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::SaveData()
{
    m_radius1 = ui->plainTextEditRadius1->toPlainText();
    m_radius2 = ui->plainTextEditRadius2->toPlainText();
    m_f1 = ui->plainTextEditF1->toPlainText();
    m_length = ui->plainTextEditLength->toPlainText();
    m_rotationAngle = ui->plainTextEditRotationAngle->toPlainText();

    auto *path = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
    SCASSERT(path != nullptr)

    path->SetCenterId(GetCenter());
    path->SetRadius1(m_radius1);
    path->SetRadius2(m_radius2);
    path->SetF1(m_f1);
    path->SetLength(m_length);
    path->SetRotationAngle(m_rotationAngle);
    path->SetApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius1->blockSignals(true);
    ui->plainTextEditRadius2->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditLength->blockSignals(true);
    ui->plainTextEditRotationAngle->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::changeEvent(QEvent *event)
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
void DialogEllipticalArcWithLength::ValidateAlias()
{
    VEllipticalArc arc;
    arc.SetAliasSuffix(GetAliasSuffix());
    if (QRegularExpression const rx(NameRegExp());
        not GetAliasSuffix().isEmpty()
        && (not rx.match(arc.GetAlias()).hasMatch()
            || (m_originAliasSuffix != GetAliasSuffix() && not data->IsUnique(arc.GetAlias()))))
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
void DialogEllipticalArcWithLength::Radius1()
{
    Eval({.formula = ui->plainTextEditRadius1->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditRadius1,
          .labelResult = ui->labelResultRadius1,
          .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
          .checkLessThanZero = true},
         m_flagRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::Radius2()
{
    Eval({.formula = ui->plainTextEditRadius2->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditRadius2,
          .labelResult = ui->labelResultRadius2,
          .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
          .checkLessThanZero = true},
         m_flagRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::Length()
{
    Eval({.formula = ui->plainTextEditLength->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditLength,
          .labelResult = ui->labelResultLength,
          .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true)},
         m_flagLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::EvalF()
{
    Eval({.formula = ui->plainTextEditF1->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditF1,
          .labelResult = ui->labelResultF1,
          .postfix = degreeSymbol},
         m_flagF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::RotationAngle()
{
    Eval({.formula = ui->plainTextEditRotationAngle->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditRotationAngle,
          .labelResult = ui->labelResultRotationAngle,
          .postfix = degreeSymbol},
         m_flagRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArcWithLength::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    const auto fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprRadius1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprRadius2->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprF1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprLength->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprRotationAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const auto equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_1->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_12->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_9->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_10->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_13->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}
