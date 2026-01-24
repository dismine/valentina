/************************************************************************
 **
 **  @file   vtoollineintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
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

#ifndef VTOOLLINEINTERSECTAXIS_H
#define VTOOLLINEINTERSECTAXIS_H


#include <QDomElement>
#include <QGraphicsItem>
#include <QLineF>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/vformula.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

struct VToolLineIntersectAxisInitData : VToolLinePointInitData
{
    QString formulaAngle{'0'};
    quint32 basePointId{NULL_ID};
    quint32 firstPointId{NULL_ID};
    quint32 secondPointId{NULL_ID};
};

class VToolLineIntersectAxis : public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    ~VToolLineIntersectAxis() override = default;
    void SetDialog() override;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolLineIntersectAxis *;
    static auto Create(VToolLineIntersectAxisInitData &initData) -> VToolLineIntersectAxis *;

    static auto FindPoint(const QLineF &axis, const QLineF &line, QPointF *intersectionPoint) -> bool;

    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::LineIntersectAxis)};

    auto FirstLinePoint() const -> QString;
    auto SecondLinePoint() const -> QString;

    auto GetFormulaAngle() const -> VFormula;
    void SetFormulaAngle(const VFormula &value);

    void ShowVisualization(bool show) override;
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolLineIntersectAxis) // NOLINT

    QString formulaAngle;
    quint32 firstPointId;
    quint32 secondPointId;

    explicit VToolLineIntersectAxis(const VToolLineIntersectAxisInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLLINEINTERSECTAXIS_H
