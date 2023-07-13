/************************************************************************
 **
 **  @file   vemptyproperty.h
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

#ifndef VEMPTYPROPERTY_H
#define VEMPTYPROPERTY_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "../vproperty.h"
#include "../vpropertyexplorer_global.h"

namespace VPE
{
class VPropertyPrivate;
} // namespace VPE

namespace VPE
{

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

class VPROPERTYEXPLORERSHARED_EXPORT VEmptyProperty : public VProperty
{
    Q_OBJECT // NOLINT

public:
    //! Standard constructor, takes a name and a parent property as argument
    explicit VEmptyProperty(const QString &name);

    //! Destructor
    virtual ~VEmptyProperty() override;

    //! Get the data how it should be displayed
    virtual auto data(int column = DPC_Name, int role = Qt::DisplayRole) const -> QVariant override;

    //! Returns an editor widget, or NULL if it doesn't supply one
    //! \param parent The widget to which the editor will be added as a child
    //! \options Render options
    //! \delegate A pointer to the QAbstractItemDelegate requesting the editor. This can be used to connect signals and
    //! slots.
    virtual auto createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                              const QAbstractItemDelegate *delegate) -> QWidget * override;

    //! Gets the data from the widget
    virtual auto getEditorData(const QWidget *editor) const -> QVariant override;

    //! Returns item flags
    virtual auto flags(int column = DPC_Name) const -> Qt::ItemFlags override;

    //! Returns a string containing the type of the property
    virtual auto type() const -> QString override;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    Q_REQUIRED_RESULT virtual auto clone(bool include_children = true, VProperty *container = nullptr) const
        -> VProperty * override;

protected:
    //! Protected constructor
    explicit VEmptyProperty(VPropertyPrivate *d);

private:
    Q_DISABLE_COPY_MOVE(VEmptyProperty) // NOLINT
};

QT_WARNING_POP

} // namespace VPE

#endif // VEMPTYPROPERTY_H
