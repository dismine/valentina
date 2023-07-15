/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#ifndef VPIECEPATH_H
#define VPIECEPATH_H

#include <QSharedDataPointer>
#include <QtGlobal>

#include "../vgeometry/vabstractcurve.h"
#include "../vmisc/def.h"

class VPiecePathData;
class VSAPoint;
class VContainer;
class QPainterPath;
class VPointF;
class VPieceNode;
class VInternalVariable;
class VLayoutPoint;

class VPiecePath
{
public:
    VPiecePath();
    explicit VPiecePath(PiecePathType type);
    VPiecePath(const VPiecePath &path);

    ~VPiecePath();

    auto operator=(const VPiecePath &path) -> VPiecePath &;
#ifdef Q_COMPILER_RVALUE_REFS
    VPiecePath(VPiecePath &&path) noexcept;
    auto operator=(VPiecePath &&path) noexcept -> VPiecePath &;
#endif

    void Append(const VPieceNode &node);
    void Clear();
    auto CountNodes() const -> vsizetype;

    auto operator[](vsizetype indx) -> VPieceNode &;
    auto at(vsizetype indx) const -> const VPieceNode &;

    auto GetNodes() const -> QVector<VPieceNode>;
    void SetNodes(const QVector<VPieceNode> &nodes);

    auto GetType() const -> PiecePathType;
    void SetType(PiecePathType type);

    auto GetName() const -> QString;
    void SetName(const QString &name);

    auto GetPenType() const -> Qt::PenStyle;
    void SetPenType(const Qt::PenStyle &type);

    auto IsCutPath() const -> bool;
    void SetCutPath(bool cut);

    auto GetVisibilityTrigger() const -> QString;
    void SetVisibilityTrigger(const QString &formula);

    void SetFirstToCuttingContour(bool value);
    auto IsFirstToCuttingContour() const -> bool;

    void SetLastToCuttingContour(bool value);
    auto IsLastToCuttingContour() const -> bool;

    auto PathPoints(const VContainer *data, const QVector<QPointF> &cuttingPath = QVector<QPointF>()) const
        -> QVector<VLayoutPoint>;
    auto PathNodePoints(const VContainer *data, bool showExcluded = true) const -> QVector<VPointF>;
    auto PathCurvePoints(const VContainer *data) const -> QVector<QVector<QPointF>>;
    auto SeamAllowancePoints(const VContainer *data, qreal width, bool reverse) const -> QVector<VSAPoint>;

    auto PainterPath(const VContainer *data, const QVector<QPointF> &cuttingPath) const -> QPainterPath;
    auto CurvesPainterPath(const VContainer *data) const -> QVector<QPainterPath>;

    auto Dependencies() const -> QList<quint32>;
    auto MissingNodes(const VPiecePath &path) const -> QVector<quint32>;

    auto NodeName(int nodeIndex, const VContainer *data) const -> QString;

    auto indexOfNode(quint32 id) const -> int;
    void NodeOnEdge(quint32 index, VPieceNode &p1, VPieceNode &p2) const;
    auto Contains(quint32 id) const -> bool;
    auto OnEdge(quint32 p1, quint32 p2) const -> bool;
    auto Edge(quint32 p1, quint32 p2) const -> vsizetype;

    auto ListNodePoint() const -> QVector<VPieceNode>;

    auto RemoveEdge(quint32 index) const -> VPiecePath;

    auto StartSegment(const VContainer *data, int i) const -> VSAPoint;
    auto EndSegment(const VContainer *data, int i) const -> VSAPoint;

    auto NodePreviousPoint(const VContainer *data, int i) const -> QPointF;
    auto NodeNextPoint(const VContainer *data, int i) const -> QPointF;

    auto IsVisible(const QHash<QString, QSharedPointer<VInternalVariable>> *vars) const -> bool;

    static auto indexOfNode(const QVector<VPieceNode> &nodes, quint32 id) -> int;

    static auto FindInLoopNotExcludedUp(vsizetype start, const QVector<VPieceNode> &nodes) -> vsizetype;
    static auto FindInLoopNotExcludedDown(vsizetype start, const QVector<VPieceNode> &nodes) -> vsizetype;

    static auto StartSegment(const VContainer *data, const QVector<VPieceNode> &nodes, vsizetype i) -> VSAPoint;
    static auto EndSegment(const VContainer *data, const QVector<VPieceNode> &nodes, vsizetype i) -> VSAPoint;

    static auto PreparePointEkv(const VPieceNode &node, const VContainer *data) -> VSAPoint;

    static auto CurveSeamAllowanceSegment(const VContainer *data, const QVector<VPieceNode> &nodes,
                                          const QSharedPointer<VAbstractCurve> &curve, vsizetype i, bool reverse,
                                          qreal width, const QString &piece = QString()) -> QVector<VSAPoint>;

    static auto NodeName(const QVector<VPieceNode> &nodes, vsizetype nodeIndex, const VContainer *data) -> QString;

    static auto NodesToPoints(const VContainer *data, const QVector<VPieceNode> &nodes,
                              const QString &piece = QString()) -> QVector<VLayoutPoint>;

private:
    QSharedDataPointer<VPiecePathData> d;
};

Q_DECLARE_TYPEINFO(VPiecePath, Q_MOVABLE_TYPE); // NOLINT
Q_DECLARE_METATYPE(VPiecePath)                  // NOLINT

#endif // VPIECEPATH_H
