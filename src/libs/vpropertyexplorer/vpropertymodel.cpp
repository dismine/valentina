/************************************************************************
 **
 **  @file   vpropertymodel.cpp
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

#include "vpropertymodel.h"

#include <QList>

#include "vproperty.h"
#include "vpropertyset.h"
#include "vpropertydef.h"

#include "vpropertymodel_p.h"

VPE::VPropertyModel::VPropertyModel(VPropertyModelPrivate *d, QObject *parent)
    : QAbstractItemModel(parent), d_ptr(d)
{
}


VPE::VPropertyModel::VPropertyModel(QObject * parent) :
    QAbstractItemModel(parent), d_ptr(new VPropertyModelPrivate())
{
}

VPE::VPropertyModel::~VPropertyModel()
{
    delete d_ptr->Properties;
    delete d_ptr;
}

//! Adds the property to the model and attaches it to the parentid
auto VPE::VPropertyModel::addProperty(VProperty *property, const QString &id, const QString &parentid, bool emitsignals)
    -> bool
{
    if (!property)
    {
        return false;
    }

    if (!d_ptr->Properties) // If not existant, create property set
    {
        d_ptr->Properties = new VPropertySet();
    }

    if (emitsignals)
    {
        VProperty* tmpParent = getProperty(parentid);
        vpesizetype const tmpRow =
            tmpParent != nullptr ? tmpParent->getRowCount() : d_ptr->Properties->getRootPropertyCount();
        beginInsertRows((tmpParent != nullptr ? getIndexFromProperty(tmpParent) : QModelIndex()),
                        static_cast<int>(tmpRow), static_cast<int>(tmpRow));
    }

    d_ptr->Properties->addProperty(property, id, parentid);

    if (emitsignals)
    {
        endInsertRows();
    }

    return true;

}

//! Creates a property and adds it to the model
auto VPE::VPropertyModel::createProperty(const QString &id, const QString &name, const QString &parentid,
                                         const QVariant &data) -> VPE::VProperty *
{
    auto *tmpProp = new VProperty(name);
    tmpProp->setValue(data);
    if (addProperty(tmpProp, id, parentid))
    {
        return tmpProp;
    }
    else
        return nullptr;
}

//! Gets a property by it's ID
auto VPE::VPropertyModel::getProperty(const QString &id) -> VPE::VProperty *
{
    return d_ptr->Properties != nullptr ? d_ptr->Properties->getProperty(id) : nullptr;
}

//! Returns the model index at row/column
auto VPE::VPropertyModel::index(int row, int column, const QModelIndex &parent) const -> QModelIndex
{
    if (d_ptr->Properties == nullptr || (parent.isValid() && parent.column() > 1))
    {
            return QModelIndex();
    }

    if (parent.isValid())
    {
        // Get the parent index
        VProperty* parentItem = getProperty(parent);
        if (parentItem)
        {
            VProperty* childItem = parentItem->getChild(row);
            if (childItem)
            {
                return createIndex(row, column, childItem);
            }
        }
    }
    else if (row >= 0 && row < d_ptr->Properties->count())
    {
        return createIndex(row, column, d_ptr->Properties->getRootProperty(row));
    }

    return QModelIndex();
}

//! Returns the parent of one model index
auto VPE::VPropertyModel::parent(const QModelIndex &index) const -> QModelIndex
{
    if (!index.isValid())
    {
        return {};
    }

    VProperty* childItem = getProperty(index);
    if (childItem)
    {
        VProperty* parentItem = childItem->getParent();
        if (parentItem)
        {
            VProperty* grandParentItem = parentItem->getParent();
            vpesizetype const parents_row = grandParentItem != nullptr
                                                ? grandParentItem->getChildRow(parentItem)
                                                : d_ptr->Properties->getRootProperties().indexOf(parentItem);

            if (parents_row >= 0)
            {
                return createIndex(static_cast<int>(parents_row), 0, parentItem);
            }
        }
    }

    return {};
}

//! Returns the item flags for the given index
auto VPE::VPropertyModel::flags(const QModelIndex &index) const -> Qt::ItemFlags
{
    VProperty* tmpProperty = getProperty(index);
    if (!tmpProperty)
    {
        return Qt::NoItemFlags;
    }
    else
        return tmpProperty->flags(index.column());
}

//! Sets the role data for the item at index to value
auto VPE::VPropertyModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool
{
    VProperty* tmpProperty = getProperty(index);
    if (index.column() == 1 && tmpProperty)
    {
        bool const tmpHasChanged = tmpProperty->setData(value, role);
        if (tmpProperty->getUpdateParent() && tmpHasChanged)
        {   // If neccessary, update the parent as well
            QModelIndex const tmpParentIndex = parent(index);
            emit dataChanged(tmpParentIndex, tmpParentIndex);
        }

        if (tmpHasChanged)
        {
            emit onDataChangedByEditor(tmpProperty);
        }
    }


    return true;
}


//! Returns the data of an model index
auto VPE::VPropertyModel::data(const QModelIndex &index, int role) const -> QVariant
{
    VProperty* tmpProperty = getProperty(index);
    if (!tmpProperty)
    {
        return QVariant();
    }
    else
        return tmpProperty->data(index.column(), role);
}

auto VPE::VPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        // Header data
        if (section == 0)
        {
            return d_ptr->HeadlineProperty;
        }
        else if (section == 1)
        {
            return d_ptr->HeadlineValue;
        }
    }
    else if (role == Qt::DisplayRole)
    {
        return QVariant(section);
    }

    return QVariant();
}


//! Returns the number of rows
auto VPE::VPropertyModel::rowCount(const QModelIndex &parent) const -> int
{
    if (parent.isValid())
    {
        VProperty* tmpParent = getProperty(parent);
        if (tmpParent)
        {
            return static_cast<int>(tmpParent->getRowCount());
        }
    }

    // Return the root property count
    if (d_ptr->Properties)
    {
        return static_cast<int>(d_ptr->Properties->getRootPropertyCount());
    }

    return 0;
}


//! Returns the number of columns
auto VPE::VPropertyModel::columnCount(const QModelIndex &parent) const -> int
{
    Q_UNUSED(parent)
    return 2;
}


//! Gets a property by its ModelIndex
auto VPE::VPropertyModel::getProperty(const QModelIndex &index) const -> VPE::VProperty *
{
    if (index.isValid())
    {
        auto *prop = static_cast<VProperty *>(index.internalPointer());

        if (prop)
        {
            return prop;
        }
    }
    return nullptr;
}

auto VPE::VPropertyModel::getPropertyID(const VProperty *prop) const -> QString
{
    return d_ptr->Properties != nullptr ? d_ptr->Properties->getPropertyID(prop) : QString();
}

auto VPE::VPropertyModel::getIndexFromProperty(VProperty *property, int column) const -> QModelIndex
{
    if (!property || column > columnCount() || column < 0)
    {
        return {};
    }

    VProperty* parentItem = property->getParent();
    vpesizetype row = 0;

    if (parentItem)
    {
        row = parentItem->getChildRow(property);
    }

    return createIndex(static_cast<int>(row), column, property);
}


void VPE::VPropertyModel::onDataChangedByModel(VProperty* property)
{
    QModelIndex const tmpIndex = getIndexFromProperty(property, 1);
    if (tmpIndex.isValid())
    {
        emit dataChanged(tmpIndex, tmpIndex);
        emit onDataChangedByEditor(property);
    }
}

auto VPE::VPropertyModel::getPropertySet() const -> const VPE::VPropertySet *
{
    return d_ptr->Properties;
}

void VPE::VPropertyModel::clear(bool emit_signals)
{
    setPropertySet(nullptr, emit_signals);
}

auto VPE::VPropertyModel::takePropertySet(VPropertySet *new_property_set, bool emit_signals) -> VPE::VPropertySet *
{
    VPropertySet* tmpOldPropertySet = d_ptr->Properties;

    if (emit_signals)
    {
        beginResetModel();
    }
    d_ptr->Properties = new_property_set;
    if (emit_signals)
    {
        endResetModel();
    }

    return tmpOldPropertySet;
}

void VPE::VPropertyModel::setPropertySet(VPropertySet *property_set, bool emit_signals)
{
    VPropertySet* tmpOldPropertySet = takePropertySet(property_set, emit_signals);
    delete tmpOldPropertySet;
}

auto VPE::VPropertyModel::takeProperty(const QString &id) -> VPE::VProperty *
{
    QModelIndex const tmpIndex = getIndexFromProperty(getProperty(id));
    if (d_ptr->Properties && tmpIndex.isValid())
    {
        beginRemoveRows(tmpIndex.parent(), tmpIndex.row(), tmpIndex.row());
        VProperty* tmpProp = d_ptr->Properties->takeProperty(id);
        endRemoveRows();
        return tmpProp;
    }

    return nullptr;
}

void VPE::VPropertyModel::removeProperty(const QString &id)
{
    QModelIndex const tmpIndex = getIndexFromProperty(getProperty(id));
    if (d_ptr->Properties && tmpIndex.isValid())
    {
        beginRemoveRows(tmpIndex.parent(), tmpIndex.row(), tmpIndex.row());
        d_ptr->Properties->removeProperty(id);
        endRemoveRows();
    }
}
