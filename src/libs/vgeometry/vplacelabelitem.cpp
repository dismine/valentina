/************************************************************************
 **
 **  @file   vplacelabelitem.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2017
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
#include "vplacelabelitem.h"
#include "vplacelabelitem_p.h"
#include "../vpatterndb/vcontainer.h"

#include <qnumeric.h>
#include <QPolygonF>
#include <QTransform>
#include <QPainterPath>

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::VPlaceLabelItem()
    : VPointF(), d(new VPlaceLabelItemData)
{
    setType(GOType::PlaceLabel);
    setMode(Draw::Modeling);
}

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::VPlaceLabelItem(const VPlaceLabelItem &item)
    : VPointF(item), d(item.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::~VPlaceLabelItem()
{}

//---------------------------------------------------------------------------------------------------------------------
QString VPlaceLabelItem::GetWidthFormula() const
{
    return d->width;
}

//---------------------------------------------------------------------------------------------------------------------
QString &VPlaceLabelItem::GetWidthFormula()
{
    return d->width;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPlaceLabelItem::GetWidth() const
{
    return d->wValue;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetWidth(qreal value, const QString &formula)
{
    d->wValue = value;
    d->width = formula;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPlaceLabelItem::GetHeightFormula() const
{
    return d->height;
}

//---------------------------------------------------------------------------------------------------------------------
QString &VPlaceLabelItem::GetHeightFormula()
{
    return d->height;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPlaceLabelItem::GetHeight() const
{
    return d->hValue;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetHeight(qreal value, const QString &formula)
{
    d->hValue = value;
    d->height = formula;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPlaceLabelItem::GetAngleFormula() const
{
    return d->angle;
}

//---------------------------------------------------------------------------------------------------------------------
QString &VPlaceLabelItem::GetAngleFormula()
{
    return d->angle;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPlaceLabelItem::GetAngle() const
{
    return d->aValue;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetAngle(qreal value, const QString &formula)
{
    d->aValue = value;
    d->angle = formula;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPlaceLabelItem::GetVisibilityTrigger() const
{
    return d->visibilityTrigger;
}

//---------------------------------------------------------------------------------------------------------------------
QString &VPlaceLabelItem::GetVisibilityTrigger()
{
    return d->visibilityTrigger;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPlaceLabelItem::IsVisible() const
{
    bool visible = true;

    if (qIsInf(d->isVisible) || qIsNaN(d->isVisible))
    {
        qWarning() << QObject::tr("Visibility trigger contains error and will be ignored");
    }
    else if (qFuzzyIsNull(d->isVisible))
    {
        visible = false;
    }
    return visible;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetVisibilityTrigger(qreal visible, const QString &formula)
{
    d->visibilityTrigger = formula;
    d->isVisible = visible;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPlaceLabelItem::GetCorrectionAngle() const
{
    return d->correctionAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetCorrectionAngle(qreal value)
{
    d->correctionAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VPlaceLabelItem::GetCenterPoint() const
{
    return d->centerPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetCenterPoint(quint32 id)
{
    d->centerPoint = id;
}

//---------------------------------------------------------------------------------------------------------------------
PlaceLabelType VPlaceLabelItem::GetLabelType() const
{
    return d->type;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetLabelType(PlaceLabelType type)
{
    d->type = type;
}

//---------------------------------------------------------------------------------------------------------------------
QTransform VPlaceLabelItem::RotationMatrix() const
{
    QTransform t;
    t.translate(x(), y());
    t.rotate(-d->aValue-d->correctionAngle);
    t.translate(-x(), -y());
    return t;
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VPlaceLabelItem::Box() const
{
    return QRectF(0, 0, d->wValue, d->hValue);
}

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem &VPlaceLabelItem::operator=(const VPlaceLabelItem &item)
{
    if ( &item == this )
    {
        return *this;
    }
    VPointF::operator=(item);
    d = item.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::VPlaceLabelItem(VPlaceLabelItem &&item) Q_DECL_NOTHROW
    : VPointF(std::move(item)), d(std::move(item.d))
{}

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem &VPlaceLabelItem::operator=(VPlaceLabelItem &&item) Q_DECL_NOTHROW
{
    VPointF::operator=(item);
    std::swap(d, item.d);
    return *this;
}
#endif
