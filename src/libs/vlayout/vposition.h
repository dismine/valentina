/************************************************************************
 **
 **  @file   vposition.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 1, 2015
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

#ifndef VPOSITION_H
#define VPOSITION_H

#include <QRunnable>
#include <QVector>
#include <QtGlobal>
#include <atomic>

#ifdef LAYOUT_DEBUG
#include <QMutex>
#endif

#include "vbestsquare.h"
#include "vcontour.h"
#include "vlayoutdef.h"
#include "vlayoutpiece.h"

struct VPositionData
{
    VContour gContour{};
    VLayoutPiece detail{};
    int i{-1};
    int j{-1};
    bool rotate{false};
    int rotationNumber{0};
    bool followGrainline{false};
    QVector<VCachedPositions> positionsCache{};
    bool isOriginPaperOrientationPortrait{true};
#ifdef LAYOUT_DEBUG
    QVector<VLayoutPiece> details{};
    QMutex *mutex{nullptr};
#endif
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

class VPosition
{
public:
    VPosition() = default;
    VPosition(const VPositionData &data, std::atomic_bool *stop, bool saveLength);
    VPosition(const VPosition &) = default;
    auto operator=(const VPosition &) -> VPosition & = default;

    VPosition(VPosition &&) = default;
    auto operator=(VPosition &&) -> VPosition & = default;

    ~VPosition() = default;

    void run();

    auto getBestResult() const -> VBestSquare;

    static auto ArrangeDetail(const VPositionData &data, std::atomic_bool *stop, bool saveLength) -> VBestSquare;

#ifdef LAYOUT_DEBUG
    static void DumpFrame(const VContour &contour, const VLayoutPiece &detail, QMutex *mutex,
                          const QVector<VLayoutPiece> &details);
#endif

private:
    bool m_isValid{false};
    VBestSquare m_bestResult{};
    VPositionData m_data{};
    std::atomic_bool *stop{nullptr};
    /**
     * @brief angle_between keep angle between global edge and detail edge. Need for optimization rotation.
     */
    qreal angle_between{0};

    enum class CrossingType : qint8
    {
        NoIntersection = 0,
        Intersection = 1,
        EdgeError = 2
    };

    enum class InsideType : qint8
    {
        Outside = 0,
        Inside = 1,
        EdgeError = 2
    };

    void SaveCandidate(VBestSquare &bestResult, const VLayoutPiece &detail, int globalI, int detJ, BestFrom type);

    auto CheckCombineEdges(VLayoutPiece &detail, int j, int &dEdge) -> bool;
    auto CheckRotationEdges(VLayoutPiece &detail, int j, int dEdge, qreal angle) const -> bool;

    void RotateOnAngle(qreal angle);

    auto Crossing(const VLayoutPiece &detail) const -> CrossingType;
    auto SheetContains(const QRectF &rect) const -> bool;

    void CombineEdges(VLayoutPiece &detail, const QLineF &globalEdge, int dEdge);
    static void RotateEdges(VLayoutPiece &detail, const QLineF &globalEdge, int dEdge, qreal angle);

    void Rotate(int number);
    void FollowGrainline();

    auto FabricGrainline() const -> QLineF;

    void FindBestPosition();
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VPosition::FabricGrainline return fabric gainline accoding to paper orientation
 * @return fabric gainline line
 */
inline auto VPosition::FabricGrainline() const -> QLineF
{
    return m_data.isOriginPaperOrientationPortrait ? QLineF(10, 10, 10, -100) : QLineF(10, 10, -100, 10);
}

#endif // VPOSITION_H
