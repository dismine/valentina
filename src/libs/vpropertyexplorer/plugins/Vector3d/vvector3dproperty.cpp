/************************************************************************
 **
 **  @file   vvector3dproperty.cpp
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

#include "vvector3dproperty.h"

#include <QFlags>
#include <QList>
#include <QStringList>

#include "../../vproperty_p.h"
#include "../vnumberproperty.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

VPE::QVector3DProperty::QVector3DProperty(const QString &name)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              QMetaType::QString) // todo: QVariant::Vector3D??
#else
              QVariant::String) // todo: QVariant::Vector3D??
#endif
{
    QVariant tmpFloat(0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    tmpFloat.convert(QMetaType(QMetaType::Double));
#else
    tmpFloat.convert(QVariant::Double);
#endif
    auto *tmpX = new VDoubleProperty("X");
    addChild(tmpX);
    tmpX->setUpdateBehaviour(true, false);
    auto *tmpY = new VDoubleProperty("Y");
    addChild(tmpY);
    tmpY->setUpdateBehaviour(true, false);
    auto *tmpZ = new VDoubleProperty("Z");
    addChild(tmpZ);
    tmpZ->setUpdateBehaviour(true, false);
    VPE::QVector3DProperty::setVector(Vector3D());
}

//! Get the data how it should be displayed
auto VPE::QVector3DProperty::data(int column, int role) const -> QVariant
{
    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        Vector3D tmpVect = getVector();
        return u"(%1, %2, %3)"_s.arg(QString::number(tmpVect.X), QString::number(tmpVect.Y),
                                     QString::number(tmpVect.Z));
    }
    return VProperty::data(column, role);
}

//! Returns item flags
auto VPE::QVector3DProperty::flags(int column) const -> Qt::ItemFlags
{
    if (column == DPC_Name || column == DPC_Data)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return Qt::NoItemFlags;
}

//! Returns the Vector3d
auto VPE::QVector3DProperty::getVector() const -> VPE::Vector3D
{
    Vector3D tmpVect;

    if (d_ptr->Children.count() < 3)
    {
        return tmpVect;
    }

    tmpVect.X = d_ptr->Children.at(0)->getValue().toDouble();
    tmpVect.Y = d_ptr->Children.at(1)->getValue().toDouble();
    tmpVect.Z = d_ptr->Children.at(2)->getValue().toDouble();

    return tmpVect;
}

//! Sets the Vector3d
void VPE::QVector3DProperty::setVector(const Vector3D &vect)
{
    setVector(vect.X, vect.Y, vect.Z);
}

void VPE::QVector3DProperty::setVector(double x, double y, double z)
{
    if (d_ptr->Children.count() < 3)
    {
        return;
    }

    QVariant tmpX(x);
    QVariant tmpY(y);
    QVariant tmpZ(z);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    tmpX.convert(QMetaType(QMetaType::Double));
    tmpY.convert(QMetaType(QMetaType::Double));
    tmpZ.convert(QMetaType(QMetaType::Double));
#else
    tmpX.convert(QVariant::Double);
    tmpY.convert(QVariant::Double);
    tmpZ.convert(QVariant::Double);
#endif
    d_ptr->Children.at(0)->setValue(tmpX);
    d_ptr->Children.at(1)->setValue(tmpY);
    d_ptr->Children.at(2)->setValue(tmpZ);
}

auto VPE::QVector3DProperty::type() const -> QString
{
    return "vector3d";
}

auto VPE::QVector3DProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    if (!container)
    {
        container = new QVector3DProperty(getName());

        if (!include_children)
        {
            const QList<VProperty *> &tmpChildren = container->getChildren();
            for (auto *tmpChild : tmpChildren)
            {
                container->removeChild(tmpChild);
                delete tmpChild;
            }
        }
    }

    return VProperty::clone(false, container); // Child
}

void VPE::QVector3DProperty::setValue(const QVariant &value)
{
    QStringList tmpStrings = value.toString().split(QChar(','));
    if (tmpStrings.count() == 3)
    {
        setVector(tmpStrings[0].toDouble(), tmpStrings[1].toDouble(), tmpStrings[2].toDouble());
    }
}

auto VPE::QVector3DProperty::getValue() const -> QVariant
{
    Vector3D tmpVect = getVector();
    return u"%1,%2,%3"_s.arg(QString::number(tmpVect.X), QString::number(tmpVect.Y), QString::number(tmpVect.Z));
}
