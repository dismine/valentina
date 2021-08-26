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
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
template <class T>
auto NumberToString(T number) -> QString
{
    const QLocale locale = QLocale::c();
    return locale.toString(number, 'g', 12).remove(locale.groupSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
auto TransformToString(const QTransform &m) -> QString
{
    QStringList matrix
    {
        NumberToString(m.m11()),
        NumberToString(m.m12()),
        NumberToString(m.m13()),
        NumberToString(m.m21()),
        NumberToString(m.m22()),
        NumberToString(m.m23()),
        NumberToString(m.m31()),
        NumberToString(m.m32()),
        NumberToString(m.m33())
    };
    return matrix.join(ML::groupSep);
}

//---------------------------------------------------------------------------------------------------------------------
auto PointToString(const QPointF &p) -> QString
{
    return NumberToString(p.x()) + ML::coordintatesSep + NumberToString(p.y());
}

//---------------------------------------------------------------------------------------------------------------------
auto PathToString(const QVector<QPointF> &pathPoints) -> QString
{
    QStringList path;

    for (auto point : pathPoints)
    {
        path.append(PointToString(point));
    }

    return path.join(ML::pointsSep);
}

//---------------------------------------------------------------------------------------------------------------------
auto RectToString(const QRectF &r) -> QString
{
    return NumberToString(r.x()) + ML::groupSep +
            NumberToString(r.y()) + ML::groupSep +
            NumberToString(r.width()) + ML::groupSep +
            NumberToString(r.height());
}

//---------------------------------------------------------------------------------------------------------------------
auto MarkerShapeToString(const PlaceLabelImg &shape) -> QString
{
    QStringList s;
    for (const auto& path : shape)
    {
        s.append(PathToString(path));
    }
    return s.join(ML::itemsSep);
}

//---------------------------------------------------------------------------------------------------------------------
auto LineToString(const QLineF &line) -> QString
{
    return PointToString(line.p1()) + ML::groupSep + PointToString(line.p2());
}

//---------------------------------------------------------------------------------------------------------------------
auto LinesToString(const QVector<QLineF> &lines) -> QString
{
    QStringList l;
    for (auto line : lines)
    {
        l.append(LineToString(line));
    }
    return l.join(ML::itemsSep);
}

//---------------------------------------------------------------------------------------------------------------------
auto GrainlineArrowDirrectionToString(GrainlineArrowDirection type) -> QString
{
    switch(type)
    {
        case GrainlineArrowDirection::atFront:
            return ML::atFrontStr;
        case GrainlineArrowDirection::atRear:
            return ML::atRearStr;
        case GrainlineArrowDirection::atBoth:
        default:
            return ML::atBothStr;
    }
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteFile(const VPLayoutPtr &layout, QIODevice *file)
{
    setDevice(file);
    setAutoFormatting(true);

    writeStartDocument();
    writeComment(QStringLiteral("Layout created with Valentina v%1 (https://smart-pattern.com.ua/).")
                 .arg(APP_VERSION_STR));
    WriteLayout(layout);
    writeEndDocument();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteLayout(const VPLayoutPtr &layout)
{
    writeStartElement(ML::TagLayout);
    SetAttribute(ML::AttrVersion, VLayoutConverter::LayoutMaxVerStr);
    WriteProperties(layout);
    WritePieceList(layout->GetUnplacedPieces(), ML::TagUnplacedPieces);
    WriteSheets(layout);
    writeEndElement(); //layout
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteProperties(const VPLayoutPtr &layout)
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
    SetAttribute(ML::AttrStickyEdges, layout->LayoutSettings().GetStickyEdges());
    SetAttribute(ML::AttrPiecesGap, layout->LayoutSettings().GetPiecesGap());
    SetAttribute(ML::AttrFollowGrainline, layout->LayoutSettings().GetFollowGrainline());
    writeEndElement(); // control

    WriteTiles(layout);

    writeEndElement(); // properties
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheets(const VPLayoutPtr &layout)
{
    writeStartElement(ML::TagSheets);

    QList<VPSheetPtr> sheets = layout->GetSheets();
    for (const auto &sheet : sheets)
    {
        if (not sheet.isNull() && sheet->IsVisible())
        {
            WriteSheet(sheet);
        }
    }

    writeEndElement(); // sheets
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheet(const VPSheetPtr &sheet)
{
    writeStartElement(ML::TagSheet);

    writeTextElement(ML::TagName, sheet->GetName());
    WritePieceList(sheet->GetPieces(), ML::TagPieces);

    writeEndElement(); // sheet

}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteTiles(const VPLayoutPtr &layout)
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
void VPLayoutFileWriter::WritePieceList(const QList<VPPiecePtr> &list, const QString &tagName)
{
    writeStartElement(tagName); // piece list
    for (const auto &piece : list)
    {
        WritePiece(piece);
    }

    writeEndElement(); // piece list
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WritePiece(const VPPiecePtr &piece)
{
    writeStartElement(ML::TagPiece);
    SetAttribute(ML::AttrID, piece->GetUUID().toString());
    SetAttribute(ML::AttrName, piece->GetName());
    SetAttributeOrRemoveIf<bool>(ML::AttrMirrored, piece->IsMirror(), [](bool mirrored){return not mirrored;});
    SetAttribute(ML::AttrTransform, TransformToString(piece->GetMatrix()));

    writeStartElement(ML::TagSeamLine);
    writeCharacters(PathToString(piece->GetContourPoints()));
    writeEndElement();

    writeStartElement(ML::TagSeamAllowance);
    SetAttributeOrRemoveIf<bool>(ML::AttrEnabled, piece->IsSeamAllowance(), [](bool enabled){return not enabled;});
    SetAttributeOrRemoveIf<bool>(ML::AttrBuiltIn, piece->IsSeamAllowanceBuiltIn(),
                                 [](bool builtin){return not builtin;});
    if (piece->IsSeamAllowance() && not piece->IsSeamAllowanceBuiltIn())
    {
        writeCharacters(PathToString(piece->GetSeamAllowancePoints()));
    }
    writeEndElement();

    writeStartElement(ML::TagGrainline);
    SetAttributeOrRemoveIf<bool>(ML::AttrEnabled, piece->IsGrainlineEnabled(), [](bool enabled){return not enabled;});
    if (piece->IsGrainlineEnabled())
    {
        SetAttribute(ML::AttrAngle, piece->GrainlineAngle());
        SetAttribute(ML::AttrArrowDirection, GrainlineArrowDirrectionToString(piece->GrainlineArrowType()));
        writeCharacters(PathToString(piece->GetGrainline()));
    }
    writeEndElement();

    writeStartElement(ML::TagNotches);
    QVector<VLayoutPassmark> passmarks = piece->GetPassmarks();
    for (const auto& passmark : passmarks)
    {
        writeStartElement(ML::TagNotch);
        SetAttribute(ML::AttrBuiltIn, passmark.isBuiltIn);
        SetAttribute(ML::AttrType, static_cast<int>(passmark.type));
        SetAttribute(ML::AttrBaseLine, LineToString(passmark.baseLine));
        SetAttribute(ML::AttrPath, LinesToString(passmark.lines));
        writeEndElement();
    }
    writeEndElement();

    writeStartElement(ML::TagInternalPaths);
    QVector<VLayoutPiecePath> internalPaths = piece->GetInternalPaths();
    for (const auto& path : internalPaths)
    {
        writeStartElement(ML::TagInternalPath);
        SetAttribute(ML::AttrCut, path.IsCutPath());
        SetAttribute(ML::AttrPenStyle, PenStyleToLineStyle(path.PenStyle()));
        writeCharacters(PathToString(path.Points()));
        writeEndElement();
    }
    writeEndElement();

    writeStartElement(ML::TagMarkers);
    QVector<VLayoutPlaceLabel> placelabels = piece->GetPlaceLabels();
    for (const auto& label : placelabels)
    {
        writeStartElement(ML::TagMarker);
        SetAttribute(ML::AttrTransform, TransformToString(label.rotationMatrix));
        SetAttribute(ML::AttrType, static_cast<int>(label.type));
        SetAttribute(ML::AttrCenter, PointToString(label.center));
        SetAttribute(ML::AttrBox, RectToString(label.box));
        writeCharacters(MarkerShapeToString(label.shape));

        writeEndElement();
    }
    writeEndElement();

    writeStartElement(ML::TagLabels);
    WriteLabel(piece->GetPieceLabelRect(), piece->GetPieceLabelData(), ML::TagPieceLabel);
    WriteLabel(piece->GetPatternLabelRect(), piece->GetPatternLabelData(), ML::TagPatternLabel);
    writeEndElement();

    writeEndElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteLabel(const QVector<QPointF> &labelShape, const VTextManager &tm, const QString &tagName)
{
    if (labelShape.size() > 2 && tm.GetSourceLinesCount() > 0)
    {
        writeStartElement(tagName);
        SetAttribute(ML::AttrShape, PathToString(labelShape));
        WriteLabelLines(tm);
        writeEndElement();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteLabelLines(const VTextManager &tm)
{
    writeStartElement(ML::TagLines);
    SetAttribute(ML::AttrFont, tm.GetFont().toString());

    for (int i = 0; i < tm.GetSourceLinesCount(); ++i)
    {
        writeStartElement(ML::TagLine);
        const TextLine& tl = tm.GetSourceLine(i);
        SetAttribute(ML::AttrFontSize, tl.m_iFontSize);
        SetAttribute(ML::AttrBold, tl.m_bold);
        SetAttribute(ML::AttrItalic, tl.m_italic);
        SetAttribute(ML::AttrAlignment, static_cast<int>(tl.m_eAlign));
        writeCharacters(tl.m_qsText);
        writeEndElement();
    }
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
