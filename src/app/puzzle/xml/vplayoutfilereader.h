/************************************************************************
 **
 **  @file   vplayoutfilereader.h
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

#ifndef VPLAYOUTFILEREADER_H
#define VPLAYOUTFILEREADER_H

#include <QXmlStreamReader>
#include "../ifc/xml/vabstractconverter.h"
#include "vplayout.h"
#include "vppiecelist.h"
#include "vppiece.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(MLReader)

class VPLayoutFileReader : public QXmlStreamReader
{
    Q_DECLARE_TR_FUNCTIONS(VPLayoutFileReader)
public:
    VPLayoutFileReader()=default;
    ~VPLayoutFileReader()=default;

    bool ReadFile(VPLayout *layout, QFile *file);

private:
    Q_DISABLE_COPY(VPLayoutFileReader)

    void ReadLayout(VPLayout *layout);
    void ReadProperties(VPLayout *layout);
    void ReadControl(VPLayout *layout);
    void ReadTiles(VPLayout *layout);
    void ReadUnplacedPieces(VPLayout *layout);
    void ReadSheets(VPLayout *layout);
    void ReadSheetPieces(VPSheet *sheet);
    void ReadSheet(VPLayout *layout);
    void ReadPieceList(VPPieceList *pieceList);
    void ReadPiece(VPPiece *piece);

    QMarginsF ReadMargins();
    QSizeF ReadSize();

    void AssertRootTag(const QString &tag) const;

    static QString ReadAttributeString(const QXmlStreamAttributes &attribs, const QString &name,
                                       const QString &defValue);
    static QString ReadAttributeEmptyString(const QXmlStreamAttributes &attribs, const QString &name);
    static bool    ReadAttributeBool(const QXmlStreamAttributes &attribs, const QString &name, const QString &defValue);
    static qreal   ReadAttributeDouble(const QXmlStreamAttributes &attribs, const QString &name,
                                       const QString &defValue);
};

#endif // VPLAYOUTFILEREADER_H
