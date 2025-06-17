/************************************************************************
 **
 **  @file   vtextcodec.cpp
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
#include "vtextcodec.h"

#include <QMap>
#include <QStringConverter>

#include "vabstractapplication.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto QtCodecs() -> QMap<QStringConverter::Encoding, QStringList>
{
    static auto codecs = QMap<QStringConverter::Encoding, QStringList>{
        {QStringConverter::Latin1,
         {"ISO-8859-1", "ANSI_1252", "CP1252", "windows-1252", "LATIN1", "CP819", "CSISO", "IBM819", "ISO_8859-1",
          "APPLE ROMAN", "ISO8859-1", "ISO8859-15", "ISO-IR-100", "L1", "IBM 850", "850"}}, // Western Europe
        {QStringConverter::Utf8, {"UTF-8", "UTF8", "UTF8-BIT"}},
        {QStringConverter::Utf16, {"UTF-16", "UTF16", "UTF16-BIT"}},
        {QStringConverter::Utf16BE, {"UTF-16BE"}},
        {QStringConverter::Utf16LE, {"UTF-16LE"}},
        {QStringConverter::Utf32, {"UTF-32"}},
        {QStringConverter::Utf32BE, {"UTF-32BE"}},
        {QStringConverter::Utf32LE, {"UTF-32LE"}},
        {QStringConverter::System, {"System"}},
    };

    return codecs;
}

//---------------------------------------------------------------------------------------------------------------------
auto CodecMibs() -> QMap<QStringConverter::Encoding, int>
{
    static auto mibs = QMap<QStringConverter::Encoding, int>{
        {QStringConverter::Utf8, 106},     {QStringConverter::Utf16, 1015}, {QStringConverter::Utf16BE, 1013},
        {QStringConverter::Utf16LE, 1014}, {QStringConverter::Utf32, 1017}, {QStringConverter::Utf32BE, 1018},
        {QStringConverter::Utf32LE, 1019}, {QStringConverter::Latin1, 4},   {QStringConverter::System, 0},
    };

    return mibs;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VTextCodec::VTextCodec(QStringConverter::Encoding encoding)
  : m_encoding(encoding)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::codecForName(const QString &name) -> VTextCodec *
{
    QMap<QStringConverter::Encoding, QStringList> const codecs = QtCodecs();

    auto i = codecs.constBegin();
    while (i != codecs.constEnd())
    {
        QStringList const aliases = i.value();
        if (aliases.contains(name))
        {
            return MakeCodec(i.key());
        }
        ++i;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::codecForLocale() -> VTextCodec *
{
    return MakeCodec(QStringConverter::System);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::codecForMib(int mib) -> VTextCodec *
{
    QMap<QStringConverter::Encoding, int> const mibs = CodecMibs();

    auto i = mibs.constBegin();
    while (i != mibs.constEnd())
    {
        if (mib == i.value())
        {
            return MakeCodec(i.key());
        }
        ++i;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::availableMibs() -> QList<int>
{
    return CodecMibs().values();
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::availableCodecs() -> QList<QByteArray>
{
    QMap<QStringConverter::Encoding, QStringList> const codecs = QtCodecs();

    QList<QByteArray> names;
    names.reserve(codecs.size());

    auto i = codecs.constBegin();
    while (i != codecs.constEnd())
    {
        names.append(i.value().constFirst().toLatin1());
        ++i;
    }

    return names;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::name() const -> QString
{
    return QtCodecs().value(m_encoding).constFirst();
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::mibEnum() const -> int
{
    return CodecMibs().value(m_encoding, 0);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextCodec::MakeCodec(QStringConverter::Encoding encoding) -> VTextCodec *
{
    VTextCodec *codec = VAbstractApplication::VApp()->TextCodecCache(encoding);
    if (codec != nullptr)
    {
        return codec;
    }

    codec = new VTextCodec(encoding);
    VAbstractApplication::VApp()->CacheTextCodec(encoding, codec);
    return codec;
}
