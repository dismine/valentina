/************************************************************************
 **
 **  @file   vpuzzlepiece.h
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
#ifndef VPUZZLEPIECE_H
#define VPUZZLEPIECE_H

#include <QUuid>
#include <QVector>
#include <QPoint>

class VPuzzlePiece
{
public:
    VPuzzlePiece();
    ~VPuzzlePiece();

    /**
     * @brief GetName Returns the name of the piece
     * @return the piece's name
     */
    QString GetName() const;

    /**
     * @brief SetName Sets the piece's name to the given name
     * @param name new name of the piece
     */
    void SetName(const QString &name);

    /**
     * @brief GetUuid Returns the uuid of the piece
     * @return the uuid of the piece
     */
    QUuid GetUuid() const;

    /**
     * @brief SetUuid Sets the uuid of the piece to the given value
     * @return the uuid of the piece
     */
    void SetUuid(const QUuid &uuid);

    QVector<QPointF> GetCuttingLine() const;

    void SetCuttingLine(const QVector<QPointF> &cuttingLine);


private:
    QUuid m_uuid{QUuid()};
    QString m_name{QString()};
    QVector<QPointF> m_cuttingLine{QVector<QPointF>()};
};

#endif // VPUZZLEPIECE_H
