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
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(MLReader)

struct TextLine;

class VPLayoutFileReader : public QXmlStreamReader
{
    Q_DECLARE_TR_FUNCTIONS(VPLayoutFileReader)
public:
    VPLayoutFileReader()=default;
    ~VPLayoutFileReader()=default;

    auto ReadFile(VPLayout *layout, QFile *file) -> bool;

private:
    Q_DISABLE_COPY(VPLayoutFileReader)

    void ReadLayout(VPLayout *layout);
    void ReadProperties(VPLayout *layout);
    void ReadControl(VPLayout *layout);
    void ReadTiles(VPLayout *layout);
    void ReadUnplacedPieces(VPLayout *layout);
    void ReadSheets(VPLayout *layout);
    void ReadSheet(VPLayout *layout);
    void ReadPieces(VPLayout *layout, VPSheet *sheet=nullptr);
    void ReadPiece(VPPiece *piece);
    void ReadSeamAllowance(VPPiece *piece);
    void ReadGrainline(VPPiece *piece);
    void ReadNotches(VPPiece *piece);
    auto ReadNotch() -> VLayoutPassmark;
    void ReadInternalPaths(VPPiece *piece);
    auto ReadInternalPath() -> VLayoutPiecePath;
    void ReadMarkers(VPPiece *piece);
    auto ReadMarker() -> VLayoutPlaceLabel;
    void ReadLabels(VPPiece *piece);
    auto ReadLabelLines() -> VTextManager;
    auto ReadLabelLine() -> TextLine;

    auto ReadMargins() -> QMarginsF;
    auto ReadSize() -> QSizeF;

    void AssertRootTag(const QString &tag) const;

    static auto ReadAttributeString(const QXmlStreamAttributes &attribs, const QString &name,
                                    const QString &defValue) -> QString;
    static auto ReadAttributeEmptyString(const QXmlStreamAttributes &attribs, const QString &name) -> QString;
    static auto ReadAttributeBool(const QXmlStreamAttributes &attribs, const QString &name,
                                  const QString &defValue) -> bool;
    static auto ReadAttributeDouble(const QXmlStreamAttributes &attribs, const QString &name,
                                    const QString &defValue) -> qreal;
    static auto ReadAttributeUInt(const QXmlStreamAttributes &attribs, const QString &name,
                                  const QString &defValue) -> quint32;
    static auto ReadAttributeInt(const QXmlStreamAttributes &attribs, const QString &name,
                                 const QString &defValue) -> int;
};

#endif // VPLAYOUTFILEREADER_H
