/************************************************************************
 **
 **  @file   vposter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 4, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vposter.h"

#include <QDebug>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QImageReader>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPixmapCache>
#include <QPrinter>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QtMath>

#include "../ifc/exception/vexception.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractvalapplication.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ToPixel(qreal val) -> qreal
{
    return val / 25.4 * PrintDPI; // Mm to pixels with current dpi.
}

//---------------------------------------------------------------------------------------------------------------------
auto Grayscale(QImage image) -> QImage
{
    for (int ii = 0; ii < image.height(); ii++)
    {
        uchar *scan = image.scanLine(ii);
        int const depth = 4;
        for (int jj = 0; jj < image.width(); jj++)
        {
            auto *rgbpixel = reinterpret_cast<QRgb *>(scan + jj * depth);
            int const gray = qGray(*rgbpixel);
            *rgbpixel = QColor(gray, gray, gray, qAlpha(*rgbpixel)).rgba();
        }
    }

    return image;
}

//---------------------------------------------------------------------------------------------------------------------
auto WatermarkImageFromCache(const VWatermarkData &watermarkData, const QString &watermarkPath, QString &error)
    -> QPixmap
{
    QPixmap pixmap;
    QString const imagePath = AbsoluteMPath(watermarkPath, watermarkData.path);

    if (not QPixmapCache::find(imagePath, &pixmap))
    {
        QImageReader imageReader(imagePath);
        QImage watermark = imageReader.read();
        if (watermark.isNull())
        {
            error = imageReader.errorString();
            return pixmap;
        }

        if (watermarkData.grayscale)
        {
            watermark = Grayscale(watermark);
        }

        // Workaround for QGraphicsPixmapItem opacity problem.
        // Opacity applied only if use a cached pixmap and only after first draw. First image always has opacity 1.
        // Preparing an image manually allows to avoid the problem.
        QImage tmp(watermark.width(), watermark.height(), watermark.format());
        tmp = tmp.convertToFormat(QImage::Format_ARGB32);
        tmp.fill(Qt::transparent);
        QPainter p;
        p.begin(&tmp);
        p.setOpacity(watermarkData.opacity / 100.);
        p.drawImage(QPointF(), watermark);
        p.end();

        pixmap = QPixmap::fromImage(tmp);

        QPixmapCache::insert(imagePath, pixmap);
    }
    return pixmap;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPoster::VPoster(const QPrinter *printer)
  : printer(printer),
    allowance(static_cast<quint32>(qRound(CmToPixel(1.))))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::Calc(const QSize &imageRect, int page, PageOrientation orientation) const -> QVector<PosterData>
{
    QVector<PosterData> poster;

    if (printer == nullptr)
    {
        return poster;
    }

    const int rows = CountRows(imageRect.height(), orientation);
    const int columns = CountColumns(imageRect.width(), orientation);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            PosterData data = Cut(i, j, imageRect, orientation);
            data.index = static_cast<quint32>(page);
            data.rows = static_cast<quint32>(rows);
            data.columns = static_cast<quint32>(columns);
            poster.append(data);
        }
    }

    return poster;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::Tile(QGraphicsItem *parent, const PosterData &img, vsizetype sheets, const VWatermarkData &watermarkData,
                   const QString &watermarkPath) const -> QVector<QGraphicsItem *>
{
    QVector<QGraphicsItem *> data;
    data.append(Borders(parent, img, sheets));

    if (watermarkData.opacity > 0)
    {
        if (watermarkData.showImage && not watermarkData.path.isEmpty())
        {
            data += ImageWatermark(parent, img, watermarkData, watermarkPath);
        }

        if (watermarkData.showText && not watermarkData.text.isEmpty())
        {
            data += TextWatermark(parent, img, watermarkData);
        }
    }

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::Borders(QGraphicsItem *parent, const PosterData &img, vsizetype sheets) const -> QVector<QGraphicsItem *>
{
    SCASSERT(parent != nullptr)

    QVector<QGraphicsItem *> data;
    QPen pen(Qt::NoBrush, 1, Qt::DashLine);
    pen.setColor(Qt::black);

    if (img.columns == 1 && img.rows == 1)
    {
        return data;
    }

    const QRect rec = img.rect;
    if (img.column != 0)
    { // Left border
        auto *line = new QGraphicsLineItem(parent);
        line->setPen(pen);
        line->setLine(rec.x(), rec.y(), rec.x(), rec.y() + rec.height());
        data.append(line);

        auto *scissors = new QGraphicsPixmapItem(QPixmap(QStringLiteral("://scissors_vertical.png")), parent);
        scissors->setPos(rec.x(), rec.y() + rec.height() - static_cast<int>(allowance));
        data.append(scissors);
    }

    if (img.column != img.columns - 1)
    { // Right border
        auto *line = new QGraphicsLineItem(parent);
        line->setPen(pen);
        line->setLine(rec.x() + rec.width() - static_cast<int>(allowance), rec.y(),
                      rec.x() + rec.width() - static_cast<int>(allowance), rec.y() + rec.height());
        data.append(line);
    }

    if (img.row != 0)
    { // Top border
        auto *line = new QGraphicsLineItem(parent);
        line->setPen(pen);
        line->setLine(rec.x(), rec.y(), rec.x() + rec.width(), rec.y());
        data.append(line);

        auto *scissors = new QGraphicsPixmapItem(QPixmap(QStringLiteral("://scissors_horizontal.png")), parent);
        scissors->setPos(rec.x() + rec.width() - static_cast<int>(allowance), rec.y());
        data.append(scissors);
    }

    // Bottom border (mandatory)
    auto *line = new QGraphicsLineItem(parent);
    line->setPen(pen);
    line->setLine(rec.x(), rec.y() + rec.height() - static_cast<int>(allowance), rec.x() + rec.width(),
                  rec.y() + rec.height() - static_cast<int>(allowance));
    data.append(line);

    // Ruler
    Ruler(data, parent, rec);

    // Labels
    auto *labels = new QGraphicsTextItem(parent);

    const int layoutX = 15;
    const int layoutY = 5;
    labels->setPos(rec.x() + layoutX, rec.y() + rec.height() - static_cast<int>(allowance) + layoutY);
    labels->setTextWidth(rec.width() - (static_cast<int>(allowance) + layoutX));

    const QString grid =
        QCoreApplication::translate("VPoster", "Grid ( %1 , %2 )").arg(img.row + 1).arg(img.column + 1);
    const QString page = QCoreApplication::translate("VPoster", "Page %1 of %2")
                             .arg(img.row * (img.columns) + img.column + 1)
                             .arg(img.rows * img.columns);

    QString sheet;
    if (sheets > 1)
    {
        sheet = QCoreApplication::translate("VPoster", "Sheet %1 of %2").arg(img.index + 1).arg(sheets);
    }

    labels->setHtml(u"<table width='100%' style='color:rgb(0,0,0);'>"
                    u"<tr>"
                    u"<td>%1</td><td align='center'>%2</td><td align='right'>%3</td>"
                    u"</tr>"
                    u"</table>"_s.arg(grid, page, sheet));

    data.append(labels);

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::TextWatermark(QGraphicsItem *parent, const PosterData &img, const VWatermarkData &watermarkData) const
    -> QVector<QGraphicsItem *>
{
    SCASSERT(parent != nullptr)

    QVector<QGraphicsItem *> data;

    auto *text = new QGraphicsSimpleTextItem(watermarkData.text, parent);
    text->setFont(watermarkData.font);

    QPen pen = text->pen();
    pen.setColor(watermarkData.textColor);
    text->setPen(pen);

    text->setOpacity(watermarkData.opacity / 100.);
    text->setTransformOriginPoint(text->boundingRect().center());
    text->setRotation(-watermarkData.textRotation);

    const QRect boundingRect = text->boundingRect().toRect();
    int const x = img.rect.x() + (img.rect.width() - boundingRect.width()) / 2;
    int const y = img.rect.y() + (img.rect.height() - boundingRect.height()) / 2;

    text->setX(x);
    text->setY(y);
    text->setZValue(-1);

    data.append(text);

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::ImageWatermark(QGraphicsItem *parent, const PosterData &img, const VWatermarkData &watermarkData,
                             const QString &watermarkPath) const -> QVector<QGraphicsItem *>
{
    SCASSERT(parent != nullptr)

    QVector<QGraphicsItem *> data;

    QGraphicsItem *image = nullptr;

    QFileInfo const f(watermarkData.path);
    if (f.suffix() == "png" || f.suffix() == "jpg" || f.suffix() == "jpeg" || f.suffix() == "bmp")
    {
        QString error;
        QPixmap const watermark = WatermarkImageFromCache(watermarkData, watermarkPath, error);

        if (watermark.isNull())
        {
            const QString errorMsg =
                QCoreApplication::translate("VPoster", "Cannot open the watermark image.") + ' '_L1 + error;
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            return data;
        }

        image = new QGraphicsPixmapItem(watermark, parent);
    }
    else
    {
        const QString errorMsg =
            QCoreApplication::translate("VPoster", "Not supported file suffix '%1'").arg(f.suffix());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return data;
    }

    image->setZValue(-1);
    image->setTransformOriginPoint(image->boundingRect().center());
    image->setRotation(-watermarkData.imageRotation);

    const QRect boundingRect = image->boundingRect().toRect();
    int const x = img.rect.x() + (img.rect.width() - boundingRect.width()) / 2;
    int const y = img.rect.y() + (img.rect.height() - boundingRect.height()) / 2;

    image->setX(x);
    image->setY(y);

    data.append(image);

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::CountRows(int height, PageOrientation orientation) const -> int
{
    const qreal imgLength = height;
    qreal pageLength = 0;

    if (orientation == PageOrientation::Landscape)
    {
        pageLength = PageRect().width();
    }
    else
    {
        pageLength = PageRect().height();
    }

    if (pageLength >= imgLength)
    {
        return 1;
    }

    return qCeil(imgLength / (pageLength - static_cast<int>(allowance)));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::CountColumns(int width, PageOrientation orientation) const -> int
{
    const qreal imgLength = width;
    qreal pageLength = 0;

    if (orientation == PageOrientation::Landscape)
    {
        pageLength = PageRect().height();
    }
    else
    {
        pageLength = PageRect().width();
    }

    if (pageLength >= imgLength)
    {
        return 1;
    }

    return qCeil(imgLength / (pageLength - static_cast<int>(allowance)));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::Cut(int i, int j, const QSize &imageRect, PageOrientation orientation) const -> PosterData
{
    Q_UNUSED(imageRect)

    int pageLengthX, pageLengthY;

    if (orientation == PageOrientation::Landscape)
    {
        pageLengthX = PageRect().height();
        pageLengthY = PageRect().width();
    }
    else
    {
        pageLengthX = PageRect().width();
        pageLengthY = PageRect().height();
    }

    const int x = j * pageLengthX - j * static_cast<int>(allowance);
    const int y = i * pageLengthY - i * static_cast<int>(allowance);

    SCASSERT(x <= imageRect.width())
    SCASSERT(y <= imageRect.height())

    PosterData data;
    data.row = static_cast<quint32>(i);
    data.column = static_cast<quint32>(j);
    data.rect = QRect(x, y, pageLengthX, pageLengthY);

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPoster::PageRect() const -> QRect
{
    // Because the Point unit is defined to be 1/72th of an inch
    // we can't use method pageRect(QPrinter::Point). Our dpi value can be different.
    // We convert value yourself to pixels.
    const QRectF rect = printer->pageRect(QPrinter::Millimeter);

    if (printer->fullPage())
    {
        QPageLayout layout = printer->pageLayout();
        layout.setUnits(QPageLayout::Millimeter);
        QMarginsF const pMargins = layout.margins();
        QRectF const newRect = rect.marginsRemoved(pMargins);
        const QRect pageRectFP(0, 0, qFloor(ToPixel(newRect.width())), qFloor(ToPixel(newRect.height())));
        return pageRectFP;
    }

    return {0, 0, qFloor(ToPixel(rect.width())), qFloor(ToPixel(rect.height()))};
}

//---------------------------------------------------------------------------------------------------------------------
void VPoster::Ruler(QVector<QGraphicsItem *> &data, QGraphicsItem *parent, QRect rec) const
{
    SCASSERT(parent != nullptr)

    QPen rulePen(Qt::NoBrush, 1, Qt::SolidLine);
    rulePen.setColor(Qt::black);

    const qreal notchHeight = ToPixel(3);        // mm
    const qreal shortNotchHeight = ToPixel(1.1); // mm
    Unit const patternUnits = VAbstractValApplication::VApp()->patternUnits();
    const qreal step = UnitConvertor(1, patternUnits, Unit::Px);
    double const marksCount = rec.width() / step;
    int i = 0;
    while (i < marksCount)
    {
        if (i != 0)
        { // don't need 0 notch
            auto *middleRuleLine = new QGraphicsLineItem(parent);
            middleRuleLine->setPen(rulePen);
            middleRuleLine->setLine(rec.x() + step * i - step / 2.,
                                    rec.y() + rec.height() - static_cast<int>(allowance),
                                    rec.x() + step * i - step / 2.,
                                    rec.y() + rec.height() - static_cast<int>(allowance) + shortNotchHeight);
            data.append(middleRuleLine);

            auto *ruleLine = new QGraphicsLineItem(parent);
            ruleLine->setPen(rulePen);
            ruleLine->setLine(rec.x() + step * i, rec.y() + rec.height() - static_cast<int>(allowance),
                              rec.x() + step * i, rec.y() + rec.height() - static_cast<int>(allowance) + notchHeight);
            data.append(ruleLine);
        }
        else
        {
            auto *units = new QGraphicsTextItem(parent);
            units->setDefaultTextColor(Qt::black);
            units->setPlainText(patternUnits == Unit::Cm || patternUnits == Unit::Mm ? tr("cm", "unit")
                                                                                     : tr("in", "unit"));
            QFont fnt = units->font();
            fnt.setPointSize(10);

            qreal unitsWidth = 0;
            QFontMetrics const fm(fnt);
            unitsWidth = fm.horizontalAdvance(units->toPlainText());
            units->setPos(rec.x() + step * 0.5 - unitsWidth * 0.7,
                          rec.y() + rec.height() - static_cast<int>(allowance) - shortNotchHeight);
            units->setFont(fnt);
            data.append(units);
        }
        ++i;
    }
}
