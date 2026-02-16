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

#include "../../visualization/line/operation/vistoolflippingbyline.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "ui_dialogflippingbyline.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogFlippingByLine::DialogFlippingByLine(const VContainer *data,
                                           VAbstractPattern *doc,
                                           quint32 toolId,
                                           QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogFlippingByLine)
{
    ui->setupUi(this);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->comboBoxFirstLinePoint);
    FillComboBoxPoints(ui->comboBoxSecondLinePoint);
    FillComboBoxTypeLine(ui->comboBoxPenStyle,
                         OperationLineStylesPics(ui->comboBoxPenStyle->palette().color(QPalette::Base),
                                                 ui->comboBoxPenStyle->palette().color(QPalette::Text)),
                         TypeLineDefault);
    InitOperationColorPicker(ui->pushButtonColor,
                             VAbstractValApplication::VApp()->ValentinaSettings()->GetUserToolColors());
    ui->pushButtonColor->setUseNativeDialog(!VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

    connect(ui->lineEditVisibilityGroup, &QLineEdit::textChanged, this, &DialogFlippingByLine::GroupNameChanged);
    connect(ui->comboBoxFirstLinePoint, &QComboBox::currentTextChanged, this, &DialogFlippingByLine::PointChanged);
    connect(ui->comboBoxSecondLinePoint, &QComboBox::currentTextChanged, this, &DialogFlippingByLine::PointChanged);

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &DialogFlippingByLine::ShowSourceDetails);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &DialogFlippingByLine::NameChanged);
    connect(ui->comboBoxPenStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogFlippingByLine::PenStyleChanged);
    connect(ui->pushButtonColor, &VPE::QtColorPicker::colorChanged, this, &DialogFlippingByLine::ColorChanged);

    vis = new VisToolFlippingByLine(data);

    ui->tabWidget->setCurrentIndex(0);
    SetTabStopDistance(ui->plainTextEditToolNotes);
}

//---------------------------------------------------------------------------------------------------------------------
DialogFlippingByLine::~DialogFlippingByLine()
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetUserToolColors(ui->pushButtonColor->CustomColors());
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
    auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
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
    auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetSecondLinePointId(value);
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
        if (m_sourceObjects.isEmpty())
        {
            return;
        }

        stage1 = false;

        auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
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

        FillDefSourceNames(m_sourceObjects, data, "fl"_L1);
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
    return m_sourceObjects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SetSourceObjects(const QVector<SourceItem> &value)
{
    m_sourceObjects = value;
    FillSourceList();

    auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetObjects(SourceToObjects(m_sourceObjects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::CheckDependencyTreeComplete()
{
    m_dependencyReady = m_doc->IsPatternGraphComplete();
    ui->lineEditName->setEnabled(m_dependencyReady);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare && type == SceneObject::Point) // After first choose we ignore all objects
    {
        auto obj = std::find_if(m_sourceObjects.begin(),
                                m_sourceObjects.end(),
                                [id](const SourceItem &sItem) { return sItem.id == id; });
        switch (number)
        {
            case 0:
                if (obj != m_sourceObjects.end())
                {
                    emit ToolTip(tr("Select first line point that is not part of the list of objects"));
                    return;
                }

                if (SetObject(id, ui->comboBoxFirstLinePoint, tr("Select second line point")))
                {
                    number++;
                    auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
                    SCASSERT(operation != nullptr)
                    operation->SetFirstLinePointId(id);
                    operation->RefreshGeometry();
                }
                break;
            case 1:
                if (obj != m_sourceObjects.end())
                {
                    emit ToolTip(tr("Select second line point that is not part of the list of objects"));
                    return;
                }

                if (getCurrentObjectId(ui->comboBoxFirstLinePoint) != id &&
                    SetObject(id, ui->comboBoxSecondLinePoint, QString()) && flagError)
                {
                    number = 0;
                    prepare = true;

                    auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
                    SCASSERT(operation != nullptr)
                    operation->SetSecondLinePointId(id);
                    operation->RefreshGeometry();
                }
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogFlippingByLine::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    Q_UNUSED(tool)
    if (stage1)
    {
        auto obj = std::find_if(m_sourceObjects.begin(),
                                m_sourceObjects.end(),
                                [object](const SourceItem &sItem) { return sItem.id == object; });
        if (selected)
        {
            if (obj == m_sourceObjects.cend())
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
void DialogFlippingByLine::GroupNameChanged()
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
void DialogFlippingByLine::ShowSourceDetails(int row)
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
            ;
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
void DialogFlippingByLine::NameChanged(const QString &text)
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
        QColor const color = ui->pushButtonColor->currentColor();
        sourceItem.color = color.isValid() ? color.name() : ColorDefault;
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
    m_sourceObjects = SaveSourceObjects();

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            m_sourceObjects.append(sourceItem);
        }
    }

    auto *operation = qobject_cast<VisToolFlippingByLine *>(vis);
    SCASSERT(operation != nullptr)

    operation->SetObjects(SourceToObjects(m_sourceObjects));
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

    quint32 const id1 = getCurrentObjectId(ui->comboBoxFirstLinePoint);
    auto obj1 = std::find_if(m_sourceObjects.begin(),
                             m_sourceObjects.end(),
                             [id1](const SourceItem &sItem) { return sItem.id == id1; });

    quint32 const id2 = getCurrentObjectId(ui->comboBoxSecondLinePoint);
    auto obj2 = std::find_if(m_sourceObjects.begin(),
                             m_sourceObjects.end(),
                             [id2](const SourceItem &sItem) { return sItem.id == id2; });

    if (getCurrentObjectId(ui->comboBoxFirstLinePoint) == getCurrentObjectId(ui->comboBoxSecondLinePoint))
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->labelFirstLinePoint, color);
        ChangeColor(ui->labelSecondLinePoint, color);
        ui->labelStatus->setText(tr("Invalid line points"));
    }
    else if (obj1 != m_sourceObjects.end())
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->labelFirstLinePoint, color);
        ui->labelStatus->setText(tr("Invalid first line point"));
    }
    else if (obj2 != m_sourceObjects.end())
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
auto DialogFlippingByLine::SaveSourceObjects() const -> QVector<SourceItem>
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
    bool const ready = flagError && flagName && flagGroupName;

    if (ready)
    {
        ui->labelStatus->setText(tr("Ready"));
    }

    return ready;
}
