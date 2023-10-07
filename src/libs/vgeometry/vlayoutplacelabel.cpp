/************************************************************************
 **
 **  @file   vlayoutplacelabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 10, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "vlayoutplacelabel.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include "../vmisc/vdatastreamenum.h"
#endif

#include "../ifc/exception/vexception.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

// See https://stackoverflow.com/a/46719572/3045403
#if __cplusplus < 201703L                          // C++17
constexpr quint32 VLayoutPlaceLabel::streamHeader; // NOLINT(readability-redundant-declaration)
constexpr quint16 VLayoutPlaceLabel::classVersion; // NOLINT(readability-redundant-declaration)
#endif

//---------------------------------------------------------------------------------------------------------------------
VLayoutPlaceLabel::VLayoutPlaceLabel(const VPlaceLabelItem &item)
  : m_center(item.toQPointF()),
    m_type(item.GetLabelType()),
    m_rotationMatrix(item.RotationMatrix()),
    m_box(item.Box())
{
}

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPlaceLabel &data) -> QDataStream &
{
    dataStream << VLayoutPlaceLabel::streamHeader << VLayoutPlaceLabel::classVersion;

    // Added in classVersion = 1
    dataStream << data.m_center;
    dataStream << data.m_type;
    dataStream << data.m_rotationMatrix;
    dataStream << data.m_box;

    // Added in classVersion = 2

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPlaceLabel &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VLayoutPlaceLabel::streamHeader)
    {
        QString message = QCoreApplication::tr("VLayoutPlaceLabel prefix mismatch error: actualStreamHeader = 0x%1 and "
                                               "streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                              .arg(VLayoutPlaceLabel::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VLayoutPlaceLabel::classVersion)
    {
        QString message = QCoreApplication::tr("VLayoutPlaceLabel compatibility error: actualClassVersion = %1 and "
                                               "classVersion = %2")
                              .arg(actualClassVersion)
                              .arg(VLayoutPlaceLabel::classVersion);
        throw VException(message);
    }

    dataStream >> data.m_center;
    dataStream >> data.m_type;

    if (actualClassVersion == 1)
    {
        QVector<QPolygonF> shape;
        dataStream >> shape; // no longer in use
    }

    dataStream >> data.m_rotationMatrix;
    dataStream >> data.m_box;

    //    if (actualClassVersion >= 2)
    //    {

    //    }

    return dataStream;
}
