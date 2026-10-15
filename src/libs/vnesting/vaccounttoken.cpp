/************************************************************************
 **
 **  @file   vaccounttoken.cpp
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

#include "vaccounttoken.h"

#include <QJsonDocument>

//---------------------------------------------------------------------------------------------------------------------
VAccountToken::VAccountToken(const QString &jwt)
  : m_raw(jwt)
{
    const QStringList parts = jwt.split('.');
    if (parts.size() != 3)
    {
        return;
    }
    const QByteArray payload = QByteArray::fromBase64(parts.at(1).toLatin1(), QByteArray::Base64UrlEncoding);
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (not doc.isObject())
    {
        return;
    }
    m_claims = doc.object();
    m_valid = m_claims.value(QStringLiteral("exp")).isDouble();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::IsValid() const -> bool
{
    return m_valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::Raw() const -> QString
{
    return m_raw;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::Subject() const -> QString
{
    return m_claims.value(QStringLiteral("sub")).toString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::Email() const -> QString
{
    return m_claims.value(QStringLiteral("email")).toString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::Name() const -> QString
{
    return m_claims.value(QStringLiteral("name")).toString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::IsPremium() const -> bool
{
    return m_valid && m_claims.value(QStringLiteral("premium")).toBool(false);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::PremiumUntil() const -> QDate
{
    return QDate::fromString(m_claims.value(QStringLiteral("premium_until")).toString(), Qt::ISODate);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::Expiry() const -> QDateTime
{
    if (not m_valid)
    {
        return {};
    }
    // QTimeZone::UTC needs Qt 6.5
    return QDateTime::fromSecsSinceEpoch(static_cast<qint64>(m_claims.value(QStringLiteral("exp")).toDouble())).toUTC();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountToken::IsExpired(const QDateTime &now) const -> bool
{
    return not m_valid || Expiry() <= now;
}

//---------------------------------------------------------------------------------------------------------------------
auto SameOrigin(const QUrl &a, const QUrl &b) -> bool
{
    auto port = [](const QUrl &u) { return u.port(u.scheme() == QLatin1String("https") ? 443 : 80); };
    return a.scheme() == b.scheme() && a.host().compare(b.host(), Qt::CaseInsensitive) == 0 && port(a) == port(b);
}
