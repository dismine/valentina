/************************************************************************
 **
 **  @file   vlinetypeproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vlinetypeproperty.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QLocale>
#include <QSize>
#include <QWidget>

#include "../vproperty_p.h"

VPE::VLineTypeProperty::VLineTypeProperty(const QString &name)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::Int),
#else
              QVariant::Int),
#endif
    styles(),
    indexList()
{
    VProperty::vproperty_d_ptr->VariantValue = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::vproperty_d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::vproperty_d_ptr->VariantValue.convert(QVariant::Int);
#endif
}

auto VPE::VLineTypeProperty::data(int column, int role) const -> QVariant
{
    if (styles.empty())
    {
        return {};
    }

    int tmpIndex = VProperty::vproperty_d_ptr->VariantValue.toInt();

    if (tmpIndex < 0 || tmpIndex >= indexList.count())
    {
        tmpIndex = 0;
    }

    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        return indexList.at(tmpIndex);
    }

    if (column == DPC_Data && Qt::EditRole == role)
    {
        return tmpIndex;
    }
    return VProperty::data(column, role);
}

auto VPE::VLineTypeProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                          const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    auto *tmpEditor = new QComboBox(parent);
    tmpEditor->clear();
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setIconSize(QSize(80, 14));
    tmpEditor->setMinimumWidth(80);
    tmpEditor->setMaximumWidth(110);

    QMap<QString, QIcon>::const_iterator i = styles.constBegin();
    while (i != styles.constEnd())
    {
        tmpEditor->addItem(i.value(), QString(), QVariant(i.key()));
        ++i;
    }

    tmpEditor->setCurrentIndex(VProperty::vproperty_d_ptr->VariantValue.toInt());
    connect(tmpEditor, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VLineTypeProperty::currentIndexChanged);

    VProperty::vproperty_d_ptr->editor = tmpEditor;
    return VProperty::vproperty_d_ptr->editor;
}

auto VPE::VLineTypeProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpEditor = qobject_cast<const QComboBox *>(editor);
    if (tmpEditor)
    {
        return tmpEditor->currentIndex();
    }

    return {0};
}

void VPE::VLineTypeProperty::setStyles(const QMap<QString, QIcon> &styles)
{
    this->styles = styles;
    indexList.clear();
    QMap<QString, QIcon>::const_iterator i = styles.constBegin();
    while (i != styles.constEnd())
    {
        indexList.append(i.key());
        ++i;
    }
}

// cppcheck-suppress unusedFunction
auto VPE::VLineTypeProperty::getStyles() const -> QMap<QString, QIcon>
{
    return styles;
}

void VPE::VLineTypeProperty::setValue(const QVariant &value)
{
    int tmpIndex = value.toInt();

    if (tmpIndex < 0 || tmpIndex >= indexList.count())
    {
        tmpIndex = 0;
    }

    VProperty::vproperty_d_ptr->VariantValue = tmpIndex;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::vproperty_d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::vproperty_d_ptr->VariantValue.convert(QVariant::Int);
#endif

    if (VProperty::vproperty_d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::vproperty_d_ptr->editor);
    }
}

auto VPE::VLineTypeProperty::type() const -> QString
{
    return QStringLiteral("lineType");
}

auto VPE::VLineTypeProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VLineTypeProperty(getName()));
}

auto VPE::VLineTypeProperty::IndexOfStyle(const QMap<QString, QIcon> &styles, const QString &style) -> vpesizetype
{
    QVector<QString> indexList;
    QMap<QString, QIcon>::const_iterator i = styles.constBegin();
    while (i != styles.constEnd())
    {
        indexList.append(i.key());
        ++i;
    }
    return indexList.indexOf(style);
}

void VPE::VLineTypeProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    auto *event = new UserChangeEvent();
    QCoreApplication::postEvent(VProperty::vproperty_d_ptr->editor, event);
}
