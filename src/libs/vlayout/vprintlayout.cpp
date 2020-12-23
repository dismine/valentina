/************************************************************************
 **
 **  @file   vprintlayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 12, 2020
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
#include "vprintlayout.h"

#include <QPrinterInfo>
#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>
#include <QPrintDialog>
#include <QGuiApplication>
#include <QDir>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QPixmapCache>
#include <QGraphicsScene>
#include <QPrintPreviewDialog>
#include <QPageSize>

#include "dialogs/dialoglayoutscale.h"
#include "../vmisc/vabstractvalapplication.h"
#include "dialogs/vabstractlayoutdialog.h"
#include "vposter.h"
#include "../vformat/vwatermark.h"
#include "../ifc/xml/vwatermarkconverter.h"
#include "../ifc/exception/vexception.h"
#include "../vmisc/vmath.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FindPageSizeId(QSizeF size) -> QPageSize::PageSizeId
{
    auto TestSize = [size](float width, float height)
    {
        return size == QSizeF(width, height) || size == QSizeF(height, width);
    };

    constexpr int A0Width = 841;
    constexpr int A0Height = 1189;
    if (TestSize(A0Width, A0Height))
    {
        return QPageSize::A0;
    }

    constexpr int A1Width = 594;
    constexpr int A1Height = 841;
    if (TestSize(A1Width, A1Height))
    {
        return QPageSize::A1;
    }

    constexpr int A2Width = 420;
    constexpr int A2Height = 594;
    if (TestSize(A2Width, A2Height))
    {
        return QPageSize::A2;
    }

    constexpr int A3Width = 297;
    constexpr int A3Height = 420;
    if (TestSize(A3Width, A3Height))
    {
        return QPageSize::A3;
    }

    constexpr int A4Width = 210;
    constexpr int A4Height = 297;
    if (TestSize(A4Width, A4Height))
    {
        return QPageSize::A4;
    }

    constexpr float LegalWidth = 215.9F;
    constexpr float LegalHeight = 355.6F;
    if (TestSize(LegalWidth, LegalHeight))
    {
        return QPageSize::Legal;
    }

    constexpr float LetterWidth = 215.9F;
    constexpr float LetterHeight = 279.4F;
    if (TestSize(LetterWidth, LetterHeight))
    {
        return QPageSize::Letter;
    }

    return QPageSize::Custom;
}

//---------------------------------------------------------------------------------------------------------------------
Q_REQUIRED_RESULT auto PreparePrinter(
        const QPrinterInfo &info, QPrinter::PrinterMode mode = QPrinter::ScreenResolution) -> QSharedPointer<QPrinter>;
auto PreparePrinter(const QPrinterInfo &info, QPrinter::PrinterMode mode) -> QSharedPointer<QPrinter>
{
    QPrinterInfo tmpInfo = info;
    if(tmpInfo.isNull() || tmpInfo.printerName().isEmpty())
    {
        const QStringList list = QPrinterInfo::availablePrinterNames();
        if(list.isEmpty())
        {
            return QSharedPointer<QPrinter>();
        }

        tmpInfo = QPrinterInfo::printerInfo(list.first());
    }

    auto printer = QSharedPointer<QPrinter>(new QPrinter(tmpInfo, mode));
    printer->setResolution(static_cast<int>(PrintDPI));
    return printer;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
VPrintLayout::VPrintLayout(QObject *parent)
    : QObject(parent)
{}

//---------------------------------------------------------------------------------------------------------------------
VPrintLayout::~VPrintLayout()
{
    qDeleteAll(m_layoutScenes);
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintTiled()
{
    m_isTiled = true;
    PrintLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintOrigin()
{
    if (not IsPagesUniform())
    {
        qCritical()<<tr("For printing multipages document all sheet should have the same size.");
        return;
    }

    m_isTiled = false;
    PrintLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintPreviewTiled()
{
    m_isTiled = true;
    PrintPreview();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintPreviewOrigin()
{
    if (not IsPagesUniform())
    {
        qCritical()<<tr("For previewing multipage document all sheet should have the same size.");
        return;
    }

    m_isTiled = false;
    PrintPreview();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PdfTiledFile(const QString &name)
{
    m_isTiled = true;

    if (m_isLayoutStale)
    {
        if (ContinueIfLayoutStale(m_parentWidget) == QMessageBox::No)
        {
            return;
        }
    }
    QPrinter printer;
    SetPrinterSettings(&printer, PrintType::PrintPDF, name);

    // Call IsPagesFit after setting a printer settings and check if pages is not bigger than printer's paper size
    if (not m_isTiled && not IsPagesFit(printer.pageLayout().paintRectPixels(printer.resolution()).size()))
    {
        qWarning()<<tr("Pages will be cropped because they do not fit printer paper size.");
    }

    printer.setResolution(static_cast<int>(PrintDPI));
    PrintPages(&printer);
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::CleanLayout()
{
    qDeleteAll(m_layoutScenes);
    m_layoutScenes.clear();
    m_layoutPapers.clear();
    m_layoutShadows.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintLayout()
{
    if (m_isLayoutStale)
    {
        if (ContinueIfLayoutStale(m_parentWidget) == QMessageBox::No)
        {
            return;
        }
    }
    // display print dialog and if accepted print
    QPrinterInfo info = QPrinterInfo::printerInfo(m_layoutPrinterName);
    if(info.isNull() || info.printerName().isEmpty())
    {
        info = QPrinterInfo::defaultPrinter();
    }
    QSharedPointer<QPrinter> printer = PreparePrinter(info, QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    {
        DialogLayoutScale layoutScale(m_isTiled, m_parentWidget);
        layoutScale.SetXScale(1);
        layoutScale.SetYScale(1);
        layoutScale.exec();

        m_xscale = layoutScale.GetXScale();
        m_yscale = layoutScale.GetYScale();
    }

    SetPrinterSettings(printer.data(), PrintType::PrintNative);
    QPrintDialog dialog(printer.data(), m_parentWidget);
    // If only user couldn't change page margins we could use method setMinMax();
    dialog.setOption(QPrintDialog::PrintCurrentPage, false);
    if (dialog.exec() == QDialog::Accepted)
    {
        printer->setResolution(static_cast<int>(PrintDPI));
        PrintPages(printer.data());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintPreview()
{
    if (m_isLayoutStale)
    {
        if (ContinueIfLayoutStale(m_parentWidget) == QMessageBox::No)
        {
            return;
        }
    }

    QPrinterInfo info = QPrinterInfo::printerInfo(m_layoutPrinterName);
    if(info.isNull() || info.printerName().isEmpty())
    {
        info = QPrinterInfo::defaultPrinter();
    }
    QSharedPointer<QPrinter> printer = PreparePrinter(info);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    {
        DialogLayoutScale layoutScale(m_isTiled, m_parentWidget);
        layoutScale.SetXScale(1);
        layoutScale.SetYScale(1);
        layoutScale.exec();

        m_xscale = layoutScale.GetXScale();
        m_yscale = layoutScale.GetYScale();
    }

    SetPrinterSettings(printer.data(), PrintType::PrintPreview);
    printer->setResolution(static_cast<int>(PrintDPI));

    // display print preview dialog
    QPrintPreviewDialog preview(printer.data());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &VPrintLayout::PrintPages);
    preview.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PrintPages(QPrinter *printer)
{
    // Here we try understand difference between printer's dpi and our.
    // Get printer rect acording to our dpi.
    const QRectF printerPageRect(0, 0, ToPixel(printer->pageRect(QPrinter::Millimeter).width(), Unit::Mm),
                                 ToPixel(printer->pageRect(QPrinter::Millimeter).height(), Unit::Mm));
    const QRect pageRect = printer->pageLayout().paintRectPixels(printer->resolution());
    const double xscale = pageRect.width() / printerPageRect.width();
    const double yscale = pageRect.height() / printerPageRect.height();

    QPainter painter;
    if (not painter.begin(printer))
    { // failed to open file
        qCritical() << tr("Failed to open file, is it writable?");
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, qApp->Settings()->WidthMainLine(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush ( QBrush ( Qt::NoBrush ) );

    int count = 0;
    QSharedPointer<QVector<PosterData>> poster;
    QSharedPointer<VPoster> posterazor;

    if (m_isTiled)
    {
        // when isTiled, the landscape tiles have to be rotated, because the pages
        // stay portrait in the pdf
        if(m_tiledPDFOrientation == PageOrientation::Landscape)
        {
            const int angle = -90;
            painter.rotate(angle);
            painter.translate(-ToPixel(printer->pageRect(QPrinter::Millimeter).height(), Unit::Mm), 0);
        }

        poster = QSharedPointer<QVector<PosterData>>(new QVector<PosterData>());
        posterazor = QSharedPointer<VPoster>(new VPoster(printer));

        for (int i=0; i < m_layoutScenes.size(); ++i)
        {
            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(i));
            if (paper != nullptr)
            {
                QRectF paperRect = paper->rect();
                QSizeF image(paperRect.width() * m_xscale, paperRect.height() * m_yscale);
                *poster += posterazor->Calc(image.toSize(), i, m_tiledPDFOrientation);
            }
        }

        count = poster->size();
    }
    else
    {
        count = m_layoutScenes.size();
    }

    // Handle the fromPage(), toPage(), supportsMultipleCopies(), and numCopies() values from QPrinter.
    int firstPage = printer->fromPage() - 1;
    if (firstPage >= count)
    {
        return;
    }
    if (firstPage == -1)
    {
        firstPage = 0;
    }

    int lastPage = printer->toPage() - 1;
    if (lastPage == -1 || lastPage >= count)
    {
        lastPage = count - 1;
    }

    const int numPages = lastPage - firstPage + 1;
    int copyCount = 1;
    if (not printer->supportsMultipleCopies())
    {
        copyCount = printer->copyCount();
    }

    VWatermarkData data;
    if (not m_watermarkPath.isEmpty())
    {
        try
        {
            VWatermarkConverter converter(m_watermarkPath);
            VWatermark watermark;
            watermark.setXMLContent(converter.Convert());
            data = watermark.GetWatermark();

            if (not data.path.isEmpty())
            {
                // Clean previous cache
                QPixmapCache::remove(AbsoluteMPath(m_watermarkPath, data.path));
            }
        }
        catch (VException &e)
        {
            const QString errorMsg = tr("File error.\n\n%1\n\n%2").arg(e.ErrorMessage(), e.DetailedInformation());
            qApp->IsPedantic() ? throw VException(errorMsg) :
                                 qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    }

    for (int i = 0; i < copyCount; ++i)
    {
        for (int j = 0; j < numPages; ++j)
        {
            if (i != 0 || j != 0)
            {
                if (not printer->newPage())
                {
                    qCritical() << tr("Failed in flushing page to disk, disk full?");
                    return;
                }
            }
            int index;
            if (printer->pageOrder() == QPrinter::FirstPageFirst)
            {
                index = firstPage + j;
            }
            else
            {
                index = lastPage - j;
            }

            int paperIndex = -1;
            m_isTiled ? paperIndex = static_cast<int>(poster->at(index).index) : paperIndex = index;

            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(paperIndex));
            if (paper != nullptr)
            {
                QVector<QGraphicsItem *> posterData;

                if (m_isTiled)
                {
                    // Draw tile
                    posterData = posterazor->Tile(paper, poster->at(index), m_layoutScenes.size(), data,
                                                  m_watermarkPath);
                }

                PreparePaper(paperIndex);

                // Render
                QRectF source;
                m_isTiled ? source = poster->at(index).rect : source = paper->rect();

                qreal x;
                qreal y;
                if(printer->fullPage())
                {
                    QPageLayout layout = printer->pageLayout();
                    layout.setUnits(QPageLayout::Millimeter);
                    QMarginsF printerMargins = layout.margins();
                    x = qFloor(ToPixel(printerMargins.left(),Unit::Mm));
                    y = qFloor(ToPixel(printerMargins.top(),Unit::Mm));
                }
                else
                {
                    x = 0; y = 0;
                }

                QRectF target(x * xscale, y * yscale, source.width() * xscale, source.height() * yscale);

                m_layoutScenes.at(paperIndex)->render(&painter, target, source, Qt::IgnoreAspectRatio);

                if (m_isTiled)
                {
                    // Remove borders
                    qDeleteAll(posterData);
                }

                // Restore
                RestorePaper(paperIndex);
            }
        }
    }

    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::SetPrinterSettings(QPrinter *printer, PrintType printType, const QString &filePath)
{
    SCASSERT(printer != nullptr)
    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());
    printer->setPageOrientation(m_isLayoutPortrait ? QPageLayout::Portrait : QPageLayout::Landscape);
    SetPrinterPageSize(printer);
    printer->setFullPage(m_ignorePrinterMargins);
    SetPrinterPrinterMargins(printer);
    SetPrinterOutputFileName(printer, printType, filePath);
    printer->setDocName(filePath.isEmpty() ? DocName() : QFileInfo(filePath).baseName());
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::SetPrinterPrinterMargins(QPrinter *printer)
{
    SCASSERT(printer != nullptr)
    QMarginsF printerMargins;

    if (not m_isTiled)
    {
        printerMargins = QMarginsF(UnitConvertor(m_layoutMargins, Unit::Px, Unit::Mm));
    }
    else
    {
        if(m_tiledPDFOrientation == PageOrientation::Landscape)
        {
            // because when painting we have a -90rotation in landscape mode,
            // see function PrintPages.
            printerMargins = QMarginsF(m_tiledMargins.bottom(), m_tiledMargins.left(), m_tiledMargins.top(),
                                       m_tiledMargins.right());
        }
        else
        {
            printerMargins = m_tiledMargins;
        }
    }

    const bool success = printer->setPageMargins(printerMargins, QPageLayout::Millimeter);
    if (not success)
    {
        qWarning() << tr("Cannot set printer margins");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::SetPrinterOutputFileName(QPrinter *printer, PrintType printType, const QString &filePath)
{
    SCASSERT(printer != nullptr)

    switch(printType)
    {
        case PrintType::PrintPDF:
        {
            const QString outputFileName = filePath.isEmpty() ? QDir::homePath() + QDir::separator() + DocName()
                                                              : filePath;
#ifdef Q_OS_WIN
            printer->setOutputFileName(outputFileName);
#else
            printer->setOutputFileName(outputFileName + QStringLiteral(".pdf"));
#endif

#ifdef Q_OS_MAC
            printer->setOutputFormat(QPrinter::NativeFormat);
#else
            printer->setOutputFormat(QPrinter::PdfFormat);
#endif
            break;
        }
        case PrintType::PrintNative:
            printer->setOutputFileName(QString());//Disable printing to file if was enabled.
            printer->setOutputFormat(QPrinter::NativeFormat);
            break;
        case PrintType::PrintPreview: /*do nothing*/
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::SetPrinterPageSize(QPrinter *printer)
{
    SCASSERT(printer != nullptr)

    if (not m_isTiled)
    {
        qreal width = FromPixel(m_layoutPaperSize.width(), Unit::Mm);
        qreal height = FromPixel(m_layoutPaperSize.height(), Unit::Mm);

        if (m_isAutoCropLength || m_isUnitePages)
        {
            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(0));
            if (paper != nullptr)
            {
                if (m_isLayoutPortrait)
                {
                    height = FromPixel(paper->rect().height() + m_layoutMargins.top() + m_layoutMargins.bottom(),
                                       Unit::Mm);
                }
                else
                {
                    width = FromPixel(paper->rect().width() + m_layoutMargins.left() + m_layoutMargins.right(),
                                      Unit::Mm);
                }
            }
        }

        if (m_isAutoCropWidth)
        {
            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(0));
            if (paper != nullptr)
            {
                if (m_isLayoutPortrait)
                {
                    width = FromPixel(paper->rect().width() + m_layoutMargins.left() + m_layoutMargins.right(),
                                      Unit::Mm);
                }
                else
                {
                    height = FromPixel(paper->rect().height() + m_layoutMargins.top() + m_layoutMargins.bottom(),
                                       Unit::Mm);
                }
            }
        }

        QSizeF size = QSizeF(width, height);

        if (not m_isLayoutPortrait)
        {
            size.transpose(); // QPrinter reverse this for landscape orientation
        }

        const QPageSize::PageSizeId pSZ = FindPageSizeId(size);
        if (pSZ == QPageSize::Custom)
        {
            if (not printer->setPageSize(QPageSize(size, QPageSize::Millimeter)))
            {
                qWarning() << tr("Cannot set custom printer page size");
            }
        }
        else
        {
            if (not printer->setPageSize(QPageSize(pSZ)))
            {
                qWarning() << tr("Cannot set printer page size");
            }
        }
    }
    else
    {
        if (not printer->setPageSize(QPageSize(m_tiledPDFPaperSize, QPageSize::Millimeter)))
        {
            qWarning() << tr("Cannot set printer tiled page size");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPrintLayout::DocName() const -> QString
{
    return not m_fileName.isEmpty() ? m_fileName : tr("unamed");
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::PreparePaper(int index) const
{
    auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(index));
    if (paper != nullptr)
    {
        QBrush brush(Qt::white);
        m_layoutScenes.at(index)->setBackgroundBrush(brush);
        m_layoutShadows.at(index)->setVisible(false);
        const float thinPen = 0.1F;
        paper->setPen(QPen(Qt::white, thinPen, Qt::NoPen));// border
    }

    QTransform matrix;
    matrix.scale(m_xscale, m_yscale);

    QList<QGraphicsItem *> paperDetails = m_layoutDetails.at(index);
    for (auto *detail : paperDetails)
    {
        QTransform m = detail->transform();
        m *= matrix;
        detail->setTransform(m);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPrintLayout::RestorePaper(int index) const
{
    auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(index));
    if (paper != nullptr)
    {
        // Restore
        paper->setPen(QPen(Qt::black, 1));
        QBrush brush(Qt::gray);
        m_layoutScenes.at(index)->setBackgroundBrush(brush);
        m_layoutShadows.at(index)->setVisible(true);
    }

    QTransform matrix;
    matrix.scale(1./m_xscale, 1./m_yscale);

    QList<QGraphicsItem *> paperDetails = m_layoutDetails.at(index);
    for (auto *detail : paperDetails)
    {
        QTransform m = detail->transform();
        m *= matrix;
        detail->setTransform(m);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPrintLayout::IsPagesUniform() const -> bool
{
    if (m_layoutPapers.size() < 2)
    {
        return true;
    }

    auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(0));
    SCASSERT(paper != nullptr)

    return std::all_of(m_layoutPapers.begin(), m_layoutPapers.end(), [paper](QGraphicsItem *paperItem)
    {
        auto *p = qgraphicsitem_cast<QGraphicsRectItem *>(paperItem);
        SCASSERT(p != nullptr)
        return paper->rect() == p->rect();
    });
}

//---------------------------------------------------------------------------------------------------------------------
auto VPrintLayout::IsPagesFit(QSizeF printPaper) const -> bool
{
    // On previous stage already was checked if pages have uniform size
    // Enough will be to check only one page
    auto *p = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutPapers.at(0));
    SCASSERT(p != nullptr)
    const QSizeF pSize = p->rect().size();
    return pSize.height() <= printPaper.height() && pSize.width() <= printPaper.width();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPrintLayout::ContinueIfLayoutStale(QWidget *parent) -> int
{
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("The layout is stale."));
    msgBox.setText(tr("The layout was not updated since last pattern modification. Do you want to continue?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    const int width = 500;
    auto* horizontalSpacer = new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto* layout = qobject_cast<QGridLayout*>(msgBox.layout());
    SCASSERT(layout != nullptr)
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    return msgBox.exec();
}
