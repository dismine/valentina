/************************************************************************
 **
 **  @file   vtoolsinglepoint.h
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

#ifndef VTOOLSINGLEPOINT_H
#define VTOOLSINGLEPOINT_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../vabstractpoint.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "../vwidgets/vscenepoint.h"

template <class T> class QSharedPointer;

struct VToolSinglePointInitData : VDrawToolInitData
{
    QString name{};
    qreal mx{labelMX};
    qreal my{labelMY};
    bool showLabel{true};
};

/**
 * @brief The VToolSinglePoint class parent for all tools what create points.
 */
class VToolSinglePoint : public VAbstractPoint, public VScenePoint
{
    Q_OBJECT // NOLINT

public:
    VToolSinglePoint(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes,
                     QGraphicsItem *parent = nullptr);
    ~VToolSinglePoint() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::SinglePoint)
    };

    auto name() const -> QString;
    void setName(const QString &name);

    void SetEnabled(bool enabled);

    void GroupVisibility(quint32 object, bool visible) override;
    void ChangeLabelPosition(quint32 id, const QPointF &pos) override;

    auto IsLabelVisible(quint32 id) const -> bool override;
    void SetLabelVisible(quint32 id, bool visible) override;

public slots:
    void NameChangePosition(const QPointF &pos);
    void Disable(bool disable, const QString &namePP) override;
    void EnableToolMove(bool move) override;
    void PointChoosed();
    void PointSelected(bool selected);
    void FullUpdateFromFile() override;
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;
    void AllowLabelHover(bool enabled);
    void AllowLabelSelecting(bool enabled);
    void ToolSelectionType(const SelectionType &type) override;

protected:
    void UpdateNamePosition(quint32 id, const QPointF &pos) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ChangeLabelVisibility(quint32 id, bool visible) override;

    template <class Item>
    static auto InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId, const QString &alias1,
                        const QString &alias2) -> QPair<QString, QString>;
    static auto InitSegments(GOType curveType, qreal segLength, const VPointF *p, quint32 curveId, VContainer *data,
                             const QString &alias1, const QString &alias2) -> QPair<QString, QString>;

private:
    Q_DISABLE_COPY_MOVE(VToolSinglePoint) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
inline auto VToolSinglePoint::InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId,
                                      const QString &alias1, const QString &alias2) -> QPair<QString, QString>
{
    QSharedPointer<Item> a1;
    QSharedPointer<Item> a2;

    const QSharedPointer<Item> arc = data->GeometricObject<Item>(curveId);
    Item arc1;
    Item arc2;

    if (not VFuzzyComparePossibleNulls(segLength, -1))
    {
        arc->CutArc(segLength, arc1, arc2, p->name());
    }
    else
    {
        arc->CutArc(0, arc1, arc2, p->name());
    }

    // Arc highly depend on id. Need for creating the name.
    arc1.setId(p->id() + 1);
    arc2.setId(p->id() + 2);

    arc1.SetAliasSuffix(alias1);
    arc2.SetAliasSuffix(alias2);

    if (not VFuzzyComparePossibleNulls(segLength, -1))
    {
        a1 = QSharedPointer<Item>(new Item(arc1));
        a2 = QSharedPointer<Item>(new Item(arc2));
    }
    else
    {
        a1 = QSharedPointer<Item>(new Item());
        a2 = QSharedPointer<Item>(new Item());

        // Take names for empty arcs from donors.
        a1->setName(arc1.name());
        a2->setName(arc2.name());
    }

    data->AddArc(a1, arc1.id(), p->id());
    data->AddArc(a2, arc2.id(), p->id());

    // Because we don't store segments, but only data about them we must register the names manually
    data->RegisterUniqueName(a1);
    data->RegisterUniqueName(a2);

    return qMakePair(arc1.ObjectName(), arc2.ObjectName());
}

#endif // VTOOLSINGLEPOINT_H
