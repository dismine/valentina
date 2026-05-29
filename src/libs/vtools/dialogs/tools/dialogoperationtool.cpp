/************************************************************************
 **
 **  @file   dialogoperationtool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "dialogoperationtool.h"

#include <QAbstractButton>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QSharedPointer>
#include <QStringList>

#include "../support/dialogbulkrename.h"
#include "../vpropertyexplorer/qtcolorpicker.h"
#include "vlineedit.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogOperationTool::DialogOperationTool(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                         QWidget *parent)
  : DialogTool(data, doc, toolId, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::GetVisibilityGroupName() const -> QString
{
    return VisibilityGroupLineEdit()->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetVisibilityGroupName(const QString &name)
{
    VisibilityGroupLineEdit()->setText(name.isEmpty() ? tr("Rotation") : name);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::HasLinkedVisibilityGroup() const -> bool
{
    return VisibilityGroupBox()->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetHasLinkedVisibilityGroup(bool linked)
{
    VisibilityGroupBox()->setChecked(linked);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetVisibilityGroupTags(const QStringList &tags)
{
    GroupTagsLineEdit()->setText(tags.join(", "));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::GetVisibilityGroupTags() const -> QStringList
{
    return GroupTagsLineEdit()->text().split(',');
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetGroupCategories(const QStringList &categories)
{
    m_groupTags = categories;
    GroupTagsLineEdit()->SetCompletion(m_groupTags);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::GetSourceObjects() const -> QVector<SourceItem>
{
    return m_sourceObjects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetSourceObjects(const QVector<SourceItem> &value)
{
    m_sourceObjects = value;
    FillSourceList();
    OnSourceObjectsSet();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetDestinationObjects(const QVector<DestinationItem> &value)
{
    m_destination = value;
    CurrentObjectChanged(SourceListWidget()->currentRow());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::CheckDependencyTreeComplete()
{
    m_dependencyReady = m_doc->IsPatternGraphComplete();
    NameLineEdit()->setEnabled(m_dependencyReady);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetNotes(const QString &notes)
{
    NotesPlainTextEdit()->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::GetNotes() const -> QString
{
    return NotesPlainTextEdit()->toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ClearSourceObjects()
{
    SetSourceObjects({});
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::GroupNameChanged()
{
    if (const auto *edit = qobject_cast<QLineEdit *>(sender()); edit)
    {
        if (edit->text().isEmpty())
        {
            flagGroupName = false;
            ChangeColor(LabelGroupName(), errorColor);
            LabelStatus()->setText(tr("Invalid group name"));
            CheckState();
            return;
        }

        flagGroupName = true;
        ChangeColor(LabelGroupName(), OkColor(this));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ShowSourceDetails(int row)
{
    NameLineEdit()->setDisabled(true);
    PenStyleComboBox()->setDisabled(true);
    ColorButton()->setDisabled(true);

    if (SourceListWidget()->count() == 0)
    {
        return;
    }

    const auto *item = SourceListWidget()->item(row);
    if (item == nullptr)
    {
        return;
    }

    const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
    if (data.GetGObject(sourceItem.id)->getType() == GOType::Point)
    {
        const QSignalBlocker blockerPenStyle(PenStyleComboBox());
        const QSignalBlocker blockerColor(ColorButton());

        PenStyleComboBox()->setCurrentIndex(-1);
        ColorButton()->setCurrentColor(QColor());
    }
    else
    {
        auto SetValue = [](QComboBox *box, const QString &value, const QString &def) -> void
        {
            const QSignalBlocker blocker(box);

            if (const int index = box->findData(value); index != -1)
            {
                box->setCurrentIndex(index);
            }
            else
            {
                box->setCurrentIndex(box->findData(def));
            }
        };

        SetValue(PenStyleComboBox(), sourceItem.penStyle, TypeLineDefault);

        if (sourceItem.penStyle.isEmpty() || sourceItem.penStyle == TypeLineDefault)
        {
            int const index = PenStyleComboBox()->currentIndex();
            PenStyleComboBox()->setItemText(index, '<' + tr("Default") + '>');
        }

        QSignalBlocker blockerColor(ColorButton());
        QColor const color(sourceItem.color);
        ColorButton()->setCurrentColor(color.isValid() ? color : ColorDefault);
        blockerColor.unblock();

        if (sourceItem.color.isEmpty() || sourceItem.color == ColorDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data.GeometricObject<VAbstractCurve>(sourceItem.id);
            ColorButton()->setDefaultColor(curve->GetColor());
        }

        PenStyleComboBox()->setEnabled(true);
        ColorButton()->setEnabled(true);
    }

    const QSignalBlocker blockerName(NameLineEdit());
    NameLineEdit()->setText(sourceItem.name);
    NameLineEdit()->setEnabled(m_dependencyReady);

    const QVector<SourceItem> sourceObjects = SaveSourceObjects();
    const QSet<QString> freeNames = FindFreeNames(m_sourceObjects, sourceObjects);
    const bool nameValid = IsValidSourceName(sourceItem.name, sourceItem.id, m_sourceObjects, &data, freeNames);
    ChangeColor(LabelName(), nameValid ? OkColor(this) : errorColor);
    flagName = nameValid;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::CurrentObjectChanged(int row)
{
    if (SourceListWidget()->count() < 2)
    {
        RemoveObjectButton()->setDisabled(true);
        return;
    }

    const auto *item = SourceListWidget()->item(row);
    if (item == nullptr)
    {
        RemoveObjectButton()->setDisabled(true);
        return;
    }

    const auto target = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
    const QUuid recordId = target.recordId;

    const auto it = std::find_if(m_destination.cbegin(),
                                 m_destination.cend(),
                                 [recordId](const DestinationItem &item) -> bool { return item.recordId == recordId; });

    if (const quint32 targetId = it != m_destination.cend() ? it->id : NULL_ID;
        !IsSafeToRemoveGroupObject(targetId, m_doc))
    {
        RemoveObjectButton()->setDisabled(true);
        return;
    }

    RemoveObjectButton()->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::NameChanged(const QString &text)
{
    if (SourceListWidget()->count() == 0)
    {
        return;
    }

    if (auto *item = SourceListWidget()->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));

        const QVector<SourceItem> objects = SaveSourceObjects();
        const QSet<QString> freeNames = FindFreeNames(m_sourceObjects, objects);
        const bool valid = IsValidSourceName(text, sourceItem.id, objects, &data, freeNames);

        if (valid)
        {
            sourceItem.name = text;
            item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
        }

        const QSharedPointer<VGObject> obj = data.GetGObject(sourceItem.id);
        ChangeColor(LabelName(), valid ? OkColor(this) : errorColor);

        if (!valid)
        {
            LabelStatus()->setText(obj->getType() == GOType::Point ? tr("Invalid label") : tr("Invalid name"));
        }

        flagName = valid;
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::PenStyleChanged()
{
    if (SourceListWidget()->count() == 0)
    {
        return;
    }

    if (auto *item = SourceListWidget()->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        sourceItem.penStyle = GetComboBoxCurrentData(PenStyleComboBox(), TypeLineDefault);
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ObjectTypeChanged(int index)
{
    AddObjectButton()->setDisabled(true);

    const QSignalBlocker blocker(NewObjectComboBox());
    NewObjectComboBox()->clear();

    if (index == -1)
    {
        return;
    }

    auto const type = ObjectTypeComboBox()->itemData(index).value<GOType>();

    switch (type)
    {
        case GOType::Point:
            FillComboBoxPoints(NewObjectComboBox());
            break;
        case GOType::Arc:
            FillComboBoxArcs(NewObjectComboBox());
            break;
        case GOType::EllipticalArc:
            FillComboBoxEllipticalArcs(NewObjectComboBox());
            break;
        case GOType::Spline:
        case GOType::CubicBezier:
            FillComboBoxSplines(NewObjectComboBox());
            break;
        case GOType::SplinePath:
        case GOType::CubicBezierPath:
            FillComboBoxSplinesPath(NewObjectComboBox());
            break;
        case GOType::PlaceLabel:
        case GOType::Unknown:
        default:
            return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::NewObjectChanged()
{
    quint32 const id = getCurrentObjectId(NewObjectComboBox());
    if (id == NULL_ID)
    {
        return;
    }

    const QVector<SourceItem> sourceObjects = SaveSourceObjects();
    const auto it = std::find_if(sourceObjects.cbegin(),
                                 sourceObjects.cend(),
                                 [id](const SourceItem &item) -> bool { return item.id == id; });
    AddObjectButton()->setDisabled(it != sourceObjects.cend());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::AddNewObject()
{
    quint32 const id = getCurrentObjectId(NewObjectComboBox());
    if (id == NULL_ID)
    {
        return;
    }

    QVector<SourceItem> sourceObjects = SaveSourceObjects();
    QSet<QString> occupiedNames;
    for (const auto &sourceItem : std::as_const(sourceObjects))
    {
        if (!sourceItem.name.isEmpty())
        {
            occupiedNames.insert(sourceItem.name);
        }
    }

    sourceObjects.append({.id = id, .name = GetDefSourceName(id, &data, "m"_L1, occupiedNames)});
    SetSourceObjects(sourceObjects);
    AddObjectButton()->setDisabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ColorChanged()
{
    if (SourceListWidget()->count() == 0)
    {
        return;
    }

    if (auto *item = SourceListWidget()->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        QColor const color = ColorButton()->currentColor();
        sourceItem.color = color.isValid() ? color.name() : ColorDefault;
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::BulkRename()
{
    if (DialogBulkRename dlg(SaveSourceObjects(), data, this); dlg.exec() == QDialog::Accepted && dlg.HasChanges())
    {
        SetSourceObjects(dlg.RenamedItems());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::RemoveObject()
{
    if (SourceListWidget()->count() < 2)
    {
        RemoveObjectButton()->setDisabled(true);
        return;
    }

    const QListWidgetItem *item = SourceListWidget()->currentItem();
    if (!item)
    {
        return;
    }

    const SourceItem target = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
    const QUuid recordId = target.recordId;

    const auto it = std::find_if(m_destination.cbegin(),
                                 m_destination.cend(),
                                 [recordId](const DestinationItem &item) -> bool { return item.recordId == recordId; });

    if (const quint32 targetId = it != m_destination.cend() ? it->id : NULL_ID;
        !IsSafeToRemoveGroupObject(targetId, m_doc))
    {
        RemoveObjectButton()->setDisabled(true);
        return;
    }

    const int currentRow = SourceListWidget()->row(item);
    delete SourceListWidget()->takeItem(currentRow);

    SetSourceObjects(SaveSourceObjects());

    RemoveObjectButton()->setDisabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::FillSourceList()
{
    QSignalBlocker blocker(SourceListWidget());

    SourceListWidget()->clear();

    int row = -1;

    for (const auto &sourceItem : std::as_const(m_sourceObjects))
    {
        const QSharedPointer<VGObject> obj = data.GetGObject(sourceItem.id);
        auto *item = new QListWidgetItem(obj->ObjectName());
        item->setToolTip(obj->ObjectName());
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
        SourceListWidget()->insertItem(++row, item);
    }

    blocker.unblock();

    if (SourceListWidget()->count() > 0)
    {
        SourceListWidget()->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::SaveSourceObjects() const -> QVector<SourceItem>
{
    QVector<SourceItem> objects;
    objects.reserve(SourceListWidget()->count());

    for (int i = 0; i < SourceListWidget()->count(); ++i)
    {
        if (const QListWidgetItem *item = SourceListWidget()->item(i))
        {
            const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            objects.append(sourceItem);
        }
    }

    return objects;
}
