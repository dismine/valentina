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
#include "../layout/layoutdef.h"
#include "../vmisc/defglobal.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(MLReader) // NOLINT

struct TextLine;
struct VLayoutPassmark;
class VLayoutPlaceLabel;
class VLayoutPiecePath;
class VTextManager;
class VLayoutPoint;

class VPLayoutFileReader : public QXmlStreamReader
{
    Q_DECLARE_TR_FUNCTIONS(VPLayoutFileReader) // NOLINT
public:
    VPLayoutFileReader()=default;
    ~VPLayoutFileReader()=default;

    auto ReadFile(const VPLayoutPtr &layout, QFile *file) -> bool;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPLayoutFileReader) // NOLINT

    void ReadLayout(const VPLayoutPtr &layout);
    void ReadProperties(const VPLayoutPtr &layout);
    void ReadControl(const VPLayoutPtr &layout);
    void ReadTiles(const VPLayoutPtr &layout);
    void ReadScale(const VPLayoutPtr &layout);
    void ReadUnplacedPieces(const VPLayoutPtr &layout);
    void ReadSheets(const VPLayoutPtr &layout);
    void ReadSheet(const VPLayoutPtr &layout);
    void ReadPieces(const VPLayoutPtr &layout, const VPSheetPtr &sheet=VPSheetPtr());
    void ReadPiece(const VPPiecePtr &piece);
    auto ReadLayoutPoint() -> VLayoutPoint;
    auto ReadLayoutPoints() -> QVector<VLayoutPoint>;
    void ReadSeamLine(const VPPiecePtr &piece);
    void ReadSeamAllowance(const VPPiecePtr &piece);
    void ReadGrainline(const VPPiecePtr &piece);
    void ReadNotches(const VPPiecePtr &piece);
    auto ReadNotch() -> VLayoutPassmark;
    void ReadInternalPaths(const VPPiecePtr &piece);
    auto ReadInternalPath() -> VLayoutPiecePath;
    void ReadMarkers(const VPPiecePtr &piece);
    auto ReadMarker() -> VLayoutPlaceLabel;
    void ReadLabels(const VPPiecePtr &piece);
    void ReadPieceLabel(const VPPiecePtr &piece);
    void ReadPatternLabel(const VPPiecePtr &piece);
    auto ReadLabelLines() -> VTextManager;
    auto ReadLabelLine() -> TextLine;
    void ReadWatermark(const VPLayoutPtr &layout);


    void ReadLayoutMargins(const VPLayoutPtr &layout);
    void ReadSheetMargins(const VPSheetPtr &sheet);
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
