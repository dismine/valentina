// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Most of the code here was originally written by Ming-Che Chuang and
// is included in Qt with the author's permission, and the grateful
// thanks of the Qt team.

#ifndef QBIG5CODEC_P_H
#define QBIG5CODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qlist.h>

#include "qtextcodec.h"

#ifndef WITH_BIG_CODECS
#error \
    "QBig5Codec support is required but has not been enabled. Please enable the withBigCodecs in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QBig5Codec : public QTextCodec {
    Q_DISABLE_COPY_MOVE(QBig5Codec)

public:
    QBig5Codec() = default;
    virtual ~QBig5Codec() = default;

    static QByteArray _name();
    static QList<QByteArray> _aliases();
    static int _mibEnum();

    QByteArray name() const override { return _name(); }
    QList<QByteArray> aliases() const override { return _aliases(); }
    int mibEnum() const override { return _mibEnum(); }

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *state) const override;
};

class QBig5hkscsCodec : public QTextCodec {
    Q_DISABLE_COPY_MOVE(QBig5hkscsCodec)

public:
    QBig5hkscsCodec() = default;
    virtual ~QBig5hkscsCodec() = default;

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return {}; }
    static int _mibEnum();

    QByteArray name() const override { return _name(); }
    QList<QByteArray> aliases() const override { return _aliases(); }
    int mibEnum() const override { return _mibEnum(); }

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *state) const override;
};

QT_END_NAMESPACE

#endif // QBIG5CODEC_P_H
