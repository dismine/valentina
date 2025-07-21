/************************************************************************
 **
 **  @file   vlayoutexporter.cpp
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
#include "vlayoutexporter.h"

#include <QCursor>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QImageWriter>
#include <QPainter>
#include <QPrinter>
#include <QProcess>
#include <QSvgGenerator>
#include <QTemporaryFile>
#include <QtDebug>
#include <QtMath>

#include "../ifc/exception/vexception.h"
#include "../vdxf/vdxfpaintdevice.h"
#include "../vhpgl/vhpglpaintdevice.h"
#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/vabstractapplication.h"
#include "../vobj/vobjpaintdevice.h"
#include "vgraphicsfillitem.h"
#include "vprintlayout.h"
#include "vrawlayout.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

#ifdef Q_OS_WIN
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PDFTOPS, ("pdftops.exe"_L1)) // NOLINT
#else
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PDFTOPS, ("pdftops"_L1)) // NOLINT
#endif

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PrepareTextForDXF prepare QGraphicsSimpleTextItem items for export to
 * flat dxf.
 *
 * Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark
 * end of each string by adding special placholder. This method append it.
 *
 * @param placeholder placeholder that will be appended to each
 * QGraphicsSimpleTextItem item's text string.
 */
void PrepareDetailsForDXF(const QString &placeholder, const QList<QGraphicsItem *> &paperItems)
{
    for (auto *item : paperItems)
    {
        QList<QGraphicsItem *> pieceChildren = item->childItems();
        for (auto *child : qAsConst(pieceChildren))
        {
            if (child->type() == QGraphicsSimpleTextItem::Type)
            {
                if (auto *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(child))
                {
                    textItem->setText(textItem->text() + placeholder);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RestoreTextAfterDXF restore QGraphicsSimpleTextItem items after export
 * to flat dxf.
 *
 * Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark
 * end of each string by adding special placholder. This method remove it.
 *
 * @param placeholder placeholder that will be removed from each
 * QGraphicsSimpleTextItem item's text string.
 */
void RestoreDetailsAfterDXF(const QString &placeholder, const QList<QGraphicsItem *> &paperItems)
{
    for (auto *item : paperItems)
    {
        QList<QGraphicsItem *> pieceChildren = item->childItems();
        for (auto *child : qAsConst(pieceChildren))
        {
            if (child->type() == QGraphicsSimpleTextItem::Type)
            {
                if (auto *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(child))
                {
                    QString text = textItem->text();
                    text.replace(placeholder, QString());
                    textItem->setText(text);
                }
            }
        }
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::DxfApparelCompatibility() const -> DXFApparelCompatibility
{
    return m_dxfCompatibility;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::SetDxfApparelCompatibility(DXFApparelCompatibility mode)
{
    m_dxfCompatibility = mode;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::IsShowGrainline() const -> bool
{
    return m_showGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::SetShowGrainline(bool show)
{
    m_showGrainline = show;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToSVG(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    PrepareGrainlineForExport(details, m_showGrainline);

    QSvgGenerator generator;
    generator.setFileName(m_fileName);

    QSize drawingSize;
    drawingSize.setWidth(qFloor(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right()));
    drawingSize.setHeight(qFloor(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom()));
    generator.setSize(drawingSize);

    QRectF viewBoxRect;
    viewBoxRect.setWidth(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right());
    viewBoxRect.setHeight(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom());
    generator.setViewBox(viewBoxRect);

    generator.setTitle(m_title);
    generator.setDescription(m_description);
    generator.setResolution(static_cast<int>(PrintDPI));

    QPainter painter;
    painter.begin(&generator);
    painter.translate(-m_offset.x(), -m_offset.y());
    painter.translate(m_margins.left(), m_margins.top());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(m_pen);
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.scale(m_xScale, m_yScale);
    scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);
    painter.end();

    RestoreGrainlineAfterExport(details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPNG(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    PrepareGrainlineForExport(details, m_showGrainline);

    // Create the image with the exact size of the shrunk scene
    QSize drawingSize;
    drawingSize.setWidth(qFloor(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right()));
    drawingSize.setHeight(qFloor(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom()));

    QImage image(drawingSize, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.translate(m_margins.left(), m_margins.top());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(m_pen);
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.scale(m_xScale, m_yScale);

    scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);
    image.save(m_fileName);

    RestoreGrainlineAfterExport(details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToTIF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    PrepareGrainlineForExport(details, m_showGrainline);

    // Create the image with the exact size of the shrunk scene
    QSize drawingSize;
    drawingSize.setWidth(qFloor(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right()));
    drawingSize.setHeight(qFloor(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom()));

    QImage image(drawingSize, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.translate(m_margins.left(), m_margins.top());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(m_pen);
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.scale(m_xScale, m_yScale);

    scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);

    QImageWriter writer;
    writer.setFormat("TIF");
    writer.setCompression(1); // LZW-compression
    writer.setFileName(m_fileName);

    if (not writer.write(image))
    { // failed to save file
        qCritical() << qUtf8Printable(tr("Can't save file '%1'. Error: %2.").arg(m_fileName, writer.errorString()));
        RestoreGrainlineAfterExport(details);
        return;
    }

    RestoreGrainlineAfterExport(details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPDF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    ExportToPDF(scene, details, m_fileName);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToOBJ(QGraphicsScene *scene) const
{
    VObjPaintDevice generator;
    generator.setFileName(m_fileName);
    generator.setSize(m_imageRect.size().toSize());
    generator.setResolution(static_cast<int>(PrintDPI));

    QPainter painter;
    painter.begin(&generator);
    scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);
    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPS(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        const QString fileName = m_fileName;
        ExportToPDF(scene, details, tmp.fileName());
        PdfToPs(QStringList{tmp.fileName(), fileName});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToEPS(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        const QString fileName = m_fileName;
        ExportToPDF(scene, details, tmp.fileName());
        PdfToPs(QStringList{QStringLiteral("-eps"), tmp.fileName(), fileName});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToFlatDXF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    PrepareDetailsForDXF(endStringPlaceholder, details);
    PrepareGrainlineForExport(details, m_showGrainline);

    VDxfPaintDevice generator;
    generator.SetFileName(m_fileName);
    generator.SetSize(QSize(qFloor(m_imageRect.width() * m_xScale), qFloor(m_imageRect.height() * m_yScale)));
    generator.SetResolution(PrintDPI);
    generator.SetVersion(static_cast<DRW::Version>(m_dxfVersion));
    generator.SetBinaryFormat(m_binaryDxfFormat);
    generator.SetInsunits(VarInsunits::Millimeters); // Decided to always use mm. See issue #745

    QPainter painter;
    if (painter.begin(&generator))
    {
        painter.scale(m_xScale, m_yScale);
        scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);
        if (not painter.end())
        {
            qCritical() << tr("Can't create a flat dxf file.") << generator.ErrorString();
        }
    }

    RestoreDetailsAfterDXF(endStringPlaceholder, details);
    RestoreGrainlineAfterExport(details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToAAMADXF(const QVector<VLayoutPiece> &details) const
{
    VDxfPaintDevice generator;
    generator.SetFileName(m_fileName);
    generator.SetSize(QSize(qCeil(m_imageRect.width() * m_xScale), qCeil(m_imageRect.height() * m_yScale)));
    generator.SetResolution(PrintDPI);
    generator.SetVersion(static_cast<DRW::Version>(m_dxfVersion));
    generator.SetBinaryFormat(m_binaryDxfFormat);
    generator.SetInsunits(VarInsunits::Millimeters); // Decided to always use mm. See issue #745
    generator.SetXScale(m_xScale);
    generator.SetYScale(m_yScale);
    generator.SetBoundaryTogetherWithNotches(m_togetherWithNotches);
    generator.SetDxfApparelCompatibility(m_dxfCompatibility);
    if (not generator.ExportToAAMA(details))
    {
        qCritical() << tr("Can't create an AAMA dxf file.") << generator.ErrorString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToASTMDXF(const QVector<VLayoutPiece> &details) const
{
    VDxfPaintDevice generator;
    generator.SetFileName(m_fileName);
    generator.SetSize(QSize(qCeil(m_imageRect.width() * m_xScale), qCeil(m_imageRect.height() * m_yScale)));
    generator.SetResolution(PrintDPI);
    generator.SetVersion(static_cast<DRW::Version>(m_dxfVersion));
    generator.SetBinaryFormat(m_binaryDxfFormat);
    generator.SetInsunits(VarInsunits::Millimeters); // Decided to always use mm. See issue #745
    generator.SetXScale(m_xScale);
    generator.SetYScale(m_yScale);
    generator.SetBoundaryTogetherWithNotches(m_togetherWithNotches);
    generator.SetDxfApparelCompatibility(m_dxfCompatibility);
    if (not generator.ExportToASTM(details))
    {
        qCritical() << tr("Can't create an ASTM dxf file.") << generator.ErrorString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToRLD(const QVector<VLayoutPiece> &details) const
{
    QVector<VLayoutPiece> scaledPieces;
    scaledPieces.reserve(details.size());

    for (auto detail : details)
    {
        detail.Scale(m_xScale, m_yScale);
        scaledPieces.append(detail);
    }

    VRawLayoutData layoutData;
    layoutData.pieces = scaledPieces;

    VRawLayout generator;
    if (not generator.WriteFile(m_fileName, layoutData))
    {
        const QString errorMsg = tr("Export raw layout data failed. %1.").arg(generator.ErrorString());
        VAbstractApplication::VApp()->IsPedantic() ? throw VException(errorMsg) : qCritical() << errorMsg;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToHPGL(const QVector<VLayoutPiece> &details) const
{
    VHPGLPaintDevice generator;
    generator.SetFileName(m_fileName);
    generator.SetSize(QSize(qCeil(m_imageRect.width() * m_xScale), qCeil(m_imageRect.height() * m_yScale)));
    generator.SetXScale(m_xScale);
    generator.SetYScale(m_yScale);
    generator.SetShowGrainline(m_showGrainline);
    generator.SetSingleLineFont(m_singleLineFont);
    generator.SetSingleStrokeOutlineFont(m_singleStrokeOutlineFont);
    generator.SetPenWidth(m_penWidth);
    generator.SetBoundaryTogetherWithNotches(m_togetherWithNotches);
    if (not generator.ExportToHPGL(details))
    {
        qCritical() << tr("Can't create an HP-GL file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToHPGL2(const QVector<VLayoutPiece> &details) const
{
    VHPGLPaintDevice generator;
    generator.SetFileName(m_fileName);
    generator.SetSize(QSize(qCeil(m_imageRect.width() * m_xScale), qCeil(m_imageRect.height() * m_yScale)));
    generator.SetXScale(m_xScale);
    generator.SetYScale(m_yScale);
    generator.SetShowGrainline(m_showGrainline);
    generator.SetSingleLineFont(m_singleLineFont);
    generator.SetSingleStrokeOutlineFont(m_singleStrokeOutlineFont);
    generator.SetPenWidth(m_penWidth);
    generator.SetBoundaryTogetherWithNotches(m_togetherWithNotches);
    if (not generator.ExportToHPGL2(details))
    {
        qCritical() << tr("Can't create an HP-GL file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::SupportPDFConversion() -> bool
{
    auto Test = [](const QString &program)
    {
        QProcess proc;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        proc.start(program);
#else
        proc.start(program, QStringList());
#endif

        const int timeout = 15000;
        if (proc.waitForStarted(timeout) && (proc.waitForFinished(timeout) || proc.state() == QProcess::NotRunning))
        {
            return true;
        }

        qDebug() << program << "error" << proc.error() << proc.errorString();
        return false;
    };

#if defined(Q_OS_WIN)
    return Test(qApp->applicationDirPath() + '/'_L1 + *PDFTOPS);
#else
    return Test(*PDFTOPS);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PdfToPs use external tool "pdftops" for converting pdf too eps or ps
 * format.
 * @param params string with parameter for tool. Parameters have format: "-eps
 * input_file out_file". Use -eps when need create eps file.
 */
void VLayoutExporter::PdfToPs(const QStringList &params)
{
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    QProcess proc;
    proc.start(*PDFTOPS, params);

    const int timeout = 15000;
    if (proc.waitForStarted(timeout))
    {
        proc.waitForFinished(timeout);
    }

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    if (QFile const f(params.constLast()); not f.exists())
    {
        qCritical() << qUtf8Printable(tr("Creating file '%1' failed! %2").arg(params.constLast(), proc.errorString()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPDF(QGraphicsScene *scene,
                                  const QList<QGraphicsItem *> &details,
                                  const QString &filename) const
{
    PrepareGrainlineForExport(details, m_showGrainline);

    QPrinter printer;
    printer.setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space)
                       + QCoreApplication::applicationVersion());
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setDocName(QFileInfo(filename).fileName());
    printer.setResolution(static_cast<int>(PrintDPI));
    printer.setFullPage(m_ignorePrinterMargins);

    QPageLayout::Orientation const imageOrientation = m_imageRect.height() >= m_imageRect.width()
                                                          ? QPageLayout::Portrait
                                                          : QPageLayout::Landscape;

    qreal const width = FromPixel(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right(), Unit::Mm);
    qreal const height = FromPixel(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom(), Unit::Mm);

    QSizeF const pageSize = imageOrientation == QPageLayout::Portrait ? QSizeF(width, height) : QSizeF(height, width);
    if (not printer.setPageSize(QPageSize(pageSize, QPageSize::Millimeter)))
    {
        qWarning() << tr("Cannot set printer page size");
    }

    printer.setPageOrientation(imageOrientation);

    if (not m_ignorePrinterMargins)
    {
        const qreal left = FromPixel(m_margins.left(), Unit::Mm);
        const qreal top = FromPixel(m_margins.top(), Unit::Mm);
        const qreal right = FromPixel(m_margins.right(), Unit::Mm);
        const qreal bottom = FromPixel(m_margins.bottom(), Unit::Mm);

        if (not printer.setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Millimeter))
        {
            qWarning() << tr("Cannot set printer margins");
        }
    }

    QPainter painter;
    if (not painter.begin(&printer))
    { // failed to open file
        qCritical() << qUtf8Printable(tr("Can't open file '%1'").arg(m_fileName));
        RestoreGrainlineAfterExport(details);
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(m_pen);
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.scale(m_xScale, m_yScale);
    scene->render(&painter, VPrintLayout::SceneTargetRect(&printer, m_imageRect), m_imageRect, Qt::IgnoreAspectRatio);
    painter.end();

    RestoreGrainlineAfterExport(details);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::ExportFormatDescription(LayoutExportFormats format) -> QString
{
    const QString dxfFlatFilesStr = tr("(flat) files");
    const QString filesStr = tr("files");

    switch (format)
    {
        case LayoutExportFormats::SVG:
            return QStringLiteral("Svg %1 (*.svg)").arg(filesStr);
        case LayoutExportFormats::PDF:
            return QStringLiteral("PDF %1 (*.pdf)").arg(filesStr);
        case LayoutExportFormats::PNG:
            return tr("Image files") + QStringLiteral(" (*.png)");
        case LayoutExportFormats::OBJ:
            return QStringLiteral("Wavefront OBJ (*.obj)");
        case LayoutExportFormats::PS:
            return QStringLiteral("PS %1 (*.ps)").arg(filesStr);
        case LayoutExportFormats::EPS:
            return QStringLiteral("EPS %1 (*.eps)").arg(filesStr);
        case LayoutExportFormats::DXF_AC1006_Flat:
            return QStringLiteral("AutoCAD DXF R10 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1009_Flat:
            return QStringLiteral("AutoCAD DXF R11/12 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1012_Flat:
            return QStringLiteral("AutoCAD DXF R13 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1014_Flat:
            return QStringLiteral("AutoCAD DXF R14 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1015_Flat:
            return QStringLiteral("AutoCAD DXF 2000 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1018_Flat:
            return QStringLiteral("AutoCAD DXF 2004 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1021_Flat:
            return QStringLiteral("AutoCAD DXF 2007 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1024_Flat:
            return QStringLiteral("AutoCAD DXF 2010 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AC1027_Flat:
            return QStringLiteral("AutoCAD DXF 2013 %1 (*.dxf)").arg(dxfFlatFilesStr);
        case LayoutExportFormats::DXF_AAMA:
            return QStringLiteral("AutoCAD DXF AAMA %1 (*.dxf)").arg(filesStr);
        case LayoutExportFormats::DXF_ASTM:
            return QStringLiteral("AutoCAD DXF ASTM %1 (*.dxf)").arg(filesStr);
        case LayoutExportFormats::PDFTiled:
            return QStringLiteral("PDF %1 %2 (*.pdf)").arg(tr("tiled"), filesStr);
        case LayoutExportFormats::NC:
            return QStringLiteral("%1 %2 (*.nc)").arg(tr("Numerical control"), filesStr);
        case LayoutExportFormats::RLD:
            return QStringLiteral("%1 %2 (*.rld)").arg(tr("Raw Layout Data"), filesStr);
        case LayoutExportFormats::TIF:
            return QStringLiteral("TIFF %1 (*.tif)").arg(filesStr);
        case LayoutExportFormats::HPGL:
            return QStringLiteral("HP-GL %1 (*.hpgl)").arg(filesStr);
        case LayoutExportFormats::HPGL2:
            return QStringLiteral("HP-GL/2 %1 (*.hpgl)").arg(filesStr);
        case LayoutExportFormats::HPGL_PLT:
            return QStringLiteral("PLT (HP-GL) %1 (*.plt)").arg(filesStr);
        case LayoutExportFormats::HPGL2_PLT:
            return QStringLiteral("PLT (HP-GL/2) %1 (*.plt)").arg(filesStr);
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::ExportFormatSuffix(LayoutExportFormats format) -> QString
{
    switch (format)
    {
        case LayoutExportFormats::SVG:
            return QStringLiteral(".svg");
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
            return QStringLiteral(".pdf");
        case LayoutExportFormats::PNG:
            return QStringLiteral(".png");
        case LayoutExportFormats::OBJ:
            return QStringLiteral(".obj");
        case LayoutExportFormats::PS:
            return QStringLiteral(".ps");
        case LayoutExportFormats::EPS:
            return QStringLiteral(".eps");
        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            return QStringLiteral(".dxf");
        case LayoutExportFormats::NC:
            return QStringLiteral(".nc");
        case LayoutExportFormats::RLD:
            return QStringLiteral(".rld");
        case LayoutExportFormats::TIF:
            return QStringLiteral(".tif");
        case LayoutExportFormats::HPGL:
        case LayoutExportFormats::HPGL2:
            return QStringLiteral(".hpgl");
        case LayoutExportFormats::HPGL_PLT:
        case LayoutExportFormats::HPGL2_PLT:
            return QStringLiteral(".plt");
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::PrepareGrainlineForExport(const QList<QGraphicsItem *> &items, bool showGrainline)
{
    for (auto *item : items)
    {
        QList<QGraphicsItem *> pieceChildren = item->childItems();
        for (auto *child : qAsConst(pieceChildren))
        {
            if (child->type() == VGraphicsFillItem::Type)
            {
                child->setVisible(showGrainline);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::RestoreGrainlineAfterExport(const QList<QGraphicsItem *> &items)
{
    for (auto *item : items)
    {
        QList<QGraphicsItem *> pieceChildren = item->childItems();
        for (auto *child : qAsConst(pieceChildren))
        {
            if (child->type() == VGraphicsFillItem::Type)
            {
                child->setVisible(true);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
using LayoutExportFileFormatEnum = LayoutExportFileFormat; // Stupid Clang format!
auto VLayoutExporter::LayoutExportFileFormat(LayoutExportFormats format) -> LayoutExportFileFormatEnum
{
    Q_STATIC_ASSERT_X(static_cast<int>(LayoutExportFormats::COUNT) == 41, "Update to cover all cases.");

    switch (format)
    {
        case LayoutExportFormats::SVG:
            return LayoutExportFileFormat::SVG;

        case LayoutExportFormats::PS:
        case LayoutExportFormats::EPS:
        case LayoutExportFormats::PDF:
        case LayoutExportFormats::PDFTiled:
            return LayoutExportFileFormat::PDF;

        case LayoutExportFormats::PNG:
            return LayoutExportFileFormat::PNG;

        case LayoutExportFormats::OBJ:
            return LayoutExportFileFormat::OBJ;

        case LayoutExportFormats::DXF_AC1006_Flat:
        case LayoutExportFormats::DXF_AC1009_Flat:
        case LayoutExportFormats::DXF_AC1012_Flat:
        case LayoutExportFormats::DXF_AC1014_Flat:
        case LayoutExportFormats::DXF_AC1015_Flat:
        case LayoutExportFormats::DXF_AC1018_Flat:
        case LayoutExportFormats::DXF_AC1021_Flat:
        case LayoutExportFormats::DXF_AC1024_Flat:
        case LayoutExportFormats::DXF_AC1027_Flat:
        case LayoutExportFormats::DXF_AAMA:
        case LayoutExportFormats::DXF_ASTM:
            return LayoutExportFileFormat::DXF;

        case LayoutExportFormats::NC:
            return LayoutExportFileFormat::NC;

        case LayoutExportFormats::RLD:
            return LayoutExportFileFormat::RLD;

        case LayoutExportFormats::TIF:
            return LayoutExportFileFormat::TIF;

        case LayoutExportFormats::HPGL:
        case LayoutExportFormats::HPGL2:
            return LayoutExportFileFormat::HPGL;

        case LayoutExportFormats::HPGL_PLT:
        case LayoutExportFormats::HPGL2_PLT:
            return LayoutExportFileFormat::PLT;

        case LayoutExportFormats::COUNT:
        default:
            Q_UNREACHABLE();
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::MapLayoutExportFormats(enum LayoutExportFileFormat format) -> QVector<LayoutExportFormats>
{
    Q_STATIC_ASSERT_X(static_cast<int>(LayoutExportFileFormat::COUNT) == 10, "Update to cover all cases.");

    static bool const pdfConversionSupported = VLayoutExporter::SupportPDFConversion();

    switch (format)
    {
        case LayoutExportFileFormat::SVG:
            return {LayoutExportFormats::SVG};
        case LayoutExportFileFormat::PDF:
        {
            QVector<LayoutExportFormats> formats{LayoutExportFormats::PDFTiled, LayoutExportFormats::PDF};

            if (pdfConversionSupported)
            {
                formats.append(LayoutExportFormats::PS);
                formats.append(LayoutExportFormats::EPS);
            }
            return formats;
        }
        case LayoutExportFileFormat::PNG:
            return {LayoutExportFormats::PNG};
        case LayoutExportFileFormat::OBJ:
            return {LayoutExportFormats::OBJ};
        case LayoutExportFileFormat::DXF:
            return {LayoutExportFormats::DXF_AAMA,
                    LayoutExportFormats::DXF_ASTM,
                    LayoutExportFormats::DXF_AC1006_Flat,
                    LayoutExportFormats::DXF_AC1009_Flat,
                    LayoutExportFormats::DXF_AC1012_Flat,
                    LayoutExportFormats::DXF_AC1014_Flat,
                    LayoutExportFormats::DXF_AC1015_Flat,
                    LayoutExportFormats::DXF_AC1018_Flat,
                    LayoutExportFormats::DXF_AC1021_Flat,
                    LayoutExportFormats::DXF_AC1024_Flat,
                    LayoutExportFormats::DXF_AC1027_Flat};
        case LayoutExportFileFormat::NC:
            return {LayoutExportFormats::NC};
        case LayoutExportFileFormat::RLD:
            return {LayoutExportFormats::RLD};
        case LayoutExportFileFormat::TIF:
            return {LayoutExportFormats::TIF};
        case LayoutExportFileFormat::HPGL:
            return {LayoutExportFormats::HPGL2, LayoutExportFormats::HPGL};
        case LayoutExportFileFormat::PLT:
            return {LayoutExportFormats::HPGL2_PLT, LayoutExportFormats::HPGL_PLT};
        case LayoutExportFileFormat::COUNT:
        default:
            Q_UNREACHABLE();
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutExporter::AllLayoutExportFormats() -> QVector<LayoutExportFormats>
{
    Q_STATIC_ASSERT_X(static_cast<int>(LayoutExportFormats::COUNT) == 41, "Update to cover all cases.");

    QVector<LayoutExportFormats> list;

    list.append(LayoutExportFormats::SVG);
    list.append(LayoutExportFormats::PDF);
    list.append(LayoutExportFormats::PNG);
    list.append(LayoutExportFormats::OBJ);

    if (static bool const pdfConversionSupported = VLayoutExporter::SupportPDFConversion(); pdfConversionSupported)
    {
        list.append(LayoutExportFormats::PS);
        list.append(LayoutExportFormats::EPS);
    }
    list.append(LayoutExportFormats::DXF_AC1006_Flat);
    list.append(LayoutExportFormats::DXF_AC1009_Flat);
    list.append(LayoutExportFormats::DXF_AC1012_Flat);
    list.append(LayoutExportFormats::DXF_AC1014_Flat);
    list.append(LayoutExportFormats::DXF_AC1015_Flat);
    list.append(LayoutExportFormats::DXF_AC1018_Flat);
    list.append(LayoutExportFormats::DXF_AC1021_Flat);
    list.append(LayoutExportFormats::DXF_AC1024_Flat);
    list.append(LayoutExportFormats::DXF_AC1027_Flat);
    list.append(LayoutExportFormats::DXF_AAMA);
    list.append(LayoutExportFormats::DXF_ASTM);
    list.append(LayoutExportFormats::PDFTiled);
    //    list.append(LayoutExportFormats::NC);
    list.append(LayoutExportFormats::RLD);
    list.append(LayoutExportFormats::TIF);
    list.append(LayoutExportFormats::HPGL);
    list.append(LayoutExportFormats::HPGL2);
    list.append(LayoutExportFormats::HPGL_PLT);
    list.append(LayoutExportFormats::HPGL2_PLT);

    return list;
}
