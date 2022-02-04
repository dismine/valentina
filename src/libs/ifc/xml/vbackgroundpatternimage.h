/************************************************************************
 **
 **  @file   vbackgroundpatternimage.h
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
#ifndef VBACKGROUNDPATTERNIMAGE_H
#define VBACKGROUNDPATTERNIMAGE_H

#include <QPoint>
#include <QCoreApplication>
#include <QUuid>
#include <QTransform>

#include "../vmisc/typedef.h"

class QPixmap;
class QMimeType;

enum class PatternImage
{
    Raster,
    Vector,
    Unknown
};

class VBackgroundPatternImage
{
    Q_DECLARE_TR_FUNCTIONS(VBackgroundPatternImage) // NOLINT
public:
    VBackgroundPatternImage() = default;

    static auto FromFile(const QString &fileName, bool builtIn) -> VBackgroundPatternImage;

    auto ContentType() const -> const QString &;

    auto ContentData() const -> const QByteArray &;
    void SetContentData(const QByteArray &newContentData, const QString & newContentType);

    auto IsNull() const -> bool;
    auto IsValid() const -> bool;

    auto MimeTypeFromData() const -> QMimeType;

    auto FilePath() const -> const QString &;
    void SetFilePath(const QString &newFilePath);

    auto Name() const -> const QString &;
    void SetName(const QString &newName);

    auto ZValue() const -> qreal;
    void SetZValue(qreal newZValue);

    auto Hold() const -> bool;
    void SetHold(bool newHold);

    auto Id() const -> QUuid;
    void SetId(const QUuid &newId);

    auto Size() const -> QSize;

    auto ErrorString() const -> const QString &;

    auto Matrix() const -> const QTransform &;
    void SetMatrix(const QTransform &newMatrix);

    auto Type() const -> PatternImage;

    auto BoundingRect() const -> QRectF;

    auto Visible() const -> bool;
    void SetVisible(bool newVisible);

    auto Opacity() const -> qreal;
    void SetOpacity(qreal newOpacity);

private:
    QUuid           m_id{QUuid::createUuid()};
    QString         m_contentType{};
    QByteArray      m_contentData{};
    mutable QString m_errorString{};
    QString         m_filePath{};
    QString         m_name{};
    qreal           m_zValue{0};
    QTransform      m_matrix{};
    bool            m_hold{false};
    bool            m_visible{true};
    qreal           m_opacity{1.0};
    mutable QSize   m_size{};
};

#endif // VBACKGROUNDPATTERNIMAGE_H
