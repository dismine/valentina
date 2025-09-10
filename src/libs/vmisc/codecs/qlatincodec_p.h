// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLATINCODEC_P_H
#define QLATINCODEC_P_H

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

#include "qtextcodec.h"

#ifndef WITH_TEXTCODEC
#error \
    "QTextCodec support is required but has not been enabled. Please enable the withTextCodec in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QLatin1Codec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QLatin1Codec)

public:
    QLatin1Codec() = default;
    virtual ~QLatin1Codec() = default;

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *ch, int len, ConverterState *state) const override;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

class QLatin15Codec: public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QLatin15Codec)

public:
    QLatin15Codec() = default;
    virtual ~QLatin15Codec() = default;

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *ch, int len, ConverterState *state) const override;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;
};

QT_END_NAMESPACE

#endif // QLATINCODEC_P_H
