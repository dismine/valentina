/************************************************************************
 **
 **  @file   vistoolcubicbezierpath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 3, 2016
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

#ifndef VISTOOLCUBICBEZIERPATH_H
#define VISTOOLCUBICBEZIERPATH_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vgeometry/vcubicbezierpath.h"
#include "../vmisc/def.h"
#include "vispath.h"

class VisToolCubicBezierPath : public VisPath
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolCubicBezierPath(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolCubicBezierPath() override;

    void RefreshGeometry() override;
    void VisualMode(quint32 id = NULL_ID) override;

    void SetPath(const VCubicBezierPath &value);
    auto GetPath() const -> VCubicBezierPath;

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolCubicBezierPath)};
private:
    Q_DISABLE_COPY_MOVE(VisToolCubicBezierPath) // NOLINT
    QVector<VScaledEllipse *>  mainPoints{};
    QVector<VScaledEllipse *>  ctrlPoints{};
    QVector<VScaledLine *>     lines{};
    VCurvePathItem            *newCurveSegment{nullptr};
    VCubicBezierPath           path{};
    VScaledLine               *helpLine1{nullptr};
    VScaledLine               *helpLine2{nullptr};

    auto GetPoint(QVector<VScaledEllipse *> &points, quint32 i, qreal z = 0) -> VScaledEllipse *;
    auto GetLine(quint32 i) -> VScaledLine *;
    void Creating(const QVector<VPointF> &pathPoints , vsizetype pointsLeft);
    void RefreshToolTip();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VisToolCubicBezierPath::GetPath() const -> VCubicBezierPath
{
    return path;
}

#endif // VISTOOLCUBICBEZIERPATH_H
