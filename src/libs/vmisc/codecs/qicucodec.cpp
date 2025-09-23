// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qicucodec_p.h"

#include "qtextcodec_p.h"
#include "qutfcodec_p.h"
#include "qlatincodec_p.h"
#include "qsimplecodec_p.h"

#include <unicode/ucnv.h>
#include <QDebug>

#if defined(WITH_BASIC_CODECS)
#include "qtsciicodec_p.h"
#include "qisciicodec_p.h"
#endif

#if defined(Q_OS_WIN)
#include "qwindowscodec_p.h"
#endif

#include "../defglobal.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

typedef QList<QTextCodec*>::ConstIterator TextCodecListConstIt;
typedef QList<QByteArray>::ConstIterator ByteArrayListConstIt;

static void qIcuCodecStateFree(QTextCodec::ConverterState *state) noexcept
{
    ucnv_close(static_cast<UConverter *>(state->d[0]));
}

bool qTextCodecNameMatch(const char *n, const char *h)
{
    return ucnv_compareNames(n, h) == 0;
}

/* The list below is generated from http://www.iana.org/assignments/character-sets/
   using the snippet of code below:

#include <QtCore>
#include <unicode/ucnv.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QFile file("character-sets.txt");
    file.open(QFile::ReadOnly);
    QByteArray name;
    int mib = -1;
    QByteArray nameList;
    int pos = 0;
    while (!file.atEnd()) {
        QByteArray s = file.readLine().trimmed();
        if (s.isEmpty()) {
            if (mib != -1) {
                UErrorCode error = U_ZERO_ERROR;
                const char *standard_name = ucnv_getStandardName(name, "MIME", &error);
                if (U_FAILURE(error) || !standard_name) {
                    error = U_ZERO_ERROR;
                    standard_name = ucnv_getStandardName(name, "IANA", &error);
                }
                UConverter *conv = ucnv_open(standard_name, &error);
                if (!U_FAILURE(error) && conv && standard_name) {
                    ucnv_close(conv);
                    printf("    { %d, %d },\n", mib, pos);
                    nameList += "\"";
                    nameList += standard_name;
                    nameList += "\\0\"\n";
                    pos += strlen(standard_name) + 1;
                }
            }
            name = QByteArray();
            mib = -1;
        }
        if (s.startsWith("Name: ")) {
            name = s.mid(5).trimmed();
            if (name.indexOf(' ') > 0)
                name = name.left(name.indexOf(' '));
        }
        if (s.startsWith("MIBenum:"))
            mib = s.mid(8).trimmed().toInt();
        if (s.startsWith("Alias:") && s.contains("MIME")) {
            name = s.mid(6).trimmed();
            name = name.left(name.indexOf(' ')).trimmed();
        }
    }
    qDebug() << nameList;
}
*/

struct MibEntry
{
    int mib;
    QLatin1String name;
};

