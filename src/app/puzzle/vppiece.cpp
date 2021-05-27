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

#include "vppiecelist.h"
#include "../vmisc/def.h"

#include <QIcon>
#include <QLoggingCategory>
#include <QPainter>

Q_LOGGING_CATEGORY(pPiece, "p.piece")

//---------------------------------------------------------------------------------------------------------------------
VPPiece::VPPiece()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPPiece::VPPiece(VLayoutPiece layoutPiece): VLayoutPiece(layoutPiece)
{
    // Resets the translation of the matrix
    QTransform matrix = GetMatrix();
    matrix.translate(-matrix.dx() ,-matrix.dy());
    SetMatrix(matrix);

    // then translate the piece so that the top left corner of the bouding rect of the piece is at the position
    // (0,0) in the sheet coordinate system
    QRectF boundingRect = MappedDetailBoundingRect();
    QPointF offset = boundingRect.topLeft();
    matrix = GetMatrix();
    matrix.translate(-offset.x() ,-offset.y());
    SetMatrix(matrix);
}

//---------------------------------------------------------------------------------------------------------------------
VPPiece::~VPPiece()
{

}

//---------------------------------------------------------------------------------------------------------------------
bool VPPiece::GetShowSeamLine() const
{
    return m_showSeamline;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetShowSeamLine(bool value)
{
    m_showSeamline = value;

    emit PropertiesChanged();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPPiece::GetPieceMirrored() const
{
    return m_mirrorPiece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPieceMirrored(bool value)
{
    m_mirrorPiece = value;

    emit PropertiesChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPosition(QPointF point)
{
    m_transform.translate(point.x() - m_transform.dx(), point.y() - m_transform.dy());

    emit PositionChanged();
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VPPiece::GetPosition()
{
    return QPointF(m_transform.dx(),m_transform.dy());
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetRotation(qreal angle)
{
    //    qreal currentAngle = GetRotation();
    //    qreal newAngle = angle - currentAngle;

    //    m_transform.rotate(newAngle);

    if(not VFuzzyComparePossibleNulls(m_pieceAngle, angle))
    {
        m_pieceAngle = angle;

        // make sure the angle is  [0 <= angle < 360]
        while(m_pieceAngle >= 360)
        {
            m_pieceAngle -= 360;
        }

        while(m_pieceAngle < 0)
        {
            m_pieceAngle += 360;
        }

        emit RotationChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::RotateBy(qreal angle)
{
    SetRotation(m_pieceAngle + angle);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::RotateToGrainline(qreal angleOfGrainline, bool add180IfAlreadyInPosition)
{
    qreal newAngle = -GrainlineAngle() + angleOfGrainline;
    if(newAngle < 0)
    {
        newAngle += 360;
    }

    if(not VFuzzyComparePossibleNulls(m_pieceAngle, newAngle))
    {
        SetRotation(newAngle);
    }
    else if(add180IfAlreadyInPosition)
    {
        newAngle += 180;
        SetRotation(newAngle);
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPPiece::GetRotation()
{
    return m_pieceAngle;

    // We don't use the QTransform vor now because the math behind it to retrieve the angle is not trivial.
    // TODO / FIXME:  we can use QTransform later for optimization


//    QTransform tmpTransform = m_transform;
//    tmpTransform.translate(-tmpTransform.dx(), -tmpTransform.dy()); // make sure there is only the rotation in the matrix

//    qreal angle = qRadiansToDegrees(qAcos(tmpTransform.m11()));

//    qCDebug(pPiece, "new angle : %f", angle);

//    return angle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetIsSelected(bool value)
{
    if(m_isSelected != value)
    {
        m_isSelected = value;
        emit SelectionChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VPPiece::GetIsSelected()
{
    return m_isSelected;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPPiece::GetPieceList()
{
    return m_pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPieceList(VPPieceList* pieceList)
{
    if(pieceList != m_pieceList)
    {
        m_pieceList = pieceList;
    }
}
