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
#include "../vwidgets/global.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "ui_dialogrotation.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::DialogRotation(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogRotation),
    timerAngle(new QTimer(this))
{
    ui->setupUi(this);

    InitIcons();

    this->formulaBaseHeightAngle = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    timerAngle->setSingleShot(true);
    connect(timerAngle, &QTimer::timeout, this, &DialogRotation::EvalAngle);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxOriginPoint);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         OperationLineStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                                 ui->comboBoxPenStyle->palette().color(QPalette::Text)),
                         TypeLineDefault);
    InitOperationColorPicker(ui->pushButtonColor,
                             VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogRotation::GroupNameChanged);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogRotation::FXAngle);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this,
            [this]() { timerAngle->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogRotation::DeployAngleTextEdit);
    connect(ui->comboBoxOriginPoint, &QComboBox::currentTextChanged, this, &DialogRotation::PointChanged);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogRotation::ShowSourceDetails);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogRotation::NameChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogRotation::PenStyleChanged);
    connect(ui->pushButtonColor, &VPE::QtColorPicker::colorChanged, this, &DialogRotation::ColorChanged);

    vis = new VisToolRotation(data);

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
auto DialogRotation::GetVisibilityGroupName() const -> QString
{
    return ui->lineEditVisibilityGroup->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetVisibilityGroupName(const QString &name)
{
    ui->lineEditVisibilityGroup->setText(name.isEmpty() ? tr("Rotation") : name);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::HasLinkedVisibilityGroup() const -> bool
{
    return ui->groupBoxVisibilityGroup->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetHasLinkedVisibilityGroup(bool linked)
{
    ui->groupBoxVisibilityGroup->setChecked(linked);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetVisibilityGroupTags(const QStringList &tags)
{
    ui->lineEditGroupTags->setText(tags.join(", "));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::GetVisibilityGroupTags() const -> QStringList
{
    return ui->lineEditGroupTags->text().split(',');
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetGroupCategories(const QStringList &categories)
{
    m_groupTags = categories;
    ui->lineEditGroupTags->SetCompletion(m_groupTags);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (m_sourceObjects.isEmpty())
        {
            return;
        }

        stage1 = false;

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)
        operation->SetObjects(SourceToObjects(m_sourceObjects));
        operation->VisualMode();

        scene->ToggleArcSelection(false);
        scene->ToggleElArcSelection(false);
        scene->ToggleSplineSelection(false);
        scene->ToggleSplinePathSelection(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        VAbstractValApplication::VApp()->getSceneView()->AllowRubberBand(false);

        FillDefSourceNames(m_sourceObjects, data, "r"_L1);
        FillSourceList();

        emit ToolTip(tr("Select origin point"));
    }
    else if (not stage1 && prepare && click)
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
            const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(GetOrigPointId());
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
auto DialogRotation::GetSourceObjects() const -> QVector<SourceItem>
{
    return m_sourceObjects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetSourceObjects(const QVector<SourceItem> &value)
{
    m_sourceObjects = value;
    FillSourceList();

    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(m_sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::CheckDependencyTreeComplete()
{
    m_dependencyReady = m_doc->IsPatternGraphComplete();
    ui->lineEditName->setEnabled(m_dependencyReady);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare && type == SceneObject::Point) // After first choose we ignore all objects
    {
        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)

        auto obj = std::find_if(m_sourceObjects.begin(),
                                m_sourceObjects.end(),
                                [id](const SourceItem &sItem) { return sItem.id == id; });

        if (obj != m_sourceObjects.end())
        {
            if (m_sourceObjects.size() > 1)
            {
                // It's not really logical for a user that a center of rotation no need to select.
                // To fix this issue we just silently remove it from the list.
                m_sourceObjects.erase(obj);
                operation->SetObjects(SourceToObjects(m_sourceObjects));
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
void DialogRotation::SelectedObject(bool selected, quint32 object, quint32 tool)
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
void DialogRotation::DeployAngleTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::FXAngle()
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
void DialogRotation::GroupNameChanged()
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
void DialogRotation::ShowVisualization()
{
    AddVisualization<VisToolRotation>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SaveData()
{
    formulaAngle = ui->plainTextEditFormula->toPlainText();
    m_sourceObjects = SaveSourceObjects();

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            m_sourceObjects.append(sourceItem);
        }
    }

    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)

    operation->SetObjects(SourceToObjects(m_sourceObjects));
    operation->SetOriginPointId(GetOrigPointId());
    operation->SetAngle(formulaAngle);
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

    auto obj = std::find_if(m_sourceObjects.begin(),
                            m_sourceObjects.end(),
                            [id](const SourceItem &sItem) { return sItem.id == id; });

    QColor color;
    if (obj != m_sourceObjects.end())
    {
        flagError = false;
        color = errorColor;
        ui->labelStatus->setText(tr("Invalid rotation point"));
    }
    else
    {
        flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelOriginPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::FillSourceList()
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
void DialogRotation::InitIcons()
{
    const auto resource = QStringLiteral("icon");

    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, QStringLiteral("24x24/fx.png")));
    ui->label_2->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/equal.png")));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::SaveSourceObjects() const -> QVector<SourceItem>
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
void DialogRotation::EvalAngle()
{
    Eval({.formula = ui->plainTextEditFormula->toPlainText(),
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
void DialogRotation::ShowSourceDetails(int row)
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
void DialogRotation::NameChanged(const QString &text)
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
void DialogRotation::PenStyleChanged()
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
void DialogRotation::ColorChanged()
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
void DialogRotation::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogRotation::IsValid() const -> bool
{
    bool const ready = flagAngle && flagName && flagError && flagGroupName;

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
