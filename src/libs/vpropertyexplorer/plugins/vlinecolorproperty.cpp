/************************************************************************
 **
 **  @file   vlinecolorproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 2, 2015
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

#include "vlinecolorproperty.h"

#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QWidget>

#include "../qtcolorpicker.h"
#include "../vproperty_p.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// The list of all string we use for conversion
// Better to use global variables because repeating QStringLiteral blows up code size
Q_GLOBAL_STATIC_WITH_ARGS(const QString, defaultColorName, ("default"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrCustomColors, ("customColors"_L1)) // NOLINT

QT_WARNING_POP
} // namespace

VPE::VLineColorProperty::VLineColorProperty(const QString &name)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::QString)
#else
              QVariant::QString)
#endif
{
}

auto VPE::VLineColorProperty::data(int column, int role) const -> QVariant
{
    if (column == DPC_Data && (Qt::DisplayRole == role || Qt::EditRole == role))
    {
        return vproperty_d_ptr->VariantValue.toString();
    }

    return VProperty::data(column, role);
}

auto VPE::VLineColorProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                           const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    auto *tmpPicker = new VPE::QtColorPicker(parent);
    tmpPicker->setPalette(parent->palette());
    tmpPicker->setUseNativeDialog(useNativeDialog);

    bool foundDefaultColor = false;

    if (defaultColors.contains(*defaultColorName))
    {
        tmpPicker->insertColor(QColor(), defaultColors.value(*defaultColorName));
        defaultColors.remove(*defaultColorName);
        foundDefaultColor = true;
    }

    auto i = defaultColors.constBegin();
    while (i != defaultColors.constEnd())
    {
        tmpPicker->insertColor(QColor(i.key()), i.value());
        ++i;
    }

    for (auto color : customColors)
    {
        tmpPicker->insertCustomColor(color);
    }

    tmpPicker->setCurrentColor(foundDefaultColor ? QColor() : QColor(Qt::black));
    tmpPicker->setLocale(parent->locale());
    tmpPicker->setCurrentColor(vproperty_d_ptr->VariantValue.value<QColor>());
    connect(tmpPicker, &VPE::QtColorPicker::colorChanged, this, &VLineColorProperty::currentColorChanged);

    VProperty::vproperty_d_ptr->editor = tmpPicker;
    return VProperty::vproperty_d_ptr->editor;
}

auto VPE::VLineColorProperty::setEditorData(QWidget *editor) -> bool
{
    if (auto *tmpWidget = qobject_cast<VPE::QtColorPicker *>(editor); tmpWidget != nullptr)
    {
        QString const colorName = vproperty_d_ptr->VariantValue.toString();
        QColor const color = colorName != *defaultColorName ? QColor(colorName) : QColor();

        tmpWidget->setCurrentColor(color);
    }
    else
    {
        return false;
    }

    return true;
}

auto VPE::VLineColorProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    if (const auto *tmpPicker = qobject_cast<const VPE::QtColorPicker *>(editor); tmpPicker != nullptr)
    {
        QColor const color = tmpPicker->currentColor();
        return color.isValid() ? color : *defaultColorName;
    }

    return *defaultColorName;
}

auto VPE::VLineColorProperty::getSetting(const QString &key) const -> QVariant
{
    if (key == *StrCustomColors)
    {
        if (const auto *tmpPicker = qobject_cast<const VPE::QtColorPicker *>(VProperty::vproperty_d_ptr->editor);
            tmpPicker != nullptr)
        {
            return QVariant::fromValue(tmpPicker->CustomColors());
        }
    }

    return VProperty::getSetting(key);
}

auto VPE::VLineColorProperty::getSettingKeys() const -> QStringList
{
    return {*StrCustomColors};
}

auto VPE::VLineColorProperty::type() const -> QString
{
    return QStringLiteral("lineColor");
}

auto VPE::VLineColorProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VLineColorProperty(getName()));
}

void VPE::VLineColorProperty::currentColorChanged(const QColor &color)
{
    Q_UNUSED(color)
    auto *event = new UserChangeEvent();
    QCoreApplication::postEvent(VProperty::vproperty_d_ptr->editor, event);
}

void VPE::VLineColorProperty::SetUseNativeDialog(bool newUseNativeDialog)
{
    useNativeDialog = newUseNativeDialog;
}

void VPE::VLineColorProperty::SetCustomColors(const QVector<QColor> &newCustomColors)
{
    customColors = newCustomColors;
}

void VPE::VLineColorProperty::SetDefaultColors(const QMap<QString, QString> &newDefaultColors)
{
    defaultColors = newDefaultColors;
}
