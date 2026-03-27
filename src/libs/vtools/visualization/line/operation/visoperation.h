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

#ifndef VISOPERATION_H
#define VISOPERATION_H

#include <QtGlobal>

#include "../visline.h"
#include "../vpatterndb/vcontainer.h"

class VisOperation : public VisLine
{
    Q_OBJECT // NOLINT

public:
    explicit VisOperation(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisOperation() override;

    void SetObjects(const QVector<quint32> &objects);
    void StartAction();
    void VisualMode(quint32 id) override;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::GroupOperation)
    };

protected:
    auto GetPoint(quint32 i, VColorRole role) -> VScaledEllipse *;
    auto GetCurve(quint32 i, VColorRole role) -> VCurvePathItem *;

    auto Objects() const -> const QVector<quint32> &;

    auto ObjectSelected() const -> bool;

    auto AddOriginCurve(quint32 id, int &i) -> QGraphicsPathItem *;
    auto AddOriginPoint(quint32 id, int &i) -> VScaledEllipse *;

    auto CreateOriginObjects(int &iPoint, int &iCurve) -> QVector<QGraphicsItem *>;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisOperation) // NOLINT

    QVector<quint32> m_objects{};

    QVector<VScaledEllipse *> m_points{};
    QVector<VCurvePathItem *> m_curves{};

    bool m_startAction{false};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisOperation::StartAction()
{
    m_startAction = true;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VisOperation::Objects() const -> const QVector<quint32> &
{
    return m_objects;
}

#endif // VISOPERATION_H
