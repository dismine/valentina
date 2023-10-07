/************************************************************************
 **
 **  @file   dialogpointofcontact.cpp
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

#include "dialogpointofcontact.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QTimer>
#include <QToolButton>

#include "../../visualization/line/vistoolpointofcontact.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogpointofcontact.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogPointOfContact create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogPointOfContact::DialogPointOfContact(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                           QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogPointOfContact),
    m_timerFormula(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    m_timerFormula->setSingleShot(true);
    connect(m_timerFormula, &QTimer::timeout, this, &DialogPointOfContact::EvalFormula);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    this->m_formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxFirstPoint);
    FillComboBoxPoints(ui->comboBoxSecondPoint);
    FillComboBoxPoints(ui->comboBoxCenter);

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogPointOfContact::FXRadius);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, m_pointName, this->data,
                                m_flagName);
                CheckState();
            });
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerFormula->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogPointOfContact::DeployFormulaTextEdit);
    connect(ui->comboBoxFirstPoint, &QComboBox::currentTextChanged, this, &DialogPointOfContact::PointNameChanged);
    connect(ui->comboBoxSecondPoint, &QComboBox::currentTextChanged, this, &DialogPointOfContact::PointNameChanged);
    connect(ui->comboBoxCenter, &QComboBox::currentTextChanged, this, &DialogPointOfContact::PointNameChanged);

    vis = new VisToolPointOfContact(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfContact::~DialogPointOfContact()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfContact::GetPointName() const -> QString
{
    return m_pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::PointNameChanged()
{
    QSet<quint32> set;
    set.insert(getCurrentObjectId(ui->comboBoxFirstPoint));
    set.insert(getCurrentObjectId(ui->comboBoxSecondPoint));
    set.insert(getCurrentObjectId(ui->comboBoxCenter));

    QColor color;
    if (set.size() != 3)
    {
        m_flagError = false;
        color = errorColor;
    }
    else
    {
        m_flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelFirstPoint, color);
    ChangeColor(ui->labelSecondPoint, color);
    ChangeColor(ui->labelArcCenter, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::FXRadius()
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
void DialogPointOfContact::EvalFormula()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormula->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormula;
    formulaData.labelResult = ui->labelResultCalculation;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

    Eval(formulaData, m_flagFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::ShowVisualization()
{
    AddVisualization<VisToolPointOfContact>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::DeployFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLength, m_formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::ShowDialog(bool click)
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

        QLineF line(static_cast<QPointF>(*center), scene->getScenePos());

        SetRadius(QString::number(FromPixel(line.length(), *data->GetPatternUnit())));
    }

    FinishCreating();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogPointOfContact::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if (type != SceneObject::Point)
    {
        return;
    }

    auto *line = qobject_cast<VisToolPointOfContact *>(vis);
    SCASSERT(line != nullptr)

    switch (m_number)
    {
        case 0:
            if (SetObject(id, ui->comboBoxFirstPoint, tr("Select second point of line")))
            {
                m_number++;
                line->VisualMode(id);
            }
            break;
        case 1:
            if (getCurrentObjectId(ui->comboBoxFirstPoint) != id)
            {
                if (SetObject(id, ui->comboBoxSecondPoint, tr("Select point of center of arc")))
                {
                    m_number++;
                    line->SetLineP2Id(id);
                    line->RefreshGeometry();
                }
            }
            break;
        case 2:
        {
            QSet<quint32> set;
            set.insert(getCurrentObjectId(ui->comboBoxFirstPoint));
            set.insert(getCurrentObjectId(ui->comboBoxSecondPoint));
            set.insert(id);

            if (set.size() == 3)
            {
                if (SetObject(id, ui->comboBoxCenter, QString()))
                {
                    auto *window =
                        qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
                    SCASSERT(window != nullptr)
                    connect(line, &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

                    line->SetRadiusId(id);
                    line->RefreshGeometry();
                    prepare = true;

                    if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                    {
                        FinishCreating();
                    }
                }
            }
        }
        break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::SaveData()
{
    m_pointName = ui->lineEditNamePoint->text();
    m_radius = ui->plainTextEditFormula->toPlainText();

    auto *line = qobject_cast<VisToolPointOfContact *>(vis);
    SCASSERT(line != nullptr)

    line->SetLineP1Id(GetFirstPoint());
    line->SetLineP2Id(GetSecondPoint());
    line->SetRadiusId(GetCenter());
    line->SetRadius(m_radius);
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::changeEvent(QEvent *event)
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
void DialogPointOfContact::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    ui->toolButtonExprRadius->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSecondPoint set id second point
 * @param value id
 */
void DialogPointOfContact::SetSecondPoint(quint32 value)
{
    setCurrentPointId(ui->comboBoxSecondPoint, value);

    auto *line = qobject_cast<VisToolPointOfContact *>(vis);
    SCASSERT(line != nullptr)
    line->SetLineP2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFirstPoint set id first point
 * @param value id
 */
void DialogPointOfContact::SetFirstPoint(quint32 value)
{
    setCurrentPointId(ui->comboBoxFirstPoint, value);

    auto *line = qobject_cast<VisToolPointOfContact *>(vis);
    SCASSERT(line != nullptr)
    line->SetLineP1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetCenter set id of center point
 * @param value id
 */
void DialogPointOfContact::SetCenter(quint32 value)
{
    setCurrentPointId(ui->comboBoxCenter, value);

    auto *line = qobject_cast<VisToolPointOfContact *>(vis);
    SCASSERT(line != nullptr)
    line->SetRadiusId(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setRadius set formula radius of arc
 * @param value formula
 */
void DialogPointOfContact::SetRadius(const QString &value)
{
    m_radius = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (m_radius.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(m_radius);

    auto *line = qobject_cast<VisToolPointOfContact *>(vis);
    SCASSERT(line != nullptr)
    line->SetRadius(m_radius);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name
 */
void DialogPointOfContact::SetPointName(const QString &value)
{
    m_pointName = value;
    ui->lineEditNamePoint->setText(m_pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getRadius return formula radius of arc
 * @return formula
 */
auto DialogPointOfContact::GetRadius() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(m_radius, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetCenter return id of center point
 * @return id
 */
auto DialogPointOfContact::GetCenter() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCenter);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFirstPoint return id first point
 * @return id
 */
auto DialogPointOfContact::GetFirstPoint() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxFirstPoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetSecondPoint return id second point
 * @return id
 */
auto DialogPointOfContact::GetSecondPoint() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxSecondPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfContact::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfContact::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
