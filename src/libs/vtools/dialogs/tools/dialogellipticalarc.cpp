/************************************************************************
 **
 **  @file   dialogellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   15 9, 2016
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

#include "dialogellipticalarc.h"

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <climits>

#include "../../visualization/path/vistoolellipticalarc.h"
#include "../../visualization/visualization.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogellipticalarc.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogEllipticalArc create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogEllipticalArc::DialogEllipticalArc(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogEllipticalArc),
    m_timerRadius1(new QTimer(this)),
    m_timerRadius2(new QTimer(this)),
    m_timerF1(new QTimer(this)),
    m_timerF2(new QTimer(this)),
    m_timerRotationAngle(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    this->m_formulaBaseHeightRadius1 = ui->plainTextEditRadius1->height();
    this->m_formulaBaseHeightRadius2 = ui->plainTextEditRadius2->height();
    this->m_formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->m_formulaBaseHeightF2 = ui->plainTextEditF2->height();
    this->m_formulaBaseHeightRotationAngle = ui->plainTextEditRotationAngle->height();

    ui->plainTextEditRadius1->installEventFilter(this);
    ui->plainTextEditRadius2->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditF2->installEventFilter(this);
    ui->plainTextEditRotationAngle->installEventFilter(this);

    m_timerRadius1->setSingleShot(true);
    connect(m_timerRadius1, &QTimer::timeout, this, &DialogEllipticalArc::EvalRadiuses);

    m_timerRadius2->setSingleShot(true);
    connect(m_timerRadius2, &QTimer::timeout, this, &DialogEllipticalArc::EvalRadiuses);

    m_timerF1->setSingleShot(true);
    connect(m_timerF1, &QTimer::timeout, this, &DialogEllipticalArc::EvalAngles);

    m_timerF2->setSingleShot(true);
    connect(m_timerF2, &QTimer::timeout, this, &DialogEllipticalArc::EvalAngles);

    m_timerRotationAngle->setSingleShot(true);
    connect(m_timerRotationAngle, &QTimer::timeout, this, &DialogEllipticalArc::EvalAngles);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxBasePoint);
    FillComboBoxLineColors(ui->comboBoxColor);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         CurvePenStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                            ui->comboBoxPenStyle->palette().color(QPalette::Text)));

    connect(ui->toolButtonExprRadius1, &QPushButton::clicked, this, &DialogEllipticalArc::FXRadius1);
    connect(ui->toolButtonExprRadius2, &QPushButton::clicked, this, &DialogEllipticalArc::FXRadius2);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogEllipticalArc::FXF1);
    connect(ui->toolButtonExprF2, &QPushButton::clicked, this, &DialogEllipticalArc::FXF2);
    connect(ui->toolButtonExprRotationAngle, &QPushButton::clicked, this, &DialogEllipticalArc::FXRotationAngle);

    connect(ui->plainTextEditRadius1, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerRadius1->start(formulaTimerTimeout); });

    connect(ui->plainTextEditRadius2, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerRadius2->start(formulaTimerTimeout); });

    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerF1->start(formulaTimerTimeout); });

    connect(ui->plainTextEditF2, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerF2->start(formulaTimerTimeout); });

    connect(ui->plainTextEditRotationAngle, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerRotationAngle->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowLengthRadius1, &QPushButton::clicked, this, &DialogEllipticalArc::DeployRadius1TextEdit);
    connect(ui->pushButtonGrowLengthRadius2, &QPushButton::clicked, this, &DialogEllipticalArc::DeployRadius2TextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogEllipticalArc::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthF2, &QPushButton::clicked, this, &DialogEllipticalArc::DeployF2TextEdit);
    connect(ui->pushButtonGrowLengthRotationAngle, &QPushButton::clicked, this,
            &DialogEllipticalArc::DeployRotationAngleTextEdit);

    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogEllipticalArc::ValidateAlias);

    vis = new VisToolEllipticalArc(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogEllipticalArc::~DialogEllipticalArc()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetCenter return id of center point
 * @return id id
 */
