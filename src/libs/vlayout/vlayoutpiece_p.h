/************************************************************************
 **
 **  @file   vlayoutdetail_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VLAYOUTDETAIL_P_H
#define VLAYOUTDETAIL_P_H

#include <QPointF>
#include <QSharedData>
#include <QTransform>
#include <QVector>

#include "../vpatterndb/floatItemData/floatitemdef.h"
#include "../vwidgets/vpiecegrainline.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include "../vmisc/vdatastreamenum.h"
#endif

#include "../ifc/exception/vexception.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "vlayoutpiecepath.h"
#include "vlayoutpoint.h"
#include "vtextmanager.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VLayoutPieceData : public QSharedData
{
public:
    VLayoutPieceData() = default;
    VLayoutPieceData(const VLayoutPieceData &detail) = default;
    ~VLayoutPieceData() = default;

    friend auto operator<<(QDataStream &dataStream, const VLayoutPieceData &piece) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VLayoutPieceData &piece) -> QDataStream &;

    /** @brief contour list of contour points. */
    QVector<VLayoutPoint> m_contour{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief seamAllowance list of seam allowance points. */
    QVector<VLayoutPoint> m_seamAllowance{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief layoutAllowance list of layout allowance points. */
    QVector<QPointF> m_layoutAllowance{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief passmarks list of passmakrs. */
    QVector<VLayoutPassmark> m_passmarks{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_internalPaths list of internal paths. */
    QVector<VLayoutPiecePath> m_internalPaths{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief matrix transformation matrix*/
    QTransform m_matrix{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief layoutWidth value layout allowance width in pixels. */
    qreal m_layoutWidth{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_mirror{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief detailLabel detail label rectangle */
    QVector<QPointF> m_detailLabel{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief patternInfo pattern info rectangle */
    QVector<QPointF> m_patternInfo{}; // NOLINT(misc-non-private-member-variables-in-classes)

    VPieceGrainline m_grainline{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_tmDetail text manager for laying out detail info */
    VTextManager m_tmDetail{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_tmPattern text manager for laying out pattern info */
    VTextManager m_tmPattern{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_placeLabels list of place labels. */
    QVector<VLayoutPlaceLabel> m_placeLabels{}; // NOLINT(misc-non-private-member-variables-in-classes)

    qint64 m_square{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    quint16 m_quantity{1}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_id keep id of original piece. */
    vidtype m_id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_gradationId{}; // NOLINT(misc-non-private-member-variables-in-classes)

    qreal m_xScale{1.0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_yScale{1.0}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VLayoutPieceData) // NOLINT

    static constexpr quint32 streamHeader{0x80D7D009}; // CRC-32Q string "VLayoutPieceData"
    static constexpr quint16 classVersion{5};
};

QT_WARNING_POP

// See https://stackoverflow.com/a/46719572/3045403
#if __cplusplus < 201703L                         // C++17
constexpr quint32 VLayoutPieceData::streamHeader; // NOLINT(readability-redundant-declaration)
constexpr quint16 VLayoutPieceData::classVersion; // NOLINT(readability-redundant-declaration)
#endif

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
inline auto operator<<(QDataStream &dataStream, const VLayoutPieceData &piece) -> QDataStream &
{
    dataStream << VLayoutPieceData::streamHeader << VLayoutPieceData::classVersion;

    dataStream << piece.m_contour;
    dataStream << piece.m_seamAllowance;
    dataStream << piece.m_layoutAllowance;
    dataStream << piece.m_passmarks;
    dataStream << piece.m_internalPaths;
    dataStream << piece.m_matrix;
    dataStream << piece.m_layoutWidth;
    dataStream << piece.m_mirror;
    dataStream << piece.m_detailLabel;
    dataStream << piece.m_patternInfo;
    dataStream << piece.m_placeLabels;
    dataStream << piece.m_square;
    dataStream << piece.m_quantity;
    dataStream << piece.m_id;
    dataStream << piece.m_tmDetail;
    dataStream << piece.m_tmPattern;
    dataStream << piece.m_gradationId;
    dataStream << piece.m_xScale;
    dataStream << piece.m_yScale;
    dataStream << piece.m_grainline;

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator>>(QDataStream &dataStream, VLayoutPieceData &piece) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VLayoutPieceData::streamHeader)
    {
        QString message = QCoreApplication::tr("VLayoutPieceData prefix mismatch error: actualStreamHeader = 0x%1 and "
                                               "streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, QLatin1Char('0'))
                              .arg(VLayoutPieceData::streamHeader, 8, 0x10, QLatin1Char('0'));
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VLayoutPieceData::classVersion)
    {
        QString message = QCoreApplication::tr("VLayoutPieceData compatibility error: actualClassVersion = %1 and "
                                               "classVersion = %2")
                              .arg(actualClassVersion)
                              .arg(VLayoutPieceData::classVersion);
        throw VException(message);
    }

    if (actualClassVersion < 4)
    {
        auto ReadPoints = [&dataStream]()
        {
            QVector<QPointF> points;
            dataStream >> points;
            QVector<VLayoutPoint> casted;
            CastTo(points, casted);
            return casted;
        };

        piece.m_contour = ReadPoints();
        piece.m_seamAllowance = ReadPoints();
    }
    else
    {
        dataStream >> piece.m_contour;
        dataStream >> piece.m_seamAllowance;
    }

    dataStream >> piece.m_layoutAllowance;
    dataStream >> piece.m_passmarks;
    dataStream >> piece.m_internalPaths;
    dataStream >> piece.m_matrix;
    dataStream >> piece.m_layoutWidth;
    dataStream >> piece.m_mirror;
    dataStream >> piece.m_detailLabel;
    dataStream >> piece.m_patternInfo;

    QVector<QPointF> shape;
    GrainlineArrowDirection arrowType = GrainlineArrowDirection::oneWayUp;
    bool grainlineEnabled = false;

    if (actualClassVersion < 5)
    {
        dataStream >> shape;

        dataStream >> arrowType;

        qreal grainlineAngle;
        dataStream >> grainlineAngle;

        dataStream >> grainlineEnabled;
    }

    dataStream >> piece.m_placeLabels;
    dataStream >> piece.m_square;

    if (actualClassVersion >= 2)
    {
        dataStream >> piece.m_quantity;
        dataStream >> piece.m_id;
    }

    if (actualClassVersion >= 3)
    {
        dataStream >> piece.m_tmDetail;
        dataStream >> piece.m_tmPattern;
        dataStream >> piece.m_gradationId;
        dataStream >> piece.m_xScale;
        dataStream >> piece.m_yScale;
    }

    if (actualClassVersion >= 5)
    {
        dataStream >> piece.m_grainline;
    }
    else
    {
        if (shape.size() >= 2)
        {
            piece.m_grainline = VPieceGrainline(QLineF(shape.constFirst(), shape.constLast()), arrowType);
            piece.m_grainline.SetEnabled(false);
        }
        else
        {
            piece.m_grainline = VPieceGrainline();
            piece.m_grainline.SetArrowType(arrowType);
            piece.m_grainline.SetEnabled(grainlineEnabled);
        }
    }

    return dataStream;
}

#endif // VLAYOUTDETAIL_P_H
