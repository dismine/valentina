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

struct VToolPointOfIntersectionCurvesInitData : VToolSinglePointInitData
{
    quint32 firstCurveId{NULL_ID};
    quint32 secondCurveId{NULL_ID};
    VCrossCurvesPoint vCrossPoint{VCrossCurvesPoint::HighestPoint};
    HCrossCurvesPoint hCrossPoint{HCrossCurvesPoint::LeftmostPoint};
    QPair<QString, QString> curve1Segments{};
    QPair<QString, QString> curve2Segments{};
    QString curve1Name1{};
    QString curve1Name2{};
    QString curve2Name1{};
    QString curve2Name2{};
    QString curve1AliasSuffix1{};
    QString curve1AliasSuffix2{};
    QString curve2AliasSuffix1{};
    QString curve2AliasSuffix2{};
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
protected slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

protected:
    void SaveDialog(QDomElement &domElement) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void ReadToolAttributes(const QDomElement &domElement) override;
    void SetVisualization() override;
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

    void SetCurve1Segments(const QPair<QString, QString> &segments);
    void SetCurve2Segments(const QPair<QString, QString> &segments);
private:
    Q_DISABLE_COPY_MOVE(VToolPointOfIntersectionCurves) // NOLINT

    quint32 firstCurveId;
    quint32 secondCurveId;

    VCrossCurvesPoint vCrossPoint;
    HCrossCurvesPoint hCrossPoint;

    QPair<QString, QString> m_curve1Segments{};
    QPair<QString, QString> m_curve2Segments{};

    QString m_curve1Name1{};
    QString m_curve1Name2{};
    QString m_curve2Name1{};
    QString m_curve2Name2{};

    QString m_curve1AliasSuffix1{};
    QString m_curve1AliasSuffix2{};
    QString m_curve2AliasSuffix1{};
    QString m_curve2AliasSuffix2{};

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
};

#endif // VTOOLPOINTOFINTERSECTIONCURVES_H
