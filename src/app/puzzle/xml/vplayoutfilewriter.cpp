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
#include "../ifc/xml/vlayoutconverter.h"
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"
#include "../vmisc/projectversion.h"
#include "vplayoutliterals.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
template <class T> auto NumberToString(T number) -> QString
{
    const QLocale locale = QLocale::c();
    return locale.toString(number, 'g', 12).remove(LocaleGroupSeparator(locale));
}

//---------------------------------------------------------------------------------------------------------------------
auto TransformToString(const QTransform &m) -> QString
{
    QStringList matrix{NumberToString(m.m11()), NumberToString(m.m12()), NumberToString(m.m13()),
                       NumberToString(m.m21()), NumberToString(m.m22()), NumberToString(m.m23()),
                       NumberToString(m.m31()), NumberToString(m.m32()), NumberToString(m.m33())};
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
    path.reserve(pathPoints.size());

    for (auto point : pathPoints)
    {
        path.append(PointToString(point));
    }

    return path.join(ML::pointsSep);
}

//---------------------------------------------------------------------------------------------------------------------
auto RectToString(const QRectF &r) -> QString
{
    return NumberToString(r.x()) + ML::groupSep + NumberToString(r.y()) + ML::groupSep + NumberToString(r.width()) +
           ML::groupSep + NumberToString(r.height());
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
    l.reserve(lines.size());
    for (auto line : lines)
    {
        l.append(LineToString(line));
    }
    return l.join(ML::itemsSep);
}

