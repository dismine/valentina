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
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogcurveintersectaxis.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

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
    InitColorPicker(ui->pushButtonLineColor, VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonLineColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

    connect(ui->lineEditName1, &QLineEdit::textEdited, this, &DialogCurveIntersectAxis::ValidateCurveNames);
    connect(ui->lineEditName2, &QLineEdit::textEdited, this, &DialogCurveIntersectAxis::ValidateCurveNames);

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
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonLineColor->CustomColors());
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

    auto *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
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

    auto *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
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

    auto *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetCurveId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetLineColor() const -> QString
{
    return ui->pushButtonLineColor->currentColor().name();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetLineColor(const QString &value)
{
    ui->pushButtonLineColor->setCurrentColor(value);
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
            auto const line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

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
void DialogCurveIntersectAxis::CheckDependencyTreeComplete()
{
    const bool ready = m_doc->IsPatternGraphComplete();
    ui->lineEditNamePoint->setEnabled(ready);
    ui->lineEditName1->setEnabled(ready);
    ui->lineEditName2->setEnabled(ready);
    ui->lineEditAlias1->setEnabled(ready);
    ui->lineEditAlias2->setEnabled(ready);
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
                if ((type == SceneObject::Spline || type == SceneObject::Arc || type == SceneObject::ElArc ||
                     type == SceneObject::SplinePath) &&
                    SetObject(id, ui->comboBoxCurve, tr("Select axis point")))
                {
                    number++;
                    line->VisualMode(id);
                    auto *window =
                        qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
                    SCASSERT(window != nullptr)
                    connect(line, &VisToolCurveIntersectAxis::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
                }
                break;
            case (1):
                if (type == SceneObject::Point && SetObject(id, ui->comboBoxAxisPoint, QString()))
                {
                    line->setAxisPointId(id);
                    line->RefreshGeometry();
                    prepare = true;

                    SetName1(GenerateDefLeftSubName());
                    SetName2(GenerateDefRightSubName());

                    if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                    {
                        emit ToolTip(QString());

                        setModal(true);
                        show();
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
    Eval({.formula = ui->plainTextEditFormula->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditFormula,
          .labelResult = ui->labelResultCalculation,
          .postfix = degreeSymbol},
         flagFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::DeployAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLengthAngle, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::FXAngle()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
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

    auto *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
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
    QRegularExpression const rx(NameRegExp());

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(getCurveId());
    auto const [aliasFirst, aliasSecond] = SegmentAliases(curve->getType(), GetAliasSuffix1(), GetAliasSuffix2());
    auto const [name1, name2] = SegmentNames(curve->getType(), GetName1(), GetName2());

    // Validate first alias
    flagAlias1 = true;
    if (not GetAliasSuffix1().isEmpty())
    {
        if (not rx.match(aliasFirst).hasMatch())
        {
            flagAlias1 = false; // Invalid format
        }
        else if (originAliasSuffix1 != GetAliasSuffix1() && not data->IsUnique(aliasFirst))
        {
            flagAlias1 = false; // Not unique in data
        }
        else if (aliasFirst == aliasSecond || aliasFirst == name1 || aliasFirst == name2)
        {
            flagAlias1 = false; // Conflicts with other identifiers
        }
    }
    ChangeColor(ui->labelAlias1, flagAlias1 ? OkColor(this) : errorColor);

    // Validate second alias
    flagAlias2 = true;
    if (not GetAliasSuffix2().isEmpty())
    {
        if (not rx.match(aliasSecond).hasMatch())
        {
            flagAlias2 = false; // Invalid format
        }
        else if (originAliasSuffix2 != GetAliasSuffix2() && not data->IsUnique(aliasSecond))
        {
            flagAlias2 = false; // Not unique in data
        }
        else if (aliasSecond == aliasFirst || aliasSecond == name1 || aliasSecond == name2)
        {
            flagAlias2 = false; // Conflicts with other identifiers
        }
    }
    ChangeColor(ui->labelAlias2, flagAlias2 ? OkColor(this) : errorColor);

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ValidateCurveNames()
{
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(getCurveId());
    auto const [name1, name2] = SegmentNames(curve->getType(), GetName1(), GetName2());
    auto const [aliasFirst, aliasSecond] = SegmentAliases(curve->getType(), GetAliasSuffix1(), GetAliasSuffix2());

    QRegularExpression const rx(NameRegExp());

    // Validate first name
    m_flagCurveName1 = true;
    if (GetName1().isEmpty())
    {
        m_flagCurveName1 = false; // Name is required
    }
    else if (not rx.match(name1).hasMatch())
    {
        m_flagCurveName1 = false; // Invalid format
    }
    else if (m_originName1 != GetName1() && not data->IsUnique(name1))
    {
        m_flagCurveName1 = false; // Not unique in data
    }
    else if (name1 == name2 || name1 == aliasFirst || name1 == aliasSecond)
    {
        m_flagCurveName1 = false; // Conflicts with other identifiers
    }
    ChangeColor(ui->labelName1, m_flagCurveName1 ? OkColor(this) : errorColor);

    // Validate second name
    m_flagCurveName2 = true;
    if (GetName2().isEmpty())
    {
        m_flagCurveName2 = false; // Name is required
    }
    else if (not rx.match(name2).hasMatch())
    {
        m_flagCurveName2 = false; // Invalid format
    }
    else if (m_originName2 != GetName2() && not data->IsUnique(name2))
    {
        m_flagCurveName2 = false; // Not unique in data
    }
    else if (name2 == name1 || name2 == aliasFirst || name2 == aliasSecond)
    {
        m_flagCurveName2 = false; // Conflicts with other identifiers
    }
    ChangeColor(ui->labelName2, m_flagCurveName2 ? OkColor(this) : errorColor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::InitIcons()
{
    auto const resource = QStringLiteral("icon");

    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label_3->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GenerateDefLeftSubName() const -> QString
{
    return GenerateDefSubCurveName(data, getCurveId(), "__ls"_L1, "LSubCurve"_L1, GetPointName());
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GenerateDefRightSubName() const -> QString
{
    return GenerateDefSubCurveName(data, getCurveId(), "__rs"_L1, "RSubCurve"_L1, GetPointName());
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
void DialogCurveIntersectAxis::SetName1(const QString &name)
{
    m_originName1 = name;
    ui->lineEditName1->setText(m_originName1);
    ValidateCurveNames();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetName1() const -> QString
{
    return ui->lineEditName1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetName2(const QString &name)
{
    m_originName2 = name;
    ui->lineEditName2->setText(m_originName2);
    ValidateCurveNames();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCurveIntersectAxis::GetName2() const -> QString
{
    return ui->lineEditName2->text();
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
