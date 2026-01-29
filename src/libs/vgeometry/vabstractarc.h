/************************************************************************
 **
 **  @file   vabstractarc.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
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

#ifndef VABSTRACTARC_H
#define VABSTRACTARC_H

#include <QSharedDataPointer>
#include <QString>
#include <QtGlobal>

#include "vabstractcurve.h"
#include "vgeometrydef.h"

class VAbstractArcData;
class VPointF;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

class VAbstractArc : public VAbstractCurve
{
public:
    explicit VAbstractArc(const GOType &type, const quint32 &idObject = NULL_ID, const Draw &mode = Draw::Calculation);
    VAbstractArc(const GOType &type, const VPointF &center, qreal f1, const QString &formulaF1, qreal f2,
                 const QString &formulaF2, quint32 idObject = 0, Draw mode = Draw::Calculation);
    VAbstractArc(const GOType &type, const VPointF &center, qreal f1, qreal f2, quint32 idObject = 0,
                 Draw mode = Draw::Calculation);
    VAbstractArc(const GOType &type, const QString &formulaLength, const VPointF &center, qreal f1,
                 const QString &formulaF1, quint32 idObject = 0, Draw mode = Draw::Calculation);
    VAbstractArc(const GOType &type, const VPointF &center, qreal f1, quint32 idObject = 0,
                 Draw mode = Draw::Calculation);
    VAbstractArc(const VAbstractArc &arc);
    ~VAbstractArc() override;

    auto operator=(const VAbstractArc &arc) -> VAbstractArc &;

    VAbstractArc(VAbstractArc &&arc) noexcept;
    auto operator=(VAbstractArc &&arc) noexcept -> VAbstractArc &;

    auto GetFormulaF1() const -> QString;
    void SetFormulaF1(const QString &formula, qreal value);
    auto GetStartAngle() const -> qreal override;

    auto GetFormulaF2() const -> QString;
    void SetFormulaF2(const QString &formula, qreal value);
    auto GetEndAngle() const -> qreal override;

    virtual auto GetCenter() const -> VPointF;
    void SetCenter(const VPointF &point);

    auto GetFormulaLength() const -> QString;
    void SetFormulaLength(const QString &formula, qreal value);

    void setId(const quint32 &id) override;
    auto NameForHistory(const QString &toolName) const -> QString override;
    auto GetMainNameForHistory() const -> QString override;

    auto IsFlipped() const -> bool;
    auto AngleArc() const -> qreal;

    auto GetPath() const -> QPainterPath override;

    auto CutArc(qreal length, VAbstractArc *arc1, VAbstractArc *arc2, const QString &pointName) const -> QPointF;
    auto CutArc(qreal length, const QString &pointName) const -> QPointF;

    virtual auto ToSplinePath() const -> VSplinePath = 0;

    auto Offset(qreal distance, const QString &name = QString()) const -> VSplinePath override;
    auto Outline(const QVector<qreal> &distances, const QString &name = QString()) const -> VSplinePath override;

protected:
    void SetFlipped(bool value);
    void SetAllowEmpty(bool value);
    auto IsAllowEmpty() const -> bool;

    virtual void FindF2(qreal length) = 0;
    void SetFormulaLength(const QString &formula);

    virtual auto DoCutArc(qreal length, VAbstractArc *arc1, VAbstractArc *arc2, const QString &pointName) const
        -> QPointF
        = 0;
    virtual auto DoCutArcByLength(qreal length, const QString &pointName) const -> QPointF = 0;

    auto AngleArc(qreal startAngle, qreal endAngle) const -> qreal;

private:
    QSharedDataPointer<VAbstractArcData> d;
};

QT_WARNING_POP

#endif // VABSTRACTARC_H
