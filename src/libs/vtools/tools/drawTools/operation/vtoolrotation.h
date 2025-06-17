/************************************************************************
 **
 **  @file   vtoolrotation.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VTOOLROTATION_H
#define VTOOLROTATION_H

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "vabstractoperation.h"

template <class T> class QSharedPointer;
class VFormula;

struct VToolRotationInitData : VAbstractOperationInitData
{
    VToolRotationInitData()
      : VAbstractOperationInitData(),
        origin(NULL_ID),
        angle()
    {
    }

    quint32 origin;
    QString angle;
};

class VToolRotation : public VAbstractOperation
{
    Q_OBJECT // NOLINT

public:
    ~VToolRotation() override = default;

    void SetDialog() override;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolRotation *;
    static auto Create(VToolRotationInitData &initData) -> VToolRotation *;

    static const QString ToolType;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::Rotation)
    };

    auto OriginPointName() const -> QString;

    auto GetFormulaAngle() const -> VFormula;
    void SetFormulaAngle(const VFormula &value);

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SetVisualization() override;
    void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    auto MakeToolTip() const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(VToolRotation) // NOLINT
    quint32 origPointId;
    QString formulaAngle;

    explicit VToolRotation(const VToolRotationInitData &initData, QGraphicsItem *parent = nullptr);

    static auto CreatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                            const QString &suffix, VContainer *data) -> DestinationItem;

    template <class Item>
    static auto CreateItem(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                           const QString &suffix, VContainer *data) -> DestinationItem;
    template <class Item>
    static auto CreateArc(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                          const QString &suffix, VContainer *data) -> DestinationItem;
    template <class Item>
    static auto CreateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                            const QString &suffix, VContainer *data) -> DestinationItem;
    template <class Item>
    static auto CreateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                        const QString &suffix, VContainer *data) -> DestinationItem;

    static void UpdatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                            const QString &suffix, VContainer *data, const DestinationItem &item);
    template <class Item>
    static void UpdateItem(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                           const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void UpdateArc(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                          const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void UpdateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                            const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void UpdateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                        const QString &suffix, VContainer *data, quint32 id);
};

#endif // VTOOLROTATION_H
