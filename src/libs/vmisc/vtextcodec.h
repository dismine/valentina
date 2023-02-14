/************************************************************************
 **
 **  @file   vtextcodec.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 2, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VTEXTCODEC_H
#define VTEXTCODEC_H

class VTextCodec
{
public:
    virtual ~VTextCodec() = default;

    static auto codecForName(const QString &name) -> VTextCodec *;
    static auto codecForLocale() -> VTextCodec *;
    static auto codecForMib(int mib) -> VTextCodec *;
    static auto availableMibs() -> QList<int>;
    static auto availableCodecs() -> QList<QByteArray>;

    auto fromUnicode(const QString &str) const -> QByteArray;
    auto toUnicode(const char *chars) const -> QString;

    auto name() const -> QString;
    auto mibEnum() const -> int;

    auto Encoding() const -> QStringConverter::Encoding;

protected:
    explicit VTextCodec(QStringConverter::Encoding encoding);
private:
    Q_DISABLE_COPY_MOVE(VTextCodec) // NOLINT

    QStringConverter::Encoding m_encoding{QStringConverter::Utf8};

    static auto MakeCodec(QStringConverter::Encoding encoding) -> VTextCodec *;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VTextCodec::Encoding() const -> QStringConverter::Encoding
{
    return m_encoding;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VTextCodec::fromUnicode(const QString &str) const -> QByteArray
{
    return QStringEncoder(m_encoding)(str);
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VTextCodec::toUnicode(const char *chars) const -> QString
{
    return QStringDecoder(m_encoding)(chars);
}

#endif // VTEXTCODEC_H
