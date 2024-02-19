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

#include "../vmisc/compatibility.h"
#include "../vmisc/defglobal.h"
#include "utils.h"

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QImageReader>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPixmap>
#include <QSvgRenderer>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

using namespace Qt::Literals::StringLiterals;

const QString VBackgroundPatternImage::brokenImage = QStringLiteral("://icon/svg/broken_path.svg");

namespace
{

//---------------------------------------------------------------------------------------------------------------------
auto ScaleRasterImage(const QImage &image) -> QSize
{
    if (image.isNull())
    {
        return {};
    }

    const double ratioX = PrintDPI / (image.dotsPerMeterX() / 100. * 2.54);
    const double ratioY = PrintDPI / (image.dotsPerMeterY() / 100. * 2.54);
    const QSize imageSize = image.size();
    return {qRound(imageSize.width() * ratioX), qRound(imageSize.height() * ratioY)};
}

//---------------------------------------------------------------------------------------------------------------------
auto ScaleVectorImage(const QSvgRenderer &renderer) -> QSize
{
    const QSize imageSize = renderer.defaultSize();
    constexpr double ratio = PrintDPI / 90.;
    return {qRound(imageSize.width() * ratio), qRound(imageSize.height() * ratio)};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::FromFile(const QString &fileName, bool builtIn) -> VBackgroundPatternImage
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VBackgroundPatternImage image;

    QT_WARNING_POP

    QMimeType const mime = QMimeDatabase().mimeTypeForFile(fileName);

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

        QString const base64 = SplitString(QString::fromLatin1(file.readAll().toBase64().data())).join('\n'_L1);
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
    m_size = QSize();
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
        QMimeType const mime = MimeTypeFromData();

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

        QMimeType const mime = MimeTypeFromData();
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
    m_size = QSize();
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

    if (not m_size.isValid())
    {
        if (not m_filePath.isEmpty())
        {
            m_size = LinkedImageSize();
            return m_size;
        }

        if (not m_contentData.isEmpty())
        {
            m_size = BuiltInImageSize();
            return m_size;
        }
    }

    return m_size;
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

    QMimeType const mime = MimeTypeFromData();

    if (mime.name().startsWith(QStringLiteral("image/svg+xml")))
    {
        return PatternImage::Vector;
    }

    return PatternImage::Raster;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::BoundingRect() const -> QRectF
{
    QSize const imageSize = Size();
    QRectF const imageRect({0, 0}, QSizeF(imageSize.width(), imageSize.height()));
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

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::Opacity() const -> qreal
{
    return m_opacity;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundPatternImage::SetOpacity(qreal newOpacity)
{
    m_opacity = qBound(0.0, newOpacity, 1.0);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::LinkedImageSize() const -> QSize
{
    if (Type() == PatternImage::Raster)
    {
        const QImage image = QImageReader(m_filePath).read();
        return image.isNull() ? ScaleVectorImage(QSvgRenderer(brokenImage)) : ScaleRasterImage(image);
    }

    if (Type() == PatternImage::Vector)
    {
        QSvgRenderer const renderer(m_filePath);
        return not renderer.isValid() ? ScaleVectorImage(QSvgRenderer(brokenImage)) : ScaleVectorImage(renderer);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundPatternImage::BuiltInImageSize() const -> QSize
{
    QByteArray array = QByteArray::fromBase64(m_contentData);

    if (Type() == PatternImage::Raster)
    {
        QBuffer buffer(&array);
        buffer.open(QIODevice::ReadOnly);
        const QImage image = QImageReader(&buffer).read();

        return image.isNull() ? ScaleVectorImage(QSvgRenderer(brokenImage)) : ScaleRasterImage(image);
    }

    if (Type() == PatternImage::Vector)
    {
        QSvgRenderer const renderer(array);
        return not renderer.isValid() ? ScaleVectorImage(QSvgRenderer(brokenImage)) : ScaleVectorImage(renderer);
    }

    return {};
}
