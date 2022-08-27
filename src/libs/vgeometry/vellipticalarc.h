/************************************************************************
 **
 **  @file   vellipticalarc.h
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   February 1, 2016
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

#ifndef VELLIPTICALARC_H
#define VELLIPTICALARC_H

#include <qcompilerdetection.h>
#include <QCoreApplication>
#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>
#include <QtMath>

#include "vabstractarc.h"
#include "vpointf.h"

class VEllipticalArcData;

class VEllipticalArc final : public VAbstractArc
{
    Q_DECLARE_TR_FUNCTIONS(VEllipticalArc) // NOLINT
public:
    VEllipticalArc();
    VEllipticalArc(const VPointF &center, qreal radius1, qreal radius2, const QString &formulaRadius1,
                   const QString &formulaRadius2, qreal f1, const QString &formulaF1, qreal f2,
                   const QString &formulaF2, qreal rotationAngle, const QString &formulaRotationAngle,
                   quint32 idObject = 0, Draw mode = Draw::Calculation);
    VEllipticalArc(const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal f2, qreal rotationAngle);
    VEllipticalArc(qreal length, const QString &formulaLength, const VPointF &center, qreal radius1, qreal radius2,
                   const QString &formulaRadius1, const QString &formulaRadius2, qreal f1, const QString &formulaF1,
                   qreal rotationAngle, const QString &formulaRotationAngle, quint32 idObject = 0,
                   Draw mode = Draw::Calculation);
    VEllipticalArc(qreal length, const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal rotationAngle);
    VEllipticalArc(const VEllipticalArc &arc);

    auto Rotate(QPointF originPoint, qreal degrees, const QString &prefix = QString()) const -> VEllipticalArc;
    auto Flip(const QLineF &axis, const QString &prefix = QString()) const -> VEllipticalArc;
    auto Move(qreal length, qreal angle, const QString &prefix = QString()) const -> VEllipticalArc;

    ~VEllipticalArc() override;

    auto operator= (const VEllipticalArc &arc) -> VEllipticalArc&;
#ifdef Q_COMPILER_RVALUE_REFS
    VEllipticalArc(VEllipticalArc &&arc) Q_DECL_NOTHROW;
    auto operator=(VEllipticalArc &&arc) Q_DECL_NOTHROW -> VEllipticalArc &;
#endif

    auto GetFormulaRotationAngle() const -> QString;
    void SetFormulaRotationAngle(const QString &formula, qreal value);
    void SetRotationAngle(qreal value);
    auto GetRotationAngle() const -> qreal;

    auto GetFormulaRadius1() const -> QString;
    void SetFormulaRadius1(const QString &formula, qreal value);
    void SetRadius1(qreal value);
    auto GetRadius1() const -> qreal;

    auto GetFormulaRadius2() const -> QString;
    void SetFormulaRadius2(const QString &formula, qreal value);
    void SetRadius2(qreal value);
    auto GetRadius2() const -> qreal;

    auto GetLength() const -> qreal override;

    auto GetP1() const -> QPointF;
    auto GetP2() const -> QPointF;

    auto GetTransform() const -> QTransform;
    void SetTransform(const QTransform &matrix, bool combine = false);

    auto GetCenter() const -> VPointF;
    auto GetPoints() const -> QVector<QPointF> override;
    auto GetStartAngle() const -> qreal override;
    auto GetEndAngle() const -> qreal override;

    auto CutArc (const qreal &length, VEllipticalArc &arc1, VEllipticalArc &arc2,
                 const QString &pointName) const -> QPointF;
    auto CutArc (const qreal &length, const QString &pointName) const -> QPointF;

    static auto OptimizeAngle(qreal angle) -> qreal;
protected:
    void CreateName() override;
    void CreateAlias() override;
    void FindF2(qreal length) override;
private:
    QSharedDataPointer<VEllipticalArcData> d;

    auto MaxLength() const -> qreal;
    auto GetP(qreal angle) const -> QPointF;
    auto ArcPoints(QVector<QPointF> points) const -> QVector<QPointF>;
};

Q_DECLARE_METATYPE(VEllipticalArc) // NOLINT
Q_DECLARE_TYPEINFO(VEllipticalArc, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
inline auto VEllipticalArc::OptimizeAngle(qreal angle) -> qreal
{
    return angle - 360.*qFloor(angle/360.);
}

#endif // VELLIPTICALARC_H
