/************************************************************************
 **
 **  @file   dialogcutarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2014
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

#include "dialogcutarc.h"

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

#include "../../visualization/path/vistoolcutarc.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/varc.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogcutarc.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogCutArc create dialog.
 * @param data container with data
 * @param parent parent widget
 */
DialogCutArc::DialogCutArc(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogCutArc),
    m_timerFormula(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    m_timerFormula->setSingleShot(true);
    connect(m_timerFormula, &QTimer::timeout, this, &DialogCutArc::EvalFormula);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    m_formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    InitOkCancelApply(ui);

    FillComboBoxArcs(ui->comboBoxArc);

    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogCutArc::FXLength);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, m_pointName, this->data,
                                m_flagName);
                CheckState();
            });
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerFormula->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogCutArc::DeployFormulaTextEdit);

    connect(ui->comboBoxArc, &QComboBox::currentTextChanged, this, &DialogCutArc::ArcChanged);

    connect(ui->lineEditAlias1, &QLineEdit::textEdited, this, &DialogCutArc::ValidateAlias);
    connect(ui->lineEditAlias2, &QLineEdit::textEdited, this, &DialogCutArc::ValidateAlias);

    vis = new VisToolCutArc(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::FXLength()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit length"));
    dialog->SetFormula(GetFormula());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormula(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::EvalFormula()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormula->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormula;
    formulaData.labelResult = ui->labelResultCalculation;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkZero = false;

    Eval(formulaData, m_flagFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::ShowVisualization()
{
    AddVisualization<VisToolCutArc>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::DeployFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLength, m_formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCutArc::~DialogCutArc()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCutArc::GetPointName() const -> QString
{
    return m_pointName;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogCutArc::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if (type == SceneObject::Arc)
    {
        if (SetObject(id, ui->comboBoxArc, QString()))
        {
            if (vis != nullptr)
            {
                vis->VisualMode(id);
            }
            prepare = true;

            auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
            SCASSERT(window != nullptr)
            connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

            if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
            {
                FinishCreating();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::SaveData()
{
    m_pointName = ui->lineEditNamePoint->text();
    m_formula = ui->plainTextEditFormula->toPlainText();

    auto *path = qobject_cast<VisToolCutArc *>(vis);
    SCASSERT(path != nullptr)

    path->SetArcId(getArcId());
    path->SetLength(m_formula);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::changeEvent(QEvent *event)
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
void DialogCutArc::ArcChanged()
{
    CurrentCurveLength(getArcId(), const_cast<VContainer *>(data));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::ValidateAlias()
{
    QRegularExpression rx(NameRegExp());

    VArc arc1;
    arc1.SetAliasSuffix(GetAliasSuffix1());

    VArc arc2;
    arc2.SetAliasSuffix(GetAliasSuffix2());

    if (not GetAliasSuffix1().isEmpty() &&
        (not rx.match(arc1.GetAlias()).hasMatch() ||
         (m_originAliasSuffix1 != GetAliasSuffix1() && not data->IsUnique(arc1.GetAlias())) ||
         arc1.GetAlias() == arc2.GetAlias()))
    {
        m_flagAlias1 = false;
        ChangeColor(ui->labelAlias1, errorColor);
    }
    else
    {
        m_flagAlias1 = true;
        ChangeColor(ui->labelAlias1, OkColor(this));
    }

    if (not GetAliasSuffix2().isEmpty() &&
        (not rx.match(arc2.GetAlias()).hasMatch() ||
         (m_originAliasSuffix2 != GetAliasSuffix2() && not data->IsUnique(arc2.GetAlias())) ||
         arc1.GetAlias() == arc2.GetAlias()))
    {
        m_flagAlias2 = false;
        ChangeColor(ui->labelAlias2, errorColor);
    }
    else
    {
        m_flagAlias2 = true;
        ChangeColor(ui->labelAlias2, OkColor(this));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();
    emit ToolTip(QString());
    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::InitIcons()
{
    QString resource = QStringLiteral("icon");

    ui->toolButtonExprLength->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label_4->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setArcId set id of arc
 * @param value id
 */
void DialogCutArc::setArcId(quint32 value)
{
    setCurrentArcId(ui->comboBoxArc, value);

    auto *path = qobject_cast<VisToolCutArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetArcId(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFormula set string with formula length
 * @param value string with formula
 */
void DialogCutArc::SetFormula(const QString &value)
{
    m_formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_formula.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(m_formula);

    auto *path = qobject_cast<VisToolCutArc *>(vis);
    SCASSERT(path != nullptr)
    path->SetLength(m_formula);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name point on arc
 * @param value name
 */
void DialogCutArc::SetPointName(const QString &value)
{
    m_pointName = value;
    ui->lineEditNamePoint->setText(m_pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormula return string with formula length
 * @return formula
 */
auto DialogCutArc::GetFormula() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getArcId return id of arc
 * @return id
 */
auto DialogCutArc::getArcId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxArc);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCutArc::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::SetAliasSuffix1(const QString &alias)
{
    m_originAliasSuffix1 = alias;
    ui->lineEditAlias1->setText(m_originAliasSuffix1);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCutArc::GetAliasSuffix1() const -> QString
{
    return ui->lineEditAlias1->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::SetAliasSuffix2(const QString &alias)
{
    m_originAliasSuffix2 = alias;
    ui->lineEditAlias2->setText(m_originAliasSuffix2);
    ValidateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogCutArc::GetAliasSuffix2() const -> QString
{
    return ui->lineEditAlias2->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::ShowDialog(bool click)
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

        const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(getArcId());
        QPointF p = arc->ClosestPoint(scene->getScenePos());
        qreal len = arc->GetLengthByPoint(p);
        if (len > 0)
        {
            SetFormula(QString::number(FromPixel(len, *data->GetPatternUnit())));
        }
    }

    FinishCreating();
}
