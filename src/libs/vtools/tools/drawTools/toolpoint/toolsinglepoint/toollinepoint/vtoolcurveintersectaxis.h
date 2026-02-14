/************************************************************************
 **
 **  @file   vtoolcurveintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VTOOLCURVEINTERSECTAXIS_H
#define VTOOLCURVEINTERSECTAXIS_H

#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vpatterndb/vformula.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

struct VToolCurveIntersectAxisInitData : VToolLinePointInitData
{
    QString formulaAngle{'0'};
    quint32 basePointId{NULL_ID};
    quint32 curveId{NULL_ID};
    QPair<QString, QString> segments{};
    QString aliasSuffix1{};
    QString aliasSuffix2{};
    QString name1{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString name2{}; // NOLINT(misc-non-private-member-variables-in-classes)
};

enum class VToolCurveIntersectAxisNameField : quint8
{
    Name1,
    Name2,
    AliasSuffix1,
    AliasSuffix2
};

class VToolCurveIntersectAxis : public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    ~VToolCurveIntersectAxis() override = default;
    void SetDialog() override;

    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolCurveIntersectAxis *;
    static auto Create(VToolCurveIntersectAxisInitData &initData) -> VToolCurveIntersectAxis *;

    static auto FindPoint(const QPointF &point, qreal angle, const QVector<QPointF> &curvePoints,
                          QPointF *intersectionPoint) -> bool;

    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::CurveIntersectAxis)};

    auto GetFormulaAngle() const -> VFormula;
    void SetFormulaAngle(const VFormula &value);

    auto GetName1() const -> QString;
    void SetName1(const QString &name);

    auto GetName2() const -> QString;
    void SetName2(const QString &name);

    auto GetAliasSuffix1() const -> QString;
    void SetAliasSuffix1(const QString &alias);

    auto GetAliasSuffix2() const -> QString;
    void SetAliasSuffix2(const QString &alias);

    auto CurveName() const -> QString;

    void ShowVisualization(bool show) override;
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void SetSegments(const QPair<QString, QString> &segments);
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private:
    Q_DISABLE_COPY_MOVE(VToolCurveIntersectAxis) // NOLINT
    QString formulaAngle;
    quint32 curveId;
    QPair<QString, QString> m_segments{};
    QString m_name1{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_name2{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_aliasSuffix1{};
    QString m_aliasSuffix2{};

    struct ToolChanges
    {
        QString oldLabel{};
        QString newLabel{};
        QString oldName1{};
        QString newName1{};
        QString oldName2{};
        QString newName2{};
        QString oldAliasSuffix1{};
        QString newAliasSuffix1{};
        QString oldAliasSuffix2{};
        QString newAliasSuffix2{};

        auto HasChanges() const -> bool
        {
            return oldLabel != newLabel || oldName1 != newName1 || oldName2 != newName2
                   || oldAliasSuffix1 != newAliasSuffix1 || oldAliasSuffix2 != newAliasSuffix2;
        }

        auto LabelChanged() const -> bool { return oldLabel != newLabel; }
        auto Name1Changed() const -> bool { return oldName1 != newName1; }
        auto Name2Changed() const -> bool { return oldName2 != newName2; }
        auto AliasSuffix1Changed() const -> bool { return oldAliasSuffix1 != newAliasSuffix1; }
        auto AliasSuffix2Changed() const -> bool { return oldAliasSuffix2 != newAliasSuffix2; }
    };

    explicit VToolCurveIntersectAxis(const VToolCurveIntersectAxisInitData &initData, QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;

    void ProcessToolOptions(const QDomElement &oldDomElement,
                            const QDomElement &newDomElement,
                            const ToolChanges &changes);

    template <class Item>
    static void InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId);
    static void InitSegments(GOType curveType, qreal segLength, const VPointF *p, quint32 curveId, VContainer *data);

    void UpdateNameField(VToolCurveIntersectAxisNameField field, const QString &value);
    auto GetFieldValue(VToolCurveIntersectAxisNameField field, VToolCurveIntersectAxisNameField excludeField) const
        -> QString;
};

#endif // VTOOLCURVEINTERSECTAXIS_H
