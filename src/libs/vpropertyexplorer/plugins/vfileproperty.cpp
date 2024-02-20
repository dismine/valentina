/************************************************************************
 **
 **  @file   vfileproperty.cpp
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

#include "vfileproperty.h"

#include <QAbstractItemDelegate>
#include <QFileInfo>
#include <QLocale>
#include <QWidget>

#include "../vfileproperty_p.h"
#include "../vproperty_p.h"
#include "vfilepropertyeditor.h"

VPE::VFileProperty::VFileProperty(const QString &name)
  : VProperty(new VFilePropertyPrivate(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                                       QMetaType::QString))
#else
                                        QVariant::String))
#endif
{
}

VPE::VFileProperty::~VFileProperty()
{
    //
}

void VPE::VFileProperty::setFileFilters(const QString &filefilters)
{
    static_cast<VFilePropertyPrivate *>(d_ptr)->FileFilters = filefilters;
}

auto VPE::VFileProperty::getFileFilters() const -> QString
{
    return static_cast<VFilePropertyPrivate *>(d_ptr)->FileFilters;
}

void VPE::VFileProperty::setFile(const QString &file)
{
    d_ptr->VariantValue.setValue(file);
}

auto VPE::VFileProperty::getFile() const -> QString
{
    return d_ptr->VariantValue.toString();
}

auto VPE::VFileProperty::data(int column, int role) const -> QVariant
{
    if (column == DPC_Data && (Qt::DisplayRole == role || Qt::EditRole == role))
    {
        QFileInfo const tmpFile(d_ptr->VariantValue.toString());
        return tmpFile.fileName();
    }
    else
        return VProperty::data(column, role);
}

auto VPE::VFileProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                      const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)

    auto *tmpWidget = new VFileEditWidget(parent);
    if (delegate)
    {
        VFileEditWidget::connect(tmpWidget, SIGNAL(commitData(QWidget *)), delegate, SIGNAL(commitData(QWidget *)));
    }
    tmpWidget->setLocale(parent->locale());
    tmpWidget->setFilter(static_cast<VFilePropertyPrivate *>(d_ptr)->FileFilters); // todo: parse this string
    tmpWidget->setFile(d_ptr->VariantValue.toString());
    tmpWidget->setDirectory(static_cast<VFilePropertyPrivate *>(d_ptr)->Directory);
    return tmpWidget;
}

auto VPE::VFileProperty::setEditorData(QWidget *editor) -> bool
{
    auto *tmpWidget = qobject_cast<VFileEditWidget *>(editor);
    if (tmpWidget)
    {
        tmpWidget->setFile(d_ptr->VariantValue.toString());
    }
    else
        return false;

    return true;
}

auto VPE::VFileProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpWidget = qobject_cast<const VFileEditWidget *>(editor);
    if (tmpWidget)
    {
        return tmpWidget->getFile();
    }

    return QVariant();
}

void VPE::VFileProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == "FileFilters")
    {
        setFileFilters(value.toString());
    }
    else if (key == "Directory")
    {
        setDirectory(value.toBool());
    }
}

auto VPE::VFileProperty::getSetting(const QString &key) const -> QVariant
{
    if (key == "FileFilters")
    {
        return getFileFilters();
    }
    else if (key == "Directory")
    {
        return isDirectory();
    }
    else
        return VProperty::getSetting(key);
}

auto VPE::VFileProperty::getSettingKeys() const -> QStringList
{
    return QStringList("FileFilters") << "Directory";
}

auto VPE::VFileProperty::type() const -> QString
{
    return "file";
}

auto VPE::VFileProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VFileProperty(getName()));
}

auto VPE::VFileProperty::isDirectory() const -> bool
{
    return static_cast<VFilePropertyPrivate *>(d_ptr)->Directory;
}

void VPE::VFileProperty::setDirectory(bool is_directory)
{
    static_cast<VFilePropertyPrivate *>(d_ptr)->Directory = is_directory;
}
