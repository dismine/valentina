/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#ifndef VTOOLFLIPPINGBYLINE_H
#define VTOOLFLIPPINGBYLINE_H

#include <QtGlobal>

#include "vabstractflipping.h"

struct VToolFlippingByLineInitData : VAbstractOperationInitData
{
    quint32 firstLinePointId{NULL_ID};
    quint32 secondLinePointId{NULL_ID};
};


class VToolFlippingByLine : public VAbstractFlipping
{
    Q_OBJECT // NOLINT
public:
    ~VToolFlippingByLine() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolFlippingByLine *;
    static auto Create(VToolFlippingByLineInitData initData) -> VToolFlippingByLine *;

    static const QString ToolType;

    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::FlippingByLine)};

    auto FirstLinePointName() const -> QString;
    auto SecondLinePointName() const -> QString;

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
    Q_DISABLE_COPY_MOVE(VToolFlippingByLine) // NOLINT

    quint32 m_firstLinePointId;
    quint32 m_secondLinePointId;

    explicit VToolFlippingByLine(const VToolFlippingByLineInitData &initData, QGraphicsItem *parent = nullptr);
};

#endif // VTOOLFLIPPINGBYLINE_H
