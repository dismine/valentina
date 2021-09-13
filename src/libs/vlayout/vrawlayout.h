/************************************************************************
 **
 **  @file   vrawlayout.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 4, 2020
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
#ifndef VRAWLAYOUT_H
#define VRAWLAYOUT_H

#include "vlayoutpiece.h"

struct VRawLayoutData
{
    QVector<VLayoutPiece> pieces{};

    friend QDataStream& operator<< (QDataStream& dataStream, const VRawLayoutData& data);
    friend QDataStream& operator>> (QDataStream& dataStream, VRawLayoutData& data);

private:
    static const quint32 streamHeader;
    static const quint16 classVersion;
};


class VRawLayout
{
    Q_DECLARE_TR_FUNCTIONS(VRawLayout)
public:
    VRawLayout();

    bool WriteFile(QIODevice* ioDevice, const VRawLayoutData& data);
    bool ReadFile(QIODevice* ioDevice, VRawLayoutData& data);

    bool WriteFile(const QString& filePath, const VRawLayoutData& data);
    bool ReadFile(const QString& filePath, VRawLayoutData& data);

    QString ErrorString() const;

private:
    QString m_errorString{};

    static const QByteArray fileHeaderByteArray;
    static const quint16 fileVersion;
};

//---------------------------------------------------------------------------------------------------------------------
inline QString VRawLayout::ErrorString() const
{
    return m_errorString;
}

#endif // VRAWLAYOUT_H
