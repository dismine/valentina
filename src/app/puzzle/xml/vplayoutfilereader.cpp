/************************************************************************
 **
 **  @file   vplayoutfilereader.cpp
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
#include "vplayoutfilereader.h"
#include "vplayoutfilewriter.h"
#include "vplayoutliterals.h"
#include "vpsheet.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionconversionerror.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(MLReader, "mlReader")

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadFile(VPLayout *layout, QFile *file) -> bool
{
    setDevice(file);

    try
    {
        if (readNextStartElement())
        {
            ReadLayout(layout);
        }
    }
    catch(const VException &e)
    {
        raiseError(e.ErrorMessage());
    }

    return hasError();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadLayout(VPLayout *layout)
{
    AssertRootTag(ML::TagLayout);

    const QStringList tags
    {
        ML::TagProperties,     // 0
        ML::TagUnplacedPieces, // 1
        ML::TagSheets,         // 2
        ML::TagSize,           // 3
        ML::TagMargin          // 4
    };

    while (readNextStartElement())
    {
        switch (tags.indexOf(name().toString()))
        {
            case 0: // ML::TagProperties
                ReadProperties(layout);
                break;
            case 1: // ML::TagUnplacedPieces
                ReadUnplacedPieces(layout);
                break;
            case 2: // ML::TagSheets
                ReadSheets(layout);
                break;
            case 3: // size
                layout->LayoutSettings().SetSheetSize(ReadSize());
                break;
            case 4: // margin
                layout->LayoutSettings().SetSheetMargins(ReadMargins());
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadProperties(VPLayout *layout)
{
    AssertRootTag(ML::TagProperties);

    const QStringList tags
    {
        ML::TagUnit,        // 0
        ML::TagTitle,       // 1
        ML::TagDescription, // 2
        ML::TagControl,     // 3
        ML::TagTiles        // 4
    };

    while (readNextStartElement())
    {
        qDebug() << name().toString();

        switch (tags.indexOf(name().toString()))
        {
            case 0:// unit
                qDebug("read unit");
                layout->LayoutSettings().SetUnit(StrToUnits(readElementText()));
                break;
            case 1:// title
                qDebug("read title");
                layout->LayoutSettings().SetTitle(readElementText());
                break;
            case 2:// description
                qDebug("read description");
                layout->LayoutSettings().SetDescription(readElementText());
                break;
            case 3:// control
                qDebug("read control");
                ReadControl(layout);
                break;
            case 4:// tiles
                qDebug("read tiles");
                ReadTiles(layout);
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadControl(VPLayout *layout)
{
    AssertRootTag(ML::TagControl);

    QXmlStreamAttributes attribs = attributes();
    layout->LayoutSettings().SetWarningSuperpositionOfPieces(
                ReadAttributeBool(attribs, ML::AttrWarningSuperposition, trueStr));
    layout->LayoutSettings().SetWarningPiecesOutOfBound(ReadAttributeBool(attribs, ML::AttrWarningOutOfBound, trueStr));

    readElementText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadUnplacedPieces(VPLayout *layout)
{
    AssertRootTag(ML::TagUnplacedPieces);

    ReadPieces(layout);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadTiles(VPLayout *layout)
{
    AssertRootTag(ML::TagTiles);

    QXmlStreamAttributes attribs = attributes();
    layout->LayoutSettings().SetShowTiles(ReadAttributeBool(attribs, ML::AttrVisible, falseStr));
//    attribs.value(ML::AttrMatchingMarks); // TODO

    const QStringList tags
    {
        ML::TagSize,  // 0
        ML::TagMargin // 1
    };

    while (readNextStartElement())
    {
        switch (tags.indexOf(name().toString()))
        {
            case 0: // size
                layout->LayoutSettings().SetTilesSize(ReadSize());
                break;
            case 1: // margin
                layout->LayoutSettings().SetTilesMargins(ReadMargins());
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }

    readElementText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadSheets(VPLayout *layout)
{
    AssertRootTag(ML::TagSheets);

    while (readNextStartElement())
    {
        if (name() == ML::TagSheet)
        {
            ReadSheet(layout);
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadSheet(VPLayout *layout)
{
    AssertRootTag(ML::TagSheet);

    const QStringList tags
    {
        ML::TagName,   // 0
        ML::TagPieces  // 1
    };

    QScopedPointer<VPSheet> sheet (new VPSheet(layout));

    while (readNextStartElement())
    {
        switch (tags.indexOf(name().toString()))
        {
            case 0: // name
                sheet->SetName(readElementText());
                break;
            case 1: // pieces
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
                ReadPieces(layout, sheet.get());
#else
                ReadPieces(layout, sheet.data());
#endif
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }

    readElementText();

    layout->AddSheet(sheet.take());
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadPieces(VPLayout *layout, VPSheet *sheet)
{
    while (readNextStartElement())
    {
        if (name() == ML::TagPiece)
        {
            QScopedPointer<VPPiece>piece(new VPPiece());
            ReadPiece(piece.data());
            piece->SetSheet(sheet);
            layout->AddPiece(piece.take());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadPiece(VPPiece *piece)
{
    AssertRootTag(ML::TagPiece);

    QXmlStreamAttributes attribs = attributes();
    piece->SetName(ReadAttributeString(attribs, ML::AttrName, tr("Piece")));

    QString uuidStr = ReadAttributeString(attribs, ML::AttrID, QUuid::createUuid().toString());
    piece->SetUUID(QUuid(uuidStr));

    bool showSeamline = ReadAttributeBool(attribs, ML::AttrShowSeamline, trueStr);
    piece->SetShowSeamLine(showSeamline);

    bool pieceMirrored = ReadAttributeBool(attribs, ML::AttrMirrored, falseStr);
    piece->SetPieceMirrored(pieceMirrored);
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
QMarginsF VPLayoutFileReader::ReadMargins()
{
    QMarginsF margins = QMarginsF();

    QXmlStreamAttributes attribs = attributes();
    margins.setLeft(ReadAttributeDouble(attribs, ML::AttrLeft, QChar('0')));
    margins.setTop(ReadAttributeDouble(attribs, ML::AttrTop, QChar('0')));
    margins.setRight(ReadAttributeDouble(attribs, ML::AttrRight, QChar('0')));
    margins.setBottom(ReadAttributeDouble(attribs, ML::AttrBottom, QChar('0')));

    readElementText();

    return margins;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPLayoutFileReader::ReadSize()
{
    QSizeF size;

    QXmlStreamAttributes attribs = attributes();
    size.setWidth(ReadAttributeDouble(attribs, ML::AttrWidth, QChar('0')));
    size.setHeight(ReadAttributeDouble(attribs, ML::AttrLength, QChar('0')));

    readElementText();

    return size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::AssertRootTag(const QString &tag) const
{
    if (not (isStartElement() && name() == tag))
    {
        throw VException(tr("Unexpected tag %1 in line %2").arg(name().toString()).arg(lineNumber()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VPLayoutFileReader::ReadAttributeString(const QXmlStreamAttributes &attribs, const QString &name,
                                                const QString &defValue)
{
    const QString parameter = attribs.value(name).toString();
    if (parameter.isEmpty())
    {
        if (defValue.isEmpty())
        {
            throw VException(tr("Got empty attribute '%1'").arg(name));
        }
        return defValue;
    }
    return parameter;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPLayoutFileReader::ReadAttributeEmptyString(const QXmlStreamAttributes &attribs, const QString &name)
{
    return attribs.value(name).toString();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPLayoutFileReader::ReadAttributeBool(const QXmlStreamAttributes &attribs, const QString &name,
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
qreal VPLayoutFileReader::ReadAttributeDouble(const QXmlStreamAttributes &attribs, const QString &name,
                                              const QString &defValue)
{
    bool ok = false;
    qreal param = 0;

    const QString message = QObject::tr("Can't convert toDouble parameter");
    try
    {
        QString parametr = ReadAttributeString(attribs, name, defValue);
        param = parametr.replace(QChar(','), QChar('.')).toDouble(&ok);
        if (not ok)
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
