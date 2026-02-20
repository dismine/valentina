/************************************************************************
 **
 **  @file   vtoolshoulderpoint.h
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

#ifndef VTOOLSHOULDERPOINT_H
#define VTOOLSHOULDERPOINT_H

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

struct VToolShoulderPointInitData : VToolLinePointInitData
{
    QString formula{};
    quint32 p1Line{NULL_ID};
    quint32 p2Line{NULL_ID};
    quint32 pShoulder{NULL_ID};
};

/**
 * @brief The VToolShoulderPoint class tool for creation point on shoulder. This tool for special situation, when you
 * want find point along line, but have only length from another point (shoulder).
 */
class VToolShoulderPoint : public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    ~VToolShoulderPoint() override = default;

    void SetDialog() override;
    static auto FindPoint(const QPointF &p1Line, const QPointF &p2Line, const QPointF &pShoulder, qreal length)
        -> QPointF;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolShoulderPoint *;
    static auto Create(VToolShoulderPointInitData &initData) -> VToolShoulderPoint *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::ShoulderPoint) };

    auto SecondPointName() const -> QString;
    auto ShoulderPointName() const -> QString;

    auto GetP2Line() const -> quint32;
    void SetP2Line(const quint32 &value);

    auto getPShoulder() const -> quint32;
    void setPShoulder(const quint32 &value);

    void ShowVisualization(bool show) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

private:
    Q_DISABLE_COPY_MOVE(VToolShoulderPoint) // NOLINT

    /** @brief p2Line id second line point. */
    quint32 p2Line;

    /** @brief pShoulder id shoulder line point. */
    quint32 pShoulder;

    VToolShoulderPoint(const VToolShoulderPointInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLSHOULDERPOINT_H
