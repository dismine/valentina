/************************************************************************
 **
 **  @file   vtoolabstractcurve.h
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

#ifndef VTOOLABSTRACTCURVE_H
#define VTOOLABSTRACTCURVE_H

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
class VAbstractCubicBezier;

struct VToolAbstractCurveInitData : VDrawToolInitData
{
    QString color{ColorBlack};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString penStyle{TypeLineLine};                       // NOLINT(misc-non-private-member-variables-in-classes)
    qreal approximationScale{defCurveApproximationScale}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString aliasSuffix{};                                // NOLINT(misc-non-private-member-variables-in-classes)
};

class VToolAbstractCurve : public VDrawTool, public QGraphicsPathItem
{
    Q_OBJECT // NOLINT

public:
    VToolAbstractCurve(
        VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes, QGraphicsItem *parent = nullptr);
    ~VToolAbstractCurve() override = default;

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
    virtual void SetAliasSuffix(const QString &alias) = 0;

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
    Q_DISABLE_COPY_MOVE(VToolAbstractCurve) // NOLINT

    bool m_isHovered{false};
    bool m_detailsMode{false};
    bool m_acceptHoverEvents{true};
    SceneObject sceneType{SceneObject::Unknown};

    void InitDefShape();
};

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolAbstractCurve::ShowToolVisualization(bool show)
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
template<typename T>
inline void VToolAbstractCurve::InitSplineToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolAbstractCurve::InitSplinePathToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolAbstractCurve::InitArcToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void VToolAbstractCurve::InitElArcToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolAbstractCurve::IsDetailsMode() const -> bool
{
    return m_detailsMode;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolAbstractCurve::GetAcceptHoverEvents() const -> bool
{
    return m_acceptHoverEvents;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolAbstractCurve::GetSceneType() const -> SceneObject
{
    return sceneType;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VToolAbstractCurve::SetSceneType(SceneObject newSceneType)
{
    sceneType = newSceneType;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VToolAbstractCurve::SetAcceptHoverEvents(bool newAcceptHoverEvents)
{
    m_acceptHoverEvents = newAcceptHoverEvents;
}

class VToolAbstractBezier : public VToolAbstractCurve
{
    Q_OBJECT // NOLINT

public:
    VToolAbstractBezier(
        VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes, QGraphicsItem *parent = nullptr);
    ~VToolAbstractBezier() override = default;

    void SetAliasSuffix(const QString &alias) override;

protected:
    struct ToolChanges
    {
        QString oldP1Label{};
        QString newP1Label{};
        QString oldP4Label{};
        QString newP4Label{};
        QString oldAliasSuffix{};
        QString newAliasSuffix{};

        auto HasChanges() const -> bool
        {
            return oldP1Label != newP1Label || oldP4Label != newP4Label || oldAliasSuffix != newAliasSuffix;
        }

        auto P1LabelChanged() const -> bool { return oldP1Label != newP1Label; }
        auto P4LabelChanged() const -> bool { return oldP4Label != newP4Label; }
        auto AliasSuffixChanged() const -> bool { return oldAliasSuffix != newAliasSuffix; }
    };

    void ProcessSplineToolOptions(const QDomElement &oldDomElement,
                                  const QDomElement &newDomElement,
                                  const ToolChanges &changes);
    void ProcessSplinePathToolOptions(const QDomElement &oldDomElement,
                                      const QDomElement &newDomElement,
                                      const ToolChanges &changes);

private:
    Q_DISABLE_COPY_MOVE(VToolAbstractBezier) // NOLINT
};

class VToolAbstractArc : public VToolAbstractCurve
{
    Q_OBJECT // NOLINT

public:
    VToolAbstractArc(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes,
                     QGraphicsItem *parent = nullptr);
    ~VToolAbstractArc() override = default;

    auto CenterPointName() const -> QString;

    void SetAliasSuffix(const QString &alias) override;

protected:
    struct ToolChanges
    {
        QString oldCenterLabel{};
        QString newCenterLabel{};
        QString oldAliasSuffix{};
        QString newAliasSuffix{};

        auto HasChanges() const -> bool { return oldCenterLabel != newCenterLabel || oldAliasSuffix != newAliasSuffix; }

        auto CenterLabelChanged() const -> bool { return oldCenterLabel != newAliasSuffix; }
        auto AliasSuffixChanged() const -> bool { return oldAliasSuffix != newAliasSuffix; }
    };

    void ProcessArcToolOptions(const QDomElement &oldDomElement,
                               const QDomElement &newDomElement,
                               const ToolChanges &changes);

private:
    Q_DISABLE_COPY_MOVE(VToolAbstractArc) // NOLINT
};

enum class VToolAbstractOffsetCurveField : quint8
{
    Name,
    AliasSuffix
};

class VToolAbstractOffsetCurve : public VToolAbstractCurve
{
    Q_OBJECT // NOLINT

public:
    VToolAbstractOffsetCurve(VAbstractPattern *doc,
                             VContainer *data,
                             quint32 id,
                             quint32 originCurveId,
                             QString name,
                             const QString &notes,
                             QGraphicsItem *parent = nullptr);
    ~VToolAbstractOffsetCurve() override = default;

    auto GetApproximationScale() const -> qreal override;
    void SetApproximationScale(qreal value);

    auto GetName() const -> QString;
    void SetName(const QString &name);

    void SetAliasSuffix(const QString &alias) override;

    auto CurveName() const -> QString;

protected:
    struct ToolChanges
    {
        QString oldName{};
        QString newName{};
        QString oldAliasSuffix{};
        QString newAliasSuffix{};

        auto HasChanges() const -> bool { return oldName != newName || oldAliasSuffix != newAliasSuffix; }

        auto NameChanged() const -> bool { return oldName != newName; }
        auto AliasSuffixChanged() const -> bool { return oldAliasSuffix != newAliasSuffix; }
    };

    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;

    void ProcessOffsetCurveToolOptions(const QDomElement &oldDomElement,
                                       const QDomElement &newDomElement,
                                       const ToolChanges &changes);

    auto OriginCurveId() const -> quint32;

private:
    Q_DISABLE_COPY_MOVE(VToolAbstractOffsetCurve) // NOLINT

    quint32 m_originCurveId;
    QString m_name;

    void UpdateNameField(VToolAbstractOffsetCurveField field, const QString &value);
    auto HasConflict(const QString &value, VToolAbstractOffsetCurveField currentField) const -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolAbstractOffsetCurve::GetName() const -> QString
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolAbstractOffsetCurve::OriginCurveId() const -> quint32
{
    return m_originCurveId;
}

#endif // VTOOLABSTRACTCURVE_H
