/************************************************************************
 **
 **  @file   vpiecegrainline_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 4, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VPIECEGRAINLINE_P_H
#define VPIECEGRAINLINE_P_H

#include "../vmisc/defglobal.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/floatItemData/floatitemdef.h"

#include <QLineF>
#include <QSharedData>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPieceGrainlinePrivate : public QSharedData
{
public:
    VPieceGrainlinePrivate() = default;
    VPieceGrainlinePrivate(const QLineF &mainLine, GrainlineArrowDirection arrowType);
    VPieceGrainlinePrivate(const VPieceGrainlinePrivate &data) = default;
    ~VPieceGrainlinePrivate() = default;

    static auto MainLine(const QPointF &p1, qreal length, qreal angle) -> QLineF;

    friend auto operator<<(QDataStream &dataStream, const VPieceGrainlinePrivate &data) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VPieceGrainlinePrivate &data) -> QDataStream &;

    QLineF m_mainLine{}; // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    GrainlineArrowDirection m_arrowType{GrainlineArrowDirection::oneWayUp};
    bool m_enabled{false}; // NOLINT(misc-non-private-member-variables-in-classes)
    bool m_visible{true};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPieceGrainlinePrivate) // NOLINT

    static constexpr quint32 streamHeader{0x5C5D5B3B}; // CRC-32Q string "VGrainlineData"
    static constexpr quint16 classVersion{2};
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VPieceGrainlinePrivate::VPieceGrainlinePrivate(const QLineF &mainLine, GrainlineArrowDirection arrowType)
  : m_mainLine(mainLine),
    m_arrowType(arrowType),
    m_enabled(true)
{
}

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
inline auto operator<<(QDataStream &dataStream, const VPieceGrainlinePrivate &data) -> QDataStream &
{
    dataStream << VPieceGrainlinePrivate::streamHeader << VPieceGrainlinePrivate::classVersion;

    // Added in classVersion = 1
    dataStream << data.m_enabled;
    dataStream << data.m_arrowType;
    dataStream << data.m_mainLine;

    // Added in classVersion = 2
    dataStream << data.m_visible;

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator>>(QDataStream &dataStream, VPieceGrainlinePrivate &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VPieceGrainlinePrivate::streamHeader)
    {
        QString const message =
            QCoreApplication::tr("VPieceGrainlinePrivate prefix mismatch error: actualStreamHeader = "
                                 "0x%1 and streamHeader = 0x%2")
                .arg(actualStreamHeader, 8, 0x10, QLatin1Char('0'))
                .arg(VPieceGrainlinePrivate::streamHeader, 8, 0x10, QLatin1Char('0'));
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VPieceGrainlinePrivate::classVersion)
    {
        QString const message =
            QCoreApplication::tr("VPieceGrainlinePrivate compatibility error: actualClassVersion = %1 "
                                 "and classVersion = %2")
                .arg(actualClassVersion)
                .arg(VPieceGrainlinePrivate::classVersion);
        throw VException(message);
    }

    dataStream >> data.m_enabled;
    dataStream >> data.m_arrowType;
    dataStream >> data.m_mainLine;

    if (actualClassVersion >= 2)
    {
        dataStream >> data.m_visible;
    }

    return dataStream;
}

#endif // VPIECEGRAINLINE_P_H
