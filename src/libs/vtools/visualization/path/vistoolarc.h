/************************************************************************
 **
 **  @file   vistoolarc.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#ifndef VISTOOLARC_H
#define VISTOOLARC_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "defglobal.h"
#include "vispath.h"

class VisToolArc final : public VisPath
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolArc(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolArc() override =default;

    void RefreshGeometry() override;
    void setRadius(const QString &expression);
    void setF1(const QString &expression);
    void setF2(const QString &expression);
    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::ToolArc)};

    static auto CorrectAngle(qreal angle) -> qreal;
    auto StickyEnd(qreal angle) const -> qreal;
private:
    Q_DISABLE_COPY_MOVE(VisToolArc) // NOLINT
    VScaledEllipse *arcCenter{nullptr};
    VScaledEllipse *f1Point{nullptr};
    qreal           radius{0};
    qreal           f1{-1};
    qreal           f2{-1};
};

#endif // VISTOOLARC_H
