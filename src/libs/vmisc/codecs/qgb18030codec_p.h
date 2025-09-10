// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// Contributed by James Su <suzhe@gnuchina.org>

#ifndef QGB18030CODEC_P_H
#define QGB18030CODEC_P_H

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
    "QGb18030Codec support is required but has not been enabled. Please enable the withBigCodecs in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QGb18030Codec : public QTextCodec {
    Q_DISABLE_COPY_MOVE(QGb18030Codec)

public:
    QGb18030Codec() = default;
    virtual ~QGb18030Codec() = default;

    static QByteArray _name() { return "GB18030"; }
    static QList<QByteArray> _aliases() { return {}; }
    static int _mibEnum() { return 114; }

    QByteArray name() const override { return _name(); }
    QList<QByteArray> aliases() const override { return _aliases(); }
    int mibEnum() const override { return _mibEnum(); }

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;
};

class QGbkCodec : public QGb18030Codec {
    Q_DISABLE_COPY_MOVE(QGbkCodec)

public:
    QGbkCodec();
    ~QGbkCodec() override = default;

    static QByteArray _name();
    static QList<QByteArray> _aliases();
    static int _mibEnum();

    QByteArray name() const override { return _name(); }
    QList<QByteArray> aliases() const override { return _aliases(); }
    int mibEnum() const override { return _mibEnum(); }

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;
};

class QGb2312Codec : public QGb18030Codec {
    Q_DISABLE_COPY_MOVE(QGb2312Codec)

public:
    QGb2312Codec();
    ~QGb2312Codec() override = default;

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return {}; }
    static int _mibEnum();

    QByteArray name() const override { return _name(); }
    int mibEnum() const override { return _mibEnum(); }

    QString convertToUnicode(const char *, int, ConverterState *) const override;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;
};

QT_END_NAMESPACE

#endif // QGB18030CODEC_P_H
