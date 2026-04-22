/************************************************************************
 **
 **  @file   vtoolpointofintersectionarcs.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 5, 2015
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

#ifndef VTOOLPOINTOFINTERSECTIONARCS_H
#define VTOOLPOINTOFINTERSECTIONARCS_H


#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;
class VArc;
class VSegmentLabel;

struct VToolPointOfIntersectionArcsInitData : VToolSinglePointInitData
{
    quint32 firstArcId{NULL_ID};
    quint32 secondArcId{NULL_ID};
    CrossCirclesPoint pType{CrossCirclesPoint::FirstPoint};
    QString arc1Name1{};
    QString arc1Name2{};
    QString arc2Name1{};
    QString arc2Name2{};
    QString arc1AliasSuffix1{};
    QString arc1AliasSuffix2{};
    QString arc2AliasSuffix1{};
    QString arc2AliasSuffix2{};
    quint32 arc1Segment1Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 arc1Segment2Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc1Segment1Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc1Segment1My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc1Segment2Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc1Segment2My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 arc2Segment1Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 arc2Segment2Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc2Segment1Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc2Segment1My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc2Segment2Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal arc2Segment2My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
};

// Helper enum to identify which field is being updated
enum class VToolPointOfIntersectionArcsNameField : quint8
{
    Arc1Name1,
    Arc1Name2,
    Arc2Name1,
    Arc2Name2,
    Arc1AliasSuffix1,
    Arc1AliasSuffix2,
    Arc2AliasSuffix1,
    Arc2AliasSuffix2
};

// Struct to hold field metadata
struct VToolPointOfIntersectionArcsFieldMetadata
{
    quint32 curveId{NULL_ID};
    bool isName{true}; // true for name fields, false for alias fields
    QString *memberPtr{nullptr};
};

class VToolPointOfIntersectionArcs : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    ~VToolPointOfIntersectionArcs() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolPointOfIntersectionArcs *;
    static auto Create(VToolPointOfIntersectionArcsInitData initData) -> VToolPointOfIntersectionArcs *;
    static auto FindPoint(const VArc *arc1, const VArc *arc2, CrossCirclesPoint pType, QPointF *intersectionPoint)
        -> bool;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::PointOfIntersectionArcs) };

    auto FirstArcName() const -> QString;
    auto SecondArcName() const -> QString;

    auto GetArc1Name1() const -> QString;
    void SetArc1Name1(const QString &name);

    auto GetArc1Name2() const -> QString;
    void SetArc1Name2(const QString &name);

    auto GetArc2Name1() const -> QString;
    void SetArc2Name1(const QString &name);

    auto GetArc2Name2() const -> QString;
    void SetArc2Name2(const QString &name);

    auto GetArc1AliasSuffix1() const -> QString;
    void SetArc1AliasSuffix1(const QString &alias);

    auto GetArc1AliasSuffix2() const -> QString;
    void SetArc1AliasSuffix2(const QString &alias);

    auto GetArc2AliasSuffix1() const -> QString;
    void SetArc2AliasSuffix1(const QString &alias);

    auto GetArc2AliasSuffix2() const -> QString;
    void SetArc2AliasSuffix2(const QString &alias);

    auto GetCrossCirclesPoint() const -> CrossCirclesPoint;
    void SetCrossCirclesPoint(const CrossCirclesPoint &value);

    void ShowVisualization(bool show) override;
    void ChangeSegmentLabelPosition(SegmentLabel segment, const QPointF &pos) override;

    auto IsRemovable() const -> RemoveStatus override;

public slots:
    void Enable() override;
    void EnableToolMove(bool move) override;
    void AllowSegmentHover(bool enabled);
    void ToolSelectionType(const SelectionType &selectionType) override;
    void SetSegmentLabelVisible(bool visible);
    void AllowLabelSelecting(bool enabled) override;
    void SetDetailsMode(bool mode) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void RefreshGeometry() override;

private slots:
    void SegmentChoosed(quint32 id, SceneObject type);
    void Arc1Segment1LabelPositionChanged(const QPointF &pos);
    void Arc1Segment2LabelPositionChanged(const QPointF &pos);
    void Arc2Segment1LabelPositionChanged(const QPointF &pos);
    void Arc2Segment2LabelPositionChanged(const QPointF &pos);

private:
    Q_DISABLE_COPY_MOVE(VToolPointOfIntersectionArcs) // NOLINT

    /** @brief firstArcId id first arc. */
    quint32 firstArcId;

    /** @brief secondArcId id second arc. */
    quint32 secondArcId;

    CrossCirclesPoint crossPoint;

    QString m_arc1Name1{};
    QString m_arc1Name2{};
    QString m_arc2Name1{};
    QString m_arc2Name2{};

    QString m_arc1AliasSuffix1{};
    QString m_arc1AliasSuffix2{};
    QString m_arc2AliasSuffix1{};
    QString m_arc2AliasSuffix2{};

    quint32 m_arc1Segment1Id{NULL_ID};
    quint32 m_arc1Segment2Id{NULL_ID};
    quint32 m_arc2Segment1Id{NULL_ID};
    quint32 m_arc2Segment2Id{NULL_ID};

    qreal m_arc1Segment1Mx{labelMX};
    qreal m_arc1Segment1My{labelMY};
    qreal m_arc1Segment2Mx{labelMX};
    qreal m_arc1Segment2My{labelMY};
    qreal m_arc2Segment1Mx{labelMX};
    qreal m_arc2Segment1My{labelMY};
    qreal m_arc2Segment2Mx{labelMX};
    qreal m_arc2Segment2My{labelMY};

    VSegmentLabel *m_arc1Segment1Label{nullptr};
    VSegmentLabel *m_arc1Segment2Label{nullptr};
    VSegmentLabel *m_arc2Segment1Label{nullptr};
    VSegmentLabel *m_arc2Segment2Label{nullptr};

    int segLableVisRefCount{0};

    struct ToolChanges
    {
        quint32 pointId{NULL_ID};
        QString oldLabel{};
        QString newLabel{};
        QString oldArc1Name1{};
        QString newArc1Name1{};
        QString oldArc1Name2{};
        QString newArc1Name2{};
        QString oldArc2Name1{};
        QString newArc2Name1{};
        QString oldArc2Name2{};
        QString newArc2Name2{};
        QString oldArc1AliasSuffix1{};
        QString newArc1AliasSuffix1{};
        QString oldArc1AliasSuffix2{};
        QString newArc1AliasSuffix2{};
        QString oldArc2AliasSuffix1{};
        QString newArc2AliasSuffix1{};
        QString oldArc2AliasSuffix2{};
        QString newArc2AliasSuffix2{};

        auto HasChanges() const -> bool
        {
            return oldLabel != newLabel || oldArc1Name1 != newArc1Name1 || oldArc1Name2 != newArc1Name2
                   || oldArc2Name1 != newArc2Name1 || oldArc2Name2 != newArc2Name2
                   || oldArc1AliasSuffix1 != newArc1AliasSuffix1 || oldArc1AliasSuffix2 != newArc1AliasSuffix2
                   || oldArc2AliasSuffix1 != newArc2AliasSuffix1 || oldArc2AliasSuffix2 != newArc2AliasSuffix2;
        }

        auto LabelChanged() const -> bool { return oldLabel != newLabel; }
        auto Arc1Name1Changed() const -> bool { return oldArc1Name1 != newArc1Name1; }
        auto Arc1Name2Changed() const -> bool { return oldArc1Name2 != newArc1Name2; }
        auto Arc2Name1Changed() const -> bool { return oldArc2Name1 != newArc2Name1; }
        auto Arc2Name2Changed() const -> bool { return oldArc2Name2 != newArc2Name2; }
        auto Arc1AliasSuffix1Changed() const -> bool { return oldArc1AliasSuffix1 != newArc1AliasSuffix1; }
        auto Arc1AliasSuffix2Changed() const -> bool { return oldArc1AliasSuffix2 != newArc1AliasSuffix2; }
        auto Arc2AliasSuffix1Changed() const -> bool { return oldArc2AliasSuffix1 != newArc2AliasSuffix1; }
        auto Arc2AliasSuffix2Changed() const -> bool { return oldArc2AliasSuffix2 != newArc2AliasSuffix2; }
    };

    explicit VToolPointOfIntersectionArcs(const VToolPointOfIntersectionArcsInitData &initData,
                                          QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;

    void ProcessToolOptions(const QDomElement &oldDomElement,
                            const QDomElement &newDomElement,
                            const ToolChanges &changes);

    auto GetFieldMetadata(VToolPointOfIntersectionArcsNameField field) -> VToolPointOfIntersectionArcsFieldMetadata;
    auto HasConflict(const QString &value, VToolPointOfIntersectionArcsNameField currentField) const -> bool;
    void UpdateNameField(VToolPointOfIntersectionArcsNameField field, const QString &value);
};

#endif // VTOOLPOINTOFINTERSECTIONARCS_H
