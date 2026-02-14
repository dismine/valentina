/************************************************************************
 **
 **  @file   vtoolpointfromcircleandtangent.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
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

#ifndef VTOOLPOINTFROMCIRCLEANDTANGENT_H
#define VTOOLPOINTFROMCIRCLEANDTANGENT_H

#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolsinglepoint.h"

class VFormula;
template <class T> class QSharedPointer;

struct VToolPointFromCircleAndTangentInitData : VToolSinglePointInitData
{
    quint32 circleCenterId{NULL_ID};
    QString circleRadius{'0'};
    quint32 tangentPointId{NULL_ID};
    CrossCirclesPoint crossPoint{CrossCirclesPoint::FirstPoint};
};

class VToolPointFromCircleAndTangent : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    ~VToolPointFromCircleAndTangent() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolPointFromCircleAndTangent *;
    static auto Create(VToolPointFromCircleAndTangentInitData &initData) -> VToolPointFromCircleAndTangent *;
    static auto FindPoint(const QPointF &p, const QPointF &center, qreal radius, const CrossCirclesPoint crossPoint,
                          QPointF *intersectionPoint) -> bool;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::PointFromCircleAndTangent)
    };

    auto TangentPointName() const -> QString;
    auto CircleCenterPointName() const -> QString;

    auto GetCircleRadius() const -> VFormula;
    void SetCircleRadius(const VFormula &value);

    auto GetCrossCirclesPoint() const -> CrossCirclesPoint;
    void SetCrossCirclesPoint(const CrossCirclesPoint &value);

    void ShowVisualization(bool show) override;
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolPointFromCircleAndTangent) // NOLINT

    quint32 circleCenterId;
    quint32 tangentPointId;
    QString circleRadius;
    CrossCirclesPoint crossPoint;

    explicit VToolPointFromCircleAndTangent(const VToolPointFromCircleAndTangentInitData &initData,
                                            QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLPOINTFROMCIRCLEANDTANGENT_H
