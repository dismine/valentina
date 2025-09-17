/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

//#define VTEXTSTREAM_DEBUG

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <qtypes.h>
#endif

static const int QTEXTSTREAM_BUFFERSIZE = 16384;

/*!
    \class VTextStream
    \inmodule QtCore

    \brief The VTextStream class provides a convenient interface for
    reading and writing text.

    \ingroup io
    \ingroup string-processing
    \reentrant

    VTextStream can operate on a QIODevice, a QByteArray or a
    QString. Using VTextStream's streaming operators, you can
    conveniently read and write words, lines and numbers. For
    generating text, VTextStream supports formatting options for field
    padding and alignment, and formatting of numbers. Example:

    \snippet code/src_corelib_io_qtextstream.cpp 0

    It's also common to use VTextStream to read console input and write
    console output. VTextStream is locale aware, and will automatically decode
    standard input using the correct codec. Example:

    \snippet code/src_corelib_io_qtextstream.cpp 1

    Besides using VTextStream's constructors, you can also set the
    device or string VTextStream operates on by calling setDevice() or
    setString(). You can seek to a position by calling seek(), and
    atEnd() will return true when there is no data left to be read. If
    you call flush(), VTextStream will empty all data from its write
    buffer into the device and call flush() on the device.

    Internally, VTextStream uses a Unicode based buffer, and
    QTextCodec is used by VTextStream to automatically support
    different character sets. By default, QTextCodec::codecForLocale()
    is used for reading and writing, but you can also set the codec by
    calling setCodec(). Automatic Unicode detection is also
    supported. When this feature is enabled (the default behavior),
    VTextStream will detect the UTF-16 or the UTF-32 BOM (Byte Order Mark) and
    switch to the appropriate UTF codec when reading. VTextStream
    does not write a BOM by default, but you can enable this by calling
    setGenerateByteOrderMark(true). When VTextStream operates on a QString
    directly, the codec is disabled.

    There are three general ways to use VTextStream when reading text
    files:

    \list

    \li Chunk by chunk, by calling readLine() or readAll().

    \li Word by word. VTextStream supports streaming into \l {QString}s,
    \l {QByteArray}s and char* buffers. Words are delimited by space, and
    leading white space is automatically skipped.

    \li Character by character, by streaming into QChar or char types.
    This method is often used for convenient input handling when
    parsing files, independent of character encoding and end-of-line
    semantics. To skip white space, call skipWhiteSpace().

    \endlist

    Since the text stream uses a buffer, you should not read from
    the stream using the implementation of a superclass. For instance,
    if you have a QFile and read from it directly using
    QFile::readLine() instead of using the stream, the text stream's
    internal position will be out of sync with the file's position.

    By default, when reading numbers from a stream of text,
    VTextStream will automatically detect the number's base
    representation. For example, if the number starts with "0x", it is
    assumed to be in hexadecimal form. If it starts with the digits
    1-9, it is assumed to be in decimal form, and so on. You can set
    the integer base, thereby disabling the automatic detection, by
    calling setIntegerBase(). Example:

    \snippet code/src_corelib_io_qtextstream.cpp 2

    VTextStream supports many formatting options for generating text.
    You can set the field width and pad character by calling
    setFieldWidth() and setPadChar(). Use setFieldAlignment() to set
    the alignment within each field. For real numbers, call
    setRealNumberNotation() and setRealNumberPrecision() to set the
    notation (SmartNotation, ScientificNotation, FixedNotation) and precision in
    digits of the generated number. Some extra number formatting
    options are also available through setNumberFlags().

    \target VTextStream manipulators

    Like \c <iostream> in the standard C++ library, VTextStream also
    defines several global manipulator functions:

    \table
    \header \li Manipulator        \li Description
    \row    \li Val::bin             \li Same as setIntegerBase(2).
    \row    \li Val::oct             \li Same as setIntegerBase(8).
    \row    \li Val::dec             \li Same as setIntegerBase(10).
    \row    \li Val::hex             \li Same as setIntegerBase(16).
    \row    \li Val::showbase        \li Same as setNumberFlags(numberFlags() | ShowBase).
    \row    \li Val::forcesign       \li Same as setNumberFlags(numberFlags() | ForceSign).
    \row    \li Val::forcepoint      \li Same as setNumberFlags(numberFlags() | ForcePoint).
    \row    \li Val::noshowbase      \li Same as setNumberFlags(numberFlags() & ~ShowBase).
    \row    \li Val::noforcesign     \li Same as setNumberFlags(numberFlags() & ~ForceSign).
    \row    \li Val::noforcepoint    \li Same as setNumberFlags(numberFlags() & ~ForcePoint).
    \row    \li Val::uppercasebase   \li Same as setNumberFlags(numberFlags() | UppercaseBase).
    \row    \li Val::uppercasedigits \li Same as setNumberFlags(numberFlags() | UppercaseDigits).
    \row    \li Val::lowercasebase   \li Same as setNumberFlags(numberFlags() & ~UppercaseBase).
    \row    \li Val::lowercasedigits \li Same as setNumberFlags(numberFlags() & ~UppercaseDigits).
    \row    \li Val::fixed           \li Same as setRealNumberNotation(FixedNotation).
    \row    \li Val::scientific      \li Same as setRealNumberNotation(ScientificNotation).
    \row    \li Val::left            \li Same as setFieldAlignment(AlignLeft).
    \row    \li Val::right           \li Same as setFieldAlignment(AlignRight).
    \row    \li Val::center          \li Same as setFieldAlignment(AlignCenter).
    \row    \li Val::endl            \li Same as operator<<('\\n') and flush().
    \row    \li Val::flush           \li Same as flush().
    \row    \li Val::reset           \li Same as reset().
    \row    \li Val::ws              \li Same as skipWhiteSpace().
    \row    \li Val::bom             \li Same as setGenerateByteOrderMark(true).
    \endtable

    In addition, Qt provides three global manipulators that take a
    parameter: qSetFieldWidth(), qSetPadChar(), and
    qSetRealNumberPrecision().

    \sa QDataStream, QIODevice, QFile, QBuffer, QTcpSocket, {Text Codecs Example}
*/

/*! \enum VTextStream::RealNumberNotation

    This enum specifies which notations to use for expressing \c
    float and \c double as strings.

    \value ScientificNotation Scientific notation (\c{printf()}'s \c %e flag).
    \value FixedNotation Fixed-point notation (\c{printf()}'s \c %f flag).
    \value SmartNotation Scientific or fixed-point notation, depending on which makes most sense (\c{printf()}'s \c %g flag).

    \sa setRealNumberNotation()
*/

/*! \enum VTextStream::FieldAlignment

    This enum specifies how to align text in fields when the field is
    wider than the text that occupies it.

    \value AlignLeft  Pad on the right side of fields.
    \value AlignRight  Pad on the left side of fields.
    \value AlignCenter  Pad on both sides of field.
    \value AlignAccountingStyle  Same as AlignRight, except that the
                                 sign of a number is flush left.

    \sa setFieldAlignment()
*/

/*! \enum VTextStream::NumberFlag

    This enum specifies various flags that can be set to affect the
    output of integers, \c{float}s, and \c{double}s.

    \value ShowBase  Show the base as a prefix if the base
                     is 16 ("0x"), 8 ("0"), or 2 ("0b").
    \value ForcePoint  Always put the decimal separator in numbers, even if
                       there are no decimals.
    \value ForceSign  Always put the sign in numbers, even for positive numbers.
    \value UppercaseBase  Use uppercase versions of base prefixes ("0X", "0B").
    \value UppercaseDigits  Use uppercase letters for expressing
                            digits 10 to 35 instead of lowercase.

    \sa setNumberFlags()
*/

/*! \enum VTextStream::Status

    This enum describes the current status of the text stream.

    \value Ok               The text stream is operating normally.
    \value ReadPastEnd      The text stream has read past the end of the
                            data in the underlying device.
    \value ReadCorruptData  The text stream has read corrupt data.
    \value WriteFailed      The text stream cannot write to the underlying device.

    \sa status()
*/

#include "vtextstream.h"
#include "vtextstream_p.h"

#include "qnumeric.h"
#include <QBuffer>
#include <QFile>

#include <clocale>
#include <limits.h>
#include <new>
#include <stdlib.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

#if defined VTEXTSTREAM_DEBUG
#include <ctype.h>
#include "private/qtools_p.h"

// Returns a human readable representation of the first \a len
// characters in \a data.
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(int(uchar(c)))) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: {
            const char buf[] = {
                '\\',
                'x',
                QtMiscUtils::toHexLower(uchar(c) / 16),
                QtMiscUtils::toHexLower(uchar(c) % 16),
                0
            };
            out += buf;
            }
        }
    }

    if (len < maxSize)
        out += "...";

    return out;
}

#endif

// A precondition macro
#define Q_VOID
#define CHECK_VALID_STREAM(x) \
    do \
    { \
        if (!d->string && !d->device) \
        { \
            qWarning("VTextStream: No device"); \
            return x; \
        } \
    } while (0)

// Base implementations of operator>> for ints and reals
#define IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(type) \
    do \
    { \
        Q_D(VTextStream); \
        CHECK_VALID_STREAM(*this); \
        qulonglong tmp; \
        QT_WARNING_PUSH \
        QT_WARNING_DISABLE_GCC("-Wswitch-default") \
        QT_WARNING_DISABLE_CLANG("-Wswitch-default") \
        switch (d->getNumber(&tmp)) \
        { \
            case VTextStreamPrivate::npsOk: \
                i = static_cast<type>(tmp); \
                break; \
            case VTextStreamPrivate::npsMissingDigit: \
            case VTextStreamPrivate::npsInvalidPrefix: \
                i = static_cast<type>(0); \
                setStatus(atEnd() ? VTextStream::ReadPastEnd : VTextStream::ReadCorruptData); \
                break; \
        } \
        QT_WARNING_POP \
        return *this; \
    } while (0)

#define IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(type) \
    do \
    { \
        Q_D(VTextStream); \
        CHECK_VALID_STREAM(*this); \
        double tmp; \
        if (d->getReal(&tmp)) \
        { \
            f = static_cast<type>(tmp); \
        } \
        else \
        { \
            f = static_cast<type>(0); \
            setStatus(atEnd() ? VTextStream::ReadPastEnd : VTextStream::ReadCorruptData); \
        } \
        return *this; \
    } while (0)

//-------------------------------------------------------------------

/*!
    \internal
*/
VTextStreamPrivate::VTextStreamPrivate(VTextStream *q_ptr)
  :
#if defined(WITH_TEXTCODEC)
    readConverterSavedState(nullptr),
#endif
    readConverterSavedStateOffset(0),
    locale(QLocale::c())
{
    this->q_ptr = q_ptr;
    reset();
}

/*!
    \internal
*/
VTextStreamPrivate::~VTextStreamPrivate()
{
    if (deleteDevice)
    {
        device->blockSignals(true);
        delete device;
    }
#if defined(WITH_TEXTCODEC)
    delete readConverterSavedState;
#endif
}

