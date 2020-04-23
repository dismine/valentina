/************************************************************************
 **
 **  @file   vpuzzlelayoutfilereader.cpp
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

#include <QXmlStreamAttributes>
#include "vpuzzlelayoutfilereader.h"
#include "vpuzzlelayoutfilewriter.h"

VPuzzleLayoutFileReader::VPuzzleLayoutFileReader()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayoutFileReader::~VPuzzleLayoutFileReader()
{
    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayoutFileReader::ReadFile(VPuzzleLayout *layout, QFile *file)
{
    setDevice(file);

    if (readNextStartElement())
    {

        // TODO extend the handling
        // if it doesn't start with layout, error
        // if it starts with version > than current version, error

        if (name() == QString("layout"))
        {
            QString versionStr = attributes().value(QString("version")).toString();
            QStringList versionStrParts = versionStr.split('.');
            m_layoutFormatVersion = FORMAT_VERSION(versionStrParts.at(0).toInt(),versionStrParts.at(1).toInt(),versionStrParts.at(2).toInt());

            if(VPuzzleLayoutFileWriter::LayoutFileFormatVersion >= m_layoutFormatVersion)
            {
                ReadLayout(layout);
            }
            else
            {
                // TODO better error handling
                raiseError(QObject::tr("You're trying to open a layout that was created with a newer version of puzzle"));
            }
        }
        else
        {
            raiseError(QObject::tr("Wrong file structure"));
        }
    }

    return !error();
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadLayout(VPuzzleLayout *layout)
{
    Q_ASSERT(isStartElement() && name() == QString("layout"));

    while (readNextStartElement())
    {
        if (name() == QString("properties"))
        {
            ReadProperties(layout);
        }
        else if (name() == QString("layers"))
        {
            ReadLayers(layout);
        }
        else
        {
            // TODO error handling, we encountered a tag that isn't defined in the specification
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadProperties(VPuzzleLayout *layout)
{
    Q_ASSERT(isStartElement() && name() == QString("properties"));

    while (readNextStartElement())
    {
        qDebug(name().toString().toLatin1());

        if (name() == QString("unit"))
        {
            qDebug("read unit");
            QString unit = readElementText();
            if(unit == UnitsToStr(Unit::Inch))
            {
                layout->SetUnit(Unit::Inch);
            }
            else if(unit == UnitsToStr(Unit::Mm))
            {
                layout->SetUnit(Unit::Cm);
            }
            else // no condition here to have a default value just in case
            {
                layout->SetUnit(Unit::Cm);
            }
        }
        else if (name() == QString("description"))
        {
            qDebug("read description");
            QString description = readElementText();
            // TODO read the description info

        }
        else if (name() == QString("size"))
        {
            qDebug("read size");
            QSizeF size = ReadSize();
            layout->SetLayoutSize(size);
            readElementText();
        }
        else if (name() == QString("margin"))
        {
            qDebug("read margin");
            QMarginsF margins = ReadMargins();
            layout->SetLayoutMargins(margins);
            readElementText();
        }
        else if (name() == QString("control"))
        {
            qDebug("read control");
            QXmlStreamAttributes attribs = attributes();

            // attribs.value("followGrainLine"); // TODO

            layout->SetWarningSuperpositionOfPieces(attribs.value("warningSuperposition") == "true");
            layout->SetWarningPiecesOutOfBound(attribs.value("warningOutOfBound") == "true");
            layout->SetStickyEdges(attribs.value("stickyEdges") == "true");

            layout->SetPiecesGap(attribs.value("piecesGap").toDouble());
            readElementText();
        }
        else if (name() == QString("tiles"))
        {
            qDebug("read tiles");
            ReadTiles(layout);
            readElementText();
        }
        else
        {
            // TODO error handling, we encountered a tag that isn't defined in the specification
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadTiles(VPuzzleLayout *layout)
{
    Q_UNUSED(layout); // to be removed when used

    Q_ASSERT(isStartElement() && name() == QString("tiles"));

    QXmlStreamAttributes attribs = attributes();
    // attribs.value("visible"); // TODO
    // attribs.value("matchingMarks"); // TODO

    while (readNextStartElement())
    {
        if (name() == QString("size"))
        {
            QSizeF size = ReadSize();
            // TODO set layout tiled size
            Q_UNUSED(size);
            readElementText();
        }
        else if (name() == QString("margin"))
        {
            QMarginsF margins = ReadMargins();
            // TODO set layout tiled margins
            Q_UNUSED(margins);
            readElementText();
        }
        else
        {
            // TODO error handling, we encountered a tag that isn't defined in the specification
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadLayers(VPuzzleLayout *layout)
{
    Q_ASSERT(isStartElement() && name() == QString("layers"));

    while (readNextStartElement())
    {
        if (name() == QString("unplacedPiecesLayer"))
        {
            ReadLayer(layout->GetUnplacedPiecesLayer());
        }
        else if (name() == QString("layer"))
        {
            VPuzzleLayer *layer = layout->AddLayer();
            ReadLayer(layer);
        }
        else
        {
            // TODO error handling, we encountered a tag that isn't defined in the specification
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadLayer(VPuzzleLayer *layer)
{
    Q_ASSERT(isStartElement() && name() == QString("layer"));

    QXmlStreamAttributes attribs = attributes();
    layer->SetName(attribs.value("name").toString());
    layer->SetIsVisible(attribs.value("visible") == "true");

    while (readNextStartElement())
    {
        if (name() == QString("piece"))
        {
            VPuzzlePiece *piece = new VPuzzlePiece();
            ReadPiece(piece);
            layer->AddPiece(piece);
        }
        else
        {
            // TODO error handling, we encountered a tag that isn't defined in the specification
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadPiece(VPuzzlePiece *piece)
{
    Q_UNUSED(piece);
    Q_ASSERT(isStartElement() && name() == QString("piece"));

    // TODO read the attributes

    while (readNextStartElement())
    {
        if (name() == QString("..."))
        {
            // TODO
             readElementText();
        }
        else
        {
            // TODO error handling, we encountered a tag that isn't defined in the specification
            skipCurrentElement();
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPuzzleLayoutFileReader::ReadMargins()
{
    QMarginsF margins = QMarginsF();

    QXmlStreamAttributes attribs = attributes();
    margins.setLeft(attribs.value("left").toDouble());
    margins.setTop(attribs.value("top").toDouble());
    margins.setRight(attribs.value("right").toDouble());
    margins.setBottom(attribs.value("bottom").toDouble());

    return margins;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayoutFileReader::ReadSize()
{
    QSizeF size = QSize();

    QXmlStreamAttributes attribs = attributes();
    size.setWidth(attribs.value("width").toDouble());
    size.setHeight(attribs.value("length").toDouble());

    return size;
}
