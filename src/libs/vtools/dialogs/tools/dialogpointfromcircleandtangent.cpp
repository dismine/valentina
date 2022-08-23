/************************************************************************
 **
 **  @file   dialogpointfromcircleandtangent.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
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

#include "dialogpointfromcircleandtangent.h"

#include <climits>
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
#include <Qt>

#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/vcontainer.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/vistoolpointfromcircleandtangent.h"
#include "../support/dialogeditwrongformula.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogpointfromcircleandtangent.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPointFromCircleAndTangent::DialogPointFromCircleAndTangent(const VContainer *data, quint32 toolId,
                                                                 QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogPointFromCircleAndTangent),
      timerCircleRadius(nullptr),
      circleRadius(),
      formulaBaseHeightCircleRadius(0),
      pointName(),
      flagCircleRadius(false),
      flagName(true),
      flagError(true)
{
    ui->setupUi(this);

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(
                VAbstractValApplication::VApp()->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));

    this->formulaBaseHeightCircleRadius = ui->plainTextEditRadius->height();

    ui->plainTextEditRadius->installEventFilter(this);

    timerCircleRadius = new QTimer(this);
    connect(timerCircleRadius, &QTimer::timeout, this, &DialogPointFromCircleAndTangent::EvalCircleRadius);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxCircleCenter);
    FillComboBoxPoints(ui->comboBoxTangentPoint);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this, [this]()
    {
        CheckPointLabel(this, ui->lineEditNamePoint, ui->labelEditNamePoint, pointName, this->data, flagName);
        CheckState();
    });
    connect(ui->comboBoxCircleCenter, &QComboBox::currentTextChanged,
            this, &DialogPointFromCircleAndTangent::PointChanged);

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this,
            &DialogPointFromCircleAndTangent::FXCircleRadius);

    connect(ui->plainTextEditRadius, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerCircleRadius->start(formulaTimerTimeout);
    });

    connect(ui->pushButtonGrowRadius, &QPushButton::clicked, this,
            &DialogPointFromCircleAndTangent::DeployCircleRadiusTextEdit);

    vis = new VisToolPointFromCircleAndTangent(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointFromCircleAndTangent::~DialogPointFromCircleAndTangent()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogPointFromCircleAndTangent::GetPointName() const
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointFromCircleAndTangent::GetCircleCenterId() const
{
    return getCurrentObjectId(ui->comboBoxCircleCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SetCircleCenterId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxCircleCenter, value);

    VisToolPointFromCircleAndTangent *point = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
    SCASSERT(point != nullptr)
    point->SetCenterId(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogPointFromCircleAndTangent::GetCircleRadius() const
{
    return VTranslateVars::TryFormulaFromUser(ui->plainTextEditRadius->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SetCircleRadius(const QString &value)
{
    const QString formula = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployCircleRadiusTextEdit();
    }
    ui->plainTextEditRadius->setPlainText(formula);

    VisToolPointFromCircleAndTangent *point = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
    SCASSERT(point != nullptr)
    point->SetCRadius(formula);

    MoveCursorToEnd(ui->plainTextEditRadius);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointFromCircleAndTangent::GetTangentPointId() const
{
    return getCurrentObjectId(ui->comboBoxTangentPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SetTangentPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxTangentPoint, value);

    VisToolPointFromCircleAndTangent *point = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
    SCASSERT(point != nullptr)
    point->SetPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint DialogPointFromCircleAndTangent::GetCrossCirclesPoint() const
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SetCrossCirclesPoint(const CrossCirclesPoint &p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        VisToolPointFromCircleAndTangent *point = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
        SCASSERT(point != nullptr)
        point->SetCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::ShowDialog(bool click)
{
    if (not prepare)
    {
        return;
    }

    auto FinishCreating = [this]()
    {
        vis->SetMode(Mode::Show);
        vis->RefreshGeometry();

        emit ToolTip(QString());

        setModal(true);
        show();
    };

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
        const QSharedPointer<VPointF> center = data->GeometricObject<VPointF>(GetCircleCenterId());
        QLineF line = QLineF(static_cast<QPointF>(*center), scene->getScenePos());

        SetCircleRadius(QString::number(FromPixel(line.length(), *data->GetPatternUnit())));

        FinishCreating();
    }

    FinishCreating();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare)// After first choose we ignore all objects
    {
        return;
    }

    if (type == SceneObject::Point)
    {
        auto *point = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
        SCASSERT(point != nullptr)

        switch (number)
        {
            case 0:
                if (SetObject(id, ui->comboBoxTangentPoint, tr("Select a circle center")))
                {
                    number++;
                    point->VisualMode(id);
                }
                break;
            case 1:
                if (getCurrentObjectId(ui->comboBoxTangentPoint) != id)
                {
                    if (SetObject(id, ui->comboBoxCircleCenter, QString()))
                    {
                        auto *window = qobject_cast<VAbstractMainWindow *>(
                            VAbstractValApplication::VApp()->getMainWindow());
                        SCASSERT(window != nullptr)
                        connect(vis.data(), &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

                        number = 0;
                        point->SetCenterId(id);
                        point->RefreshGeometry();
                        prepare = true;
                    }
                }
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::PointChanged()
{
    QColor color;
    if (getCurrentObjectId(ui->comboBoxCircleCenter) == getCurrentObjectId(ui->comboBoxTangentPoint))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelCircleCenter, color);
    ChangeColor(ui->labelTangentPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::DeployCircleRadiusTextEdit()
{
    DeployFormula(this, ui->plainTextEditRadius, ui->pushButtonGrowRadius, formulaBaseHeightCircleRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::FXCircleRadius()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit radius"));
    dialog->SetFormula(GetCircleRadius());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCircleRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::EvalCircleRadius()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditRadius->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditRadius;
    formulaData.labelResult = ui->labelResultCircleRadius;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

    const qreal radius = Eval(formulaData, flagCircleRadius);

    if (radius < 0)
    {
        flagCircleRadius = false;
        ChangeColor(ui->labelEditRadius, errorColor);
        ui->labelResultCircleRadius->setText(tr("Error"));
        ui->labelResultCircleRadius->setToolTip(tr("Radius can't be negative"));

        DialogPointFromCircleAndTangent::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::ShowVisualization()
{
    AddVisualization<VisToolPointFromCircleAndTangent>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    VisToolPointFromCircleAndTangent *point = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
    SCASSERT(point != nullptr)

    point->SetPointId(GetTangentPointId());
    point->SetCenterId(GetCircleCenterId());
    point->SetCRadius(ui->plainTextEditRadius->toPlainText());
    point->SetCrossPoint(GetCrossCirclesPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromCircleAndTangent::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogPointFromCircleAndTangent::GetNotes() const
{
    return ui->plainTextEditToolNotes->toPlainText();
}
