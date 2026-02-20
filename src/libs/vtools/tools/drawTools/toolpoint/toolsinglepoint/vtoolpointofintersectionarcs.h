/************************************************************************
 **
 **  @file   vtoolpointofintersectionarcs.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 5, 2015
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

#ifndef VTOOLPOINTOFINTERSECTIONARCS_H
#define VTOOLPOINTOFINTERSECTIONARCS_H


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

struct VToolPointOfIntersectionArcsInitData : VToolSinglePointInitData
{
    quint32 firstArcId{NULL_ID};
    quint32 secondArcId{NULL_ID};
    CrossCirclesPoint pType{CrossCirclesPoint::FirstPoint};
};

class VToolPointOfIntersectionArcs : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    ~VToolPointOfIntersectionArcs() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolPointOfIntersectionArcs *;
    static auto Create(VToolPointOfIntersectionArcsInitData initData) -> VToolPointOfIntersectionArcs *;
    static auto FindPoint(const VArc *arc1, const VArc *arc2, CrossCirclesPoint pType, QPointF *intersectionPoint)
        -> bool;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::PointOfIntersectionArcs) };

    auto FirstArcName() const -> QString;
    auto SecondArcName() const -> QString;

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
    Q_DISABLE_COPY_MOVE(VToolPointOfIntersectionArcs) // NOLINT

    /** @brief firstArcId id first arc. */
    quint32 firstArcId;

    /** @brief secondArcId id second arc. */
    quint32 secondArcId;

    CrossCirclesPoint crossPoint;

    explicit VToolPointOfIntersectionArcs(const VToolPointOfIntersectionArcsInitData &initData,
                                          QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLPOINTOFINTERSECTIONARCS_H
