/************************************************************************
 **
 **  @file   tst_vaccounttoken.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
 **
 **  @brief  
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making 
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project 
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

#include "tst_vaccounttoken.h"
#include "../vnesting/vaccounttoken.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest>

namespace
{
auto B64Url(const QByteArray &data) -> QString
{
    return QString::fromLatin1(data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

auto MakeJwt(const QJsonObject &payload) -> QString
{
    const QJsonObject header{{"alg", "EdDSA"}, {"typ", "JWT"}};
    return B64Url(QJsonDocument(header).toJson(QJsonDocument::Compact)) + '.' +
           B64Url(QJsonDocument(payload).toJson(QJsonDocument::Compact)) + ".c2lnbmF0dXJl";
}

auto Payload(bool premium, qint64 exp) -> QJsonObject
{
    return {{"iss", "https://issuer.test"},
            {"aud", "puzzle"},
            {"sub", "42"},
            {"email", "p@example.com"},
            {"name", "Premium User"},
            {"premium", premium},
            {"premium_until", QJsonValue::Null},
            {"credits", 0},
            {"iat", exp - 7 * 86400},
            {"exp", exp}};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VAccountToken::TST_VAccountToken(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::ValidPremiumToken()
{
    const qint64 exp = QDateTime::currentSecsSinceEpoch() + 3600;
    const VAccountToken t(MakeJwt(Payload(true, exp)));
    QVERIFY(t.IsValid());
    QCOMPARE(t.Subject(), QStringLiteral("42"));
    QCOMPARE(t.Email(), QStringLiteral("p@example.com"));
    QCOMPARE(t.Name(), QStringLiteral("Premium User"));
    QVERIFY(t.IsPremium());
    QVERIFY(not t.PremiumUntil().isValid());
    QCOMPARE(t.Expiry().toSecsSinceEpoch(), exp);
    QVERIFY(not t.IsExpired());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::BasicToken()
{
    QJsonObject p = Payload(false, QDateTime::currentSecsSinceEpoch() + 3600);
    p["premium_until"] = QStringLiteral("2027-01-31");
    const VAccountToken t(MakeJwt(p));
    QVERIFY(t.IsValid());
    QVERIFY(not t.IsPremium());
    QCOMPARE(t.PremiumUntil(), QDate(2027, 1, 31));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::ExpiredToken()
{
    const VAccountToken t(MakeJwt(Payload(true, QDateTime::currentSecsSinceEpoch() - 1)));
    QVERIFY(t.IsValid());
    QVERIFY(t.IsExpired());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::NotThreeParts()
{
    QVERIFY(not VAccountToken(QStringLiteral("abc.def")).IsValid());
    QVERIFY(not VAccountToken(QStringLiteral("not a jwt")).IsValid());
    QVERIFY(VAccountToken(QStringLiteral("abc.def")).IsExpired());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::PayloadNotJsonObject()
{
    // A JSON array as payload must not be read as claims
    const QString jwt = B64Url("{}") + '.' + B64Url("[1,2]") + ".sig";
    const VAccountToken t(jwt);
    QVERIFY(not t.IsValid());
    QVERIFY(not t.IsPremium());
    QVERIFY(t.IsExpired());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::ExpNotNumeric()
{
    QJsonObject p = Payload(true, 0);
    p["exp"] = QStringLiteral("tomorrow");
    const VAccountToken t(MakeJwt(p));
    QVERIFY(not t.IsValid());
    QVERIFY(not t.IsPremium());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::DefaultIsInvalid()
{
    const VAccountToken t;
    QVERIFY(not t.IsValid());
    QVERIFY(t.Raw().isEmpty());
    QVERIFY(t.IsExpired());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VAccountToken::SameOriginCheck()
{
    // Discovery endpoints must stay on the issuer's origin
    QVERIFY(SameOrigin(QUrl(QStringLiteral("https://nestor.lossouarn.net/issuer")),
                       QUrl(QStringLiteral("https://nestor.lossouarn.net/issuer/token"))));
    QVERIFY(SameOrigin(QUrl(QStringLiteral("https://Example.org")), QUrl(QStringLiteral("https://example.org:443/x"))));
    QVERIFY(not SameOrigin(QUrl(QStringLiteral("https://example.org")), QUrl(QStringLiteral("http://example.org/x"))));
    QVERIFY(not SameOrigin(QUrl(QStringLiteral("https://example.org")),
                           QUrl(QStringLiteral("https://evil.example.org/x"))));
    QVERIFY(not SameOrigin(QUrl(QStringLiteral("https://example.org")),
                           QUrl(QStringLiteral("https://example.org:8443/x"))));
}
