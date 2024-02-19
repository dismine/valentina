/************************************************************************
 **
 **  @file   vpropertyfactorymanager.cpp
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

#include "vpropertyfactorymanager.h"

#include <QList>
#include <QMap>
#include <QStringList>
#include <stddef.h>

#include "vabstractpropertyfactory.h"
#include "vproperty.h"
#include "vpropertyfactorymanager_p.h"
#include "vstandardpropertyfactory.h"

VPE::VPropertyFactoryManager *VPE::VPropertyFactoryManager::DefaultManager = nullptr;

VPE::VPropertyFactoryManager::VPropertyFactoryManager(QObject *parent)
  : QObject(parent),
    d_ptr(new VPropertyFactoryManagerPrivate())
{
}

VPE::VPropertyFactoryManager::~VPropertyFactoryManager()
{
    // Delete all factories
    QList<VAbstractPropertyFactory *> tmpFactories = d_ptr->Factories.values();
    while (!tmpFactories.isEmpty())
    {
        VAbstractPropertyFactory *tmpFactory = tmpFactories.takeLast();
        tmpFactories.removeAll(tmpFactory);
        delete tmpFactory;
    }

    delete d_ptr;
    if (this == DefaultManager)
    {
        DefaultManager = nullptr;
    }
}

void VPE::VPropertyFactoryManager::registerFactory(const QString &type, VAbstractPropertyFactory *factory)
{
    if (type.isEmpty())
    {
        return;
    }

    // Remove old factory
    unregisterFactory(getFactory(type), type, true);
    // Register new one
    d_ptr->Factories[type] = factory;
}

void VPE::VPropertyFactoryManager::unregisterFactory(VAbstractPropertyFactory *factory, const QString &type,
                                                     bool delete_if_unused)
{
    if (!factory)
    {
        return;
    }

    if (!type.isEmpty())
    {
        // Remove all occurances
        QString tmpKey;
        do
        {
            tmpKey = d_ptr->Factories.key(factory, QString());
            if (!tmpKey.isEmpty())
            {
                d_ptr->Factories.remove(tmpKey);
            }
        } while (!tmpKey.isEmpty());
    }
    else
    {
        // Only remove one type
        if (d_ptr->Factories.value(type, nullptr) == factory)
        {
            d_ptr->Factories.remove(type);
        }
    }

    if (delete_if_unused && !isRegistered(factory))
    {
        delete factory;
    }
}

auto VPE::VPropertyFactoryManager::isRegistered(VAbstractPropertyFactory *factory) -> bool
{
    return (!d_ptr->Factories.key(factory, QString()).isEmpty());
}

auto VPE::VPropertyFactoryManager::getFactory(const QString &type) -> VPE::VAbstractPropertyFactory *
{
    return d_ptr->Factories.value(type, nullptr);
}

auto VPE::VPropertyFactoryManager::createProperty(const QString &type, const QString &name, const QString &description,
                                                  const QString &default_value) -> VPE::VProperty *
{
    VAbstractPropertyFactory *tmpFactory = getFactory(type);
    VProperty *tmpResult = nullptr;
    if (tmpFactory)
    {
        tmpResult = tmpFactory->createProperty(type, name);

        if (tmpResult)
        {
            tmpResult->setDescription(description);

            if (!default_value.isEmpty())
            {
                tmpResult->deserialize(default_value);
            }
        }
    }

    return tmpResult;
}

// cppcheck-suppress unusedFunction
// VPE::VPropertyFactoryManager *VPE::VPropertyFactoryManager::getDefaultManager()
//{
//    if (!DefaultManager)
//    {
//        DefaultManager = new VPropertyFactoryManager();
//        /*VStandardPropertyFactory* tmpStandardProp = */new VStandardPropertyFactory(DefaultManager);
//    }

//    return DefaultManager;
//}

// cppcheck-suppress unusedFunction
auto VPE::VPropertyFactoryManager::getSupportedTypes() -> QStringList
{
    return d_ptr->Factories.keys();
}
