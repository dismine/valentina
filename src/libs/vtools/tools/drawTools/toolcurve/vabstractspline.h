/************************************************************************
 **
 **  @file   vabstractspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 3, 2014
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

#ifndef VABSTRACTSPLINE_H
#define VABSTRACTSPLINE_H

#include <QDomElement>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QMetaObject>
#include <QObject>
#include <QPainterPath>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtGlobal>

#include "../vdrawtool.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"

class VControlPointSpline;
template <class T> class QSharedPointer;

struct VAbstractSplineInitData : VDrawToolInitData
{
    VAbstractSplineInitData() = default;

    QString color{ColorBlack};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString penStyle{TypeLineLine};                       // NOLINT(misc-non-private-member-variables-in-classes)
    qreal approximationScale{defCurveApproximationScale}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString aliasSuffix{};                                // NOLINT(misc-non-private-member-variables-in-classes)
};

class VAbstractSpline : public VDrawTool, public QGraphicsPathItem
{
    Q_OBJECT // NOLINT

public:
    VAbstractSpline(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes,
                    QGraphicsItem *parent = nullptr);
    ~VAbstractSpline() override = default;

    auto shape() const -> QPainterPath override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::AbstractSpline)
    };
    auto getTagName() const -> QString override;

    virtual void ShowHandles(bool show);

    auto GetLineColor() const -> QString;
    void SetLineColor(const QString &value);

    auto GetPenStyle() const -> QString;
    void SetPenStyle(const QString &value);

    auto name() const -> QString;

    virtual auto GetApproximationScale() const -> qreal;

    auto GetDuplicate() const -> quint32;

    auto GetAliasSuffix() const -> QString;
    void SetAliasSuffix(QString alias);

    void GroupVisibility(quint32 object, bool visible) override;

public slots:
    void FullUpdateFromFile() override;
    void Enable() override;
    void SetDetailsMode(bool mode) override;
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;

signals:
    /**
     * @brief setEnabledPoint disable control points.
     * @param enable enable or diasable points.
     */
    void setEnabledPoint(bool enable);

protected:
    /**
     * @brief RefreshGeometry  refresh item on scene.
     */
    virtual void RefreshGeometry();
    void ShowTool(quint32 id, bool enable) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void RefreshCtrlPoints();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    auto MakeToolTip() const -> QString override;

    auto IsDetailsMode() const -> bool;

    auto GetAcceptHoverEvents() const -> bool;
    void SetAcceptHoverEvents(bool newAcceptHoverEvents);

    auto GetSceneType() const -> SceneObject;
    void SetSceneType(SceneObject newSceneType);

    static auto CorrectedSpline(const VSpline &spline, const SplinePointPosition &position, const QPointF &pos)
        -> VSpline;

    template <typename T> void ShowToolVisualization(bool show);

    template <typename T> static void InitSplineToolConnections(VMainGraphicsScene *scene, T *tool);
    template <typename T> static void InitSplinePathToolConnections(VMainGraphicsScene *scene, T *tool);
    template <typename T> static void InitArcToolConnections(VMainGraphicsScene *scene, T *tool);
    template <typename T> static void InitElArcToolConnections(VMainGraphicsScene *scene, T *tool);

private:
    Q_DISABLE_COPY_MOVE(VAbstractSpline) // NOLINT

    bool m_isHovered{false};
    bool m_detailsMode{false};
    bool m_acceptHoverEvents{true};
    SceneObject sceneType{SceneObject::Unknown};

    void InitDefShape();
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VAbstractSpline::ShowToolVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<T>();
            SetVisualization();
        }
        else
        {
            if (T *visual = qobject_cast<T *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis.data();
    }

    if (m_detailsMode)
    {
        ShowHandles(m_detailsMode);
    }
    else
    {
        ShowHandles(show);
    }

    if (QGraphicsScene *sc = scene())
    { // Showing/hiding control points require recalculation scene size.
        VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VAbstractSpline::InitSplineToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VAbstractSpline::InitSplinePathToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VAbstractSpline::InitArcToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VAbstractSpline::InitElArcToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractSpline::IsDetailsMode() const -> bool
{
    return m_detailsMode;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractSpline::GetAcceptHoverEvents() const -> bool
{
    return m_acceptHoverEvents;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractSpline::GetSceneType() const -> SceneObject
{
    return sceneType;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractSpline::SetSceneType(SceneObject newSceneType)
{
    sceneType = newSceneType;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstractSpline::SetAcceptHoverEvents(bool newAcceptHoverEvents)
{
    m_acceptHoverEvents = newAcceptHoverEvents;
}

class VToolAbstractArc : public VAbstractSpline
{
    Q_OBJECT // NOLINT

public:
    VToolAbstractArc(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes,
                     QGraphicsItem *parent = nullptr);
    ~VToolAbstractArc() override = default;

    auto CenterPointName() const -> QString;

private:
    Q_DISABLE_COPY_MOVE(VToolAbstractArc) // NOLINT
};

#endif // VABSTRACTSPLINE_H
