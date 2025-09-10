// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlatincodec_p.h"
#include "qlist.h"
#include <qtpreprocessorsupport.h>

QT_BEGIN_NAMESPACE

QString QLatin1Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    Q_UNUSED(state)
    return QString::fromLatin1(chars, len);
}

QByteArray QLatin1Codec::convertFromUnicode(const QChar *ch, int len, ConverterState *state) const
{
    const char replacement = (state && state->flags & ConvertInvalidToNull) ? 0 : '?';
    QByteArray r(len, QT_PREPEND_NAMESPACE(Qt::Uninitialized));
    char *d = r.data();
    int invalid = 0;
    for (int i = 0; i < len; ++i) {
        if (ch[i] > QChar(0xff)) {
            d[i] = replacement;
            ++invalid;
        } else {
            d[i] = static_cast<char>(ch[i].cell());
        }
    }
    if (state) {
        state->invalidChars += invalid;
    }
    return r;
}

QByteArray QLatin1Codec::name() const
{
    return "ISO-8859-1";
}

QList<QByteArray> QLatin1Codec::aliases() const
{
    QList<QByteArray> list;
    list << "latin1"
         << "CP819"
         << "IBM819"
         << "iso-ir-100"
         << "csISOLatin1";
    return list;
}


int QLatin1Codec::mibEnum() const
{
    return 4;
}

QString QLatin15Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
    Q_UNUSED(state)
    QString str = QString::fromLatin1(chars, len);
    QChar *uc = str.data();
    while(len--) {
        switch(uc->unicode()) {
            case 0xa4:
                *uc = QChar(0x20ac);
                break;
            case 0xa6:
                *uc = QChar(0x0160);
                break;
            case 0xa8:
                *uc = QChar(0x0161);
                break;
            case 0xb4:
                *uc = QChar(0x017d);
                break;
            case 0xb8:
                *uc = QChar(0x017e);
                break;
            case 0xbc:
                *uc = QChar(0x0152);
                break;
            case 0xbd:
                *uc = QChar(0x0153);
                break;
            case 0xbe:
                *uc = QChar(0x0178);
                break;
            default:
                break;
        }
        uc++;
    }
    return str;
}

QByteArray QLatin15Codec::convertFromUnicode(const QChar *ch, int len, ConverterState *state) const
{
    const char replacement = (state && state->flags & ConvertInvalidToNull) ? 0 : '?';
    QByteArray r(len, QT_PREPEND_NAMESPACE(Qt::Uninitialized));
    char *d = r.data();
    int invalid = 0;
    for (int i = 0; i < len; ++i)
    {
        uchar c;
        ushort uc = ch[i].unicode();
        if (uc < 0x0100) {
            if (uc > 0xa3) {
                switch(uc) {
                case 0xa4:
                case 0xa6:
                case 0xa8:
                case 0xb4:
                case 0xb8:
                case 0xbc:
                case 0xbd:
                case 0xbe:
                    c = static_cast<uchar>(replacement);
                    ++invalid;
                    break;
                default:
                    c = static_cast<unsigned char>(uc);
                    break;
                }
            } else {
                c = static_cast<unsigned char>(uc);
            }
        } else {
            if (uc == 0x20ac)
                c = 0xa4;
            else if ((uc & 0xff00) == 0x0100) {
                switch(uc) {
                case 0x0160:
                    c = 0xa6;
                    break;
                case 0x0161:
                    c = 0xa8;
                    break;
                case 0x017d:
                    c = 0xb4;
                    break;
                case 0x017e:
                    c = 0xb8;
                    break;
                case 0x0152:
                    c = 0xbc;
                    break;
                case 0x0153:
                    c = 0xbd;
                    break;
                case 0x0178:
                    c = 0xbe;
                    break;
                default:
                    c = static_cast<uchar>(replacement);
                    ++invalid;
                }
            } else {
                c = static_cast<uchar>(replacement);
                ++invalid;
            }
        }
        d[i] = static_cast<char>(c);
    }
    if (state) {
        state->remainingChars = 0;
        state->invalidChars += invalid;
    }
    return r;
}


QByteArray QLatin15Codec::name() const
{
    return "ISO-8859-15";
}

QList<QByteArray> QLatin15Codec::aliases() const
{
    QList<QByteArray> list;
    list << "latin9";
    return list;
}

int QLatin15Codec::mibEnum() const
{
    return 111;
}

QT_END_NAMESPACE
