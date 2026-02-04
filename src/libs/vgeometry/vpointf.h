/************************************************************************
 **
 **  @file   vpointf.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VPOINTF_H
#define VPOINTF_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QtGlobal>

#include "vgobject.h"

class VPointFData;

QT_WARNING_PUSH
QT_WARNING_DISABLE_INTEL(2304)
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

/**
 * @brief The VPointF class keep data of point.
 */
class VPointF : public VGObject
{
public:
    VPointF();
    VPointF(const VPointF &point);
    explicit VPointF(const QPointF &point);
    VPointF(qreal x,
            qreal y,
            const QString &name,
            qreal mx = 0,
            qreal my = 0,
            quint32 idObject = 0,
            const Draw &mode = Draw::Calculation);
    VPointF(const QPointF &point,
            const QString &name,
            qreal mx = 0,
            qreal my = 0,
            quint32 idObject = 0,
            const Draw &mode = Draw::Calculation);
    ~VPointF() override;

    auto operator=(const VPointF &point) -> VPointF &;

    VPointF(VPointF &&point) noexcept;
    auto operator=(VPointF &&point) noexcept -> VPointF &;

    explicit operator QPointF() const;
    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &name = QString()) const -> VPointF;
    auto Flip(const QLineF &axis, const QString &name = QString()) const -> VPointF;
    auto Move(qreal length, qreal angle, const QString &name = QString()) const -> VPointF;
    auto mx() const -> qreal;
    auto my() const -> qreal;
    void setMx(qreal mx);
    void setMy(qreal my);
    auto toQPointF() const -> QPointF;
    auto x() const -> qreal;
    void setX(const qreal &value);
    auto y() const -> qreal;
    void setY(const qreal &value);

    auto IsShowLabel() const -> bool;
    void SetShowLabel(bool hide);

    auto ToJson() const -> QJsonObject override;

    void SetAlias(const QString &alias) override;
    void SetAliasSuffix(const QString &aliasSuffix) override;

    static auto RotatePF(const QPointF &originPoint, const QPointF &point, qreal degrees) -> QPointF;
    static auto FlipPF(const QLineF &axis, const QPointF &point) -> QPointF;
    static auto MovePF(const QPointF &originPoint, qreal length, qreal angle) -> QPointF;

private:
    QSharedDataPointer<VPointFData> d;
};

Q_DECLARE_METATYPE(VPointF)                  // NOLINT
Q_DECLARE_TYPEINFO(VPointF, Q_MOVABLE_TYPE); // NOLINT

QT_WARNING_POP

#endif // VPOINTF_H
