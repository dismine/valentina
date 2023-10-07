/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VLAYOUTPIECEPATH_P_H
#define VLAYOUTPIECEPATH_P_H

#include <QDataStream>
#include <QPointF>
#include <QSharedData>
#include <QVector>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include "../vmisc/vdatastreamenum.h"
#endif

#include "../ifc/exception/vexception.h"
#include "vlayoutpoint.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VLayoutPiecePathData : public QSharedData
{
public:
    VLayoutPiecePathData() = default;
    explicit VLayoutPiecePathData(const QVector<VLayoutPoint> &points);
    VLayoutPiecePathData(const VLayoutPiecePathData &path) = default;
    ~VLayoutPiecePathData() = default;

    friend auto operator<<(QDataStream &dataStream, const VLayoutPiecePathData &path) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VLayoutPiecePathData &path) -> QDataStream &;

    /** @brief m_points list of path points. */
    QVector<VLayoutPoint> m_points{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_penStyle path pen style. */
    Qt::PenStyle m_penStyle{Qt::SolidLine}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_cut{false}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VLayoutPiecePathData) // NOLINT

    static constexpr quint32 streamHeader = 0xA53F0225; // CRC-32Q string "VLayoutPiecePathData"
    static constexpr quint16 classVersion = 2;
};

QT_WARNING_POP

// See https://stackoverflow.com/a/46719572/3045403
#if __cplusplus < 201703L                             // C++17
constexpr quint32 VLayoutPiecePathData::streamHeader; // NOLINT(readability-redundant-declaration)
constexpr quint16 VLayoutPiecePathData::classVersion; // NOLINT(readability-redundant-declaration)
#endif

//---------------------------------------------------------------------------------------------------------------------
inline VLayoutPiecePathData::VLayoutPiecePathData(const QVector<VLayoutPoint> &points)
  : m_points(points)
{
}

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPiecePathData &path) -> QDataStream &
{
    dataStream << VLayoutPiecePathData::streamHeader << VLayoutPiecePathData::classVersion;

    // Added in classVersion = 1
    dataStream << path.m_points;
    dataStream << path.m_penStyle;
    dataStream << path.m_cut;

    // Added in classVersion = 2

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPiecePathData &path) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VLayoutPiecePathData::streamHeader)
    {
        QString message = QCoreApplication::tr("VLayoutPiecePathData prefix mismatch error: actualStreamHeader = 0x%1 "
                                               "and streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, QLatin1Char('0'))
                              .arg(VLayoutPiecePathData::streamHeader, 8, 0x10, QLatin1Char('0'));
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VLayoutPiecePathData::classVersion)
    {
        QString message = QCoreApplication::tr("VLayoutPiecePathData compatibility error: actualClassVersion = %1 and "
                                               "classVersion = %2")
                              .arg(actualClassVersion)
                              .arg(VLayoutPiecePathData::classVersion);
        throw VException(message);
    }

    if (actualClassVersion == 1)
    {
        QVector<QPointF> points;
        dataStream >> points;
        CastTo(points, path.m_points);
    }
    else
    {
        dataStream >> path.m_points;
    }
    dataStream >> path.m_penStyle;
    dataStream >> path.m_cut;

    //    if (actualClassVersion >= 2)
    //    {

    //    }

    return dataStream;
}

#endif // VLAYOUTPIECEPATH_P_H
