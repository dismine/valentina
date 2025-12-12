/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "stylehelper.h"

#include <QApplication>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPixmapCache>
#include <QRect>
#include <QStyleOption>
#include <QWidget>
#include <QtMath>

QColor StyleHelper::m_baseColor;          // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
QColor StyleHelper::m_requestedBaseColor; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

//---------------------------------------------------------------------------------------------------------------------
auto StyleHelper::sidebarFontSize() -> qreal
{
#if defined(Q_OS_MAC)
    return 10;
#else
    return 7.5;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto StyleHelper::panelTextColor(bool lightColored) -> QColor
{
    // qApp->palette().highlightedText().color();
    if (!lightColored)
    {
        return Qt::white;
    }

    return Qt::black;
}

//---------------------------------------------------------------------------------------------------------------------
auto StyleHelper::baseColor(bool lightColored) -> QColor
{
    if (!lightColored)
    {
        return m_baseColor;
    }

    return m_baseColor.lighter(230);
}

//---------------------------------------------------------------------------------------------------------------------
auto StyleHelper::borderColor(bool lightColored) -> QColor
{
    QColor result = baseColor(lightColored);
    result.setHsv(result.hue(), result.saturation(), result.value() / 2);
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto StyleHelper::sidebarHighlight() -> QColor
{
    return {255, 255, 255, 40};
}

//---------------------------------------------------------------------------------------------------------------------
// We try to ensure that the actual color used are within
// reasonalbe bounds while generating the actual baseColor
// from the users request.
void StyleHelper::setBaseColor(const QColor &newcolor)
{
    m_requestedBaseColor = newcolor;

    QColor color;
    color.setHsv(newcolor.hue(), static_cast<int>(newcolor.saturation() * 0.7), 64 + newcolor.value() / 3);

    if (color.isValid() && color != m_baseColor)
    {
        m_baseColor = color;
        const QWidgetList widgets = QApplication::topLevelWidgets();
        for (auto *w : widgets)
        {
            w->update();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Draws a cached pixmap with shadow
void StyleHelper::drawIconWithShadow(const QIcon &icon, const QRect &rect, QPainter *p, QIcon::Mode iconMode,
                                     int dipRadius, const QColor &color, const QPoint &dipOffset)
{
    qDebug("StyleHelper::drawIconWithShadow: drawing icon with shadow");
    qDebug() << " rect:" << rect << " iconMode:" << iconMode << " dipRadius:" << dipRadius << " color:" << color
             << " dipOffset:" << dipOffset;

    QPixmap cache;
    QString const pixmapName =
        QString::fromLatin1("icon %0 %1 %2").arg(icon.cacheKey()).arg(iconMode).arg(rect.height());

    qDebug() << " pixmapName:" << pixmapName;

    if (!QPixmapCache::find(pixmapName, &cache))
    {
        // High-dpi support: The in parameters (rect, radius, offset) are in
        // device-independent pixels. The call to QIcon::pixmap() below might
        // return a high-dpi pixmap, which will in that case have a devicePixelRatio
        // different than 1. The shadow drawing caluculations are done in device
        // pixels.
        QPixmap px = icon.pixmap(rect.size());
        qDebug() << " px size:" << px.size() << " DPR:" << px.devicePixelRatio();
        int const devicePixelRatio = qCeil(px.devicePixelRatio());
        int const radius = dipRadius * devicePixelRatio;
        QPoint const offset = dipOffset * devicePixelRatio;
        cache = QPixmap(px.size() + QSize(radius * 2, radius * 2));
        cache.fill(Qt::transparent);

        qDebug() << " cache size:" << cache.size();

        QPainter cachePainter(&cache);
        qDebug() << " cachePainter active? " << cachePainter.isActive();

        if (iconMode == QIcon::Disabled)
        {
            QImage im = px.toImage().convertToFormat(QImage::Format_ARGB32);
            qDebug() << " Disabled mode: im size:" << im.size();

            for (int y = 0; y < im.height(); ++y)
            {
                // Scanline data is at least 32-bit aligned.
                // https://doc.qt.io/qt-6/qimage.html#scanLine
                void *voidPtr = im.scanLine(y);
                auto *scanLine = static_cast<QRgb *>(voidPtr);

                for (int x = 0; x < im.width(); ++x)
                {
                    QRgb const pixel = *scanLine;
                    auto const intensity = static_cast<char>(qGray(pixel));
                    *scanLine = qRgba(intensity, intensity, intensity, qAlpha(pixel));
                    ++scanLine;
                }
            }
            px = QPixmap::fromImage(im);
        }

        // Draw shadow
        QImage tmp(px.size() + QSize(radius * 2, radius * 2 + 1), QImage::Format_ARGB32_Premultiplied);
        tmp.fill(Qt::transparent);
        qDebug() << " tmp QImage size:" << tmp.size();

        QPainter tmpPainter(&tmp);
        qDebug() << " tmpPainter active? " << tmpPainter.isActive();
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
        tmpPainter.drawPixmap(QRect(radius, radius, px.width(), px.height()), px);
        tmpPainter.end();

        // blur the alpha channel
        QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
        blurred.fill(Qt::transparent);
        qDebug() << " blurred QImage size:" << blurred.size();

        QPainter blurPainter(&blurred);
        qDebug() << " blurPainter active? " << blurPainter.isActive();
        qt_blurImage(&blurPainter, tmp, radius, false, true);
        blurPainter.end();

        tmp = blurred;

        // blacken the image...
        bool ok = tmpPainter.begin(&tmp);
        qDebug() << " tmpPainter.begin() returned" << ok << " tmp size:" << tmp.size();
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(tmp.rect(), color);
        tmpPainter.end();

        ok = tmpPainter.begin(&tmp);
        qDebug() << " tmpPainter.begin() returned" << ok << " tmp size:" << tmp.size();
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(tmp.rect(), color);
        tmpPainter.end();

        // draw the blurred drop shadow...
        cachePainter.drawImage(QRect(0, 0, cache.rect().width(), cache.rect().height()), tmp);

        // Draw the actual pixmap...
        cachePainter.drawPixmap(QRect(QPoint(radius, radius) + offset, QSize(px.width(), px.height())), px);
        cache.setDevicePixelRatio(devicePixelRatio);
        QPixmapCache::insert(pixmapName, cache);
    }

    QRect targetRect = cache.rect();
    targetRect.setSize(targetRect.size() / cache.devicePixelRatio());
    targetRect.moveCenter(rect.center() - dipOffset);
    qDebug() << " targetRect:" << targetRect;
    p->drawPixmap(targetRect, cache);
    qDebug() << "drawIconWithShadow end";
}
