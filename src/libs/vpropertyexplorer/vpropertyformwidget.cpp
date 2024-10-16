/************************************************************************
 **
 **  @file   vpropertyformwidget.cpp
 **  @author hedgeware <internal(at)hedgeware.net>
 **  @date
 **
 **  @brief
 **  @copyright
 **  All rights reserved. This program and the accompanying materials
 **  are made available under the terms of the GNU Lesser General Public License
 **  (LGPL) version 2.1 which accompanies this distribution, and is available at
 **  http://www.gnu.org/licenses/lgpl-2.1.html
 **
 **  This library is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 **  Lesser General Public License for more details.
 **
 *************************************************************************/

#include "vpropertyformwidget.h"

#include <QEvent>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLayout>
#include <QLayoutItem>
#include <QMargins>
#include <QPlainTextEdit>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "plugins/vwidgetproperty.h"
#include "vproperty.h"
#include "vpropertyformwidget_p.h"

VPE::VPropertyFormWidget::VPropertyFormWidget(const QString &title, const QString &description,
                                              const QList<VProperty *> &properties, QWidget *parent)
  : QGroupBox(title, parent),
    vproperty_d_ptr(new VPropertyFormWidgetPrivate(properties))
{
    VPE::VPropertyFormWidget::build();
    setToolTip(description);
    setWhatsThis(description);
}

VPE::VPropertyFormWidget::VPropertyFormWidget(VProperty *parent_property, QWidget *parent)
  : QGroupBox(parent),
    vproperty_d_ptr(new VPropertyFormWidgetPrivate())
{
    if (parent_property)
    {
        vproperty_d_ptr->Properties = parent_property->getChildren();
        VPE::VPropertyFormWidget::build();
        setTitle(parent_property->getName());
        setToolTip(parent_property->getDescription());
        setWhatsThis(parent_property->getDescription());
    }
}

VPE::VPropertyFormWidget::VPropertyFormWidget(VPropertyFormWidgetPrivate *d_pointer, QWidget *parent,
                                              const QString &title, const QString &description)
  : QGroupBox(title, parent),
    vproperty_d_ptr(d_pointer)
{
    VPE::VPropertyFormWidget::build();
    setToolTip(description);
    setWhatsThis(description);
}

VPE::VPropertyFormWidget::~VPropertyFormWidget()
{
    delete vproperty_d_ptr;
}

void VPE::VPropertyFormWidget::build()
{
    // Clear the old content, delete old widgets
    vproperty_d_ptr->EditorWidgets.clear();
    if (layout() != nullptr)
    {
        while (layout()->count() > 0)
        {
            QLayoutItem *child = layout()->takeAt(0);
            if (child != nullptr)
            {
                delete child->widget();
                delete child;
            }
        }
        delete layout();
    }

    // Create new content
    if (vproperty_d_ptr->Properties.isEmpty())
    {
        return; //... only if there are properties
    }

    auto *tmpFormLayout = new QFormLayout(this);
    tmpFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    setLayout(tmpFormLayout);

    for (int i = 0; i < vproperty_d_ptr->Properties.count(); ++i)
    {
        // Get the current property
        VProperty *tmpProperty = vproperty_d_ptr->Properties.value(i, nullptr);
        if (!tmpProperty)
        {
            continue;
        }

        if (tmpProperty->getRowCount() > 0)
        {
            if (tmpProperty->propertyType() == Property::Complex)
            {
                buildEditor(tmpProperty, tmpFormLayout, Property::Complex);
                auto *group = new QWidget(this);
                tmpFormLayout->addRow(group);

                auto *subFormLayout = new QFormLayout(group);
                subFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
                QMargins margins = subFormLayout->contentsMargins();
                margins.setTop(0);
                margins.setLeft(18);
                margins.setRight(0);
                subFormLayout->setContentsMargins(margins);

                group->setLayout(subFormLayout);
                QList<VProperty *> children = tmpProperty->getChildren();
                for (int j = 0; j < children.size(); ++j)
                {
                    buildEditor(children[j], subFormLayout);
                    connect(children[j], &VProperty::childChanged, tmpProperty, &VProperty::ValueChildChanged,
                            Qt::UniqueConnection);
                    ++i;
                    vproperty_d_ptr->Properties.insert(i, children[j]);
                }
            }
            else
            {
                // Child properties, the property itself is not being added
                auto *tmpNewFormWidget = new VPropertyFormWidget(tmpProperty, this);
                tmpFormLayout->addRow(tmpNewFormWidget);
                vproperty_d_ptr->EditorWidgets.append(VPropertyFormWidgetPrivate::SEditorWidget(tmpNewFormWidget));
                tmpNewFormWidget->setCommitBehaviour(vproperty_d_ptr->UpdateEditors);
            }
        }
        else if (tmpProperty->type() == "widget")
        {
            auto *tmpWidgetProperty = static_cast<VWidgetProperty *>(tmpProperty);
            tmpFormLayout->addRow(tmpWidgetProperty->getWidget());
            vproperty_d_ptr->EditorWidgets.append(VPropertyFormWidgetPrivate::SEditorWidget(tmpWidgetProperty->getWidget()));
        }
        else
        {
            buildEditor(tmpProperty, tmpFormLayout);
        }
    }
}

