/************************************************************************
 **
 **  @file   tst_formulacache.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 6, 2026
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

#ifndef TST_FORMULACACHE_H
#define TST_FORMULACACHE_H

#include <QObject>

// Guards the per-thread compiled-bytecode cache in VAbstractTool::CheckFormula: a cached formula
// must re-read current variable values on every evaluation (no stale results) and match a fresh,
// uncached evaluation.
class TST_FormulaCache : public QObject
{
    Q_OBJECT // NOLINT
public:
    explicit TST_FormulaCache(QObject *parent = nullptr);

private slots:
    void RefreshesValueOnReevaluation();
};

#endif // TST_FORMULACACHE_H
