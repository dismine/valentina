/************************************************************************
 **
 **  @file   vproperty_p.h
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

#ifndef VPROPERTY_P_H
#define VPROPERTY_P_H

// ONLY INCLUDE THIS IN .CPP FILES

#include <QVariant>
#include <QString>
#include "vproperty.h"

namespace VPE
{

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VPropertyPrivate
{
public:
    //! The property's value.
    //! This does not have to be used by subclasses, but it makes sense in cases where QVariant supports
    //! the data type. Also, this can be used as cache, so that when the data() function gets called by
    //! the model, the data does not have to be converted in a QVariant every time.
    QVariant VariantValue{}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! Property name
    QString Name{}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! Description
    QString Description{}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! Specifies whether the property is empty or not
    bool IsEmpty{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! Stores the property type
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMetaType::Type PropertyVariantType; // NOLINT(misc-non-private-member-variables-in-classes)
#else
    QVariant::Type PropertyVariantType; // NOLINT(misc-non-private-member-variables-in-classes)
#endif

    //! Stores whether the views have to update the parent of this property if it changes
    bool UpdateParent{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! Stores whether the views have to update the children of this property if it changes
    bool UpdateChildren{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! The parent property
    VProperty* Parent{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    QWidget* editor{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    Property type{Property::Simple}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! List of child properties
    QList<VProperty*> Children{}; // NOLINT(misc-non-private-member-variables-in-classes)

    //! Constructor passing name and type
    VPropertyPrivate(const QString& name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                     QMetaType::Type type)
        : VariantValue(QMetaType(type)),
#else
                     QVariant::Type type)
        : VariantValue(type),
#endif
          Name(name),
          PropertyVariantType(type)
    {}

    //! Constructor
    VPropertyPrivate() 
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        : PropertyVariantType(QMetaType::UnknownType)
#else
        : PropertyVariantType(QVariant::Invalid)
#endif
    {}

    virtual ~VPropertyPrivate();
private:
    Q_DISABLE_COPY_MOVE(VPropertyPrivate) // NOLINT
};

QT_WARNING_POP

}  // namespace VPE

#endif // VPROPERTY_P_H
