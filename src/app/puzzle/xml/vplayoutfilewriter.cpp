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
#include "vplayout.h"
#include "vpsheet.h"
#include "vppiecelist.h"
#include "vppiece.h"
#include "vplayoutliterals.h"
#include "../ifc/xml/vlayoutconverter.h"

//---------------------------------------------------------------------------------------------------------------------
VPLayoutFileWriter::VPLayoutFileWriter()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPLayoutFileWriter::~VPLayoutFileWriter()
{

}

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

    WriteProperties(layout);
    WriteUnplacePiecesList(layout);

    writeEndElement(); //layout
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteProperties(VPLayout *layout)
{
    writeStartElement(ML::TagProperties);

    writeTextElement(ML::TagUnit, UnitsToStr(layout->GetUnit()));

    writeTextElement(ML::TagDescription, QString()); // TODO : define the value in layout

    writeStartElement(ML::TagControl);

    SetAttribute(ML::AttrWarningSuperposition, layout->GetWarningSuperpositionOfPieces());
    SetAttribute(ML::AttrWarningOutOfBound, layout->GetWarningPiecesOutOfBound());
    writeEndElement(); // control

    // WriteTiles(layout);  TODO: when tile functionality implemented, then uncomment this line

    writeEndElement(); // properties
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteUnplacePiecesList(VPLayout *layout)
{
    Q_UNUSED(layout);
    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheets(VPLayout *layout)
{
    Q_UNUSED(layout);
    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheet(VPSheet* sheet)
{
    Q_UNUSED(sheet);
    // TODO

    // WritePieceList(pieceList);

}



//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteTiles(VPLayout *layout)
{
    Q_UNUSED(layout); // to be removed

   writeStartElement(ML::TagTiles);
   SetAttribute(ML::AttrVisible, false); // TODO / Fixme get the right value
   SetAttribute(ML::AttrMatchingMarks, "standard"); // TODO / Fixme get the right value

   QSizeF size = QSizeF(); // TODO get the right size
   WriteSize(size);

   QMarginsF margins = QMarginsF(); // TODO get the right margins
   WriteMargins(margins);

   writeEndElement(); // tiles
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WritePieceList(VPPieceList *pieceList)
{
    WritePieceList(pieceList, ML::TagPieceList);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WritePieceList(VPPieceList *pieceList, const QString &tagName)
{
    writeStartElement(tagName); // piece list
    SetAttribute(ML::AttrName, pieceList->GetName());
    SetAttribute(ML::AttrVisible, pieceList->GetIsVisible());
    //  TODO selected info. Not sure how it's saved yet
    //SetAttribute("selected", pieceList->GetIsSelected());


    QList<VPPiece*> pieces = pieceList->GetPieces();
    for (auto piece : pieces)
    {
        WritePiece(piece);
    }

    writeEndElement(); // piece list
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WritePiece(VPPiece *piece)
{
    Q_UNUSED(piece);

    writeStartElement(ML::TagPiece);
    SetAttribute(ML::AttrID, piece->GetUuid().toString());
    SetAttribute(ML::AttrName, piece->GetName());
    SetAttribute(ML::AttrMirrored, piece->GetPieceMirrored()); // TODO / Fixme get the right value
    SetAttribute(ML::AttrShowSeamline, piece->GetShowSeamLine()); // TODO / Fixme get the right value
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
    SetAttribute(ML::AttrLeft, margins.left());
    SetAttribute(ML::AttrTop, margins.top());
    SetAttribute(ML::AttrRight, margins.right());
    SetAttribute(ML::AttrBottom, margins.bottom());
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
