/************************************************************************
 **
 **  @file   vtoolarcwithlength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
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

#ifndef VTOOLARCWITHLENGTH_H
#define VTOOLARCWITHLENGTH_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>


#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolabstractcurve.h"

class VFormula;
template <class T> class QSharedPointer;

struct VToolArcWithLengthInitData : VToolAbstractCurveInitData
{
    quint32 center{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString radius{'0'};     // NOLINT(misc-non-private-member-variables-in-classes)
    QString f1{'0'};         // NOLINT(misc-non-private-member-variables-in-classes)
    QString length{'0'};     // NOLINT(misc-non-private-member-variables-in-classes)
};

class VToolArcWithLength : public VToolAbstractArc
{
    Q_OBJECT // NOLINT

public:
    ~VToolArcWithLength() override = default;

    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolArcWithLength *;
    static auto Create(VToolArcWithLengthInitData &initData) -> VToolArcWithLength *;

    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::ArcWithLength)
    };
    auto getTagName() const -> QString override;

    auto GetFormulaRadius() const -> VFormula;
    void SetFormulaRadius(const VFormula &value);

    auto GetFormulaF1() const -> VFormula;
    void SetFormulaF1(const VFormula &value);

    auto GetFormulaLength() const -> VFormula;
    void SetFormulaLength(const VFormula &value);

    auto GetApproximationScale() const -> qreal override;
    void SetApproximationScale(qreal value);

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolArcWithLength) // NOLINT

    explicit VToolArcWithLength(const VToolArcWithLengthInitData &initData, QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLARCWITHLENGTH_H
