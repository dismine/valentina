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

class VPPieceList;

class VPPiece : public QObject, public VLayoutPiece
{
    Q_OBJECT
public:
    VPPiece();
    VPPiece(VLayoutPiece layoutPiece);

    ~VPPiece();


    /**
     * @brief GetShowSeamLine returns wether the seam line of the piece has to be shown or not
     * @return true if the seamline has to be shown
     */
    bool GetShowSeamLine() const;

    /**
     * @brief SetShowSeamLine sets wether the seam line of the piece has to be shown or not
     * @param value true if the seamline has to be shown
     */
    void SetShowSeamLine(bool value);

    /**
     * @brief GetMirrorPiece returns wether the piece is mirrored or not
     * @return  true if the piece is mirrored
     */
    bool GetPieceMirrored() const;

    /**
     * @brief SetMirrorPiece sets wether the piece is mirrored or not
     * @param value true if the piece will be mirrored
     */
    void SetPieceMirrored(bool value);

    /**
     * @brief SetPosition Sets the position of the piece, in relation to the origin of the scene
     * @param point the point where to set the piece
     */
    void SetPosition(QPointF point);

    /**
     * @brief GetPosition Returns the position of the piece
     * @return the position of the piece
     */
    QPointF GetPosition();

    /**
     * @brief SetRotation  Sets the rotation of the piece to the given angle.
     * @param angle the angle of the rotation in degree
     */
    void SetRotation(qreal angle);

    /**
     * @brief GetRotation Returns the angle of rotation
     * @return the angle of rotation
     */
    qreal GetRotation();

    /**
     * @brief SetIsSelected Sets wether the piece is selected
     * @param value true if the piece is selected
     */
    void SetIsSelected(bool value);

    /**
     * @brief GetIsSelected Returns wether the piece is selected. It emit the signal SelectionChanged
     * @return true if the piece is selected
     */
    bool GetIsSelected();

    /**
     * @brief GetPieceList Returns the piecelist in which the piece is.
     * @return pieceList of the piece
     */
    VPPieceList* GetPieceList();

    /**
     * @brief SetPieceList Sets the pieceList of the piece to the given pieceList
     * @param pieceList
     */
    void SetPieceList(VPPieceList* pieceList);

    QIcon PieceIcon(const QSize &size) const;

signals:
    /**
     * @brief SelectionChanged emited when the selection of the piece was
     * changed through the SetIsSelected function
     */
    void SelectionChanged();

    /**
     * @brief PositionChanged emited when the position of the piece was
     * changed through the SetPosition function
     */
    void PositionChanged();

    /**
     * @brief RotationChanged emited when the position of the piece was
     * changed through the function SetRotation
     */
    void RotationChanged();

    /**
     * @brief PropertiesChanged emited when of the properties showSemaline
     * or mirrorpiece where changed.
     */
    void PropertiesChanged();



private:
    Q_DISABLE_COPY(VPPiece)

    QPointF m_offset{QPointF()};

    QVector<QPointF> m_grainline{QVector<QPointF>()};
    bool m_isGrainlineEnabled{false};
    qreal m_grainlineAngle{0};

    QTransform m_transform{QTransform()};
    // use a separate value for now because it's not easy to get the angle from the transform matrix
    qreal m_pieceAngle{0};

    bool m_showSeamline{true};
    bool m_mirrorPiece{false};

    bool m_isSelected{false};
    VPPieceList *m_pieceList{nullptr};
};

#endif // VPPIECE_H
