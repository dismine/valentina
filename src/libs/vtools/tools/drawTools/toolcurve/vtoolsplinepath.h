/************************************************************************
 **
 **  @file   vtoolsplinepath.h
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

#ifndef VTOOLSPLINEPATH_H
#define VTOOLSPLINEPATH_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "vtoolabstractcurve.h"

class VSplinePath;
template <class T> class QSharedPointer;

struct VToolSplinePathInitData : VToolAbstractCurveInitData
{
    QVector<quint32> points{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<QString> a1{};     // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<QString> a2{};     // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<QString> l1{};     // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<QString> l2{};     // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 duplicate{0};      // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VToolSplinePath class tool for creation spline path.
 */
class VToolSplinePath : public VToolAbstractBezier
{
    Q_OBJECT // NOLINT

public:
    ~VToolSplinePath() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolSplinePath *;
    static auto Create(VToolSplinePathInitData &initData, VSplinePath *path) -> VToolSplinePath *;
    static auto Create(VToolSplinePathInitData &initData) -> VToolSplinePath *;
    static const QString ToolType;
    static const QString OldToolType;
    static void UpdatePathPoints(VAbstractPattern *doc, QDomElement &element, const VSplinePath &path);
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::SplinePath)
    };

    auto getSplinePath() const -> VSplinePath;
    void setSplinePath(const VSplinePath &splPath);

    void ShowVisualization(bool show) override;

    void ShowHandles(bool show) override;

signals:
    /**
     * @brief RefreshLine refresh control line.
     * @param indexSpline position spline in spline list.
     * @param pos position point in spline.
     * @param controlPoint new position control point.
     * @param splinePoint new position spline point.
     */
    void RefreshLine(const qint32 &indexSpline, SplinePointPosition pos, const QPointF &controlPoint,
                     const QPointF &splinePoint);

public slots:
    void ControlPointChangePosition(const qint32 &indexSpline, const SplinePointPosition &position, const QPointF &pos);
    void EnableToolMove(bool move) override;
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;
    void CurveSelected(bool selected);

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void SetVisualization() override;
    void RefreshCtrlPoints() override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private slots:
    void CurveReleased();

private:
    Q_DISABLE_COPY_MOVE(VToolSplinePath) // NOLINT
    QPointF oldPosition{};
    int splIndex{-1};
    bool moved{false};
    QSharedPointer<VSplinePath> oldMoveSplinePath{};
    QSharedPointer<VSplinePath> newMoveSplinePath{};
    /**
     * @brief controlPoints list pointers of control points.
     */
    QVector<VControlPointSpline *> controlPoints{};

    explicit VToolSplinePath(const VToolSplinePathInitData &initData, QGraphicsItem *parent = nullptr);

    auto IsMovable(int index) const -> bool;
    static void AddPathPoint(VAbstractPattern *doc, QDomElement &domElement, const VSplinePoint &splPoint);
    void UpdateControlPoints(const VSpline &spl, QSharedPointer<VSplinePath> &splPath, qint32 indexSpline) const;
    void SetSplinePathAttributes(QDomElement &domElement, const VSplinePath &path);

    void UndoCommandMove(const VSplinePath &oldPath, const VSplinePath &newPath);

    void InitControlPoints(const VSplinePath *splPath);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLSPLINEPATH_H
