/************************************************************************
 **
 **  @file   dialogspline.cpp
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

#include "dialogspline.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QTimer>
#include <QToolButton>

#include "../../visualization/path/vistoolspline.h"
#include "../../visualization/visualization.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/def.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogspline.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogSpline create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogSpline::DialogSpline(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogSpline),
    timerAngle1(new QTimer(this)),
    timerAngle2(new QTimer(this)),
    timerLength1(new QTimer(this)),
    timerLength2(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    formulaBaseHeightAngle1 = ui->plainTextEditAngle1F->height();
    formulaBaseHeightAngle2 = ui->plainTextEditAngle2F->height();
    formulaBaseHeightLength1 = ui->plainTextEditLength1F->height();
    formulaBaseHeightLength2 = ui->plainTextEditLength2F->height();

    ui->plainTextEditAngle1F->installEventFilter(this);
    ui->plainTextEditAngle2F->installEventFilter(this);
    ui->plainTextEditLength1F->installEventFilter(this);
    ui->plainTextEditLength2F->installEventFilter(this);

    timerAngle1->setSingleShot(true);
    timerAngle2->setSingleShot(true);
    timerLength1->setSingleShot(true);
    timerLength2->setSingleShot(true);

    connect(timerAngle1, &QTimer::timeout, this, &DialogSpline::EvalAngle1);
    connect(timerAngle2, &QTimer::timeout, this, &DialogSpline::EvalAngle2);
    connect(timerLength1, &QTimer::timeout, this, &DialogSpline::EvalLength1);
    connect(timerLength2, &QTimer::timeout, this, &DialogSpline::EvalLength2);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxP1);
    FillComboBoxPoints(ui->comboBoxP4);
    FillComboBoxLineColors(ui->comboBoxColor);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         CurvePenStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                            ui->comboBoxPenStyle->palette().color(QPalette::Text)));

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    connect(ui->comboBoxP1, &QComboBox::currentTextChanged, this, &DialogSpline::PointNameChanged);
    connect(ui->comboBoxP4, &QComboBox::currentTextChanged, this, &DialogSpline::PointNameChanged);

    connect(ui->toolButtonExprAngle1, &QPushButton::clicked, this, &DialogSpline::FXAngle1);
    connect(ui->toolButtonExprAngle2, &QPushButton::clicked, this, &DialogSpline::FXAngle2);
    connect(ui->toolButtonExprLength1, &QPushButton::clicked, this, &DialogSpline::FXLength1);
    connect(ui->toolButtonExprLength2, &QPushButton::clicked, this, &DialogSpline::FXLength2);

    connect(ui->plainTextEditAngle1F, &QPlainTextEdit::textChanged, this,
            [this]() { timerAngle1->start(formulaTimerTimeout); });

    connect(ui->plainTextEditAngle2F, &QPlainTextEdit::textChanged, this,
            [this]() { timerAngle2->start(formulaTimerTimeout); });

    connect(ui->plainTextEditLength1F, &QPlainTextEdit::textChanged, this,
            [this]() { timerLength1->start(formulaTimerTimeout); });

    connect(ui->plainTextEditLength2F, &QPlainTextEdit::textChanged, this,
            [this]() { timerLength2->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowAngle1, &QPushButton::clicked, this, &DialogSpline::DeployAngle1TextEdit);
    connect(ui->pushButtonGrowAngle2, &QPushButton::clicked, this, &DialogSpline::DeployAngle2TextEdit);
    connect(ui->pushButtonGrowLength1, &QPushButton::clicked, this, &DialogSpline::DeployLength1TextEdit);
    connect(ui->pushButtonGrowLength2, &QPushButton::clicked, this, &DialogSpline::DeployLength2TextEdit);

    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogSpline::ValidateAlias);

    vis = new VisToolSpline(data);
    auto *path = qobject_cast<VisToolSpline *>(vis);
    SCASSERT(path != nullptr)

    auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
    SCASSERT(scene != nullptr)
    connect(scene, &VMainGraphicsScene::MouseLeftPressed, path, &VisToolSpline::MouseLeftPressed);
    connect(scene, &VMainGraphicsScene::MouseLeftReleased, path, &VisToolSpline::MouseLeftReleased);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogSpline::~DialogSpline()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogSpline::ChosenObject(quint32 id, const SceneObject &type)
{
    if (!prepare) // After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            auto *path = qobject_cast<VisToolSpline *>(vis);
            SCASSERT(path != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxP1, tr("Select last point of curve")))
                    {
                        ++number;
                        path->VisualMode(id);
                    }
                    break;
                case 1:
                {
                    if (getCurrentObjectId(ui->comboBoxP1) != id)
                    {
                        if (SetObject(id, ui->comboBoxP4, QString()))
                        {
                            ++number;

                            path->SetPoint4Id(id);
                            path->RefreshGeometry();
                            prepare = true;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::SaveData()
{
    const quint32 d = spl.GetDuplicate(); // Save previous value
    spl = CurrentSpline();

    newDuplicate <= -1 ? spl.SetDuplicate(d) : spl.SetDuplicate(static_cast<quint32>(newDuplicate));

    auto *path = qobject_cast<VisToolSpline *>(vis);
    SCASSERT(path != nullptr)

    path->SetPoint1Id(GetP1()->id());
    path->SetPoint4Id(GetP4()->id());
    path->SetAngle1(spl.GetStartAngle());
    path->SetAngle2(spl.GetEndAngle());
    path->SetKAsm1(spl.GetKasm1());
    path->SetKAsm2(spl.GetKasm2());
    path->SetKCurve(spl.GetKcurve());
    path->SetApproximationScale(spl.GetApproximationScale());
    path->SetMode(Mode::Show);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditAngle1F->blockSignals(true);
    ui->plainTextEditAngle2F->blockSignals(true);
    ui->plainTextEditLength1F->blockSignals(true);
    ui->plainTextEditLength2F->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::changeEvent(QEvent *event)
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
void DialogSpline::DeployAngle1TextEdit()
{
    DeployFormula(this, ui->plainTextEditAngle1F, ui->pushButtonGrowAngle1, formulaBaseHeightAngle1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::DeployAngle2TextEdit()
{
    DeployFormula(this, ui->plainTextEditAngle2F, ui->pushButtonGrowAngle2, formulaBaseHeightAngle2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::DeployLength1TextEdit()
{
    DeployFormula(this, ui->plainTextEditLength1F, ui->pushButtonGrowLength1, formulaBaseHeightLength1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::DeployLength2TextEdit()
{
    DeployFormula(this, ui->plainTextEditLength2F, ui->pushButtonGrowLength2, formulaBaseHeightLength2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::FXAngle1()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit first control point angle"));
    QString angle1F = VTranslateVars::TryFormulaFromUser(ui->plainTextEditAngle1F->toPlainText(),
                                                         VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    dialog->SetFormula(angle1F);
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        angle1F = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            dialog->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        // increase height if needed.
        if (angle1F.length() > 80)
        {
            DeployAngle1TextEdit();
        }
        ui->plainTextEditAngle1F->setPlainText(angle1F);
        MoveCursorToEnd(ui->plainTextEditAngle1F);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::FXAngle2()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit second control point angle"));
    QString angle2F = VTranslateVars::TryFormulaFromUser(ui->plainTextEditAngle2F->toPlainText(),
                                                         VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    dialog->SetFormula(angle2F);
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        angle2F = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            dialog->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        // increase height if needed.
        if (angle2F.length() > 80)
        {
            DeployAngle1TextEdit();
        }
        ui->plainTextEditAngle2F->setPlainText(angle2F);
        MoveCursorToEnd(ui->plainTextEditAngle2F);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::FXLength1()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit first control point length"));
    QString length1F = VTranslateVars::TryFormulaFromUser(ui->plainTextEditLength1F->toPlainText(),
                                                          VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    dialog->SetFormula(length1F);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        length1F = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            dialog->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        // increase height if needed.
        if (length1F.length() > 80)
        {
            DeployLength1TextEdit();
        }
        ui->plainTextEditLength1F->setPlainText(length1F);
        MoveCursorToEnd(ui->plainTextEditLength1F);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::FXLength2()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit second control point length"));
    QString length2F = VTranslateVars::TryFormulaFromUser(ui->plainTextEditLength2F->toPlainText(),
                                                          VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    dialog->SetFormula(length2F);
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        length2F = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            dialog->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
        // increase height if needed.
        if (length2F.length() > 80)
        {
            DeployLength2TextEdit();
        }
        ui->plainTextEditLength2F->setPlainText(length2F);
        MoveCursorToEnd(ui->plainTextEditLength2F);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSpline::GetP1() const -> const QSharedPointer<VPointF>
{
    return data->GeometricObject<VPointF>(getCurrentObjectId(ui->comboBoxP1));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSpline::GetP4() const -> const QSharedPointer<VPointF>
{
    return data->GeometricObject<VPointF>(getCurrentObjectId(ui->comboBoxP4));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::EvalAngle1()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditAngle1F->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditAngle1;
    formulaData.labelResult = ui->labelResultAngle1;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    Eval(formulaData, flagAngle1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::EvalAngle2()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditAngle2F->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditAngle2;
    formulaData.labelResult = ui->labelResultAngle2;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    Eval(formulaData, flagAngle2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::EvalLength1()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditLength1F->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditLength1;
    formulaData.labelResult = ui->labelResultLength1;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkZero = false;
    formulaData.checkLessThanZero = true;

    Eval(formulaData, flagLength1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::EvalLength2()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditLength2F->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditLength2;
    formulaData.labelResult = ui->labelResultLength2;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkZero = false;
    formulaData.checkLessThanZero = true;

    Eval(formulaData, flagLength2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());

    VSpline spline = spl;
    spline.SetAliasSuffix(ui->lineEditAlias->text());

    if (not ui->lineEditAlias->text().isEmpty() &&
        (not rx.match(spline.GetAlias()).hasMatch() ||
         (originAliasSuffix != ui->lineEditAlias->text() && not data->IsUnique(spline.GetAlias()))))
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
auto DialogSpline::CurrentSpline() const -> VSpline
{
    QString angle1F = ui->plainTextEditAngle1F->toPlainText();
    QString angle2F = ui->plainTextEditAngle2F->toPlainText();
    QString length1F = ui->plainTextEditLength1F->toPlainText();
    QString length2F = ui->plainTextEditLength2F->toPlainText();

    const QHash<QString, QSharedPointer<VInternalVariable>> *vars = data->DataVariables();

    const qreal angle1 = Visualization::FindValFromUser(angle1F, vars);
    const qreal angle2 = Visualization::FindValFromUser(angle2F, vars);
    const qreal length1 = Visualization::FindLengthFromUser(length1F, vars);
    const qreal length2 = Visualization::FindLengthFromUser(length2F, vars);

    const bool separator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();

    angle1F = VTranslateVars::TryFormulaFromUser(angle1F, separator);
    angle2F = VTranslateVars::TryFormulaFromUser(angle2F, separator);
    length1F = VTranslateVars::TryFormulaFromUser(length1F, separator);
    length2F = VTranslateVars::TryFormulaFromUser(length2F, separator);

    VSpline spline(*GetP1(), *GetP4(), angle1, angle1F, angle2, angle2F, length1, length1F, length2, length2F);
    spline.SetApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    spline.SetPenStyle(GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine));
    spline.SetColor(GetComboBoxCurrentData(ui->comboBoxColor, ColorBlack));
    spline.SetAliasSuffix(ui->lineEditAlias->text());

    return spline;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    const QString fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprLength1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprAngle1->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprLength2->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprAngle2->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const QString equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_5->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_8->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_9->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_10->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::PointNameChanged()
{
    QSet<quint32> set;
    set.insert(getCurrentObjectId(ui->comboBoxP1));
    set.insert(getCurrentObjectId(ui->comboBoxP4));

    QColor color;
    if (getCurrentObjectId(ui->comboBoxP1) == getCurrentObjectId(ui->comboBoxP4))
    {
        flagError = false;
        color = errorColor;

        ui->lineEditSplineName->setText(tr("Invalid spline"));
    }
    else
    {
        flagError = true;
        color = OkColor(this);

        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        if (getCurrentObjectId(ui->comboBoxP1) == spl.GetP1().id() &&
            getCurrentObjectId(ui->comboBoxP4) == spl.GetP4().id())
        {
            newDuplicate = -1;
            ui->lineEditSplineName->setText(trVars->VarToUser(spl.name()));
        }
        else
        {
            try
            {
                VSpline spline(*GetP1(), *GetP4(), spl.GetStartAngle(), spl.GetEndAngle(), spl.GetKasm1(),
                               spl.GetKasm2(), spl.GetKcurve());
                if (not data->IsUnique(spline.name()))
                {
                    newDuplicate = static_cast<qint32>(DNumber(spline.name()));
                    spline.SetDuplicate(static_cast<quint32>(newDuplicate));
                }
                ui->lineEditSplineName->setText(trVars->VarToUser(spline.name()));
            }
            catch (const VExceptionBadId &)
            {
                flagError = false;
                color = errorColor;
            }
        }
    }
    ChangeColor(ui->labelName, color);
    ChangeColor(ui->labelFirstPoint, color);
    ChangeColor(ui->labelSecondPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::ShowDialog(bool click)
{
    if (prepare && click)
    {
        auto *path = qobject_cast<VisToolSpline *>(vis);
        SCASSERT(path != nullptr)

        spl = VSpline(*GetP1(), path->GetP2(), path->GetP3(), *GetP4());

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        const QString angle1F = trVars->FormulaToUser(spl.GetStartAngleFormula(), osSeparator);
        const QString angle2F = trVars->FormulaToUser(spl.GetEndAngleFormula(), osSeparator);

        ui->plainTextEditAngle1F->setPlainText(angle1F);
        ui->plainTextEditAngle2F->setPlainText(angle2F);
        ui->plainTextEditLength1F->setPlainText(trVars->FormulaToUser(spl.GetC1LengthFormula(), osSeparator));
        ui->plainTextEditLength2F->setPlainText(trVars->FormulaToUser(spl.GetC2LengthFormula(), osSeparator));

        if (not data->IsUnique(spl.name()))
        {
            spl.SetDuplicate(DNumber(spl.name()));
        }

        ui->lineEditSplineName->setText(trVars->VarToUser(spl.name()));

        DialogAccepted();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::ShowVisualization()
{
    AddVisualization<VisToolSpline>();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSpline::GetSpline() const -> VSpline
{
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::SetSpline(const VSpline &spline)
{
    spl = spline;

    ui->doubleSpinBoxApproximationScale->setValue(spl.GetApproximationScale());
    ChangeCurrentData(ui->comboBoxColor, spl.GetColor());
    ChangeCurrentData(ui->comboBoxPenStyle, spl.GetPenStyle());

    setCurrentPointId(ui->comboBoxP1, spl.GetP1().id());
    setCurrentPointId(ui->comboBoxP4, spl.GetP4().id());

    const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
    const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

    const QString angle1F = trVars->FormulaToUser(spl.GetStartAngleFormula(), osSeparator);
    const QString angle2F = trVars->FormulaToUser(spl.GetEndAngleFormula(), osSeparator);

    ui->plainTextEditAngle1F->setPlainText(angle1F);
    ui->plainTextEditAngle2F->setPlainText(angle2F);

    const QString length1F = trVars->FormulaToUser(spl.GetC1LengthFormula(), osSeparator);
    const QString length2F = trVars->FormulaToUser(spl.GetC2LengthFormula(), osSeparator);

    ui->plainTextEditLength1F->setPlainText(length1F);
    ui->plainTextEditLength2F->setPlainText(length2F);
    ui->lineEditSplineName->setText(trVars->VarToUser(spl.name()));

    originAliasSuffix = spl.GetAliasSuffix();
    ui->lineEditAlias->setText(originAliasSuffix);
    ValidateAlias();

    auto *path = qobject_cast<VisToolSpline *>(vis);
    SCASSERT(path != nullptr)

    path->SetPoint1Id(spl.GetP1().id());
    path->SetPoint4Id(spl.GetP4().id());
    path->SetAngle1(spl.GetStartAngle());
    path->SetAngle2(spl.GetEndAngle());
    path->SetKAsm1(spl.GetKasm1());
    path->SetKAsm2(spl.GetKasm2());
    path->SetKCurve(spl.GetKcurve());
    path->SetApproximationScale(spl.GetApproximationScale());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSpline::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogSpline::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
