/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
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

#ifndef VPIECENODE_H
#define VPIECENODE_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QtGlobal>

#include "../vmisc/def.h"

class VPieceNodeData;
class VContainer;

class VPieceNode
{
public:
    VPieceNode();
    VPieceNode(quint32 id, Tool typeTool, bool reverse = false);
    VPieceNode(const VPieceNode &node);

    ~VPieceNode();

    auto operator=(const VPieceNode &node) -> VPieceNode &;

    VPieceNode(VPieceNode &&node) noexcept;
    auto operator=(VPieceNode &&node) noexcept -> VPieceNode &;

    friend auto operator<<(QDataStream &out, const VPieceNode &p) -> QDataStream &;
    friend auto operator>>(QDataStream &in, VPieceNode &p) -> QDataStream &;

    auto GetId() const -> quint32;
    void SetId(quint32 id);

    auto GetTypeTool() const -> Tool;
    void SetTypeTool(Tool value);

    auto GetReverse() const -> bool;
    void SetReverse(bool reverse);

    auto IsExcluded() const -> bool;
    void SetExcluded(bool exclude);

    auto GetSABefore(const VContainer *data) const -> qreal;
    auto GetSABefore(const VContainer *data, Unit unit) const -> qreal;

    auto GetFormulaSABefore() const -> QString;
    void SetFormulaSABefore(const QString &formula);

    auto GetSAAfter(const VContainer *data) const -> qreal;
    auto GetSAAfter(const VContainer *data, Unit unit) const -> qreal;

    auto GetFormulaSAAfter() const -> QString;
    void SetFormulaSAAfter(const QString &formula);

    auto GetFormulaPassmarkLength() const -> QString;
    void SetFormulaPassmarkLength(const QString &formula);

    auto GetFormulaPassmarkWidth() const -> QString;
    void SetFormulaPassmarkWidth(const QString &formula);

    auto GetFormulaPassmarkAngle() const -> QString;
    void SetFormulaPassmarkAngle(const QString &formula);

    auto GetPassmarkLength(const VContainer *data, Unit unit) const -> qreal;
    auto GetPassmarkWidth(const VContainer *data, Unit unit) const -> qreal;
    auto GetPassmarkAngle(const VContainer *data) const -> qreal;

    auto GetAngleType() const -> PieceNodeAngle;
    void SetAngleType(PieceNodeAngle type);

    auto IsPassmark() const -> bool;
    void SetPassmark(bool passmark);

    auto IsMainPathNode() const -> bool;
    void SetMainPathNode(bool value);

    auto GetPassmarkLineType() const -> PassmarkLineType;
    void SetPassmarkLineType(PassmarkLineType lineType);

    auto GetPassmarkAngleType() const -> PassmarkAngleType;
    void SetPassmarkAngleType(PassmarkAngleType angleType);

    auto IsShowSecondPassmark() const -> bool;
    void SetShowSecondPassmark(bool value);

    auto IsPassmarkClockwiseOpening() const -> bool;
    void SetPassmarkClockwiseOpening(bool value);

    auto IsCheckUniqueness() const -> bool;
    void SetCheckUniqueness(bool value);

    auto IsManualPassmarkLength() const -> bool;
    void SetManualPassmarkLength(bool value);

    auto IsManualPassmarkWidth() const -> bool;
    void SetManualPassmarkWidth(bool value);

    auto IsManualPassmarkAngle() const -> bool;
    void SetManualPassmarkAngle(bool value);

    auto IsTurnPoint() const -> bool;
    void SetTurnPoint(bool value);

    auto IsPassmarkNotMirrored() const -> bool;
    void SetPassmarkNotMirrored(bool value);

private:
    QSharedDataPointer<VPieceNodeData> d;
};

Q_DECLARE_METATYPE(VPieceNode)                  // NOLINT
Q_DECLARE_TYPEINFO(VPieceNode, Q_MOVABLE_TYPE); // NOLINT

#endif // VPIECENODE_H
