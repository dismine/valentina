/************************************************************************
 **
 **  @file   vtoolpointofintersectioncurves.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2016
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

#ifndef VTOOLPOINTOFINTERSECTIONCURVES_H
#define VTOOLPOINTOFINTERSECTIONCURVES_H


#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;
class VSegmentLabel;

struct VToolPointOfIntersectionCurvesInitData : VToolSinglePointInitData
{
    quint32 firstCurveId{NULL_ID};
    quint32 secondCurveId{NULL_ID};
    VCrossCurvesPoint vCrossPoint{VCrossCurvesPoint::HighestPoint};
    HCrossCurvesPoint hCrossPoint{HCrossCurvesPoint::LeftmostPoint};
    QString curve1Name1{};
    QString curve1Name2{};
    QString curve2Name1{};
    QString curve2Name2{};
    QString curve1AliasSuffix1{};
    QString curve1AliasSuffix2{};
    QString curve2AliasSuffix1{};
    QString curve2AliasSuffix2{};
    quint32 curve1Segment1Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 curve1Segment2Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve1Segment1Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve1Segment1My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve1Segment2Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve1Segment2My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 curve2Segment1Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 curve2Segment2Id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve2Segment1Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve2Segment1My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve2Segment2Mx{labelMX};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal curve2Segment2My{labelMY};   // NOLINT(misc-non-private-member-variables-in-classes)
};

// Helper enum to identify which field is being updated
enum class VToolPointOfIntersectionCurvesNameField : quint8
{
    Curve1Name1,
    Curve1Name2,
    Curve2Name1,
    Curve2Name2,
    Curve1AliasSuffix1,
    Curve1AliasSuffix2,
    Curve2AliasSuffix1,
    Curve2AliasSuffix2
};

// Struct to hold field metadata
struct VToolPointOfIntersectionCurvesFieldMetadata
{
    quint32 curveId{NULL_ID};
    bool isName{true}; // true for name fields, false for alias fields
    QString *memberPtr{nullptr};
};

class VToolPointOfIntersectionCurves : public VToolSinglePoint
{
    Q_OBJECT // NOLINT
public:
    ~VToolPointOfIntersectionCurves() override = default;
    void SetDialog() override;
    static auto Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolPointOfIntersectionCurves *;
    static auto Create(VToolPointOfIntersectionCurvesInitData initData) -> VToolPointOfIntersectionCurves *;
    static auto FindPoint(const QVector<QPointF> &curve1Points, const QVector<QPointF> &curve2Points,
                          VCrossCurvesPoint vCrossPoint, HCrossCurvesPoint hCrossPoint, QPointF *intersectionPoint)
        -> bool;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum { Type = UserType + static_cast<int>(Tool::PointOfIntersectionCurves) };

    auto FirstCurveName() const -> QString;
    auto SecondCurveName() const -> QString;

    auto GetCurve1Name1() const -> QString;
    void SetCurve1Name1(const QString &name);

    auto GetCurve1Name2() const -> QString;
    void SetCurve1Name2(const QString &name);

    auto GetCurve2Name1() const -> QString;
    void SetCurve2Name1(const QString &name);

    auto GetCurve2Name2() const -> QString;
    void SetCurve2Name2(const QString &name);

    auto GetCurve1AliasSuffix1() const -> QString;
    void SetCurve1AliasSuffix1(const QString &alias);

    auto GetCurve1AliasSuffix2() const -> QString;
    void SetCurve1AliasSuffix2(const QString &alias);

    auto GetCurve2AliasSuffix1() const -> QString;
    void SetCurve2AliasSuffix1(const QString &alias);

    auto GetCurve2AliasSuffix2() const -> QString;
    void SetCurve2AliasSuffix2(const QString &alias);

    auto GetVCrossPoint() const -> VCrossCurvesPoint;
    void SetVCrossPoint(VCrossCurvesPoint value);

    auto GetHCrossPoint() const -> HCrossCurvesPoint;
    void SetHCrossPoint(HCrossCurvesPoint value);

    void ShowVisualization(bool show) override;
    void ChangeSegmentLabelPosition(SegmentLabel segment, const QPointF &pos) override;

    auto IsRemovable() const -> RemoveStatus override;

public slots:
    void Enable() override;
    void EnableToolMove(bool move) override;
    void AllowArcSegmentHover(bool enabled);
    void AllowElArcSegmentHover(bool enabled);
    void AllowSplineSegmentHover(bool enabled);
    void AllowSplinePathSegmentHover(bool enabled);
    void ToolSelectionType(const SelectionType &selectionType) override;
    void SetArcSegmentLabelVisible(bool visible);
    void SetElArcSegmentLabelVisible(bool visible);
    void SetSplineSegmentLabelVisible(bool visible);
    void SetSplinePathSegmentLabelVisible(bool visible);
    void AllowLabelSelecting(bool enabled) override;
    void SetDetailsMode(bool mode) override;

protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void RefreshGeometry() override;

private slots:
    void SegmentChoosed(quint32 id, SceneObject type);
    void Curve1Segment1LabelPositionChanged(const QPointF &pos);
    void Curve1Segment2LabelPositionChanged(const QPointF &pos);
    void Curve2Segment1LabelPositionChanged(const QPointF &pos);
    void Curve2Segment2LabelPositionChanged(const QPointF &pos);

private:
    Q_DISABLE_COPY_MOVE(VToolPointOfIntersectionCurves) // NOLINT

    quint32 firstCurveId;
    quint32 secondCurveId;

    VCrossCurvesPoint vCrossPoint;
    HCrossCurvesPoint hCrossPoint;

    QString m_curve1Name1{};
    QString m_curve1Name2{};
    QString m_curve2Name1{};
    QString m_curve2Name2{};

    QString m_curve1AliasSuffix1{};
    QString m_curve1AliasSuffix2{};
    QString m_curve2AliasSuffix1{};
    QString m_curve2AliasSuffix2{};

    quint32 m_curve1Segment1Id{NULL_ID};
    quint32 m_curve1Segment2Id{NULL_ID};
    quint32 m_curve2Segment1Id{NULL_ID};
    quint32 m_curve2Segment2Id{NULL_ID};

    qreal m_curve1Segment1Mx{labelMX};
    qreal m_curve1Segment1My{labelMY};
    qreal m_curve1Segment2Mx{labelMX};
    qreal m_curve1Segment2My{labelMY};
    qreal m_curve2Segment1Mx{labelMX};
    qreal m_curve2Segment1My{labelMY};
    qreal m_curve2Segment2Mx{labelMX};
    qreal m_curve2Segment2My{labelMY};

    VSegmentLabel *m_curve1Segment1Label{nullptr};
    VSegmentLabel *m_curve1Segment2Label{nullptr};
    VSegmentLabel *m_curve2Segment1Label{nullptr};
    VSegmentLabel *m_curve2Segment2Label{nullptr};

    int segLableVisRefCount{0};

    struct ToolChanges
    {
        QString oldLabel{};
        QString newLabel{};
        QString oldCurve1Name1{};
        QString newCurve1Name1{};
        QString oldCurve1Name2{};
        QString newCurve1Name2{};
        QString oldCurve2Name1{};
        QString newCurve2Name1{};
        QString oldCurve2Name2{};
        QString newCurve2Name2{};
        QString oldCurve1AliasSuffix1{};
        QString newCurve1AliasSuffix1{};
        QString oldCurve1AliasSuffix2{};
        QString newCurve1AliasSuffix2{};
        QString oldCurve2AliasSuffix1{};
        QString newCurve2AliasSuffix1{};
        QString oldCurve2AliasSuffix2{};
        QString newCurve2AliasSuffix2{};

        auto HasChanges() const -> bool
        {
            return oldLabel != newLabel || oldCurve1Name1 != newCurve1Name1 || oldCurve1Name2 != newCurve1Name2
                   || oldCurve2Name1 != newCurve2Name1 || oldCurve2Name2 != newCurve2Name2
                   || oldCurve1AliasSuffix1 != newCurve1AliasSuffix1 || oldCurve1AliasSuffix2 != newCurve1AliasSuffix2
                   || oldCurve2AliasSuffix1 != newCurve2AliasSuffix1 || oldCurve2AliasSuffix2 != newCurve2AliasSuffix2;
        }

        auto LabelChanged() const -> bool { return oldLabel != newLabel; }
        auto Curve1Name1Changed() const -> bool { return oldCurve1Name1 != newCurve1Name1; }
        auto Curve1Name2Changed() const -> bool { return oldCurve1Name2 != newCurve1Name2; }
        auto Curve2Name1Changed() const -> bool { return oldCurve2Name1 != newCurve2Name1; }
        auto Curve2Name2Changed() const -> bool { return oldCurve2Name2 != newCurve2Name2; }
        auto Curve1AliasSuffix1Changed() const -> bool { return oldCurve1AliasSuffix1 != newCurve1AliasSuffix1; }
        auto Curve1AliasSuffix2Changed() const -> bool { return oldCurve1AliasSuffix2 != newCurve1AliasSuffix2; }
        auto Curve2AliasSuffix1Changed() const -> bool { return oldCurve2AliasSuffix1 != newCurve2AliasSuffix1; }
        auto Curve2AliasSuffix2Changed() const -> bool { return oldCurve2AliasSuffix2 != newCurve2AliasSuffix2; }
    };

    explicit VToolPointOfIntersectionCurves(const VToolPointOfIntersectionCurvesInitData &initData,
                                            QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;

    void ProcessToolOptions(const QDomElement &oldDomElement,
                            const QDomElement &newDomElement,
                            const ToolChanges &changes);

    auto GetFieldMetadata(VToolPointOfIntersectionCurvesNameField field) -> VToolPointOfIntersectionCurvesFieldMetadata;
    auto HasConflict(const QString &value, VToolPointOfIntersectionCurvesNameField currentField) const -> bool;
    void UpdateNameField(VToolPointOfIntersectionCurvesNameField field, const QString &value);

    void SetSegmentLabelVisible(quint32 curveId,
                                bool visible,
                                int &refCount,
                                VSegmentLabel *seg1Label,
                                VSegmentLabel *seg2Label,
                                std::initializer_list<GOType> types) const;
    void SetSegmentLabelHover(quint32 curveId,
                              bool enabled,
                              VSegmentLabel *seg1Label,
                              VSegmentLabel *seg2Label,
                              std::initializer_list<GOType> types) const;
};

#endif // VTOOLPOINTOFINTERSECTIONCURVES_H