void VPE::VPropertyFormWidget::buildEditor(VProperty *property, QFormLayout *formLayout, Property type)
{
    // Add property (no child properties)
    // Create the editor (if it doesn't work, create empty widget)
    QWidget *tmpEditor = property->createEditor(this, QStyleOptionViewItem(), nullptr);
    if (!tmpEditor)
    {
        tmpEditor = new QWidget(this);
    }

    // set tooltip and whats this
    tmpEditor->setToolTip(property->getDescription());
    tmpEditor->setWhatsThis(property->getDescription());

    // Install event filter
    tmpEditor->installEventFilter(this);

    // Set the editor data
    property->setEditorData(tmpEditor);

    // add new row
    if (type == Property::Complex)
    {
        QString const name = "<b>" + property->getName() + "</b>";
        formLayout->addRow(name, tmpEditor);
    }
    else
    {
        formLayout->addRow(property->getName(), tmpEditor);
    }

    vproperty_d_ptr->EditorWidgets.append(VPropertyFormWidgetPrivate::SEditorWidget(tmpEditor));
}

void VPE::VPropertyFormWidget::commitData()
{
    for (int i = 0; i < vproperty_d_ptr->Properties.count(); ++i)
    {
        commitData(i);
    }
}

void VPE::VPropertyFormWidget::loadData()
{
    for (int i = 0; i < vproperty_d_ptr->Properties.count(); ++i)
    {
        loadData(i);
    }
}

void VPE::VPropertyFormWidget::commitData(int row)
{
    if (row < 0 || row >= vproperty_d_ptr->EditorWidgets.count() || row >= vproperty_d_ptr->Properties.count())
    {
        return;
    }

    VPropertyFormWidgetPrivate::SEditorWidget &tmpEditorWidget = vproperty_d_ptr->EditorWidgets[row];
    VProperty *tmpProperty = vproperty_d_ptr->Properties[row];
    if (tmpEditorWidget.FormWidget)
    {
        tmpEditorWidget.FormWidget->commitData();
    }
    else if (tmpEditorWidget.Editor && tmpProperty)
    {
        QVariant const newValue = tmpProperty->getEditorData(tmpEditorWidget.Editor);
        QVariant const oldValue = tmpProperty->data(VProperty::DPC_Data, Qt::EditRole);
        if (oldValue != newValue)
        {
            VProperty *parent = tmpProperty->getParent();
            if (parent == nullptr || parent->propertyType() != Property::Complex)
            {
                tmpProperty->setValue(newValue);
                emit propertyDataSubmitted(tmpProperty);
            }
            else if (parent->propertyType() == Property::Complex)
            {
                tmpProperty->UpdateParent(newValue);
                emit propertyDataSubmitted(parent);
            }
        }
    }
}