#if defined(WITH_TEXTCODEC)
static void resetCodecConverterStateHelper(QTextCodec::ConverterState *state)
{
    state->~State();
    new (state) QTextCodec::ConverterState;
}

static void copyConverterStateHelper(QTextCodec::ConverterState *dest,
    const QTextCodec::ConverterState *src)
{
    dest->flags = src->flags;
    dest->invalidChars = src->invalidChars;
    dest->state_data[0] = src->state_data[0];
    dest->state_data[1] = src->state_data[1];
    dest->state_data[2] = src->state_data[2];
}
#endif

void VTextStreamPrivate::Params::reset()
{
    realNumberPrecision = 6;
    integerBase = 0;
    fieldWidth = 0;
    padChar = QLatin1Char(' ');
    fieldAlignment = VTextStream::AlignRight;
    realNumberNotation = VTextStream::SmartNotation;
    numberFlags = { };
}

/*!
    \internal
*/
void VTextStreamPrivate::reset()
{
    params.reset();

    device = nullptr;
    deleteDevice = false;
    string = nullptr;
    stringOffset = 0;
    stringOpenMode = QIODevice::NotOpen;

    readBufferOffset = 0;
    readBufferStartDevicePos = 0;
    lastTokenSize = 0;

#if defined(WITH_TEXTCODEC)
    codec = QTextCodec::codecForLocale();
    resetCodecConverterStateHelper(&readConverterState);
    resetCodecConverterStateHelper(&writeConverterState);
    delete readConverterSavedState;
    readConverterSavedState = nullptr;
    writeConverterState.flags |= QTextCodec::IgnoreHeader;
    autoDetectUnicode = true;
#endif
}

/*!
    \internal
*/
bool VTextStreamPrivate::fillReadBuffer(qint64 maxBytes)
{
    // no buffer next to the QString itself; this function should only
    // be called internally, for devices.
    Q_ASSERT(!string);
    Q_ASSERT(device);

    // handle text translation and bypass the Text flag in the device.
    bool textModeEnabled = device->isTextModeEnabled();
    if (textModeEnabled)
        device->setTextModeEnabled(false);

    // read raw data into a temporary buffer
    char buf[QTEXTSTREAM_BUFFERSIZE];
    qint64 bytesRead = 0;
#if defined(Q_OS_WIN)
    // On Windows, there is no non-blocking stdin - so we fall back to reading
    // lines instead. If there is no QOBJECT, we read lines for all sequential
    // devices; otherwise, we read lines only for stdin.
    QFile *file = 0;
    Q_UNUSED(file);
    if (device->isSequential()
        && (file = qobject_cast<QFile *>(device)) && file->handle() == 0
        ) {
        if (maxBytes != -1)
            bytesRead = device->readLine(buf, qMin<qint64>(sizeof(buf), maxBytes));
        else
            bytesRead = device->readLine(buf, sizeof(buf));
    } else
#endif
    {
        if (maxBytes != -1)
            bytesRead = device->read(buf, qMin<qint64>(sizeof(buf), maxBytes));
        else
            bytesRead = device->read(buf, sizeof(buf));
    }

    // reset the Text flag.
    if (textModeEnabled)
        device->setTextModeEnabled(true);

    if (bytesRead <= 0)
        return false;

#if defined(WITH_TEXTCODEC)
    // codec auto detection, explicitly defaults to locale encoding if the
    // codec has been set to 0.
    if (!codec || autoDetectUnicode) {
        autoDetectUnicode = false;

        codec = QTextCodec::codecForUtfText(QByteArray::fromRawData(buf, bytesRead), codec);
        if (!codec) {
            codec = QTextCodec::codecForLocale();
            writeConverterState.flags |= QTextCodec::IgnoreHeader;
        }
    }
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::fillReadBuffer(), using %s codec", codec ? codec->name().constData() : "no");
#endif
#endif

#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::fillReadBuffer(), device->read(\"%s\", %d) == %d",
           qt_prettyDebug(buf, qMin(32, int(bytesRead)), int(bytesRead)).constData(),
           int(sizeof(buf)),
           int(bytesRead));
#endif

    qsizetype oldReadBufferSize = readBuffer.size();
#if defined(WITH_TEXTCODEC)
    // convert to unicode
    readBuffer += Q_LIKELY(codec) ? codec->toUnicode(buf, static_cast<int>(bytesRead), &readConverterState)
                                  : QString::fromLatin1(buf, bytesRead);
#else
    readBuffer += QString::fromLatin1(buf, bytesRead);
#endif

    // remove all '\r\n' in the string.
    if (readBuffer.size() > oldReadBufferSize && textModeEnabled) {
        QChar CR = QLatin1Char('\r');
        QChar *writePtr = readBuffer.data() + oldReadBufferSize;
        QChar *readPtr = readBuffer.data() + oldReadBufferSize;
        QChar *endPtr = readBuffer.data() + readBuffer.size();

        qsizetype n = oldReadBufferSize;
        if (readPtr < endPtr) {
            // Cut-off to avoid unnecessary self-copying.
            while (*readPtr++ != CR) {
                ++n;
                if (++writePtr == endPtr)
                    break;
            }
        }
        while (readPtr < endPtr) {
            QChar ch = *readPtr++;
            if (ch != CR) {
                *writePtr++ = ch;
            } else {
                if (n < readBufferOffset)
                    --readBufferOffset;
                --bytesRead;
            }
            ++n;
        }
        readBuffer.resize(writePtr - readBuffer.data());
    }

#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::fillReadBuffer() read %d bytes from device. readBuffer = [%s]",
           int(bytesRead),
           qt_prettyDebug(readBuffer.toLatin1(), readBuffer.size(), readBuffer.size()).data());
#endif
    return true;
}

/*!
    \internal
*/
void VTextStreamPrivate::resetReadBuffer()
{
    readBuffer.clear();
    readBufferOffset = 0;
    readBufferStartDevicePos = (device ? device->pos() : 0);
}

/*!
    \internal
*/
void VTextStreamPrivate::flushWriteBuffer()
{
    // no buffer next to the QString itself; this function should only
    // be called internally, for devices.
    if (string || !device)
        return;

    // Stream went bye-bye already. Appending further data may succeed again,
    // but would create a corrupted stream anyway.
    if (status != VTextStream::Ok)
        return;

    if (writeBuffer.isEmpty())
        return;

#if defined (Q_OS_WIN)
    // handle text translation and bypass the Text flag in the device.
    bool textModeEnabled = device->isTextModeEnabled();
    if (textModeEnabled) {
        device->setTextModeEnabled(false);
        writeBuffer.replace(QLatin1Char('\n'), QLatin1String("\r\n"));
    }
#endif

#if defined(WITH_TEXTCODEC)
    if (!codec)
        codec = QTextCodec::codecForLocale();
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::flushWriteBuffer(), using %s codec (%s generating BOM)",
           codec ? codec->name().constData() : "no",
           !codec || (writeConverterState.flags & QTextCodec::IgnoreHeader) ? "not" : "");
#endif

    // convert from unicode to raw data
    // codec might be null if we're already inside global destructors (QTestCodec::codecForLocale returned null)
    QByteArray data = Q_LIKELY(codec) ? codec->fromUnicode(writeBuffer.data(),
                                                           static_cast<int>(writeBuffer.size()),
                                                           &writeConverterState)
                                      : writeBuffer.toLatin1();
#else
    QByteArray data = writeBuffer.toLatin1();
#endif
    writeBuffer.clear();

    // write raw data to the device
    qint64 bytesWritten = device->write(data);
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::flushWriteBuffer(), device->write(\"%s\") == %d",
           qt_prettyDebug(data.constData(), qMin(data.size(), 32), data.size()).constData(),
           int(bytesWritten));
#endif

#if defined (Q_OS_WIN)
    // reset the text flag
    if (textModeEnabled)
        device->setTextModeEnabled(true);
#endif

    if (bytesWritten <= 0) {
        status = VTextStream::WriteFailed;
        return;
    }

    // flush the file
    QFileDevice *file = qobject_cast<QFileDevice *>(device);
    bool flushed = !file || file->flush();

#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::flushWriteBuffer() wrote %d bytes", int(bytesWritten));
#endif
    if (!flushed || bytesWritten != qint64(data.size()))
        status = VTextStream::WriteFailed;
}

QString VTextStreamPrivate::read(qsizetype maxlen)
{
    QString ret;
    if (string) {
        lastTokenSize = qMin(maxlen, string->size() - stringOffset);
        ret = string->mid(stringOffset, lastTokenSize);
    } else {
        while (readBuffer.size() - readBufferOffset < maxlen && fillReadBuffer()) ;
        lastTokenSize = qMin(maxlen, readBuffer.size() - readBufferOffset);
        ret = readBuffer.mid(readBufferOffset, lastTokenSize);
    }
    consumeLastToken();

#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::read() maxlen = %d, token length = %d", maxlen, ret.length());
#endif
    return ret;
}

/*!
    \internal

    Scans no more than \a maxlen QChars in the current buffer for the
    first \a delimiter. Stores a pointer to the start offset of the
    token in \a ptr, and the length in QChars in \a length.
*/
bool VTextStreamPrivate::scan(const QChar **ptr, int *length, int maxlen, TokenDelimiter delimiter)
{
    int totalSize = 0;
    int delimSize = 0;
    bool consumeDelimiter = false;
    bool foundToken = false;
    qsizetype startOffset = device ? readBufferOffset : stringOffset;
    QChar lastChar;

    bool canStillReadFromDevice = true;
    do {
        qsizetype endOffset;
        const QChar *chPtr;
        if (device) {
            chPtr = readBuffer.constData();
            endOffset = readBuffer.size();
        } else {
            chPtr = string->constData();
            endOffset = string->size();
        }
        chPtr += startOffset;

        for (; !foundToken && startOffset < endOffset && (!maxlen || totalSize < maxlen); ++startOffset) {
            const QChar ch = *chPtr++;
            ++totalSize;

            QT_WARNING_PUSH
            QT_WARNING_DISABLE_GCC("-Wswitch-default")
            QT_WARNING_DISABLE_CLANG("-Wswitch-default")

            switch (delimiter)
            {
                case Space:
                    if (ch.isSpace())
                    {
                        foundToken = true;
                        delimSize = 1;
                    }
                    break;
                case NotSpace:
                    if (!ch.isSpace())
                    {
                        foundToken = true;
                        delimSize = 1;
                    }
                    break;
                case EndOfLine:
                    if (ch == QLatin1Char('\n'))
                    {
                        foundToken = true;
                        delimSize = (lastChar == QLatin1Char('\r')) ? 2 : 1;
                        consumeDelimiter = true;
                    }
                    lastChar = ch;
                    break;
            }

            QT_WARNING_POP
        }
    } while (!foundToken
             && (!maxlen || totalSize < maxlen)
             && (device && (canStillReadFromDevice = fillReadBuffer())));

    if (totalSize == 0) {
#if defined(VTEXTSTREAM_DEBUG)
        qDebug("VTextStreamPrivate::scan() reached the end of input.");
#endif
        return false;
    }

    // if we find a '\r' at the end of the data when reading lines,
    // don't make it part of the line.
    if (delimiter == EndOfLine && totalSize > 0 && !foundToken) {
        if (((string && stringOffset + totalSize == string->size()) || (device && device->atEnd()))
            && lastChar == QLatin1Char('\r')) {
            consumeDelimiter = true;
            ++delimSize;
        }
    }

    // set the read offset and length of the token
    if (length)
        *length = totalSize - delimSize;
    if (ptr)
        *ptr = readPtr();

    // update last token size. the callee will call consumeLastToken() when
    // done.
    lastTokenSize = totalSize;
    if (!consumeDelimiter)
        lastTokenSize -= delimSize;

#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::scan(%p, %p, %d, %x) token length = %d, delimiter = %d",
           ptr,
           length,
           maxlen,
           (int) delimiter,
           totalSize - delimSize,
           delimSize);
