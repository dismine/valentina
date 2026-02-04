/************************************************************************
 **
 **  @file   varc.h
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

#ifndef VARC_H
#define VARC_H

#include <QCoreApplication>
#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vabstractarc.h"
#include "vpointf.h"

class VArcData;
class VSplinePath;

/**
 * @brief VArc class for anticlockwise arc.
 */
class VArc final : public VAbstractArc
{
    Q_DECLARE_TR_FUNCTIONS(VArc) // NOLINT

public:
    VArc();
    VArc(const VPointF &center, qreal radius, const QString &formulaRadius, qreal f1, const QString &formulaF1,
         qreal f2, const QString &formulaF2, quint32 idObject = 0, Draw mode = Draw::Calculation);
    VArc(const VPointF &center, qreal radius, qreal f1, qreal f2);
    VArc(qreal length, const QString &formulaLength, const VPointF &center, qreal radius, const QString &formulaRadius,
         qreal f1, const QString &formulaF1, quint32 idObject = 0, Draw mode = Draw::Calculation);
    VArc(qreal length, const VPointF &center, qreal radius, qreal f1);
    VArc(const VArc &arc);
    ~VArc() override;

    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &name = QString()) const -> VArc;
    auto Flip(const QLineF &axis, const QString &name = QString()) const -> VArc;
    auto Move(qreal length, qreal angle, const QString &name = QString()) const -> VArc;

    auto operator=(const VArc &arc) -> VArc &;

    VArc(VArc &&arc) noexcept;
    auto operator=(VArc &&arc) noexcept -> VArc &;

    auto GetFormulaRadius() const -> QString;
    void SetFormulaRadius(const QString &formula, qreal value);
    auto GetRadius() const -> qreal;

    auto GetLength() const -> qreal override;

    auto GetP1() const -> QPointF;
    auto GetP2() const -> QPointF;

    auto GetPoints() const -> QVector<QPointF> override;

    auto ToSplinePath() const -> VSplinePath override;

    static auto OptimalApproximationScale(qreal radius, qreal f1, qreal f2, qreal tolerance) -> qreal;

    auto HeadlessName() const -> QString override;

    auto GetTypeHead() const -> QString override;
    void SetNameSuffix(const QString &suffix) override;

protected:
    void CreateName() override;
    void CreateAlias() override;
    void FindF2(qreal length) override;
    auto DoCutArc(qreal length, VAbstractArc *arc1, VAbstractArc *arc2, const QString &pointName) const
        -> QPointF override;
    auto DoCutArcByLength(qreal length, const QString &pointName) const -> QPointF override;

private:
    QSharedDataPointer<VArcData> d;

    auto MaxLength() const -> qreal;

    auto CutPoint(qreal length, qreal fullLength, const QString &pointName) const -> QLineF;
    auto CutPointFlipped(qreal length, qreal fullLength, const QString &pointName) const -> QLineF;
};

Q_DECLARE_TYPEINFO(VArc, Q_MOVABLE_TYPE); // NOLINT

#endif // VARC_H
