/************************************************************************
 **
 **  @file   tst_vcontainer.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 8, 2026
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

#ifndef TST_VCONTAINER_H
#define TST_VCONTAINER_H

#include <QObject>

class TST_VContainer : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VContainer(QObject *parent = nullptr);
    ~TST_VContainer() override = default;

private slots:
    // Nothing rejects a second object claiming a name another object already registered.
    // This is the gap that lets a name freed by deleting/undoing a tool be silently claimed
    // twice once the deleted tool is redone/restored.
    void DuplicateNameIsNotRejected();

    // UpdateGObject() rewrites the object's own hash entry before the duplicate-name warning
    // check runs, so a routine recompute of an object must not warn about colliding with itself.
    void UpdatingObjectWithUnchangedNameDoesNotWarn();

    // A Draw::Modeling point/curve is an intentional copy of its source Draw::Calculation object
    // and always shares its name by design (see VNodePoint::Create() and friends) -- not a bug.
    void ModelingMirrorSharingCalculationNameDoesNotWarn();

    // Regression tests: pin snapshot semantics preserved by the rewrite.
    void OldToolSnapshotUnaffectedByLaterStructuralChange();
    void OldToolSnapshotSeesValueUpdateToExistingVariable();

    // Benchmarks: measure QHash baseline before persistent-map swap.
    void BenchmarkToolParseShape_data();
    void BenchmarkToolParseShape();
    void BenchmarkVariableLookup_data();
    void BenchmarkVariableLookup();
    void BenchmarkObjectLookup_data();
    void BenchmarkObjectLookup();

private:
    Q_DISABLE_COPY_MOVE(TST_VContainer) // NOLINT
};

#endif // TST_VCONTAINER_H
