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
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPixmapCache>
#include <QRect>
#include <QStyleOption>
#include <QWidget>
#include <QtMath>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(styleHelper, "v.styleHelper") // NOLINT

QT_WARNING_POP

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
void StyleHelper::drawIconWithShadow(const QIcon &icon,
                                     const QRect &rect,
                                     QPainter *p,
                                     QIcon::Mode iconMode,
                                     int dipRadius,
                                     const QColor &color,
                                     const QPoint &dipOffset)
{
    if (icon.isNull() || rect.isEmpty())
    {
        return; // Nothing to draw; a degenerate rect leads to null intermediate images below.
    }

    QPixmap cache;
    QString const pixmapName = QString::fromLatin1("icon %0 %1 %2").arg(icon.cacheKey()).arg(iconMode).arg(rect.height());

    // Diagnostics for an unreproducible freeze. This cache is keyed on QIcon::cacheKey(), which is unique per QIcon
    // instance - so a caller that hands us a freshly constructed icon every time can never hit it, and pays for the
    // allocations and qt_blurImage() below on every single draw. Count what actually happens on the machine that
    // freezes rather than guessing. Costs one branch on the hit path.
    static qint64 hits = 0;     // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static qint64 misses = 0;   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static qint64 missMs = 0;   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static qint64 reported = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    const bool cached = QPixmapCache::find(pixmapName, &cache);
    cached ? ++hits : ++misses;

    QElapsedTimer missTimer;
    if (not cached)
    {
        missTimer.start();
    }

    auto ReportIconShadowStats = [&]()
    {
        if (cached)
        {
            return;
        }

        missMs += missTimer.elapsed();

        // Every 200 misses, so a healthy build stays silent and a pathological one is obvious.
        if (misses - reported >= 200)
        {
            reported = misses;
            qCInfo(styleHelper,
                   "%s",
                   qUtf8Printable(QStringLiteral("Icon shadow cache: %1 misses, %2 hits, %3 ms spent rebuilding.")
                                      .arg(misses)
                                      .arg(hits)
                                      .arg(missMs)));
        }
    };

    if (not cached)
    {
        // High-dpi support: The in parameters (rect, radius, offset) are in
        // device-independent pixels. The call to QIcon::pixmap() below might
        // return a high-dpi pixmap, which will in that case have a devicePixelRatio
        // different than 1. The shadow drawing caluculations are done in device
        // pixels.
        QPixmap px = icon.pixmap(rect.size());
        int const devicePixelRatio = qCeil(px.devicePixelRatio());
        int const radius = dipRadius * devicePixelRatio;
        QPoint const offset = dipOffset * devicePixelRatio;
        cache = QPixmap(px.size() + QSize(radius * 2, radius * 2));

        // Any of the QImage/QPixmap allocations below can return a null buffer (degenerate size or
        // out of memory). Painting on a null device triggers "engine == 0" warnings and, worse, a
        // crash deep in Qt's paint/blur code. Fall back to a plain icon draw instead of that.
        if (px.isNull() || cache.isNull())
        {
            qCWarning(styleHelper) << "Skipping icon shadow: null buffer. rect:" << rect << "px size:" << px.size()
                                   << "radius:" << radius << "cache size:" << cache.size();
            icon.paint(p, rect, Qt::AlignCenter, iconMode);
            return;
        }

        cache.fill(Qt::transparent);

        QPainter cachePainter(&cache);

        if (iconMode == QIcon::Disabled)
        {
            QImage im = px.toImage().convertToFormat(QImage::Format_ARGB32);

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

        // blur the alpha channel
        QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);

        if (tmp.isNull() || blurred.isNull())
        {
            qCWarning(styleHelper) << "Skipping icon shadow: null blur buffer. rect:" << rect << "px size:" << px.size()
                                   << "radius:" << radius << "tmp size:" << tmp.size()
                                   << "blurred size:" << blurred.size();
            cachePainter.end();
            icon.paint(p, rect, Qt::AlignCenter, iconMode);
            return;
        }

        tmp.fill(Qt::transparent);

        QPainter tmpPainter(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
        tmpPainter.drawPixmap(QRect(radius, radius, px.width(), px.height()), px);
        tmpPainter.end();

        blurred.fill(Qt::transparent);

        QPainter blurPainter(&blurred);
        qt_blurImage(&blurPainter, tmp, radius, false, true);
        blurPainter.end();

        tmp = blurred;

        // blacken the image...
        tmpPainter.begin(&tmp);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(tmp.rect(), color);
        tmpPainter.end();

        // draw the blurred drop shadow...
        cachePainter.drawImage(QRect(0, 0, cache.rect().width(), cache.rect().height()), tmp);

        // Draw the actual pixmap...
        cachePainter.drawPixmap(QRect(QPoint(radius, radius) + offset, QSize(px.width(), px.height())), px);
        cache.setDevicePixelRatio(devicePixelRatio);
        QPixmapCache::insert(pixmapName, cache);
        ReportIconShadowStats();
    }

    QRect targetRect = cache.rect();
    targetRect.setSize(targetRect.size() / cache.devicePixelRatio());
    targetRect.moveCenter(rect.center() - dipOffset);
    p->drawPixmap(targetRect, cache);
}
