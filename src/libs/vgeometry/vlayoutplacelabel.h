/************************************************************************
 **
 **  @file   vlayoutplacelabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 10, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef VLAYOUTPLACELABEL_H
#define VLAYOUTPLACELABEL_H

#include <QPolygonF>

#include "vgeometrydef.h"
#include "vplacelabelitem.h"

class VLayoutPlaceLabel
{
public:
    VLayoutPlaceLabel() = default;
    explicit VLayoutPlaceLabel(const VPlaceLabelItem &item);

    friend auto operator<<(QDataStream &dataStream, const VLayoutPlaceLabel &data) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VLayoutPlaceLabel &data) -> QDataStream &;

    auto Center() const -> QPointF;
    void SetCenter(QPointF newCenter);

    auto Type() const -> PlaceLabelType;
    void SetType(PlaceLabelType newType);

    auto RotationMatrix() const -> const QTransform &;
    void SetRotationMatrix(const QTransform &newRotationMatrix);

    auto Box() const -> const QRectF &;
    void SetBox(const QRectF &newBox);

    auto IsNotMirrored() const -> bool;
    void SetNotMirrored(bool newNotMirrored);

private:
    static constexpr quint32 streamHeader = 0xB282E284; // CRC-32Q string "VLayoutPlaceLabel"
    static constexpr quint16 classVersion = 3;

    QPointF m_center{};
    PlaceLabelType m_type{PlaceLabelType::Button};
    QTransform m_rotationMatrix{};
    QRectF m_box{};
    bool m_notMirrored{false};
};

Q_DECLARE_METATYPE(VLayoutPlaceLabel)                  // NOLINT
Q_DECLARE_TYPEINFO(VLayoutPlaceLabel, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutPlaceLabel::Center() const -> QPointF
{
    return m_center;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutPlaceLabel::SetCenter(QPointF newCenter)
{
    m_center = newCenter;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutPlaceLabel::Type() const -> PlaceLabelType
{
    return m_type;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutPlaceLabel::SetType(PlaceLabelType newType)
{
    m_type = newType;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutPlaceLabel::RotationMatrix() const -> const QTransform &
{
    return m_rotationMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutPlaceLabel::SetRotationMatrix(const QTransform &newRotationMatrix)
{
    m_rotationMatrix = newRotationMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutPlaceLabel::Box() const -> const QRectF &
{
    return m_box;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutPlaceLabel::SetBox(const QRectF &newBox)
{
    m_box = newBox;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutPlaceLabel::IsNotMirrored() const -> bool
{
    return m_notMirrored;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutPlaceLabel::SetNotMirrored(bool newNotMirrored)
{
    m_notMirrored = newNotMirrored;
}

#endif // VLAYOUTPLACELABEL_H
