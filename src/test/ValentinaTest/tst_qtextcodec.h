// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2016 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/************************************************************************
 **
 **  @file   tst_qtextcodec.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 9, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
#ifndef TST_QTEXTCODEC_H
#define TST_QTEXTCODEC_H

#include "../vtest/abstracttest.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <qtclasshelpermacros.h>
#endif

class TST_QTextCodec : public AbstractTest
{
    Q_OBJECT // NOLINT

public:
    explicit TST_QTextCodec(QObject *parent = nullptr);
    ~TST_QTextCodec() override = default;

private slots:
    void threadSafety();

    void toUnicode_data();
    void toUnicode();
    void codecForName_data();
    void codecForName();
    void fromUnicode_data();
    void fromUnicode();
    void toUnicode_codecForHtml();
    void toUnicode_incremental();
    void codecForLocale();

    void asciiToIscii() const;
    void unicodeToISCII();

    void nonFlaggedCodepointFFFF() const;
    void flagF7808080() const;
    void nonFlaggedEFBFBF() const;
    void decode0D() const;
    void aliasForUTF16() const;
    void mibForTSCII() const;
    void codecForTSCII() const;
    void asciiToTSCII();
    void unicodeToTSCII();

    void iso8859_1() const;
    void iso8859_15() const;
    void iso8859_16() const;

    void utf32Codec_data();
    void utf32Codec();

    void utf8Codec_data();
    void utf8Codec();

    void utf8bom_data();
    void utf8bom();

    void utf8stateful_data();
    void utf8stateful();

    void utfHeaders_data();
    void utfHeaders();

    void codecForHtml_data();
    void codecForHtml();

    void codecForUtfText_data();
    void codecForUtfText();

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
    void toLocal8Bit();
#endif

    void invalidNames();
    void checkAliases_data();
    void checkAliases();

    void moreToFromUnicode_data();
    void moreToFromUnicode();

    void shiftJis();
    void userCodec();

    void nullInputZeroOrNegativeLength_data();
    void nullInputZeroOrNegativeLength();

    void retainNullness_data();
    void retainNullness();

    void canEncode();
    void canEncode_data();

private:
    Q_DISABLE_COPY_MOVE(TST_QTextCodec)
};

#endif // TST_QTEXTCODEC_H
