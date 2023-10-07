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

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include "../vmisc/vdatastreamenum.h"
#endif

#include "../ifc/exception/vexception.h"

#include <QCoreApplication>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const quint32 VLayoutPassmark::streamHeader = 0x943E2759; // CRC-32Q string "VLayoutPassmark"
const quint16 VLayoutPassmark::classVersion = 2;

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPassmark &data) -> QDataStream &
{
    dataStream << VLayoutPassmark::streamHeader << VLayoutPassmark::classVersion;

    dataStream << data.lines << data.type << data.baseLine << data.isBuiltIn << data.isClockwiseOpening;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPassmark &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VLayoutPassmark::streamHeader)
    {
        QString message = QCoreApplication::tr("VLayoutPassmark prefix mismatch error: actualStreamHeader = 0x%1 and "
                                               "streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                              .arg(VLayoutPassmark::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VLayoutPassmark::classVersion)
    {
        QString message = QCoreApplication::tr("VLayoutPassmark compatibility error: actualClassVersion = %1 and "
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

    return dataStream;
}