void VPE::VPropertyFormWidget::loadData(int row)
{
    if (row < 0 || row >= vproperty_d_ptr->EditorWidgets.count() || row >= vproperty_d_ptr->Properties.count())
    {
        return;
    }

    VPropertyFormWidgetPrivate::SEditorWidget &tmpEditorWidget = vproperty_d_ptr->EditorWidgets[row];
    VProperty *tmpProperty = vproperty_d_ptr->Properties[row];
    if (tmpEditorWidget.FormWidget)
    {
        tmpEditorWidget.FormWidget->loadData();
    }
    else if (tmpEditorWidget.Editor && tmpProperty)
    {
        tmpProperty->setEditorData(tmpEditorWidget.Editor);
    }
}

void VPE::VPropertyFormWidget::setCommitBehaviour(bool auto_commit)
{
    vproperty_d_ptr->UpdateEditors = auto_commit;

    const QList<VPropertyFormWidget *> tmpChildFormWidgets = getChildPropertyFormWidgets();
    for (auto *tmpChild : tmpChildFormWidgets)
    {
        if (tmpChild)
        {
            tmpChild->setCommitBehaviour(auto_commit);
        }
    }
}

auto VPE::VPropertyFormWidget::getChildPropertyFormWidgets() const -> QList<VPE::VPropertyFormWidget *>
{
    QList<VPropertyFormWidget *> tmpResult;
    for (auto &tmpEditorWidget : vproperty_d_ptr->EditorWidgets)
    {
        if (tmpEditorWidget.FormWidget)
        {
            tmpResult.append(tmpEditorWidget.FormWidget);
        }
    }

    return tmpResult;
}

auto VPE::VPropertyFormWidget::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (!vproperty_d_ptr->UpdateEditors)
    {
        return false;
    }

    QWidget *editor = qobject_cast<QWidget *>(object);
    if (!editor)
    {
        return false;
    }

    if (event->type() == QEvent::KeyPress)
    {
        if (qobject_cast<QPlainTextEdit *>(editor))
        {
            switch (static_cast<QKeyEvent *>(event)->key())
            {
                case Qt::Key_Escape:
                    commitData(editor);
                    event->accept();
                    return true;
                case Qt::Key_Tab:
                case Qt::Key_Backtab:
                case Qt::Key_Enter:
                case Qt::Key_Return:
                default:
                    return false;
            }
        }
        else
        {
            switch (static_cast<QKeyEvent *>(event)->key())
            {
                case Qt::Key_Tab:
                case Qt::Key_Backtab:
                case Qt::Key_Enter:
                case Qt::Key_Return:
                case Qt::Key_Escape:
                    commitData(editor);
                    event->accept();
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }
    if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && editor->isWindow()))
    {
        commitData(editor);
        return false;
    }
    else if (event->type() == QEvent::ShortcutOverride)
    {
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape)
        {
            commitData(editor);
            event->accept();
            return true;
        }
    }
    else if (event->type() == MyCustomEventType)
    {
        commitData(editor);
        event->accept();
        return true;
    }
    else
    {
        return QGroupBox::eventFilter(object, event);
    }

    // Default:
    return false;
}

void VPE::VPropertyFormWidget::commitData(const QWidget *editor)
{
    if (!editor)
    {
        return;
    }

    for (int i = 0; i < vproperty_d_ptr->EditorWidgets.count(); ++i)
    {
        VPropertyFormWidgetPrivate::SEditorWidget const &tmpEditorWidget = vproperty_d_ptr->EditorWidgets[i];
        if (tmpEditorWidget.Editor == editor)
        {
            commitData(i);
        }
    }
}