#endif
    return true;
}

/*!
    \internal
*/
inline const QChar *VTextStreamPrivate::readPtr() const
{
    Q_ASSERT(readBufferOffset <= readBuffer.size());
    if (string)
        return string->constData() + stringOffset;
    return readBuffer.constData() + readBufferOffset;
}

/*!
    \internal
*/
inline void VTextStreamPrivate::consumeLastToken()
{
    if (lastTokenSize)
        consume(lastTokenSize);
    lastTokenSize = 0;
}

/*!
    \internal
*/
inline void VTextStreamPrivate::consume(qsizetype size)
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStreamPrivate::consume(%d)", size);
#endif
    if (string) {
        stringOffset += size;
        if (stringOffset > string->size())
            stringOffset = string->size();
    } else {
        readBufferOffset += size;
        if (readBufferOffset >= readBuffer.size()) {
            readBufferOffset = 0;
            readBuffer.clear();
            saveConverterState(device->pos());
        } else if (readBufferOffset > QTEXTSTREAM_BUFFERSIZE) {
            readBuffer = readBuffer.remove(0,readBufferOffset);
            readConverterSavedStateOffset += readBufferOffset;
            readBufferOffset = 0;
        }
    }
}

/*!
    \internal
*/
inline void VTextStreamPrivate::saveConverterState(qint64 newPos)
{
#if defined(WITH_TEXTCODEC)
    if (readConverterState.clearFn)
    {
        // converter cannot be copied, so don't save anything
        // don't update readBufferStartDevicePos either
        return;
    }

    if (!readConverterSavedState)
    {
        readConverterSavedState = new QTextCodec::ConverterState;
    }
    copyConverterStateHelper(readConverterSavedState, &readConverterState);
#endif

    readBufferStartDevicePos = newPos;
    readConverterSavedStateOffset = 0;
}

/*!
    \internal
*/
inline void VTextStreamPrivate::restoreToSavedConverterState()
{
#if defined(WITH_TEXTCODEC)
    if (readConverterSavedState) {
        // we have a saved state
        // that means the converter can be copied
        copyConverterStateHelper(&readConverterState, readConverterSavedState);
    } else {
        // the only state we could save was the initial
        // so reset to that
        resetCodecConverterStateHelper(&readConverterState);
    }
#endif
}

/*!
    \internal
*/
void VTextStreamPrivate::write(const QChar *data, qsizetype len)
{
    if (string) {
        // ### What about seek()??
        string->append(data, len);
    } else {
        writeBuffer.append(data, len);
        if (writeBuffer.size() > QTEXTSTREAM_BUFFERSIZE)
            flushWriteBuffer();
    }
}

/*!
    \internal
*/
inline void VTextStreamPrivate::write(QChar ch)
{
    if (string) {
        // ### What about seek()??
        string->append(ch);
    } else {
        writeBuffer += ch;
        if (writeBuffer.size() > QTEXTSTREAM_BUFFERSIZE)
            flushWriteBuffer();
    }
}

/*!
    \internal
*/
void VTextStreamPrivate::write(QLatin1String data)
{
    if (string) {
        // ### What about seek()??
        string->append(data);
    } else {
        writeBuffer += data;
        if (writeBuffer.size() > QTEXTSTREAM_BUFFERSIZE)
            flushWriteBuffer();
    }
}

/*!
    \internal
*/
void VTextStreamPrivate::writePadding(qsizetype len)
{
    if (string) {
        // ### What about seek()??
        string->resize(string->size() + len, params.padChar);
    } else {
        writeBuffer.resize(writeBuffer.size() + len, params.padChar);
        if (writeBuffer.size() > QTEXTSTREAM_BUFFERSIZE)
            flushWriteBuffer();
    }
}

/*!
    \internal
*/
inline bool VTextStreamPrivate::getChar(QChar *ch)
{
    if ((string && stringOffset == string->size())
        || (device && readBuffer.isEmpty() && !fillReadBuffer())) {
        if (ch)
            *ch = QChar();
        return false;
    }
    if (ch)
        *ch = *readPtr();
    consume(1);
    return true;
}

/*!
    \internal
*/
inline void VTextStreamPrivate::ungetChar(QChar ch)
{
    if (string) {
        if (stringOffset == 0)
            string->prepend(ch);
        else
            (*string)[--stringOffset] = ch;
        return;
    }

    if (readBufferOffset == 0) {
        readBuffer.prepend(ch);
        return;
    }

    readBuffer[--readBufferOffset] = ch;
}

/*!
    \internal
*/
inline void VTextStreamPrivate::putChar(QChar ch)
{
    if (params.fieldWidth > 0)
        putString(&ch, 1);
    else
        write(ch);
}


/*!
    \internal
*/
VTextStreamPrivate::PaddingResult VTextStreamPrivate::padding(qsizetype len) const
{
    Q_ASSERT(params.fieldWidth > len); // calling padding() when no padding is needed is an error

    qsizetype left = 0, right = 0;

    const qsizetype padSize = params.fieldWidth - len;

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (params.fieldAlignment) {
        case VTextStream::AlignLeft:
            right = padSize;
            break;
        case VTextStream::AlignRight:
        case VTextStream::AlignAccountingStyle:
            left = padSize;
            break;
        case VTextStream::AlignCenter:
            left = padSize / 2;
            right = padSize - padSize / 2;
            break;
    }

    QT_WARNING_POP
    return { left, right };
}

/*!
    \internal
*/
void VTextStreamPrivate::putString(const QChar *data, qsizetype len, bool number)
{
    if (Q_UNLIKELY(params.fieldWidth > len)) {

        // handle padding:

        const PaddingResult pad = padding(len);

        if (params.fieldAlignment == VTextStream::AlignAccountingStyle && number)
        {
            const QChar sign = len > 0 ? data[0] : QChar();
            if (sign == locale.negativeSign() || sign == locale.positiveSign()) {
                // write the sign before the padding, then skip it later
                write(&sign, 1);
                ++data;
                --len;
            }
        }

        writePadding(pad.left);
        write(data, len);
        writePadding(pad.right);
    } else {
        write(data, len);
    }
}

/*!
    \internal
*/
void VTextStreamPrivate::putString(QLatin1String data, bool number)
{
    if (Q_UNLIKELY(params.fieldWidth > data.size())) {

        // handle padding

        const PaddingResult pad = padding(data.size());

        if (params.fieldAlignment == VTextStream::AlignAccountingStyle && number)
        {
            const QChar sign = data.size() > 0 ? QLatin1Char(*data.data()) : QChar();
            if (sign == locale.negativeSign() || sign == locale.positiveSign()) {
                // write the sign before the padding, then skip it later
                write(&sign, 1);
                data = QLatin1String(data.data() + 1, data.size() - 1);
            }
        }

        writePadding(pad.left);
        write(data);
        writePadding(pad.right);
    } else {
        write(data);
    }
}

/*!
    Constructs a VTextStream. Before you can use it for reading or
    writing, you must assign a device or a string.

    \sa setDevice(), setString()
*/
VTextStream::VTextStream()
  : d_ptr(new VTextStreamPrivate(this))
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::VTextStream()");
#endif
    Q_D(VTextStream);
    d->status = Ok;
}

/*!
    Constructs a VTextStream that operates on \a device.
*/
VTextStream::VTextStream(QIODevice *device)
  : d_ptr(new VTextStreamPrivate(this))
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::VTextStream(QIODevice *device == *%p)", device);
#endif
    Q_D(VTextStream);
    d->device = device;
    d->deviceClosedNotifier.setupDevice(this, d->device);
    d->status = Ok;
}

/*!
    Constructs a VTextStream that operates on \a string, using \a
    openMode to define the open mode.
*/
VTextStream::VTextStream(QString *string, QIODevice::OpenMode openMode)
  : d_ptr(new VTextStreamPrivate(this))
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::VTextStream(QString *string == *%p, openMode = %d)", string, int(openMode));
#endif
    Q_D(VTextStream);
    d->string = string;
    d->stringOpenMode = openMode;
    d->status = Ok;
}

/*!
    Constructs a VTextStream that operates on \a array, using \a
    openMode to define the open mode. Internally, the array is wrapped
    by a QBuffer.
*/
VTextStream::VTextStream(QByteArray *array, QIODevice::OpenMode openMode)
  : d_ptr(new VTextStreamPrivate(this))
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::VTextStream(QByteArray *array == *%p, openMode = %d)", array, int(openMode));
#endif
    Q_D(VTextStream);
    d->device = new QBuffer(array);
    d->device->open(openMode);
    d->deleteDevice = true;
    d->deviceClosedNotifier.setupDevice(this, d->device);
    d->status = Ok;
}

/*!
    Constructs a VTextStream that operates on \a array, using \a
    openMode to define the open mode. The array is accessed as
    read-only, regardless of the values in \a openMode.

    This constructor is convenient for working on constant
    strings. Example:

    \snippet code/src_corelib_io_qtextstream.cpp 3
*/
VTextStream::VTextStream(const QByteArray &array, QIODevice::OpenMode openMode)
  : d_ptr(new VTextStreamPrivate(this))
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::VTextStream(const QByteArray &array == *(%p), openMode = %d)", &array, int(openMode));
#endif
    QBuffer *buffer = new QBuffer;
    buffer->setData(array);
    buffer->open(openMode);

    Q_D(VTextStream);
    d->device = buffer;
    d->deleteDevice = true;
    d->deviceClosedNotifier.setupDevice(this, d->device);
    d->status = Ok;
}

/*!
    Constructs a VTextStream that operates on \a fileHandle, using \a
    openMode to define the open mode. Internally, a QFile is created
    to handle the FILE pointer.

    This constructor is useful for working directly with the common
    FILE based input and output streams: stdin, stdout and stderr. Example:

    \snippet code/src_corelib_io_qtextstream.cpp 4
*/

