/************************************************************************
 **
 **  @file   vpropertyset.cpp
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

#include "vpropertyset.h"

#include <stddef.h>
#include <QList>
#include <QMap>

#include "vproperty.h"
#include "vpropertyset_p.h"


VPE::VPropertySet::VPropertySet()
    // cppcheck-suppress noCopyConstructor
    // cppcheck-suppress noOperatorEq
    : d_ptr(new VPropertySetPrivate())
{
}


VPE::VPropertySet::~VPropertySet()
{
    // Delete all the properties
    VPE::VPropertySet::clear(true);

    delete d_ptr;
}

auto VPE::VPropertySet::addProperty(VProperty *property, const QString &id, const QString &parentid) -> bool
{
    // Check if the property to add is not a null pointer
    if (!property)
    {
        return false;
    }

    VProperty* tmpParent = parentid.isEmpty() ? nullptr : getProperty(parentid);
    return addProperty(property, id, tmpParent);
}

auto VPE::VPropertySet::addProperty(VProperty *property, const QString &id, VProperty *parent_property) -> bool
{
    // Check if the property to add is not a null pointer
    if (!property)
    {
        return false;
    }

    QString tmpOldID = getPropertyID(property);
    if (!tmpOldID.isEmpty())
    {
        d_ptr->Properties.remove(tmpOldID);
    }

    if (parent_property)
    {
        parent_property->addChild(property);
    }
    else
    {
        d_ptr->RootProperties.append(property);
        if (property->getParent())
        {
            property->getParent()->removeChild(property);
        }
    }

    if (!id.isEmpty())
    {
        d_ptr->Properties.insert(id, property);
    }

    return true;
}

auto VPE::VPropertySet::hasProperty(VProperty *property) const -> bool
{
    if (!property)
    {
        return false;
    }

    return hasProperty(property, nullptr);
}

auto VPE::VPropertySet::getProperty(const QString &id) const -> VPE::VProperty *
{
    return d_ptr->Properties.value(id, nullptr);
}

auto VPE::VPropertySet::takeProperty(const QString &id) -> VPE::VProperty *
{
    VProperty* tmpProp = getProperty(id);
    removeProperty(tmpProp, false);

    // Return the property
    return tmpProp;
}

void VPE::VPropertySet::removeProperty(const QString &id)
{
    VProperty* tmpProp = takeProperty(id);
    delete tmpProp;
}

void VPE::VPropertySet::removeProperty(VProperty* prop, bool delete_property)
{
    // Remove all the children
    removePropertyFromSet(prop);

    // Remove from parent and optionally delete
    prop->setParent(nullptr);

    if (delete_property)
    {
        delete prop;
    }
}

auto VPE::VPropertySet::count() const -> vpesizetype
{
    return d_ptr->Properties.count();
}

void VPE::VPropertySet::clear(bool delete_properties)
{
    d_ptr->Properties.clear();
    while (!d_ptr->RootProperties.isEmpty())
    {
        VProperty* tmpProp = d_ptr->RootProperties.takeLast();
        if (tmpProp != nullptr && delete_properties)
        {
            delete tmpProp;
        }
    }
}

auto VPE::VPropertySet::getPropertyID(const VProperty *prop, bool look_for_parent_id) const -> QString
{
//    QString tmpResult;
    const VProperty* tmpCurrentProp = prop;

    while (tmpCurrentProp && (look_for_parent_id || prop == tmpCurrentProp) /*&& tmpResult.isEmpty()*/)
    {

        // todo: The following code doesn't work, because .key() doesn't accept a const VProperty* pointer ...
        //tmpResult = d_ptr->Properties.key(tmpCurrentProp, QString());

        // ... which is why we need the code below
        for (QMap<QString, VProperty*>::const_iterator i = d_ptr->Properties.constBegin();
             i != d_ptr->Properties.constEnd(); ++i)
        {
            if (tmpCurrentProp == (*i))
            {
                return i.key();
            }
        }

        tmpCurrentProp = tmpCurrentProp->getParent();
    }

//    return tmpResult;
    return QString();
}

// cppcheck-suppress unusedFunction
auto VPE::VPropertySet::getPropertiesMap() const -> const QMap<QString, VPE::VProperty *> &
{
    return d_ptr->Properties;
}

auto VPE::VPropertySet::getRootProperties() const -> const QList<VPE::VProperty *> &
{
    return d_ptr->RootProperties;
}

auto VPE::VPropertySet::getRootProperty(int row) const -> VPE::VProperty *
{
    return d_ptr->RootProperties.value(row, nullptr);
}

auto VPE::VPropertySet::getRootPropertyCount() const -> vpesizetype
{
    return d_ptr->RootProperties.count();
}

auto VPE::VPropertySet::clone() const -> VPE::VPropertySet *
{
    VPropertySet* tmpResult = new VPropertySet();

    const QList<VProperty*> rootProperties = d_ptr->RootProperties;
    for (auto *tmpProperty : rootProperties)
    {
        cloneProperty(tmpProperty, nullptr, tmpResult);
    }

    return tmpResult;
}

auto VPE::VPropertySet::hasProperty(VProperty *property, VProperty *parent) const -> bool
{
    if (!property)
    {
        return false;
    }

    const QList<VProperty*>& tmpChildrenList = (parent != nullptr ? parent->getChildren() : d_ptr->RootProperties);
    return std::any_of(tmpChildrenList.begin(), tmpChildrenList.end(), [this, property](VProperty* tmpProp)
                       {return tmpProp && (tmpProp == property || hasProperty(property, tmpProp));});
}

void VPE::VPropertySet::cloneProperty(VProperty* property_to_clone, VProperty *parent_property,
                                      VPropertySet *output_set) const
{
    if (!output_set || !property_to_clone || !hasProperty(property_to_clone))
    {
        return;
    }

    QString tmpID = getPropertyID(property_to_clone, false);

    // We want to clone the children ourselves (because of the IDs)
    VProperty* tmpNewProperty = property_to_clone->clone(false);

    output_set->addProperty(tmpNewProperty, tmpID, parent_property);
    for (int i = 0; i < property_to_clone->getRowCount(); ++i)
    {
        cloneProperty(property_to_clone->getChild(i), tmpNewProperty, output_set);
    }
}

void VPE::VPropertySet::removePropertyFromSet(VProperty *prop)
{
    // Remove all the children
    const QList<VPE::VProperty*>& children = prop->getChildren();
    for (auto *tmpChild : children)
    {
        removeProperty(tmpChild);
    }

    const QList<QString> tmpKeys = d_ptr->Properties.keys(prop);
    for (const auto &tmpID : tmpKeys)
    {
        d_ptr->Properties.remove(tmpID);
    }

    // Remove from list
    d_ptr->RootProperties.removeAll(prop);
}
