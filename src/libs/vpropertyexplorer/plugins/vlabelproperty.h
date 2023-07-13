/************************************************************************
 **
 **  @file   vlabelproperty.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 3, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VLABELPROPERTY_H
#define VLABELPROPERTY_H


#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QtGlobal>

#include "../vproperty.h"
#include "../vpropertyexplorer_global.h"

namespace VPE
{


//! Class for holding a string property
class VPROPERTYEXPLORERSHARED_EXPORT VLabelProperty : public VProperty
{
    Q_OBJECT // NOLINT
public:
    VLabelProperty(const QString& name, const QMap<QString, QVariant>& settings);

    explicit VLabelProperty(const QString& name);

    //! Returns an editor widget, or NULL if it doesn't supply one
    //! \param parent The widget to which the editor will be added as a child
    //! \options Render options
    //! \delegate A pointer to the QAbstractItemDelegate requesting the editor. This can be used to connect signals and
    //! slots.
    virtual auto createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                              const QAbstractItemDelegate *delegate) -> QWidget * override;

    //! Sets the property's data to the editor (returns false, if the standard delegate should do that)
    virtual auto setEditorData(QWidget *editor) -> bool override;

    //! Gets the data from the widget
    virtual auto getEditorData(const QWidget *editor) const -> QVariant override;

    //! Sets the settings.
    virtual void setSetting(const QString& key, const QVariant& value) override;

    //! Get the settings. This function has to be implemented in a subclass in order to have an effect
    virtual auto getSetting(const QString &key) const -> QVariant override;

    //! Returns the list of keys of the property's settings
    virtual auto getSettingKeys() const -> QStringList override;

    //! Returns a string containing the type of the property
    virtual auto type() const -> QString override;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    Q_REQUIRED_RESULT virtual auto clone(bool include_children = true, VProperty *container = nullptr) const
        -> VProperty * override;

    virtual void UpdateParent(const QVariant &value) override;

    auto getTypeForParent() const -> int;
    void setTypeForParent(int value);

protected:
    int typeForParent;

private:
    Q_DISABLE_COPY_MOVE(VLabelProperty) // NOLINT
};

}

#endif // VLABELPROPERTY_H
