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

#include <QFileInfo>
#include <QGraphicsScene>
#include <QGuiApplication>
#include <QPainter>
#include <QPrinter>
#include <QProcess>
#include <QSvgGenerator>
#include <QTemporaryFile>
#include <QtDebug>
#include <QCursor>
#include <QGraphicsItem>
#include <QImageWriter>

#include "../vmisc/vmath.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/def.h"
#include "../vobj/vobjpaintdevice.h"
#include "../vdxf/vdxfpaintdevice.h"
#include "vrawlayout.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../ifc/exception/vexception.h"
#include "vprintlayout.h"

namespace
{
#ifdef Q_OS_WIN
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PDFTOPS, (QLatin1String("pdftops.exe")))
#else
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PDFTOPS, (QLatin1String("pdftops")))
#endif

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PrepareTextForDXF prepare QGraphicsSimpleTextItem items for export to flat dxf.
 *
 * Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark end of each string by adding special
 * placholder. This method append it.
 *
 * @param placeholder placeholder that will be appended to each QGraphicsSimpleTextItem item's text string.
 */
void PrepareTextForDXF(const QString &placeholder, const QList<QGraphicsItem *> &paperItems)
{
    for (auto *item : paperItems)
    {
        QList<QGraphicsItem *> pieceChildren = item->childItems();
        for (auto *child : qAsConst(pieceChildren))
        {
            if (child->type() == QGraphicsSimpleTextItem::Type)
            {
                if(auto *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(child))
                {
                    textItem->setText(textItem->text() + placeholder);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RestoreTextAfterDXF restore QGraphicsSimpleTextItem items after export to flat dxf.
 *
 * Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark end of each string by adding special
 * placholder. This method remove it.
 *
 * @param placeholder placeholder that will be removed from each QGraphicsSimpleTextItem item's text string.
 */
void RestoreTextAfterDXF(const QString &placeholder, const QList<QGraphicsItem *> &paperItems)
{
    for (auto *item : paperItems)
    {
        QList<QGraphicsItem *> pieceChildren = item->childItems();
        for (auto *child : qAsConst(pieceChildren))
        {
            if (child->type() == QGraphicsSimpleTextItem::Type)
            {
                if(auto *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(child))
                {
                    QString text = textItem->text();
                    text.replace(placeholder, QString());
                    textItem->setText(text);
                }
            }
        }
    }
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToSVG(QGraphicsScene *scene) const
{
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
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPNG(QGraphicsScene *scene) const
{
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
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToTIF(QGraphicsScene *scene) const
{
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
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPDF(QGraphicsScene *scene) const
{
    ExportToPDF(scene, m_fileName);
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
void VLayoutExporter::ExportToPS(QGraphicsScene *scene) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        const QString fileName = m_fileName;
        ExportToPDF(scene, tmp.fileName());
        PdfToPs(QStringList{tmp.fileName(), fileName});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToEPS(QGraphicsScene *scene) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        const QString fileName = m_fileName;
        ExportToPDF(scene, tmp.fileName());
        PdfToPs(QStringList{QStringLiteral("-eps"), tmp.fileName(), fileName});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToFlatDXF(QGraphicsScene *scene, const QList<QGraphicsItem *> &details) const
{
    PrepareTextForDXF(endStringPlaceholder, details);

    VDxfPaintDevice generator;
    generator.setFileName(m_fileName);
    generator.setSize(QSize(qFloor(m_imageRect.width() * m_xScale), qFloor(m_imageRect.height() * m_yScale)));
    generator.setResolution(PrintDPI);
    generator.SetVersion(static_cast<DRW::Version>(m_dxfVersion));
    generator.SetBinaryFormat(m_binaryDxfFormat);
    generator.setInsunits(VarInsunits::Millimeters);// Decided to always use mm. See issue #745

    QPainter painter;
    if (painter.begin(&generator))
    {
        painter.scale(m_xScale, m_yScale);
        scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);
        if (not painter.end())
        {
            qCritical() << tr("Can't create an flat dxf file.")
                        << generator.ErrorString();
        }
    }

    RestoreTextAfterDXF(endStringPlaceholder, details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToAAMADXF(const QVector<VLayoutPiece> &details) const
{
    VDxfPaintDevice generator;
    generator.setFileName(m_fileName);
    generator.setSize(QSize(qCeil(m_imageRect.width() * m_xScale), qCeil(m_imageRect.height() * m_yScale)));
    generator.setResolution(PrintDPI);
    generator.SetVersion(static_cast<DRW::Version>(m_dxfVersion));
    generator.SetBinaryFormat(m_binaryDxfFormat);
    generator.setInsunits(VarInsunits::Millimeters);// Decided to always use mm. See issue #745
    generator.SetXScale(m_xScale);
    generator.SetYScale(m_yScale);
    if (not generator.ExportToAAMA(details))
    {
        qCritical() << tr("Can't create an AAMA dxf file.")
                    << generator.ErrorString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToASTMDXF(const QVector<VLayoutPiece> &details) const
{
    VDxfPaintDevice generator;
    generator.setFileName(m_fileName);
    generator.setSize(QSize(qCeil(m_imageRect.width() * m_xScale), qCeil(m_imageRect.height() * m_yScale)));
    generator.setResolution(PrintDPI);
    generator.SetVersion(static_cast<DRW::Version>(m_dxfVersion));
    generator.SetBinaryFormat(m_binaryDxfFormat);
    generator.setInsunits(VarInsunits::Millimeters);// Decided to always use mm. See issue #745
    generator.SetXScale(m_xScale);
    generator.SetYScale(m_yScale);
    if (not generator.ExportToASTM(details))
    {
        qCritical() << tr("Can't create an ASTM dxf file.")
                    << generator.ErrorString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToRLD(const QVector<VLayoutPiece> &details) const
{
    QVector<VLayoutPiece> scaledPieces;
    scaledPieces.reserve(details.size());

    for(auto detail : details)
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
auto VLayoutExporter::SupportPDFConversion() -> bool
{
    bool res = false;

    QProcess proc;
#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
    // Seek pdftops in app bundle or near valentina.exe
    proc.start(qApp->applicationDirPath() + QLatin1String("/")+ *PDFTOPS, QStringList());
#else
    proc.start(*PDFTOPS, QStringList()); // Seek pdftops in standard path
#endif

    const int timeout = 15000;
    if (proc.waitForStarted(timeout) && (proc.waitForFinished(timeout) || proc.state() == QProcess::NotRunning))
    {
        res = true;
    }
    else
    {
        qDebug() << *PDFTOPS << "error" << proc.error() << proc.errorString();
    }
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VLayoutExporter::offset() const
{
    return m_offset;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::SetOffset(const QPointF &newOffset)
{
    m_offset = newOffset;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PdfToPs use external tool "pdftops" for converting pdf too eps or ps format.
 * @param params string with parameter for tool. Parameters have format: "-eps input_file out_file". Use -eps when
 * need create eps file.
 */
void VLayoutExporter::PdfToPs(const QStringList &params)
{
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    QProcess proc;
#if defined(Q_OS_MAC)
    // Fix issue #594. Broken export on Mac.
    proc.setWorkingDirectory(qApp->applicationDirPath());
    proc.start(QLatin1String("./") + *PDFTOPS, params);
#else
    proc.start(*PDFTOPS, params);
#endif

    const int timeout = 15000;
    if (proc.waitForStarted(timeout))
    {
        proc.waitForFinished(timeout);
    }

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    QFile f(ConstLast<QString>(params));
    if (not f.exists())
    {
        qCritical() << qUtf8Printable(tr("Creating file '%1' failed! %2")
                                      .arg(ConstLast<QString>(params), proc.errorString()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToPDF(QGraphicsScene *scene, const QString &filename) const
{
    QPrinter printer;
    printer.setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                       QCoreApplication::applicationVersion());
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setDocName(QFileInfo(filename).fileName());
    printer.setResolution(static_cast<int>(PrintDPI));
    printer.setFullPage(m_ignorePrinterMargins);

    QPageLayout::Orientation imageOrientation = m_imageRect.height() >= m_imageRect.width() ? QPageLayout::Portrait
                                                                                            : QPageLayout::Landscape;

    qreal width = FromPixel(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right(), Unit::Mm);
    qreal height = FromPixel(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom(), Unit::Mm);

    QSizeF pageSize = imageOrientation == QPageLayout::Portrait ? QSizeF(width, height) : QSizeF(height, width);
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
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(m_pen);
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.scale(m_xScale, m_yScale);
    scene->render(&painter, VPrintLayout::SceneTargetRect(&printer, m_imageRect), m_imageRect, Qt::IgnoreAspectRatio);
    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
QString VLayoutExporter::ExportFormatDescription(LayoutExportFormats format)
{
    const QString dxfSuffix = QStringLiteral("(*.dxf)");
    const QString dxfFlatFilesStr = tr("(flat) files");
    const QString filesStr = tr("files");

    switch(format)
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
            return QStringLiteral("AutoCAD DXF R10 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1009_Flat:
            return QStringLiteral("AutoCAD DXF R11/12 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1012_Flat:
            return QStringLiteral("AutoCAD DXF R13 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1014_Flat:
            return QStringLiteral("AutoCAD DXF R14 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1015_Flat:
            return QStringLiteral("AutoCAD DXF 2000 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1018_Flat:
            return QStringLiteral("AutoCAD DXF 2004 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1021_Flat:
            return QStringLiteral("AutoCAD DXF 2007 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1024_Flat:
            return QStringLiteral("AutoCAD DXF 2010 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1027_Flat:
            return QStringLiteral("AutoCAD DXF 2013 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1006_AAMA:
            return QStringLiteral("AutoCAD DXF R10 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1009_AAMA:
            return QStringLiteral("AutoCAD DXF R11/12 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1012_AAMA:
            return QStringLiteral("AutoCAD DXF R13 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1014_AAMA:
            return QStringLiteral("AutoCAD DXF R14 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1015_AAMA:
            return QStringLiteral("AutoCAD DXF 2000 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1018_AAMA:
            return QStringLiteral("AutoCAD DXF 2004 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1021_AAMA:
            return QStringLiteral("AutoCAD DXF 2007 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1024_AAMA:
            return QStringLiteral("AutoCAD DXF 2010 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1027_AAMA:
            return QStringLiteral("AutoCAD DXF 2013 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1006_ASTM:
            return QStringLiteral("AutoCAD DXF R10 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1009_ASTM:
            return QStringLiteral("AutoCAD DXF R11/12 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1012_ASTM:
            return QStringLiteral("AutoCAD DXF R13 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1014_ASTM:
            return QStringLiteral("AutoCAD DXF R14 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1015_ASTM:
            return QStringLiteral("AutoCAD DXF 2000 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1018_ASTM:
            return QStringLiteral("AutoCAD DXF 2004 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1021_ASTM:
            return QStringLiteral("AutoCAD DXF 2007 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1024_ASTM:
            return QStringLiteral("AutoCAD DXF 2010 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::DXF_AC1027_ASTM:
            return QStringLiteral("AutoCAD DXF 2013 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormats::PDFTiled:
            return QStringLiteral("PDF %1 %2 (*.pdf)").arg(tr("tiled"), filesStr);
        case LayoutExportFormats::NC:
            return QStringLiteral("%1 %2 (*.nc)").arg(tr("Numerical control"), filesStr);
        case LayoutExportFormats::RLD:
            return QStringLiteral("%1 %2 (*.rld)").arg(tr("Raw Layout Data"), filesStr);
        case LayoutExportFormats::TIF:
            return QStringLiteral("TIFF %1 (*.tif)").arg(filesStr);
        default:
            return QString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VLayoutExporter::ExportFormatSuffix(LayoutExportFormats format)
{
    switch(format)
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
        case LayoutExportFormats::DXF_AC1006_AAMA:
        case LayoutExportFormats::DXF_AC1009_AAMA:
        case LayoutExportFormats::DXF_AC1012_AAMA:
        case LayoutExportFormats::DXF_AC1014_AAMA:
        case LayoutExportFormats::DXF_AC1015_AAMA:
        case LayoutExportFormats::DXF_AC1018_AAMA:
        case LayoutExportFormats::DXF_AC1021_AAMA:
        case LayoutExportFormats::DXF_AC1024_AAMA:
        case LayoutExportFormats::DXF_AC1027_AAMA:
        case LayoutExportFormats::DXF_AC1006_ASTM:
        case LayoutExportFormats::DXF_AC1009_ASTM:
        case LayoutExportFormats::DXF_AC1012_ASTM:
        case LayoutExportFormats::DXF_AC1014_ASTM:
        case LayoutExportFormats::DXF_AC1015_ASTM:
        case LayoutExportFormats::DXF_AC1018_ASTM:
        case LayoutExportFormats::DXF_AC1021_ASTM:
        case LayoutExportFormats::DXF_AC1024_ASTM:
        case LayoutExportFormats::DXF_AC1027_ASTM:
            return QStringLiteral(".dxf");
        case LayoutExportFormats::NC:
            return QStringLiteral(".nc");
        case LayoutExportFormats::RLD:
            return QStringLiteral(".rld");
        case LayoutExportFormats::TIF:
            return QStringLiteral(".tif");
        default:
            return QString();
    }
}