auto DialogEllipticalArc::GetCenter() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxBasePoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetCenter set id of center point
 * @param value id
 */
void DialogEllipticalArc::SetCenter(quint32 value)
{
    ChangeCurrentData(ui->comboBoxBasePoint, value);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetCenterId(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius1 return formula of radius1
 * @return formula
 */
auto DialogEllipticalArc::GetRadius1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_radius1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRadius1 set formula of radius1
 * @param value formula
 */
void DialogEllipticalArc::SetRadius1(const QString &value)
{
    m_radius1 = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_radius1.length() > 80)
    {
        this->DeployRadius1TextEdit();
    }
    ui->plainTextEditRadius1->setPlainText(m_radius1);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetRadius1(m_radius1);

    MoveCursorToEnd(ui->plainTextEditRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius2 return formula of radius2
 * @return formula
 */
auto DialogEllipticalArc::GetRadius2() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_radius2, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRadius2 set formula of radius2
 * @param value formula
 */
void DialogEllipticalArc::SetRadius2(const QString &value)
{
    m_radius2 = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_radius2.length() > 80)
    {
        this->DeployRadius2TextEdit();
    }
    ui->plainTextEditRadius2->setPlainText(m_radius2);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetRadius2(m_radius2);

    MoveCursorToEnd(ui->plainTextEditRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetF1 return formula first angle of elliptical arc
 * @return formula
 */
auto DialogEllipticalArc::GetF1() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_f1, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetF1 set formula first angle of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetF1(const QString &value)
{
    m_f1 = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(m_f1);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetF1(m_f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetF2 return formula second angle of elliptical arc
 * @return formula
 */
auto DialogEllipticalArc::GetF2() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_f2, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetF2 set formula second angle of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetF2(const QString &value)
{
    m_f2 = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_f2.length() > 80)
    {
        this->DeployF2TextEdit();
    }
    ui->plainTextEditF2->setPlainText(m_f2);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetF2(m_f2);

    MoveCursorToEnd(ui->plainTextEditF2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRotationAngle return formula rotation angle of elliptical arc
 * @return formula
 */
auto DialogEllipticalArc::GetRotationAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_rotationAngle,
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRotationAngle set formula rotation angle of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetRotationAngle(const QString &value)
{
    m_rotationAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_rotationAngle.length() > 80)
    {
        this->DeployRotationAngleTextEdit();
    }
    ui->plainTextEditRotationAngle->setPlainText(m_rotationAngle);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetRotationAngle(m_rotationAngle);

    MoveCursorToEnd(ui->plainTextEditRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArc::GetPenStyle() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetPenStyle(const QString &value)
{
    ChangeCurrentData(ui->comboBoxPenStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetColor return color of elliptical arc
 * @return formula
 */
auto DialogEllipticalArc::GetColor() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetColor set color of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetColor(const QString &value)
{
    ChangeCurrentData(ui->comboBoxColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArc::GetApproximationScale() const -> qreal
{
    return ui->doubleSpinBoxApproximationScale->value();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetApproximationScale(qreal value)
{
    ui->doubleSpinBoxApproximationScale->setValue(value);

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetApproximationScale(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalRadiuses calculate value of radiuses
 */
void DialogEllipticalArc::EvalRadiuses()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditRadius1->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditRadius1;
    formulaData.labelResult = ui->labelResultRadius1;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkLessThanZero = true;

    Eval(formulaData, m_flagRadius1);

    formulaData.formula = ui->plainTextEditRadius2->toPlainText();
    formulaData.labelEditFormula = ui->labelEditRadius2;
    formulaData.labelResult = ui->labelResultRadius2;

    Eval(formulaData, m_flagRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalAngles calculate value of angles
 */
void DialogEllipticalArc::EvalAngles()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditF1->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditF1;
    formulaData.labelResult = ui->labelResultF1;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    m_angleF1 = Eval(formulaData, m_flagF1);

    formulaData.formula = ui->plainTextEditF2->toPlainText();
    formulaData.labelEditFormula = ui->labelEditF2;
    formulaData.labelResult = ui->labelResultF2;

    m_angleF2 = Eval(formulaData, m_flagF2);

    formulaData.formula = ui->plainTextEditRotationAngle->toPlainText();
    formulaData.labelEditFormula = ui->labelEditRotationAngle;
    formulaData.labelResult = ui->labelResultRotationAngle;

    m_angleRotation = Eval(formulaData, m_flagRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    const QString fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprRadius1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprRadius2->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprF1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprF2->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprRotationAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const QString equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_1->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_7->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_8->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_9->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_12->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FXRadius1()
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
void DialogEllipticalArc::FXRadius2()
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
void DialogEllipticalArc::FXF1()
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
void DialogEllipticalArc::FXF2()
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
void DialogEllipticalArc::FXRotationAngle()
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
void DialogEllipticalArc::DeployRadius1TextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, m_formulaBaseHeightRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployRadius2TextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, m_formulaBaseHeightRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployF1TextEdit()
{
    DeployFormula(this, ui->plainTextEditF1, ui->pushButtonGrowLengthF1, m_formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployF2TextEdit()
{
    DeployFormula(this, ui->plainTextEditF2, ui->pushButtonGrowLengthF2, m_formulaBaseHeightF2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployRotationAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditRotationAngle, ui->pushButtonGrowLengthRotationAngle,
                  m_formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::ShowDialog(bool click)
{
    if (not prepare)
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

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)

        const QSharedPointer<VPointF> center = data->GeometricObject<VPointF>(GetCenter());
        QLineF line = QLineF(static_cast<QPointF>(*center), scene->getScenePos());

        auto Angle = [line]()
        {
            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                QLineF correction = line;
                correction.setAngle(Visualization::CorrectAngle(correction.angle()));
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
            QLineF radius2Line(center->x(), center->y(), center->x(), center->y() - 100);
            QPointF p = VGObject::ClosestPoint(radius2Line, scene->getScenePos());
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

        if (m_stage == 3) // f2
        {
            SetF2(QString::number(Angle()));
            vis->RefreshGeometry();
            ++m_stage;
            return;
        }

        auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
        SCASSERT(path != nullptr)

        SetRotationAngle(QString::number(Angle() - path->StartingRotationAngle()));
    }

    FinishCreating();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogEllipticalArc::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if (type != SceneObject::Point)
    {
        return;
    }

    if (SetObject(id, ui->comboBoxBasePoint, QString()))
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
void DialogEllipticalArc::ShowVisualization()
{
    AddVisualization<VisToolEllipticalArc>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SaveData()
{
    m_radius1 = ui->plainTextEditRadius1->toPlainText();
    m_radius2 = ui->plainTextEditRadius2->toPlainText();
    m_f1 = ui->plainTextEditF1->toPlainText();
    m_f2 = ui->plainTextEditF2->toPlainText();
    m_rotationAngle = ui->plainTextEditRotationAngle->toPlainText();

    auto *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)

    path->SetCenterId(GetCenter());
    path->SetRadius1(m_radius1);
    path->SetRadius2(m_radius2);
    path->SetF1(m_f1);
    path->SetF2(m_f2);
    path->SetRotationAngle(m_rotationAngle);
    path->SetApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius1->blockSignals(true);
    ui->plainTextEditRadius2->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditF2->blockSignals(true);
    ui->plainTextEditRotationAngle->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::changeEvent(QEvent *event)
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
void DialogEllipticalArc::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());
    VEllipticalArc arc;
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
void DialogEllipticalArc::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArc::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetAliasSuffix(const QString &alias)
{
    m_originAliasSuffix = alias;
    ui->lineEditAlias->setText(m_originAliasSuffix);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogEllipticalArc::GetAliasSuffix() const -> QString
{
    return ui->lineEditAlias->text();
}
