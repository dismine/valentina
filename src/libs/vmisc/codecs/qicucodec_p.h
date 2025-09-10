// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QICUCODEC_P_H
#define QICUCODEC_P_H

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

extern "C" {
    using UConverter = struct UConverter;
}

#ifndef WITH_TEXTCODEC
#error \
    "QIcuCodec support is required but has not been enabled. Please enable the withTextCodec in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QIcuCodec : public QTextCodec
{
    Q_DISABLE_COPY_MOVE(QIcuCodec)

public:
    static QList<QByteArray> availableCodecs();
    static QList<int> availableMibs();

    static QTextCodec *defaultCodecUnlocked();

    static QTextCodec *codecForNameUnlocked(const char *name);
    static QTextCodec *codecForMibUnlocked(int mib);

    QString convertToUnicode(const char *chars, int length, ConverterState *state) const override;
    QByteArray convertFromUnicode(const QChar *unicode, int length, ConverterState *state) const override;

    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

private:
    explicit QIcuCodec(const char *name);
    virtual ~QIcuCodec() = default;

    UConverter *getConverter(QTextCodec::ConverterState *state) const;

    const char *m_name;
};

QT_END_NAMESPACE

#endif
