/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
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

#ifndef VTOOLFLIPPINGBYAXIS_H
#define VTOOLFLIPPINGBYAXIS_H

#include <QtGlobal>

#include "vabstractflipping.h"

struct VToolFlippingByAxisInitData : VAbstractOperationInitData
{
    quint32 originPointId{NULL_ID};
    AxisType axisType{AxisType::VerticalAxis};
};

class VToolFlippingByAxis : public VAbstractFlipping
{
    Q_OBJECT // NOLINT

public:
    ~VToolFlippingByAxis() override = default;

    void SetDialog() override;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolFlippingByAxis *;
    static auto Create(VToolFlippingByAxisInitData initData) -> VToolFlippingByAxis *;

    static const QString ToolType;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::FlippingByAxis)
    };

    auto GetAxisType() const -> AxisType;
    void SetAxisType(AxisType value);

    auto OriginPointName() const -> QString;

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SetVisualization() override;
    void SaveDialog(QDomElement &domElement) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolFlippingByAxis) // NOLINT

    quint32 m_originPointId;
    AxisType m_axisType;

    explicit VToolFlippingByAxis(const VToolFlippingByAxisInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLFLIPPINGBYAXIS_H
