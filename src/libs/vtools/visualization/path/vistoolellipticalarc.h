/************************************************************************
 **
 **  @file   vistoolellipticalarc.h
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   24 10, 2016
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
#ifndef VISTOOLELLIPTICALARC_H
#define VISTOOLELLIPTICALARC_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolEllipticalArc : public VisPath
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolEllipticalArc(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolEllipticalArc() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetCenterId(quint32 newCenterId);
    void SetRadius1(const QString &expression);
    void SetRadius2(const QString &expression);
    void SetF1(const QString &expression);
    void SetF2(const QString &expression);
    void SetRotationAngle(const QString &expression);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolEllipticalArc)};
private:
    Q_DISABLE_COPY_MOVE(VisToolEllipticalArc) // NOLINT
    VScaledEllipse *m_arcCenter{nullptr};
    qreal           m_radius1{0};
    qreal           m_radius2{0};
    qreal           m_f1{0};
    qreal           m_f2{0};
    qreal           m_rotationAngle{0};
    quint32         m_centerId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolEllipticalArc::SetCenterId(quint32 newCenterId)
{
    m_centerId = newCenterId;
}

#endif // VISTOOLELLIPTICALARC_H
