// Copyright (C) 2018 The Qt Company Ltd.
// Copyright (C) 2018 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QUTFCODEC_P_H
#define QUTFCODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qendian.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

#include "qtextcodec.h"

QT_BEGIN_NAMESPACE

struct QUtf8BaseTraits
{
    static const bool isTrusted = false;
    static const bool allowNonCharacters = true;
    static const bool skipAsciiHandling = false;
    static const int Error = -1;
    static const int EndOfString = -2;

    static bool isValidCharacter(uint u) { return int(u) >= 0; }

    static void appendByte(uchar *&ptr, uchar b) { *ptr++ = b; }

    static uchar peekByte(const uchar *ptr, int n = 0) { return ptr[n]; }

    static qptrdiff availableBytes(const uchar *ptr, const uchar *end) { return end - ptr; }

    static void advanceByte(const uchar *&ptr, int n = 1) { ptr += n; }

    static void appendUtf16(ushort *&ptr, ushort uc) { *ptr++ = uc; }

    static void appendUcs4(ushort *&ptr, uint uc)
    {
        appendUtf16(ptr, QChar::highSurrogate(uc));
        appendUtf16(ptr, QChar::lowSurrogate(uc));
    }

    static ushort peekUtf16(const ushort *ptr, int n = 0) { return ptr[n]; }

    static qptrdiff availableUtf16(const ushort *ptr, const ushort *end) { return end - ptr; }

    static void advanceUtf16(const ushort *&ptr, int n = 1) { ptr += n; }

    // it's possible to output to UCS-4 too
    static void appendUtf16(uint *&ptr, ushort uc) { *ptr++ = uc; }

    static void appendUcs4(uint *&ptr, uint uc) { *ptr++ = uc; }
};

struct QUtf8BaseTraitsNoAscii : public QUtf8BaseTraits
{
    static const bool skipAsciiHandling = true;
};

namespace QUtf8Functions
{
/// returns 0 on success; errors can only happen if \a u is a surrogate:
/// Error if \a u is a low surrogate;
/// if \a u is a high surrogate, Error if the next isn't a low one,
/// EndOfString if we run into the end of the string.
template<typename Traits, typename OutputPtr, typename InputPtr>
inline int toUtf8(ushort u, OutputPtr &dst, InputPtr &src, InputPtr end)
{
    if (!Traits::skipAsciiHandling && u < 0x80)
    {
        // U+0000 to U+007F (US-ASCII) - one byte
        Traits::appendByte(dst, uchar(u));
        return 0;
    }
    else if (u < 0x0800)
    {
        // U+0080 to U+07FF - two bytes
        // first of two bytes
        Traits::appendByte(dst, 0xc0 | uchar(u >> 6));
    }
    else
    {
        if (!QChar::isSurrogate(u))
        {
            // U+0800 to U+FFFF (except U+D800-U+DFFF) - three bytes
            if (!Traits::allowNonCharacters && QChar::isNonCharacter(u))
                return Traits::Error;

            // first of three bytes
            Traits::appendByte(dst, 0xe0 | uchar(u >> 12));
        }
        else
        {
            // U+10000 to U+10FFFF - four bytes
            // need to get one extra codepoint
            if (Traits::availableUtf16(src, end) == 0)
                return Traits::EndOfString;

            ushort low = Traits::peekUtf16(src);
            if (!QChar::isHighSurrogate(u))
                return Traits::Error;
            if (!QChar::isLowSurrogate(low))
                return Traits::Error;

            Traits::advanceUtf16(src);
            uint ucs4 = QChar::surrogateToUcs4(u, low);

            if (!Traits::allowNonCharacters && QChar::isNonCharacter(ucs4))
                return Traits::Error;

            // first byte
            Traits::appendByte(dst, 0xf0 | (uchar(ucs4 >> 18) & 0xf));

            // second of four bytes
            Traits::appendByte(dst, 0x80 | (uchar(ucs4 >> 12) & 0x3f));

            // for the rest of the bytes
            u = ushort(ucs4);
        }

        // second to last byte
        Traits::appendByte(dst, 0x80 | (uchar(u >> 6) & 0x3f));
    }

    // last byte
    Traits::appendByte(dst, 0x80 | (u & 0x3f));
    return 0;
}

inline bool isContinuationByte(uchar b)
{
    return (b & 0xc0) == 0x80;
}

/// returns the number of characters consumed (including \a b) in case of success;
/// returns negative in case of error: Traits::Error or Traits::EndOfString
template<typename Traits, typename OutputPtr, typename InputPtr>
inline int fromUtf8(uchar b, OutputPtr &dst, InputPtr &src, InputPtr end)
{
    int charsNeeded;
    uint min_uc;
    uint uc;

    if (!Traits::skipAsciiHandling && b < 0x80)
    {
        // US-ASCII
        Traits::appendUtf16(dst, b);
        return 1;
    }

    if (!Traits::isTrusted && Q_UNLIKELY(b <= 0xC1))
    {
        // an UTF-8 first character must be at least 0xC0
        // however, all 0xC0 and 0xC1 first bytes can only produce overlong sequences
        return Traits::Error;
    }
    else if (b < 0xe0)
    {
        charsNeeded = 2;
        min_uc = 0x80;
        uc = b & 0x1f;
    }
    else if (b < 0xf0)
    {
        charsNeeded = 3;
        min_uc = 0x800;
        uc = b & 0x0f;
    }
    else if (b < 0xf5)
    {
        charsNeeded = 4;
        min_uc = 0x10000;
        uc = b & 0x07;
    }
    else
    {
        // the last Unicode character is U+10FFFF
        // it's encoded in UTF-8 as "\xF4\x8F\xBF\xBF"
        // therefore, a byte higher than 0xF4 is not the UTF-8 first byte
        return Traits::Error;
    }

    int bytesAvailable = static_cast<int>(Traits::availableBytes(src, end));
    if (Q_UNLIKELY(bytesAvailable < charsNeeded - 1))
    {
        // it's possible that we have an error instead of just unfinished bytes
        if (bytesAvailable > 0 && !isContinuationByte(Traits::peekByte(src, 0)))
            return Traits::Error;
        if (bytesAvailable > 1 && !isContinuationByte(Traits::peekByte(src, 1)))
            return Traits::Error;
        return Traits::EndOfString;
    }

    // first continuation character
    b = Traits::peekByte(src, 0);
    if (!isContinuationByte(b))
        return Traits::Error;
    uc <<= 6;
    uc |= b & 0x3f;

    if (charsNeeded > 2)
    {
        // second continuation character
        b = Traits::peekByte(src, 1);
        if (!isContinuationByte(b))
            return Traits::Error;
        uc <<= 6;
        uc |= b & 0x3f;

        if (charsNeeded > 3)
        {
            // third continuation character
            b = Traits::peekByte(src, 2);
            if (!isContinuationByte(b))
                return Traits::Error;
            uc <<= 6;
            uc |= b & 0x3f;
        }
    }

    // we've decoded something; safety-check it
    if (!Traits::isTrusted)
    {
        if (uc < min_uc)
            return Traits::Error;
        if (QChar::isSurrogate(uc) || uc > QChar::LastValidCodePoint)
            return Traits::Error;
        if (!Traits::allowNonCharacters && QChar::isNonCharacter(uc))
            return Traits::Error;
    }

    // write the UTF-16 sequence
    if (!QChar::requiresSurrogates(uc))
    {
        // UTF-8 decoded and no surrogates are required
        // detach if necessary
        Traits::appendUtf16(dst, ushort(uc));
    }
    else
    {
        // UTF-8 decoded to something that requires a surrogate pair
        Traits::appendUcs4(dst, uc);
    }

    Traits::advanceByte(src, charsNeeded - 1);
    return charsNeeded;
}
} // namespace QUtf8Functions

