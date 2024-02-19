/************************************************************************
 **
 **  @file   vobjectproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
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

#include "vobjectproperty.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QLocale>
#include <QWidget>

#include "../vproperty_p.h"

VPE::VObjectProperty::VObjectProperty(const QString& name)
    : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QMetaType::UInt),
#else
                QVariant::UInt),
#endif
      objects()
{
    VProperty::d_ptr->VariantValue = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::UInt));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::UInt);
#endif
}

//! Get the data how it should be displayed
auto VPE::VObjectProperty::data(int column, int role) const -> QVariant
{
    if (objects.empty())
    {
        return QVariant();
    }

    QComboBox* tmpEditor = qobject_cast<QComboBox*>(VProperty::d_ptr->editor);

    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        return VProperty::d_ptr->VariantValue;
    }
    
    if (column == DPC_Data && Qt::EditRole == role)
    {
        return tmpEditor->currentIndex();
    }
    return VProperty::data(column, role);
}

//! Returns an editor widget, or NULL if it doesn't supply one
auto VPE::VObjectProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                        const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    QComboBox* tmpEditor = new QComboBox(parent);
    tmpEditor->clear();
    tmpEditor->setLocale(parent->locale());
    FillList(tmpEditor, objects);
    tmpEditor->setCurrentIndex(tmpEditor->findData(VProperty::d_ptr->VariantValue.toUInt()));
    connect(tmpEditor, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     &VObjectProperty::currentIndexChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

auto VPE::VObjectProperty::setEditorData(QWidget *editor) -> bool
{
    if (!editor)
    {
        return false;
    }

    QComboBox* tmpEditor = qobject_cast<QComboBox*>(editor);
    if (tmpEditor)
    {
        quint32 const objId = VProperty::d_ptr->VariantValue.toUInt();
        qint32 tmpIndex = tmpEditor->findData(objId);

        if (tmpIndex == -1)
        {
            tmpIndex = 0;
        }
        tmpEditor->blockSignals(true);
        tmpEditor->setCurrentIndex(tmpIndex);
        tmpEditor->blockSignals(false);
        return true;
    }

    return false;
}

//! Gets the data from the widget
auto VPE::VObjectProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const QComboBox* tmpEditor = qobject_cast<const QComboBox*>(editor);
    if (tmpEditor)
    {
        return tmpEditor->itemData(tmpEditor->currentIndex());
    }

    return QVariant(0);
}

//! Sets the objects list
// cppcheck-suppress unusedFunction
void VPE::VObjectProperty::setObjectsList(const QMap<QString, quint32> &objects)
{
    this->objects = objects;
}

//! Get the settings. This function has to be implemented in a subclass in order to have an effect
// cppcheck-suppress unusedFunction
auto VPE::VObjectProperty::getObjects() const -> QMap<QString, quint32>
{
    return objects;
}

//! Sets the value of the property
void VPE::VObjectProperty::setValue(const QVariant& value)
{
    VProperty::d_ptr->VariantValue = value;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::UInt));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::UInt);
#endif

    if (VProperty::d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::d_ptr->editor);
    }
}

auto VPE::VObjectProperty::type() const -> QString
{
    return "objectList";
}

auto VPE::VObjectProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VObjectProperty(getName()));
}

void VPE::VObjectProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent ( VProperty::d_ptr->editor, event );
}

void VPE::VObjectProperty::FillList(QComboBox *box, const QMap<QString, quint32> &list) const
{
    box->clear();

    QMap<QString, quint32>::const_iterator i;
    for (i = list.constBegin(); i != list.constEnd(); ++i)
    {
        box->addItem(i.key(), i.value());
    }
}
