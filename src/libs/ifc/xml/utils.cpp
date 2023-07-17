/************************************************************************
 **
 **  @file   utils.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "utils.h"

#include <QImageReader>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpressionMatch>
#include <QSet>
#include <QStringList>
#include <QSvgRenderer>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#include "../vmisc/compatibility.h"

//---------------------------------------------------------------------------------------------------------------------
auto IsMimeTypeImage(const QMimeType &mime) -> bool
{
    QStringList aliases = mime.aliases();
    aliases.prepend(mime.name());

    static QRegularExpression rx(QStringLiteral("^image\\/[-\\w]+(\\.[-\\w]+)*([+][-\\w]+)?$"));

    return std::any_of(aliases.begin(), aliases.end(), [](const QString &name) { return rx.match(name).hasMatch(); });
}

//---------------------------------------------------------------------------------------------------------------------
auto SplitString(QString str) -> QStringList
{
    QStringList list;

    const int n = 80;
    while (not str.isEmpty())
    {
        list.append(str.left(n));
        str.remove(0, n);
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto MimeTypeFromByteArray(const QByteArray &data) -> QMimeType
{
    QMimeType mime = QMimeDatabase().mimeTypeForData(data);

    QSet<QString> aliases = ConvertToSet<QString>(mime.aliases());
    aliases.insert(mime.name());

    QSet<QString> gzipMime{"application/gzip", "application/x-gzip"};

    if (gzipMime.contains(aliases))
    {
        QSvgRenderer render(data);
        if (render.isValid())
        {
            mime = QMimeDatabase().mimeTypeForName(QStringLiteral("image/svg+xml-compressed"));
        }
    }

    return mime;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareImageFilters() -> QString
{
    const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
    const QSet<QString> filterFormats{"bmp", "jpeg", "jpg", "png", "svg", "svgz", "tif", "tiff", "webp"};
    QStringList sufixes;
    for (const auto &format : supportedFormats)
    {
        if (filterFormats.contains(format))
        {
            sufixes.append(QStringLiteral("*.%1").arg(QString(format)));
        }
    }

    QStringList filters;

    if (not sufixes.isEmpty())
    {
        filters.append(QObject::tr("Images") + QStringLiteral(" (%1)").arg(sufixes.join(' ')));
    }

    filters.append(QObject::tr("All files") + QStringLiteral(" (*.*)"));

    return filters.join(QStringLiteral(";;"));
}
