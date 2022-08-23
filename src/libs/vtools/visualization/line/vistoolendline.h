/************************************************************************
 **
 **  @file   vistoolendline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 7, 2014
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

#ifndef VISTOOLENDLINE_H
#define VISTOOLENDLINE_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolEndLine final : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolEndLine(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolEndLine() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    auto Angle() const -> QString;
    void SetAngle(const QString &expression);

    auto Length() const -> QString;
    void SetLength(const QString &expression);

    void SetBasePointId(quint32 id);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolEndLine)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolEndLine) // NOLINT
    qreal           m_length{0};
    qreal           m_angle{0};
    VScaledEllipse *m_point{nullptr};
    quint32         m_basePointId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolEndLine::SetBasePointId(quint32 id)
{
    m_basePointId = id;
}

#endif // VISTOOLENDLINE_H
