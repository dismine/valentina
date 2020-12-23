/************************************************************************
 **
 **  @file   vprintlayout.h
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
#ifndef VPRINTLAYOUT_H
#define VPRINTLAYOUT_H

#include <QMargins>
#include <QObject>
#include <QSizeF>

#include "../vmisc/def.h"

enum class PrintType : qint8 {PrintPDF, PrintPreview, PrintNative};

class QPrinter;
class QGraphicsScene;

class VPrintLayout : public QObject
{
    Q_OBJECT
public:
    VPrintLayout(QObject *parent = nullptr);
    ~VPrintLayout();

    auto FileName() const -> QString;
    void SetFileName(const QString &fileName);

    auto LayoutPaperSize() const -> QSizeF;
    void SetLayoutPaperSize(const QSizeF &paperSize);

    auto LayoutMargins() const -> QMarginsF;
    void SetLayoutMargins(const QMarginsF &layoutMargins);

    auto ParentWidget() const -> QWidget *;
    void SetParentWidget(QWidget *parentWidget);

    auto IsLayoutPortrait() const -> bool;
    void SetLayoutPortrait(bool isLayoutPortrait);

    auto IgnorePrinterMargins() const -> bool;
    void SetIgnorePrinterMargins(bool ignorePrinterFields);

    auto IsAutoCropLength() const -> bool;
    void SetAutoCropLength(bool isAutoCropLength);

    auto IsAutoCropWidth() const -> bool;
    void SetAutoCropWidth(bool isAutoCropWidth);

    auto IsUnitePages() const -> bool;
    void SetUnitePages(bool isUnitePages);

    auto LayoutPrinterName() const -> QString;
    void SetLayoutPrinterName(const QString &layoutPrinterName);

    auto IsLayoutStale() const -> bool;
    void SetLayoutStale(bool isLayoutStale);

    auto TiledMargins() const -> QMarginsF;
    void SetTiledMargins(const QMarginsF &tiledMargins);

    auto TiledPDFOrientation() const -> PageOrientation;
    void SetTiledPDFOrientation(const PageOrientation &tiledPDFOrientation);

    auto TiledPDFPaperSize() const -> QSizeF;
    void SetTiledPDFPaperSize(const QSizeF &tiledPDFPaperSize);

    auto XScale() const -> qreal;
    void SetXScale(const qreal &xscale);

    auto YScale() const -> qreal;
    void SetYScale(const qreal &yscale);

    auto WatermarkPath() const -> QString;
    void SetWatermarkPath(const QString &watermarkPath);

    auto LayoutPapers() const -> QList<QGraphicsItem *>;
    void SetLayoutPapers(const QList<QGraphicsItem *> &layoutPapers);

    auto LayoutScenes() const -> QList<QGraphicsScene *>;
    void SetLayoutScenes(const QList<QGraphicsScene *> &layoutScenes);

    auto LayoutShadows() const -> QList<QGraphicsItem *>;
    void SetLayoutShadows(const QList<QGraphicsItem *> &layoutShadows);

    auto LayoutDetails() const -> QList<QList<QGraphicsItem *> >;
    void SetLayoutDetails(const QList<QList<QGraphicsItem *> > &layoutDetails);

    void PrintTiled();
    void PrintOrigin();

    void PrintPreviewTiled();
    void PrintPreviewOrigin();

    void PdfTiledFile(const QString &name);

    void CleanLayout();

    static auto ContinueIfLayoutStale(QWidget *parent) -> int;

private slots:
    void PrintPages (QPrinter *printer);

private:
    Q_DISABLE_COPY(VPrintLayout)

    QString         m_fileName{};
    QSizeF          m_layoutPaperSize{};
    QMarginsF       m_layoutMargins{};
    QWidget        *m_parentWidget{nullptr};
    bool            m_isLayoutPortrait{true};
    bool            m_ignorePrinterMargins{false};
    bool            m_isAutoCropLength{false};
    bool            m_isAutoCropWidth{false};
    bool            m_isUnitePages{false};
    QString         m_layoutPrinterName{};
    bool            m_isLayoutStale{true};
    QMarginsF       m_tiledMargins{};
    PageOrientation m_tiledPDFOrientation{PageOrientation::Portrait};
    QSizeF          m_tiledPDFPaperSize{};
    QString         m_watermarkPath{};


    QList<QGraphicsItem *>         m_layoutPapers{};
    QList<QGraphicsScene *>        m_layoutScenes{};
    QList<QGraphicsItem *>         m_layoutShadows{};
    QList<QList<QGraphicsItem *> > m_layoutDetails{};

    bool  m_isTiled{false};
    qreal m_xscale{1};
    qreal m_yscale{1};

    void PrintLayout();
    void PrintPreview();

    void SetPrinterSettings(QPrinter *printer, PrintType printType, const QString &filePath = QString());
    void SetPrinterPrinterMargins(QPrinter *printer);
    void SetPrinterOutputFileName(QPrinter *printer, PrintType printType, const QString &filePath = QString());
    void SetPrinterPageSize(QPrinter *printer);

    QString DocName() const;

    void PreparePaper(int index) const;
    void RestorePaper(int index) const;

    bool IsPagesUniform() const;
    bool IsPagesFit(QSizeF printPaper) const;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::FileName() const -> QString
{
    return m_fileName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetFileName(const QString &fileName)
{
    m_fileName = fileName;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutPaperSize() const -> QSizeF
{
    return m_layoutPaperSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutPaperSize(const QSizeF &paperSize)
{
    m_layoutPaperSize = paperSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutMargins() const -> QMarginsF
{
    return m_layoutMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutMargins(const QMarginsF &layoutMargins)
{
    m_layoutMargins = layoutMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::ParentWidget() const -> QWidget *
{
    return m_parentWidget;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetParentWidget(QWidget *parentWidget)
{
    m_parentWidget = parentWidget;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::IsLayoutPortrait() const -> bool
{
    return m_isLayoutPortrait;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutPortrait(bool isLayoutPortrait)
{
    m_isLayoutPortrait = isLayoutPortrait;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::IgnorePrinterMargins() const -> bool
{
    return m_ignorePrinterMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetIgnorePrinterMargins(bool ignorePrinterFields)
{
    m_ignorePrinterMargins = ignorePrinterFields;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::IsAutoCropLength() const -> bool
{
    return m_isAutoCropLength;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetAutoCropLength(bool isAutoCropLength)
{
    m_isAutoCropLength = isAutoCropLength;
}

//---------------------------------------------------------------------------------------------------------------------
inline bool VPrintLayout::IsAutoCropWidth() const
{
    return m_isAutoCropWidth;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetAutoCropWidth(bool isAutoCropWidth)
{
    m_isAutoCropWidth = isAutoCropWidth;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::IsUnitePages() const -> bool
{
    return m_isUnitePages;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetUnitePages(bool isUnitePages)
{
    m_isUnitePages = isUnitePages;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutPrinterName() const -> QString
{
    return m_layoutPrinterName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutPrinterName(const QString &layoutPrinterName)
{
    m_layoutPrinterName = layoutPrinterName;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::IsLayoutStale() const -> bool
{
    return m_isLayoutStale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutStale(bool isLayoutStale)
{
    m_isLayoutStale = isLayoutStale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::TiledMargins() const -> QMarginsF
{
    return m_tiledMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetTiledMargins(const QMarginsF &tiledMargins)
{
    m_tiledMargins = tiledMargins;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::TiledPDFOrientation() const -> PageOrientation
{
    return m_tiledPDFOrientation;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetTiledPDFOrientation(const PageOrientation &tiledPDFOrientation)
{
    m_tiledPDFOrientation = tiledPDFOrientation;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::TiledPDFPaperSize() const -> QSizeF
{
    return m_tiledPDFPaperSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetTiledPDFPaperSize(const QSizeF &tiledPDFPaperSize)
{
    m_tiledPDFPaperSize = tiledPDFPaperSize;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutPapers() const -> QList<QGraphicsItem *>
{
    return m_layoutPapers;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutPapers(const QList<QGraphicsItem *> &layoutPapers)
{
    m_layoutPapers = layoutPapers;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutScenes() const -> QList<QGraphicsScene *>
{
    return m_layoutScenes;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutScenes(const QList<QGraphicsScene *> &layoutScenes)
{
    m_layoutScenes = layoutScenes;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutShadows() const -> QList<QGraphicsItem *>
{
    return m_layoutShadows;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutShadows(const QList<QGraphicsItem *> &layoutShadows)
{
    m_layoutShadows = layoutShadows;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::LayoutDetails() const -> QList<QList<QGraphicsItem *> >
{
    return m_layoutDetails;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetLayoutDetails(const QList<QList<QGraphicsItem *> > &layoutDetails)
{
    m_layoutDetails = layoutDetails;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::YScale() const -> qreal
{
    return m_yscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetYScale(const qreal &yscale)
{
    m_yscale = yscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::XScale() const -> qreal
{
    return m_xscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetXScale(const qreal &xscale)
{
    m_xscale = xscale;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPrintLayout::WatermarkPath() const -> QString
{
    return m_watermarkPath;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPrintLayout::SetWatermarkPath(const QString &watermarkPath)
{
    m_watermarkPath = watermarkPath;
}
#endif // VPRINTLAYOUT_H
