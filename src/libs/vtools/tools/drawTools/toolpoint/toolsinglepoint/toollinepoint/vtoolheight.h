/************************************************************************
 **
 **  @file   vtoolheight.h
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

#ifndef VTOOLHEIGHT_H
#define VTOOLHEIGHT_H

#include <QDomElement>
#include <QGraphicsItem>
#include <QLineF>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

struct VToolHeightInitData : VToolLinePointInitData
{
    quint32 basePointId{NULL_ID};
    quint32 p1LineId{NULL_ID};
    quint32 p2LineId{NULL_ID};
};

/**
 * @brief The VToolHeight class tool for creation point of height. Help find point of projection onto line.
 */
class VToolHeight: public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    ~VToolHeight() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolHeight *;
    static auto Create(VToolHeightInitData initData) -> VToolHeight *;
    static auto FindPoint(const QLineF &line, const QPointF &point) -> QPointF;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::Height)};

    auto FirstLinePointName() const -> QString;
    auto SecondLinePointName() const -> QString;

    void ShowVisualization(bool show) override;
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolHeight) // NOLINT

    /** @brief p1LineId id first point of line. */
    quint32         p1LineId;

    /** @brief p2LineId id second point of line. */
    quint32         p2LineId;

    explicit VToolHeight(const VToolHeightInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLHEIGHT_H
