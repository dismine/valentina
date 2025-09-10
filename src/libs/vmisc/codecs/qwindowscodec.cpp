// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwindowscodec_p.h"

#include <qbytearray.h>
#include <qstring.h>
#include <qt_windows.h>
#include <qvarlengtharray.h>

QT_BEGIN_NAMESPACE

QString QWindowsLocalCodec::convertToUnicode(const char *chars, int length, ConverterState *state) const
{
    const char *mb = chars;
    int mblen = length;

    if (!mb || !mblen)
        return QString();

    QVarLengthArray<wchar_t, 4096> wc(4096);
    int len;
    QString sp;
    bool prepend = false;
    char state_data = 0;
    int remainingChars = 0;

    //save the current state information
    if (state)
    {
        state_data = (char) state->state_data[0];
        remainingChars = state->remainingChars;
    }

    //convert the pending charcter (if available)
    if (state && remainingChars)
    {
        char prev[3] = {0};
        prev[0] = state_data;
        prev[1] = mb[0];
        remainingChars = 0;
        len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, prev, 2, wc.data(), wc.length());
        if (len)
        {
            sp.append(QChar(wc[0]));
            if (mblen == 1)
            {
                state->remainingChars = 0;
                return sp;
            }
            prepend = true;
            mb++;
            mblen--;
            wc[0] = 0;
        }
    }

    while (
        !(len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, mb, mblen, wc.data(), wc.length())))
    {
        int r = GetLastError();
        if (r == ERROR_INSUFFICIENT_BUFFER)
        {
            const int wclen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mb, mblen, 0, 0);
            wc.resize(wclen);
        }
        else if (r == ERROR_NO_UNICODE_TRANSLATION)
        {
            //find the last non NULL character
            while (mblen > 1 && !(mb[mblen - 1]))
                mblen--;
            //check whether,  we hit an invalid character in the middle
            if ((mblen <= 1) || (remainingChars && state_data))
                return convertToUnicodeCharByChar(chars, length, state);
            //Remove the last character and try again...
            state_data = mb[mblen - 1];
            remainingChars = 1;
            mblen--;
        }
        else
        {
            // Fail.
            qWarning("MultiByteToWideChar: Cannot convert multibyte text");
            break;
        }
    }

    if (len <= 0)
        return QString();

    if (wc[len - 1] == 0) // len - 1: we don't want terminator
        --len;

    //save the new state information
    if (state)
    {
        state->state_data[0] = (char) state_data;
        state->remainingChars = remainingChars;
    }
    QString s((QChar *) wc.data(), len);
    if (prepend)
    {
        return sp + s;
    }
    return s;
}

QByteArray QWindowsLocalCodec::convertFromUnicode(const QChar *ch, int uclen, ConverterState *state) const
{
    if (!ch)
        return QByteArray();
    if (uclen == 0)
        return QByteArray("");
    BOOL used_def;
    QByteArray mb(4096, 0);
    int len;
    while (!(len = WideCharToMultiByte(CP_ACP, 0, (const wchar_t *) ch, uclen, mb.data(), mb.size() - 1, 0, &used_def)))
    {
        int r = GetLastError();
        if (r == ERROR_INSUFFICIENT_BUFFER)
        {
            mb.resize(1 + WideCharToMultiByte(CP_ACP, 0, (const wchar_t *) ch, uclen, 0, 0, 0, &used_def));
            // and try again...
        }
        else
        {
            // Fail.  Probably can't happen in fact (dwFlags is 0).
#ifndef QT_NO_DEBUG
            // Can't use qWarning(), as it'll recurse to handle %ls
            fprintf(stderr,
                    "WideCharToMultiByte: Cannot convert multibyte text (error %d): %ls\n",
                    r,
                    reinterpret_cast<const wchar_t *>(QString(ch, uclen).utf16()));
#endif
            break;
        }
    }
    mb.resize(len);
    return mb;
}

QString QWindowsLocalCodec::convertToUnicodeCharByChar(const char *chars, int length, ConverterState *state) const
{
    const char *mb = chars;
    int mblen = length;

    if (!mb || !mblen)
        return QString();

    QVarLengthArray<wchar_t, 4096> wc(4096);
    int len;
    QString sp;
    bool prepend = false;
    char state_data = 0;
    int remainingChars = 0;

    //save the current state information
    if (state)
    {
        state_data = (char) state->state_data[0];
        remainingChars = state->remainingChars;
    }

    //convert the pending charcter (if available)
    if (state && remainingChars)
    {
        char prev[3] = {0};
        prev[0] = state_data;
        prev[1] = mb[0];
        remainingChars = 0;
        len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, prev, 2, wc.data(), wc.length());
        if (len)
        {
            sp.append(QChar(wc[0]));
            if (mblen == 1)
            {
                state->remainingChars = 0;
                return sp;
            }
            prepend = true;
            mb++;
            mblen--;
            wc[0] = 0;
        }
    }

    while (
        !(len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, mb, mblen, wc.data(), wc.length())))
    {
        int r = GetLastError();
        if (r == ERROR_INSUFFICIENT_BUFFER)
        {
            const int wclen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mb, mblen, 0, 0);
            wc.resize(wclen);
        }
        else if (r == ERROR_NO_UNICODE_TRANSLATION)
        {
            //find the last non NULL character
            while (mblen > 1 && !(mb[mblen - 1]))
                mblen--;
            //check whether,  we hit an invalid character in the middle
            if ((mblen <= 1) || (remainingChars && state_data))
                return convertToUnicodeCharByChar(chars, length, state);
            //Remove the last character and try again...
            state_data = mb[mblen - 1];
            remainingChars = 1;
            mblen--;
        }
        else
        {
            // Fail.
            qWarning("MultiByteToWideChar: Cannot convert multibyte text");
            break;
        }
    }

    if (len <= 0)
        return QString();

    if (wc[len - 1] == 0) // len - 1: we don't want terminator
        --len;

    //save the new state information
    if (state)
    {
        state->state_data[0] = (char) state_data;
        state->remainingChars = remainingChars;
    }
    QString s((QChar *) wc.data(), len);
    if (prepend)
    {
        return sp + s;
    }
    return s;
}

QByteArray QWindowsLocalCodec::name() const
{
    return "System";
}

int QWindowsLocalCodec::mibEnum() const
{
    return 0;
}

QT_END_NAMESPACE