VTextStream::VTextStream(FILE *fileHandle, QIODevice::OpenMode openMode)
  : d_ptr(new VTextStreamPrivate(this))
{
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::VTextStream(FILE *fileHandle = %p, openMode = %d)", fileHandle, int(openMode));
#endif
    auto *file = new QFile;
    // Discarding the return value of open; even if it failed
    // (and the file is not open), QTextStream still reports `Ok`
    // for closed QIODevices, so there's nothing really to do here.
    (void) file->open(fileHandle, openMode);

    Q_D(VTextStream);
    d->device = file;
    d->deleteDevice = true;
    d->deviceClosedNotifier.setupDevice(this, d->device);
    d->status = Ok;
}

/*!
    Destroys the VTextStream.

    If the stream operates on a device, flush() will be called
    implicitly. Otherwise, the device is unaffected.
*/
VTextStream::~VTextStream()
{
    Q_D(VTextStream);
#if defined(VTEXTSTREAM_DEBUG)
    qDebug("VTextStream::~VTextStream()");
#endif
    if (!d->writeBuffer.isEmpty())
        d->flushWriteBuffer();
}

/*!
    Resets VTextStream's formatting options, bringing it back to its
    original constructed state. The device, string and any buffered
    data is left untouched.
*/
void VTextStream::reset()
{
    Q_D(VTextStream);

    d->params.reset();
}

/*!
    Flushes any buffered data waiting to be written to the device.

    If VTextStream operates on a string, this function does nothing.
*/
void VTextStream::flush()
{
    Q_D(VTextStream);
    d->flushWriteBuffer();
}

/*!
    Seeks to the position \a pos in the device. Returns \c true on
    success; otherwise returns \c false.
*/
bool VTextStream::seek(qint64 pos)
{
    Q_D(VTextStream);
    d->lastTokenSize = 0;

    if (d->device) {
        // Empty the write buffer
        d->flushWriteBuffer();
        if (!d->device->seek(pos))
            return false;
        d->resetReadBuffer();

#if defined(WITH_TEXTCODEC)
        // Reset the codec converter states.
        resetCodecConverterStateHelper(&d->readConverterState);
        resetCodecConverterStateHelper(&d->writeConverterState);
        delete d->readConverterSavedState;
        d->readConverterSavedState = nullptr;
        d->writeConverterState.flags |= QTextCodec::IgnoreHeader;
#endif
        return true;
    }

    // string
    if (d->string && pos <= d->string->size()) {
        d->stringOffset = int(pos);
        return true;
    }
    return false;
}

/*!
    \since 4.2

    Returns the device position corresponding to the current position of the
    stream, or -1 if an error occurs (e.g., if there is no device or string,
    or if there's a device error).

    Because VTextStream is buffered, this function may have to
    seek the device to reconstruct a valid device position. This
    operation can be expensive, so you may want to avoid calling this
    function in a tight loop.

    \sa seek()
*/
qint64 VTextStream::pos() const
{
    Q_D(const VTextStream);
    if (d->device) {
        // Cutoff
        if (d->readBuffer.isEmpty())
            return d->device->pos();
        if (d->device->isSequential())
            return 0;

        // Seek the device
        if (!d->device->seek(d->readBufferStartDevicePos))
            return qint64(-1);

        // Reset the read buffer
        VTextStreamPrivate *thatd = const_cast<VTextStreamPrivate *>(d);
        thatd->readBuffer.clear();

#if defined(WITH_TEXTCODEC)
        thatd->restoreToSavedConverterState();
        if (d->readBufferStartDevicePos == 0)
            thatd->autoDetectUnicode = true;
#endif

        // Rewind the device to get to the current position Ensure that
        // readBufferOffset is unaffected by fillReadBuffer()
        qsizetype oldReadBufferOffset = d->readBufferOffset + d->readConverterSavedStateOffset;
        while (d->readBuffer.size() < oldReadBufferOffset) {
            if (!thatd->fillReadBuffer(1))
                return qint64(-1);
        }
        thatd->readBufferOffset = oldReadBufferOffset;
        thatd->readConverterSavedStateOffset = 0;

        // Return the device position.
        return d->device->pos();
    }

    if (d->string)
        return d->stringOffset;

    qWarning("VTextStream::pos: no device");
    return qint64(-1);
}

/*!
    Reads and discards whitespace from the stream until either a
    non-space character is detected, or until atEnd() returns
    true. This function is useful when reading a stream character by
    character.

    Whitespace characters are all characters for which
    QChar::isSpace() returns \c true.

    \sa operator>>()
*/
void VTextStream::skipWhiteSpace()
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(Q_VOID);
    d->scan(nullptr, nullptr, 0, VTextStreamPrivate::NotSpace);
    d->consumeLastToken();
}

/*!
    Sets the current device to \a device. If a device has already been
    assigned, VTextStream will call flush() before the old device is
    replaced.

    \note This function resets locale to the default locale ('C')
    and codec to the default codec, QTextCodec::codecForLocale().

    \sa device(), setString()
*/
void VTextStream::setDevice(QIODevice *device)
{
    Q_D(VTextStream);
    flush();
    if (d->deleteDevice) {
        d->deviceClosedNotifier.disconnect();
        delete d->device;
        d->deleteDevice = false;
    }

    d->reset();
    d->status = Ok;
    d->device = device;
    d->resetReadBuffer();
    d->deviceClosedNotifier.setupDevice(this, d->device);
}

/*!
    Returns the current device associated with the VTextStream,
    or \nullptr if no device has been assigned.

    \sa setDevice(), string()
*/
QIODevice *VTextStream::device() const
{
    Q_D(const VTextStream);
    return d->device;
}

/*!
    Sets the current string to \a string, using the given \a
    openMode. If a device has already been assigned, VTextStream will
    call flush() before replacing it.

    \sa string(), setDevice()
*/
void VTextStream::setString(QString *string, QIODevice::OpenMode openMode)
{
    Q_D(VTextStream);
    flush();
    if (d->deleteDevice) {
        d->deviceClosedNotifier.disconnect();
        d->device->blockSignals(true);
        delete d->device;
        d->deleteDevice = false;
    }

    d->reset();
    d->status = Ok;
    d->string = string;
    d->stringOpenMode = openMode;
}

/*!
    Returns the current string assigned to the VTextStream, or
    \nullptr if no string has been assigned.

    \sa setString(), device()
*/
QString *VTextStream::string() const
{
    Q_D(const VTextStream);
    return d->string;
}

/*!
    Sets the field alignment to \a mode. When used together with
    setFieldWidth(), this function allows you to generate formatted
    output with text aligned to the left, to the right or center
    aligned.

    \sa fieldAlignment(), setFieldWidth()
*/
void VTextStream::setFieldAlignment(FieldAlignment mode)
{
    Q_D(VTextStream);
    d->params.fieldAlignment = mode;
}

/*!
    Returns the current field alignment.

    \sa setFieldAlignment(), fieldWidth()
*/
VTextStream::FieldAlignment VTextStream::fieldAlignment() const
{
    Q_D(const VTextStream);
    return d->params.fieldAlignment;
}

/*!
    Sets the pad character to \a ch. The default value is the ASCII
    space character (' '), or QChar(0x20). This character is used to
    fill in the space in fields when generating text.

    Example:

    \snippet code/src_corelib_io_qtextstream.cpp 5

    The string \c s contains:

    \snippet code/src_corelib_io_qtextstream.cpp 6

    \sa padChar(), setFieldWidth()
*/
void VTextStream::setPadChar(QChar ch)
{
    Q_D(VTextStream);
    d->params.padChar = ch;
}

/*!
    Returns the current pad character.

    \sa setPadChar(), setFieldWidth()
*/
QChar VTextStream::padChar() const
{
    Q_D(const VTextStream);
    return d->params.padChar;
}

/*!
    Sets the current field width to \a width. If \a width is 0 (the
    default), the field width is equal to the length of the generated
    text.

    \note The field width applies to every element appended to this
    stream after this function has been called (e.g., it also pads
    endl). This behavior is different from similar classes in the STL,
    where the field width only applies to the next element.

    \sa fieldWidth(), setPadChar()
*/
void VTextStream::setFieldWidth(qsizetype width)
{
    Q_D(VTextStream);
    d->params.fieldWidth = width;
}

/*!
    Returns the current field width.

    \sa setFieldWidth()
*/
qsizetype VTextStream::fieldWidth() const
{
    Q_D(const VTextStream);
    return d->params.fieldWidth;
}

/*!
    Sets the current number flags to \a flags. \a flags is a set of
    flags from the NumberFlag enum, and describes options for
    formatting generated code (e.g., whether or not to always write
    the base or sign of a number).

    \sa numberFlags(), setIntegerBase(), setRealNumberNotation()
*/
void VTextStream::setNumberFlags(NumberFlags flags)
{
    Q_D(VTextStream);
    d->params.numberFlags = flags;
}

/*!
    Returns the current number flags.

    \sa setNumberFlags(), integerBase(), realNumberNotation()
*/
VTextStream::NumberFlags VTextStream::numberFlags() const
{
    Q_D(const VTextStream);
    return d->params.numberFlags;
}

/*!
    Sets the base of integers to \a base, both for reading and for
    generating numbers. \a base can be either 2 (binary), 8 (octal),
    10 (decimal) or 16 (hexadecimal). If \a base is 0, VTextStream
    will attempt to detect the base by inspecting the data on the
    stream. When generating numbers, VTextStream assumes base is 10
    unless the base has been set explicitly.

    \sa integerBase(), QString::number(), setNumberFlags()
*/
void VTextStream::setIntegerBase(int base)
{
    Q_D(VTextStream);
    d->params.integerBase = base;
}

/*!
    Returns the current base of integers. 0 means that the base is
    detected when reading, or 10 (decimal) when generating numbers.

    \sa setIntegerBase(), QString::number(), numberFlags()
*/
int VTextStream::integerBase() const
{
    Q_D(const VTextStream);
    return d->params.integerBase;
}

/*!
    Sets the real number notation to \a notation (SmartNotation,
    FixedNotation, ScientificNotation). When reading and generating
    numbers, VTextStream uses this value to detect the formatting of
    real numbers.

    \sa realNumberNotation(), setRealNumberPrecision(), setNumberFlags(), setIntegerBase()
*/
void VTextStream::setRealNumberNotation(RealNumberNotation notation)
{
    Q_D(VTextStream);
    d->params.realNumberNotation = notation;
}

/*!
    Returns the current real number notation.

    \sa setRealNumberNotation(), realNumberPrecision(), numberFlags(), integerBase()
*/
VTextStream::RealNumberNotation VTextStream::realNumberNotation() const
{
    Q_D(const VTextStream);
    return d->params.realNumberNotation;
}

