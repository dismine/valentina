/************************************************************************
 **
 **  @file   vabstractpoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 6, 2015
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

#ifndef VABSTRACTPOINT_H
#define VABSTRACTPOINT_H

#include <QColor>
#include <QMetaObject>
#include <QObject>
#include <QPen>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"

class VAbstractPoint : public VDrawTool
{
    Q_OBJECT // NOLINT

public:
    VAbstractPoint(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes);
    ~VAbstractPoint() override = default;

    auto getTagName() const -> QString override;

    template <typename T> void ShowToolVisualization(bool show);

public slots:
    void DeleteFromLabel();

protected:
    struct ToolChanges
    {
        quint32 pointId{NULL_ID};
        QString oldLabel{};
        QString newLabel{};

        auto HasChanges() const -> bool { return oldLabel != newLabel; }

        auto LabelChanged() const -> bool { return oldLabel != newLabel; }
    };

    void UpdatePointName(
        quint32 pointId,
        const QString &name,
        const std::function<void(const QDomElement &, const QDomElement &, const ToolChanges &)> &ProcessOptions);

    void ProcessPointToolOptions(const QDomElement &oldDomElement,
                                 const QDomElement &newDomElement,
                                 const ToolChanges &changes);

    template <typename T> static void InitToolConnections(VMainGraphicsScene *scene, T *tool);

private:
    Q_DISABLE_COPY_MOVE(VAbstractPoint) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VAbstractPoint::ShowToolVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<T>();
            SetVisualization();
        }
        else
        {
            if (T *visual = qobject_cast<T *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis.data();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VAbstractPoint::InitToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnablePointItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnablePointItemSelection, tool, &T::AllowSelecting);
    QObject::connect(scene, &VMainGraphicsScene::EnableLabelItemHover, tool, &T::AllowLabelHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableLabelItemSelection, tool, &T::AllowLabelSelecting);
}

#endif // VABSTRACTPOINT_H