constexpr std::array<MibEntry, 113> mibToName = {{
    {3, "US-ASCII"_L1},
    {4, "ISO-8859-1"_L1},
    {5, "ISO-8859-2"_L1},
    {6, "ISO-8859-3"_L1},
    {7, "ISO-8859-4"_L1},
    {8, "ISO-8859-5"_L1},
    {9, "ISO-8859-6"_L1},
    {10, "ISO-8859-7"_L1},
    {11, "ISO-8859-8"_L1},
    {12, "ISO-8859-9"_L1},
    {13, "ISO-8859-10"_L1},
    {16, "ISO-2022-JP-1"_L1},
    {17, "Shift_JIS"_L1},
    {18, "EUC-JP"_L1},
    {36, "KSC_5601"_L1},
    {37, "ISO-2022-KR"_L1},
    {38, "EUC-KR"_L1},
    {39, "ISO-2022-JP"_L1},
    {40, "ISO-2022-JP-2"_L1},
    {57, "GB_2312-80"_L1},
    {81, "ISO-8859-6-E"_L1},
    {82, "ISO-8859-6-I"_L1},
    {84, "ISO-8859-8-E"_L1},
    {85, "ISO-8859-8-I"_L1},
    {103, "UNICODE-1-1-UTF-7"_L1},
    {104, "ISO-2022-CN"_L1},
    {105, "ISO-2022-CN-EXT"_L1},
    {106, "UTF-8"_L1},
    {109, "ISO-8859-13"_L1},
    {110, "ISO-8859-14"_L1},
    {111, "ISO-8859-15"_L1},
    {112, "ISO-8859-16"_L1},
    {113, "GBK"_L1},
    {114, "GB18030"_L1},
    {1000, "ISO-10646-UCS-2"_L1},
    {1001, "ISO-10646-UCS-4"_L1},
    {1011, "SCSU"_L1},
    {1012, "UTF-7"_L1},
    {1013, "UTF-16BE"_L1},
    {1014, "UTF-16LE"_L1},
    {1015, "UTF-16"_L1},
    {1016, "CESU-8"_L1},
    {1017, "UTF-32"_L1},
    {1018, "UTF-32BE"_L1},
    {1019, "UTF-32LE"_L1},
    {1020, "BOCU-1"_L1},
    {2004, "hp-roman8"_L1},
    {2005, "Adobe-Standard-Encoding"_L1},
    {2009, "IBM850"_L1},
    {2010, "IBM852"_L1},
    {2011, "IBM437"_L1},
    {2013, "IBM862"_L1},
    {2016, "IBM-Thai"_L1},
    {2024, "Windows-31J"_L1},
    {2025, "GB2312"_L1},
    {2026, "Big5"_L1},
    {2027, "macintosh"_L1},
    {2028, "IBM037"_L1},
    {2030, "IBM273"_L1},
    {2033, "IBM277"_L1},
    {2034, "IBM278"_L1},
    {2035, "IBM280"_L1},
    {2037, "IBM284"_L1},
    {2038, "IBM285"_L1},
    {2039, "IBM290"_L1},
    {2040, "IBM297"_L1},
    {2041, "IBM420"_L1},
    {2043, "IBM424"_L1},
    {2044, "IBM500"_L1},
    {2045, "cp851"_L1},
    {2046, "IBM855"_L1},
    {2047, "IBM857"_L1},
    {2048, "IBM860"_L1},
    {2049, "IBM861"_L1},
    {2050, "IBM863"_L1},
    {2051, "IBM864"_L1},
    {2052, "IBM865"_L1},
    {2053, "IBM868"_L1},
    {2054, "IBM869"_L1},
    {2055, "IBM870"_L1},
    {2056, "IBM871"_L1},
    {2062, "IBM918"_L1},
    {2063, "IBM1026"_L1},
    {2084, "KOI8-R"_L1},
    {2085, "HZ-GB-2312"_L1},
    {2086, "IBM866"_L1},
    {2087, "IBM775"_L1},
    {2088, "KOI8-U"_L1},
    {2089, "IBM00858"_L1},
    {2091, "IBM01140"_L1},
    {2092, "IBM01141"_L1},
    {2093, "IBM01142"_L1},
    {2094, "IBM01143"_L1},
    {2095, "IBM01144"_L1},
    {2096, "IBM01145"_L1},
    {2097, "IBM01146"_L1},
    {2098, "IBM01147"_L1},
    {2099, "IBM01148"_L1},
    {2100, "IBM01149"_L1},
    {2101, "Big5-HKSCS"_L1},
    {2102, "IBM1047"_L1},
    {2109, "windows-874"_L1},
    {2250, "windows-1250"_L1},
    {2251, "windows-1251"_L1},
    {2252, "windows-1252"_L1},
    {2253, "windows-1253"_L1},
    {2254, "windows-1254"_L1},
    {2255, "windows-1255"_L1},
    {2256, "windows-1256"_L1},
    {2257, "windows-1257"_L1},
    {2258, "windows-1258"_L1},
    {2259, "TIS-620"_L1},
    {-949, "windows-949"_L1},
}};

