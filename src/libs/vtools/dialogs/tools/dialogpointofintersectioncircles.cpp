/************************************************************************
 **
 **  @file   dialogpointofintersectioncircles.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2015
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

#include "dialogpointofintersectioncircles.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <climits>

#include "../../visualization/line/vistoolpointofintersectioncircles.h"
#include "../../visualization/visualization.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "ui_dialogpointofintersectioncircles.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionCircles::DialogPointOfIntersectionCircles(const VContainer *data, VAbstractPattern *doc,
                                                                   quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogPointOfIntersectionCircles),
    m_timerCircle1Radius(new QTimer(this)),
    m_timerCircle2Radius(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
        VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));

    this->m_formulaBaseHeightCircle1Radius = ui->plainTextEditCircle1Radius->height();
    this->m_formulaBaseHeightCircle2Radius = ui->plainTextEditCircle2Radius->height();

    ui->plainTextEditCircle1Radius->installEventFilter(this);
    ui->plainTextEditCircle2Radius->installEventFilter(this);

    m_timerCircle1Radius->setSingleShot(true);
    connect(m_timerCircle1Radius, &QTimer::timeout, this, &DialogPointOfIntersectionCircles::EvalCircle1Radius);

    m_timerCircle2Radius->setSingleShot(true);
    connect(m_timerCircle2Radius, &QTimer::timeout, this, &DialogPointOfIntersectionCircles::EvalCircle2Radius);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxCircle1Center);
    FillComboBoxPoints(ui->comboBoxCircle2Center);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this,
            [this]()
            {
                CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, m_pointName, this->data,
                                m_flagName);
                CheckState();
            });
    connect(ui->comboBoxCircle1Center, &QComboBox::currentTextChanged, this,
            &DialogPointOfIntersectionCircles::PointChanged);
    connect(ui->comboBoxCircle2Center, &QComboBox::currentTextChanged, this,
            &DialogPointOfIntersectionCircles::PointChanged);

    connect(ui->toolButtonExprCircle1Radius, &QPushButton::clicked, this,
            &DialogPointOfIntersectionCircles::FXCircle1Radius);
    connect(ui->toolButtonExprCircle2Radius, &QPushButton::clicked, this,
            &DialogPointOfIntersectionCircles::FXCircle2Radius);

    connect(ui->plainTextEditCircle1Radius, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerCircle1Radius->start(formulaTimerTimeout); });

    connect(ui->plainTextEditCircle2Radius, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerCircle2Radius->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowCircle1Radius, &QPushButton::clicked, this,
            &DialogPointOfIntersectionCircles::DeployCircle1RadiusTextEdit);
    connect(ui->pushButtonGrowCircle2Radius, &QPushButton::clicked, this,
            &DialogPointOfIntersectionCircles::DeployCircle2RadiusTextEdit);

    vis = new VisToolPointOfIntersectionCircles(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionCircles::~DialogPointOfIntersectionCircles()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetPointName() const -> QString
{
    return m_pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetPointName(const QString &value)
{
    m_pointName = value;
    ui->lineEditNamePoint->setText(m_pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetFirstCircleCenterId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCircle1Center);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetFirstCircleCenterId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxCircle1Center, value);

    auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
    SCASSERT(point != nullptr)
    point->SetCircle1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetSecondCircleCenterId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxCircle2Center);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetSecondCircleCenterId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxCircle2Center, value);

    auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
    SCASSERT(point != nullptr)
    point->SetCircle2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetFirstCircleRadius() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(ui->plainTextEditCircle1Radius->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetFirstCircleRadius(const QString &value)
{
    const QString formula = VAbstractValApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployCircle1RadiusTextEdit();
    }
    ui->plainTextEditCircle1Radius->setPlainText(formula);

    auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
    SCASSERT(point != nullptr)
    point->SetC1Radius(formula);

    MoveCursorToEnd(ui->plainTextEditCircle1Radius);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetSecondCircleRadius() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(ui->plainTextEditCircle2Radius->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetSecondCircleRadius(const QString &value)
{
    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployCircle2RadiusTextEdit();
    }
    ui->plainTextEditCircle2Radius->setPlainText(formula);

    auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
    SCASSERT(point != nullptr)
    point->SetC2Radius(formula);

    MoveCursorToEnd(ui->plainTextEditCircle2Radius);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetCrossCirclesPoint() const -> CrossCirclesPoint
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetCrossCirclesPoint(const CrossCirclesPoint &p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
        SCASSERT(point != nullptr)
        point->SetCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::ShowDialog(bool click)
{
    if (m_stage == 0 || m_stage == 2)
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

        QSharedPointer<VPointF> center =
            data->GeometricObject<VPointF>(m_stage == 1 ? GetFirstCircleCenterId() : GetSecondCircleCenterId());

        QLineF line(static_cast<QPointF>(*center), scene->getScenePos());

        if (m_stage == 1)
        {
            SetFirstCircleRadius(QString::number(FromPixel(line.length(), *data->GetPatternUnit())));
            emit ToolTip(tr("Select second circle center"));
            ++m_stage;
            m_firstRelease = false;
        }
        else
        {
            SetSecondCircleRadius(QString::number(FromPixel(line.length(), *data->GetPatternUnit())));
            ++m_stage;
        }

        if (m_stage > 2)
        {
            FinishCreating();
        }

        return;
    }

    FinishCreating();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare) // After first choose we ignore all objects
    {
        return;
    }

    if (type == SceneObject::Point)
    {
        auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
        SCASSERT(point != nullptr)

        switch (m_stage)
        {
            case 0:
                if (SetObject(id, ui->comboBoxCircle1Center, QString()))
                {
                    point->VisualMode(id);

                    if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                    {
                        m_stage = 2;
                        emit ToolTip(tr("Select second circle center"));
                    }
                    else
                    {
                        ++m_stage;
                        auto *window =
                            qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
                        SCASSERT(window != nullptr)
                        connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
                    }
                }
                break;
            case 2:
                if (getCurrentObjectId(ui->comboBoxCircle1Center) != id)
                {
                    if (SetObject(id, ui->comboBoxCircle2Center, QString()))
                    {
                        point->SetCircle2Id(id);
                        point->RefreshGeometry();
                        ++m_stage;
                        prepare = true;

                        if (not VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
                        {
                            FinishCreating();
                            return;
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
void DialogPointOfIntersectionCircles::PointChanged()
{
    QColor color;
    if (getCurrentObjectId(ui->comboBoxCircle1Center) == getCurrentObjectId(ui->comboBoxCircle2Center))
    {
        m_flagError = false;
        color = errorColor;
    }
    else
    {
        m_flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelCircle1Center, color);
    ChangeColor(ui->labelCircle1Center, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::DeployCircle1RadiusTextEdit()
{
    DeployFormula(this, ui->plainTextEditCircle1Radius, ui->pushButtonGrowCircle1Radius,
                  m_formulaBaseHeightCircle1Radius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::DeployCircle2RadiusTextEdit()
{
    DeployFormula(this, ui->plainTextEditCircle2Radius, ui->pushButtonGrowCircle2Radius,
                  m_formulaBaseHeightCircle2Radius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::FXCircle1Radius()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit first circle radius"));
    dialog->SetFormula(GetFirstCircleRadius());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFirstCircleRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::FXCircle2Radius()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit second circle radius"));
    dialog->SetFormula(GetSecondCircleRadius());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetSecondCircleRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::EvalCircle1Radius()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditCircle1Radius->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditCircle1Radius;
    formulaData.labelResult = ui->labelResultCircle1Radius;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

    const qreal radius = Eval(formulaData, m_flagCircle1Radius);

    if (radius < 0)
    {
        m_flagCircle2Radius = false;
        ChangeColor(ui->labelEditCircle1Radius, errorColor);
        ui->labelResultCircle1Radius->setText(tr("Error"));
        ui->labelResultCircle1Radius->setToolTip(tr("Radius can't be negative"));

        DialogPointOfIntersectionCircles::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::EvalCircle2Radius()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditCircle2Radius->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditCircle2Radius;
    formulaData.labelResult = ui->labelResultCircle2Radius;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

    const qreal radius = Eval(formulaData, m_flagCircle2Radius);

    if (radius < 0)
    {
        m_flagCircle2Radius = false;
        ChangeColor(ui->labelEditCircle2Radius, errorColor);
        ui->labelResultCircle2Radius->setText(tr("Error"));
        ui->labelResultCircle2Radius->setToolTip(tr("Radius can't be negative"));

        DialogPointOfIntersectionCircles::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::ShowVisualization()
{
    AddVisualization<VisToolPointOfIntersectionCircles>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SaveData()
{
    m_pointName = ui->lineEditNamePoint->text();

    auto *point = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
    SCASSERT(point != nullptr)

    point->SetCircle1Id(GetFirstCircleCenterId());
    point->SetCircle2Id(GetSecondCircleCenterId());
    point->SetC1Radius(ui->plainTextEditCircle1Radius->toPlainText());
    point->SetC2Radius(ui->plainTextEditCircle2Radius->toPlainText());
    point->SetCrossPoint(GetCrossCirclesPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditCircle1Radius->blockSignals(true);
    ui->plainTextEditCircle2Radius->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::changeEvent(QEvent *event)
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
void DialogPointOfIntersectionCircles::FinishCreating()
{
    vis->SetMode(Mode::Show);
    vis->RefreshGeometry();

    emit ToolTip(QString());

    setModal(true);
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    const QString fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprCircle1Radius->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprCircle2Radius->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const QString equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_2->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_3->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCircles::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPointOfIntersectionCircles::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}
