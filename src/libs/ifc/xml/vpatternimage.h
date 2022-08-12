/************************************************************************
 **
 **  @file   vpatternimage.h
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
#ifndef VPATTERNIMAGE_H
#define VPATTERNIMAGE_H

#include <QString>
#include <QCoreApplication>

class QPixmap;
class QMimeType;

class VPatternImage
{
    Q_DECLARE_TR_FUNCTIONS(VPatternImage) // NOLINT
public:
    VPatternImage() = default;

    static auto FromFile(const QString &fileName) -> VPatternImage;

    auto ContentType() const -> const QString &;

    auto ContentData() const -> const QByteArray &;
    void SetContentData(const QByteArray &newContentData, const QString & newContentType);

    auto IsNull() const -> bool;
    auto IsValid() const -> bool;

    auto GetPixmap(int width, int height) const -> QPixmap;

    auto ErrorString() const -> const QString &;

    auto MimeTypeFromData() const -> QMimeType;

    auto Size() const -> QSize;

private:
    QString         m_contentType{};
    QByteArray      m_contentData{};
    mutable QString m_errorString{};
};

#endif // VPATTERNIMAGE_H
