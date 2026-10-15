/************************************************************************
 **
 **  @file   vpaccountclient.h
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

#ifndef VACCOUNTCLIENT_H
#define VACCOUNTCLIENT_H

#include "vaccounttoken.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkReply;

/**
 * @brief Connects Valentina or Puzzle to the account server with the OAuth 2 device flow (RFC 8628) and keeps the
 * resulting token in the settings. The only writer of the account/token, account/email and account/name keys.
 */
class VAccountClient : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VAccountClient(QObject *parent = nullptr);
    ~VAccountClient() override = default;

    auto Token() const -> VAccountToken;
    auto IsLoggedIn() const -> bool;

    void StartDeviceLogin();
    void CancelLogin();
    void RefreshUserInfo();
    void Logout();
    void CheckService(const QString &baseUrl);

signals:
    void UserCodeReady(const QString &userCode, const QUrl &verificationUrl);
    void LoggedIn();
    void LoggedOut();
    void UserInfo(const QJsonObject &info);
    void ServiceChecked(bool ok, const QString &message);
    void Failed(const QString &message);

private slots:
    void Poll();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VAccountClient) // NOLINT

    QNetworkAccessManager m_qnam{};
    QTimer m_poll{};
    QUrl m_issuer{};
    QUrl m_deviceEndpoint{};
    QUrl m_tokenEndpoint{};
    QUrl m_userinfoEndpoint{};
    QString m_deviceCode{};
    int m_intervalS{5};
    qint64 m_deadline{0};
    int m_generation{0}; // bumped by StopPolling: replies of an older sign-in attempt are stale

    auto Get(const QUrl &url, bool bearer = false) -> QNetworkReply *;
    auto PostForm(const QUrl &url, const QByteArray &form) -> QNetworkReply *;
    static auto Json(QNetworkReply *reply) -> QJsonObject;
    auto Endpoint(const QJsonObject &discovery, const QString &key) -> QUrl;
    auto DiscoveryUrl() const -> QUrl;
    void Discover();
    void RequestDeviceCode();
    void StopPolling();
};

#endif // VACCOUNTCLIENT_H
