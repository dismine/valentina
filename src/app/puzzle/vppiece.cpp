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
VPPiece::~VPPiece()
{

}


//---------------------------------------------------------------------------------------------------------------------
QString VPPiece::GetName() const
{
    return m_name;
}


//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetName(const QString &name)
{
    m_name = name;
}


//---------------------------------------------------------------------------------------------------------------------
QUuid VPPiece::GetUuid() const
{
    return m_uuid;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetUuid(const QUuid &uuid)
{
    m_uuid = uuid;
}


//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPPiece::GetCuttingLine() const
{
    return m_cuttingLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetCuttingLine(const QVector<QPointF> &cuttingLine)
{
    m_cuttingLine = cuttingLine;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPPiece::GetSeamLine() const
{
    return m_seamLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetSeamLine(const QVector<QPointF> &seamLine)
{
    m_seamLine = seamLine;
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
    return QPointF(m_transform.dx(), m_transform.dy());
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
void VPPiece::SetIsGrainlineEnabled(bool value)
{
    m_isGrainlineEnabled = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPPiece::GetIsGrainlineEnabled()
{
    return m_isGrainlineEnabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainlineAngle(qreal value)
{
    m_grainlineAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPPiece::GetGrainlineAngle()
{
    return m_grainlineAngle;
}
//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainline(QVector<QPointF> grainline)
{
    m_grainline = grainline;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPPiece::GetGrainline()
{
    return m_grainline;
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

//---------------------------------------------------------------------------------------------------------------------
QIcon VPPiece::PieceIcon(const QSize &size) const
{
    QVector<QPointF> points = GetSeamLine();
    if(points.isEmpty())
    {
        points = GetCuttingLine();
    }

    QPolygonF shape(points);
    shape << shape.first();

    QRectF boundingRect = shape.boundingRect();
    qreal canvasSize = qMax(boundingRect.height(), boundingRect.width());
    QRectF canvas = QRectF(0, 0, canvasSize, canvasSize);

    qreal dx = canvas.center().x() - boundingRect.center().x();
    qreal dy = canvas.center().y() - boundingRect.center().y();

    QPixmap pixmap(size);
    pixmap.fill(QColor("white"));

    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int spacing = 2;
    painter.translate(spacing, spacing);

    qreal scaleFactorX = canvasSize * 100 / (size.width() - spacing*2) / 100;
    qreal scaleFactorY = canvasSize * 100 / (size.height() - spacing*2) / 100;
    painter.scale(1./scaleFactorX, 1./scaleFactorY);
    painter.setPen(QPen(Qt::black, 0.8*qMax(scaleFactorX, scaleFactorY)));

    painter.translate(dx, dy);

    painter.drawPolygon(shape);
    painter.end();

    QIcon icon;

    icon.addPixmap(pixmap,QIcon::Normal);
    icon.addPixmap(pixmap,QIcon::Selected);

    return icon;
}