//---------------------------------------------------------------------------------------------------------------------
auto GrainlineArrowDirrectionToString(GrainlineArrowDirection type) -> QString
{
    switch (type)
    {
        case GrainlineArrowDirection::oneWayUp:
            return ML::oneWayUpStr;
        case GrainlineArrowDirection::oneWayDown:
            return ML::oneWayDownStr;
        case GrainlineArrowDirection::fourWays:
            return ML::fourWaysStr;
        case GrainlineArrowDirection::twoWaysUpLeft:
            return ML::twoWaysUpLeftStr;
        case GrainlineArrowDirection::twoWaysUpRight:
            return ML::twoWaysUpRightStr;
        case GrainlineArrowDirection::twoWaysDownLeft:
            return ML::twoWaysDownLeftStr;
        case GrainlineArrowDirection::twoWaysDownRight:
            return ML::twoWaysDownRightStr;
        case GrainlineArrowDirection::threeWaysUpDownLeft:
            return ML::threeWaysUpDownLeftStr;
        case GrainlineArrowDirection::threeWaysUpDownRight:
            return ML::threeWaysUpDownRightStr;
        case GrainlineArrowDirection::threeWaysUpLeftRight:
            return ML::threeWaysUpLeftRightStr;
        case GrainlineArrowDirection::threeWaysDownLeftRight:
            return ML::threeWaysDownLeftRightStr;
        case GrainlineArrowDirection::twoWaysUpDown:
        default:
            return ML::twoWaysUpDownStr;
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteFile(const VPLayoutPtr &layout, QIODevice *file)
{
    setDevice(file);
    setAutoFormatting(true);

    writeStartDocument();
    writeComment(
        QStringLiteral("Layout created with Valentina v%1 (https://smart-pattern.com.ua/).").arg(AppVersionStr()));
    WriteLayout(layout);
    writeEndDocument();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteLayout(const VPLayoutPtr &layout)
{
    writeStartElement(ML::TagLayout);
    SetAttribute(AttrLayoutVersion, VLayoutConverter::LayoutMaxVerStr);
    WriteLayoutProperties(layout);
    WritePieceList(layout->GetUnplacedPieces(), ML::TagUnplacedPieces);
    WriteSheets(layout);
    writeEndElement(); // layout
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteLayoutProperties(const VPLayoutPtr &layout)
{
    writeStartElement(ML::TagProperties);

    writeTextElement(ML::TagUnit, UnitsToStr(layout->LayoutSettings().GetUnit()));
    writeTextElement(ML::TagTitle, layout->LayoutSettings().GetTitle());
    writeTextElement(ML::TagDescription, layout->LayoutSettings().GetDescription());

    writeStartElement(ML::TagControl);
    SetAttribute(ML::AttrWarningSuperposition, layout->LayoutSettings().GetWarningSuperpositionOfPieces());
    SetAttribute(ML::AttrWarningOutOfBound, layout->LayoutSettings().GetWarningPiecesOutOfBound());
    SetAttribute(ML::AttrStickyEdges, layout->LayoutSettings().GetStickyEdges());
    SetAttribute(ML::AttrPiecesGap, layout->LayoutSettings().GetPiecesGap());
    SetAttribute(ML::AttrFollowGrainline, layout->LayoutSettings().GetFollowGrainline());
    writeEndElement(); // control

    WriteTiles(layout);

    writeStartElement(ML::TagScale);
    SetAttribute(ML::AttrXScale, layout->LayoutSettings().HorizontalScale());
    SetAttribute(ML::AttrYScale, layout->LayoutSettings().VerticalScale());
    writeEndElement(); // scale

    writeStartElement(ML::TagWatermark);
    SetAttributeOrRemoveIf<bool>(ML::AttrShowPreview, layout->LayoutSettings().GetShowWatermark(),
                                 [](bool show) noexcept { return not show; });
    writeCharacters(layout->LayoutSettings().WatermarkPath());
    writeEndElement(); // watermark

    writeEndElement(); // properties
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSheets(const VPLayoutPtr &layout)
{
    writeStartElement(ML::TagSheets);

    QList<VPSheetPtr> sheets = layout->GetSheets();
    for (const auto &sheet : sheets)
    {
        if (not sheet.isNull())
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
    SetAttributeOrRemoveIf<QString>(ML::AttrGrainlineType, GrainlineTypeToStr(sheet->GetGrainlineType()),
                                    [](const QString &type) noexcept
                                    { return type == GrainlineTypeToStr(GrainlineType::NotFixed); });

    writeTextElement(ML::TagName, sheet->GetName());
    WriteSize(sheet->GetSheetSize());
    WriteMargins(sheet->GetSheetMargins(), sheet->IgnoreMargins());
    WritePieceList(sheet->GetPieces(), ML::TagPieces);

    writeEndElement(); // sheet
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteTiles(const VPLayoutPtr &layout)
{
    writeStartElement(ML::TagTiles);
    SetAttribute(ML::AttrVisible, layout->LayoutSettings().GetShowTiles());
    SetAttribute(ML::AttrMatchingMarks, "standard"); // TODO / Fixme get the right value
    SetAttributeOrRemoveIf<bool>(ML::AttrPrintScheme, layout->LayoutSettings().GetPrintTilesScheme(),
                                 [](bool print) noexcept { return not print; });
    SetAttributeOrRemoveIf<bool>(ML::AttrTileNumber, layout->LayoutSettings().GetShowTileNumber(),
                                 [](bool show) noexcept { return not show; });

    WriteSize(layout->LayoutSettings().GetTilesSize());
    WriteMargins(layout->LayoutSettings().GetTilesMargins(), layout->LayoutSettings().IgnoreTilesMargins());

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
    SetAttribute(ML::AttrUID, piece->GetUUID().toString());
    SetAttribute(ML::AttrName, piece->GetName());
    SetAttributeOrRemoveIf<bool>(ML::AttrMirrored, piece->IsMirror(),
                                 [](bool mirrored) noexcept { return not mirrored; });
    SetAttributeOrRemoveIf<bool>(ML::AttrForbidFlipping, piece->IsForbidFlipping(),
                                 [](bool forbid) noexcept { return not forbid; });
    SetAttributeOrRemoveIf<bool>(ML::AttrForceFlipping, piece->IsForceFlipping(),
                                 [](bool force) noexcept { return not force; });
    SetAttributeOrRemoveIf<bool>(ML::AttrFollowGrainline, piece->IsFollowGrainline(),
                                 [](bool follow) noexcept { return not follow; });
    SetAttributeOrRemoveIf<bool>(ML::AttrSewLineOnDrawing, piece->IsSewLineOnDrawing(),
                                 [](bool value) noexcept { return not value; });
    SetAttribute(ML::AttrTransform, TransformToString(piece->GetMatrix()));
    SetAttributeOrRemoveIf<QString>(ML::AttrGradationLabel, piece->GetGradationId(),
                                    [](const QString &label) noexcept { return label.isEmpty(); });
    SetAttribute(ML::AttrCopyNumber, piece->CopyNumber());
    SetAttributeOrRemoveIf<bool>(ML::AttrShowSeamline, not piece->IsHideMainPath(),
                                 [](bool show) noexcept { return show; });
    SetAttributeOrRemoveIf<qreal>(ML::AttrXScale, piece->GetXScale(),
                                  [](qreal xs) noexcept { return VFuzzyComparePossibleNulls(xs, 1.0); });
    SetAttributeOrRemoveIf<qreal>(ML::AttrYScale, piece->GetYScale(),
                                  [](qreal ys) noexcept { return VFuzzyComparePossibleNulls(ys, 1.0); });
    SetAttributeOrRemoveIf<qreal>(ML::AttrZValue, piece->ZValue(),
                                  [](qreal z) noexcept { return VFuzzyComparePossibleNulls(z, 1.0); });

    writeStartElement(ML::TagSeamLine);
    QVector<VLayoutPoint> contourPoints = piece->GetContourPoints();
    for (auto &point : contourPoints)
    {
        WriteLayoutPoint(point);
    }
    writeEndElement();

    writeStartElement(ML::TagSeamAllowance);
    SetAttributeOrRemoveIf<bool>(ML::AttrEnabled, piece->IsSeamAllowance(),
                                 [](bool enabled) noexcept { return not enabled; });
    SetAttributeOrRemoveIf<bool>(ML::AttrBuiltIn, piece->IsSeamAllowanceBuiltIn(),
                                 [](bool builtin) noexcept { return not builtin; });
    if (piece->IsSeamAllowance() && not piece->IsSeamAllowanceBuiltIn())
    {
        QVector<VLayoutPoint> seamAllowancePoints = piece->GetSeamAllowancePoints();
        for (auto &point : seamAllowancePoints)
        {
            WriteLayoutPoint(point);
        }
    }
    writeEndElement();

    writeStartElement(ML::TagGrainline);
    SetAttributeOrRemoveIf<bool>(ML::AttrEnabled, piece->IsGrainlineEnabled(),
                                 [](bool enabled) noexcept { return not enabled; });
    if (piece->IsGrainlineEnabled())
    {
        SetAttribute(ML::AttrArrowDirection, GrainlineArrowDirrectionToString(piece->GetGrainline().GetArrowType()));
        writeCharacters(LineToString(piece->GetGrainlineMainLine()));
    }
    writeEndElement();

    writeStartElement(ML::TagNotches);
    QVector<VLayoutPassmark> passmarks = piece->GetPassmarks();
    for (const auto &passmark : passmarks)
    {
        writeStartElement(ML::TagNotch);
        SetAttribute(ML::AttrBuiltIn, passmark.isBuiltIn);
        SetAttribute(ML::AttrType, static_cast<int>(passmark.type));
        SetAttribute(ML::AttrBaseLine, LineToString(passmark.baseLine));
        SetAttribute(ML::AttrPath, LinesToString(passmark.lines));
        SetAttributeOrRemoveIf<bool>(ML::AttrClockwiseOpening, passmark.isClockwiseOpening,
                                     [](bool clockwise) noexcept { return not clockwise; });
        writeEndElement();
    }
    writeEndElement();

    writeStartElement(ML::TagInternalPaths);
    QVector<VLayoutPiecePath> internalPaths = piece->GetInternalPaths();
    for (const auto &path : internalPaths)
    {
        writeStartElement(ML::TagInternalPath);
        SetAttribute(ML::AttrCut, path.IsCutPath());
        SetAttribute(ML::AttrPenStyle, PenStyleToLineStyle(path.PenStyle()));

        QVector<VLayoutPoint> points = path.Points();
        for (auto &point : points)
        {
            WriteLayoutPoint(point);
        }

        writeEndElement();
    }
    writeEndElement();

    writeStartElement(ML::TagMarkers);
    QVector<VLayoutPlaceLabel> placelabels = piece->GetPlaceLabels();
    for (const auto &label : placelabels)
    {
        writeStartElement(ML::TagMarker);
        SetAttribute(ML::AttrTransform, TransformToString(label.RotationMatrix()));
        SetAttribute(ML::AttrType, static_cast<int>(label.Type()));
        SetAttribute(ML::AttrCenter, PointToString(label.Center()));
        SetAttribute(ML::AttrBox, RectToString(label.Box()));
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
    SetAttribute(ML::AttrSVGFont, QStringLiteral("%1,%2").arg(tm.GetSVGFontFamily(), tm.GetSVGFontPointSize()));

    for (int i = 0; i < tm.GetSourceLinesCount(); ++i)
    {
        writeStartElement(ML::TagLine);
        const TextLine &tl = tm.GetSourceLine(i);
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
void VPLayoutFileWriter::WriteMargins(const QMarginsF &margins, bool ignore)
{
    writeStartElement(ML::TagMargin);

    SetAttributeOrRemoveIf<qreal>(ML::AttrLeft, margins.left(), [](qreal margin) noexcept { return margin <= 0; });
    SetAttributeOrRemoveIf<qreal>(ML::AttrTop, margins.top(), [](qreal margin) noexcept { return margin <= 0; });
    SetAttributeOrRemoveIf<qreal>(ML::AttrRight, margins.right(), [](qreal margin) noexcept { return margin <= 0; });
    SetAttributeOrRemoveIf<qreal>(ML::AttrBottom, margins.bottom(), [](qreal margin) noexcept { return margin <= 0; });

    SetAttributeOrRemoveIf<bool>(ML::AttrIgnoreMargins, ignore, [](bool ignore) noexcept { return not ignore; });

    writeEndElement(); // margin
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutFileWriter::WriteSize(QSizeF size)
{
    // maybe not necessary to test this, the writer should "stupidly write", the application should take care of these
    // tests
    qreal width = size.width();
    if (width < 0)
    {
        width = 0;
    }

    qreal length = size.height();
    if (length < 0)
    {
        length = 0;
    }

    writeStartElement(ML::TagSize);
    SetAttribute(ML::AttrWidth, width);
    SetAttribute(ML::AttrLength, length);
    writeEndElement(); // size
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutFileWriter::WriteLayoutPoint(const VLayoutPoint &point) -> void
{
    writeStartElement(ML::TagPoint);
    SetAttribute(ML::AttrX, point.x());
    SetAttribute(ML::AttrY, point.y());
    SetAttributeOrRemoveIf<bool>(ML::AttrTurnPoint, point.TurnPoint(), [](bool val) noexcept { return val; });
    SetAttributeOrRemoveIf<bool>(ML::AttrCurvePoint, point.CurvePoint(), [](bool val) noexcept { return val; });
    writeEndElement();
}
