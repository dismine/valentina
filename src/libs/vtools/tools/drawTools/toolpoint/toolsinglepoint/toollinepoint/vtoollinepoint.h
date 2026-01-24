/************************************************************************
 **
 **  @file   vtoollinepoint.h
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

#ifndef VTOOLLINEPOINT_H
#define VTOOLLINEPOINT_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../vpatterndb/vformula.h"
#include "../vtoolsinglepoint.h"

template <class T> class QSharedPointer;

struct VToolLinePointInitData : VToolSinglePointInitData
{
    using VToolSinglePointInitData::VToolSinglePointInitData;

    QString typeLine{TypeLineLine};
    QString lineColor{ColorBlack};
};

/**
 * @brief The VToolLinePoint class parent for all tools what create point with line.
 */
class VToolLinePoint : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    VToolLinePoint(VAbstractPattern *doc, VContainer *data, const quint32 &id, const QString &typeLine,
                   const QString &lineColor, const QString &formula, const quint32 &basePointId, const qreal &angle,
                   const QString &notes, QGraphicsItem *parent = nullptr);
    ~VToolLinePoint() override;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::LinePoint)
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    auto GetFormulaLength() const -> VFormula;
    void SetFormulaLength(const VFormula &value);

    auto BasePointName() const -> QString;

    auto GetAngle() const -> qreal;
    void SetAngle(const qreal &value);

    auto GetLineColor() const -> QString;
    void SetLineColor(const QString &value);

public slots:
    void Enable() override;
    void FullUpdateFromFile() override;

protected:
    /** @brief formula string with length formula. */
    QString formulaLength;

    /** @brief angle line angle. */
    qreal angle;

    /** @brief basePointId id base line point. */
    quint32 basePointId;

    /** @brief mainLine line item. */
    VScaledLine *mainLine{nullptr};

    /** @brief lineColor color of a line. */
    QString lineColor;

    virtual void RefreshGeometry();
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    auto MakeToolTip() const -> QString override;

    void ProcessLinePointToolOptions(const QDomElement &oldDomElement,
                                     const QDomElement &newDomElement,
                                     const QString &newLabel,
                                     const QString &newBasePointLabel);

private:
    Q_DISABLE_COPY_MOVE(VToolLinePoint) // NOLINT
};

#endif // VTOOLLINEPOINT_H
