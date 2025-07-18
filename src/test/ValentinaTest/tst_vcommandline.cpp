/************************************************************************
 **
 **  @file   tst_vcommandline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 11, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "tst_vcommandline.h"
#include "../vmisc/commandoptions.h"

#include <QtTest>

// Header <ciso646> is removed in C++20.
#if defined(Q_CC_MSVC) && __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VCommandLine::TST_VCommandLine(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VCommandLine::UniqueKeys()
{
    const QStringList options = AllKeys();
    QSet<QString> unique;

    for (const auto &str : options)
    {
        // cppcheck-suppress unreadVariable
        const QString message = u"Options '%1' is not unique!"_s.arg(str);
        QVERIFY2(not unique.contains(str), qUtf8Printable(message));
        unique.insert(str);
    }
}
