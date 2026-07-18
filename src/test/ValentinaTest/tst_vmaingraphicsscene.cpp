/************************************************************************
 **
 **  @file   tst_vmaingraphicsscene.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 7, 2026
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
#include "tst_vmaingraphicsscene.h"

#include "../vwidgets/vmaingraphicsscene.h"

#include <QGraphicsLineItem>
#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VMainGraphicsScene::TST_VMainGraphicsScene(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// Regression test for a use-after-free crash (EXC_BAD_ACCESS during undo -> MainWindow::Clear()).
// SetOriginsVisible() must toggle only the origin (axis) items and must stay safe after the scene has been
// cleared - it derives them from the live scene, so a stale pointer to a deleted item can never be touched.
void TST_VMainGraphicsScene::SetOriginsVisibleIsSceneDerived()
{
    VMainGraphicsScene scene;

    scene.InitOrigins();
    auto *other = new QGraphicsLineItem(QLineF(0, 0, 10, 10)); // a non-origin item that must not be toggled
    scene.addItem(other);

    const QList<QGraphicsItem *> allItems = scene.items();
    QVERIFY2(allItems.size() > 1, "InitOrigins() must add origin items to the scene.");

    scene.SetOriginsVisible(false);
    for (auto *item : allItems)
    {
        // Origin items carry a marker (data key 0); everything else must keep its visibility.
        QCOMPARE(item->isVisible(), not item->data(0).toBool());
    }
    QVERIFY2(other->isVisible(), "Non-origin items must not be affected by SetOriginsVisible().");

    // After clearing the scene the origin items are gone; this must be a safe no-op, not a dangling access.
    scene.clear();
    scene.SetOriginsVisible(true);
    QVERIFY(scene.items().isEmpty());
}
