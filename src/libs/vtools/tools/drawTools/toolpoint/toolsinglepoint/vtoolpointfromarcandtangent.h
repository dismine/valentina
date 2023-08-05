/************************************************************************
 **
 **  @file   vtoolpointfromarcandtangent.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 6, 2015
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

#ifndef VTOOLPOINTFROMARCANDTANGENT_H
#define VTOOLPOINTFROMARCANDTANGENT_H

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

template <class T> class QSharedPointer;
class VArc;

struct VToolPointFromArcAndTangentInitData : VToolSinglePointInitData
{
    quint32 arcId{NULL_ID};
    quint32 tangentPointId{NULL_ID};
    CrossCirclesPoint crossPoint{CrossCirclesPoint::FirstPoint};
};

class VToolPointFromArcAndTangent : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    void SetDialog() override;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolPointFromArcAndTangent *;
    static auto Create(VToolPointFromArcAndTangentInitData initData) -> VToolPointFromArcAndTangent *;

    static auto FindPoint(const QPointF &p, const VArc *arc, const CrossCirclesPoint pType, QPointF *intersectionPoint)
        -> bool;

    static const QString ToolType;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::PointFromArcAndTangent)
    };

    auto TangentPointName() const -> QString;
    auto ArcName() const -> QString;

    auto GetCrossCirclesPoint() const -> CrossCirclesPoint;
    void SetCrossCirclesPoint(CrossCirclesPoint value);

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void RemoveReferens() override;
    void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;

private:
    Q_DISABLE_COPY_MOVE(VToolPointFromArcAndTangent) // NOLINT

    quint32 arcId;
    quint32 tangentPointId;
    CrossCirclesPoint crossPoint;

    explicit VToolPointFromArcAndTangent(const VToolPointFromArcAndTangentInitData &initData,
                                         QGraphicsItem *parent = nullptr);
};

#endif // VTOOLPOINTFROMARCANDTANGENT_H
