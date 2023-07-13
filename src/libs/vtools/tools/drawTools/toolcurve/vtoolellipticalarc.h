/************************************************************************
 **
 **  @file   vtoolellipticalarc.h
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   20 10, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VTOOLELLIPTICALARC_H
#define VTOOLELLIPTICALARC_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vabstractspline.h"

class VFormula;
template <class T> class QSharedPointer;

struct VToolEllipticalArcInitData : VAbstractSplineInitData
{
    VToolEllipticalArcInitData() = default;

    quint32 center{NULL_ID};    // NOLINT(misc-non-private-member-variables-in-classes)
    QString radius1{'0'};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString radius2{'0'};       // NOLINT(misc-non-private-member-variables-in-classes)
    QString f1{'0'};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString f2{'0'};            // NOLINT(misc-non-private-member-variables-in-classes)
    QString rotationAngle{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VToolEllipticalArc class tool for creation elliptical arc.
 */
class VToolEllipticalArc : public VToolAbstractArc
{
    Q_OBJECT // NOLINT

public:
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolEllipticalArc *;
    static auto Create(VToolEllipticalArcInitData &initData) -> VToolEllipticalArc *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::EllipticalArc)
    };
    auto getTagName() const -> QString override;

    auto GetFormulaRadius1() const -> VFormula;
    void SetFormulaRadius1(const VFormula &value);

    auto GetFormulaRadius2() const -> VFormula;
    void SetFormulaRadius2(const VFormula &value);

    auto GetFormulaF1() const -> VFormula;
    void SetFormulaF1(const VFormula &value);

    auto GetFormulaF2() const -> VFormula;
    void SetFormulaF2(const VFormula &value);

    auto GetFormulaRotationAngle() const -> VFormula;
    void SetFormulaRotationAngle(const VFormula &value);

    auto GetApproximationScale() const -> qreal;
    void SetApproximationScale(qreal value);

    void ShowVisualization(bool show) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void RemoveReferens() override;
    void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(VToolEllipticalArc) // NOLINT
    explicit VToolEllipticalArc(const VToolEllipticalArcInitData &initData, QGraphicsItem *parent = nullptr);
    ~VToolEllipticalArc() = default;
};

#endif // VTOOLELLIPTICALARC_H
