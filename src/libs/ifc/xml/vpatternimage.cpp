/************************************************************************
 **
 **  @file   vpatternimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 1, 2022
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
#include "vpatternimage.h"

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpressionMatch>
#include <QSize>
#include <QSvgRenderer>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#include "../vmisc/compatibility.h"
#include "utils.h"

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::FromFile(const QString &fileName) -> VPatternImage
{
    VPatternImage image;
    QMimeType mime = QMimeDatabase().mimeTypeForFile(fileName);

    if (not IsMimeTypeImage(mime))
    {
        qCritical() << tr("Unexpected mime type: %1").arg(mime.name());
        return {};
    }

    QFile file(fileName);
    if (not file.open(QIODevice::ReadOnly))
    {
        qCritical() << tr("Couldn't read the image. Error: %1").arg(file.errorString());
        return {};
    }

    QString base64 = SplitString(QString::fromLatin1(file.readAll().toBase64().data())).join('\n'_L1);

    image.SetContentData(base64.toLatin1(), mime.name());
    return image;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::ContentType() const -> const QString &
{
    return m_contentType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::ContentData() const -> const QByteArray &
{
    return m_contentData;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternImage::SetContentData(const QByteArray &newContentData, const QString &newContentType)
{
    m_contentData = newContentData;
    m_contentType = not newContentType.isEmpty() ? newContentType : MimeTypeFromData().name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::IsNull() const -> bool
{
    return m_contentData.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::IsValid() const -> bool
{
    m_errorString.clear();

    if (IsNull())
    {
        m_errorString = tr("No data.");
        return false;
    }

    if (m_contentType.isEmpty())
    {
        m_errorString = tr("Content type is empty.");
        return false;
    }

    QMimeType mime = MimeTypeFromData();
    QSet<QString> aliases = ConvertToSet<QString>(mime.aliases());
    aliases.insert(mime.name());

    if (not aliases.contains(m_contentType))
    {
        m_errorString = tr("Content type mismatch.");
        return false;
    }

    if (not IsMimeTypeImage(mime))
    {
        m_errorString = tr("Not image.");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::GetPixmap(int width, int height) const -> QPixmap
{
    if (not IsValid())
    {
        return {};
    }

    QByteArray array = QByteArray::fromBase64(m_contentData);
    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);

    QImageReader imageReader(&buffer);
    imageReader.setScaledSize(QSize(width, height));

    QImage image = imageReader.read();
    if (image.isNull())
    {
        qCritical() << tr("Couldn't read the image. Error: %1").arg(imageReader.errorString());
        return {};
    }

    return QPixmap::fromImage(image);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::ErrorString() const -> const QString &
{
    return m_errorString;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::MimeTypeFromData() const -> QMimeType
{
    return MimeTypeFromByteArray(QByteArray::fromBase64(m_contentData));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternImage::Size() const -> QSize
{
    if (not IsValid())
    {
        return {};
    }

    QByteArray array = QByteArray::fromBase64(m_contentData);
    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);

    return QImageReader(&buffer).size();
}