/*!
    Sets the precision of real numbers to \a precision. This value
    describes the number of fraction digits VTextStream should
    write when generating real numbers.

    The precision cannot be a negative value. The default value is 6.

    \sa realNumberPrecision(), setRealNumberNotation()
*/
void VTextStream::setRealNumberPrecision(int precision)
{
    Q_D(VTextStream);
    if (precision < 0) {
        qWarning("VTextStream::setRealNumberPrecision: Invalid precision (%d)", precision);
        d->params.realNumberPrecision = 6;
        return;
    }
    d->params.realNumberPrecision = precision;
}

/*!
    Returns the current real number precision, or the number of fraction
    digits VTextStream will write when generating real numbers.

    \sa setRealNumberNotation(), realNumberNotation(), numberFlags(), integerBase()
*/
int VTextStream::realNumberPrecision() const
{
    Q_D(const VTextStream);
    return d->params.realNumberPrecision;
}

/*!
    Returns the status of the text stream.

    \sa VTextStream::Status, setStatus(), resetStatus()
*/

VTextStream::Status VTextStream::status() const
{
    Q_D(const VTextStream);
    return d->status;
}

/*!
    \since 4.1

    Resets the status of the text stream.

    \sa VTextStream::Status, status(), setStatus()
*/
void VTextStream::resetStatus()
{
    Q_D(VTextStream);
    d->status = Ok;
}

/*!
    \since 4.1

    Sets the status of the text stream to the \a status given.

    Subsequent calls to setStatus() are ignored until resetStatus()
    is called.

    \sa Status, status(), resetStatus()
*/
void VTextStream::setStatus(Status status)
{
    Q_D(VTextStream);
    if (d->status == Ok)
        d->status = status;
}

/*!
    Returns \c true if there is no more data to be read from the
    VTextStream; otherwise returns \c false. This is similar to, but not
    the same as calling QIODevice::atEnd(), as VTextStream also takes
    into account its internal Unicode buffer.
*/
bool VTextStream::atEnd() const
{
    Q_D(const VTextStream);
    CHECK_VALID_STREAM(true);

    if (d->string)
        return d->string->size() == d->stringOffset;
    return d->readBuffer.isEmpty() && d->device->atEnd();
}

/*!
    Reads the entire content of the stream, and returns it as a
    QString. Avoid this function when working on large files, as it
    will consume a significant amount of memory.

    Calling \l {VTextStream::readLine()}{readLine()} is better if you do not know how much data is
    available.

    \sa readLine()
*/
QString VTextStream::readAll()
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(QString());

    return d->read(INT_MAX);
}

/*!
    Reads one line of text from the stream, and returns it as a
    QString. The maximum allowed line length is set to \a maxlen. If
    the stream contains lines longer than this, then the lines will be
    split after \a maxlen characters and returned in parts.

    If \a maxlen is 0, the lines can be of any length.

    The returned line has no trailing end-of-line characters ("\\n"
    or "\\r\\n"), so calling QString::trimmed() can be unnecessary.

    If the stream has read to the end of the file, \l {VTextStream::readLine()}{readLine()}
    will return a null QString. For strings, or for devices that support it,
    you can explicitly test for the end of the stream using atEnd().

    \sa readAll(), QIODevice::readLine()
*/
QString VTextStream::readLine(qint64 maxlen)
{
    QString line;

    readLineInto(&line, maxlen);
    return line;
}

/*!
    \since 5.5

    Reads one line of text from the stream into \a line.
    If \a line is \nullptr, the read line is not stored.

    The maximum allowed line length is set to \a maxlen. If
    the stream contains lines longer than this, then the lines will be
    split after \a maxlen characters and returned in parts.

    If \a maxlen is 0, the lines can be of any length.

    The resulting line has no trailing end-of-line characters ("\\n"
    or "\\r\\n"), so calling QString::trimmed() can be unnecessary.

    If \a line has sufficient capacity for the data that is about to be
    read, this function may not need to allocate new memory. Because of
    this, it can be faster than readLine().

    Returns \c false if the stream has read to the end of the file or
    an error has occurred; otherwise returns \c true. The contents in
    \a line before the call are discarded in any case.

    \sa readAll(), QIODevice::readLine()
*/
bool VTextStream::readLineInto(QString *line, qint64 maxlen)
{
    Q_D(VTextStream);
    // keep in sync with CHECK_VALID_STREAM
    if (!d->string && !d->device) {
        qWarning("VTextStream: No device");
        if (line && !line->isNull())
            line->resize(0);
        return false;
    }

    const QChar *readPtr;
    int length;
    if (!d->scan(&readPtr, &length, int(maxlen), VTextStreamPrivate::EndOfLine))
    {
        if (line && !line->isNull())
            line->resize(0);
        return false;
    }

    if (Q_LIKELY(line))
        line->setUnicode(readPtr, length);
    d->consumeLastToken();
    return true;
}

/*!
    \since 4.1

    Reads at most \a maxlen characters from the stream, and returns the data
    read as a QString.

    \sa readAll(), readLine(), QIODevice::read()
*/
QString VTextStream::read(qint64 maxlen)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(QString());

    if (maxlen <= 0)
        return QString::fromLatin1("");     // empty, not null

    return d->read(int(maxlen));
}

/*!
    \internal
*/
VTextStreamPrivate::NumberParsingStatus VTextStreamPrivate::getNumber(qulonglong *ret)
{
    scan(nullptr, nullptr, 0, NotSpace);
    consumeLastToken();

    // detect int encoding
    int base = params.integerBase;
    if (base == 0) {
        QChar ch;
        if (!getChar(&ch))
            return npsInvalidPrefix;
        if (ch == QLatin1Char('0')) {
            QChar ch2;
            if (!getChar(&ch2)) {
                // Result is the number 0
                *ret = 0;
                return npsOk;
            }
            ch2 = ch2.toLower();

            if (ch2 == QLatin1Char('x')) {
                base = 16;
            } else if (ch2 == QLatin1Char('b')) {
                base = 2;
            } else if (ch2.isDigit() && ch2.digitValue() >= 0 && ch2.digitValue() <= 7) {
                base = 8;
            } else {
                base = 10;
            }
            ungetChar(ch2);
        } else if (ch == locale.negativeSign() || ch == locale.positiveSign() || ch.isDigit()) {
            base = 10;
        } else {
            ungetChar(ch);
            return npsInvalidPrefix;
        }
        ungetChar(ch);
        // State of the stream is now the same as on entry
        // (cursor is at prefix),
        // and local variable 'base' has been set appropriately.
    }

    qulonglong val=0;
    switch (base) {
    case 2: {
        QChar pf1, pf2, dig;
        // Parse prefix '0b'
        if (!getChar(&pf1) || pf1 != QLatin1Char('0'))
            return npsInvalidPrefix;
        if (!getChar(&pf2) || pf2.toLower() != QLatin1Char('b'))
            return npsInvalidPrefix;
        // Parse digits
        int ndigits = 0;
        while (getChar(&dig)) {
            char16_t n = dig.toLower().unicode();
            if (n == '0' || n == '1') {
                val <<= 1;
                val += n - '0';
            } else {
                ungetChar(dig);
                break;
            }
            ndigits++;
        }
        if (ndigits == 0) {
            // Unwind the prefix and abort
            ungetChar(pf2);
            ungetChar(pf1);
            return npsMissingDigit;
        }
        break;
    }
    case 8: {
        QChar pf, dig;
        // Parse prefix '0'
        if (!getChar(&pf) || pf != QLatin1Char('0'))
            return npsInvalidPrefix;
        // Parse digits
        int ndigits = 0;
        while (getChar(&dig)) {
            char16_t n = dig.toLower().unicode();
            if (n >= '0' && n <= '7') {
                val *= 8;
                val += n - '0';
            } else {
                ungetChar(dig);
                break;
            }
            ndigits++;
        }
        if (ndigits == 0) {
            // Unwind the prefix and abort
            ungetChar(pf);
            return npsMissingDigit;
        }
        break;
    }
    case 10: {
        // Parse sign (or first digit)
        QChar sign;
        int ndigits = 0;
        if (!getChar(&sign))
            return npsMissingDigit;
        if (sign != locale.negativeSign() && sign != locale.positiveSign()) {
            if (!sign.isDigit()) {
                ungetChar(sign);
                return npsMissingDigit;
            }
            val += static_cast<qulonglong>(sign.digitValue());
            ndigits++;
        }
        // Parse digits
        QChar ch;
        while (getChar(&ch)) {
            if (ch.isDigit()) {
                val *= 10;
                val += static_cast<qulonglong>(ch.digitValue());
            } else if (locale != QLocale::c() && ch == locale.groupSeparator()) {
                continue;
            } else {
                ungetChar(ch);
                break;
            }
            ndigits++;
        }
        if (ndigits == 0)
            return npsMissingDigit;
        if (sign == locale.negativeSign()) {
            qlonglong ival = qlonglong(val);
            if (ival > 0)
                ival = -ival;
            val = qulonglong(ival);
        }
        break;
    }
    case 16: {
        QChar pf1, pf2, dig;
        // Parse prefix ' 0x'
        if (!getChar(&pf1) || pf1 != QLatin1Char('0'))
            return npsInvalidPrefix;
        if (!getChar(&pf2) || pf2.toLower() != QLatin1Char('x'))
            return npsInvalidPrefix;
        // Parse digits
        int ndigits = 0;
        while (getChar(&dig)) {
            char16_t n = dig.toLower().unicode();
            if (n >= '0' && n <= '9') {
                val <<= 4;
                val += n - '0';
            } else if (n >= 'a' && n <= 'f') {
                val <<= 4;
                val += 10 + (n - 'a');
            } else {
                ungetChar(dig);
                break;
            }
            ndigits++;
        }
        if (ndigits == 0) {
            return npsMissingDigit;
        }
        break;
    }
    default:
        // Unsupported integerBase
        return npsInvalidPrefix;
    }

    if (ret)
        *ret = val;
    return npsOk;
}

