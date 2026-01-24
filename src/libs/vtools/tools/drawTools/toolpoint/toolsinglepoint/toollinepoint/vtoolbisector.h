/************************************************************************
 **
 **  @file   vtoolbisector.h
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

#ifndef VTOOLBISECTOR_H
#define VTOOLBISECTOR_H


#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

struct VToolBisectorInitData : VToolLinePointInitData
{
    QString formula{QStringLiteral("100.0")};
    quint32 firstPointId{NULL_ID};
    quint32 secondPointId{NULL_ID};
    quint32 thirdPointId{NULL_ID};
};

/**
 * @brief The VToolBisector class tool for creation bisector point.
 */
class VToolBisector : public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    static auto BisectorAngle(const QPointF &firstPoint, const QPointF &secondPoint, const QPointF &thirdPoint)
        -> qreal;
    static auto FindPoint(const QPointF &firstPoint, const QPointF &secondPoint, const QPointF &thirdPoint,
                          const qreal &length) -> QPointF;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolBisector *;
    static auto Create(VToolBisectorInitData &initData) -> VToolBisector *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::Bisector)};

    auto FirstPointName() const -> QString;
    auto ThirdPointName() const -> QString;

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
    Q_DISABLE_COPY_MOVE(VToolBisector) // NOLINT

    /** @brief firstPointId id first point of angle. */
    quint32 firstPointId;

    /** @brief thirdPointId id third point of angle. */
    quint32 thirdPointId;

    explicit VToolBisector(const VToolBisectorInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLBISECTOR_H
