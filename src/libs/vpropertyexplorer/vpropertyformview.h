/************************************************************************
 **
 **  @file   vpropertyformview.h
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

#ifndef VPROPERTYFORMVIEW_H
#define VPROPERTYFORMVIEW_H

#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtGlobal>

#include "vpropertyexplorer_global.h"
#include "vpropertyformwidget.h"

namespace VPE
{

class VPropertyModel;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

//! This class populates a form layout with the properties in a model
class VPROPERTYEXPLORERSHARED_EXPORT VPropertyFormView : public VPropertyFormWidget
{
    Q_OBJECT // NOLINT

public:
    //! Constructor
    explicit VPropertyFormView(QWidget *parent = nullptr);

    //! Constructor
    explicit VPropertyFormView(VPropertyModel *model, QWidget *parent = nullptr);

    //! Constructor
    explicit VPropertyFormView(VPropertySet *property_set, QWidget *parent = nullptr);

    //! Destructor
    virtual ~VPropertyFormView() override;

public slots:
    //! Rebuilds the whole form
    virtual void build() override;

    //! Set the source model. Leads to the rebuild of the form
    //! \param model The model to use
    void setModel(VPropertyModel *model);

    //! Set the property set to use. Note that if using a property set directly, adding and removing properties to the
    //! property set leads to undifined behaviour for the property set misses notification signals.
    //! \param property_set The property set to use
    void setPropertySet(VPropertySet *property_set);

    //! Called when a row gets removed in the model
    void rowsRemoved(const QModelIndex &parent, int start, int end);

    //! Called when a new row is being inserted into the model
    void rowsInserted(const QModelIndex &parent, int start, int end);

    //! Called when the model is being reset
    void modelReset();

private slots:
    //! Called, when the model gets destroyed
    void modelDestroyed();

    //! Called when data in the model gets changed
    void dataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right);

    //! Updates the model when data was submitted by the view
    void dataSubmitted(VProperty *property);

protected:
    void showEvent(QShowEvent *event) override;

    //! Rebuilds the widegt only if it is visible
    void updatePropertyList();

    //! Removes the model and property set if they were set
    void removeModelAndSet();

    //! Function to handle newly created VPropertyWidgets
    virtual void connectPropertyFormWidget(VPropertyFormWidget *widget);

private:
    Q_DISABLE_COPY_MOVE(VPropertyFormView) // NOLINT

    void rowsChanged(const QModelIndex &parent, int start, int end);
};

QT_WARNING_POP

} // Namespace VPE

#endif // VPROPERTYFORMVIEW_H
