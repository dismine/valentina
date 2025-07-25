/************************************************************************
 **
 **  @file   tst_svgfontwritingsystem.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 6, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef TST_SVGFONTWRITINGSYSTEM_H
#define TST_SVGFONTWRITINGSYSTEM_H

#include <QObject>

class TST_SVGFontWritingSystem : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_SVGFontWritingSystem(QObject *parent = nullptr);
    ~TST_SVGFontWritingSystem() override = default;

private slots:
    void TestSimplifiedChineseWritingSystemValidCases_data();
    void TestSimplifiedChineseWritingSystemValidCases();
    void TestSimplifiedChineseWritingSystemOutOfRangeCases();

    void TestTraditionalChineseWritingSystemValidCases_data();
    void TestTraditionalChineseWritingSystemValidCases();
    void TestTraditionalChineseWritingSystemOutOfRangeCases();

private:
    Q_DISABLE_COPY_MOVE(TST_SVGFontWritingSystem) // NOLINT
};

#endif // TST_SVGFONTWRITINGSYSTEM_H
