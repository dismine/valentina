/************************************************************************
 **
 **  @file   vtooldoublepoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VTOOLDOUBLEPOINT_H
#define VTOOLDOUBLEPOINT_H

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../vabstractpoint.h"
#include "../vmisc/def.h"

class VSimplePoint;

class VToolDoublePoint : public VAbstractPoint, public QGraphicsPathItem
{
    Q_OBJECT // NOLINT

public:
    VToolDoublePoint(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 p1id, quint32 p2id,
                     const QString &notes, QGraphicsItem *parent = nullptr);
    ~VToolDoublePoint() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::DoublePoint)
    };

    auto nameP1() const -> QString;
    void setNameP1(const QString &name);

    auto nameP2() const -> QString;
    void setNameP2(const QString &name);

    void SetNotes(const QString &notes) override;

    void GroupVisibility(quint32 object, bool visible) override;
    void ChangeLabelPosition(quint32 id, const QPointF &pos) override;

    auto IsLabelVisible(quint32 id) const -> bool override;
    void SetLabelVisible(quint32 id, bool visible) override;

public slots:
    void Label1ChangePosition(const QPointF &pos);
    void Label2ChangePosition(const QPointF &pos);
    void Disable(bool disable, const QString &namePP) override;
    void EnableToolMove(bool move) override;
    void Point1Choosed();
    void Point2Choosed();
    void Point1Selected(bool selected);
    void Point2Selected(bool selected);
    void FullUpdateFromFile() override;
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;
    void AllowLabelHover(bool enabled);
    void AllowLabelSelecting(bool enabled);
    virtual void ToolSelectionType(const SelectionType &selectionType) override;

protected:
    VSimplePoint *firstPoint;
    VSimplePoint *secondPoint;

    quint32 p1id;
    quint32 p2id;

    void UpdateNamePosition(quint32 id, const QPointF &pos) override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void AddToFile() override;
    void ChangeLabelVisibility(quint32 id, bool visible) override;

    auto ComplexToolTip(quint32 itemId) const -> QString;

private:
    Q_DISABLE_COPY_MOVE(VToolDoublePoint) // NOLINT
};

#endif // VTOOLDOUBLEPOINT_H
