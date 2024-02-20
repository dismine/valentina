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

#include "../vproperty_p.h"

VPE::VLineColorProperty::VLineColorProperty(const QString &name)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::Int),
#else
              QVariant::Int),
#endif
    colors(),
    indexList()
{
    VProperty::d_ptr->VariantValue = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
#endif
}

auto VPE::VLineColorProperty::data(int column, int role) const -> QVariant
{
    if (colors.empty())
    {
        return {};
    }

    int tmpIndex = VProperty::d_ptr->VariantValue.toInt();

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

auto VPE::VLineColorProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                           const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    auto *tmpEditor = new QComboBox(parent);
    tmpEditor->setPalette(parent->palette());

    int size = tmpEditor->iconSize().height(); // NOLINT(misc-const-correctness)
    // On Mac pixmap should be little bit smaller.
#if defined(Q_OS_MAC)
    size -= 2; // Two pixels should be enough.
#endif         // defined(Q_OS_MAC)

    QMap<QString, QString>::const_iterator i = colors.constBegin();
    while (i != colors.constEnd())
    {
        QPixmap pix(size, size);
        pix.fill(QColor(i.key()));

        // Draw a white border around the icon
        QPainter painter(&pix);
        painter.setPen(tmpEditor->palette().color(QPalette::Text));
        painter.drawRect(0, 0, size - 1, size - 1);

        tmpEditor->addItem(QIcon(pix), i.value(), QVariant(i.key()));
        ++i;
    }

    tmpEditor->setLocale(parent->locale());
    tmpEditor->setCurrentIndex(VProperty::d_ptr->VariantValue.toInt());
    connect(tmpEditor, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VLineColorProperty::currentIndexChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

auto VPE::VLineColorProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpEditor = qobject_cast<const QComboBox *>(editor);
    if (tmpEditor)
    {
        return tmpEditor->currentIndex();
    }

    return {0};
}

void VPE::VLineColorProperty::setColors(const QMap<QString, QString> &colors)
{
    this->colors = colors;
    indexList.clear();
    QMap<QString, QString>::const_iterator i = colors.constBegin();
    while (i != colors.constEnd())
    {
        indexList.append(i.key());
        ++i;
    }
}

// cppcheck-suppress unusedFunction
auto VPE::VLineColorProperty::getColors() const -> QMap<QString, QString>
{
    return colors;
}

void VPE::VLineColorProperty::setValue(const QVariant &value)
{
    int tmpIndex = value.toInt();

    if (tmpIndex < 0 || tmpIndex >= indexList.count())
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

auto VPE::VLineColorProperty::type() const -> QString
{
    return QStringLiteral("lineColor");
}

auto VPE::VLineColorProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VLineColorProperty(getName()));
}

auto VPE::VLineColorProperty::IndexOfColor(const QMap<QString, QString> &colors, const QString &color) -> vpesizetype
{
    QVector<QString> indexList;
    QMap<QString, QString>::const_iterator i = colors.constBegin();
    while (i != colors.constEnd())
    {
        indexList.append(i.key());
        ++i;
    }
    return indexList.indexOf(color);
}

void VPE::VLineColorProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    auto *event = new UserChangeEvent();
    QCoreApplication::postEvent(VProperty::d_ptr->editor, event);
}
