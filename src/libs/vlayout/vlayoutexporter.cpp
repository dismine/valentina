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
void PrepareTextForDXF(const QString &placeholder, const QList<QList<QGraphicsItem *> > &details)
{
    for (const auto &paperItems : details)
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
void RestoreTextAfterDXF(const QString &placeholder, const QList<QList<QGraphicsItem *> > &details)
{
    for (const auto &paperItems : details)
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
    QPrinter printer;
    printer.setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                       QCoreApplication::applicationVersion());
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(m_fileName);
    printer.setDocName(QFileInfo(m_fileName).fileName());
    printer.setResolution(static_cast<int>(PrintDPI));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(m_ignorePrinterMargins);

    qreal width = FromPixel(m_imageRect.width() * m_xScale + m_margins.left() + m_margins.right(), Unit::Mm);
    qreal height = FromPixel(m_imageRect.height() * m_yScale + m_margins.top() + m_margins.bottom(), Unit::Mm);

    if (not printer.setPageSize(QPageSize(QSizeF(width, height), QPageSize::Millimeter)))
    {
        qWarning() << tr("Cannot set printer page size");
    }

    const qreal left = FromPixel(m_margins.left(), Unit::Mm);
    const qreal top = FromPixel(m_margins.top(), Unit::Mm);
    const qreal right = FromPixel(m_margins.right(), Unit::Mm);
    const qreal bottom = FromPixel(m_margins.bottom(), Unit::Mm);

    if (not printer.setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Millimeter))
    {
        qWarning() << tr("Cannot set printer margins");
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
    scene->render(&painter, m_imageRect, m_imageRect, Qt::IgnoreAspectRatio);
    painter.end();
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
        ExportToPDF(scene);
        PdfToPs(QStringList{tmp.fileName(), m_fileName});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToEPS(QGraphicsScene *scene) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        ExportToPDF(scene);
        PdfToPs(QStringList{QStringLiteral("-eps"), tmp.fileName(), m_fileName});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToFlatDXF(QGraphicsScene *scene, const QList<QList<QGraphicsItem *> > &details) const
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
        painter.end();
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
    generator.ExportToAAMA(details);
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
    generator.ExportToASTM(details);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutExporter::ExportToRLD(const QVector<VLayoutPiece> &details) const
{
    for(auto detail : details)
    {
        detail.Scale(m_xScale, m_yScale);
    }

    VRawLayoutData layoutData;
    layoutData.pieces = details;

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

    QFile f(params.last());
    if (not f.exists())
    {
        qCritical() << qUtf8Printable(tr("Creating file '%1' failed! %2").arg(params.last(), proc.errorString()));
    }
}
