/************************************************************************
 **
 **  @file   tst_vlockguard.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 10, 2015
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

#include "tst_vlockguard.h"
#include "../vmisc/vlockguard.h"

#include <QtTest>

// Header <ciso646> is removed in C++20.
#if defined(Q_CC_MSVC) && __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

//---------------------------------------------------------------------------------------------------------------------
TST_VLockGuard::TST_VLockGuard(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLockGuard::TryLock() const
{
    QString fileName(QCoreApplication::applicationDirPath() + "/lockFile.txt");
    QSharedPointer<VLockGuard<char>> lock;
    VlpCreateLock(lock, fileName);

    // cppcheck-suppress nullPointer
    fileName = lock->GetLockFile();
    QVERIFY2(QFileInfo::exists(fileName), "Lock file doesn't exist!");

    lock.reset();
    QVERIFY2(not QFileInfo::exists(fileName), "Lock file still exists!");
}
