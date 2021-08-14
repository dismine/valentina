/************************************************************************
 **
 **  @file   vppiece.h
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef VPPIECE_H
#define VPPIECE_H

#include <QUuid>
#include <QVector>
#include <QPoint>
#include <QTransform>

#include "../vlayout/vlayoutpiece.h"

class VPLayout;
class VPSheet;

class VPPiece : public VLayoutPiece
{
public:
    VPPiece() = default;
    explicit VPPiece(const VLayoutPiece &layoutPiece);

    virtual ~VPPiece() = default;

    void ClearTransformations();

    /**
     * @brief SetPosition Sets the position of the piece, in relation to the origin of the scene
     * @param point the point where to set the piece
     */
    void SetPosition(QPointF point);

    /**
     * @brief GetPosition Returns the position of the piece
     * @return the position of the piece
     */
    auto GetPosition() -> QPointF;

    /**
     * @brief RotateToGrainline  rotates the piece to follow the grainline
     */
    void RotateToGrainline();

    /**
     * @brief SetSelected Sets wether the piece is selected
     * @param value true if the piece is selected
     */
    void SetSelected(bool value);

    /**
     * @brief IsSelected Returns wether the piece is selected. It emit the signal SelectionChanged
     * @return true if the piece is selected
     */
    auto IsSelected() const -> bool;

    auto Sheet() const -> VPSheet *;
    void SetSheet(VPSheet *newSheet);

    auto Layout() const -> VPLayout *;
    void SetLayout(VPLayout *layout);

    void SetGrainlineEnabled(bool enabled);
    void SetGrainlineAngle(qreal angle);
    void SetGrainlineArrowType(GrainlineArrowDirection type);
    void SetGrainlinePoints(const QVector<QPointF> &points);

    auto GetPieceLabelRect() const -> QVector<QPointF>;
    void SetPieceLabelRect(const QVector<QPointF> &rect);

    auto GetPieceLabelData() const ->VTextManager;
    void SetPieceLabelData(const VTextManager &data);

    auto GetPatternLabelRect() const -> QVector<QPointF>;
    void SetPatternLabelRect(const QVector<QPointF> &rect);

    auto GetPatternLabelData() const ->VTextManager;
    void SetPatternLabelData(const VTextManager &data);

private:
    Q_DISABLE_COPY(VPPiece)

    VPLayout *m_layout{nullptr};

    VPSheet *m_sheet{nullptr};

    bool m_isSelected{false};
};

#endif // VPPIECE_H
