/************************************************************************
 **
 **  @file   vproperty.h
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

#ifndef VPROPERTY_H
#define VPROPERTY_H

#include <QAbstractItemDelegate>
#include <QEvent>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>

#include "vpropertyexplorer_global.h"

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#include "vpropertydef.h"

namespace VPE
{

enum class Property : qint8
{
    Simple,
    Complex
};

static const int MyCustomEventType = 1099;

class VPROPERTYEXPLORERSHARED_EXPORT UserChangeEvent : public QEvent
{
public:
    UserChangeEvent()
      : QEvent(static_cast<QEvent::Type>(MyCustomEventType))
    {
    }
    ~UserChangeEvent() override;
};

class VPropertyPrivate;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VPROPERTYEXPLORERSHARED_EXPORT VProperty : public QObject
{
    Q_OBJECT // NOLINT

public:
    enum DPC_DisplayColumn
    {
        DPC_Name = 0,
        DPC_Data
    };

    //! Standard constructor, takes a name and a parent property as argument
    explicit VProperty(const QString &name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                       QMetaType::Type type = QMetaType::QString);
#else
                       QVariant::Type type = QVariant::String);
#endif

    //! Destructor
    virtual ~VProperty() override;

    //! Returns a string containing the type of the property
    virtual auto type() const -> QString;

    //! Get the data how it should be displayed
    virtual auto data(int column = DPC_Name, int role = Qt::DisplayRole) const -> QVariant;

    //! This is used by the model to set the data
    //! \param data The data to set
    //! \param role The role. Default is Qt::EditRole
    //! \return Returns true, if the data was changed, false if not.
    virtual auto setData(const QVariant &data, int role = Qt::EditRole) -> bool;

    //! This is called by the delegate when the property value is being drawn.
    //! The standard implementation doesn't do anything.
    //! If you reimplement this in a sub property, make sure to return true or the delegate will draw the item.
    virtual auto paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index,
                       const QAbstractItemDelegate *delegate) const -> bool;

    //! Returns an editor widget, or NULL if it doesn't supply one
    //! \param parent The widget to which the editor will be added as a child
    //! \options Render options
    //! \delegate A pointer to the QAbstractItemDelegate requesting the editor. This can be used to connect signals and
    //! slots.
    virtual auto createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                              const QAbstractItemDelegate *delegate) -> QWidget *;

    //! Sets the property's data to the editor (returns false, if the standard delegate should do that)
    virtual auto setEditorData(QWidget *editor) -> bool;

    //! Gets the data from the widget
    virtual auto getEditorData(const QWidget *editor) const -> QVariant;

    //! Returns item flags
    virtual auto flags(int column = DPC_Name) const -> Qt::ItemFlags;

    //! Sets the value of the property
    virtual void setValue(const QVariant &value);

    //! Returns the value of the property as a QVariant
    virtual auto getValue() const -> QVariant;

    //! Serializes the value to a string
    virtual auto serialize() const -> QString;

    //! Deserializes the value from a string
    virtual void deserialize(const QString &value);

    // The following functions are experimental and not yet implemented.
    /*//! Returns a pointer to the data stored and handled by this property. In most cases this function shouldn't be
    //! used.
    //! \return Returns a void pointer to the data. Not all properties have to support this. By default, this
    //! implementation returns a NULL pointer.
    virtual void* getDataPointer();

    //! Sets the data.
    //! \return Returns a void pointer to the data. Not all properties have to support this. By default, this
    //! implementation returns a NULL pointer.
    virtual bool setDataPointer(void* pointer);*/

    //! Sets the name of the property
    virtual void setName(const QString &name);

    //! Gets the name of the property
    virtual auto getName() const -> QString;

    //! Sets the name of the property
    virtual void setDescription(const QString &desc);

    //! Gets the name of the property
    virtual auto getDescription() const -> QString;

    //! Adds a child to this property
    virtual auto addChild(VProperty *child) -> vpesizetype;

    //! Returns a reference to the list of children
    virtual auto getChildren() -> QList<VProperty *> &;

    //! Returns a reference to the list of children
    virtual auto getChildren() const -> const QList<VProperty *> &;

    //! Returns the child at a certain row
    virtual auto getChild(int row) const -> VProperty *;

    //! Gets the number of children
    virtual auto getRowCount() const -> vpesizetype;

    //! Gets the parent of this property
    virtual auto getParent() const -> VProperty *;

    //! Sets the parent of this property
    virtual void setParent(VProperty *parent);

    //! Removes a child from the children list, doesn't delete the child!
    virtual void removeChild(VProperty *child);

    //! Returns the row the child has
    virtual auto getChildRow(VProperty *child) const -> vpesizetype;

    //! Returns whether the views have to update the parent of this property if it changes
    virtual auto getUpdateParent() const -> bool;

    //! Returns whether the views have to update the children of this property if it changes
    virtual auto getUpdateChildren() const -> bool;

    //! Sets whether the views should update Parents or children after this property changes
    virtual void setUpdateBehaviour(bool update_parent, bool update_children);

    //! Sets the settings by calling the overloaded setSetting(const QString& key, const QVariant& value) for each item
    //! in the map.
    virtual void setSettings(const QMap<QString, QVariant> &settings);

    //! Get the settings.
    virtual auto getSettings() const -> QMap<QString, QVariant>;

    //! Sets the settings. This function has to be implemented in a subclass in order to have an effect
    virtual void setSetting(const QString &key, const QVariant &value);

    //! Get the settings. This function has to be implemented in a subclass in order to have an effect
    virtual auto getSetting(const QString &key) const -> QVariant;

    //! Returns the list of keys of the property's settings
    virtual auto getSettingKeys() const -> QStringList;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    Q_REQUIRED_RESULT virtual auto clone(bool include_children = true, VProperty *container = nullptr) const
        -> VProperty *;

    auto propertyType() const -> Property;
    void setPropertyType(const Property &type);

    virtual void UpdateParent(const QVariant &value);
public slots:
    virtual void ValueChildChanged(const QVariant &value, int typeForParent);
signals:
    void childChanged(const QVariant &value, int typeForParent);

protected:
    //! Protected constructor
    explicit VProperty(VPropertyPrivate *d);

    //! The protected structure holding the member variables (to assure binary compatibility)
    VPropertyPrivate *vproperty_d_ptr;

private:
    // Provide access functions for the d_ptr
    Q_DECLARE_PRIVATE(VProperty)
    Q_DISABLE_COPY_MOVE(VProperty) // NOLINT
};

QT_WARNING_POP

} // namespace VPE

#endif // VPROPERTY_H
