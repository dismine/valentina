/************************************************************************
 **
 **  @file   vcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VCUBICBEZIER_H
#define VCUBICBEZIER_H

#include <qcompilerdetection.h>
#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vabstractcubicbezier.h"
#include "vgeometrydef.h"
#include "vpointf.h"

class VCubicBezierData;

class VCubicBezier final : public VAbstractCubicBezier
{
public:
    VCubicBezier();
    VCubicBezier(const VCubicBezier &curve);
    VCubicBezier(const VPointF &p1, const VPointF &p2, const VPointF &p3, const VPointF &p4, quint32 idObject = 0,
                 Draw mode = Draw::Calculation);
    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &prefix = QString()) const -> VCubicBezier;
    auto Flip(const QLineF &axis, const QString &prefix = QString()) const -> VCubicBezier;
    auto Move(qreal length, qreal angle, const QString &prefix = QString()) const -> VCubicBezier;
    virtual ~VCubicBezier();

    auto operator=(const VCubicBezier &curve) -> VCubicBezier &;
#ifdef Q_COMPILER_RVALUE_REFS
    VCubicBezier(VCubicBezier &&curve) Q_DECL_NOTHROW;
    auto operator=(VCubicBezier &&curve) Q_DECL_NOTHROW->VCubicBezier &;
#endif

    virtual auto GetP1() const -> VPointF override;
    void            SetP1(const VPointF &p);

    virtual auto GetP2() const -> VPointF override;
    void    SetP2(const VPointF &p);

    virtual auto GetP3() const -> VPointF override;
    void    SetP3(const VPointF &p);

    virtual auto GetP4() const -> VPointF override;
    void            SetP4(const VPointF &p);

    virtual auto GetStartAngle() const -> qreal override;
    virtual auto GetEndAngle() const -> qreal override;
    virtual auto GetLength() const -> qreal override;
    virtual auto GetPoints() const -> QVector<QPointF> override;

    virtual auto GetC1Length() const -> qreal override;
    virtual auto GetC2Length() const -> qreal override;

protected:
    virtual auto GetControlPoint1() const -> QPointF override;
    virtual auto GetControlPoint2() const -> QPointF override;
    auto GetRealLength() const -> qreal override;

private:
    QSharedDataPointer<VCubicBezierData> d;
};

Q_DECLARE_TYPEINFO(VCubicBezier, Q_MOVABLE_TYPE); // NOLINT

#endif // VCUBICBEZIER_H
