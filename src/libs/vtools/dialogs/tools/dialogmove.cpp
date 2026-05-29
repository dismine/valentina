/************************************************************************
 **
 **  @file   dialogmove.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 9, 2016
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

#include "dialogmove.h"

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

#include "../../visualization/line/operation/vistoolmove.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpropertyexplorer/qtcolorpicker.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vlineedit.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "ui_dialogmove.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogMove::DialogMove(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogOperationTool(data, doc, toolId, parent),
    ui(new Ui::DialogMove),
    timerAngle(new QTimer(this)),
    timerRotationAngle(new QTimer(this)),
    timerLength(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    MakeFilterableComboBox(ui->comboBoxNewObject);

    this->formulaBaseHeightAngle = ui->plainTextEditAngle->height();
    ui->plainTextEditAngle->installEventFilter(this);

    this->formulaBaseHeightRotationAngle = ui->plainTextEditRotationAngle->height();
    ui->plainTextEditRotationAngle->installEventFilter(this);

    this->formulaBaseHeightLength = ui->plainTextEditLength->height();
    ui->plainTextEditLength->installEventFilter(this);

    timerAngle->setSingleShot(true);
    connect(timerAngle, &QTimer::timeout, this, &DialogMove::EvalAngle);

    timerRotationAngle->setSingleShot(true);
    connect(timerRotationAngle, &QTimer::timeout, this, &DialogMove::EvalRotationAngle);

    timerLength->setSingleShot(true);
    connect(timerLength, &QTimer::timeout, this, &DialogMove::EvalLength);

    InitOkCancelApply(ui);

    FillComboBoxObjectTypes(ui->comboBoxObjectType);

    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         OperationLineStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                                 ui->comboBoxPenStyle->palette().color(QPalette::Text)),
                         TypeLineDefault);
    InitOperationColorPicker(ui->pushButtonColor,
                             VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());
    FillComboBoxPoints(ui->comboBoxRotationOriginPoint);

    {
        const QSignalBlocker blocker(ui->comboBoxRotationOriginPoint);
        ui->comboBoxRotationOriginPoint->addItem(tr("Center point"), NULL_ID);
    }

    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogOperationTool::GroupNameChanged);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogMove::FXAngle);
    connect(ui->toolButtonExprRotationAngle, &QPushButton::clicked, this, &DialogMove::FXRotationAngle);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogMove::FXLength);
    connect(ui->plainTextEditAngle,
            &QPlainTextEdit::textChanged,
            this,
            [this]() -> void { timerAngle->start(formulaTimerTimeout); });

    connect(ui->plainTextEditRotationAngle,
            &QPlainTextEdit::textChanged,
            this,
            [this]() -> void { timerRotationAngle->start(formulaTimerTimeout); });

    connect(ui->plainTextEditLength,
            &QPlainTextEdit::textChanged,
            this,
            [this]() -> void { timerLength->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowAngle, &QPushButton::clicked, this, &DialogMove::DeployAngleTextEdit);
    connect(ui->pushButtonGrowRotationAngle, &QPushButton::clicked, this, &DialogMove::DeployRotationAngleTextEdit);
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogMove::DeployLengthTextEdit);

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

    vis = new VisToolMove(&this->data);

    SetRotationOrigPointId(NULL_ID);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogMove::~DialogMove()
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::SourceListWidget() const -> QListWidget *
{
    return ui->listWidget;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::NameLineEdit() const -> QLineEdit *
{
    return ui->lineEditName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::LabelName() const -> QLabel *
{
    return ui->labelName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::LabelStatus() const -> QLabel *
{
    return ui->labelStatus;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::LabelGroupName() const -> QLabel *
{
    return ui->labelGroupName;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::PenStyleComboBox() const -> QComboBox *
{
    return ui->comboBoxPenStyle;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::ColorButton() const -> VPE::QtColorPicker *
{
    return ui->pushButtonColor;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::ObjectTypeComboBox() const -> QComboBox *
{
    return ui->comboBoxObjectType;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::NewObjectComboBox() const -> QComboBox *
{
    return ui->comboBoxNewObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::AddObjectButton() const -> QAbstractButton *
{
    return ui->toolButtonNewObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::RemoveObjectButton() const -> QAbstractButton *
{
    return ui->toolButtonRemoveObject;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::NotesPlainTextEdit() const -> QPlainTextEdit *
{
    return ui->plainTextEditToolNotes;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GroupTagsLineEdit() const -> VCompleterLineEdit *
{
    return ui->lineEditGroupTags;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::VisibilityGroupLineEdit() const -> QLineEdit *
{
    return ui->lineEditVisibilityGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::VisibilityGroupBox() const -> QGroupBox *
{
    return ui->groupBoxVisibilityGroup;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GetAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetAngle(const QString &value)
{
    formulaAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formulaAngle.length() > 80)
    {
        this->DeployAngleTextEdit();
    }
    ui->plainTextEditAngle->setPlainText(formulaAngle);

    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetAngle(formulaAngle);

    MoveCursorToEnd(ui->plainTextEditAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GetLength() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(formulaLength,
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetLength(const QString &value)
{
    formulaLength = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formulaLength.length() > 80)
    {
        this->DeployLengthTextEdit();
    }
    ui->plainTextEditLength->setPlainText(formulaLength);

    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetLength(formulaLength);

    MoveCursorToEnd(ui->plainTextEditLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GetRotationAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(formulaRotationAngle,
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetRotationAngle(const QString &value)
{
    formulaRotationAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formulaRotationAngle.length() > 80)
    {
        this->DeployAngleTextEdit();
    }
    ui->plainTextEditRotationAngle->setPlainText(formulaRotationAngle);

    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetRotationAngle(formulaRotationAngle);

    MoveCursorToEnd(ui->plainTextEditRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GetRotationOrigPointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxRotationOriginPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetRotationOrigPointId(const quint32 &value)
{
    ChangeCurrentData(ui->comboBoxRotationOriginPoint, value);
    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetRotationOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (m_sourceObjects.isEmpty())
        {
            return;
        }

        stage1 = false;
        prepare = true;

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)
        operation->StartAction();
        auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
        SCASSERT(window != nullptr)
        connect(operation, &VisToolMove::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

        scene->ToggleArcSegmentVisibility(false);
        scene->ToggleElArcSegmentVisibility(false);
        scene->ToggleSplineSegmentVisibility(false);
        scene->ToggleSplinePathSegmentVisibility(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        VAbstractValApplication::VApp()->getSceneView()->AllowRubberBand(false);

        FillDefSourceNames(m_sourceObjects, &data, "m"_L1);
        FillSourceList();
    }
    else if (not stage2 && not stage1 && prepare && click)
    {
        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        if (operation->LengthValue() > 0)
        {
            formulaAngle = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                operation->Angle(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            formulaLength = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
                operation->Length(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            operation->SetAngle(formulaAngle);
            operation->SetLength(formulaLength);
            operation->RefreshGeometry();
            emit ToolTip(operation->CurrentToolTip());
            stage2 = true;
        }
    }
    else if (not stage1 && stage2 && prepare && click)
    {
        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
        {
            if (not optionalRotationOrigin)
            {
                operation->SetRotationOriginPointId(NULL_ID);
                SetObject(NULL_ID, ui->comboBoxRotationOriginPoint, QString());
                operation->RefreshGeometry();
            }
            optionalRotationOrigin = false; // Handled, next click on empty field will disable selection
        }
        else
        {
            SetAngle(VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
            SetLength(VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                formulaLength, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
            SetRotationAngle(operation->RotationAngle());
            setModal(true);
            emit ToolTip(QString());
            timerAngle->start();
            timerRotationAngle->start();
            timerLength->start();
            show();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ChosenObject(quint32 id, const SceneObject &type)
{
    if (stage1)
    {
        if (auto obj = std::find_if(m_sourceObjects.begin(),
                                    m_sourceObjects.end(),
                                    [id](const SourceItem &sItem) { return sItem.id == id; });
            obj == m_sourceObjects.end())
        {
            m_sourceObjects.push_back(SourceItem{id});
        }
        else
        {
            m_sourceObjects.erase(obj);
        }

        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)
        operation->SetObjects(SourceToObjects(m_sourceObjects));
        if (!VAbstractValApplication::VApp()->getCurrentScene()->items().contains(operation))
        {
            operation->VisualMode(NULL_ID);
        }
        else
        {
            operation->RefreshGeometry();
        }
    }
    else if (stage2 && prepare && type == SceneObject::Point
             && QGuiApplication::keyboardModifiers() == Qt::ControlModifier
             && SetObject(id, ui->comboBoxRotationOriginPoint, QString())) // After first choose we ignore all objects
    {
        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        operation->SetRotationOriginPointId(id);
        operation->RefreshGeometry();
        optionalRotationOrigin = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::DeployAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditAngle, ui->pushButtonGrowAngle, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::DeployRotationAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditRotationAngle, ui->pushButtonGrowRotationAngle,
                  formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::DeployLengthTextEdit()
{
    DeployFormula(this, ui->plainTextEditLength, ui->pushButtonGrowLength, formulaBaseHeightLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::FXAngle()
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
void DialogMove::FXRotationAngle()
{
    auto *dialog = new DialogEditWrongFormula(&data, toolId, this);
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
void DialogMove::FXLength()
{
    auto *dialog = new DialogEditWrongFormula(&data, toolId, this);
    dialog->setWindowTitle(tr("Edit length"));
    dialog->SetFormula(GetLength());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetLength(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::OnSourceObjectsSet()
{
    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(m_sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ShowVisualization()
{
    AddVisualization<VisToolMove>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SaveData()
{
    formulaAngle = ui->plainTextEditAngle->toPlainText();
    formulaRotationAngle = ui->plainTextEditRotationAngle->toPlainText();
    formulaLength = ui->plainTextEditLength->toPlainText();
    m_sourceObjects = SaveSourceObjects();

    if (!vis.isNull())
    {
        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        operation->SetObjects(SourceToObjects(m_sourceObjects));
        operation->SetAngle(formulaAngle);
        operation->SetLength(formulaLength);
        operation->SetRotationAngle(formulaRotationAngle);
        operation->SetRotationOriginPointId(GetRotationOrigPointId());
        operation->RefreshGeometry();
    }

    QStringList groupTags = ui->lineEditGroupTags->text().split(',');
    for (auto &tag : groupTags)
    {
        tag = tag.trimmed();
        if (not m_groupTags.contains(tag))
        {
            m_groupTags.append(tag);
        }
    }

    ui->lineEditGroupTags->SetCompletion(m_groupTags);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditAngle->blockSignals(true);
    ui->plainTextEditRotationAngle->blockSignals(true);
    ui->plainTextEditLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::EvalAngle()
{
    Eval({.formula = ui->plainTextEditAngle->toPlainText(),
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
void DialogMove::EvalRotationAngle()
{
    Eval({.formula = ui->plainTextEditRotationAngle->toPlainText(),
          .variables = data.DataVariables(),
          .labelEditFormula = ui->labelEditRotationAngle,
          .labelResult = ui->labelResultRotationAngle,
          .postfix = degreeSymbol},
         flagRotationAngle);

    if (not flagRotationAngle)
    {
        ui->labelStatus->setText(tr("Invalid rotation angle formula"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::EvalLength()
{
    Eval({.formula = ui->plainTextEditLength->toPlainText(),
          .variables = data.DataVariables(),
          .labelEditFormula = ui->labelEditLength,
          .labelResult = ui->labelResultLength,
          .postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true)},
         flagLength);

    if (not flagLength)
    {
        ui->labelStatus->setText(tr("Invalid length formula"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    const auto fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprLength->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprRotationAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const auto equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_2->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_3->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_4->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::IsValid() const -> bool
{
    bool const ready = flagAngle && flagRotationAngle && flagLength && flagName && flagGroupName;

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
