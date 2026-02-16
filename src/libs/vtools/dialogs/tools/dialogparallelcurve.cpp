/************************************************************************
 **
 **  @file   dialogparallelcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 11, 2025
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
#include "dialogparallelcurve.h"
#include "../../visualization/path/vistoolparallelcurve.h"
#include "../../visualization/visualization.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogparallelcurve.h"

#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogParallelCurve::DialogParallelCurve(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogParallelCurve),
    m_timerWidth(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    this->m_formulaBaseHeightWidth = ui->plainTextEditWidth->height();

    ui->plainTextEditWidth->installEventFilter(this);

    m_timerWidth->setSingleShot(true);
    connect(m_timerWidth, &QTimer::timeout, this, &DialogParallelCurve::Width);

    InitOkCancelApply(ui);

    FillComboBoxCurves(ui->comboBoxCurve);

    InitColorPicker(ui->pushButtonColor, VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         CurvePenStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                            ui->comboBoxPenStyle->palette().color(QPalette::Text)));

    ui->doubleSpinBoxApproximationScale->setMaximum(maxCurveApproximationScale);

    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &DialogParallelCurve::FXWidth);

    connect(ui->plainTextEditWidth,
            &QPlainTextEdit::textChanged,
            this,
            [this]() -> void { m_timerWidth->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowLengthWidth, &QPushButton::clicked, this, &DialogParallelCurve::DeployWidthTextEdit);

    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogParallelCurve::ValidateName);
    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogParallelCurve::ValidateAlias);

    vis = new VisToolParallelCurve(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogParallelCurve::~DialogParallelCurve()
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetOriginCurveId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCurve);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetOriginCurveId(const quint32 &value)
{
    ChangeCurrentData(ui->comboBoxCurve, value);
    auto *path = qobject_cast<VisToolParallelCurve *>(vis);
    SCASSERT(path != nullptr)
    path->SetCurveId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetFormulaWidth() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_width, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetFormulaWidth(const QString &value)
{
    m_width = VAbstractApplication::VApp()
                  ->TrVars()
                  ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_width.length() > 80)
    {
        this->DeployWidthTextEdit();
    }
    ui->plainTextEditWidth->setPlainText(m_width);

    auto *path = qobject_cast<VisToolParallelCurve *>(vis);
    SCASSERT(path != nullptr)
    path->SetOffsetWidth(m_width);

    MoveCursorToEnd(ui->plainTextEditWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetPenStyle() const -> QString
{
    return GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetPenStyle(const QString &value)
{
    ChangeCurrentData(ui->comboBoxPenStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetColor() const -> QString
{
    return ui->pushButtonColor->currentColor().name();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetColor(const QString &value)
{
    ui->pushButtonColor->setCurrentColor(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetApproximationScale() const -> qreal
{
    return ui->doubleSpinBoxApproximationScale->value();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetApproximationScale(qreal value)
{
    ui->doubleSpinBoxApproximationScale->setValue(value);

    auto *path = qobject_cast<VisToolParallelCurve *>(vis);
    SCASSERT(path != nullptr)
    path->SetApproximationScale(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetName(const QString &name)
{
    m_originName = name;
    ui->lineEditName->setText(m_originName);
    ValidateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetName() const -> QString
{
    return ui->lineEditName->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SetAliasSuffix(const QString &alias)
{
    m_originAliasSuffix = alias;
    ui->lineEditAlias->setText(m_originAliasSuffix);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GetAliasSuffix() const -> QString
{
    return ui->lineEditAlias->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::ShowDialog(bool click)
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

        const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
        QPointF const p = curve->ClosestPoint(scene->getScenePos());

        auto const line = QLineF(p, scene->getScenePos());
        if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
        {
            return;
        }

        qreal len = line.length();
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            len = -len;
        }

        SetFormulaWidth(QString::number(FromPixel(len, *data->GetPatternUnit())));
    }

    FinishCreating();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::CheckDependencyTreeComplete()
{
    const bool ready = m_doc->IsPatternGraphComplete();
    ui->lineEditName->setEnabled(ready);
    ui->lineEditAlias->setEnabled(ready);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if ((type == SceneObject::Arc || type == SceneObject::ElArc || type == SceneObject::Spline
         || type == SceneObject::SplinePath)
        && SetObject(id, ui->comboBoxCurve, QString()))
    {
        if (vis != nullptr)
        {
            vis->VisualMode(id);
        }
        prepare = true;

        SetName(GenerateDefName());

        auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
        SCASSERT(window != nullptr)
        connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

        if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
        {
            FinishCreating();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::DeployWidthTextEdit()
{
    DeployFormula(this, ui->plainTextEditWidth, ui->pushButtonGrowLengthWidth, m_formulaBaseHeightWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::FXWidth()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit width to parallel curve"));
    dialog->SetFormula(GetFormulaWidth());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaWidth(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::ShowVisualization()
{
    AddVisualization<VisToolParallelCurve>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::SaveData()
{
    m_width = ui->plainTextEditWidth->toPlainText();

    auto *path = qobject_cast<VisToolParallelCurve *>(vis);
    SCASSERT(path != nullptr)

    path->SetCurveId(GetOriginCurveId());
    path->SetOffsetWidth(m_width);
    path->SetApproximationScale(ui->doubleSpinBoxApproximationScale->value());
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditWidth->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::changeEvent(QEvent *event)
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
void DialogParallelCurve::ValidateName()
{
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
    VSplinePath const splPath = curve->Offset(0, GetName());

    if (QRegularExpression const rx(NameRegExp()); GetName().isEmpty() || not rx.match(splPath.name()).hasMatch()
                                                   || (m_originName != GetName() && not data->IsUnique(splPath.name())))
    {
        m_flagName = false;
        ChangeColor(ui->labelName, errorColor);
    }
    else
    {
        m_flagName = true;
        ChangeColor(ui->labelName, OkColor(this));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::ValidateAlias()
{
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
    VSplinePath splPath = curve->Offset(0, GetName());

    splPath.SetAliasSuffix(GetAliasSuffix());
    if (QRegularExpression const rx(NameRegExp());
        not GetAliasSuffix().isEmpty()
        && (not rx.match(splPath.GetAlias()).hasMatch()
            || (m_originAliasSuffix != GetAliasSuffix() && not data->IsUnique(splPath.GetAlias()))))
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
void DialogParallelCurve::Width()
{
    Eval({.formula = ui->plainTextEditWidth->toPlainText(),
          .variables = data->DataVariables(),
          .labelEditFormula = ui->labelEditWidth,
          .labelResult = ui->labelResultWidth,
          .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true)},
         m_flagWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogParallelCurve::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    const auto fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const auto equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_10->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogParallelCurve::GenerateDefName() const -> QString
{
    auto GenerateName = [this](const QString &base) -> QString
    {
        VSplinePath path;
        qint32 num = 1;
        QString name;
        do
        {
            name = base + QString::number(num++);
            path.SetNameSuffix(name);

        } while (!data->IsUnique(path.name()));

        return name;
    };
    QSharedPointer<VAbstractCurve> const curve = data->GeometricObject<VAbstractCurve>(GetOriginCurveId());
    if (!curve->IsDerivative())
    {
        return GenerateName(curve->HeadlessName() + "__o"_L1);
    }

    return GenerateName("Curve"_L1 + offset_);
}
