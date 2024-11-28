/************************************************************************
 **
 **  @file   vgeometrydef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 7, 2019
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

#include "vgeometrydef.h"

#include "../ifc/exception/vexception.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const quint32 VLayoutPassmark::streamHeader = 0x943E2759; // CRC-32Q string "VLayoutPassmark"
const quint16 VLayoutPassmark::classVersion = 3;

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPassmark &data) -> QDataStream &
{
    dataStream << VLayoutPassmark::streamHeader << VLayoutPassmark::classVersion;

    dataStream << data.lines << data.type << data.baseLine << data.isBuiltIn << data.isClockwiseOpening << data.label;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPassmark &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VLayoutPassmark::streamHeader)
    {
        QString const message =
            QCoreApplication::tr("VLayoutPassmark prefix mismatch error: actualStreamHeader = 0x%1 and "
                                 "streamHeader = 0x%2")
                .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                .arg(VLayoutPassmark::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VLayoutPassmark::classVersion)
    {
        QString const message = QCoreApplication::tr("VLayoutPassmark compatibility error: actualClassVersion = %1 and "
                                                     "classVersion = %2")
                                    .arg(actualClassVersion)
                                    .arg(VLayoutPassmark::classVersion);
        throw VException(message);
    }

    dataStream >> data.lines >> data.type >> data.baseLine >> data.isBuiltIn;

    if (actualClassVersion >= 2)
    {
        dataStream >> data.isClockwiseOpening;
    }

    if (actualClassVersion >= 3)
    {
        dataStream >> data.label;
    }

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto SingleParallelPoint(const QPointF &p1, const QPointF &p2, qreal angle, qreal width) -> QPointF
{
    QLineF pLine(p1, p2);
    pLine.setAngle(pLine.angle() + angle);
    pLine.setLength(width);
    return pLine.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto SimpleParallelLine(const QPointF &p1, const QPointF &p2, qreal width) -> QLineF
{
    return {SingleParallelPoint(p1, p2, 90, width), SingleParallelPoint(p2, p1, -90, width)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPassmark::toJson() const -> QJsonObject
{
    QJsonObject object;
    object["type"_L1] = "VLayoutPassmark";

    // Serialize lines if not empty
    if (!lines.isEmpty())
    {
        QJsonArray linesArray;
        for (const auto &line : lines)
        {
            QJsonObject lineObject;
            lineObject["x1"_L1] = line.x1();
            lineObject["y1"_L1] = line.y1();
            lineObject["x2"_L1] = line.x2();
            lineObject["y2"_L1] = line.y2();
            linesArray.append(lineObject);
        }
        object["lines"_L1] = linesArray;
    }

    // Serialize type if not the default value
    if (type != PassmarkLineType::OneLine)
    {
        object["passmarkType"_L1] = static_cast<int>(type);
    }

    // Serialize baseLine if it's not default
    if (baseLine != QLineF{})
    {
        object["baseLine"_L1] = QJsonObject{
            {"x1"_L1, baseLine.x1()},
            {"y1"_L1, baseLine.y1()},
            {"x2"_L1, baseLine.x2()},
            {"y2"_L1, baseLine.y2()},
        };
    }

    // Serialize isBuiltIn if not false
    if (isBuiltIn)
    {
        object["isBuiltIn"_L1] = isBuiltIn;
    }

    // Serialize isClockwiseOpening if not false
    if (isClockwiseOpening)
    {
        object["isClockwiseOpening"_L1] = isClockwiseOpening;
    }

    // Serialize label if not empty
    if (!label.isEmpty())
    {
        object["label"_L1] = label;
    }

    return object;
}
