/************************************************************************
 **
 **  @file   vplacelabelitem.h
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
#ifndef VPLACELABELITEM_H
#define VPLACELABELITEM_H

#include <QSharedDataPointer>
#include <QTypeInfo>
#include <QtGlobal>
#include <QMetaType>

#include "vpointf.h"

class VPlaceLabelItemData;
class VContainer;
class QPainterPath;

class VPlaceLabelItem : public VPointF
{
public:
    VPlaceLabelItem();
    VPlaceLabelItem(const VPlaceLabelItem &item);
    virtual ~VPlaceLabelItem() override;

    auto operator=(const VPlaceLabelItem &item) -> VPlaceLabelItem &;
#ifdef Q_COMPILER_RVALUE_REFS
    VPlaceLabelItem(VPlaceLabelItem &&item) noexcept;
    auto operator=(VPlaceLabelItem &&item) noexcept->VPlaceLabelItem &;
#endif

    auto GetWidthFormula() const -> QString;
    auto GetWidthFormula() -> QString &;
    auto GetWidth() const -> qreal;
    void     SetWidth(qreal value, const QString &formula);

    auto GetHeightFormula() const -> QString;
    auto GetHeightFormula() -> QString &;
    auto GetHeight() const -> qreal;
    void     SetHeight(qreal value, const QString &formula);

    auto GetAngleFormula() const -> QString;
    auto GetAngleFormula() -> QString &;
    auto GetAngle() const -> qreal;
    void     SetAngle(qreal value, const QString &formula);

    auto GetVisibilityTrigger() const -> QString;
    auto GetVisibilityTrigger() -> QString &;
    auto IsVisible() const -> bool;
    void     SetVisibilityTrigger(qreal visible, const QString &formula);

    auto GetCorrectionAngle() const -> qreal;
    void     SetCorrectionAngle(qreal value);

    auto GetCenterPoint() const -> quint32;
    void    SetCenterPoint(quint32 id);

    auto GetLabelType() const -> PlaceLabelType;
    void           SetLabelType(PlaceLabelType type);

    auto RotationMatrix() const -> QTransform;
    auto Box() const -> QRectF;

private:
    QSharedDataPointer<VPlaceLabelItemData> d;
};

Q_DECLARE_METATYPE(VPlaceLabelItem)
Q_DECLARE_TYPEINFO(VPlaceLabelItem, Q_MOVABLE_TYPE); // NOLINT

#endif // VPLACELABELITEM_H
