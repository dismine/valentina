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
#include "../vpatterndb/vcontainer.h"
#include "vplacelabelitem_p.h"

#include <QPainterPath>
#include <QPolygonF>
#include <QTransform>
#include <qnumeric.h>

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::VPlaceLabelItem()
  : d(new VPlaceLabelItemData)
{
    setType(GOType::PlaceLabel);
    setMode(Draw::Modeling);
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VPlaceLabelItem, VPointF)

//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::~VPlaceLabelItem() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetWidthFormula() const -> QString
{
    return d->width;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetWidthFormula() -> QString &
{
    return d->width;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetWidth() const -> qreal
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
auto VPlaceLabelItem::GetHeightFormula() const -> QString
{
    return d->height;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetHeightFormula() -> QString &
{
    return d->height;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetHeight() const -> qreal
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
auto VPlaceLabelItem::GetAngleFormula() const -> QString
{
    return d->angle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetAngleFormula() -> QString &
{
    return d->angle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetAngle() const -> qreal
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
auto VPlaceLabelItem::GetVisibilityTrigger() const -> QString
{
    return d->visibilityTrigger;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetVisibilityTrigger() -> QString &
{
    return d->visibilityTrigger;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::IsVisible() const -> bool
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
auto VPlaceLabelItem::GetCorrectionAngle() const -> qreal
{
    return d->correctionAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetCorrectionAngle(qreal value)
{
    d->correctionAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetCenterPoint() const -> quint32
{
    return d->centerPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetCenterPoint(quint32 id)
{
    d->centerPoint = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::GetLabelType() const -> PlaceLabelType
{
    return d->type;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlaceLabelItem::SetLabelType(PlaceLabelType type)
{
    d->type = type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::RotationMatrix() const -> QTransform
{
    QTransform t;
    t.translate(x(), y());
    t.rotate(-d->aValue - d->correctionAngle);
    t.translate(-x(), -y());
    return t;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::Box() const -> QRectF
{
    return QRectF(0, 0, d->wValue, d->hValue);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::operator=(const VPlaceLabelItem &item) -> VPlaceLabelItem &
{
    if (&item == this)
    {
        return *this;
    }
    VPointF::operator=(item);
    d = item.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPlaceLabelItem::VPlaceLabelItem(VPlaceLabelItem &&item) noexcept
  : VPointF(std::move(item)),
    d(std::move(item.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlaceLabelItem::operator=(VPlaceLabelItem &&item) noexcept -> VPlaceLabelItem &
{
    VPointF::operator=(item);
    std::swap(d, item.d);
    return *this;
}
#endif