/*!
    \internal
    (hihi)
*/
bool VTextStreamPrivate::getReal(double *f)
{
    // We use a table-driven FSM to parse floating point numbers
    // strtod() cannot be used directly since we may be reading from a
    // QIODevice.
    enum ParserState {
        Init = 0,
        Sign = 1,
        Mantissa = 2,
        Dot = 3,
        Abscissa = 4,
        ExpMark = 5,
        ExpSign = 6,
        Exponent = 7,
        Nan1 = 8,
        Nan2 = 9,
        Inf1 = 10,
        Inf2 = 11,
        NanInf = 12,
        Done = 13
    };
    enum InputToken {
        None = 0,
        InputSign = 1,
        InputDigit = 2,
        InputDot = 3,
        InputExp = 4,
        InputI = 5,
        InputN = 6,
        InputF = 7,
        InputA = 8,
        InputT = 9
    };

    static const uchar table[13][10] = {
        // None InputSign InputDigit InputDot InputExp InputI    InputN    InputF    InputA    InputT
        { 0,    Sign,     Mantissa,  Dot,     0,       Inf1,     Nan1,     0,        0,        0      }, // 0  Init
        { 0,    0,        Mantissa,  Dot,     0,       Inf1,     Nan1,     0,        0,        0      }, // 1  Sign
        { Done, Done,     Mantissa,  Dot,     ExpMark, 0,        0,        0,        0,        0      }, // 2  Mantissa
        { 0,    0,        Abscissa,  0,       0,       0,        0,        0,        0,        0      }, // 3  Dot
        { Done, Done,     Abscissa,  Done,    ExpMark, 0,        0,        0,        0,        0      }, // 4  Abscissa
        { 0,    ExpSign,  Exponent,  0,       0,       0,        0,        0,        0,        0      }, // 5  ExpMark
        { 0,    0,        Exponent,  0,       0,       0,        0,        0,        0,        0      }, // 6  ExpSign
        { Done, Done,     Exponent,  Done,    Done,    0,        0,        0,        0,        0      }, // 7  Exponent
        { 0,    0,        0,         0,       0,       0,        0,        0,        Nan2,     0      }, // 8  Nan1
        { 0,    0,        0,         0,       0,       0,        NanInf,   0,        0,        0      }, // 9  Nan2
        { 0,    0,        0,         0,       0,       0,        Inf2,     0,        0,        0      }, // 10 Inf1
        { 0,    0,        0,         0,       0,       0,        0,        NanInf,   0,        0      }, // 11 Inf2
        { Done, 0,        0,         0,       0,       0,        0,        0,        0,        0      }, // 11 NanInf
    };

    ParserState state = Init;
    InputToken input = None;

    scan(nullptr, nullptr, 0, NotSpace);
    consumeLastToken();

    const int BufferSize = 128;
    char buf[BufferSize];
    int i = 0;

    QChar c;
    while (getChar(&c)) {
        switch (c.unicode()) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            input = InputDigit;
            break;
        case 'i': case 'I':
            input = InputI;
            break;
        case 'n': case 'N':
            input = InputN;
            break;
        case 'f': case 'F':
            input = InputF;
            break;
        case 'a': case 'A':
            input = InputA;
            break;
        case 't': case 'T':
            input = InputT;
            break;
        default: {
            QChar lc = c.toLower();
            if (lc == locale.decimalPoint().toLower())
                input = InputDot;
            else if (lc == locale.exponential().toLower())
                input = InputExp;
            else if (lc == locale.negativeSign().toLower()
                     || lc == locale.positiveSign().toLower())
                input = InputSign;
            else if (locale != QLocale::c() // backward-compatibility
                     && lc == locale.groupSeparator().toLower())
                input = InputDigit; // well, it isn't a digit, but no one cares.
            else
                input = None;
        }
            break;
        }

        state = ParserState(table[state][input]);

        if  (state == Init || state == Done || i > (BufferSize - 5)) {
            ungetChar(c);
            if (i > (BufferSize - 5)) { // ignore rest of digits
                while (getChar(&c)) {
                    if (!c.isDigit()) {
                        ungetChar(c);
                        break;
                    }
                }
            }
            break;
        }

        buf[i++] = c.toLatin1();
    }

    if (i == 0)
        return false;
    if (!f)
        return true;
    buf[i] = '\0';

    // backward-compatibility. Old implementation supported +nan/-nan
    // for some reason. QLocale only checks for lower-case
    // nan/+inf/-inf, so here we also check for uppercase and mixed
    // case versions.
    if (!qstricmp(buf, "nan") || !qstricmp(buf, "+nan") || !qstricmp(buf, "-nan")) {
        *f = qQNaN();
        return true;
    } else if (!qstricmp(buf, "+inf") || !qstricmp(buf, "inf")) {
        *f = qInf();
        return true;
    } else if (!qstricmp(buf, "-inf")) {
        *f = -qInf();
        return true;
    }
    bool ok;
    *f = locale.toDouble(QString::fromLatin1(buf), &ok);
    return ok;
}

/*!
    Reads a character from the stream and stores it in \a c. Returns a
    reference to the VTextStream, so several operators can be
    nested. Example:

    \snippet code/src_corelib_io_qtextstream.cpp 7

    Whitespace is \e not skipped.
*/

VTextStream &VTextStream::operator>>(QChar &c)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->scan(nullptr, nullptr, 0, VTextStreamPrivate::NotSpace);
    if (!d->getChar(&c))
        setStatus(ReadPastEnd);
    return *this;
}

/*!
    \overload

    Reads a character from the stream and stores it in \a c. The
    character from the stream is converted to ISO-8859-1 before it is
    stored.

    \sa QChar::toLatin1()
*/
VTextStream &VTextStream::operator>>(char &c)
{
    QChar ch;
    *this >> ch;
    c = ch.toLatin1();
    return *this;
}

/*!
    Reads an integer from the stream and stores it in \a i, then
    returns a reference to the VTextStream. The number is cast to
    the correct type before it is stored. If no number was detected on
    the stream, \a i is set to 0.

    By default, VTextStream will attempt to detect the base of the
    number using the following rules:

    \table
    \header \li Prefix                \li Base
    \row    \li "0b" or "0B"          \li 2 (binary)
    \row    \li "0" followed by "0-7" \li 8 (octal)
    \row    \li "0" otherwise         \li 10 (decimal)
    \row    \li "0x" or "0X"          \li 16 (hexadecimal)
    \row    \li "1" to "9"            \li 10 (decimal)
    \endtable

    By calling setIntegerBase(), you can specify the integer base
    explicitly. This will disable the auto-detection, and speed up
    VTextStream slightly.

    Leading whitespace is skipped.
*/
VTextStream &VTextStream::operator>>(signed short &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed short);
}

/*!
    \overload

    Stores the integer in the unsigned short \a i.
*/
VTextStream &VTextStream::operator>>(unsigned short &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned short);
}

/*!
    \overload

    Stores the integer in the signed int \a i.
*/
VTextStream &VTextStream::operator>>(signed int &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed int);
}

/*!
    \overload

    Stores the integer in the unsigned int \a i.
*/
VTextStream &VTextStream::operator>>(unsigned int &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned int);
}

/*!
    \overload

    Stores the integer in the signed long \a i.
*/
VTextStream &VTextStream::operator>>(signed long &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(signed long);
}

/*!
    \overload

    Stores the integer in the unsigned long \a i.
*/
VTextStream &VTextStream::operator>>(unsigned long &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(unsigned long);
}

/*!
    \overload

    Stores the integer in the qlonglong \a i.
*/
VTextStream &VTextStream::operator>>(qlonglong &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(qlonglong);
}

/*!
    \overload

    Stores the integer in the qulonglong \a i.
*/
VTextStream &VTextStream::operator>>(qulonglong &i)
{
    IMPLEMENT_STREAM_RIGHT_INT_OPERATOR(qulonglong);
}

/*!
    Reads a real number from the stream and stores it in \a f, then
    returns a reference to the VTextStream. The number is cast to
    the correct type. If no real number is detect on the stream, \a f
    is set to 0.0.

    As a special exception, VTextStream allows the strings "nan" and "inf" to
    represent NAN and INF floats or doubles.

    Leading whitespace is skipped.
*/
VTextStream &VTextStream::operator>>(float &f)
{
    IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(float);
}

/*!
    \overload

    Stores the real number in the double \a f.
*/
VTextStream &VTextStream::operator>>(double &f)
{
    IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR(double);
}

/*!
    Reads a word from the stream and stores it in \a str, then returns
    a reference to the stream. Words are separated by whitespace
    (i.e., all characters for which QChar::isSpace() returns \c true).

    Leading whitespace is skipped.
*/
VTextStream &VTextStream::operator>>(QString &str)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);

    str.clear();
    d->scan(nullptr, nullptr, 0, VTextStreamPrivate::NotSpace);
    d->consumeLastToken();

    const QChar *ptr;
    int length;
    if (!d->scan(&ptr, &length, 0, VTextStreamPrivate::Space))
    {
        setStatus(ReadPastEnd);
        return *this;
    }

    str = QString(ptr, length);
    d->consumeLastToken();
    return *this;
}

/*!
    \overload

    Converts the word to ISO-8859-1, then stores it in \a array.

    \sa QString::toLatin1()
*/
VTextStream &VTextStream::operator>>(QByteArray &array)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);

    array.clear();
    d->scan(nullptr, nullptr, 0, VTextStreamPrivate::NotSpace);
    d->consumeLastToken();

    const QChar *ptr;
    int length;
    if (!d->scan(&ptr, &length, 0, VTextStreamPrivate::Space))
    {
        setStatus(ReadPastEnd);
        return *this;
    }

    for (int i = 0; i < length; ++i)
        array += ptr[i].toLatin1();

    d->consumeLastToken();
    return *this;
}

/*!
    \overload

    Stores the word in \a c, terminated by a '\\0' character. If no word is
    available, only the '\\0' character is stored.

    Warning: Although convenient, this operator is dangerous and must
    be used with care. VTextStream assumes that \a c points to a
    buffer with enough space to hold the word. If the buffer is too
    small, your application may crash.

    If possible, use the QByteArray operator instead.
*/
VTextStream &VTextStream::operator>>(char *c)
{
    Q_D(VTextStream);
    *c = 0;
    CHECK_VALID_STREAM(*this);
    d->scan(nullptr, nullptr, 0, VTextStreamPrivate::NotSpace);
    d->consumeLastToken();

    const QChar *ptr;
    int length;
    if (!d->scan(&ptr, &length, 0, VTextStreamPrivate::Space))
    {
        setStatus(ReadPastEnd);
        return *this;
    }

    for (int i = 0; i < length; ++i)
        *c++ = ptr[i].toLatin1();
    *c = '\0';
    d->consumeLastToken();
    return *this;
}

/*!
    \internal
 */
void VTextStreamPrivate::putNumber(qulonglong number, bool negative)
{
    QString result;

    const VTextStream::NumberFlags numberFlags = params.numberFlags;
    int const base = (params.integerBase != 0) ? params.integerBase : 10;

    // Step 1: Convert number to string
    result = base == 10 ? locale.toString(number) : QString::number(number, base);
    if (locale.numberOptions().testFlag(QLocale::OmitGroupSeparator) && !locale.groupSeparator().isEmpty())
    {
        result.replace(locale.groupSeparator(), QString());
    }

    // Step 2: Uppercase digits if requested
    if (numberFlags & VTextStream::UppercaseDigits)
    {
        result = result.toUpper();
    }

    // Step 3: Add base prefixes if requested
    if (numberFlags & VTextStream::ShowBase && base != 10)
    {
        QString prefix;
        switch (base)
        {
            case 16:
                prefix = (numberFlags & VTextStream::UppercaseBase) ? "0X"_L1 : "0x"_L1;
                break;
            case 8:
                prefix = QLatin1Char('0');
                break;
            case 2:
                prefix = (numberFlags & VTextStream::UppercaseBase) ? "0B"_L1 : "0b"_L1;
                break;
            default:
                break; // no prefix for unusual bases
        }
        result.prepend(prefix);

        // Special case: octal zero with ShowBase → "00"
        if (number == 0 && base == 8 && result == QLatin1Char('0'))
        {
            result.prepend(QLatin1Char('0'));
        }
    }

    // Step 4: Add sign for positive numbers if ForceSign is set
    if (negative)
    {
        // Handle negative numbers in non-decimal bases: prepend locale-specific sign
        result.prepend(locale.negativeSign());
    }
    else if (numberFlags & VTextStream::ForceSign)
    {
        result.prepend(locale.positiveSign().isEmpty() ? "+"_L1 : locale.positiveSign());
    }

    putString(result, true);
}

