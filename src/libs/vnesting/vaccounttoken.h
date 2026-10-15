/************************************************************************
 **
 **  @file   vaccounttoken.h
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

#ifndef VACCOUNTTOKEN_H
#define VACCOUNTTOKEN_H

#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QUrl>

/**
 * @brief Reads the claims Puzzle needs from the account server's JWT. The signature is NOT checked here:
 * only the nesting service verifies it. This class decides what the UI shows, never what the server grants.
 */
class VAccountToken
{
public:
    VAccountToken() = default;
    explicit VAccountToken(const QString &jwt);

    auto IsValid() const -> bool;
    auto Raw() const -> QString;
    auto Subject() const -> QString;
    auto Email() const -> QString;
    auto Name() const -> QString;
    auto IsPremium() const -> bool;
    auto PremiumUntil() const -> QDate;
    auto Expiry() const -> QDateTime;
    auto IsExpired(const QDateTime &now = QDateTime::currentDateTimeUtc()) const -> bool;

private:
    QString m_raw{};
    QJsonObject m_claims{};
    bool m_valid{false};
};

/// Same scheme, host (case-insensitive) and port (after scheme defaults): a discovery document may not send
/// the client to another host.
auto SameOrigin(const QUrl &a, const QUrl &b) -> bool;

#endif // VACCOUNTTOKEN_H
