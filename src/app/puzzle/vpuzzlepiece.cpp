/************************************************************************
 **
 **  @file   vpuzzlepiece.cpp
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
#include "vpuzzlepiece.h"

#include "vpuzzlelayer.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pPiece, "p.piece")

//---------------------------------------------------------------------------------------------------------------------
VPuzzlePiece::VPuzzlePiece()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzlePiece::~VPuzzlePiece()
{

}


//---------------------------------------------------------------------------------------------------------------------
QString VPuzzlePiece::GetName() const
{
    return m_name;
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetName(const QString &name)
{
    m_name = name;
}


//---------------------------------------------------------------------------------------------------------------------
QUuid VPuzzlePiece::GetUuid() const
{
    return m_uuid;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetUuid(const QUuid &uuid)
{
    m_uuid = uuid;
}


//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPuzzlePiece::GetCuttingLine() const
{
    return m_cuttingLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetCuttingLine(const QVector<QPointF> &cuttingLine)
{
    m_cuttingLine = cuttingLine;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPuzzlePiece::GetSeamLine() const
{
    return m_seamLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetSeamLine(const QVector<QPointF> &seamLine)
{
    m_seamLine = seamLine;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzlePiece::GetShowSeamLine()
{
    return m_showSeamline;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetShowSeamLine(bool value)
{
    m_showSeamline = value;

    emit PropertiesChanged();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzlePiece::GetPieceMirrored()
{
    return m_mirrorPiece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetPieceMirrored(bool value)
{
    m_mirrorPiece = value;

    emit PropertiesChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetPosition(QPointF point)
{
    m_transform.translate(point.x() - m_transform.dx(), point.y() - m_transform.dy());

    emit PositionChanged();
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VPuzzlePiece::GetPosition()
{
    return QPointF(m_transform.dx(), m_transform.dy());
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetRotation(qreal angle)
{
    Q_UNUSED(angle);
    //TODO

    emit RotationChanged();
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzlePiece::GetRotation()
{
    // TODO
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetIsSelected(bool value)
{
    if(m_isSelected != value)
    {
        m_isSelected = value;
        emit SelectionChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzlePiece::GetIsSelected()
{
    return m_isSelected;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetIsGrainlineEnabled(bool value)
{
    m_isGrainlineEnabled = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzlePiece::GetIsGrainlineEnabled()
{
    return m_isGrainlineEnabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetGrainlineAngle(qreal value)
{
    m_grainlineAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzlePiece::GetGrainlineAngle()
{
    return m_grainlineAngle;
}
//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetGrainline(QVector<QPointF> grainline)
{
    m_grainline = grainline;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPuzzlePiece::GetGrainline()
{
    return m_grainline;
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer* VPuzzlePiece::GetLayer()
{
    return m_layer;
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzlePiece::SetLayer(VPuzzleLayer* layer)
{
    if(layer != m_layer)
    {
        m_layer = layer;
    }
}
