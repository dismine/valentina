/************************************************************************
 **
 **  @file   vppiece.cpp
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
#include "vppiece.h"

#include <QtMath>

#include "../vmisc/def.h"
#include "vpsheet.h"
#include "../vlayout/vtextmanager.h"

#include <QIcon>
#include <QLoggingCategory>
#include <QPainter>

Q_LOGGING_CATEGORY(pPiece, "p.piece")

//---------------------------------------------------------------------------------------------------------------------
VPPiece::VPPiece(const VLayoutPiece &layoutPiece)
    : VLayoutPiece(layoutPiece)
{
    ClearTransformations();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::ClearTransformations()
{
    // Reset the piece position to the default state
    QTransform matrix;
    SetMatrix(matrix);
    // translate the piece so that the top left corner of the bouding rect of the piece is at the position
    // (0,0) in the sheet coordinate system
    const QPointF offset = MappedDetailBoundingRect().topLeft();
    matrix.translate(-offset.x() ,-offset.y());
    SetMatrix(matrix);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPosition(QPointF point)
{
    QTransform matrix = GetMatrix();
    const QPointF offset = MappedDetailBoundingRect().topLeft();
    matrix.translate(point.x() - offset.x(), point.y() - offset.y());
    SetMatrix(matrix);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetPosition() -> QPointF
{
    QTransform matrix = GetMatrix();
    return QPointF(matrix.dx(), matrix.dy());
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::RotateToGrainline()
{
    VPSheetPtr sheet = Sheet();
    if (not IsGrainlineEnabled() || sheet.isNull())
    {
        return;
    }

    const QVector<QPointF> grainlinePoints = GetMappedGrainline();
    if (grainlinePoints.count() < 2)
    {
        return;
    }

    QLineF grainline(grainlinePoints.first(), grainlinePoints.last());

    QLineF canonical(grainlinePoints.first().x(), grainlinePoints.first().y(),
                     grainlinePoints.first().x()+100, grainlinePoints.first().y());

    GrainlineType grainlineType = sheet->GrainlineType();

    auto DegreesAtFront = [grainline, canonical, grainlineType]()
    {
        QLineF atFront = canonical;
        if (grainlineType == GrainlineType::Vertical)
        {
            atFront.setAngle(90);
        }

        return grainline.angleTo(atFront);
    };

    auto DegreesAtRear = [grainline, canonical, grainlineType]()
    {
        QLineF atRear = canonical;
        atRear.setAngle(grainlineType == GrainlineType::Vertical ? 270 : 180);

        return grainline.angleTo(atRear);
    };

    GrainlineArrowDirection type = GrainlineArrowType();
    qreal degrees = 0;

    if (type == GrainlineArrowDirection::atFront)
    {
        degrees = DegreesAtFront();
    }
    else if (type == GrainlineArrowDirection::atRear)
    {
        degrees = DegreesAtRear();
    }
    else
    {
        degrees = qMin(DegreesAtFront(), DegreesAtRear());
    }

    Rotate(MappedDetailBoundingRect().center(), degrees);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetSelected(bool value)
{
    m_isSelected = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::IsSelected() const -> bool
{
    return m_isSelected;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::Sheet() const -> VPSheetPtr
{
    return m_sheet;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetSheet(const VPSheetPtr &newSheet)
{
    m_sheet = newSheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::Layout() const -> VPLayoutPtr
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetLayout(const VPLayoutPtr &layout)
{
    SCASSERT(layout != nullptr)
    m_layout = layout;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainlineEnabled(bool enabled)
{
    VLayoutPiece::SetGrainlineEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainlineAngle(qreal angle)
{
    VLayoutPiece::SetGrainlineAngle(angle);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainlineArrowType(GrainlineArrowDirection type)
{
    VLayoutPiece::SetGrainlineArrowType(type);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainlinePoints(const QVector<QPointF> &points)
{
    VLayoutPiece::SetGrainlinePoints(points);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetPieceLabelRect() const -> QVector<QPointF>
{
    return VLayoutPiece::GetPieceLabelRect();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPieceLabelRect(const QVector<QPointF> &rect)
{
    VLayoutPiece::SetPieceLabelRect(rect);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetPieceLabelData() const -> VTextManager
{
    return VLayoutPiece::GetPieceLabelData();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPieceLabelData(const VTextManager &data)
{
    VLayoutPiece::SetPieceLabelData(data);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetPatternLabelRect() const -> QVector<QPointF>
{
    return VLayoutPiece::GetPatternLabelRect();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPatternLabelRect(const QVector<QPointF> &rect)
{
    VLayoutPiece::SetPatternLabelRect(rect);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetPatternLabelData() const -> VTextManager
{
    return VLayoutPiece::GetPatternLabelData();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPatternLabelData(const VTextManager &data)
{
    VLayoutPiece::SetPatternLabelData(data);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::Flip()
{
    QTransform pieceMatrix = GetMatrix();
    QPointF center = pieceMatrix.map(DetailBoundingRect().center());

    QTransform m;
    m.translate(center.x(), 0);
    m.scale(-1, 1);
    m.translate(-center.x(), 0);

    pieceMatrix *= m;
    SetMatrix(pieceMatrix);
    SetMirror(!IsMirror());
}
