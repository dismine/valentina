/************************************************************************
 **
 **  @file   vtoolspline.h
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

#ifndef VTOOLSPLINE_H
#define VTOOLSPLINE_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/def.h"
#include "vtoolabstractcurve.h"

template <class T> class QSharedPointer;

struct VToolSplineInitData : VToolAbstractCurveInitData
{
    quint32 point1{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 point4{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString a1{};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString a2{};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString l1{};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString l2{};            // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 duplicate{0};    // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VToolSpline class tool for creation spline. I mean bezier curve.
 */
class VToolSpline : public VToolAbstractBezier
{
    Q_OBJECT // NOLINT

public:
    ~VToolSpline() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolSpline *;
    static auto Create(VToolSplineInitData &initData, VSpline *spline) -> VToolSpline *;
    static auto Create(VToolSplineInitData &initData) -> VToolSpline *;
    static const QString ToolType;
    static const QString OldToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::Spline)
    };

    auto getSpline() const -> VSpline;
    void setSpline(const VSpline &spl);

    void ShowVisualization(bool show) override;

    void ShowHandles(bool show) override;

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
    Q_DISABLE_COPY_MOVE(VToolSpline) // NOLINT
    QPointF oldPosition{};
    bool moved{false};
    QSharedPointer<VSpline> oldMoveSpline{};
    QSharedPointer<VSpline> newMoveSpline{};
    /**
     * @brief controlPoints list pointers of control points.
     */
    QVector<VControlPointSpline *> controlPoints{};

    explicit VToolSpline(const VToolSplineInitData &initData, QGraphicsItem *parent = nullptr);

    auto IsMovable() const -> bool;
    void SetSplineAttributes(QDomElement &domElement, const VSpline &spl);

    void UndoCommandMove(const VSpline &oldSpl, const VSpline &newSpl);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLSPLINE_H
