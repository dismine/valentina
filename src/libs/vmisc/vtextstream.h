/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef VTEXTSTREAM_H
#define VTEXTSTREAM_H

#include <QtCore/qiodevice.h>
#include <QtCore/qstring.h>
#include <QtCore/qchar.h>
#include <QtCore/qscopedpointer.h>

#include <stdio.h>


class QTextCodec;
class QTextDecoder;

class VTextStreamPrivate;
class VTextStream // text stream class
{
    Q_DECLARE_PRIVATE(VTextStream)

public:
    enum RealNumberNotation {
        SmartNotation,
        FixedNotation,
        ScientificNotation
    };
    enum FieldAlignment {
        AlignLeft,
        AlignRight,
        AlignCenter,
        AlignAccountingStyle
    };
    enum Status {
        Ok,
        ReadPastEnd,
        ReadCorruptData,
        WriteFailed
    };
    enum NumberFlag {
        ShowBase = 0x1,
        ForcePoint = 0x2,
        ForceSign = 0x4,
        UppercaseBase = 0x8,
        UppercaseDigits = 0x10
    };
    Q_DECLARE_FLAGS(NumberFlags, NumberFlag)

    VTextStream();
    explicit VTextStream(QIODevice *device);
    explicit VTextStream(FILE *fileHandle, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit VTextStream(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit VTextStream(QByteArray *array, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    explicit VTextStream(const QByteArray &array, QIODevice::OpenMode openMode = QIODevice::ReadOnly);
    virtual ~VTextStream();

#if defined(WITH_TEXTCODEC)
    void setCodec(QTextCodec *codec);
    void setCodec(const char *codecName);
    QTextCodec *codec() const;
    void setAutoDetectUnicode(bool enabled);
    bool autoDetectUnicode() const;
    void setGenerateByteOrderMark(bool generate);
    bool generateByteOrderMark() const;
#endif

    void setLocale(const QLocale &locale);
    QLocale locale() const;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setString(QString *string, QIODevice::OpenMode openMode = QIODevice::ReadWrite);
    QString *string() const;

    Status status() const;
    void setStatus(Status status);
    void resetStatus();

    bool atEnd() const;
    void reset();
    void flush();
    bool seek(qint64 pos);
    qint64 pos() const;

    void skipWhiteSpace();

    QString readLine(qint64 maxlen = 0);
    bool readLineInto(QString *line, qint64 maxlen = 0);
    QString readAll();
    QString read(qint64 maxlen);

    void setFieldAlignment(FieldAlignment alignment);
    FieldAlignment fieldAlignment() const;

    void setPadChar(QChar ch);
    QChar padChar() const;

    void setFieldWidth(qsizetype width);
    qsizetype fieldWidth() const;

    void setNumberFlags(NumberFlags flags);
    NumberFlags numberFlags() const;

    void setIntegerBase(int base);
    int integerBase() const;

    void setRealNumberNotation(RealNumberNotation notation);
    RealNumberNotation realNumberNotation() const;

    void setRealNumberPrecision(int precision);
    int realNumberPrecision() const;

    VTextStream &operator>>(QChar &ch);
    VTextStream &operator>>(char &ch);
    VTextStream &operator>>(signed short &i);
    VTextStream &operator>>(unsigned short &i);
    VTextStream &operator>>(signed int &i);
    VTextStream &operator>>(unsigned int &i);
    VTextStream &operator>>(signed long &i);
    VTextStream &operator>>(unsigned long &i);
    VTextStream &operator>>(qlonglong &i);
    VTextStream &operator>>(qulonglong &i);
    VTextStream &operator>>(float &f);
    VTextStream &operator>>(double &f);
    VTextStream &operator>>(QString &s);
    VTextStream &operator>>(QByteArray &array);
    VTextStream &operator>>(char *c);

    VTextStream &operator<<(QChar ch);
    VTextStream &operator<<(char ch);
    VTextStream &operator<<(signed short i);
    VTextStream &operator<<(unsigned short i);
    VTextStream &operator<<(signed int i);
    VTextStream &operator<<(unsigned int i);
    VTextStream &operator<<(signed long i);
    VTextStream &operator<<(unsigned long i);
    VTextStream &operator<<(qlonglong i);
    VTextStream &operator<<(qulonglong i);
    VTextStream &operator<<(float f);
    VTextStream &operator<<(double f);
    VTextStream &operator<<(const QString &s);
    VTextStream &operator<<(QStringView s);
    VTextStream &operator<<(QLatin1String s);
    VTextStream &operator<<(const QByteArray &array);
    VTextStream &operator<<(const char *c);
    VTextStream &operator<<(const void *ptr);

private:
    Q_DISABLE_COPY_MOVE(VTextStream)
    friend class QDebugStateSaverPrivate;
    friend class QDebug;

    QScopedPointer<VTextStreamPrivate> d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VTextStream::NumberFlags)

/*****************************************************************************
  VTextStream manipulators
 *****************************************************************************/

using VTextStreamFunction = VTextStream &(*) (VTextStream &); // manipulator function
using VTSMFI = void (VTextStream::*)(int);                    // manipulator w/int argument
using VTSMFS = void (VTextStream::*)(qsizetype);              // manipulator w/qsizetype argument
using VTSMFC = void (VTextStream::*)(QChar);                  // manipulator w/QChar argument

class Q_CORE_EXPORT VTextStreamManipulator
{
public:
    Q_DECL_CONSTEXPR VTextStreamManipulator(VTSMFS m, qsizetype a) noexcept
      : mfi(nullptr),
        mfs(m),
        mc(nullptr),
        argS(a),
        argI(0),
        ch()
    {
    }
    Q_DECL_CONSTEXPR VTextStreamManipulator(VTSMFI m, int a) noexcept
      : mfi(m),
        mfs(nullptr),
        mc(nullptr),
        argS(0),
        argI(a),
        ch()
    {
    }
    Q_DECL_CONSTEXPR VTextStreamManipulator(VTSMFC m, QChar c) noexcept
      : mfi(nullptr),
        mfs(nullptr),
        mc(m),
        argS(-1),
        argI(-1),
        ch(c)
    {
    }
    void exec(VTextStream &s)
    {
        if (mfi)
        {
            (s.*mfi)(argI);
        }
        else if (mfs)
        {
            (s.*mfs)(argS);
        }
        else
        {
            (s.*mc)(ch);
        }
    }

private:
    VTSMFI mfi;     // VTextStream member function
    VTSMFS mfs;     // VTextStream member function
    VTSMFC mc;      // VTextStream member function
    qsizetype argS; // member function argument
    int argI;       // member function argument
    QChar ch;
};

inline VTextStream &operator>>(VTextStream &s, VTextStreamFunction f)
{
    return (*f)(s);
}

inline VTextStream &operator<<(VTextStream &s, VTextStreamFunction f)
{
    return (*f)(s);
}

inline VTextStream &operator<<(VTextStream &s, VTextStreamManipulator m)
{
    m.exec(s);
    return s;
}

namespace Val
{
VTextStream &bin(VTextStream &s);
VTextStream &oct(VTextStream &s);
VTextStream &dec(VTextStream &s);
VTextStream &hex(VTextStream &s);

VTextStream &showbase(VTextStream &s);
VTextStream &forcesign(VTextStream &s);
VTextStream &forcepoint(VTextStream &s);
VTextStream &noshowbase(VTextStream &s);
VTextStream &noforcesign(VTextStream &s);
VTextStream &noforcepoint(VTextStream &s);

VTextStream &uppercasebase(VTextStream &s);
VTextStream &uppercasedigits(VTextStream &s);
VTextStream &lowercasebase(VTextStream &s);
VTextStream &lowercasedigits(VTextStream &s);

VTextStream &fixed(VTextStream &s);
VTextStream &scientific(VTextStream &s);

VTextStream &left(VTextStream &s);
VTextStream &right(VTextStream &s);
VTextStream &center(VTextStream &s);

VTextStream &endl(VTextStream &s);
VTextStream &flush(VTextStream &s);
VTextStream &reset(VTextStream &s);

VTextStream &bom(VTextStream &s);

VTextStream &ws(VTextStream &s);

} // namespace Val

#endif // VTEXTSTREAM_H