enum DataEndianness : quint8
{
    DetectEndianness,
    BigEndianness,
    LittleEndianness
};

struct QUtf8
{
    static QChar *convertToUnicode(QChar *, const char *, int) noexcept;
    static QString convertToUnicode(const char *, int);
    static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *);
    static QByteArray convertFromUnicode(const QChar *, int);
    static QByteArray convertFromUnicode(const QChar *, int, QTextCodec::ConverterState *);
    struct ValidUtf8Result
    {
        bool isValidUtf8;
        bool isValidAscii;
    };
    static ValidUtf8Result isValidUtf8(const char *, qsizetype);
    static int compareUtf8(const char *, qsizetype, const QChar *, int);
    static int compareUtf8(const char *, qsizetype, QLatin1String s);
};

struct QUtf16
{
    static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
    static QByteArray convertFromUnicode(const QChar *,
                                         int,
                                         QTextCodec::ConverterState *,
                                         DataEndianness = DetectEndianness);
};

struct QUtf32
{
    static QString convertToUnicode(const char *, int, QTextCodec::ConverterState *, DataEndianness = DetectEndianness);
    static QByteArray convertFromUnicode(const QChar *,
                                         int,
                                         QTextCodec::ConverterState *,
                                         DataEndianness = DetectEndianness);
};

/*
 Converts from different utf encodings looking at a possible byte order mark at the
 beginning of the string. If no BOM exists, utf-8 is assumed.
 */
QString qFromUtfEncoded(const QByteArray &ba);

#if defined(WITH_TEXTCODEC)

class QUtf8Codec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QUtf8Codec)

public:
    QUtf8Codec() = default;
    ~QUtf8Codec() override = default;

    QByteArray name() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *state) const override;
    void convertToUnicode(QString *target, const char *chars, int len, ConverterState *state) const;
};

class QUtf16Codec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QUtf16Codec)

protected:

public:
    QUtf16Codec() = default;
    ~QUtf16Codec() override = default;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *state) const override;

protected:
    DataEndianness e{DetectEndianness}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
};

class QUtf16BECodec : public QUtf16Codec
{
    Q_DISABLE_COPY_MOVE(QUtf16BECodec)

public:
    QUtf16BECodec()
      : QUtf16Codec()
    {
        e = BigEndianness;
    }
    ~QUtf16BECodec() override = default;
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QUtf16LECodec : public QUtf16Codec
{
    Q_DISABLE_COPY_MOVE(QUtf16LECodec)

public:
    QUtf16LECodec()
      : QUtf16Codec()
    {
        e = LittleEndianness;
    }
    ~QUtf16LECodec() override = default;
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QUtf32Codec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QUtf32Codec)

public:
    QUtf32Codec() = default;
    ~QUtf32Codec() override = default;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *state) const override;

protected:
    DataEndianness e{DetectEndianness}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
};

class QUtf32BECodec : public QUtf32Codec
{
    Q_DISABLE_COPY_MOVE(QUtf32BECodec)

public:
    QUtf32BECodec()
      : QUtf32Codec()
    {
        e = BigEndianness;
    }
    ~QUtf32BECodec() override = default;
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QUtf32LECodec : public QUtf32Codec
{
    Q_DISABLE_COPY_MOVE(QUtf32LECodec)

public:
    QUtf32LECodec()
      : QUtf32Codec()
    {
        e = LittleEndianness;
    }
    ~QUtf32LECodec() override = default;
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

#endif // WITH_TEXTCODEC

QT_END_NAMESPACE

#endif // QUTFCODEC_P_H
