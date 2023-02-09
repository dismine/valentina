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
#include <Qt>
#include <new>

#include "../../visualization/visualization.h"
#include "../../visualization/line/operation/vistoolrotation.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vwidgets/global.h"
#include "ui_dialogrotation.h"
#include "../../tools/drawTools/operation/vabstractoperation.h"

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::DialogRotation(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogRotation),
      timerAngle(new QTimer(this)),
      formulaAngle(),
      formulaBaseHeightAngle(0),
      stage1(true),
      m_suffix(),
      m_firstRelease(false),
      flagAngle(false),
      flagName(true),
      flagGroupName(true),
      flagError(false)
{
    ui->setupUi(this);

    this->formulaBaseHeightAngle = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    ui->lineEditSuffix->setText(VAbstractValApplication::VApp()->getCurrentDocument()->GenerateSuffix());

    timerAngle->setSingleShot(true);
    connect(timerAngle, &QTimer::timeout, this, &DialogRotation::EvalAngle);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxOriginPoint);
    FillComboBoxTypeLine(ui->comboBoxPenStyle, OperationLineStylesPics(), TypeLineDefault);
    FillComboBoxLineColors(ui->comboBoxColor, VAbstractOperation::OperationColorsList());

    connect(ui->lineEditSuffix, &QLineEdit::textChanged, this, &DialogRotation::SuffixChanged);
    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogRotation::GroupNameChanged);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogRotation::FXAngle);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, [this]()
    {
        timerAngle->start(formulaTimerTimeout);
    });
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogRotation::DeployAngleTextEdit);
    connect(ui->comboBoxOriginPoint, &QComboBox::currentTextChanged,
            this, &DialogRotation::PointChanged);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogRotation::ShowSourceDetails);
    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogRotation::AliasChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogRotation::PenStyleChanged);
    connect(ui->comboBoxColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogRotation::ColorChanged);

    vis = new VisToolRotation(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::~DialogRotation()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogRotation::GetOrigPointId() const
{
    return getCurrentObjectId(ui->comboBoxOriginPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetOrigPointId(quint32 value)
{
    ChangeCurrentData(ui->comboBoxOriginPoint, value);
    VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogRotation::GetAngle() const
{
    return VTranslateVars::TryFormulaFromUser(formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetAngle(const QString &value)
{
    formulaAngle = VAbstractApplication::VApp()->TrVars()
            ->FormulaToUser(value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
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
QString DialogRotation::GetSuffix() const
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetSuffix(const QString &value)
{
    m_suffix = value;
    ui->lineEditSuffix->setText(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogRotation::GetVisibilityGroupName() const
{
    return ui->lineEditVisibilityGroup->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetVisibilityGroupName(const QString &name)
{
    ui->lineEditVisibilityGroup->setText(name.isEmpty() ? tr("Rotation") : name);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogRotation::HasLinkedVisibilityGroup() const
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
QStringList DialogRotation::GetVisibilityGroupTags() const
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
        if (sourceObjects.isEmpty())
        {
            return;
        }

        stage1 = false;

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)
        operation->SetObjects(SourceToObjects(sourceObjects));
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
            const QLineF line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

            //Radius of point circle, but little bigger. Need handle with hover sizes.
            if (line.length() <= ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*1.5)
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

        SetAngle(operation->Angle());//Show in dialog angle that a user choose
        setModal(true);
        emit ToolTip(QString());
        timerAngle->start();
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<SourceItem> DialogRotation::GetSourceObjects() const
{
    return sourceObjects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetSourceObjects(const QVector<SourceItem> &value)
{
    sourceObjects = value;
    FillSourceList();

    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            auto *operation = qobject_cast<VisToolRotation *>(vis);
            SCASSERT(operation != nullptr)

            auto obj = std::find_if(sourceObjects.begin(), sourceObjects.end(),
                                    [id](const SourceItem &sItem) { return sItem.id == id; });

            if (obj != sourceObjects.cend())
            {
                if (sourceObjects.size() > 1)
                {
                    // It's not really logical for a user that a center of rotation no need to select.
                    // To fix this issue we just silently remove it from the list.
                    sourceObjects.erase(obj);
                    operation->SetObjects(SourceToObjects(sourceObjects));
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
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    Q_UNUSED(tool)
    if (stage1)
    {
        auto obj = std::find_if(sourceObjects.begin(), sourceObjects.end(),
                                [object](const SourceItem &sItem) { return sItem.id == object; });
        if (selected)
        {
            if (obj == sourceObjects.cend())
            {
                SourceItem item;
                item.id = object;
                sourceObjects.append(item);
            }
        }
        else
        {
            if (obj != sourceObjects.cend())
            {
                sourceObjects.erase(obj);
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
void DialogRotation::SuffixChanged()
{
    auto* edit = qobject_cast<QLineEdit*>(sender());
    if (edit)
    {
        const QString suffix = edit->text();
        if (suffix.isEmpty())
        {
            flagName = false;
            ChangeColor(ui->labelSuffix, errorColor);
            ui->labelStatus->setText(tr("Invalid suffix"));
            CheckState();
            return;
        }
        else
        {
            if (m_suffix != suffix)
            {
                QRegularExpression rx(NameRegExp());
                const QStringList uniqueNames = data->AllUniqueNames();
                for (const auto &uniqueName : uniqueNames)
                {
                    const QString name = uniqueName + suffix;
                    if (not rx.match(name).hasMatch() || not data->IsUnique(name))
                    {
                        flagName = false;
                        ChangeColor(ui->labelSuffix, errorColor);
                        ui->labelStatus->setText(tr("Invalid suffix"));
                        CheckState();
                        return;
                    }
                }
            }
        }

        flagName = true;
        ChangeColor(ui->labelSuffix, OkColor(this));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::GroupNameChanged()
{
    auto* edit = qobject_cast<QLineEdit*>(sender());
    if (edit)
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
    m_suffix = ui->lineEditSuffix->text();
    formulaAngle = ui->plainTextEditFormula->toPlainText();

    sourceObjects.clear();
    sourceObjects.reserve(ui->listWidget->count());

    for (int i=0; i<ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            sourceObjects.append(sourceItem);
        }
    }

    auto *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)

    operation->SetObjects(SourceToObjects(sourceObjects));
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
void DialogRotation::PointChanged()
{
    quint32 id = getCurrentObjectId(ui->comboBoxOriginPoint);

    auto obj = std::find_if(sourceObjects.begin(), sourceObjects.end(),
                            [id](const SourceItem &sItem) { return sItem.id == id; });

    QColor color;
    if (obj != sourceObjects.cend())
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
    ui->listWidget->blockSignals(true);

    ui->listWidget->clear();

    int row = -1;

    for (auto &sourceItem : sourceObjects)
    {
        const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);
        bool valid = SourceAliasValid(sourceItem, obj, data, OriginAlias(sourceItem.id, sourceObjects, obj));

        auto *item = new QListWidgetItem(valid ? obj->ObjectName() : obj->ObjectName() + '*');
        item->setToolTip(obj->ObjectName());
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
        ui->listWidget->insertItem(++row, item);
    }

    ui->listWidget->blockSignals(false);

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
    }

    ValidateSourceAliases();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ValidateSourceAliases()
{
    for (int i=0; i<ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));

            const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);

            if (not SourceAliasValid(sourceItem, obj, data, OriginAlias(sourceItem.id, sourceObjects, obj)))
            {
                flagAlias = false;
                ui->labelStatus->setText(obj->getType() == GOType::Point ? tr("Invalid label") : tr("Invalid alias"));
                SetAliasValid(sourceItem.id, false);
                CheckState();
                return;
            }

            SetAliasValid(sourceItem.id, true);
        }
    }

    flagAlias = true;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetAliasValid(quint32 id, bool valid)
{
    if (ui->listWidget->currentRow() != -1)
    {
        auto *item = ui->listWidget->item(ui->listWidget->currentRow());
        const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));

        if (id == sourceItem.id)
        {
            const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);
            item->setText(valid ? obj->ObjectName() : obj->ObjectName() + '*');

            ChangeColor(ui->labelAlias, valid ? OkColor(this) : errorColor);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::EvalAngle()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormula->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditAngle;
    formulaData.labelResult = ui->labelResultAngle;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    Eval(formulaData, flagAngle);

    if (not flagAngle)
    {
        ui->labelStatus->setText(tr("Invalid angle formula"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ShowSourceDetails(int row)
{
    ui->lineEditAlias->setDisabled(true);
    ui->comboBoxPenStyle->setDisabled(true);
    ui->comboBoxColor->setDisabled(true);

    if (ui->listWidget->count() == 0)
    {
        return;
    }

    const auto sourceItem = qvariant_cast<SourceItem>(ui->listWidget->item(row)->data(Qt::UserRole));

    const QSharedPointer<VGObject> obj = data->GetGObject(sourceItem.id);

    if (obj->getType() == GOType::Point)
    {
        ui->labelAlias->setText(tr("Label:"));

        ui->comboBoxPenStyle->blockSignals(true);
        ui->comboBoxColor->blockSignals(true);

        ui->comboBoxPenStyle->setCurrentIndex(-1);
        ui->comboBoxColor->setCurrentIndex(-1);

        ui->comboBoxPenStyle->blockSignals(false);
        ui->comboBoxColor->blockSignals(false);
    }
    else
    {
        ui->labelAlias->setText(tr("Alias:"));

        auto SetValue = [sourceItem](QComboBox *box, const QString &value, const QString &def)
        {
            box->blockSignals(true);

            int index = box->findData(value);
            if (index != -1)
            {
                box->setCurrentIndex(index);
            }
            else
            {
                index = box->findData(def);
                box->setCurrentIndex(index);
            }

            box->blockSignals(false);
        };

        SetValue(ui->comboBoxPenStyle, sourceItem.penStyle, TypeLineDefault);

        if (sourceItem.penStyle.isEmpty() || sourceItem.penStyle == TypeLineDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(sourceItem.id);
            int index = ui->comboBoxPenStyle->currentIndex();
            ui->comboBoxPenStyle->setItemText(index, '<' + tr("Default") + '>');
        }

        SetValue(ui->comboBoxColor, sourceItem.color, ColorDefault);

        if (sourceItem.color.isEmpty() || sourceItem.color == ColorDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(sourceItem.id);
            int index = ui->comboBoxColor->currentIndex();
            ui->comboBoxColor->setItemIcon(index, LineColor(ui->comboBoxColor->iconSize().height(), curve->GetColor()));
        }

        ui->comboBoxPenStyle->setEnabled(true);
        ui->comboBoxColor->setEnabled(true);
    }

    ui->lineEditAlias->blockSignals(true);
    ui->lineEditAlias->setText(sourceItem.alias);
    ui->lineEditAlias->setEnabled(true);
    ui->lineEditAlias->blockSignals(false);

    SetAliasValid(sourceItem.id,
                  SourceAliasValid(sourceItem, obj, data, OriginAlias(sourceItem.id, sourceObjects, obj)));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::AliasChanged(const QString &text)
{
    if (ui->listWidget->count() == 0)
    {
        return;
    }

    if (auto *item = ui->listWidget->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        sourceItem.alias = text;

        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));

        ValidateSourceAliases();
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
        sourceItem.color = GetComboBoxCurrentData(ui->comboBoxColor, ColorDefault);
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogRotation::GetNotes() const
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogRotation::IsValid() const
{
    bool ready = flagAngle && flagName && flagError && flagGroupName && flagAlias;

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
