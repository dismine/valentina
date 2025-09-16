/************************************************************************
 **
 **  @file   tst_utf8.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 9, 2025
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
#ifndef TST_UTF8_H
#define TST_UTF8_H

#include <QObject>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <qtclasshelpermacros.h>
#endif

class QTextCodec;

class TST_Utf8 : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_Utf8(QObject *parent = nullptr);
    ~TST_Utf8() override = default;

public slots:
    void initTestCase();
    void init();

private slots:
    void roundTrip_data();
    void roundTrip();

    void charByChar_data();
    void charByChar();

    void invalidUtf8_data();
    void invalidUtf8();

    void nonCharacters_data();
    void nonCharacters();

private:
    Q_DISABLE_COPY_MOVE(TST_Utf8)

    // test data:
    QTextCodec *codec{nullptr};
    QString (*from8BitPtr)(const char *, qsizetype){nullptr};
    static QByteArray to8Bit(const QString &s);
    QString from8Bit(const QByteArray &ba);

    void loadInvalidUtf8Rows();
    void loadNonCharactersRows();
};

#endif // TST_UTF8_H
