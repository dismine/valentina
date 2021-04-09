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

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../vabstractpoint.h"
#include "../vmisc/def.h"
#include "../vwidgets/vscenepoint.h"

template <class T> class QSharedPointer;

struct VToolSinglePointInitData : VDrawToolInitData
{
    VToolSinglePointInitData()
        : VDrawToolInitData(),
          name(),
          mx(labelMX),
          my(labelMY),
          showLabel(true)
    {}

    QString name;
    qreal   mx;
    qreal   my;
    bool    showLabel;
};

/**
 * @brief The VToolSinglePoint class parent for all tools what create points.
 */
class VToolSinglePoint: public VAbstractPoint, public VScenePoint
{
    Q_OBJECT
public:
    VToolSinglePoint(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes,
                     QGraphicsItem * parent = nullptr);
    virtual ~VToolSinglePoint() Q_DECL_EQ_DEFAULT;

    virtual int type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::SinglePoint)};

    QString name() const;
    void    setName(const QString &name);

    void SetEnabled(bool enabled);

    virtual void GroupVisibility(quint32 object, bool visible) override;
    virtual void ChangeLabelPosition(quint32 id, const QPointF &pos) override;

    virtual bool IsLabelVisible(quint32 id) const override;
    virtual void SetLabelVisible(quint32 id, bool visible) override;
public slots:
    void         NameChangePosition(const QPointF &pos);
    virtual void Disable(bool disable, const QString &namePP) override;
    virtual void EnableToolMove(bool move) override;
    void         PointChoosed();
    void         PointSelected(bool selected);
    virtual void FullUpdateFromFile() override;
    virtual void AllowHover(bool enabled) override;
    virtual void AllowSelecting(bool enabled) override;
    void         AllowLabelHover(bool enabled);
    void         AllowLabelSelecting(bool enabled);
    virtual void ToolSelectionType(const SelectionType &type) override;
protected:
    virtual void     UpdateNamePosition(quint32 id, const QPointF &pos) override;
    virtual void     mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void     mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) override;
    virtual void     hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual QVariant itemChange ( GraphicsItemChange change, const QVariant &value ) override;
    virtual void     keyReleaseEvent(QKeyEvent * event) override;
    virtual void     contextMenuEvent ( QGraphicsSceneContextMenuEvent * event ) override;
    virtual void     SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void     ChangeLabelVisibility(quint32 id, bool visible) override;

    template <class Item>
    static void InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId);
    static void InitSegments(GOType curveType, qreal segLength, const VPointF *p, quint32 curveId, VContainer *data);
private:
    Q_DISABLE_COPY(VToolSinglePoint)
};

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
inline void VToolSinglePoint::InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId)
{
    QSharedPointer<Item> a1;
    QSharedPointer<Item> a2;

    const QSharedPointer<Item> arc = data->GeometricObject<Item>(curveId);
    Item arc1;
    Item arc2;

    if (not VFuzzyComparePossibleNulls(segLength, -1))
    {
        arc->CutArc(segLength, arc1, arc2);
    }
    else
    {
        arc->CutArc(0, arc1, arc2);
    }

    // Arc highly depend on id. Need for creating the name.
    arc1.setId(p->id() + 1);
    arc2.setId(p->id() + 2);

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
}

#endif // VTOOLSINGLEPOINT_H
