/************************************************************************
 **
 **  @file   tst_xsdschema.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 2, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef TST_XSDSCHEMA_H
#define TST_XSDSCHEMA_H

#include <QObject>

class TST_XSDShema : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_XSDShema(QObject *parent = nullptr);
    ~TST_XSDShema() override = default;

private slots:
    void TestPatternSchema_data();
    void TestPatternSchema();
    void TestPatternLabelSchema_data();
    void TestPatternLabelSchema();
    void TestLayoutSchema_data();
    void TestLayoutSchema();
    void TestVITSchema_data();
    void TestVITSchema();
    void TestVSTSchema_data();
    void TestVSTSchema();
    void TestWatermarkSchema_data();
    void TestWatermarkSchema();

private:
    Q_DISABLE_COPY_MOVE(TST_XSDShema) // NOLINT
};

#endif // TST_XSDSCHEMA_H
