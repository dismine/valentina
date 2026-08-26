/************************************************************************
 **
 **  @file   tst_fancytabbar.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 8, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "tst_fancytabbar.h"
#include "../vwidgets/fancytabbar/fancytabbar.h"

#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_FancyTabBar::TST_FancyTabBar(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// A tab's hover-fade QPropertyAnimation targets the FancyTab QObject, which holds a raw, non-owning pointer back to
// the tab bar. Destroying the tab bar while that animation is still running must not leave the animation's timer
// ticking against freed memory (~FancyTabBar used to leak the FancyTab, see the crash this guards against).
void TST_FancyTabBar::TestHoverAnimationDoesNotOutliveDestroyedTabBar()
{
    auto *tabBar = new FancyTabBar(FancyTabBar::Left); // NOLINT(cppcoreguidelines-owning-memory)
    tabBar->resize(100, 400);
    tabBar->InsertTab(0, QIcon(), QStringLiteral("Test"));

    // Start the hover-fade animation without waiting for it to finish.
    const QPointF pos = tabBar->TabRect(0).center();
    QMouseEvent moveEvent(QEvent::MouseMove,
                          pos,
                          tabBar->mapToGlobal(pos.toPoint()),
                          Qt::NoButton,
                          Qt::NoButton,
                          Qt::NoModifier);
    QCoreApplication::sendEvent(tabBar, &moveEvent);

    delete tabBar; // Must stop the animation (and its timer), not orphan it.

    // If the animation's timer is still alive, this tick calls FancyTab::setFader() -> m_TabBar->update() on
    // freed memory (EXCEPTION_ACCESS_VIOLATION_READ on Windows, a crash or ASan failure everywhere else).
    QTest::qWait(200);
}
