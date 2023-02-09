/************************************************************************
 **
 **  @file   vtextproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2020
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
#include "vtextproperty.h"
#include "../vproperty_p.h"
#include "../vmisc/compatibility.h"

#include <QPlainTextEdit>
#include <QTextEdit>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void SetTabStopDistance(QPlainTextEdit *edit, int tabWidthChar=4);
void SetTabStopDistance(QPlainTextEdit *edit, int tabWidthChar)
{
    const auto fontMetrics = edit->fontMetrics();

    const QString testString(" ");

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    const int singleCharWidth = fontMetrics.width(testString);
    edit->setTabStopWidth(tabWidthChar * singleCharWidth);
#else
    // compute the size of a char in double-precision
    static constexpr int bigNumber = 1000; // arbitrary big number.
    const int many_char_width = TextWidth(fontMetrics, testString.repeated(bigNumber));
    const double singleCharWidthDouble = many_char_width / double(bigNumber);
    // set the tab stop with double precision
    edit->setTabStopDistance(tabWidthChar * singleCharWidthDouble);
#endif
}
}


VPE::VTextProperty::VTextProperty(const QString &name, const QMap<QString, QVariant> &settings)
    : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QMetaType::QString),
#else
                QVariant::String),
#endif
    readOnly(false)
{
    VProperty::setSettings(settings);
    d_ptr->VariantValue.setValue(QString());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    d_ptr->VariantValue.convert(QMetaType(QMetaType::QString));
#else
    d_ptr->VariantValue.convert(QVariant::String);
#endif
}

VPE::VTextProperty::VTextProperty(const QString &name)
    : VProperty(name),
    readOnly(false)
{
    d_ptr->VariantValue.setValue(QString());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    d_ptr->VariantValue.convert(QMetaType(QMetaType::QString));
#else
    d_ptr->VariantValue.convert(QVariant::String);
#endif
}

QWidget *VPE::VTextProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                          const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    QPlainTextEdit* tmpEditor = new QPlainTextEdit(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setReadOnly(readOnly);
    tmpEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tmpEditor->setPlainText(d_ptr->VariantValue.toString());
    SetTabStopDistance(tmpEditor);

    d_ptr->editor = tmpEditor;
    return d_ptr->editor;
}

bool VPE::VTextProperty::setEditorData(QWidget *editor)
{
    if (QPlainTextEdit* tmpWidget = qobject_cast<QPlainTextEdit*>(editor))
    {
        tmpWidget->setPlainText(d_ptr->VariantValue.toString());
        return true;
    }

    return false;
}

QVariant VPE::VTextProperty::getEditorData(const QWidget *editor) const
{
    const QPlainTextEdit* tmpEditor = qobject_cast<const QPlainTextEdit*>(editor);
    if (tmpEditor)
    {
        return tmpEditor->toPlainText();
    }

    return QVariant(QString());
}

void VPE::VTextProperty::setReadOnly(bool readOnly)
{
    this->readOnly = readOnly;
}

void VPE::VTextProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == QLatin1String("ReadOnly"))
    {
        setReadOnly(value.toBool());
    }
}

QVariant VPE::VTextProperty::getSetting(const QString &key) const
{
    if (key == QLatin1String("ReadOnly"))
    {
        return readOnly;
    }
    else
        return VProperty::getSetting(key);
}

QStringList VPE::VTextProperty::getSettingKeys() const
{
    QStringList settings;
    settings << QStringLiteral("ReadOnly");
    return settings;
}

QString VPE::VTextProperty::type() const
{
    return QStringLiteral("string");
}

VPE::VProperty *VPE::VTextProperty::clone(bool include_children, VPE::VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new VTextProperty(getName(), getSettings()));
}
