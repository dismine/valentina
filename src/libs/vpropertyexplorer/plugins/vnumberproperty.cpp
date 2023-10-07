/************************************************************************
 **
 **  @file   vnumberproperty.cpp
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

#include "vnumberproperty.h"

#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QLatin1String>
#include <QLocale>
#include <QSizePolicy>
#include <QSpinBox>
#include <QWidget>

#include "../vproperty_p.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrMin, ("Min"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrMax, ("Max"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrInteger, ("integer"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrStep, ("Step"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrSuffix, ("Suffix"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrPrecision, ("Precision"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, StrDouble, ("double"_L1))       // NOLINT
} // namespace

const int VPE::VIntegerProperty::StandardMin = -1000000;
const int VPE::VIntegerProperty::StandardMax = 1000000;

VPE::VIntegerProperty::VIntegerProperty(const QString &name, const QMap<QString, QVariant> &settings)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::Int),
#else
              QVariant::Int),
#endif
    m_minValue(StandardMin),
    m_maxValue(StandardMax),
    m_singleStep(1.0)
{
    VProperty::setSettings(settings);
    VProperty::d_ptr->VariantValue.setValue(0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
#endif
}

VPE::VIntegerProperty::VIntegerProperty(const QString &name)
  : VProperty(name),
    m_minValue(StandardMin),
    m_maxValue(StandardMax),
    m_singleStep(1.0)
{
    VProperty::d_ptr->VariantValue.setValue(0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Int));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
#endif
}

//! Returns an editor widget, or NULL if it doesn't supply one
auto VPE::VIntegerProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                         const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    auto *tmpEditor = new QSpinBox(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setMinimum(static_cast<int>(m_minValue));
    tmpEditor->setMaximum(static_cast<int>(m_maxValue));
    tmpEditor->setSingleStep(static_cast<int>(m_singleStep));
    tmpEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tmpEditor->setValue(VProperty::d_ptr->VariantValue.toInt());
    tmpEditor->setSuffix(m_suffix);
    connect(tmpEditor, QOverload<int>::of(&QSpinBox::valueChanged), this, &VIntegerProperty::valueChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

//! Gets the data from the widget
auto VPE::VIntegerProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpEditor = qobject_cast<const QSpinBox *>(editor);
    if (tmpEditor)
    {
        return tmpEditor->value();
    }

    return {0};
}

void VPE::VIntegerProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == *StrMax)
    {
        m_maxValue = value.toInt();
    }
    else if (key == *StrMin)
    {
        m_minValue = value.toInt();
    }
    else if (key == *StrStep)
    {
        m_singleStep = value.toInt();
    }
    else if (key == *StrSuffix)
    {
        m_suffix = value.toString();
    }
}

auto VPE::VIntegerProperty::getSetting(const QString &key) const -> QVariant
{
    if (key == *StrMin)
    {
        return m_minValue;
    }

    if (key == *StrMax)
    {
        return m_maxValue;
    }

    if (key == *StrStep)
    {
        return m_singleStep;
    }

    if (key == *StrSuffix)
    {
        return m_suffix;
    }

    return VProperty::getSetting(key);
}

auto VPE::VIntegerProperty::getSettingKeys() const -> QStringList
{
    return {*StrMin, *StrMax, *StrStep, *StrSuffix};
}

auto VPE::VIntegerProperty::type() const -> QString
{
    return *StrInteger;
}

auto VPE::VIntegerProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VIntegerProperty(getName()));
}

void VPE::VIntegerProperty::valueChanged(int i)
{
    Q_UNUSED(i)
    QCoreApplication::postEvent(VProperty::d_ptr->editor, new UserChangeEvent());
}

const int VPE::VDoubleProperty::StandardMin = -1000000;
const int VPE::VDoubleProperty::StandardMax = 1000000;
const double VPE::VDoubleProperty::StandardPrecision = 5;

VPE::VDoubleProperty::VDoubleProperty(const QString &name, const QMap<QString, QVariant> &settings)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::Double),
#else
              QVariant::Double),
#endif

    m_minValue(StandardMin),
    m_maxValue(StandardMax),
    m_singleStep(1.0),
    m_precision(static_cast<int>(StandardPrecision))
{
    VProperty::setSettings(settings);
    VProperty::d_ptr->VariantValue.setValue(0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Double));
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Double);
#endif
}

VPE::VDoubleProperty::VDoubleProperty(const QString &name)
  : VProperty(name),
    m_minValue(StandardMin),
    m_maxValue(StandardMax),
    m_singleStep(1.0),
    m_precision(static_cast<int>(StandardPrecision))
{
    VProperty::d_ptr->VariantValue.setValue(0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    VProperty::d_ptr->VariantValue.convert(QMetaType(QMetaType::Double));
    VProperty::d_ptr->PropertyVariantType = QMetaType::Double;
#else
    VProperty::d_ptr->VariantValue.convert(QVariant::Double);
    VProperty::d_ptr->PropertyVariantType = QVariant::Double;
#endif
}

//! Returns an editor widget, or NULL if it doesn't supply one
auto VPE::VDoubleProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                        const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    auto *tmpEditor = new QDoubleSpinBox(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setMinimum(m_minValue);
    tmpEditor->setMaximum(m_maxValue);
    tmpEditor->setDecimals(m_precision);
    tmpEditor->setSingleStep(m_singleStep);
    tmpEditor->setSuffix(m_suffix);
    tmpEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tmpEditor->setValue(VProperty::d_ptr->VariantValue.toDouble());
    connect(tmpEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VDoubleProperty::valueChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

//! Gets the data from the widget
auto VPE::VDoubleProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    const auto *tmpEditor = qobject_cast<const QDoubleSpinBox *>(editor);
    if (tmpEditor)
    {
        return tmpEditor->value();
    }

    return {0};
}

void VPE::VDoubleProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == *StrMin)
    {
        m_minValue = value.toDouble();
    }
    else if (key == *StrMax)
    {
        m_maxValue = value.toDouble();
    }
    else if (key == *StrStep)
    {
        m_singleStep = value.toDouble();
    }
    else if (key == *StrSuffix)
    {
        m_suffix = value.toString();
    }
    else if (key == *StrPrecision)
    {
        m_precision = value.toInt();
    }
}

auto VPE::VDoubleProperty::getSetting(const QString &key) const -> QVariant
{
    if (key == *StrMin)
    {
        return m_minValue;
    }

    if (key == *StrMax)
    {
        return m_maxValue;
    }

    if (key == *StrStep)
    {
        return m_singleStep;
    }

    if (key == *StrSuffix)
    {
        return m_suffix;
    }

    if (key == *StrPrecision)
    {
        return m_precision;
    }

    return VProperty::getSetting(key);
}

auto VPE::VDoubleProperty::getSettingKeys() const -> QStringList
{
    return {*StrMin, *StrMax, *StrStep, *StrSuffix, *StrPrecision};
}

auto VPE::VDoubleProperty::type() const -> QString
{
    return *StrDouble;
}

auto VPE::VDoubleProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    return VProperty::clone(include_children, container ? container : new VDoubleProperty(getName()));
}

void VPE::VDoubleProperty::valueChanged(int i)
{
    Q_UNUSED(i)
    QCoreApplication::postEvent(VProperty::d_ptr->editor, new UserChangeEvent());
}
