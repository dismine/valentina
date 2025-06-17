/************************************************************************
 **
 **  @file   vistoolsplinepath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 9, 2014
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

#ifndef VISTOOLSPLINEPATH_H
#define VISTOOLSPLINEPATH_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vgeometry/vsplinepath.h"
#include "../vmisc/def.h"
#include "vispath.h"

class VControlPointSpline;

class VisToolSplinePath : public VisPath // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolSplinePath(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolSplinePath() override;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPath(const VSplinePath &value);
    auto GetPath() -> VSplinePath;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolSplinePath)
    };
signals:
    void PathChanged(const VSplinePath &path);

public slots:
    void MouseLeftPressed();
    void MouseLeftReleased();

private:
    Q_DISABLE_COPY_MOVE(VisToolSplinePath) // NOLINT
    QVector<VScaledEllipse *> m_points{};
    QVector<VControlPointSpline *> m_ctrlPoints{};
    VCurvePathItem *m_newCurveSegment{nullptr};
    VSplinePath m_path{};

    bool m_isLeftMousePressed{false};
    bool m_pointSelected{false};

    QPointF m_ctrlPoint{};

    auto GetPoint(quint32 i) -> VScaledEllipse *;
    void DragControlPoint(vsizetype lastPoint, vsizetype preLastPoint, const QPointF &pSpl, vsizetype size);
    void NewCurveSegment(const VSpline &spline, const QPointF &pSpl, vsizetype size);
    void Creating(const QPointF &pSpl, vsizetype size);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSplinePath::SetPath(const VSplinePath &value)
{
    m_path = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VisToolSplinePath::GetPath() -> VSplinePath
{
    return m_path;
}

#endif // VISTOOLSPLINEPATH_H
