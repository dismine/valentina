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
    return Widgets().visibilityGroupLine->text();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetVisibilityGroupName(const QString &name) const
{
    Widgets().visibilityGroupLine->setText(name.isEmpty() ? tr("Rotation") : name);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::HasLinkedVisibilityGroup() const -> bool
{
    return Widgets().visibilityGroup->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetHasLinkedVisibilityGroup(bool linked) const
{
    Widgets().visibilityGroup->setChecked(linked);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetVisibilityGroupTags(const QStringList &tags) const
{
    Widgets().groupTags->setText(tags.join(", "));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::GetVisibilityGroupTags() const -> QStringList
{
    return Widgets().groupTags->text().split(',');
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetGroupCategories(const QStringList &categories)
{
    m_groupTags = categories;
    Widgets().groupTags->SetCompletion(m_groupTags);
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

    if (!m_sourceBaselineCaptured)
    {
        // First call populates the dialog from the tool; remember the committed object set (by recordId)
        // so later edits (add/remove) can be detected. Subsequent calls (add/remove/rename) must not move
        // the baseline.
        m_baselineSourceRecords.clear();
        m_baselineSourceRecords.reserve(value.size());
        for (const auto &item : value)
        {
            m_baselineSourceRecords.insert(item.recordId);
        }
        m_sourceBaselineCaptured = true;
    }

    FillSourceList();
    OnSourceObjectsSet();
    CheckState(); // adding/removing objects toggles the Apply button (see CheckState)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetDestinationObjects(const QVector<DestinationItem> &value)
{
    m_destination = value;
    CurrentObjectChanged(Widgets().sourceList->currentRow());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::CheckDependencyTreeComplete()
{
    m_dependencyReady = m_doc->IsPatternGraphComplete();
    Widgets().name->setEnabled(m_dependencyReady);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::SetNotes(const QString &notes) const
{
    Widgets().notes->setPlainText(notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::GetNotes() const -> QString
{
    return Widgets().notes->toPlainText();
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
            ChangeColor(Widgets().labelGroupName, errorColor);
            Widgets().labelStatus->setText(tr("Invalid group name"));
            CheckState();
            return;
        }

        flagGroupName = true;
        ChangeColor(Widgets().labelGroupName, OkColor(this));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ShowSourceDetails(int row)
{
    Widgets().name->setDisabled(true);
    Widgets().penStyle->setDisabled(true);
    Widgets().color->setDisabled(true);

    if (Widgets().sourceList->count() == 0)
    {
        return;
    }

    const auto *item = Widgets().sourceList->item(row);
    if (item == nullptr)
    {
        return;
    }

    const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
    if (data.GetGObject(sourceItem.id)->getType() == GOType::Point)
    {
        const QSignalBlocker blockerPenStyle(Widgets().penStyle);
        const QSignalBlocker blockerColor(Widgets().color);

        Widgets().penStyle->setCurrentIndex(-1);
        Widgets().color->setCurrentColor(QColor());
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

        SetValue(Widgets().penStyle, sourceItem.penStyle, TypeLineDefault);

        if (sourceItem.penStyle.isEmpty() || sourceItem.penStyle == TypeLineDefault)
        {
            int const index = Widgets().penStyle->currentIndex();
            Widgets().penStyle->setItemText(index, '<' + tr("Default") + '>');
        }

        QSignalBlocker blockerColor(Widgets().color);
        QColor const color(sourceItem.color);
        Widgets().color->setCurrentColor(color.isValid() ? color : ColorDefault);
        blockerColor.unblock();

        if (sourceItem.color.isEmpty() || sourceItem.color == ColorDefault)
        {
            const QSharedPointer<VAbstractCurve> curve = data.GeometricObject<VAbstractCurve>(sourceItem.id);
            Widgets().color->setDefaultColor(curve->GetColor());
        }

        Widgets().penStyle->setEnabled(true);
        Widgets().color->setEnabled(true);
    }

    const QSignalBlocker blockerName(Widgets().name);
    Widgets().name->setText(sourceItem.name);
    Widgets().name->setEnabled(m_dependencyReady);

    const QVector<SourceItem> sourceObjects = SaveSourceObjects();
    const QSet<QString> freeNames = FindFreeNames(m_sourceObjects, sourceObjects);
    const bool nameValid = IsValidSourceName(sourceItem.name, sourceItem.id, m_sourceObjects, &data, freeNames);
    ChangeColor(Widgets().labelName, nameValid ? OkColor(this) : errorColor);
    flagName = nameValid;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::CurrentObjectChanged(int row) const
{
    if (Widgets().sourceList->count() < 2)
    {
        Widgets().removeObject->setDisabled(true);
        return;
    }

    const auto *item = Widgets().sourceList->item(row);
    if (item == nullptr)
    {
        Widgets().removeObject->setDisabled(true);
        return;
    }

    const auto target = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
    const QUuid recordId = target.recordId;

    const auto it = std::find_if(m_destination.cbegin(),
                                 m_destination.cend(),
                                 [recordId](const DestinationItem &item) -> bool { return item.recordId == recordId; });

    // A source object that is not yet present in the destination list has not been committed to the
    // pattern, so no other tool can depend on it - it is always safe to remove. Only check safety for
    // objects that already exist in the pattern.
    if (it != m_destination.cend() && !IsSafeToRemoveGroupObject(it->id, m_doc))
    {
        Widgets().removeObject->setDisabled(true);
        return;
    }

    Widgets().removeObject->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::NameChanged(const QString &text)
{
    if (Widgets().sourceList->count() == 0)
    {
        return;
    }

    if (auto *item = Widgets().sourceList->currentItem())
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
        ChangeColor(Widgets().labelName, valid ? OkColor(this) : errorColor);

        if (!valid)
        {
            Widgets().labelStatus->setText(obj->getType() == GOType::Point ? tr("Invalid label") : tr("Invalid name"));
        }

        flagName = valid;
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::PenStyleChanged() const
{
    if (Widgets().sourceList->count() == 0)
    {
        return;
    }

    if (auto *item = Widgets().sourceList->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        sourceItem.penStyle = GetComboBoxCurrentData(Widgets().penStyle, TypeLineDefault);
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ObjectTypeChanged(int index) const
{
    Widgets().addObject->setDisabled(true);

    const QSignalBlocker blocker(Widgets().newObject);
    Widgets().newObject->clear();

    if (index == -1)
    {
        return;
    }

    auto const type = Widgets().objectType->itemData(index).value<GOType>();

    switch (type)
    {
        case GOType::Point:
            FillComboBoxPoints(Widgets().newObject);
            break;
        case GOType::Arc:
            FillComboBoxArcs(Widgets().newObject);
            break;
        case GOType::EllipticalArc:
            FillComboBoxEllipticalArcs(Widgets().newObject);
            break;
        case GOType::Spline:
        case GOType::CubicBezier:
            FillComboBoxSplines(Widgets().newObject);
            break;
        case GOType::SplinePath:
        case GOType::CubicBezierPath:
            FillComboBoxSplinesPath(Widgets().newObject);
            break;
        case GOType::PlaceLabel:
        case GOType::Unknown:
        default:
            return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::NewObjectChanged() const
{
    quint32 const id = getCurrentObjectId(Widgets().newObject);
    if (id == NULL_ID)
    {
        return;
    }

    const QVector<SourceItem> sourceObjects = SaveSourceObjects();
    const auto it = std::find_if(sourceObjects.cbegin(),
                                 sourceObjects.cend(),
                                 [id](const SourceItem &item) -> bool { return item.id == id; });
    Widgets().addObject->setDisabled(it != sourceObjects.cend());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::AddNewObject()
{
    quint32 const id = getCurrentObjectId(Widgets().newObject);
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
    Widgets().addObject->setDisabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::ColorChanged() const
{
    if (Widgets().sourceList->count() == 0)
    {
        return;
    }

    if (auto *item = Widgets().sourceList->currentItem())
    {
        auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
        QColor const color = Widgets().color->currentColor();
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
    if (Widgets().sourceList->count() < 2)
    {
        Widgets().removeObject->setDisabled(true);
        return;
    }

    const QListWidgetItem *item = Widgets().sourceList->currentItem();
    if (item == nullptr)
    {
        return;
    }

    const auto target = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
    const QUuid recordId = target.recordId;

    const auto it = std::find_if(m_destination.cbegin(),
                                 m_destination.cend(),
                                 [recordId](const DestinationItem &item) -> bool { return item.recordId == recordId; });

    // A source object that is not yet present in the destination list has not been committed to the
    // pattern, so no other tool can depend on it - it is always safe to remove. Only check safety for
    // objects that already exist in the pattern.
    if (it != m_destination.cend() && !IsSafeToRemoveGroupObject(it->id, m_doc))
    {
        Widgets().removeObject->setDisabled(true);
        return;
    }

    const int currentRow = Widgets().sourceList->row(item);
    delete Widgets().sourceList->takeItem(currentRow);

    SetSourceObjects(SaveSourceObjects());

    Widgets().removeObject->setDisabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::FillSourceList()
{
    QSignalBlocker blocker(Widgets().sourceList);

    Widgets().sourceList->clear();

    int row = -1;

    for (const auto &sourceItem : std::as_const(m_sourceObjects))
    {
        const QSharedPointer<VGObject> obj = data.GetGObject(sourceItem.id);
        auto *item = new QListWidgetItem(obj->ObjectName());
        item->setToolTip(obj->ObjectName());
        item->setData(Qt::UserRole, QVariant::fromValue(sourceItem));
        Widgets().sourceList->insertItem(++row, item);
    }

    blocker.unblock();

    if (Widgets().sourceList->count() > 0)
    {
        Widgets().sourceList->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::SaveSourceObjects() const -> QVector<SourceItem>
{
    QVector<SourceItem> objects;
    objects.reserve(Widgets().sourceList->count());

    for (int i = 0; i < Widgets().sourceList->count(); ++i)
    {
        if (const QListWidgetItem *item = Widgets().sourceList->item(i))
        {
            const auto sourceItem = qvariant_cast<SourceItem>(item->data(Qt::UserRole));
            objects.append(sourceItem);
        }
    }

    return objects;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogOperationTool::SourceObjectsChanged() const -> bool
{
    if (!m_sourceBaselineCaptured)
    {
        return false;
    }

    QSet<QUuid> current;
    current.reserve(m_sourceObjects.size());
    for (const auto &item : m_sourceObjects)
    {
        current.insert(item.recordId);
    }
    return current != m_baselineSourceRecords;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogOperationTool::CheckState()
{
    DialogTool::CheckState();

    if (bApply == nullptr)
    {
        return;
    }

    // Adding or removing operated objects forces a full reparse that recreates this tool and would
    // orphan the open dialog (breaking further Apply/Save). Disable Apply for such structural changes
    // while editing an existing tool - the user commits them with OK. Parameter-only edits (angle,
    // length, origin, rename) reparse lite and keep working with Apply.
    if (GetToolId() != NULL_ID && SourceObjectsChanged())
    {
        bApply->setEnabled(false);
        bApply->setToolTip(tr("Adding or removing objects can't be applied while the dialog stays open. "
                              "Use OK to confirm."));
    }
    else
    {
        bApply->setToolTip(QString());
    }
}
