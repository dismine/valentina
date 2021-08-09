/************************************************************************
 **
 **  @file   vplayoutfilewriter.cpp
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
 *************************************************************************/

#include "vplayoutfilewriter.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "../layout/vppiece.h"
#include "vplayoutliterals.h"
#include "../ifc/xml/vlayoutconverter.h"
#include "../vmisc/projectversion.h"

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteFile(VPLayout *layout, QFile *file)
{
    setDevice(file);
    setAutoFormatting(true);

    writeStartDocument();
    WriteLayout(layout);
    writeEndDocument();

    file->close();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteLayout(VPLayout *layout)
{
    writeStartElement(ML::TagLayout);
    SetAttribute(ML::AttrVersion, VLayoutConverter::LayoutMaxVerStr);
    writeComment(QStringLiteral("Layout created with Valentina v%1 (https://smart-pattern.com.ua/).")
                 .arg(APP_VERSION_STR));

    WriteProperties(layout);
    WritePieceList(layout->GetUnplacedPieces(), ML::TagUnplacedPieces);
    WriteSheets(layout);

    writeEndElement(); //layout
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteProperties(VPLayout *layout)
{
    writeStartElement(ML::TagProperties);

    writeTextElement(ML::TagUnit, UnitsToStr(layout->LayoutSettings().GetUnit()));
    writeTextElement(ML::TagTitle, layout->LayoutSettings().GetTitle());
    writeTextElement(ML::TagDescription, layout->LayoutSettings().GetDescription());
    WriteSize(layout->LayoutSettings().GetSheetSize());
    WriteMargins(layout->LayoutSettings().GetSheetMargins());

    writeStartElement(ML::TagControl);
    SetAttribute(ML::AttrWarningSuperposition, layout->LayoutSettings().GetWarningSuperpositionOfPieces());
    SetAttribute(ML::AttrWarningOutOfBound, layout->LayoutSettings().GetWarningPiecesOutOfBound());
    writeEndElement(); // control

    WriteTiles(layout);

    writeEndElement(); // properties
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheets(VPLayout *layout)
{
    writeStartElement(ML::TagSheets);

    QList<VPSheet *> sheets = layout->GetSheets();
    for (auto *sheet : sheets)
    {
        WriteSheet(sheet);
    }

    writeEndElement(); // sheets
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheet(VPSheet* sheet)
{
    writeStartElement(ML::TagSheet);

    writeTextElement(ML::TagName, sheet->GetName());
    WritePieceList(sheet->GetPieces(), ML::TagPieces);

    writeEndElement(); // sheet

}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteTiles(VPLayout *layout)
{
    Q_UNUSED(layout); // to be removed

   writeStartElement(ML::TagTiles);
   SetAttribute(ML::AttrVisible, layout->LayoutSettings().GetShowTiles());
   SetAttribute(ML::AttrMatchingMarks, "standard"); // TODO / Fixme get the right value

   WriteSize(layout->LayoutSettings().GetTilesSize());
   WriteMargins(layout->LayoutSettings().GetTilesMargins());

   writeEndElement(); // tiles
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WritePieceList(const QList<VPPiece *> &list, const QString &tagName)
{
    writeStartElement(tagName); // piece list
    for (auto *piece : list)
    {
        WritePiece(piece);
    }

    writeEndElement(); // piece list
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WritePiece(VPPiece *piece)
{
    writeStartElement(ML::TagPiece);
    SetAttribute(ML::AttrID, piece->GetUUID().toString());
    SetAttribute(ML::AttrName, piece->GetName());
    SetAttribute(ML::AttrMirrored, piece->IsMirror());
//    SetAttribute(ML::AttrShowSeamline, piece->GetShowSeamLine());
    SetAttribute(ML::AttrTransform, "string representation of the transformation"); // TODO / Fixme get the right value

    // TODO cuttingLine
    // TODO seamLine
    // TODO grainline
    // TODO passmarks
    // TODO internal paths
    // TODO placeLabels (buttonholes etc.)

    // TODO labels

    writeEndElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteMargins(const QMarginsF &margins)
{
    writeStartElement(ML::TagMargin);
    SetAttributeOrRemoveIf<qreal>(ML::AttrLeft, margins.left(), [](qreal margin){return margin <= 0;});
    SetAttributeOrRemoveIf<qreal>(ML::AttrTop, margins.top(), [](qreal margin){return margin <= 0;});
    SetAttributeOrRemoveIf<qreal>(ML::AttrRight, margins.right(), [](qreal margin){return margin <= 0;});
    SetAttributeOrRemoveIf<qreal>(ML::AttrBottom, margins.bottom(), [](qreal margin){return margin <= 0;});
    writeEndElement(); // margin
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSize(QSizeF size)
{
    // maybe not necessary to test this, the writer should "stupidly write", the application should take care of these tests
    qreal width = size.width();
    if(width < 0)
    {
        width = 0;
    }

    qreal length = size.height();
    if(length < 0)
    {
        length = 0;
    }

    writeStartElement(ML::TagSize);
    SetAttribute(ML::AttrWidth, width);
    SetAttribute(ML::AttrLength, length);
    writeEndElement(); // size
}
