/************************************************************************
 **
 **  @file   venumproperty.cpp
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

#include "venumproperty.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QLocale>
#include <QWidget>

#include "../vproperty_p.h"

VPE::VEnumProperty::VEnumProperty(const QString& name)
    : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QMetaType::Int),
#else
                QVariant::Int),
#endif
      EnumerationLiterals()
{
    VProperty::d_ptr->VariantValue = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
#endif
}


//! Get the data how it should be displayed
auto VPE::VEnumProperty::data(int column, int role) const -> QVariant
{
    if (EnumerationLiterals.empty())
    {
        return QVariant();
    }

    int tmpIndex = VProperty::d_ptr->VariantValue.toInt();

    if (tmpIndex < 0 || tmpIndex >= EnumerationLiterals.count())
    {
        tmpIndex = 0;
    }

    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        return EnumerationLiterals.at(tmpIndex);
    }
    else if (column == DPC_Data && Qt::EditRole == role)
    {
        return tmpIndex;
    }
    else
        return VProperty::data(column, role);
}


//! Returns an editor widget, or NULL if it doesn't supply one
auto VPE::VEnumProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                      const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    auto *tmpEditor = new QComboBox(parent);
    tmpEditor->clear();
    tmpEditor->setLocale(parent->locale());
    tmpEditor->addItems(EnumerationLiterals);
    tmpEditor->setCurrentIndex(VProperty::d_ptr->VariantValue.toInt());
    connect(tmpEditor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VEnumProperty::currentIndexChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

//! Gets the data from the widget
auto VPE::VEnumProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpEditor = qobject_cast<const QComboBox *>(editor);
    if (tmpEditor)
    {
        return tmpEditor->currentIndex();
    }

    return QVariant(0);
}

//! Sets the enumeration literals
void VPE::VEnumProperty::setLiterals(const QStringList& literals)
{
    EnumerationLiterals = literals;
}

//! Get the settings. This function has to be implemented in a subclass in order to have an effect
auto VPE::VEnumProperty::getLiterals() const -> QStringList
{
    return EnumerationLiterals;
}

//! Sets the value of the property
void VPE::VEnumProperty::setValue(const QVariant& value)
{
    int tmpIndex = value.toInt();

    if (tmpIndex < 0 || tmpIndex >= EnumerationLiterals.count())
    {
        tmpIndex = 0;
    }

    VProperty::d_ptr->VariantValue = tmpIndex;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
#endif

    if (VProperty::d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::d_ptr->editor);
    }
}

auto VPE::VEnumProperty::type() const -> QString
{
    return "enum";
}

auto VPE::VEnumProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VEnumProperty(getName()));
}

void VPE::VEnumProperty::setSetting(const QString& key, const QVariant& value)
{
    if (key == "literals")
    {
        setLiterals(value.toString().split(";;"));
    }
}

auto VPE::VEnumProperty::getSetting(const QString &key) const -> QVariant
{
    if (key == "literals")
    {
        return getLiterals().join(";;");
    }
    else
        return VProperty::getSetting(key);
}

auto VPE::VEnumProperty::getSettingKeys() const -> QStringList
{
    return QStringList("literals");
}

void VPE::VEnumProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    auto *event = new UserChangeEvent();
    QCoreApplication::postEvent ( VProperty::d_ptr->editor, event );
}
