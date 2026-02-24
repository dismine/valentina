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

#ifndef VPIECENODE_P_H
#define VPIECENODE_P_H

#include <QCoreApplication>
#include <QDataStream>
#include <QSharedData>

#include "../ifc/ifcdef.h"
#include "../vmisc/exception/vexception.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPieceNodeData : public QSharedData
{
public:
    VPieceNodeData() = default;
    VPieceNodeData(quint32 id, Tool typeTool, bool reverse);
    VPieceNodeData(const VPieceNodeData &node) = default;
    ~VPieceNodeData() = default;

    friend auto operator<<(QDataStream &out, const VPieceNodeData &p) -> QDataStream &;
    friend auto operator>>(QDataStream &in, VPieceNodeData &p) -> QDataStream &;

    /** @brief id object id. */
    quint32 m_id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief typeTool type of tool */
    Tool m_typeTool{Tool::NodePoint}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief reverse true if need reverse points list for node. */
    bool m_reverse{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_excluded true if item excluded from main path. Excluded item is not visible and also will not has
     * affect on main path. Also include to exist path items automatically setted excluded. */
    bool m_excluded{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_isPassmark has sense only for points. If true to seam allowance should be added a passmark. */
    bool m_isPassmark{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_isMainPathNode need fin know if allowed for this passmakr to be double. */
    bool m_isMainPathNode{true}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_formulaWidthBefore{currentSeamAllowance}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_formulaWidthAfter{currentSeamAllowance};  // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_formulaPassmarkLength{};                  // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_formulaPassmarkWidth{};                   // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_formulaPassmarkAngle{};                   // NOLINT(misc-non-private-member-variables-in-classes)

    PieceNodeAngle m_angleType{PieceNodeAngle::ByLength}; // NOLINT(misc-non-private-member-variables-in-classes)

    PassmarkLineType m_passmarkLineType{// NOLINT(misc-non-private-member-variables-in-classes)
                                        PassmarkLineType::OneLine};
    PassmarkAngleType m_passmarkAngleType{// NOLINT(misc-non-private-member-variables-in-classes)
                                          PassmarkAngleType::Straightforward};

    bool m_isShowSecondPassmark{true};        // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_isPassmarkClockwiseOpening{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_checkUniqueness need in cases where different points have the same coordinates, become one point.
     * By default the check enabled. Disable it only if in a path cannot be used just one point. For example if
     * gradation change a piece shape and the seond point should be remaind.*/
    bool m_checkUniqueness{true}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_manualPassmarkLength{false}; // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_manualPassmarkWidth{false};  // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_manualPassmarkAngle{false};  // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_turnPoint{true}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_passmarkNotMirrored{false}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPieceNodeData) // NOLINT

    static constexpr quint32 streamHeader = 0x2198CBC8; // CRC-32Q string "VPieceNodeData"
    static constexpr quint16 classVersion = 3;
};

//---------------------------------------------------------------------------------------------------------------------
inline VPieceNodeData::VPieceNodeData(quint32 id, Tool typeTool, bool reverse)
  : m_id(id),
    m_typeTool(typeTool),
    m_reverse(reverse)
{
    if (m_typeTool == Tool::NodePoint)
    {
        m_reverse = false;
    }
}

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
inline auto operator<<(QDataStream &out, const VPieceNodeData &p) -> QDataStream &
{
    out << VPieceNodeData::streamHeader << VPieceNodeData::classVersion;

    out << p.m_id << p.m_typeTool << p.m_reverse << p.m_excluded << p.m_isPassmark << p.m_formulaWidthBefore
        << p.m_formulaWidthAfter << p.m_formulaPassmarkLength << p.m_angleType << p.m_passmarkLineType
        << p.m_passmarkAngleType << p.m_isShowSecondPassmark << p.m_checkUniqueness << p.m_manualPassmarkLength
        << p.m_turnPoint << p.m_formulaPassmarkWidth << p.m_formulaPassmarkAngle << p.m_manualPassmarkWidth
        << p.m_manualPassmarkAngle << p.m_isPassmarkClockwiseOpening;

    return out;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator>>(QDataStream &in, VPieceNodeData &p) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    in >> actualStreamHeader;

    if (actualStreamHeader != VPieceNodeData::streamHeader)
    {
        QString const message = QCoreApplication::tr("VPieceNodeData prefix mismatch error: actualStreamHeader = 0x%1 "
                                                     "and streamHeader = 0x%2")
                                    .arg(actualStreamHeader, 8, 0x10, QChar('0'))
                                    .arg(VPieceNodeData::streamHeader, 8, 0x10, QChar('0'));
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    in >> actualClassVersion;

    if (actualClassVersion > VPieceNodeData::classVersion)
    {
        QString const message = QCoreApplication::tr("VPieceNodeData compatibility error: actualClassVersion = %1 and "
                                                     "classVersion = %2")
                                    .arg(actualClassVersion)
                                    .arg(VPieceNodeData::classVersion);
        throw VException(message);
    }

    in >> p.m_id >> p.m_typeTool >> p.m_reverse >> p.m_excluded >> p.m_isPassmark >> p.m_formulaWidthBefore >>
        p.m_formulaWidthAfter >> p.m_formulaPassmarkLength >> p.m_angleType >> p.m_passmarkLineType >>
        p.m_passmarkAngleType >> p.m_isShowSecondPassmark >> p.m_checkUniqueness >> p.m_manualPassmarkLength;

    if (actualClassVersion >= 2)
    {
        in >> p.m_turnPoint;
    }

    if (actualClassVersion >= 3)
    {
        in >> p.m_formulaPassmarkWidth >> p.m_formulaPassmarkAngle >> p.m_manualPassmarkWidth >>
            p.m_manualPassmarkAngle >> p.m_isPassmarkClockwiseOpening;
    }

    return in;
}

QT_WARNING_POP

#endif // VPIECENODE_P_H
