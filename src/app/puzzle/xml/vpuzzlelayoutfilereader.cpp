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
#include "layoutliterals.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionconversionerror.h"

//---------------------------------------------------------------------------------------------------------------------
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
        ReadLayout(layout);
    }

    return !error();
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadLayout(VPuzzleLayout *layout)
{
    Q_ASSERT(isStartElement() && name() == ML::TagLayout);

    while (readNextStartElement())
    {
        if (name() == ML::TagProperties)
        {
            ReadProperties(layout);
        }
        else if (name() == ML::TagLayers)
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
    Q_ASSERT(isStartElement() && name() == ML::TagProperties);

    while (readNextStartElement())
    {
        qDebug() << name().toString();

        const QStringList tags = QStringList(
            {
                ML::TagUnit,
                ML::TagDescription,
                ML::TagSize,
                ML::TagMargin,
                ML::TagControl,
                ML::TagTiles
            });

        switch (tags.indexOf(name().toString()))
        {
        case 0:// unit
            qDebug("read unit");
            layout->SetUnit(StrToUnits(readElementText()));
            break;
        case 1:// description
        {
            qDebug("read description");
            QString description = readElementText();
            // TODO read the description info
            break;
        }
        case 2:// size
        {
            qDebug("read size");
            QSizeF size = ReadSize();
            layout->SetLayoutSize(size);
            readElementText();
            break;
        }
        case 3:// margin
        {
            qDebug("read margin");
            QMarginsF margins = ReadMargins();
            layout->SetLayoutMargins(margins);
            readElementText();
            break;
        }
        case 4:// control
        {
            qDebug("read control");
            QXmlStreamAttributes attribs = attributes();

            // attribs.value("followGrainLine"); // TODO

            layout->SetWarningSuperpositionOfPieces(ReadAttributeBool(attribs, ML::AttrWarningSuperposition, trueStr));
            layout->SetWarningPiecesOutOfBound(ReadAttributeBool(attribs, ML::AttrWarningOutOfBound, trueStr));
            layout->SetStickyEdges(ReadAttributeBool(attribs, ML::AttrStickyEdges, trueStr));

            layout->SetPiecesGap(ReadAttributeDouble(attribs, ML::AttrPiecesGap, QChar('0')));
            readElementText();
            break;
        }
        case 5:// tiles
            qDebug("read tiles");
            ReadTiles(layout);
            readElementText();
            break;
        default:
            // TODO error handling, we encountered a tag that isn't defined in the specification
            skipCurrentElement();
            break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayoutFileReader::ReadTiles(VPuzzleLayout *layout)
{
    Q_UNUSED(layout); // to be removed when used

    Q_ASSERT(isStartElement() && name() == ML::TagTiles);

//    QXmlStreamAttributes attribs = attributes();
    // attribs.value(ML::AttrVisible); // TODO
    // attribs.value(ML::AttrMatchingMarks); // TODO

    while (readNextStartElement())
    {
        if (name() == ML::TagSize)
        {
            QSizeF size = ReadSize();
            // TODO set layout tiled size
            Q_UNUSED(size);
            readElementText();
        }
        else if (name() == ML::TagMargin)
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
    Q_ASSERT(isStartElement() && name() == ML::TagLayers);

    while (readNextStartElement())
    {
        if (name() == ML::TagUnplacedPiecesLayer)
        {
            ReadLayer(layout->GetUnplacedPiecesLayer());
        }
        else if (name() == ML::TagLayer)
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
    Q_ASSERT(isStartElement() && (name() == ML::TagLayer || name() == ML::TagUnplacedPiecesLayer));

    QXmlStreamAttributes attribs = attributes();
    layer->SetName(ReadAttributeString(attribs, ML::AttrName, tr("Layer")));
    layer->SetIsVisible(ReadAttributeBool(attribs, ML::AttrVisible, trueStr));

    while (readNextStartElement())
    {
        if (name() == ML::TagPiece)
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
    Q_ASSERT(isStartElement() && name() == ML::TagPiece);

    QXmlStreamAttributes attribs = attributes();
    piece->SetName(ReadAttributeString(attribs, ML::AttrName, tr("Piece")));
    QString uuidStr = ReadAttributeString(attribs, ML::AttrID, QUuid().toString());// FIXME: is that correct to have a default value here?
    piece->SetUuid(QUuid(uuidStr));

    // TODO read the further attributes


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
    margins.setLeft(ReadAttributeDouble(attribs, ML::AttrLeft, QChar('0')));
    margins.setTop(ReadAttributeDouble(attribs, ML::AttrTop, QChar('0')));
    margins.setRight(ReadAttributeDouble(attribs, ML::AttrRight, QChar('0')));
    margins.setBottom(ReadAttributeDouble(attribs, ML::AttrBottom, QChar('0')));

    return margins;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayoutFileReader::ReadSize()
{
    QSizeF size = QSize();

    QXmlStreamAttributes attribs = attributes();
    size.setWidth(ReadAttributeDouble(attribs, ML::AttrWidth, QChar('0')));
    size.setHeight(ReadAttributeDouble(attribs, ML::AttrLength, QChar('0')));

    return size;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPuzzleLayoutFileReader::ReadAttributeString(const QXmlStreamAttributes &attribs, const QString &name,
                                                     const QString &defValue)
{
    const QString parameter = attribs.value(name).toString();
    if (parameter.isEmpty())
    {
        if (defValue.isEmpty())
        {
            throw VException(tr("Got empty attribute '%1'").arg(name));
        }
        else
        {
            return defValue;
        }
    }
    return parameter;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPuzzleLayoutFileReader::ReadAttributeEmptyString(const QXmlStreamAttributes &attribs, const QString &name)
{
    return attribs.value(name).toString();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayoutFileReader::ReadAttributeBool(const QXmlStreamAttributes &attribs, const QString &name,
                                                const QString &defValue)
{
    QString parametr;
    bool val = true;

    const QString message = QObject::tr("Can't convert toBool parameter");
    try
    {
        parametr = ReadAttributeString(attribs, name, defValue);

        const QStringList bools {trueStr, falseStr, QChar('1'), QChar('0')};
        switch (bools.indexOf(parametr))
        {
            case 0: // true
            case 2: // 1
                val = true;
                break;
            case 1: // false
            case 3: // 0
                val = false;
                break;
            default:// others
                throw VExceptionConversionError(message, name);
        }
    }
    catch (const VException &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }

    return val;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzleLayoutFileReader::ReadAttributeDouble(const QXmlStreamAttributes &attribs, const QString &name,
                                                   const QString &defValue)
{
    bool ok = false;
    qreal param = 0;

    const QString message = QObject::tr("Can't convert toDouble parameter");
    try
    {
        QString parametr = ReadAttributeString(attribs, name, defValue);
        param = parametr.replace(QChar(','), QChar('.')).toDouble(&ok);
        if (ok == false)
        {
            throw VExceptionConversionError(message, name);
        }
    }
    catch (const VException &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    return param;
}
