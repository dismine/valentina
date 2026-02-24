/************************************************************************
 **
 **  @file   vboolproperty.cpp
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

#include "vboolproperty.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QFlags>
#include <QObject>

#include "../vmisc/compatibility.h"
#include "../vproperty_p.h"

VPE::VBoolProperty::VBoolProperty(const QString& name) :
    VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::Bool)
#else
              QVariant::Bool)
#endif
{
    vproperty_d_ptr->VariantValue.setValue(false);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    vproperty_d_ptr->VariantValue.convert(QMetaType(QMetaType::Bool));
#else
    vproperty_d_ptr->VariantValue.convert(QVariant::Bool);
#endif
}


//! Get the data how it should be displayed
auto VPE::VBoolProperty::data (int column, int role) const -> QVariant
{
    auto* tmpEditor = qobject_cast<QCheckBox*>(VProperty::vproperty_d_ptr->editor);

    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        return tmpEditor->checkState();
    }

    if (column == DPC_Data && Qt::EditRole == role)
    {
        return VProperty::vproperty_d_ptr->VariantValue;
    }

    return VProperty::data(column, role);
}

auto VPE::VBoolProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                      const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    auto* tmpEditor = new QCheckBox(parent);
    tmpEditor->setCheckState(vproperty_d_ptr->VariantValue.toBool() ? Qt::Checked : Qt::Unchecked);

    connect(tmpEditor, CHECKBOX_STATE_CHANGED, this, &VBoolProperty::StateChanged);

    VProperty::vproperty_d_ptr->editor = tmpEditor;
    return VProperty::vproperty_d_ptr->editor;
}

auto VPE::VBoolProperty::setEditorData(QWidget *editor) -> bool
{
    if (!editor)
    {
        return false;
    }

    auto* tmpEditor = qobject_cast<QCheckBox*>(editor);
    if (tmpEditor)
    {
        const QSignalBlocker blocker(tmpEditor);
        tmpEditor->setCheckState(vproperty_d_ptr->VariantValue.toBool() ? Qt::Checked : Qt::Unchecked);
        return true;
    }

    return false;
}

auto VPE::VBoolProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto* tmpEditor = qobject_cast<const QCheckBox*>(editor);
    if (tmpEditor)
    {
        return tmpEditor->checkState() == Qt::Checked ? Qt::Checked : Qt::Unchecked;
    }

    return {0};
}

void VPE::VBoolProperty::setValue(const QVariant &value)
{
    VProperty::vproperty_d_ptr->VariantValue = value;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::vproperty_d_ptr->VariantValue.convert(QMetaType(QMetaType::Bool));
#else
    VProperty::vproperty_d_ptr->VariantValue.convert(QVariant::Bool);
#endif

    if (VProperty::vproperty_d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::vproperty_d_ptr->editor);
    }
}

//! Returns item flags
auto VPE::VBoolProperty::flags(int column) const -> Qt::ItemFlags
{
    if (column == DPC_Data)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    return VProperty::flags(column);
}

auto VPE::VBoolProperty::type() const -> QString
{
    return "bool";
}

auto VPE::VBoolProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VBoolProperty(getName()));
}

void VPE::VBoolProperty::StateChanged()
{
    auto *event = new UserChangeEvent();
    QCoreApplication::postEvent ( VProperty::vproperty_d_ptr->editor, event );
}
