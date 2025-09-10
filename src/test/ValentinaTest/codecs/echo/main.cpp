// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <cstdio>
#include <cstdlib>
#include <QtCore/QCoreApplication>

#include "../vmisc/codecs/qtextcodec.h"

int main(int argc, char **argv)
{
    qputenv("LC_ALL", "C");
    QCoreApplication const app(argc, argv);

    QString const string(QChar(0x410));
    const QTextCodec *locale = QTextCodec::codecForLocale();
    QTextEncoder *encoder = locale->makeEncoder();
    QByteArray output = encoder->fromUnicode(string);
    printf("%s\n", output.data());
    delete encoder;

    return 0;
}