static QTextCodec *loadQtCodec(const char *name)
{
    if (!strcmp(name, "UTF-8"))
        return new QUtf8Codec;
    if (!strcmp(name, "UTF-16"))
        return new QUtf16Codec;
    if (!strcmp(name, "ISO-8859-1"))
        return new QLatin1Codec;
    if (!strcmp(name, "UTF-16BE"))
        return new QUtf16BECodec;
    if (!strcmp(name, "UTF-16LE"))
        return new QUtf16LECodec;
    if (!strcmp(name, "UTF-32"))
        return new QUtf32Codec;
    if (!strcmp(name, "UTF-32BE"))
        return new QUtf32BECodec;
    if (!strcmp(name, "UTF-32LE"))
        return new QUtf32LECodec;
    if (!strcmp(name, "ISO-8859-16") || !strcmp(name, "latin10") || !strcmp(name, "iso-ir-226"))
        return new QSimpleTextCodec(13 /* == 8859-16*/);
#if defined(WITH_BASIC_CODECS)
    if (!strcmp(name, "TSCII"))
        return new QTsciiCodec;
    if (!qstrnicmp(name, "iscii", 5))
        return QIsciiCodec::create(name);
#endif
#if defined(Q_OS_WIN)
    if (!strcmp(name, "System"))
        return new QWindowsLocalCodec;
#endif

    return nullptr;
}

/// \threadsafe
QList<QByteArray> QIcuCodec::availableCodecs()
{
    QList<QByteArray> codecs;
    int n = ucnv_countAvailable();
    for (int i = 0; i < n; ++i) {
        const char *name = ucnv_getAvailableName(i);

        UErrorCode error = U_ZERO_ERROR;
        const char *standardName = ucnv_getStandardName(name, "MIME", &error);
        if (U_FAILURE(error) || !standardName) {
            error = U_ZERO_ERROR;
            standardName = ucnv_getStandardName(name, "IANA", &error);
        }
        if (U_FAILURE(error))
            continue;

        error = U_ZERO_ERROR;
        int ac = ucnv_countAliases(standardName, &error);
        if (U_FAILURE(error))
            continue;
        for (int j = 0; j < ac; ++j) {
            error = U_ZERO_ERROR;
            const char *alias = ucnv_getAlias(standardName, static_cast<uint16_t>(j), &error);
            if (!U_SUCCESS(error))
                continue;
            codecs += alias;
        }
    }

#if defined(WITH_BASIC_CODECS)
    // handled by Qt and not in ICU:
    codecs += "TSCII";
#endif

    return codecs;
}

/// \threadsafe
QList<int> QIcuCodec::availableMibs()
{
    QList<int> mibs;
    mibs.reserve(static_cast<vsizetype>(mibToName.size()) + 1);

    std::transform(mibToName.begin(),
                   mibToName.end(),
                   std::back_inserter(mibs),
                   [](const auto &entry) { return entry.mib; });

#if defined(WITH_BASIC_CODECS)
    // handled by Qt and not in ICU:
    mibs += 2107; // TSCII
#endif

    return mibs;
}

QTextCodec *QIcuCodec::defaultCodecUnlocked()
{
    QTextCodecData *globalData = QTextCodecData::instance();
    if (!globalData)
        return nullptr;
    QTextCodec *c = globalData->codecForLocale.loadAcquire();
    if (c)
        return c;

#if defined(QT_LOCALE_IS_UTF8)
    const char *name = "UTF-8";
#else
    const char *name = ucnv_getDefaultName();
#endif
    c = codecForNameUnlocked(name);
    globalData->codecForLocale.storeRelease(c);
    return c;
}