/*!
    Writes the character \a c to the stream, then returns a reference
    to the VTextStream.

    \sa setFieldWidth()
*/
VTextStream &VTextStream::operator<<(QChar c)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putChar(c);
    return *this;
}

/*!
    \overload

    Converts \a c from ASCII to a QChar, then writes it to the stream.
*/
VTextStream &VTextStream::operator<<(char c)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putChar(QChar::fromLatin1(c));
    return *this;
}

/*!
    Writes the integer number \a i to the stream, then returns a
    reference to the VTextStream. By default, the number is stored in
    decimal form, but you can also set the base by calling
    setIntegerBase().

    \sa setFieldWidth(), setNumberFlags()
*/
VTextStream &VTextStream::operator<<(signed short i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(qAbs(static_cast<qlonglong>(i))), i < 0);
    return *this;
}

/*!
    \overload

    Writes the unsigned short \a i to the stream.
*/
VTextStream &VTextStream::operator<<(unsigned short i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(i), false);
    return *this;
}

/*!
    \overload

    Writes the signed int \a i to the stream.
*/
VTextStream &VTextStream::operator<<(signed int i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(qAbs(qlonglong(i))), i < 0);
    return *this;
}

/*!
    \overload

    Writes the unsigned int \a i to the stream.
*/
VTextStream &VTextStream::operator<<(unsigned int i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(i), false);
    return *this;
}

/*!
    \overload

    Writes the signed long \a i to the stream.
*/
VTextStream &VTextStream::operator<<(signed long i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(qAbs(qlonglong(i))), i < 0);
    return *this;
}

/*!
    \overload

    Writes the unsigned long \a i to the stream.
*/
VTextStream &VTextStream::operator<<(unsigned long i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(i), false);
    return *this;
}

/*!
    \overload

    Writes the qlonglong \a i to the stream.
*/
VTextStream &VTextStream::operator<<(qlonglong i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(static_cast<qulonglong>(qAbs(i)), i < 0);
    return *this;
}

/*!
    \overload

    Writes the qulonglong \a i to the stream.
*/
VTextStream &VTextStream::operator<<(qulonglong i)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putNumber(i, false);
    return *this;
}

/*!
    Writes the real number \a f to the stream, then returns a
    reference to the VTextStream. By default, VTextStream stores it
    using SmartNotation, with up to 6 digits of precision. You can
    change the textual representation VTextStream will use for real
    numbers by calling setRealNumberNotation(),
    setRealNumberPrecision() and setNumberFlags().

    \sa setFieldWidth(), setRealNumberNotation(),
    setRealNumberPrecision(), setNumberFlags()
*/
VTextStream &VTextStream::operator<<(float f)
{
    return *this << double(f);
}

/*!
    \overload

    Writes the double \a f to the stream.
*/
VTextStream &VTextStream::operator<<(double f)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    char formatChar = 'g';
    switch (realNumberNotation())
    {
        case FixedNotation:
            formatChar = 'f';
            break;
        case ScientificNotation:
            formatChar = 'e';
            break;
        case SmartNotation:
            formatChar = 'g';
            break;
    }

    QT_WARNING_POP

    QLocale loc = locale();

    // Apply number options (public API only)
    QLocale::NumberOptions opts = loc.numberOptions();
    if (numberFlags() & ForcePoint)
    {
        opts |= QLocale::IncludeTrailingZeroesAfterDot;
    }
    loc.setNumberOptions(opts);

    QString num = loc.toString(f, formatChar, d->params.realNumberPrecision);

    // Post-process if needed (sign handling, uppercase E, etc.)
    if (numberFlags() & ForceSign && !num.startsWith(loc.positiveSign()) && !num.startsWith(loc.negativeSign()))
    {
        num.prepend(loc.positiveSign());
    }
    if (numberFlags() & UppercaseDigits)
    {
        // Uppercase exponent marker
        num.replace('e', 'E');
        // Uppercase special values like "inf" and "nan"
        if (num.compare("inf"_L1, Qt::CaseInsensitive) == 0 || num.compare("+inf"_L1, Qt::CaseInsensitive) == 0
            || num.compare("-inf"_L1, Qt::CaseInsensitive) == 0 || num.compare("nan"_L1, Qt::CaseInsensitive) == 0)
        {
            num = num.toUpper();
        }
    }

    d->putString(num, true);
    return *this;
}

/*!
    Writes the string \a string to the stream, and returns a reference
    to the VTextStream. The string is first encoded using the assigned
    codec (the default codec is QTextCodec::codecForLocale()) before
    it is written to the stream.

    \sa setFieldWidth(), setCodec()
*/
VTextStream &VTextStream::operator<<(const QString &string)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(string);
    return *this;
}

/*!
    \overload

    Writes \a string to the stream, and returns a reference to the
    VTextStream.
    \since 5.12
*/
VTextStream &VTextStream::operator<<(QStringView string)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(string.cbegin(), int(string.size()));
    return *this;
}

/*!
    \overload

    Writes \a string to the stream, and returns a reference to the
    VTextStream.
*/
VTextStream &VTextStream::operator<<(QLatin1String string)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(string);
    return *this;
}

/*!
    \overload

    Writes \a array to the stream. The contents of \a array are
    converted with QString::fromUtf8().
*/
VTextStream &VTextStream::operator<<(const QByteArray &array)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    d->putString(QString::fromUtf8(array.constData(), array.length()));
    return *this;
}

/*!
    \overload

    Writes the constant string pointed to by \a string to the stream. \a
    string is assumed to be in ISO-8859-1 encoding. This operator
    is convenient when working with constant string data. Example:

    \snippet code/src_corelib_io_qtextstream.cpp 8

    Warning: VTextStream assumes that \a string points to a string of
    text, terminated by a '\\0' character. If there is no terminating
    '\\0' character, your application may crash.
*/
VTextStream &VTextStream::operator<<(const char *string)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    // ### Qt6: consider changing to UTF-8
    d->putString(QLatin1String(string));
    return *this;
}

/*!
    \overload

    Writes \a ptr to the stream as a hexadecimal number with a base.
*/

VTextStream &VTextStream::operator<<(const void *ptr)
{
    Q_D(VTextStream);
    CHECK_VALID_STREAM(*this);
    const int oldBase = d->params.integerBase;
    const NumberFlags oldFlags = d->params.numberFlags;
    d->params.integerBase = 16;
    d->params.numberFlags |= ShowBase;
    d->putNumber(reinterpret_cast<quintptr>(ptr), false);
    d->params.integerBase = oldBase;
    d->params.numberFlags = oldFlags;
    return *this;
}

/*!
    \fn VTextStream &bin(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::bin() instead.
*/

/*!
    \fn VTextStream &oct(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::oct() instead.
*/

/*!
    \fn VTextStream &dec(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::dec() instead.
*/

/*!
    \fn VTextStream &hex(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::hex() instead.
*/

/*!
    \fn VTextStream &showbase(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::showbase() instead.
*/

/*!
    \fn VTextStream &forcesign(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::forcesign() instead.
*/

/*!
    \fn VTextStream &forcepoint(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::forcepoint() instead.
*/

/*!
    \fn VTextStream &noshowbase(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::noshowbase() instead.
*/

/*!
    \fn VTextStream &noforcesign(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::noforcesign() instead.
*/

/*!
    \fn VTextStream &noforcepoint(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::noforcepoint() instead.
*/

/*!
    \fn VTextStream &uppercasebase(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::uppercasebase() instead.
*/

/*!
    \fn VTextStream &uppercasedigits(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::uppercasedigits() instead.
*/

/*!
    \fn VTextStream &lowercasebase(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::lowercasebase() instead.
*/

/*!
    \fn VTextStream &lowercasedigits(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::lowercasedigits() instead.
*/

/*!
    \fn VTextStream &fixed(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::fixed() instead.
*/

/*!
    \fn VTextStream &scientific(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::scientific() instead.
*/

/*!
    \fn VTextStream &left(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::left() instead.
*/

/*!
    \fn VTextStream &right(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::right() instead.
*/

/*!
    \fn VTextStream &center(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::center() instead.
*/

/*!
    \fn VTextStream &endl(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::endl() instead.
*/

/*!
    \fn VTextStream &flush(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::flush() instead.
*/

/*!
    \fn VTextStream &reset(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::reset() instead.
*/

/*!
    \fn VTextStream &ws(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::ws() instead.
*/

namespace Val
{

/*!
    Calls VTextStream::setIntegerBase(2) on \a stream and returns \a
    stream.

    \since 5.14

    \sa oct(), dec(), hex(), {VTextStream manipulators}
*/
VTextStream &bin(VTextStream &stream)
{
    stream.setIntegerBase(2);
    return stream;
}

/*!
    Calls VTextStream::setIntegerBase(8) on \a stream and returns \a
    stream.

    \since 5.14

    \sa bin(), dec(), hex(), {VTextStream manipulators}
*/
VTextStream &oct(VTextStream &stream)
{
    stream.setIntegerBase(8);
    return stream;
}

/*!
    Calls VTextStream::setIntegerBase(10) on \a stream and returns \a
    stream.

    \since 5.14

    \sa bin(), oct(), hex(), {VTextStream manipulators}
*/
VTextStream &dec(VTextStream &stream)
{
    stream.setIntegerBase(10);
    return stream;
}

/*!
    Calls VTextStream::setIntegerBase(16) on \a stream and returns \a
    stream.

    \since 5.14

    \note The hex modifier can only be used for writing to streams.
    \sa bin(), oct(), dec(), {VTextStream manipulators}
*/
VTextStream &hex(VTextStream &stream)
{
    stream.setIntegerBase(16);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() |
    VTextStream::ShowBase) on \a stream and returns \a stream.

    \since 5.14

    \sa noshowbase(), forcesign(), forcepoint(), {VTextStream manipulators}
*/
VTextStream &showbase(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() | VTextStream::ShowBase);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() |
    VTextStream::ForceSign) on \a stream and returns \a stream.

    \since 5.14

    \sa noforcesign(), forcepoint(), showbase(), {VTextStream manipulators}
