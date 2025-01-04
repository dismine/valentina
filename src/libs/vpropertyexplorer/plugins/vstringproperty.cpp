/************************************************************************
 **
 **  @file   vstringproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 8, 2014
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

#include "vstringproperty.h"

#include <QKeyEvent>
#include <QLatin1String>
#include <QLineEdit>
#include <QLocale>
#include <QSizePolicy>
#include <QWidget>

#include "../vproperty_p.h"

VPE::VStringProperty::VStringProperty(const QString &name, const QMap<QString, QVariant> &settings)
    : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QMetaType::QString),
#else
                QVariant::String),
#endif
      readOnly(false), typeForParent(0), clearButton(false), m_osSeparator(false)
{
    VProperty::setSettings(settings);
    vproperty_d_ptr->VariantValue.setValue(QString());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    vproperty_d_ptr->VariantValue.convert(QMetaType(QMetaType::QString));
#else
    vproperty_d_ptr->VariantValue.convert(QVariant::String);
#endif
}

VPE::VStringProperty::VStringProperty(const QString &name)
    : VProperty(name), readOnly(false), typeForParent(0), clearButton(false), m_osSeparator(false)
{
    vproperty_d_ptr->VariantValue.setValue(QString());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    vproperty_d_ptr->VariantValue.convert(QMetaType(QMetaType::QString));
#else
    vproperty_d_ptr->VariantValue.convert(QVariant::String);
#endif
}

auto VPE::VStringProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                        const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    auto *tmpEditor = new QLineEdit(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setReadOnly(readOnly);
    tmpEditor->installEventFilter(this);
    tmpEditor->setClearButtonEnabled(clearButton);
    tmpEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tmpEditor->setText(vproperty_d_ptr->VariantValue.toString());

    vproperty_d_ptr->editor = tmpEditor;
    return vproperty_d_ptr->editor;
}

auto VPE::VStringProperty::setEditorData(QWidget *editor) -> bool
{
    if (auto *tmpWidget = qobject_cast<QLineEdit *>(editor))
    {
        if (not readOnly)
        {
            tmpWidget->setText(vproperty_d_ptr->VariantValue.toString());
        }
        return true;
    }

    return false;
}

auto VPE::VStringProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpEditor = qobject_cast<const QLineEdit *>(editor);
    if (tmpEditor)
    {
        return tmpEditor->text();
    }

    return QVariant(QString());
}

void VPE::VStringProperty::setReadOnly(bool readOnly)
{
    this->readOnly = readOnly;
}

void VPE::VStringProperty::setOsSeparator(bool separator)
{
    m_osSeparator = separator;
}

void VPE::VStringProperty::setClearButtonEnable(bool value)
{
    this->clearButton = value;
}

void VPE::VStringProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == QLatin1String("ReadOnly"))
    {
        setReadOnly(value.toBool());
    }
    if (key == QLatin1String("TypeForParent"))
    {
        setTypeForParent(value.toInt());
    }
}

auto VPE::VStringProperty::getSetting(const QString &key) const -> QVariant
{
    if (key == QLatin1String("ReadOnly"))
    {
        return readOnly;
    }
    else if (key == QLatin1String("TypeForParent"))
    {
        return typeForParent;
    }
    else
        return VProperty::getSetting(key);
}

auto VPE::VStringProperty::getSettingKeys() const -> QStringList
{
    QStringList settings;
    settings << QStringLiteral("ReadOnly") << QStringLiteral("TypeForParent");
    return settings;
}

auto VPE::VStringProperty::type() const -> QString
{
    return QStringLiteral("string");
}

auto VPE::VStringProperty::clone(bool include_children, VPE::VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VStringProperty(getName(), getSettings()));
}

void VPE::VStringProperty::UpdateParent(const QVariant &value)
{
    emit childChanged(value, typeForParent);
}

// cppcheck-suppress unusedFunction
auto VPE::VStringProperty::getTypeForParent() const -> int
{
    return typeForParent;
}

void VPE::VStringProperty::setTypeForParent(int value)
{
    typeForParent = value;
}

auto VPE::VStringProperty::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (auto *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            if (const auto *keyEvent = static_cast<QKeyEvent *>(event);
                (keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (m_osSeparator)
                {
                    textEdit->insert(VPELocaleDecimalPoint(QLocale()));
                }
                else
                {
                    textEdit->insert(VPELocaleDecimalPoint(QLocale::c()));
                }
                return true;
            }
        }
    }
    else
    {
        // pass the event on to the parent class
        return VProperty::eventFilter(object, event);
    }
    return false;// pass the event to the widget
}
