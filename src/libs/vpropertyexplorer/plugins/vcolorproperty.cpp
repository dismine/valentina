/************************************************************************
 **
 **  @file   vcolorproperty.cpp
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

#include "vcolorproperty.h"

#include <QColor>
#include <QLocale>
#include <QPixmap>
#include <QWidget>

#include "../vproperty_p.h"
#include "vcolorpropertyeditor.h"

VPE::VColorProperty::VColorProperty(const QString &name) :
    VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::QColor)
#else
              QVariant::Color)
#endif
{
}

//! Get the data how it should be displayed
auto VPE::VColorProperty::data(int column, int role) const -> QVariant
{
    if (column == DPC_Data && (Qt::DisplayRole == role))
    {
        return VColorPropertyEditor::GetColorString(vproperty_d_ptr->VariantValue.value<QColor>());
    }
    
    if (Qt::EditRole == role)
    {
        return {};
    }
    
    if (column == DPC_Data && (Qt::DecorationRole == role))
    {
        return VColorPropertyEditor::GetColorPixmap(vproperty_d_ptr->VariantValue.value<QColor>());
    }

    return VProperty::data(column, role);
}

//! Returns an editor widget, or NULL if it doesn't supply one
auto VPE::VColorProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                       const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    auto *tmpWidget = new VColorPropertyEditor(parent);
    tmpWidget->setLocale(parent->locale());
    tmpWidget->SetColor(vproperty_d_ptr->VariantValue.value<QColor>());
    return tmpWidget;
}

//! Sets the property's data to the editor (returns false, if the standard delegate should do that)
auto VPE::VColorProperty::setEditorData(QWidget *editor) -> bool
{
    if (auto *tmpWidget = qobject_cast<VColorPropertyEditor *>(editor); tmpWidget != nullptr)
    {
        tmpWidget->SetColor(vproperty_d_ptr->VariantValue.value<QColor>());
    }
    else
    {
        return false;
    }

    return true;
}

//! Gets the data from the widget
auto VPE::VColorProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    if (const auto *tmpWidget = qobject_cast<const VColorPropertyEditor *>(editor); tmpWidget != nullptr)
    {
        return tmpWidget->GetColor();
    }

    return {};
}

auto VPE::VColorProperty::type() const -> QString
{
    return QStringLiteral("color");
}

auto VPE::VColorProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container != nullptr ? container : new VColorProperty(getName()));
}
