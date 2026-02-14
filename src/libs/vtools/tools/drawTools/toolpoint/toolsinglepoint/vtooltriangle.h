/************************************************************************
 **
 **  @file   vtooltriangle.h
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

#ifndef VTOOLTRIANGLE_H
#define VTOOLTRIANGLE_H

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

struct VToolTriangleInitData : VToolSinglePointInitData
{
    quint32 axisP1Id{NULL_ID};      // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 axisP2Id{NULL_ID};      // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 firstPointId{NULL_ID};  // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 secondPointId{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VToolTriangle class for tool that find point intersection two foots right triangle
 * (triangle with 90 degree).
 */
class VToolTriangle : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    ~VToolTriangle() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolTriangle *;
    static auto Create(VToolTriangleInitData initData) -> VToolTriangle *;
    static auto FindPoint(const QPointF &axisP1, const QPointF &axisP2, const QPointF &firstPoint,
                          const QPointF &secondPoint, QPointF *intersectionPoint) -> bool;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::Triangle)
    };

    auto AxisP1Name() const -> QString;
    auto AxisP2Name() const -> QString;
    auto FirstPointName() const -> QString;
    auto SecondPointName() const -> QString;

    auto GetAxisP1Id() const -> quint32;
    void SetAxisP1Id(const quint32 &value);

    auto GetAxisP2Id() const -> quint32;
    void SetAxisP2Id(const quint32 &value);

    auto GetFirstPointId() const -> quint32;
    void SetFirstPointId(const quint32 &value);

    auto GetSecondPointId() const -> quint32;
    void SetSecondPointId(const quint32 &value);

    void ShowVisualization(bool show) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

private:
    Q_DISABLE_COPY_MOVE(VToolTriangle) // NOLINT
    /** @brief axisP1Id id first axis point. */
    quint32 axisP1Id;

    /** @brief axisP2Id id second axis point. */
    quint32 axisP2Id;

    /** @brief firstPointId id first triangle point, what lies on the hypotenuse. */
    quint32 firstPointId;

    /** @brief secondPointId id second triangle point, what lies on the hypotenuse. */
    quint32 secondPointId;

    explicit VToolTriangle(const VToolTriangleInitData &initData, QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLTRIANGLE_H