QTextCodec *QIcuCodec::codecForNameUnlocked(const char *name)
{
    // backwards compatibility with Qt 4.x
    if (!qstrcmp(name, "CP949"))
        name = "windows-949";
    else if (!qstrcmp(name, "Apple Roman"))
        name = "macintosh";
    // these are broken data in ICU 4.4, and can't be resolved even though they are aliases to tis-620
    if (!qstrcmp(name, "windows-874-2000")
        || !qstrcmp(name, "windows-874")
        || !qstrcmp(name, "MS874")
        || !qstrcmp(name, "x-windows-874")
        || !qstrcmp(name, "ISO 8859-11"))
        name = "TIS-620";

    UErrorCode error = U_ZERO_ERROR;
    // MIME gives better default names
    const char *standardName = ucnv_getStandardName(name, "MIME", &error);
    if (U_FAILURE(error) || !standardName) {
        error = U_ZERO_ERROR;
        standardName = ucnv_getStandardName(name, "IANA", &error);
    }
    bool qt_only = false;
    if (U_FAILURE(error) || !standardName) {
        standardName = name;
        qt_only = true;
    } else {
        // correct some issues where the ICU data set contains duplicated entries.
        // Where this happens it's because one data set is a subset of another. We
        // always use the larger data set.

        if (qstrcmp(standardName, "GB2312") == 0 || qstrcmp(standardName, "GB_2312-80") == 0)
            standardName = "GBK";
        else if (qstrcmp(standardName, "KSC_5601") == 0 || qstrcmp(standardName, "EUC-KR") == 0 || qstrcmp(standardName, "cp1363") == 0)
            standardName = "windows-949";
    }

    QTextCodecData *globalData = QTextCodecData::instance();
    QTextCodecCache *cache = &globalData->codecCache;

    QTextCodec *codec;
    if (cache) {
        codec = cache->value(standardName);
        if (codec)
            return codec;
    }

    for (TextCodecListConstIt it = globalData->allCodecs.constBegin(), cend = globalData->allCodecs.constEnd(); it != cend; ++it) {
        QTextCodec *cursor = *it;
        if (qTextCodecNameMatch(cursor->name(), standardName)) {
            if (cache)
                cache->insert(standardName, cursor);
            return cursor;
        }
        QList<QByteArray> aliases = cursor->aliases();
        for (ByteArrayListConstIt ait = aliases.constBegin(), acend = aliases.constEnd(); ait != acend; ++ait) {
            if (qTextCodecNameMatch(*ait, standardName)) {
                if (cache)
                    cache->insert(standardName, cursor);
                return cursor;
            }
        }
    }

    QTextCodec *c = loadQtCodec(standardName);
    if (c)
        return c;

    if (qt_only)
        return nullptr;

    // check whether there is really a converter for the name available.
    UConverter *conv = ucnv_open(standardName, &error);
    if (!conv) {
        qDebug("codecForName: ucnv_open failed %s %s", standardName, u_errorName(error));
        return nullptr;
    }
    //qDebug() << "QIcuCodec: Standard name for " << name << "is" << standardName;
    ucnv_close(conv);


    c = new QIcuCodec(standardName);
    if (cache)
        cache->insert(standardName, c);
    return c;
}


QTextCodec *QIcuCodec::codecForMibUnlocked(int mib)
{
    if (auto it = std::find_if(mibToName.begin(),
                               mibToName.end(),
                               [mib](const auto &entry) { return entry.mib == mib; });
        it != mibToName.end())
    {
        return codecForNameUnlocked(it->name.data());
    }

#if defined(WITH_BASIC_CODECS)
    if (mib == 2107)
    {
        return codecForNameUnlocked("TSCII");
    }
#endif

    return nullptr;
}


QIcuCodec::QIcuCodec(const char *name)
    : m_name(name)
{
}

/*!
    \internal

    Custom callback for the ICU from Unicode conversion. It's invoked when the
    conversion from Unicode detects illegal or unrecognized character.

    Assumes that context contains a pointer to QTextCodec::ConverterState
    structure. Updates its invalid characters count and calls a default
    callback, that replaces the invalid characters properly.
*/
static void customFromUnicodeSubstitutionCallback(const void *context,
                                                  UConverterFromUnicodeArgs *fromUArgs,
                                                  const UChar *codeUnits,
                                                  int32_t length,
                                                  UChar32 codePoint,
                                                  UConverterCallbackReason reason,
                                                  UErrorCode *err)
{
    auto *state = reinterpret_cast<QTextCodec::ConverterState *>(const_cast<void *>(context));
    if (state)
        state->invalidChars++;
    // Call the default callback that replaces all illegal or unrecognized
    // sequences with the substitute string
    UCNV_FROM_U_CALLBACK_SUBSTITUTE(nullptr, fromUArgs, codeUnits, length, codePoint, reason, err);
}

UConverter *QIcuCodec::getConverter(QTextCodec::ConverterState *state) const
{
    UConverter *conv = nullptr;
    if (state) {
        if (!state->d[0]) {
            // first time
            state->clearFn = qIcuCodecStateFree;
            UErrorCode error = U_ZERO_ERROR;
            state->d[0] = ucnv_open(m_name, &error);
            ucnv_setSubstChars(static_cast<UConverter *>(state->d[0]),
                               state->flags & QTextCodec::ConvertInvalidToNull ? "\0" : "?", 1, &error);
            if (U_FAILURE(error)) {
                qDebug("getConverter(state) ucnv_open failed %s %s", m_name, u_errorName(error));
            } else {
                error = U_ZERO_ERROR;
                ucnv_setFromUCallBack(static_cast<UConverter *>(state->d[0]),
                                      customFromUnicodeSubstitutionCallback, state, nullptr,
                                      nullptr, &error);
                if (U_FAILURE(error)) {
                    qDebug("getConverter(state) failed to install custom callback. "
                           "canEncode() may report incorrect results.");
                }
            }
        }
        conv = static_cast<UConverter *>(state->d[0]);
    }
    if (!conv) {
        // stateless conversion
        UErrorCode error = U_ZERO_ERROR;
        conv = ucnv_open(m_name, &error);
        ucnv_setSubstChars(conv, "?", 1, &error);
        if (U_FAILURE(error))
            qDebug("getConverter(no state) ucnv_open failed %s %s", m_name, u_errorName(error));
    }
    return conv;
}

