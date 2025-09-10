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

#ifndef VTEXTSTREAM_P_H
#define VTEXTSTREAM_P_H

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

#include "vtextstream.h"
#include <qtypes.h>
#if defined(WITH_TEXTCODEC)
#include "codecs/qtextcodec.h"
#endif

#include <QIODevice>
#include <QLocale>
#include <QObject>

#ifndef QT_NO_QOBJECT
class QDeviceClosedNotifier : public QObject
{
    Q_OBJECT
public:
    inline QDeviceClosedNotifier()
    { }

    inline void setupDevice(VTextStream *stream, QIODevice *device)
    {
        disconnect();
        if (device)
            connect(device, &QIODevice::aboutToClose, this, &QDeviceClosedNotifier::flushStream);
        this->stream = stream;
    }

public Q_SLOTS:
    inline void flushStream() { stream->flush(); }

private:
    VTextStream *stream;
};
#endif

class VTextStreamPrivate
{
    Q_DECLARE_PUBLIC(VTextStream)

public:
    // streaming parameters
    class Params
    {
    public:
        void reset();

        int realNumberPrecision;
        int integerBase;
        qsizetype fieldWidth;
        QChar padChar;
        VTextStream::FieldAlignment fieldAlignment;
        VTextStream::RealNumberNotation realNumberNotation;
        VTextStream::NumberFlags numberFlags;
    };

    VTextStreamPrivate(VTextStream *q_ptr);
    ~VTextStreamPrivate();
    void reset();

    // device
    QIODevice *device;
#ifndef QT_NO_QOBJECT
    QDeviceClosedNotifier deviceClosedNotifier;
#endif

    // string
    QString *string;
    qsizetype stringOffset;
    QIODevice::OpenMode stringOpenMode;

#if defined(WITH_TEXTCODEC)
    // codec
    QTextCodec *codec;
    QTextCodec::ConverterState readConverterState;
    QTextCodec::ConverterState writeConverterState;
    QTextCodec::ConverterState *readConverterSavedState;
#endif

    QString writeBuffer;
    QString readBuffer;
    qsizetype readBufferOffset;
    qsizetype readConverterSavedStateOffset; //the offset between readBufferStartDevicePos and that start of the buffer
    qint64 readBufferStartDevicePos;

    Params params;

    // status
    VTextStream::Status status;
    QLocale locale;
    VTextStream *q_ptr;

    qsizetype lastTokenSize;
    bool deleteDevice;
#if defined(WITH_TEXTCODEC)
    bool autoDetectUnicode;
#endif

    // i/o
    enum TokenDelimiter {
        Space,
        NotSpace,
        EndOfLine
    };

    QString read(qsizetype maxlen);
    bool scan(const QChar **ptr, int *tokenLength,
              int maxlen, TokenDelimiter delimiter);
    inline const QChar *readPtr() const;
    inline void consumeLastToken();
    inline void consume(qsizetype nchars);
    void saveConverterState(qint64 newPos);
    void restoreToSavedConverterState();

    // Return value type for getNumber()
    enum NumberParsingStatus {
        npsOk,
        npsMissingDigit,
        npsInvalidPrefix
    };

    inline bool getChar(QChar *ch);
    inline void ungetChar(QChar ch);
    NumberParsingStatus getNumber(qulonglong *l);
    bool getReal(double *f);

    inline void write(const QString &data) { write(data.begin(), data.length()); }
    inline void write(QChar ch);
    void write(const QChar *data, qsizetype len);
    void write(QLatin1String data);
    void writePadding(qsizetype len);
    inline void putString(const QString &ch, bool number = false) { putString(ch.constData(), ch.length(), number); }
    void putString(const QChar *data, qsizetype len, bool number = false);
    void putString(QLatin1String data, bool number = false);
    inline void putChar(QChar ch);
    void putNumber(qulonglong number, bool negative);

    struct PaddingResult {
        qsizetype left, right;
    };
    PaddingResult padding(qsizetype len) const;

    // buffers
    bool fillReadBuffer(qint64 maxBytes = -1);
    void resetReadBuffer();
    void flushWriteBuffer();
};

#endif // VTEXTSTREAM_P_H
