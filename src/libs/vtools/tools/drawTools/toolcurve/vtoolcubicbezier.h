/************************************************************************
 **
 **  @file   vtoolcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
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

#ifndef VTOOLCUBICBEZIER_H
#define VTOOLCUBICBEZIER_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolabstractcurve.h"

class VCubicBezier;
template <class T> class QSharedPointer;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct VToolCubicBezierInitData : VDrawToolInitData
{
    VCubicBezier *spline{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)
};

QT_WARNING_POP

class VToolCubicBezier : public VToolAbstractBezier
{
    Q_OBJECT // NOLINT

public:
    ~VToolCubicBezier() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolCubicBezier *;
    static auto Create(VToolCubicBezierInitData initData) -> VToolCubicBezier *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::CubicBezier)
    };

    auto FirstPointName() const -> QString;
    auto SecondPointName() const -> QString;
    auto ThirdPointName() const -> QString;
    auto ForthPointName() const -> QString;

    auto getSpline() const -> VCubicBezier;
    void setSpline(const VCubicBezier &spl);

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void SetVisualization() override;
    void RefreshGeometry() override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolCubicBezier) // NOLINT

    explicit VToolCubicBezier(const VToolCubicBezierInitData &initData, QGraphicsItem *parent = nullptr);

    void SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLCUBICBEZIER_H
