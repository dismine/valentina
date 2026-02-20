/************************************************************************
 **
 **  @file   vtooltruedarts.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 6, 2015
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

#ifndef VTOOLTRUEDARTS_H
#define VTOOLTRUEDARTS_H

#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtooldoublepoint.h"

template <class T> class QSharedPointer;

struct VToolTrueDartsInitData : VDrawToolInitData
{
    quint32 p1id{NULL_ID};         // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 p2id{NULL_ID};         // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 baseLineP1Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 baseLineP2Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 dartP1Id{NULL_ID};     // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 dartP2Id{NULL_ID};     // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 dartP3Id{NULL_ID};     // NOLINT(misc-non-private-member-variables-in-classes)
    QString name1{};               // NOLINT(misc-non-private-member-variables-in-classes)
    qreal mx1{labelMX};            // NOLINT(misc-non-private-member-variables-in-classes)
    qreal my1{labelMY};            // NOLINT(misc-non-private-member-variables-in-classes)
    bool showLabel1{true};         // NOLINT(misc-non-private-member-variables-in-classes)
    QString name2{};               // NOLINT(misc-non-private-member-variables-in-classes)
    qreal mx2{10};                 // NOLINT(misc-non-private-member-variables-in-classes)
    qreal my2{15};                 // NOLINT(misc-non-private-member-variables-in-classes)
    bool showLabel2{true};         // NOLINT(misc-non-private-member-variables-in-classes)
};

class VToolTrueDarts : public VToolDoublePoint
{
    Q_OBJECT // NOLINT

public:
    ~VToolTrueDarts() override = default;
    static void FindPoint(const QPointF &baseLineP1, const QPointF &baseLineP2, const QPointF &dartP1,
                          const QPointF &dartP2, const QPointF &dartP3, QPointF &p1, QPointF &p2);
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolTrueDarts *;
    static auto Create(VToolTrueDartsInitData initData) -> VToolTrueDarts *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::TrueDarts)
    };

    void ShowVisualization(bool show) override;

    auto BaseLineP1Name() const -> QString;
    auto BaseLineP2Name() const -> QString;
    auto DartP1Name() const -> QString;
    auto DartP2Name() const -> QString;
    auto DartP3Name() const -> QString;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolTrueDarts) // NOLINT
    quint32 baseLineP1Id;
    quint32 baseLineP2Id;
    quint32 dartP1Id;
    quint32 dartP2Id;
    quint32 dartP3Id;

    explicit VToolTrueDarts(const VToolTrueDartsInitData &initData, QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLTRUEDARTS_H
