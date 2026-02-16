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

#include <QColor>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
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
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "ui_dialogmove.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogMove::DialogMove(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogMove),
    timerAngle(new QTimer(this)),
    timerRotationAngle(new QTimer(this)),
    timerLength(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

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

    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogMove::GroupNameChanged);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogMove::FXAngle);
    connect(ui->toolButtonExprRotationAngle, &QPushButton::clicked, this, &DialogMove::FXRotationAngle);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogMove::FXLength);
    connect(ui->plainTextEditAngle, &QPlainTextEdit::textChanged, this,
            [this]() { timerAngle->start(formulaTimerTimeout); });

    connect(ui->plainTextEditRotationAngle, &QPlainTextEdit::textChanged, this,
            [this]() { timerRotationAngle->start(formulaTimerTimeout); });

    connect(ui->plainTextEditLength, &QPlainTextEdit::textChanged, this,
            [this]() { timerLength->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowAngle, &QPushButton::clicked, this, &DialogMove::DeployAngleTextEdit);
    connect(ui->pushButtonGrowRotationAngle, &QPushButton::clicked, this, &DialogMove::DeployRotationAngleTextEdit);
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogMove::DeployLengthTextEdit);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogMove::ShowSourceDetails);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogMove::NameChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogMove::PenStyleChanged);
    connect(ui->pushButtonColor, &VPE::QtColorPicker::colorChanged, this, &DialogMove::ColorChanged);

    vis = new VisToolMove(data);

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
auto DialogMove::GetVisibilityGroupName() const -> QString
{
    return ui->lineEditVisibilityGroup->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetVisibilityGroupName(const QString &name)
{
    ui->lineEditVisibilityGroup->setText(name.isEmpty() ? tr("Rotation") : name);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::HasLinkedVisibilityGroup() const -> bool
{
    return ui->groupBoxVisibilityGroup->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetHasLinkedVisibilityGroup(bool linked)
{
    ui->groupBoxVisibilityGroup->setChecked(linked);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetVisibilityGroupTags(const QStringList &tags)
{
    ui->lineEditGroupTags->setText(tags.join(", "));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GetVisibilityGroupTags() const -> QStringList
{
    return ui->lineEditGroupTags->text().split(',');
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetGroupCategories(const QStringList &categories)
{
    m_groupTags = categories;
    ui->lineEditGroupTags->SetCompletion(m_groupTags);
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
        operation->SetObjects(SourceToObjects(m_sourceObjects));
        operation->VisualMode();

        auto *window = qobject_cast<VAbstractMainWindow *>(VAbstractValApplication::VApp()->getMainWindow());
        SCASSERT(window != nullptr)
        connect(operation, &VisToolMove::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

        scene->ToggleArcSelection(false);
        scene->ToggleElArcSelection(false);
        scene->ToggleSplineSelection(false);
        scene->ToggleSplinePathSelection(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        VAbstractValApplication::VApp()->getSceneView()->AllowRubberBand(false);

        FillDefSourceNames(m_sourceObjects, data, "m"_L1);
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
    if (not stage1 && stage2 && prepare && type == SceneObject::Point &&
        QGuiApplication::keyboardModifiers() == Qt::ControlModifier &&
        SetObject(id, ui->comboBoxRotationOriginPoint, QString())) // After first choose we ignore all objects
    {
        auto *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        operation->SetRotationOriginPointId(id);
        operation->RefreshGeometry();
        optionalRotationOrigin = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    Q_UNUSED(tool)
    if (stage1)
    {
        auto obj = std::find_if(m_sourceObjects.begin(),
                                m_sourceObjects.end(),
                                [object](const SourceItem &sItem) { return sItem.id == object; });
        if (selected)
        {
            if (obj == m_sourceObjects.end())
            {
                m_sourceObjects.append({.id = object});
            }
        }
        else
        {
            if (obj != m_sourceObjects.end())
            {
                m_sourceObjects.erase(obj);
            }
        }
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
void DialogMove::FXRotationAngle()
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
void DialogMove::FXLength()
{
    auto *dialog = new DialogEditWrongFormula(data, toolId, this);
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
void DialogMove::GroupNameChanged()
{
    if (const auto *edit = qobject_cast<QLineEdit *>(sender()); edit)
    {
        const QString name = edit->text();
        if (name.isEmpty())
        {
            flagGroupName = false;
            ChangeColor(ui->labelGroupName, errorColor);
            ui->labelStatus->setText(tr("Invalid group name"));
            CheckState();
            return;
        }

        flagGroupName = true;
        ChangeColor(ui->labelGroupName, OkColor(this));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ShowSourceDetails(int row)
{
    ui->lineEditName->setDisabled(true);
    ui->comboBoxPenStyle->setDisabled(true);
    ui->pushButtonColor->setDisabled(true);

    if (ui->listWidget->count() == 0)
    {
        return;
    }

    auto *item = ui->listWidget->item(row);
    if (item == nullptr)
    {
        return;
    }

    const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));

    const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);

    if (obj->getType() == GOType::Point)
    {
        const QSignalBlocker blockerPenStyle(ui->comboBoxPenStyle);
        const QSignalBlocker blockerColor(ui->pushButtonColor);

        ui->comboBoxPenStyle->setCurrentIndex(-1);
        ui->pushButtonColor->setCurrentColor(QColor());
    }
    else
    {
        auto SetValue = [](QComboBox *box, const QString &value, const QString &def)
        {
            const QSignalBlocker blocker(box);

            if (int index = box->findData(value); index != -1)
            {
                box->setCurrentIndex(index);
            }
            else
            {
                box->setCurrentIndex(box->findData(def));
            }
        };

        SetValue(ui->comboBoxPenStyle, sourceItem.penStyle, TypeLineDefault);

        if (sourceItem.penStyle.isEmpty() || sourceItem.penStyle == TypeLineDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(sourceItem.id);
            int const index = ui->comboBoxPenStyle->currentIndex();
            ui->comboBoxPenStyle->setItemText(index, '<' + tr("Default") + '>');
        }

        QSignalBlocker blockerColor(ui->pushButtonColor);
        QColor const color(sourceItem.color);
        ui->pushButtonColor->setCurrentColor(color.isValid() ? color : ColorDefault);
        blockerColor.unblock();

        if (sourceItem.color.isEmpty() || sourceItem.color == ColorDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(sourceItem.id);
            ui->pushButtonColor->setDefaultColor(curve->GetColor());
        }

        ui->comboBoxPenStyle->setEnabled(true);
        ui->pushButtonColor->setEnabled(true);
    }

    const QSignalBlocker blockerName(ui->lineEditName);
    ui->lineEditName->setText(sourceItem.name);
    ui->lineEditName->setEnabled(m_dependencyReady);

    const bool nameValid = IsValidSourceName(sourceItem.name, sourceItem.id, m_sourceObjects, data);
    ChangeColor(ui->labelName, nameValid ? OkColor(this) : errorColor);
    flagName = nameValid;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::NameChanged(const QString &text)
{
    if (ui->listWidget->count() == 0)
    {
        return;
    }

    if (auto *item = ui->listWidget->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));

        const QVector<SourceItem> objects = SaveSourceObjects();
        const bool valid = IsValidSourceName(text, sourceItem.id, objects, data);

        if (valid)
        {
            sourceItem.name = text;
            item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
        }

        const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);
        ChangeColor(ui->labelName, valid ? OkColor(this) : errorColor);

        if (!valid)
        {
            ui->labelStatus->setText(obj->getType() == GOType::Point ? tr("Invalid label") : tr("Invalid name"));
        }

        flagName = valid;
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::PenStyleChanged()
{
    if (ui->listWidget->count() == 0)
    {
        return;
    }

    if (auto *item = ui->listWidget->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        sourceItem.penStyle = GetComboBoxCurrentData(ui->comboBoxPenStyle, TypeLineDefault);
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ColorChanged()
{
    if (ui->listWidget->count() == 0)
    {
        return;
    }

    if (auto *item = ui->listWidget->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        QColor const color = ui->pushButtonColor->currentColor();
        sourceItem.color = color.isValid() ? color.name() : ColorDefault;
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
    }
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

    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)

    operation->SetObjects(SourceToObjects(m_sourceObjects));
    operation->SetAngle(formulaAngle);
    operation->SetLength(formulaLength);
    operation->SetRotationAngle(formulaRotationAngle);
    operation->SetRotationOriginPointId(GetRotationOrigPointId());
    operation->RefreshGeometry();

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
auto DialogMove::GetSourceObjects() const -> QVector<SourceItem>
{
    return m_sourceObjects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetSourceObjects(const QVector<SourceItem> &value)
{
    m_sourceObjects = value;
    FillSourceList();

    auto *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(m_sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::CheckDependencyTreeComplete()
{
    m_dependencyReady = m_doc->IsPatternGraphComplete();
    ui->lineEditName->setEnabled(m_dependencyReady);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::EvalAngle()
{
    Eval({.formula = ui->plainTextEditAngle->toPlainText(),
          .variables = data->DataVariables(),
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
          .variables = data->DataVariables(),
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
          .variables = data->DataVariables(),
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
void DialogMove::FillSourceList()
{
    QSignalBlocker blocker(ui->listWidget);

    ui->listWidget->clear();

    int row = -1;

    for (const auto &sourceItem : std::as_const(m_sourceObjects))
    {
        const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);
        auto *item = new QListWidgetItem(obj->ObjectName());
        item->setToolTip(obj->ObjectName());
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
        ui->listWidget->insertItem(++row, item);
    }

    blocker.unblock();

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
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
auto DialogMove::SaveSourceObjects() const -> QVector<SourceItem>
{
    QVector<SourceItem> objects;
    objects.reserve(ui->listWidget->count());

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            objects.append(sourceItem);
        }
    }

    return objects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogMove::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
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