QString QIcuCodec::convertToUnicode(const char *chars, int length, QTextCodec::ConverterState *state) const
{
    UConverter *conv = getConverter(state);

    QString string(length + 2, QT_PREPEND_NAMESPACE(Qt::Uninitialized));

    const char *end = chars + length;
    int convertedChars = 0;
    while (1) {
        UChar *uc = reinterpret_cast<UChar *>(string.data());
        UChar *ucEnd = uc + string.size();
        uc += convertedChars;
        UErrorCode error = U_ZERO_ERROR;
        ucnv_toUnicode(conv,
                       &uc, ucEnd,
                       &chars, end,
                       nullptr, false, &error);
        if (!U_SUCCESS(error) && error != U_BUFFER_OVERFLOW_ERROR) {
            qDebug("convertToUnicode failed: %s", u_errorName(error));
            break;
        }
        // flag the state if we have incomplete input
        if (error == U_TRUNCATED_CHAR_FOUND)
            state->remainingChars = 1;

        convertedChars = static_cast<int>(uc - reinterpret_cast<UChar *>(string.data()));
        if (chars >= end)
            break;
        string.resize(string.size()*2);
    }
    string.resize(convertedChars);

    if (!state)
        ucnv_close(conv);
    return string;
}


QByteArray QIcuCodec::convertFromUnicode(const QChar *unicode, int length, QTextCodec::ConverterState *state) const
{
    UConverter *conv = getConverter(state);

    int requiredLength = UCNV_GET_MAX_BYTES_FOR_STRING(length, ucnv_getMaxCharSize(conv));
    QByteArray string(requiredLength, QT_PREPEND_NAMESPACE(Qt::Uninitialized));

    const UChar *uc = reinterpret_cast<const UChar *>(unicode);
    const UChar *end = uc + length;
    int convertedChars = 0;
    while (1) {
        char *ch = reinterpret_cast<char *>(string.data());
        char *chEnd = ch + string.size();
        ch += convertedChars;
        UErrorCode error = U_ZERO_ERROR;
        ucnv_fromUnicode(conv,
                         &ch, chEnd,
                         &uc, end,
                         nullptr, false, &error);
        if (!U_SUCCESS(error))
            qDebug("convertFromUnicode failed: %s", u_errorName(error));
        // flag the state if we have incomplete input
        if (error == U_TRUNCATED_CHAR_FOUND)
            state->remainingChars = 1;

        convertedChars = static_cast<int>(ch - string.data());
        if (uc >= end)
            break;
        string.resize(string.size()*2);
    }
    string.resize(convertedChars);

    if (!state)
        ucnv_close(conv);

    return string;
}


QByteArray QIcuCodec::name() const
{
    return m_name;
}


QList<QByteArray> QIcuCodec::aliases() const
{
    UErrorCode error = U_ZERO_ERROR;

    int const n = ucnv_countAliases(m_name, &error);

    QList<QByteArray> aliases;
    for (int i = 0; i < n; ++i)
    {
        const char *a = ucnv_getAlias(m_name, static_cast<uint16_t>(i), &error);
        // skip the canonical name
        if (!a || !qstrcmp(a, m_name))
        {
            continue;
        }
        aliases += a;
    }

    return aliases;
}


int QIcuCodec::mibEnum() const
{
    if (const auto *it = std::find_if(mibToName.begin(),
                                      mibToName.end(),
                                      [this](const auto &entry)
                                      { return qTextCodecNameMatch(m_name, entry.name.data()); });
        it != mibToName.end())
    {
        return it->mib;
    }

    return 0;
}

QT_END_NAMESPACE
