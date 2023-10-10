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

#include "vplayoutfilereader.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/floatItemData/floatitemdef.h"
#include "../vwidgets/vpiecegrainline.h"
#include "vplayoutliterals.h"
#include <QFont>
#include <QXmlStreamAttributes>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(MLReader, "mlReader") // NOLINT

QT_WARNING_POP

namespace
{

//---------------------------------------------------------------------------------------------------------------------
auto StringToTransfrom(const QString &matrix) -> QTransform
{
    QStringList elements = matrix.split(ML::groupSep);
    if (elements.count() == 9)
    {
        qreal m11 = elements.at(0).toDouble();
        qreal m12 = elements.at(1).toDouble();
        qreal m13 = elements.at(2).toDouble();
        qreal m21 = elements.at(3).toDouble();
        qreal m22 = elements.at(4).toDouble();
        qreal m23 = elements.at(5).toDouble();
        qreal m31 = elements.at(6).toDouble();
        qreal m32 = elements.at(7).toDouble();
        qreal m33 = elements.at(8).toDouble();
        return {m11, m12, m13, m21, m22, m23, m31, m32, m33};
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToPoint(const QString &point) -> QPointF
{
    QStringList coordinates = point.split(ML::coordintatesSep);
    if (coordinates.count() == 2)
    {
        return {coordinates.at(0).toDouble(), coordinates.at(1).toDouble()};
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToPath(const QString &path) -> QVector<QPointF>
{
    QVector<QPointF> p;
    if (path.isEmpty())
    {
        return p;
    }

    QStringList points = path.split(ML::pointsSep);
    p.reserve(points.size());
    for (const auto &point : points)
    {
        p.append(StringToPoint(point));
    }

    return p;
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToGrainlineArrowDirrection(const QString &dirrection) -> GrainlineArrowDirection
{
    const QStringList arrows{
        ML::twoWaysUpDownStr,          // 0
        ML::oneWayUpStr,               // 1
        ML::oneWayDownStr,             // 2
        ML::fourWaysStr,               // 3
        ML::twoWaysUpLeftStr,          // 4
        ML::twoWaysUpRightStr,         // 5
        ML::twoWaysDownLeftStr,        // 6
        ML::twoWaysDownRightStr,       // 7
        ML::threeWaysUpDownLeftStr,    // 8
        ML::threeWaysUpDownRightStr,   // 9
        ML::threeWaysUpLeftRightStr,   // 10
        ML::threeWaysDownLeftRightStr, // 11
    };

    GrainlineArrowDirection arrowDirection = GrainlineArrowDirection::twoWaysUpDown;
    switch (arrows.indexOf(dirrection))
    {
        case 1: // oneWayUp
            arrowDirection = GrainlineArrowDirection::oneWayUp;
            break;
        case 2: // oneWayDown
            arrowDirection = GrainlineArrowDirection::oneWayDown;
            break;
        case 3: // fourWays
            arrowDirection = GrainlineArrowDirection::fourWays;
            break;
        case 4: // twoWaysUpLeft
            arrowDirection = GrainlineArrowDirection::twoWaysUpLeft;
            break;
        case 5: // twoWaysUpRight
            arrowDirection = GrainlineArrowDirection::twoWaysUpRight;
            break;
        case 6: // twoWaysDownLeft
            arrowDirection = GrainlineArrowDirection::twoWaysDownLeft;
            break;
        case 7: // twoWaysDownRight
            arrowDirection = GrainlineArrowDirection::twoWaysDownRight;
            break;
        case 8: // threeWaysUpDownLeft
            arrowDirection = GrainlineArrowDirection::threeWaysUpDownLeft;
            break;
        case 9: // threeWaysUpDownRight
            arrowDirection = GrainlineArrowDirection::threeWaysUpDownRight;
            break;
        case 10: // threeWaysUpLeftRight
            arrowDirection = GrainlineArrowDirection::threeWaysUpLeftRight;
            break;
        case 11: // threeWaysDownLeftRight
            arrowDirection = GrainlineArrowDirection::threeWaysDownLeftRight;
            break;
        case 0: // twoWaysUpDown
        default:
            arrowDirection = GrainlineArrowDirection::twoWaysUpDown;
            break;
    }
    return arrowDirection;
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToLine(const QString &string) -> QLineF
{
    QStringList points = string.split(ML::groupSep);
    if (points.count() == 2)
    {
        return {StringToPoint(points.at(0)), StringToPoint(points.at(1))};
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToLines(const QString &string) -> QVector<QLineF>
{
    QStringList lines = string.split(ML::itemsSep);
    QVector<QLineF> path;
    path.reserve(lines.size());

    for (const auto &line : lines)
    {
        QLineF l = StringToLine(line);
        if (not l.isNull())
        {
            path.append(StringToLine(line));
        }
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToRect(const QString &string) -> QRectF
{
    QStringList points = string.split(ML::groupSep);
    if (points.count() == 4)
    {
        return {points.at(0).toDouble(), points.at(1).toDouble(), points.at(2).toDouble(), points.at(3).toDouble()};
    }

    return {};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadFile(const VPLayoutPtr &layout, QFile *file) -> bool
{
    setDevice(file);

    try
    {
        if (readNextStartElement())
        {
            ReadLayout(layout);
        }
    }
    catch (const VException &e)
    {
        raiseError(e.ErrorMessage());
    }

    return hasError();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadLayout(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagLayout);

    const QStringList tags{
        ML::TagProperties,     // 0
        ML::TagUnplacedPieces, // 1
        ML::TagSheets          // 2
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
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadProperties(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagProperties);

    const QStringList tags{
        ML::TagUnit,        // 0
        ML::TagTitle,       // 1
        ML::TagDescription, // 2
        ML::TagControl,     // 3
        ML::TagTiles,       // 4
        ML::TagScale,       // 5
        ML::TagWatermark    // 6
    };

    while (readNextStartElement())
    {
        qDebug() << name().toString();

        switch (tags.indexOf(name().toString()))
        {
            case 0: // unit
                qDebug("read unit");
                layout->LayoutSettings().SetUnit(StrToUnits(readElementText()));
                break;
            case 1: // title
                qDebug("read title");
                layout->LayoutSettings().SetTitle(readElementText());
                break;
            case 2: // description
                qDebug("read description");
                layout->LayoutSettings().SetDescription(readElementText());
                break;
            case 3: // control
                qDebug("read control");
                ReadControl(layout);
                break;
            case 4: // tiles
                qDebug("read tiles");
                ReadTiles(layout);
                break;
            case 5: // scale
                qDebug("read scale");
                ReadScale(layout);
                break;
            case 6: // watermark
                qDebug("read watermark");
                ReadWatermark(layout);
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadControl(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagControl);

    QXmlStreamAttributes attribs = attributes();
    layout->LayoutSettings().SetWarningSuperpositionOfPieces(
        ReadAttributeBool(attribs, ML::AttrWarningSuperposition, trueStr));
    layout->LayoutSettings().SetWarningPiecesOutOfBound(ReadAttributeBool(attribs, ML::AttrWarningOutOfBound, trueStr));
    layout->LayoutSettings().SetStickyEdges(ReadAttributeBool(attribs, ML::AttrStickyEdges, trueStr));
    layout->LayoutSettings().SetPiecesGap(qMax(ReadAttributeDouble(attribs, ML::AttrPiecesGap, QChar('0')), 0.0));
    layout->LayoutSettings().SetFollowGrainline(ReadAttributeBool(attribs, ML::AttrFollowGrainline, falseStr));

    readElementText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadUnplacedPieces(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagUnplacedPieces);

    ReadPieces(layout);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadTiles(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagTiles);

    QXmlStreamAttributes attribs = attributes();
    layout->LayoutSettings().SetShowTiles(ReadAttributeBool(attribs, ML::AttrVisible, falseStr));
    layout->LayoutSettings().SetPrintTilesScheme(ReadAttributeBool(attribs, ML::AttrPrintScheme, falseStr));
    layout->LayoutSettings().SetShowTileNumber(ReadAttributeBool(attribs, ML::AttrTileNumber, falseStr));
    //    attribs.value(ML::AttrMatchingMarks); // TODO

    const QStringList tags{
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
                ReadLayoutMargins(layout);
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
void VPLayoutFileReader::ReadScale(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagScale);

    QXmlStreamAttributes attribs = attributes();
    layout->LayoutSettings().SetHorizontalScale(ReadAttributeDouble(attribs, ML::AttrXScale, QChar('1')));
    layout->LayoutSettings().SetVerticalScale(ReadAttributeDouble(attribs, ML::AttrYScale, QChar('1')));

    readElementText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadSheets(const VPLayoutPtr &layout)
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
void VPLayoutFileReader::ReadSheet(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagSheet);

    VPSheetPtr sheet(new VPSheet(layout));

    QXmlStreamAttributes attribs = attributes();
    sheet->SetGrainlineType(StrToGrainlineType(ReadAttributeEmptyString(attribs, ML::AttrGrainlineType)));

    const QStringList tags{
        ML::TagName,   // 0
        ML::TagSize,   // 1
        ML::TagMargin, // 2
        ML::TagPieces  // 3
    };

    while (readNextStartElement())
    {
        switch (tags.indexOf(name().toString()))
        {
            case 0: // name
                sheet->SetName(readElementText());
                break;
            case 1: // size
                sheet->SetSheetSize(ReadSize());
                break;
            case 2: // margin
                ReadSheetMargins(sheet);
                break;
            case 3: // pieces
                ReadPieces(layout, sheet);
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }

    readElementText();

    layout->AddSheet(sheet);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadPieces(const VPLayoutPtr &layout, const VPSheetPtr &sheet)
{
    while (readNextStartElement())
    {
        if (name() == ML::TagPiece)
        {
            VPPiecePtr piece(new VPPiece());
            ReadPiece(piece);

            QString error;
            if (not piece->IsValid(error))
            {
                throw VException(tr("Piece %1 invalid. %2").arg(piece->GetName(), error));
            }

            piece->SetSheet(sheet);
            VPLayout::AddPiece(layout, piece);
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadPiece(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagPiece);

    QXmlStreamAttributes attribs = attributes();
    piece->SetName(ReadAttributeString(attribs, ML::AttrName, tr("Piece")));

    QString uuidStr = ReadAttributeString(attribs, ML::AttrUID, QUuid::createUuid().toString());
    piece->SetUUID(QUuid(uuidStr));

    piece->SetGradationId(ReadAttributeEmptyString(attribs, ML::AttrGradationLabel));
    piece->SetCopyNumber(static_cast<quint16>(ReadAttributeUInt(attribs, ML::AttrCopyNumber, QChar('1'))));
    piece->SetHideMainPath(not ReadAttributeBool(attribs, ML::AttrShowSeamline, trueStr));
    piece->SetXScale(ReadAttributeDouble(attribs, ML::AttrXScale, QChar('1')));
    piece->SetYScale(ReadAttributeDouble(attribs, ML::AttrYScale, QChar('1')));
    piece->SetZValue(ReadAttributeDouble(attribs, ML::AttrZValue, QChar('1')));
    piece->SetMirror(ReadAttributeBool(attribs, ML::AttrMirrored, falseStr));
    piece->SetForbidFlipping(ReadAttributeBool(attribs, ML::AttrForbidFlipping, falseStr));
    piece->SetForceFlipping(ReadAttributeBool(attribs, ML::AttrForceFlipping, falseStr));
    piece->SetFollowGrainline(ReadAttributeBool(attribs, ML::AttrFollowGrainline, falseStr));
    piece->SetSewLineOnDrawing(ReadAttributeBool(attribs, ML::AttrSewLineOnDrawing, falseStr));
    piece->SetMatrix(StringToTransfrom(ReadAttributeEmptyString(attribs, ML::AttrTransform)));

    const QStringList tags{
        ML::TagSeamLine,      // 0
        ML::TagSeamAllowance, // 1
        ML::TagGrainline,     // 2
        ML::TagNotches,       // 3
        ML::TagInternalPaths, // 4
        ML::TagMarkers,       // 5
        ML::TagLabels         // 6
    };

    while (readNextStartElement())
    {
        switch (tags.indexOf(name().toString()))
        {
            case 0: // seam line
                ReadSeamLine(piece);
                break;
            case 1: // seam allowance
                ReadSeamAllowance(piece);
                break;
            case 2: // grainline
                ReadGrainline(piece);
                break;
            case 3: // notches
                ReadNotches(piece);
                break;
            case 4: // internal paths
                ReadInternalPaths(piece);
                break;
            case 5: // markers
                ReadMarkers(piece);
                break;
            case 6: // labels
                ReadLabels(piece);
                break;
            default:
                qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
                skipCurrentElement();
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadLayoutPoint() -> VLayoutPoint
{
    AssertRootTag(ML::TagPoint);

    VLayoutPoint point;

    QXmlStreamAttributes attribs = attributes();
    point.setX(ReadAttributeDouble(attribs, ML::AttrX, QChar('0')));
    point.setY(ReadAttributeDouble(attribs, ML::AttrY, QChar('0')));
    point.SetTurnPoint(ReadAttributeBool(attribs, ML::AttrTurnPoint, falseStr));
    point.SetCurvePoint(ReadAttributeBool(attribs, ML::AttrCurvePoint, falseStr));

    readElementText();

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadLayoutPoints() -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> points;

    while (readNextStartElement())
    {
        if (name() == ML::TagPoint)
        {
            points.append(ReadLayoutPoint());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadSeamLine(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagSeamLine);
    piece->SetContourPoints(ReadLayoutPoints());
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadSeamAllowance(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagSeamAllowance);

    QXmlStreamAttributes attribs = attributes();
    bool enabled = ReadAttributeBool(attribs, ML::AttrEnabled, falseStr);
    piece->SetSeamAllowance(enabled);

    bool builtIn = ReadAttributeBool(attribs, ML::AttrBuiltIn, falseStr);
    piece->SetSeamAllowanceBuiltIn(builtIn);

    QVector<VLayoutPoint> path = ReadLayoutPoints();

    if (enabled && not builtIn)
    {
        if (path.isEmpty())
        {
            throw VException(tr("Error in line %1. Seam allowance is empty.").arg(lineNumber()));
        }
        piece->SetSeamAllowancePoints(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadGrainline(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagGrainline);

    VPieceGrainline grainline;

    QXmlStreamAttributes attribs = attributes();
    bool enabled = ReadAttributeBool(attribs, ML::AttrEnabled, falseStr);
    grainline.SetEnabled(enabled);
    QLineF mainLine = StringToLine(readElementText());

    if (enabled)
    {
        QString arrowDirection = ReadAttributeEmptyString(attribs, ML::AttrArrowDirection);
        grainline.SetArrowType(StringToGrainlineArrowDirrection(arrowDirection));

        if (mainLine.isNull())
        {
            throw VException(tr("Error in line %1. Grainline main line is empty.").arg(lineNumber()));
        }
        grainline.SetMainLine(mainLine);
    }

    piece->SetGrainline(grainline);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadNotches(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagNotches);

    QVector<VLayoutPassmark> passmarks;

    while (readNextStartElement())
    {
        if (name() == ML::TagNotch)
        {
            passmarks.append(ReadNotch());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }

    piece->SetPassmarks(passmarks);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadNotch() -> VLayoutPassmark
{
    AssertRootTag(ML::TagNotch);

    QXmlStreamAttributes attribs = attributes();

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutPassmark passmark;
    passmark.isBuiltIn = ReadAttributeBool(attribs, ML::AttrBuiltIn, falseStr);
    passmark.baseLine = StringToLine(ReadAttributeEmptyString(attribs, ML::AttrBaseLine));
    passmark.lines = StringToLines(ReadAttributeEmptyString(attribs, ML::AttrPath));
    passmark.isClockwiseOpening = ReadAttributeBool(attribs, ML::AttrClockwiseOpening, falseStr);

    QString defaultType = QString::number(static_cast<int>(PassmarkLineType::OneLine));
    passmark.type = static_cast<PassmarkLineType>(ReadAttributeUInt(attribs, ML::AttrType, defaultType));

    QT_WARNING_POP

    readElementText();

    return passmark;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadInternalPaths(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagInternalPaths);

    QVector<VLayoutPiecePath> internalPaths;

    while (readNextStartElement())
    {
        if (name() == ML::TagInternalPath)
        {
            internalPaths.append(ReadInternalPath());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }

    piece->SetInternalPaths(internalPaths);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadInternalPath() -> VLayoutPiecePath
{
    AssertRootTag(ML::TagInternalPath);

    VLayoutPiecePath path;

    QXmlStreamAttributes attribs = attributes();
    path.SetCutPath(ReadAttributeBool(attribs, ML::AttrCut, falseStr));
    path.SetPenStyle(LineStyleToPenStyle(ReadAttributeString(attribs, ML::AttrPenStyle, TypeLineLine)));

    QVector<VLayoutPoint> shape = ReadLayoutPoints();
    if (shape.isEmpty())
    {
        throw VException(tr("Error in line %1. Internal path shape is empty.").arg(lineNumber()));
    }

    path.SetPoints(shape);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadMarkers(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagMarkers);

    QVector<VLayoutPlaceLabel> markers;

    while (readNextStartElement())
    {
        if (name() == ML::TagMarker)
        {
            markers.append(ReadMarker());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }

    piece->SetPlaceLabels(markers);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadMarker() -> VLayoutPlaceLabel
{
    AssertRootTag(ML::TagMarker);

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutPlaceLabel marker;

    QXmlStreamAttributes attribs = attributes();

    QString matrix = ReadAttributeEmptyString(attribs, ML::AttrTransform);
    marker.SetRotationMatrix(StringToTransfrom(matrix));

    marker.SetType(static_cast<PlaceLabelType>(ReadAttributeUInt(attribs, ML::AttrType, QChar('0'))));
    marker.SetCenter(StringToPoint(ReadAttributeEmptyString(attribs, ML::AttrCenter)));
    marker.SetBox(StringToRect(ReadAttributeEmptyString(attribs, ML::AttrBox)));

    // cppcheck-suppress unknownMacro
    QT_WARNING_POP

    readElementText();

    return marker;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadLabels(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagLabels);

    while (readNextStartElement())
    {
        if (name() == ML::TagPieceLabel)
        {
            ReadPieceLabel(piece);
        }
        else if (name() == ML::TagPatternLabel)
        {
            ReadPatternLabel(piece);
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadPieceLabel(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagPieceLabel);

    QXmlStreamAttributes attribs = attributes();
    piece->SetPieceLabelRect(StringToPath(ReadAttributeEmptyString(attribs, ML::AttrShape)));

    while (readNextStartElement())
    {
        if (name() == ML::TagLines)
        {
            piece->SetPieceLabelData(ReadLabelLines());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadPatternLabel(const VPPiecePtr &piece)
{
    AssertRootTag(ML::TagPatternLabel);

    QXmlStreamAttributes attribs = attributes();
    piece->SetPatternLabelRect(StringToPath(ReadAttributeEmptyString(attribs, ML::AttrShape)));

    while (readNextStartElement())
    {
        if (name() == ML::TagLines)
        {
            piece->SetPatternLabelData(ReadLabelLines());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadLabelLines() -> VTextManager
{
    AssertRootTag(ML::TagLines);

    VTextManager text;
    QVector<TextLine> lines;

    QXmlStreamAttributes attribs = attributes();
    text.SetFont(FontFromString(ReadAttributeEmptyString(attribs, ML::AttrFont)));

    QStringList svgFontData = ReadAttributeEmptyString(attribs, ML::AttrSVGFont).split(',');
    if (!svgFontData.isEmpty())
    {
        text.SetSVGFontFamily(svgFontData.constFirst());

        if (svgFontData.size() > 1)
        {
            text.SetSVGFontPointSize(svgFontData.at(1).toInt());
        }
    }

    while (readNextStartElement())
    {
        if (name() == ML::TagLine)
        {
            lines.append(ReadLabelLine());
        }
        else
        {
            qCDebug(MLReader, "Ignoring tag %s", qUtf8Printable(name().toString()));
            skipCurrentElement();
        }
    }

    text.SetAllSourceLines(lines);

    return text;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadLabelLine() -> TextLine
{
    AssertRootTag(ML::TagLine);

    TextLine line;

    QXmlStreamAttributes attribs = attributes();

    line.m_iFontSize =
        ReadAttributeInt(attribs, ML::AttrFontSize, QString::number(VCommonSettings::MinPieceLabelFontPointSize()));
    line.m_bold = ReadAttributeBool(attribs, ML::AttrBold, falseStr);
    line.m_italic = ReadAttributeBool(attribs, ML::AttrItalic, falseStr);
    int alignment = ReadAttributeInt(attribs, ML::AttrAlignment, QString::number(static_cast<int>(Qt::AlignCenter)));
    line.m_eAlign = static_cast<Qt::Alignment>(alignment);
    line.m_qsText = readElementText();

    return line;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadWatermark(const VPLayoutPtr &layout)
{
    AssertRootTag(ML::TagWatermark);

    QXmlStreamAttributes attribs = attributes();
    layout->LayoutSettings().SetShowWatermark(ReadAttributeBool(attribs, ML::AttrShowPreview, falseStr));
    layout->LayoutSettings().SetWatermarkPath(readElementText());
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadLayoutMargins(const VPLayoutPtr &layout)
{
    QXmlStreamAttributes attribs = attributes();

    QMarginsF margins = QMarginsF();
    margins.setLeft(ReadAttributeDouble(attribs, ML::AttrLeft, QChar('0')));
    margins.setTop(ReadAttributeDouble(attribs, ML::AttrTop, QChar('0')));
    margins.setRight(ReadAttributeDouble(attribs, ML::AttrRight, QChar('0')));
    margins.setBottom(ReadAttributeDouble(attribs, ML::AttrBottom, QChar('0')));
    layout->LayoutSettings().SetTilesMargins(margins);

    layout->LayoutSettings().SetIgnoreTilesMargins(ReadAttributeBool(attribs, ML::AttrIgnoreMargins, falseStr));

    readElementText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileReader::ReadSheetMargins(const VPSheetPtr &sheet)
{
    QXmlStreamAttributes attribs = attributes();

    QMarginsF margins = QMarginsF();
    margins.setLeft(ReadAttributeDouble(attribs, ML::AttrLeft, QChar('0')));
    margins.setTop(ReadAttributeDouble(attribs, ML::AttrTop, QChar('0')));
    margins.setRight(ReadAttributeDouble(attribs, ML::AttrRight, QChar('0')));
    margins.setBottom(ReadAttributeDouble(attribs, ML::AttrBottom, QChar('0')));
    sheet->SetSheetMargins(margins);

    sheet->SetIgnoreMargins(ReadAttributeBool(attribs, ML::AttrIgnoreMargins, falseStr));

    readElementText();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadSize() -> QSizeF
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
    if (not(isStartElement() && name() == tag))
    {
        throw VException(tr("Unexpected tag %1 in line %2").arg(name().toString()).arg(lineNumber()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadAttributeString(const QXmlStreamAttributes &attribs, const QString &name,
                                             const QString &defValue) -> QString
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
auto VPLayoutFileReader::ReadAttributeEmptyString(const QXmlStreamAttributes &attribs, const QString &name) -> QString
{
    return attribs.value(name).toString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadAttributeBool(const QXmlStreamAttributes &attribs, const QString &name,
                                           const QString &defValue) -> bool
{
    QString parametr;
    bool val = true;

    const QString message = QObject::tr("Can't convert toBool parameter");
    try
    {
        parametr = ReadAttributeString(attribs, name, defValue);

        const QStringList bools{trueStr, falseStr, QChar('1'), QChar('0')};
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
            default: // others
                throw VExceptionConversionError(message, name);
        }
    }
    catch (const VException &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep; // NOLINT(cert-err09-cpp)
    }

    return val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadAttributeDouble(const QXmlStreamAttributes &attribs, const QString &name,
                                             const QString &defValue) -> qreal
{
    bool ok = false;
    qreal param = 0;

    const QString message = QObject::tr("Can't convert toDouble parameter");
    try
    {
        QString parametr = ReadAttributeString(attribs, name, defValue);
        param = parametr.replace(','_L1, '.'_L1).toDouble(&ok);
        if (not ok)
        {
            throw VExceptionConversionError(message, name);
        }
    }
    catch (const VException &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep; // NOLINT(cert-err09-cpp)
    }
    return param;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadAttributeUInt(const QXmlStreamAttributes &attribs, const QString &name,
                                           const QString &defValue) -> quint32
{
    bool ok = false;
    quint32 param = 0;

    const QString message = QObject::tr("Can't convert toUInt parameter");
    try
    {
        QString parametr = ReadAttributeString(attribs, name, defValue);
        param = parametr.replace(','_L1, '.'_L1).toUInt(&ok);
        if (not ok)
        {
            throw VExceptionConversionError(message, name);
        }
    }
    catch (const VException &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep; // NOLINT(cert-err09-cpp)
    }
    return param;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileReader::ReadAttributeInt(const QXmlStreamAttributes &attribs, const QString &name,
                                          const QString &defValue) -> int
{
    bool ok = false;
    int param = 0;

    const QString message = QObject::tr("Can't convert toInt parameter");
    try
    {
        QString parametr = ReadAttributeString(attribs, name, defValue);
        param = parametr.replace(','_L1, '.'_L1).toInt(&ok);
        if (not ok)
        {
            throw VExceptionConversionError(message, name);
        }
    }
    catch (const VException &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep; // NOLINT(cert-err09-cpp)
    }
    return param;
}
