/************************************************************************
 **
 **  @file   vvector3dproperty.h
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

#ifndef VVECTOR3DPROPERTY_H
#define VVECTOR3DPROPERTY_H


#include <stddef.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "../../vproperty.h"
#include "../../vpropertyexplorer_global.h"

namespace VPE
{

struct VPROPERTYEXPLORERSHARED_EXPORT Vector3D //-V690
{
public:
    Vector3D()
        :X(0), Y(0), Z(0)
    {}

    Vector3D(const Vector3D& other)
        :X(other.X), Y(other.Y), Z(other.Z)
    {}

    auto operator=(const Vector3D &other) -> Vector3D &
    {
        if ( &other == this )
        {
            return *this;
        }
        X = other.X;
        Y = other.Y;
        Z = other.Z;
        return *this;
    }

    ~Vector3D() {}

    double X, Y, Z;
};
/*
}

Q_DECLARE_METATYPE(QPE::Vector3D)   // todo

*/

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

class VPROPERTYEXPLORERSHARED_EXPORT QVector3DProperty : public VProperty
{
    Q_OBJECT // NOLINT
public:
    explicit QVector3DProperty(const QString& name);

    virtual ~QVector3DProperty() override {}

    //! Get the data how it should be displayed
    virtual auto data(int column = DPC_Name, int role = Qt::DisplayRole) const -> QVariant override;

    //! Returns item flags
    virtual auto flags(int column = DPC_Name) const -> Qt::ItemFlags override;

    //! Returns the Vector3d
    virtual auto getVector() const -> Vector3D;

    //! Sets the Vector3d
    virtual void setVector(const Vector3D& vect);

    //! Sets the Vector3d
    virtual void setVector(double x, double y, double z);

    //! Returns a string containing the type of the property
    virtual auto type() const -> QString override;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    virtual auto clone(bool include_children = true, VProperty *container = NULL) const -> VProperty * override;

    //! Sets the value of the property
    virtual void setValue(const QVariant& value) override;

    //! Returns the value of the property as a QVariant
    virtual auto getValue() const -> QVariant override;

private:
    Q_DISABLE_COPY_MOVE(QVector3DProperty) // NOLINT
};

QT_WARNING_POP

}

#endif // VVECTOR3DPROPERTY_H
