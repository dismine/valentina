/************************************************************************
 **
 **  @file   tst_vtheme.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 10, 2025
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
#ifndef TST_VTHEME_H
#define TST_VTHEME_H

#include <QObject>

class TST_VTheme : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VTheme(QObject *parent = nullptr);
    ~TST_VTheme() override = default;

private slots:
    void initTestCase();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void TestThemeIconMappings_data();
    void TestThemeIconMappings();
#endif
    void TestThemeIconStringMappings_data();
    void TestThemeIconStringMappings();
    void cleanupTestCase();

private:
    Q_DISABLE_COPY_MOVE(TST_VTheme) // NOLINT

    QStringList m_themeSearchPaths{};
    QString m_themeName{};
};

#endif // TST_VTHEME_H
