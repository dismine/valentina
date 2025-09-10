// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QICONVCODEC_P_H
#define QICONVCODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtextcodec.h"

#ifndef WITH_ICONV_CODECS
#error \
    "QIconvCodec support is required but has not been enabled. Please enable the withICONVCodecs in your build configuration."
#endif

#include <iconv.h>

QT_BEGIN_NAMESPACE

class QIconvCodec: public QTextCodec
{
private:
    mutable QTextCodec *utf16Codec;

    Q_DISABLE_COPY_MOVE(QIconvCodec)

public:
    QIconvCodec();
    virtual ~QIconvCodec() = default;

    QString convertToUnicode(const char *chars, int len, ConverterState *convState) const override;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *convState) const override;

    QByteArray name() const override;
    int mibEnum() const override;

    void init() const;
    iconv_t createIconv_t(const char *to, const char *from) const;

    class IconvState
    {
    public:
        IconvState(iconv_t x);
        ~IconvState();
        ConverterState internalState;
        char *buffer;
        int bufferLen;
        iconv_t cd;

        char array[8];

        void saveChars(const char *c, int count);
    };
};

QT_END_NAMESPACE

#endif // QICONVCODEC_P_H
