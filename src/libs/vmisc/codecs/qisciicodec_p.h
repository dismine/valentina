// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QISCIICODEC_P_H
#define QISCIICODEC_P_H

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

#ifndef WITH_BASIC_CODECS
#error \
    "QTsciiCodec support is required but has not been enabled. Please enable the withBasicCodecs in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QIsciiCodec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QIsciiCodec)

public:
    explicit QIsciiCodec(int i) : idx(i) {}
    virtual ~QIsciiCodec() = default;

    static QTextCodec *create(const char *name);

    QByteArray name() const override;
    int mibEnum() const override;

    QString convertToUnicode(const char *chars, int len, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *state) const override;

private:
    int idx;
};

QT_END_NAMESPACE

#endif // QISCIICODEC_P_H
