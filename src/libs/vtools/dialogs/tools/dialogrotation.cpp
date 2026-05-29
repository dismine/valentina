/************************************************************************
 **
 **  @file   dialogrotation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
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

#include "dialogrotation.h"

#include <QAbstractButton>
#include <QColor>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>
#include <QTimer>
#include <QToolButton>

#include "../../visualization/line/operation/vistoolrotation.h"
#include "../../visualization/visualization.h"
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
#include "../vpropertyexplorer/qtcolorpicker.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vlineedit.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "ui_dialogrotation.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::DialogRotation(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogOperationTool(data, doc, toolId, parent),
    ui(new Ui::DialogRotation),
    timerAngle(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    MakeFilterableComboBox(ui->comboBoxNewObject);

    this->formulaBaseHeightAngle = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    timerAngle->setSingleShot(true);
    connect(timerAngle, &QTimer::timeout, this, &DialogRotation::EvalAngle);

    InitOkCancelApply(ui);

    FillComboBoxObjectTypes(ui->comboBoxObjectType);

    FillComboBoxPoints(ui->comboBoxOriginPoint);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         OperationLineStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                                 ui->comboBoxPenStyle->palette().color(QPalette::Text)),
                         TypeLineDefault);
    InitOperationColorPicker(ui->pushButtonColor,
                             VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogOperationTool::GroupNameChanged);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogRotation::FXAngle);
    connect(ui->plainTextEditFormula,
            &QPlainTextEdit::textChanged,
            this,
            [this]() -> void { timerAngle->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogRotation::DeployAngleTextEdit);
    connect(ui->comboBoxOriginPoint, &QComboBox::currentTextChanged, this, &DialogRotation::PointChanged);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogOperationTool::ShowSourceDetails);
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogOperationTool::CurrentObjectChanged);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogOperationTool::NameChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogOperationTool::PenStyleChanged);
    connect(ui->pushButtonColor, &VPE::QtColorPicker::colorChanged, this, &DialogOperationTool::ColorChanged);
    connect(ui->toolButtonBulkRename, &QToolButton::clicked, this, &DialogOperationTool::BulkRename);

    connect(ui->comboBoxObjectType,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DialogOperationTool::ObjectTypeChanged);
    connect(ui->comboBoxNewObject,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DialogOperationTool::NewObjectChanged);
    connect(ui->toolButtonNewObject, &QPushButton::clicked, this, &DialogOperationTool::AddNewObject);
    connect(ui->toolButtonRemoveObject, &QPushButton::clicked, this, &DialogOperationTool::RemoveObject);

    ObjectTypeChanged(ui->comboBoxObjectType->currentIndex());

    vis = new VisToolRotation(&this->data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::~DialogRotation()
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::SourceListWidget() const -> QListWidget *
{
    return ui->listWidget;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::NameLineEdit() const -> QLineEdit *
{
    return ui->lineEditName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::LabelName() const -> QLabel *
{
    return ui->labelName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::LabelStatus() const -> QLabel *
{
    return ui->labelStatus;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::LabelGroupName() const -> QLabel *
{
    return ui->labelGroupName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::PenStyleComboBox() const -> QComboBox *
{
    return ui->comboBoxPenStyle;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::ColorButton() const -> VPE::QtColorPicker *
{
    return ui->pushButtonColor;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::ObjectTypeComboBox() const -> QComboBox *
{
    return ui->comboBoxObjectType;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::NewObjectComboBox() const -> QComboBox *
{
    return ui->comboBoxNewObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::AddObjectButton() const -> QAbstractButton *
{
    return ui->toolButtonNewObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::RemoveObjectButton() const -> QAbstractButton *
{
    return ui->toolButtonRemoveObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::NotesPlainTextEdit() const -> QPlainTextEdit *
{
    return ui->plainTextEditToolNotes;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::GroupTagsLineEdit() const -> VCompleterLineEdit *
{
    return ui->lineEditGroupTags;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::VisibilityGroupLineEdit() const -> QLineEdit *
{
    return ui->lineEditVisibilityGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::VisibilityGroupBox() const -> QGroupBox *
{
    return ui->groupBoxVisibilityGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::GetOrigPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxOriginPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetOrigPointId(quint32 value)
{
    ChangeCurrentData(ui->comboBoxOriginPoint, value);
    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::GetAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetAngle(const QString &value)
{
    formulaAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formulaAngle.length() > 80)
    {
        this->DeployAngleTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formulaAngle);

    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetAngle(formulaAngle);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ShowDialog(bool click)
{
    if (IsStage1() && not click)
    {
        if (SourceObjects().isEmpty())
        {
            return;
        }

        SetStage1(false);

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)
        operation->StartAction();

        scene->ToggleArcSegmentVisibility(false);
        scene->ToggleElArcSegmentVisibility(false);
        scene->ToggleSplineSegmentVisibility(false);
        scene->ToggleSplinePathSegmentVisibility(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        VAbstractValApplication::VApp()->getSceneView()->AllowRubberBand(false);

        FillDefSourceNames(SourceObjects(), &data, "r"_L1);
        FillSourceList();

        emit ToolTip(tr("Select origin point"));
    }
    else if (not IsStage1() && prepare && click)
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
        try
        {
            const QSharedPointer<VPointF> point = data.GeometricObject<VPointF>(GetOrigPointId());
            const auto line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

            // Radius of point circle, but little bigger. Need handle with hover sizes.
            if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene())) * 1.5)
            {
                return;
            }
        }
        catch (const VExceptionBadId &)
        {
            return;
        }

        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)

        SetAngle(operation->Angle()); // Show in dialog angle that a user choose
        setModal(true);
        emit ToolTip(QString());
        timerAngle->start();
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ChosenObject(quint32 id, const SceneObject &type)
{
    if (IsStage1())
    {
        if (auto obj = std::find_if(SourceObjects().begin(),
                                    SourceObjects().end(),
                                    [id](const SourceItem &sItem) { return sItem.id == id; });
            obj == SourceObjects().end())
        {
            SourceObjects().push_back(SourceItem{id});
        }
        else
        {
            SourceObjects().erase(obj);
        }

        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)
        operation->SetObjects(SourceToObjects(SourceObjects()));
        if (!VAbstractValApplication::VApp()->getCurrentScene()->items().contains(operation))
        {
            operation->VisualMode(NULL_ID);
        }
        else
        {
            operation->RefreshGeometry();
        }
    }
    else if (not prepare && type == SceneObject::Point) // After first choose we ignore all objects
    {
        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)

        auto obj = std::find_if(SourceObjects().begin(),
                                SourceObjects().end(),
                                [id](const SourceItem &sItem) { return sItem.id == id; });

        if (obj != SourceObjects().end())
        {
            if (SourceObjects().size() > 1)
            {
                // It's not really logical for a user that a center of rotation no need to select.
                // To fix this issue we just silently remove it from the list.
                SourceObjects().erase(obj);
                operation->SetObjects(SourceToObjects(SourceObjects()));
            }
            else
            {
                emit ToolTip(tr("This point cannot be origin point. Please, select another origin point"));
                return;
            }
        }

        if (SetObject(id, ui->comboBoxOriginPoint, QString()))
        {
            auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
            SCASSERT(window != nullptr)
            connect(operation, &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

            operation->SetOriginPointId(id);
            operation->RefreshGeometry();

            prepare = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::DeployAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::FXAngle()
{
    auto *dialog = new DialogEditWrongFormula(&data, toolId, this);
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
void DialogRotation::OnSourceObjectsSet()
{
    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(SourceObjects()));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ShowVisualization()
{
    AddVisualization<VisToolRotation>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SaveData()
{
    formulaAngle = ui->plainTextEditFormula->toPlainText();
    SourceObjects() = SaveSourceObjects();

    if (!vis.isNull())
    {
        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)

        operation->SetObjects(SourceToObjects(SourceObjects()));
        operation->SetOriginPointId(GetOrigPointId());
        operation->SetAngle(formulaAngle);
        operation->RefreshGeometry();
    }

    QStringList groupTags = ui->lineEditGroupTags->text().split(',');
    for (auto &tag : groupTags)
    {
        tag = tag.trimmed();
        if (not GroupTags().contains(tag))
        {
            GroupTags().append(tag);
        }
    }

    ui->lineEditGroupTags->SetCompletion(GroupTags());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::changeEvent(QEvent *event)
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
void DialogRotation::PointChanged()
{
    quint32 const id = getCurrentObjectId(ui->comboBoxOriginPoint);

    auto obj = std::find_if(SourceObjects().begin(),
                            SourceObjects().end(),
                            [id](const SourceItem &sItem) { return sItem.id == id; });

    QColor color;
    if (obj != SourceObjects().end())
    {
        SetFlagError(false);
        color = errorColor;
        ui->labelStatus->setText(tr("Invalid rotation point"));
    }
    else
    {
        SetFlagError(true);
        color = OkColor(this);
    }
    ChangeColor(ui->labelOriginPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label_2->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::EvalAngle()
{
    Eval({.formula = ui->plainTextEditFormula->toPlainText(),
          .variables = data.DataVariables(),
          .labelEditFormula = ui->labelEditAngle,
          .labelResult = ui->labelResultAngle,
          .postfix = degreeSymbol},
         flagAngle);

    if (not flagAngle)
    {
        ui->labelStatus->setText(tr("Invalid angle formula"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::IsValid() const -> bool
{
    bool const ready = flagAngle && IsFlagName() && IsFlagError() && IsFlagGroupName();

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