*/
VTextStream &forcesign(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() | VTextStream::ForceSign);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() |
    VTextStream::ForcePoint) on \a stream and returns \a stream.

    \since 5.14

    \sa noforcepoint(), forcesign(), showbase(), {VTextStream manipulators}
*/
VTextStream &forcepoint(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() | VTextStream::ForcePoint);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() &
    ~VTextStream::ShowBase) on \a stream and returns \a stream.

    \since 5.14

    \sa showbase(), noforcesign(), noforcepoint(), {VTextStream manipulators}
*/
VTextStream &noshowbase(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() &= ~VTextStream::ShowBase);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() &
    ~VTextStream::ForceSign) on \a stream and returns \a stream.

    \since 5.14

    \sa forcesign(), noforcepoint(), noshowbase(), {VTextStream manipulators}
*/
VTextStream &noforcesign(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() &= ~VTextStream::ForceSign);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() &
    ~VTextStream::ForcePoint) on \a stream and returns \a stream.

    \since 5.14

    \sa forcepoint(), noforcesign(), noshowbase(), {VTextStream manipulators}
*/
VTextStream &noforcepoint(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() &= ~VTextStream::ForcePoint);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() |
    VTextStream::UppercaseBase) on \a stream and returns \a stream.

    \since 5.14

    \sa lowercasebase(), uppercasedigits(), {VTextStream manipulators}
*/
VTextStream &uppercasebase(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() | VTextStream::UppercaseBase);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() |
    VTextStream::UppercaseDigits) on \a stream and returns \a stream.

    \since 5.14

    \sa lowercasedigits(), uppercasebase(), {VTextStream manipulators}
*/
VTextStream &uppercasedigits(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() | VTextStream::UppercaseDigits);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() &
    ~VTextStream::UppercaseBase) on \a stream and returns \a stream.

    \since 5.14

    \sa uppercasebase(), lowercasedigits(), {VTextStream manipulators}
*/
VTextStream &lowercasebase(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() & ~VTextStream::UppercaseBase);
    return stream;
}

/*!
    Calls VTextStream::setNumberFlags(VTextStream::numberFlags() &
    ~VTextStream::UppercaseDigits) on \a stream and returns \a stream.

    \since 5.14

    \sa uppercasedigits(), lowercasebase(), {VTextStream manipulators}
*/
VTextStream &lowercasedigits(VTextStream &stream)
{
    stream.setNumberFlags(stream.numberFlags() & ~VTextStream::UppercaseDigits);
    return stream;
}

/*!
    Calls VTextStream::setRealNumberNotation(VTextStream::FixedNotation)
    on \a stream and returns \a stream.

    \since 5.14

    \sa scientific(), {VTextStream manipulators}
*/
VTextStream &fixed(VTextStream &stream)
{
    stream.setRealNumberNotation(VTextStream::FixedNotation);
    return stream;
}

/*!
    Calls VTextStream::setRealNumberNotation(VTextStream::ScientificNotation)
    on \a stream and returns \a stream.

    \since 5.14

    \sa fixed(), {VTextStream manipulators}
*/
VTextStream &scientific(VTextStream &stream)
{
    stream.setRealNumberNotation(VTextStream::ScientificNotation);
    return stream;
}

/*!
    Calls VTextStream::setFieldAlignment(VTextStream::AlignLeft)
    on \a stream and returns \a stream.

    \since 5.14

    \sa right(), center(), {VTextStream manipulators}
*/
VTextStream &left(VTextStream &stream)
{
    stream.setFieldAlignment(VTextStream::AlignLeft);
    return stream;
}

/*!
    Calls VTextStream::setFieldAlignment(VTextStream::AlignRight)
    on \a stream and returns \a stream.

    \since 5.14

    \sa left(), center(), {VTextStream manipulators}
*/
VTextStream &right(VTextStream &stream)
{
    stream.setFieldAlignment(VTextStream::AlignRight);
    return stream;
}

/*!
    Calls VTextStream::setFieldAlignment(VTextStream::AlignCenter)
    on \a stream and returns \a stream.

    \since 5.14

    \sa left(), right(), {VTextStream manipulators}
*/
VTextStream &center(VTextStream &stream)
{
    stream.setFieldAlignment(VTextStream::AlignCenter);
    return stream;
}

/*!
    Writes '\\n' to the \a stream and flushes the stream.

    Equivalent to

    \snippet code/src_corelib_io_qtextstream.cpp 9

    Note: On Windows, all '\\n' characters are written as '\\r\\n' if
    VTextStream's device or string is opened using the QIODevice::Text flag.

    \since 5.14

    \sa flush(), reset(), {VTextStream manipulators}
*/
VTextStream &endl(VTextStream &stream)
{
    return stream << QLatin1Char('\n') << Val::flush;
}

/*!
    Calls VTextStream::flush() on \a stream and returns \a stream.

    \since 5.14

    \sa endl(), reset(), {VTextStream manipulators}
*/
VTextStream &flush(VTextStream &stream)
{
    stream.flush();
    return stream;
}

/*!
    Calls VTextStream::reset() on \a stream and returns \a stream.

    \since 5.14

    \sa flush(), {VTextStream manipulators}
*/
VTextStream &reset(VTextStream &stream)
{
    stream.reset();
    return stream;
}

/*!
    Calls \l {VTextStream::}{skipWhiteSpace()} on \a stream and returns \a stream.

    \since 5.14

    \sa {VTextStream manipulators}
*/
VTextStream &ws(VTextStream &stream)
{
    stream.skipWhiteSpace();
    return stream;
}

} // namespace Val

/*!
    \fn VTextStreamManipulator qSetFieldWidth(int width)
    \relates VTextStream

    Equivalent to VTextStream::setFieldWidth(\a width).
*/

/*!
    \fn VTextStreamManipulator qSetPadChar(QChar ch)
    \relates VTextStream

    Equivalent to VTextStream::setPadChar(\a ch).
*/

/*!
    \fn VTextStreamManipulator qSetRealNumberPrecision(int precision)
    \relates VTextStream

    Equivalent to VTextStream::setRealNumberPrecision(\a precision).
*/

#if defined(WITH_TEXTCODEC)
/*!
    \fn VTextStream &bom(VTextStream &stream)
    \relates VTextStream
    \obsolete Use Val::bom() instead.
*/

namespace Val
{
/*!
    Toggles insertion of the Byte Order Mark on \a stream when VTextStream is
    used with a UTF codec.

    \since 5.14

    \sa VTextStream::setGenerateByteOrderMark(), {VTextStream manipulators}
*/
VTextStream &bom(VTextStream &stream)
{
    stream.setGenerateByteOrderMark(true);
    return stream;
}

} // namespace Val

/*!
    Sets the codec for this stream to \a codec. The codec is used for
    decoding any data that is read from the assigned device, and for
    encoding any data that is written. By default,
    QTextCodec::codecForLocale() is used, and automatic unicode
    detection is enabled.

    If VTextStream operates on a string, this function does nothing.

    \warning If you call this function while the text stream is reading
    from an open sequential socket, the internal buffer may still contain
    text decoded using the old codec.

    \sa codec(), setAutoDetectUnicode(), setLocale()
*/
void VTextStream::setCodec(QTextCodec *codec)
{
    Q_D(VTextStream);
    qint64 seekPos = -1;
    if (!d->readBuffer.isEmpty()) {
        if (!d->device->isSequential()) {
            seekPos = pos();
        }
    }
    d->codec = codec;
    if (seekPos >=0 && !d->readBuffer.isEmpty())
        seek(seekPos);
}

/*!
    Sets the codec for this stream to the QTextCodec for the encoding
    specified by \a codecName. Common values for \c codecName include
    "ISO 8859-1", "UTF-8", and "UTF-16". If the encoding isn't
    recognized, nothing happens.

    Example:

    \snippet code/src_corelib_io_qtextstream.cpp 10

    \sa QTextCodec::codecForName(), setLocale()
*/
void VTextStream::setCodec(const char *codecName)
{
    QTextCodec *codec = QTextCodec::codecForName(codecName);
    if (codec)
        setCodec(codec);
}

/*!
    Returns the codec that is current assigned to the stream.

    \sa setCodec(), setAutoDetectUnicode(), locale()
*/
QTextCodec *VTextStream::codec() const
{
    Q_D(const VTextStream);
    return d->codec;
}

/*!
    If \a enabled is true, VTextStream will attempt to detect Unicode encoding
    by peeking into the stream data to see if it can find the UTF-8, UTF-16, or
    UTF-32 Byte Order Mark (BOM). If this mark is found, VTextStream will
    replace the current codec with the UTF codec.

    This function can be used together with setCodec(). It is common
    to set the codec to UTF-8, and then enable UTF-16 detection.

    \sa autoDetectUnicode(), setCodec(), QTextCodec::codecForUtfText()
*/
void VTextStream::setAutoDetectUnicode(bool enabled)
{
    Q_D(VTextStream);
    d->autoDetectUnicode = enabled;
}

/*!
    Returns \c true if automatic Unicode detection is enabled, otherwise
    returns \c false. Automatic Unicode detection is enabled by default.

    \sa setAutoDetectUnicode(), setCodec(), QTextCodec::codecForUtfText()
*/
bool VTextStream::autoDetectUnicode() const
{
    Q_D(const VTextStream);
    return d->autoDetectUnicode;
}

/*!
    If \a generate is true and a UTF codec is used, VTextStream will insert
    the BOM (Byte Order Mark) before any data has been written to the
    device. If \a generate is false, no BOM will be inserted. This function
    must be called before any data is written. Otherwise, it does nothing.

    \sa generateByteOrderMark(), {Val::}{bom()}
*/
void VTextStream::setGenerateByteOrderMark(bool generate)
{
    Q_D(VTextStream);
    if (d->writeBuffer.isEmpty()) {
        d->writeConverterState.flags.setFlag(QTextCodec::IgnoreHeader, !generate);
    }
}

/*!
    Returns \c true if VTextStream is set to generate the UTF BOM (Byte Order
    Mark) when using a UTF codec; otherwise returns \c false. UTF BOM generation is
    set to false by default.

    \sa setGenerateByteOrderMark()
*/
bool VTextStream::generateByteOrderMark() const
{
    Q_D(const VTextStream);
    return (d->writeConverterState.flags & QTextCodec::IgnoreHeader) == 0;
}

#endif

/*!
    \since 4.5

    Sets the locale for this stream to \a locale. The specified locale is
    used for conversions between numbers and their string representations.

    The default locale is C and it is a special case - the thousands
    group separator is not used for backward compatibility reasons.

    \sa locale()
*/
void VTextStream::setLocale(const QLocale &locale)
{
    Q_D(VTextStream);
    d->locale = locale;
}

/*!
    \since 4.5

    Returns the locale for this stream. The default locale is C.

    \sa setLocale()
*/
QLocale VTextStream::locale() const
{
    Q_D(const VTextStream);
    return d->locale;
}
