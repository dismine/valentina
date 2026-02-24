/************************************************************************
 **
 **  @file   vpassmark.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 5, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VPASSMARK_H
#define VPASSMARK_H

#include <QMetaType>
#include <QtGlobal>

#include "../vmisc/typedef.h"
#include "vpiece.h"

class QPainterPath;

enum class PassmarkStatus : qint8
{
    Error = 0,
    Common = 1,
    Rollback = -1
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct VPiecePassmarkData
{
    VSAPoint previousSAPoint{}; // NOLINT(misc-non-private-member-variables-in-classes)
    VSAPoint passmarkSAPoint{}; // NOLINT(misc-non-private-member-variables-in-classes)
    VSAPoint nextSAPoint{};     // NOLINT(misc-non-private-member-variables-in-classes)
    bool trueZeroWidth{false};  // NOLINT(misc-non-private-member-variables-in-classes)
    qreal saWidth{0};           // NOLINT(misc-non-private-member-variables-in-classes)
    QString nodeName{};         // NOLINT(misc-non-private-member-variables-in-classes)
    QString pieceName{};        // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    PassmarkLineType passmarkLineType{PassmarkLineType::OneLine};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    PassmarkAngleType passmarkAngleType{PassmarkAngleType::Straightforward};
    bool isMainPathNode{true};       // NOLINT(misc-non-private-member-variables-in-classes)
    bool isShowSecondPassmark{true}; // NOLINT(misc-non-private-member-variables-in-classes)
    vsizetype passmarkIndex{-1};     // NOLINT(misc-non-private-member-variables-in-classes)
    vidtype id{NULL_ID};             // NOLINT(misc-non-private-member-variables-in-classes)
    qreal globalPassmarkLength{0};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal globalPassmarkWidth{0};    // NOLINT(misc-non-private-member-variables-in-classes)
    bool notMirrored{false};         // NOLINT(misc-non-private-member-variables-in-classes)

    auto toJson() const -> QJsonObject;
};

Q_DECLARE_METATYPE(VPiecePassmarkData)                  // NOLINT
Q_DECLARE_TYPEINFO(VPiecePassmarkData, Q_MOVABLE_TYPE); // NOLINT

QT_WARNING_POP

enum class PassmarkSide : qint8
{
    All = 0,
    Left = 1,
    Right = 2
};

class VPassmark
{
public:
    VPassmark() = default;
    explicit VPassmark(const VPiecePassmarkData &data);

    auto FullPassmark(const VPiece &piece, const VContainer *data) const -> QVector<QLineF>;
    auto SAPassmark(const VPiece &piece, const VContainer *data, PassmarkSide side) const -> QVector<QLineF>;
    auto SAPassmark(const QVector<QPointF> &seamAllowance, const QVector<QPointF> &rotatedSeamAllowance,
                    PassmarkSide side) const -> QVector<QLineF>;
    auto BuiltInSAPassmark(const VPiece &piece, const VContainer *data) const -> QVector<QLineF>;

    auto BuiltInSAPassmarkBaseLine(const VPiece &piece, const QLineF& mirrorLine) const -> QVector<QLineF>;
    auto SAPassmarkBaseLine(const VPiece &piece, const VContainer *data, PassmarkSide side) const -> QVector<QLineF>;
    auto SAPassmarkBaseLine(const QVector<QPointF> &seamAllowance, const QVector<QPointF> &rotatedSeamAllowance,
                            PassmarkSide side) const -> QVector<QLineF>;

    auto SAPassmarkPath(const VPiece &piece, const VContainer *data, PassmarkSide side) const -> QPainterPath;
    auto BuiltInSAPassmarkPath(const VPiece &piece, const VContainer *data) const -> QPainterPath;

    auto IsNull() const -> bool;

    auto Data() const -> VPiecePassmarkData;

    static auto FindIntersection(const QLineF &line, const QVector<QPointF> &seamAllowance) -> QLineF;

    static const qreal passmarkRadiusFactor;

private:
    VPiecePassmarkData m_data{};
    bool m_null{true};

    auto PassmarkIntersection(const QVector<QPointF> &path, QLineF line, qreal width) const -> QLineF;

    auto PassmarkStraightforwardBaseLine(const QPointF &seamPassmarkSAPoint) const -> QVector<QLineF>;
    auto PassmarkBisectorBaseLine(PassmarkStatus seamPassmarkType, const QPointF &seamPassmarkSAPoint,
                                  const QVector<QPointF> &seamAllowance) const -> QVector<QLineF>;
    auto PassmarkIntersectionBaseLine(const QVector<QPointF> &path, PassmarkSide side) const -> QVector<QLineF>;
    auto PassmarkIntersection2BaseLine(const QVector<QPointF> &path, PassmarkSide side) const -> QVector<QLineF>;

    auto CreatePassmarkLines(const QVector<QLineF> &lines, const QVector<QPointF> &seamAllowance,
                             PassmarkSide side) const -> QVector<QLineF>;

    auto CalculatePassmarkLength(const VPiece &piece) const -> qreal;
    auto DetermineEdges(const VPiece &piece, const QLineF &mirrorLine) const -> std::pair<QLineF, QLineF>;
};

#endif // VPASSMARK_H
