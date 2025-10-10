/************************************************************************
 **
 **  @file   tst_vtheme.cpp
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
#include "tst_vtheme.h"

#include <QIcon>
#include <QtTest>

#include "../vmisc/theme/themeDef.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VTheme::TST_VTheme(QObject *parent)
  : QObject{parent}
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTheme::initTestCase()
{
    m_themeSearchPaths = QIcon::themeSearchPaths();
    QIcon::setThemeSearchPaths({":/icons"});

    m_themeName = QIcon::themeName();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
//---------------------------------------------------------------------------------------------------------------------
void TST_VTheme::TestThemeIconMappings_data()
{
    QTest::addColumn<QString>("themeName");
    QTest::addColumn<VThemeIcon>("icon");

    QHash<VThemeIcon, QIcon::ThemeIcon> const mappings = ThemeIconMappings();
    QHash<VThemeIcon, QString> const stringMappings = ThemeIconStringMappings();

    QStringList const themes{
#if defined(Q_OS_MACX)
        "La-Sierra-Light"_L1,
        "La-Sierra-Dark"_L1,
#else
        "Eleven-Light"_L1,
        "Eleven-Dark"_L1,
#endif
    };

    for (const auto &theme : themes)
    {
        for (auto i = mappings.cbegin(), end = mappings.cend(); i != end; ++i)
        {
            QString const testCase = QStringLiteral("%1: %2").arg(theme, stringMappings.value(i.key(), "Unknown"_L1));
            QTest::newRow(qPrintable(testCase)) << theme << i.key();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTheme::TestThemeIconMappings()
{
    QFETCH(QString, themeName);
    QFETCH(VThemeIcon, icon);

    QIcon::setThemeName(themeName);

    QVERIFY(HasThemeIcon(icon));
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void TST_VTheme::TestThemeIconStringMappings_data()
{
    QTest::addColumn<QString>("themeName");
    QTest::addColumn<QString>("icon");

    QHash<VThemeIcon, QString> const stringMappings = ThemeIconStringMappings();

    QStringList const themes{
#if defined(Q_OS_MACX)
        "La-Sierra-Light"_L1,
        "La-Sierra-Dark"_L1,
#else
        "Eleven-Light"_L1,
        "Eleven-Dark"_L1,
#endif
    };

    for (const auto &theme : themes)
    {
        for (auto i = stringMappings.cbegin(), end = stringMappings.cend(); i != end; ++i)
        {
            QString const testCase = QStringLiteral("%1: %2").arg(theme, stringMappings.value(i.key()));
            QTest::newRow(qPrintable(testCase)) << theme << i.value();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTheme::TestThemeIconStringMappings()
{
    QFETCH(QString, themeName);
    QFETCH(QString, icon);

    QIcon::setThemeName(themeName);

    QVERIFY(QIcon::hasThemeIcon(icon));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VTheme::cleanupTestCase()
{
    QIcon::setThemeSearchPaths(m_themeSearchPaths);
    QIcon::setThemeName(m_themeName);
}
