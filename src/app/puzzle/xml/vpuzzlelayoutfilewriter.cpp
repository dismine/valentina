/************************************************************************
 **
 **  @file   vpuzzlelayoutfilewriter.cpp
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

#include "vpuzzlelayoutfilewriter.h"
#include "vpuzzlelayout.h"
#include "vpuzzlelayer.h"
#include "vpuzzlepiece.h"
#include "layoutliterals.h"
#include "../ifc/xml/vlayoutconverter.h"

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayoutFileWriter::VPuzzleLayoutFileWriter()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayoutFileWriter::~VPuzzleLayoutFileWriter()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteFile(VPuzzleLayout *layout, QFile *file)
{
    setDevice(file);
    setAutoFormatting(true);

    writeStartDocument();
    WriteLayout(layout);
    writeEndDocument();

    file->close();
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteLayout(VPuzzleLayout *layout)
{
    writeStartElement(ML::TagLayout);
    SetAttribute(ML::AttrVersion, VLayoutConverter::LayoutMaxVerStr);

    WriteProperties(layout);
    WriteLayers(layout);

    writeEndElement(); //layout
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteProperties(VPuzzleLayout *layout)
{
    writeStartElement(ML::TagProperties);

    writeTextElement(ML::TagUnit, UnitsToStr(layout->GetUnit()));

    writeTextElement(ML::TagDescription, QString()); // TODO : define the value in layout

    WriteSize(layout->GetLayoutSize());

    WriteMargins(layout->GetLayoutMargins());

    writeStartElement(ML::TagControl);
    SetAttribute(ML::AttrFollowGrainLine, "no"); // TODO / Fixme get the right value
    SetAttribute(ML::AttrWarningSuperposition, layout->GetWarningSuperpositionOfPieces());
    SetAttribute(ML::AttrWarningOutOfBound, layout->GetWarningPiecesOutOfBound());
    SetAttribute(ML::AttrStickyEdges, layout->GetStickyEdges());
    SetAttribute(ML::AttrPiecesGap, layout->GetPiecesGap());
    writeEndElement(); // control

    // WriteTiles(layout);  TODO: when tile functionality implemented, then uncomment this line

    writeEndElement(); // properties
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteTiles(VPuzzleLayout *layout)
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
void VPuzzleLayoutFileWriter::WriteLayers(VPuzzleLayout *layout)
{
   writeStartElement(ML::TagLayers);

   WriteLayer(layout->GetUnplacedPiecesLayer(), ML::TagUnplacedPiecesLayer);

   QList<VPuzzleLayer*> layers = layout->GetLayers();
   for (auto layer : layers)
   {
       WriteLayer(layer);
   }

   writeEndElement(); // layers
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteLayer(VPuzzleLayer *layer)
{
    WriteLayer(layer, ML::TagLayer);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteLayer(VPuzzleLayer *layer, const QString &tagName)
{
    writeStartElement(tagName); // layer
    SetAttribute(ML::AttrName, layer->GetName());
    SetAttribute(ML::AttrVisible, layer->GetIsVisible());
    //  TODO selected info. Not sure how it's saved yet
    //SetAttribute("selected", layer->GetIsSelected());


    QList<VPuzzlePiece*> pieces = layer->GetPieces();
    for (auto piece : pieces)
    {
        WritePiece(piece);
    }

    writeEndElement(); // layer
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WritePiece(VPuzzlePiece *piece)
{
    Q_UNUSED(piece);

    writeStartElement(ML::TagPiece);
    SetAttribute(ML::AttrID, "uuid1"); // TODO / Fixme get the right value
    SetAttribute(ML::AttrName, "Piece name"); // TODO / Fixme get the right value
    SetAttribute(ML::AttrMirrored, "false"); // TODO / Fixme get the right value
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
void VPuzzleLayoutFileWriter::WriteMargins(const QMarginsF &margins)
{
    writeStartElement(ML::TagMargin);
    SetAttribute(ML::AttrLeft, margins.left());
    SetAttribute(ML::AttrTop, margins.top());
    SetAttribute(ML::AttrRight, margins.right());
    SetAttribute(ML::AttrBottom, margins.bottom());
    writeEndElement(); // margin
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteSize(QSizeF size)
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
