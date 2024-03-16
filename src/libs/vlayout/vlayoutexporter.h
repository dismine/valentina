/************************************************************************
 **
 **  @file   vlayoutexporter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 12, 2020
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
#ifndef VLAYOUTEXPORTER_H
#define VLAYOUTEXPORTER_H

#include <QCoreApplication>
#include <QMargins>
#include <QPen>
#include <QRectF>
#include <QString>

#include "../vdxf/dxfdef.h"
#include "../vlayout/vlayoutdef.h"

class QGraphicsScene;
class QGraphicsItem;
class VLayoutPiece;

class VLayoutExporter
{
    Q_DECLARE_TR_FUNCTIONS(VLayoutExporter) // NOLINT

public:
    VLayoutExporter() = default;

    auto FileName() const -> QString;
    void SetFileName(const QString &fileName);

    auto Margins() const -> QMarginsF;
    void SetMargins(const QMarginsF &margins);

    auto ImageRect() const -> QRectF;
    void SetImageRect(const QRectF &imageRect);

    auto XScale() const -> qreal;
    void SetXScale(qreal xScale);

    auto YScale() const -> qreal;
    void SetYScale(qreal yScale);

    auto Title() const -> QString;
    void SetTitle(const QString &title);

    auto Description() const -> QString;
    void SetDescription(const QString &description);

    auto Pen() const -> QPen;
    void SetPen(const QPen &pen);

    auto IgnorePrinterMargins() const -> bool;
    void SetIgnorePrinterMargins(bool ignorePrinterMargins);

    auto BinaryDxfFormat() const -> bool;
    void SetBinaryDxfFormat(bool binaryFormat);

    auto DxfApparelCompatibility() const -> DXFApparelCompatibility;
    void SetDxfApparelCompatibility(DXFApparelCompatibility mode);

    auto IsShowGrainline() const -> bool;
    void SetShowGrainline(bool show);

    auto DxfVersion() const -> int;
    void SetDxfVersion(int dxfVersion);

    void SetBoundaryTogetherWithNotches(bool value);
    auto IsBoundaryTogetherWithNotches() const -> bool;

    void ExportToSVG(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToPNG(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToTIF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToPDF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToOBJ(QGraphicsScene *scene) const;
    void ExportToPS(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToEPS(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToFlatDXF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const;
    void ExportToAAMADXF(const QVector<VLayoutPiece> &details) const;
    void ExportToASTMDXF(const QVector<VLayoutPiece> &details) const;
    void ExportToRLD(const QVector<VLayoutPiece> &details) const;
    void ExportToHPGL(const QVector<VLayoutPiece> &details) const;
    void ExportToHPGL2(const QVector<VLayoutPiece> &details) const;

    static auto SupportPDFConversion() -> bool;

    static auto ExportFormatDescription(LayoutExportFormats format) -> QString;
    static auto ExportFormatSuffix(LayoutExportFormats format) -> QString;
    static void PdfToPs(const QStringList &params);
    static void PrepareGrainlineForExport(const QList<QGraphicsItem *> &items, bool showGrainline);
    static void RestoreGrainlineAfterExport(const QList<QGraphicsItem *> &items);

    auto offset() const -> QPointF;
    void SetOffset(const QPointF &newOffset);

    auto GetSingleLineFont() const -> bool;
    void SetSingleLineFont(bool newSingleLineFont);

    auto GetSingleStrokeOutlineFont() const -> bool;
    void SetSingleStrokeOutlineFont(bool newSingleStrokeOutlineFont);

    auto GetPenWidth() const -> int;
    void SetPenWidth(int newPenWidth);

private:
    QString m_fileName{};
    QMarginsF m_margins{};
    QRectF m_imageRect{};
    qreal m_xScale{1.0};
    qreal m_yScale{1.0};
    QString m_title{};
    QString m_description{};
    QPen m_pen{};
    bool m_ignorePrinterMargins{false};
    bool m_binaryDxfFormat{false};
    bool m_showGrainline{true};
    int m_dxfVersion{0};
    QPointF m_offset{};
    bool m_singleLineFont{false};
    bool m_singleStrokeOutlineFont{false};
    int m_penWidth{1};
    bool m_togetherWithNotches{false};
    DXFApparelCompatibility m_dxfCompatibility{DXFApparelCompatibility::STANDARD};

    void ExportToPDF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details, const QString &filename) const;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::FileName() const -> QString
{
    return m_fileName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetFileName(const QString &fileName)
{
    m_fileName = fileName;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::Margins() const -> QMarginsF
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetMargins(const QMarginsF &margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::ImageRect() const -> QRectF
{
    return m_imageRect;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetImageRect(const QRectF &imageRect)
{
    m_imageRect = imageRect;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::XScale() const -> qreal
{
    return m_xScale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetXScale(qreal xScale)
{
    m_xScale = xScale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::YScale() const -> qreal
{
    return m_yScale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetYScale(qreal yScale)
{
    m_yScale = yScale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::Title() const -> QString
{
    return m_title;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetTitle(const QString &title)
{
    m_title = title;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::Description() const -> QString
{
    return m_description;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetDescription(const QString &description)
{
    m_description = description;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::Pen() const -> QPen
{
    return m_pen;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetPen(const QPen &pen)
{
    m_pen = pen;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::IgnorePrinterMargins() const -> bool
{
    return m_ignorePrinterMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetIgnorePrinterMargins(bool ignorePrinterMargins)
{
    m_ignorePrinterMargins = ignorePrinterMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::BinaryDxfFormat() const -> bool
{
    return m_binaryDxfFormat;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetBinaryDxfFormat(bool binaryFormat)
{
    m_binaryDxfFormat = binaryFormat;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::DxfVersion() const -> int
{
    return m_dxfVersion;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetDxfVersion(int dxfVersion)
{
    m_dxfVersion = dxfVersion;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetBoundaryTogetherWithNotches(bool value)
{
    m_togetherWithNotches = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::IsBoundaryTogetherWithNotches() const -> bool
{
    return m_togetherWithNotches;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::offset() const -> QPointF
{
    return m_offset;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetOffset(const QPointF &newOffset)
{
    m_offset = newOffset;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::GetSingleLineFont() const -> bool
{
    return m_singleLineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetSingleLineFont(bool newSingleLineFont)
{
    m_singleLineFont = newSingleLineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutExporter::GetSingleStrokeOutlineFont() const -> bool
{
    return m_singleStrokeOutlineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetSingleStrokeOutlineFont(bool newSingleStrokeOutlineFont)
{
    m_singleStrokeOutlineFont = newSingleStrokeOutlineFont;
}

//---------------------------------------------------------------------------------------------------------------------
inline int VLayoutExporter::GetPenWidth() const
{
    return m_penWidth;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VLayoutExporter::SetPenWidth(int newPenWidth)
{
    m_penWidth = newPenWidth;
}

#endif // VLAYOUTEXPORTER_H
