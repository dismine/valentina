/************************************************************************
 **
 **  @file   vpuzzlelayoutfilereader.h
 **  @author Ronan Le Tiec
 **  @date   18 4, 2020
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
 ** *************************************************************************/

#ifndef VPUZZLELAYOUTFILEREADER_H
#define VPUZZLELAYOUTFILEREADER_H

#include <QXmlStreamReader>
#include "../ifc/xml/vabstractconverter.h"
#include "vpuzzlelayout.h"
#include "vpuzzlelayer.h"
#include "vpuzzlepiece.h"

class VPuzzleLayoutFileReader : public QXmlStreamReader
{
    Q_DECLARE_TR_FUNCTIONS(VPuzzleLayoutFileReader)
public:
    VPuzzleLayoutFileReader();
    ~VPuzzleLayoutFileReader();

    bool ReadFile(VPuzzleLayout *layout, QFile *file);

private:
    Q_DISABLE_COPY(VPuzzleLayoutFileReader)

    void ReadLayout(VPuzzleLayout *layout);
    void ReadProperties(VPuzzleLayout *layout);
    void ReadTiles(VPuzzleLayout *layout);
    void ReadLayers(VPuzzleLayout *layout);
    void ReadLayer(VPuzzleLayer *layer);
    void ReadPiece(VPuzzlePiece *piece);

    QMarginsF ReadMargins();
    QSizeF ReadSize();

    static QString ReadAttributeString(const QXmlStreamAttributes &attribs, const QString &name,
                                       const QString &defValue);
    static QString ReadAttributeEmptyString(const QXmlStreamAttributes &attribs, const QString &name);
    static bool    ReadAttributeBool(const QXmlStreamAttributes &attribs, const QString &name, const QString &defValue);
    static qreal   ReadAttributeDouble(const QXmlStreamAttributes &attribs, const QString &name,
                                     const QString &defValue);
};

#endif // VPUZZLELAYOUTFILEREADER_H
