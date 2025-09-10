// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWINDOWSCODEC_P_H
#define QWINDOWSCODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qtextcodec.h"

#ifndef WITH_TEXTCODEC
#error \
    "QWindowsLocalCodec support is required but has not been enabled. Please enable the withTextCodec in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QWindowsLocalCodec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QWindowsLocalCodec)

public:
    QWindowsLocalCodec() = default;
    virtual ~QWindowsLocalCodec() = default;

    QString convertToUnicode(const char *chars, int length, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *ch, int uclen, ConverterState *state) const override;
    QString convertToUnicodeCharByChar(const char *chars, int length, ConverterState *state) const;

    QByteArray name() const override;
    int mibEnum() const override;
};

QT_END_NAMESPACE

#endif
