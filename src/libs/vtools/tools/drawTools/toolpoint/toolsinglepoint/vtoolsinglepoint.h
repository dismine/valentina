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

    void GroupVisibility(quint32 object, bool visible) override;
    void ChangeLabelPosition(quint32 id, const QPointF &pos) override;

    auto IsLabelVisible(quint32 id) const -> bool override;
    void SetLabelVisible(quint32 id, bool visible) override;

public slots:
    void NameChangePosition(const QPointF &pos);
    void Enable() override;
    void EnableToolMove(bool move) override;
    void PointChoosed();
    void PointSelected(bool selected);
    void FullUpdateFromFile() override;
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;
    virtual void AllowLabelHover(bool enabled);
    virtual void AllowLabelSelecting(bool enabled);
    void ToolSelectionType(const SelectionType &selectionType) override;

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
    virtual void RefreshGeometry();

    struct SegmentDetails
    {
        Source typeCreation{Source::FromFile};
        GOType curveType{GOType::Unknown};
        qreal segLength{-1};
        VPointF p{};
        quint32 curveId{NULL_ID};
        VContainer *data{nullptr};
        VAbstractPattern *doc{nullptr};
        QString name1{};
        QString name2{};
        QString alias1{};
        QString alias2{};
        quint32 id{NULL_ID};
        QString name1AttrName{};
        QString name2AttrName{};
        quint32 segment1Id{NULL_ID};
        quint32 segment2Id{NULL_ID};
    };

    template<typename T>
    static void FixSubCurveNames(SegmentDetails &details,
                                 const QSharedPointer<T> &baseCurve,
                                 const QSharedPointer<T> &leftSub,
                                 const QSharedPointer<T> &rightSub);

    template<class Item>
    static void InitArc(SegmentDetails &details);

    static void InitSpline(SegmentDetails &details);
    static void InitSplinePath(SegmentDetails &details);
    static void InitSegments(SegmentDetails &details);

    template<typename T>
    static void InitSegmentConnections(VMainGraphicsScene *scene, T *tool, GOType type);

    template<typename T>
    static void InitUniqueSegmentConnections(VMainGraphicsScene *scene, T *tool, GOType type);

private:
    Q_DISABLE_COPY_MOVE(VToolSinglePoint) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolSinglePoint::InitUniqueSegmentConnections(VMainGraphicsScene *scene, T *tool, GOType type)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    switch (type)
    {
        case GOType::Arc:
            QObject::connect(scene,
                             &VMainGraphicsScene::ShowArcSegmentLabel,
                             tool,
                             &T::SetArcSegmentLabelVisible,
                             Qt::UniqueConnection);
            QObject::connect(scene,
                             &VMainGraphicsScene::EnableArcItemHover,
                             tool,
                             &T::AllowArcSegmentHover,
                             Qt::UniqueConnection);
            break;
        case GOType::EllipticalArc:
            QObject::connect(scene,
                             &VMainGraphicsScene::ShowElArcSegmentLabel,
                             tool,
                             &T::SetElArcSegmentLabelVisible,
                             Qt::UniqueConnection);
            QObject::connect(scene,
                             &VMainGraphicsScene::EnableElArcItemHover,
                             tool,
                             &T::AllowElArcSegmentHover,
                             Qt::UniqueConnection);
            break;
        case GOType::Spline:
        case GOType::CubicBezier:
            QObject::connect(scene,
                             &VMainGraphicsScene::ShowSplineSegmentLabel,
                             tool,
                             &T::SetSplineSegmentLabelVisible,
                             Qt::UniqueConnection);
            QObject::connect(scene,
                             &VMainGraphicsScene::EnableSplineItemHover,
                             tool,
                             &T::AllowSplineSegmentHover,
                             Qt::UniqueConnection);
            break;
        case GOType::SplinePath:
        case GOType::CubicBezierPath:
            QObject::connect(scene,
                             &VMainGraphicsScene::ShowSplinePathSegmentLabel,
                             tool,
                             &T::SetSplinePathSegmentLabelVisible,
                             Qt::UniqueConnection);
            QObject::connect(scene,
                             &VMainGraphicsScene::EnableSplinePathItemHover,
                             tool,
                             &T::AllowSplinePathSegmentHover,
                             Qt::UniqueConnection);
            break;
        case GOType::Point:
        case GOType::PlaceLabel:
        case GOType::Unknown:
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolSinglePoint::InitSegmentConnections(VMainGraphicsScene *scene, T *tool, GOType type)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    switch (type)
    {
        case GOType::Arc:
            QObject::connect(scene, &VMainGraphicsScene::ShowArcSegmentLabel, tool, &T::SetSegmentLabelVisible);
            QObject::connect(scene, &VMainGraphicsScene::EnableArcItemHover, tool, &T::AllowSegmentHover);
            break;
        case GOType::EllipticalArc:
            QObject::connect(scene, &VMainGraphicsScene::ShowElArcSegmentLabel, tool, &T::SetSegmentLabelVisible);
            QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemHover, tool, &T::AllowSegmentHover);
            break;
        case GOType::Spline:
        case GOType::CubicBezier:
            QObject::connect(scene, &VMainGraphicsScene::ShowSplineSegmentLabel, tool, &T::SetSegmentLabelVisible);
            QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemHover, tool, &T::AllowSegmentHover);
            break;
        case GOType::SplinePath:
        case GOType::CubicBezierPath:
            QObject::connect(scene, &VMainGraphicsScene::ShowSplinePathSegmentLabel, tool, &T::SetSegmentLabelVisible);
            QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemHover, tool, &T::AllowSegmentHover);
            break;
        case GOType::Point:
        case GOType::PlaceLabel:
        case GOType::Unknown:
        default:
            break;
    }
}

#endif // VTOOLSINGLEPOINT_H
