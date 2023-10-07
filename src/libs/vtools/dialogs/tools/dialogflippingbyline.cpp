/************************************************************************
 **
 **  @file   dialogflippingbyline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#include "dialogflippingbyline.h"

#include <QColor>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>
#include <QToolButton>
#include <new>

#include "../../visualization/line/operation/vistoolflippingbyline.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../../tools/drawTools/operation/vabstractoperation.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "ui_dialogflippingbyline.h"

//---------------------------------------------------------------------------------------------------------------------
DialogFlippingByLine::DialogFlippingByLine(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                           QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogFlippingByLine),
    stage1(true),
    m_suffix(),
    flagName(true),
    flagGroupName(true),
    flagError(false)
{
    ui->setupUi(this);

    ui->lineEditSuffix->setText(VAbstractValApplication::VApp()->getCurrentDocument()->GenerateSuffix());

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxFirstLinePoint);
    FillComboBoxPoints(ui->comboBoxSecondLinePoint);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         OperationLineStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                                 ui->comboBoxPenStyle->palette().color(QPalette::Text)),
                         TypeLineDefault);
    FillComboBoxLineColors(ui->comboBoxColor, VAbstractOperation::OperationColorsList());

    connect(ui->lineEditSuffix, &QLineEdit::textChanged, this, &DialogFlippingByLine::SuffixChanged);
    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogFlippingByLine::GroupNameChanged);
    connect(ui->comboBoxFirstLinePoint, &QComboBox::currentTextChanged, this, &DialogFlippingByLine::PointChanged);
    connect(ui->comboBoxSecondLinePoint, &QComboBox::currentTextChanged, this, &DialogFlippingByLine::PointChanged);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogFlippingByLine::ShowSourceDetails);
    connect(ui->lineEditAlias, &QLineEdit::textEdited, this, &DialogFlippingByLine::AliasChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogFlippingByLine::PenStyleChanged);
    connect(ui->comboBoxColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogFlippingByLine::ColorChanged);

    vis = new VisToolFlippingByLine(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogFlippingByLine::~DialogFlippingByLine()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetFirstLinePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxFirstLinePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetFirstLinePointId(quint32 value)
{
    ChangeCurrentData(ui->comboBoxFirstLinePoint, value);
    VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetFirstLinePointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetSecondLinePointId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxSecondLinePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetSecondLinePointId(quint32 value)
{
    ChangeCurrentData(ui->comboBoxSecondLinePoint, value);
    VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetSecondLinePointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetSuffix() const -> QString
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetSuffix(const QString &value)
{
    m_suffix = value;
    ui->lineEditSuffix->setText(value);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetVisibilityGroupName() const -> QString
{
    return ui->lineEditVisibilityGroup->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetVisibilityGroupName(const QString &name)
{
    ui->lineEditVisibilityGroup->setText(name.isEmpty() ? tr("Rotation") : name);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::HasLinkedVisibilityGroup() const -> bool
{
    return ui->groupBoxVisibilityGroup->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetHasLinkedVisibilityGroup(bool linked)
{
    ui->groupBoxVisibilityGroup->setChecked(linked);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetVisibilityGroupTags(const QStringList &tags)
{
    ui->lineEditGroupTags->setText(tags.join(", "));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetVisibilityGroupTags() const -> QStringList
{
    return ui->lineEditGroupTags->text().split(',');
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetGroupCategories(const QStringList &categories)
{
    m_groupTags = categories;
    ui->lineEditGroupTags->SetCompletion(m_groupTags);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (sourceObjects.isEmpty())
        {
            return;
        }

        stage1 = false;

        VMainGraphicsScene *scene =
            qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
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

        emit ToolTip(tr("Select first line point"));
    }
    else if (not stage1 && prepare && click)
    {
        setModal(true);
        emit ToolTip(QString());
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetSourceObjects() const -> QVector<SourceItem>
{
    return sourceObjects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetSourceObjects(const QVector<SourceItem> &value)
{
    sourceObjects = value;
    FillSourceList();

    VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare) // After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            auto obj = std::find_if(sourceObjects.begin(), sourceObjects.end(),
                                    [id](const SourceItem &sItem) { return sItem.id == id; });
            switch (number)
            {
                case 0:
                    if (obj != sourceObjects.end())
                    {
                        emit ToolTip(tr("Select first line point that is not part of the list of objects"));
                        return;
                    }

                    if (SetObject(id, ui->comboBoxFirstLinePoint, tr("Select second line point")))
                    {
                        number++;
                        VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
                        SCASSERT(operation != nullptr)
                        operation->SetFirstLinePointId(id);
                        operation->RefreshGeometry();
                    }
                    break;
                case 1:
                    if (obj != sourceObjects.end())
                    {
                        emit ToolTip(tr("Select second line point that is not part of the list of objects"));
                        return;
                    }

                    if (getCurrentObjectId(ui->comboBoxFirstLinePoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxSecondLinePoint, QString()))
                        {
                            if (flagError)
                            {
                                number = 0;
                                prepare = true;

                                VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
                                SCASSERT(operation != nullptr)
                                operation->SetSecondLinePointId(id);
                                operation->RefreshGeometry();
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SelectedObject(bool selected, quint32 object, quint32 tool)
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
            if (obj != sourceObjects.end())
            {
                sourceObjects.erase(obj);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SuffixChanged()
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
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

        if (m_suffix != suffix)
        {
            QRegularExpression rx(NameRegExp());
            const QStringList uniqueNames = data->AllUniqueNames();
            for (auto &uniqueName : uniqueNames)
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

        flagName = true;
        ChangeColor(ui->labelSuffix, OkColor(this));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::GroupNameChanged()
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
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
void DialogFlippingByLine::ShowSourceDetails(int row)
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

        auto SetValue = [](QComboBox *box, const QString &value, const QString &def)
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
            ;
            int index = ui->comboBoxPenStyle->currentIndex();
            ui->comboBoxPenStyle->setItemText(index, '<' + tr("Default") + '>');
        }

        SetValue(ui->comboBoxColor, sourceItem.color, ColorDefault);

        if (sourceItem.color.isEmpty() || sourceItem.color == ColorDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(sourceItem.id);
            int index = ui->comboBoxColor->currentIndex();
            ui->comboBoxColor->setItemIcon(index, LineColor(ui->comboBoxColor->palette().color(QPalette::Text),
                                                            ui->comboBoxColor->iconSize().height(), curve->GetColor()));
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
void DialogFlippingByLine::AliasChanged(const QString &text)
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
void DialogFlippingByLine::PenStyleChanged()
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
void DialogFlippingByLine::ColorChanged()
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
void DialogFlippingByLine::ShowVisualization()
{
    AddVisualization<VisToolFlippingByLine>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SaveData()
{
    m_suffix = ui->lineEditSuffix->text();

    sourceObjects.clear();
    sourceObjects.reserve(ui->listWidget->count());

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            sourceObjects.append(sourceItem);
        }
    }

    VisToolFlippingByLine *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)

    operation->SetObjects(SourceToObjects(sourceObjects));
    operation->SetFirstLinePointId(GetFirstLinePointId());
    operation->SetSecondLinePointId(GetSecondLinePointId());
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
void DialogFlippingByLine::PointChanged()
{
    QColor color = OkColor(this);
    flagError = true;
    ChangeColor(ui->labelFirstLinePoint, color);
    ChangeColor(ui->labelSecondLinePoint, color);

    quint32 id1 = getCurrentObjectId(ui->comboBoxFirstLinePoint);
    auto obj1 = std::find_if(sourceObjects.begin(), sourceObjects.end(),
                             [id1](const SourceItem &sItem) { return sItem.id == id1; });

    quint32 id2 = getCurrentObjectId(ui->comboBoxSecondLinePoint);
    auto obj2 = std::find_if(sourceObjects.begin(), sourceObjects.end(),
                             [id2](const SourceItem &sItem) { return sItem.id == id2; });

    if (getCurrentObjectId(ui->comboBoxFirstLinePoint) == getCurrentObjectId(ui->comboBoxSecondLinePoint))
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->labelFirstLinePoint, color);
        ChangeColor(ui->labelSecondLinePoint, color);
        ui->labelStatus->setText(tr("Invalid line points"));
    }
    else if (obj1 != sourceObjects.end())
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->labelFirstLinePoint, color);
        ui->labelStatus->setText(tr("Invalid first line point"));
    }
    else if (obj2 != sourceObjects.end())
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->labelSecondLinePoint, color);
        ui->labelStatus->setText(tr("Invalid second line point"));
    }

    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::FillSourceList()
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
void DialogFlippingByLine::ValidateSourceAliases()
{
    for (int i = 0; i < ui->listWidget->count(); ++i)
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
void DialogFlippingByLine::SetAliasValid(quint32 id, bool valid)
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
void DialogFlippingByLine::SetNotes(const QString &notes)
{
    ui->plainTextEditToolNotes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::GetNotes() const -> QString
{
    return ui->plainTextEditToolNotes->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogFlippingByLine::IsValid() const -> bool
{
    bool ready = flagError && flagName && flagGroupName && flagAlias;

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
