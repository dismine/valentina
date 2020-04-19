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
    writeStartElement("layout");
    writeAttribute("version", LayoutFileFormatVersionStr);

    WriteProperties(layout);
    WriteLayers(layout);

    writeEndElement(); //layout
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteProperties(VPuzzleLayout *layout)
{
    writeStartElement("properties");

    writeTextElement("unit", UnitsToStr(layout->getUnit()));

    writeTextElement("description", ""); // TODO : define the value in layout

    WriteSize(layout->GetLayoutSize());

    WriteMargins(layout->GetLayoutMargins());

    writeStartElement("control");
    writeAttribute("followGrainLine", "no"); // TODO / Fixme get the right value
    writeAttribute("warningSuperposition", QString(layout->GetWarningSuperpositionOfPieces() ? "true" : "false"));
    writeAttribute("warningOutOfBound", QString(layout->GetWarningPiecesOutOfBound() ? "true" : "false"));
    writeAttribute("stickyEdges", QString(layout->GetStickyEdges() ? "true" : "false"));
    writeAttribute("piecesGap", QString::number(layout->GetPiecesGap()));
    writeEndElement(); // control

    // WriteTiles(layout);  TODO: when tile functionality implemented, then uncomment this line

    writeEndElement(); // properties
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteTiles(VPuzzleLayout *layout)
{
    Q_UNUSED(layout); // to be removed

   writeStartElement("tiles");
   writeAttribute("visible", QString(false ? "true" : "false")); // TODO / Fixme get the right value
   writeAttribute("matchingMarks", "standard"); // TODO / Fixme get the right value

   QSizeF size = QSizeF(); // TODO get the right size
   WriteSize(size);

   QMarginsF margins = QMarginsF(); // TODO get the right margins
   WriteMargins(margins);

   writeEndElement(); // tiles
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteLayers(VPuzzleLayout *layout)
{
   writeStartElement("layers");

   WriteLayer(layout->GetUnplacedPiecesLayer(), "unplacedPieces");

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
    WriteLayer(layer, "layer");
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteLayer(VPuzzleLayer *layer, const QString &tagName)
{
    writeStartElement(tagName); // layer
    writeAttribute("name", layer->GetName());
    writeAttribute("visible", QString(layer->GetIsVisible()? "true" : "false"));
    //  TODO selected info. Not sure how it's saved yet
    //writeAttribute("selected", QString(layer->GetIsSelected()? "true" : "false"));


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

    writeStartElement("piece");
    writeAttribute("id", "uuid1"); // TODO / Fixme get the right value
    writeAttribute("name", "Piece name"); // TODO / Fixme get the right value
    writeAttribute("mirrored", "false"); // TODO / Fixme get the right value
    writeAttribute("transform", "string representation of the transformation"); // TODO / Fixme get the right value

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
void VPuzzleLayoutFileWriter::WriteMargins(QMarginsF margins)
{
    writeStartElement("margin");
    writeAttribute("left", QString::number(margins.left()));
    writeAttribute("top", QString::number(margins.top()));
    writeAttribute("right", QString::number(margins.right()));
    writeAttribute("bottom", QString::number(margins.bottom()));
    writeEndElement(); // margin
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileWriter::WriteSize(QSizeF size)
{
    // maybe not necessary to test this, the writer should "stupidly write", the application should take care of these tests
    qreal width = size.width();
    if(width < 0) {
        width = 0;
    }

    qreal length = size.height();
    if(length < 0) {
        length = 0;
    }

    writeStartElement("size");
    writeAttribute("width", QString::number(width));
    writeAttribute("length", QString::number(length));
    writeEndElement(); // size
}
