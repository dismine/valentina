/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 9, 2016
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

#ifndef TST_VSPLINEPATH_H
#define TST_VSPLINEPATH_H

#include <QObject>

class TST_VSplinePath : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VSplinePath(QObject *parent = nullptr);

private slots:
    void TestRotation_data();
    void TestRotation();
    void TestFlip_data();
    void TestFlip();

private:
    Q_DISABLE_COPY_MOVE(TST_VSplinePath) // NOLINT
};

#endif // TST_VSPLINEPATH_H
