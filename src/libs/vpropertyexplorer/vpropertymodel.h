/************************************************************************
 **
 **  @file   vpropertymodel.h
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

#ifndef VPROPERTYMODEL_H
#define VPROPERTYMODEL_H


#include <QAbstractItemModel>
#include <QMap>
#include <QMetaObject>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "vproperty.h"
#include "vpropertyexplorer_global.h"

namespace VPE
{

class VPropertyModelPrivate;
class VPropertySet;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

//! \brief  This is the base model for managing all the properties
//!         and passing them to the view.
//!
//! When you create your own "proxy models", this is the place to
//! start: just subclass VPropertyModel and extend the new class.
//! Have a look at existing examples of proxies.
//!
//! <strong>Note that in this context, the term "proxy model" does not refer
//! to VProxyModel as that is another concept.</strong>
//! The idea behind "proxy models" in the QtPropertyExplorer framework
//! is to provide an convenient interface which takes data as your
//! application (or a third-party-library) provides it, and converts this
//! data to VProperty-objects, manage them and produce output for the views.
//!
//! In most cases, you will not need to rewrite the basic functions of
//! QAbstractItemModel, as VPropertyModel provides standard implementations
//! to work with. Thus, instead of subclassing VPropertyModel, it is also
//! possible to use VPropertyModel directly (as it is not an abstract class).
//! This might be more convenient in some cases.
class VPROPERTYEXPLORERSHARED_EXPORT VPropertyModel : public QAbstractItemModel
{
    Q_OBJECT // NOLINT
public:
    explicit VPropertyModel(QObject * parent = nullptr);
    virtual ~VPropertyModel() override;

    //! Adds the property to the model and attaches it to the parentid
    //! \param emitsignals If this is set to false, this function will not call beginInsertRows() and endInsertRows(),
    //! so it has to be called from a subclass
    virtual auto addProperty(VProperty *property, const QString &id, const QString &parentid = QString(),
                             bool emitsignals = true) -> bool;

    //! Creates a property and adds it to the model
    virtual auto createProperty(const QString &id, const QString &name, const QString &parentid = QString(),
                                const QVariant &data = QVariant()) -> VProperty *;

    //! Gets a property by it's ID
    virtual auto getProperty(const QString &id) -> VProperty *;

    //! Returns the item flags for the given index
    virtual auto flags(const QModelIndex &index) const -> Qt::ItemFlags override;

    //! Sets the role data for the item at index to value
    virtual auto setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) -> bool override;

    //! Returns the model index at row/column
    virtual auto index(int row, int column, const QModelIndex &parent = QModelIndex()) const -> QModelIndex override;

    //! Returns the parent of one model index
    virtual auto parent(const QModelIndex &index) const -> QModelIndex override;

    //! Returns the data of an model index
    virtual auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant override;

    //! Returns the data for the given role and section in the header with the specified orientation.
    virtual auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
        -> QVariant override;

    //! Returns the number of rows
    virtual auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int override;

    //! Returns the number of columns
    virtual auto columnCount(const QModelIndex &parent = QModelIndex()) const -> int override;

    //! Gets a property by its ModelIndex
    //! \param index The modelIndex of the property.
    //! \return Returns the property with the given index, or NULL if none such property exists
    virtual auto getProperty(const QModelIndex &index) const -> VProperty *;

    //! Returns the ID of the property within the model
    //! The concept of property IDs is, that the object that manages the properties
    //! and not the properties themselves handle the IDs.
    //! \return Returns the ID under which the property is stored within the model
    virtual auto getPropertyID(const VProperty *prop) const -> QString;

    //! Returns a const pointer to the property set managed by this model. If you want to manipulate the property set,
    //! either use the methods provided by the model or use takePropertySet() and setPropertySet().
    //! \return A constant pointer to the property set or NULL if there currently is none.
    virtual auto getPropertySet() const -> const VPropertySet *;

    //! Clears the model, deletes the property set managed by this model.
    //! \param emit_signals Default: true. Set this to false if you want to prevent the model from emmiting the reset
    //! model signals
    virtual void clear(bool emit_signals = true);

    //! Removes the current property set and returns it. If new_property_set is set, the old one will be replaced by the
    //! new one
    //! \param new_property_set The new property set to replace the old one with. Default: NULL
    //! \param emit_signals Default: true. Set this to false if you want to prevent the model from emmiting the reset
    //! model signals
    //! \return A constant pointer to the property set or NULL if there currently is none.
    virtual auto takePropertySet(VPropertySet *new_property_set = nullptr, bool emit_signals = true) -> VPropertySet *;

    //! Sets a new property set. The model will take ownership of the property set. The old property set will be
    //! deleted.
    //! \param property_set The new property set. Setting this to NULL has the same effect as calling clear.
    //! \param emit_signals Default: true. Set this to false if you want to prevent the model from emmiting the reset
    //! model signals
    virtual void setPropertySet(VPropertySet* property_set, bool emit_signals = true);

    //! Removes a property from the model and returns it
    virtual auto takeProperty(const QString &id) -> VProperty *;

    //! Removes a property from the model and deletes it
    virtual void removeProperty(const QString& id);

signals:
    //! This signal is being emitted, when the setData method is being called
    void onDataChangedByEditor(VProperty* property);

public slots:
    //! This function causes the views to update the property
    void onDataChangedByModel(VProperty* property);

protected:
    //! Gets a property by its ModelIndex
    virtual auto getIndexFromProperty(VProperty *property, int column = 0) const -> QModelIndex;

    //! Protected constructor passing the private object
    explicit VPropertyModel(VPropertyModelPrivate* d, QObject* parent = nullptr);

    //! The model data
    VPropertyModelPrivate* d_ptr;

private:
    Q_DISABLE_COPY_MOVE(VPropertyModel) // NOLINT
};

QT_WARNING_POP

}

#endif // VPROPERTYMODEL_H
