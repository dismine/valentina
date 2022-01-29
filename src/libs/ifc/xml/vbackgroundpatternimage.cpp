/************************************************************************
 **
 **  @file   vbackgroundpatternimage.cpp
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
#include "vbackgroundpatternimage.h"

#include "utils.h"
#include "../vmisc/compatibility.h"

#include <QMimeType>
#include <QDebug>
#include <QFile>
#include <QMimeDatabase>
#include <QPixmap>
#include <QBuffer>
#include <QImageReader>

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::FromFile(const QString &fileName, bool builtIn) -> VBackgroundPatternImage
{
    VBackgroundPatternImage image;
    QMimeType mime = QMimeDatabase().mimeTypeForFile(fileName);

    if (not IsMimeTypeImage(mime))
    {
        qCritical() << tr("Unexpected mime type: %1").arg(mime.name());
        return {};
    }

    if (builtIn)
    {
        QFile file(fileName);
        if (not file.open(QIODevice::ReadOnly))
        {
            qCritical() << tr("Couldn't read the image. Error: %1").arg(file.errorString());
            return {};
        }

        QString base64 = SplitString(QString::fromLatin1(file.readAll().toBase64().data())).join('\n');
        image.SetContentData(base64.toLatin1(), mime.name());
    }
    else
    {
        image.SetFilePath(fileName);
    }

    return image;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::ContentType() const -> const QString &
{
    return m_contentType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::ContentData() const -> const QByteArray &
{
    return m_contentData;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetContentData(const QByteArray &newContentData, const QString &newContentType)
{
    m_contentData = newContentData;
    m_contentType = newContentType;
    m_filePath.clear();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::IsNull() const -> bool
{
    return m_filePath.isEmpty() && m_contentData.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::IsValid() const -> bool
{
    m_errorString.clear();

    if (IsNull())
    {
        m_errorString = tr("No data.");
        return false;
    }

    if (m_id.isNull())
    {
        m_errorString = tr("Invalid id.");
        return false;
    }

    if (not m_filePath.isEmpty())
    {
        QMimeType mime = MimeTypeFromData();

        if (not IsMimeTypeImage(mime))
        {
            qCritical() << tr("Unexpected mime type: %1").arg(mime.name());
            return false;
        }
    }
    else
    {
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
            m_errorString = tr("Content type mistmatch.");
            return false;
        }

        if (not IsMimeTypeImage(mime))
        {
            m_errorString = tr("Not image.");
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::MimeTypeFromData() const -> QMimeType
{
    if (not m_filePath.isEmpty())
    {
        return QMimeDatabase().mimeTypeForFile(m_filePath);
    }

    if (not m_contentData.isEmpty())
    {
        return MimeTypeFromByteArray(QByteArray::fromBase64(m_contentData));
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::FilePath() const -> const QString &
{
    return m_filePath;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetFilePath(const QString &newFilePath)
{
    m_filePath = newFilePath;
    m_contentData.clear();
    m_contentType.clear();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Name() const -> const QString &
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetName(const QString &newName)
{
    m_name = newName;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::ZValue() const -> qreal
{
    return m_zValue;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetZValue(qreal newZValue)
{
    m_zValue = newZValue;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Hold() const -> bool
{
    return m_hold;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetHold(bool newHold)
{
    m_hold = newHold;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Id() const -> QUuid
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetId(const QUuid &newId)
{
    m_id = newId;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Size() const -> QSize
{
    if (not IsValid())
    {
        return {};
    }

    if (not m_filePath.isEmpty())
    {
        return QImageReader(m_filePath).size();
    }

    if (not m_contentData.isEmpty())
    {
        QByteArray array = QByteArray::fromBase64(m_contentData);
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);

        return QImageReader(&buffer).size();
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::ErrorString() const -> const QString &
{
    return m_errorString;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Matrix() const -> const QTransform &
{
    return m_matrix;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetMatrix(const QTransform &newMatrix)
{
    m_matrix = newMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Type() const -> PatternImage
{
    if (not IsValid())
    {
        return PatternImage::Unknown;
    }

    QMimeType mime = MimeTypeFromData();

    if (mime.name().startsWith(QStringLiteral("image/svg+xml")))
    {
        return PatternImage::Vector;
    }

    return PatternImage::Raster;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::BoundingRect() const -> QRectF
{
    QSize imageSize = Size();
    QRectF imageRect({0, 0}, QSizeF(imageSize.width(), imageSize.height()));
    return m_matrix.mapRect(imageRect);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Visible() const -> bool
{
    return m_visible;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetVisible(bool newVisible)
{
    m_visible = newVisible;
}
