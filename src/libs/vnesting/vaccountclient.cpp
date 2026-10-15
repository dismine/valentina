/************************************************************************
 **
 **  @file   vpaccountclient.cpp
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

#include "vaccountclient.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "vnestorclient.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

using namespace Qt::Literals::StringLiterals;

namespace
{
const auto grantDeviceCode = "urn:ietf:params:oauth:grant-type:device_code"_L1;
const auto clientId = "puzzle"_L1;
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VAccountClient::VAccountClient(QObject *parent)
  : QObject(parent)
{
    m_poll.setSingleShot(true);
    connect(&m_poll, &QTimer::timeout, this, &VAccountClient::Poll);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::Token() const -> VAccountToken
{
    return VAccountToken(VAbstractApplication::VApp()->Settings()->GetAccountToken());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::IsLoggedIn() const -> bool
{
    return not Token().IsExpired();
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::StartDeviceLogin()
{
    StopPolling();
    m_issuer = QUrl(VAbstractApplication::VApp()->Settings()->GetAccountIssuerUrl());
    if (not m_issuer.isValid() || m_issuer.host().isEmpty())
    {
        emit Failed(tr("The account server address is not a valid URL."));
        return;
    }
    Discover();
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::CancelLogin()
{
    StopPolling();
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::Logout()
{
    StopPolling();
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    settings->SetAccountToken(QString());
    settings->SetAccountEmail(QString());
    settings->SetAccountName(QString());
    emit LoggedOut();
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::RefreshUserInfo()
{
    if (not IsLoggedIn())
    {
        emit Failed(tr("Not signed in."));
        return;
    }
    m_issuer = QUrl(VAbstractApplication::VApp()->Settings()->GetAccountIssuerUrl());
    QNetworkReply *reply = Get(DiscoveryUrl());
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply]()
            {
                reply->deleteLater();
                const QUrl userinfo = Endpoint(Json(reply), QStringLiteral("userinfo_endpoint"));
                if (not userinfo.isValid())
                {
                    return;
                }
                QNetworkReply *info = Get(userinfo, true);
                connect(info,
                        &QNetworkReply::finished,
                        this,
                        [this, info]()
                        {
                            info->deleteLater();
                            const int status = info->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                            if (status == 401)
                            {
                                Logout();
                                emit Failed(tr("Your session has expired. Please sign in again."));
                                return;
                            }
                            if (info->error() != QNetworkReply::NoError)
                            {
                                emit Failed(info->errorString());
                                return;
                            }
                            emit UserInfo(Json(info));
                        });
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::CheckService(const QString &baseUrl)
{
    const QUrl base(baseUrl);
    if (not base.isValid() || base.host().isEmpty())
    {
        emit ServiceChecked(false, tr("The service address is not a valid URL."));
        return;
    }
    QNetworkReply *reply = Get(QUrl(base.toString(QUrl::StripTrailingSlash) + "/health"_L1));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply]()
            {
                reply->deleteLater();
                if (reply->error() != QNetworkReply::NoError)
                {
                    emit ServiceChecked(false, reply->errorString());
                    return;
                }
                const bool ok = Json(reply).value("status"_L1).toString() == "ok"_L1;
                emit ServiceChecked(ok, ok ? tr("The service is reachable.") : tr("Unexpected reply from the service."));
            });
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::Get(const QUrl &url, bool bearer) -> QNetworkReply *
{
    if (bearer)
    {
        return m_qnam.get(VNestorClient::BearerRequest(url, Token().Raw()));
    }
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return m_qnam.get(request);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::PostForm(const QUrl &url, const QByteArray &form) -> QNetworkReply *
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    return m_qnam.post(request, form);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::Json(QNetworkReply *reply) -> QJsonObject
{
    return QJsonDocument::fromJson(reply->readAll()).object();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::Endpoint(const QJsonObject &discovery, const QString &key) -> QUrl
{
    const QUrl url(discovery.value(key).toString());
    if (not url.isValid() || not SameOrigin(m_issuer, url))
    {
        // never follow a discovery document to another host
        emit Failed(tr("The account server returned an invalid %1.").arg(key));
        return {};
    }
    return url;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAccountClient::DiscoveryUrl() const -> QUrl
{
    return QUrl(m_issuer.toString(QUrl::StripTrailingSlash) + "/.well-known/openid-configuration"_L1);
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::Discover()
{
    const int generation = m_generation;
    QNetworkReply *reply = Get(DiscoveryUrl());
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, generation]()
            {
                reply->deleteLater();
                if (generation != m_generation)
                {
                    return; // cancelled meanwhile
                }
                if (reply->error() != QNetworkReply::NoError)
                {
                    emit Failed(tr("Cannot reach the account server: %1").arg(reply->errorString()));
                    return;
                }
                const QJsonObject d = Json(reply);
                for (const auto &key : {QStringLiteral("device_authorization_endpoint"),
                                        QStringLiteral("token_endpoint"),
                                        QStringLiteral("userinfo_endpoint")})
                {
                    const QUrl url(d.value(key).toString());
                    if (not url.isValid() || not SameOrigin(m_issuer, url))
                    {
                        // never follow a discovery document to another host
                        emit Failed(tr("The account server returned an invalid %1.").arg(key));
                        return;
                    }
                }
                m_deviceEndpoint = QUrl(d.value(QStringLiteral("device_authorization_endpoint")).toString());
                m_tokenEndpoint = QUrl(d.value(QStringLiteral("token_endpoint")).toString());
                m_userinfoEndpoint = QUrl(d.value(QStringLiteral("userinfo_endpoint")).toString());
                RequestDeviceCode();
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::RequestDeviceCode()
{
    QUrlQuery form;
    form.addQueryItem(QStringLiteral("client_id"), clientId);
    form.addQueryItem(QStringLiteral("scope"), QStringLiteral("openid email profile premium"));
    const int generation = m_generation;
    QNetworkReply *reply = PostForm(m_deviceEndpoint, form.toString(QUrl::FullyEncoded).toLatin1());
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, generation]()
            {
                reply->deleteLater();
                if (generation != m_generation)
                {
                    return; // cancelled meanwhile: no code shown, no browser, no polling
                }
                const QJsonObject d = Json(reply);
                if (reply->error() != QNetworkReply::NoError || not d.contains("device_code"_L1))
                {
                    emit Failed(tr("The account server refused the sign-in request: %1").arg(reply->errorString()));
                    return;
                }
                m_deviceCode = d.value("device_code"_L1).toString();
                m_intervalS = qMax(1, d.value("interval"_L1).toInt(5));
                m_deadline = QDateTime::currentSecsSinceEpoch() + d.value("expires_in"_L1).toInt(600);
                const QString complete = d.value("verification_uri_complete"_L1).toString();
                const QUrl url(complete.isEmpty() ? d.value("verification_uri"_L1).toString() : complete);
                if (not url.isValid() || (url.scheme() != "https"_L1 && url.scheme() != "http"_L1))
                {
                    // the URL goes to QDesktopServices::openUrl: never hand the desktop a non-web scheme
                    emit Failed(tr("The account server returned an invalid verification address."));
                    return;
                }
                emit UserCodeReady(d.value("user_code"_L1).toString(), url);
                m_poll.start(m_intervalS * 1000);
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::Poll()
{
    if (m_deviceCode.isEmpty())
    {
        return;
    }
    if (QDateTime::currentSecsSinceEpoch() > m_deadline)
    {
        StopPolling();
        emit Failed(tr("The sign-in code expired before it was confirmed. Please try again."));
        return;
    }
    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"), grantDeviceCode);
    form.addQueryItem(QStringLiteral("device_code"), m_deviceCode);
    form.addQueryItem(QStringLiteral("client_id"), clientId);
    const QString code = m_deviceCode; // a reply for a stale code (cancel, restart) is ignored
    QNetworkReply *reply = PostForm(m_tokenEndpoint, form.toString(QUrl::FullyEncoded).toLatin1());
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, code]()
            {
                reply->deleteLater();
                if (code != m_deviceCode)
                {
                    return;
                }
                const QJsonObject d = Json(reply);
                const QString error = d.value("error"_L1).toString();
                if (d.contains("access_token"_L1))
                {
                    StopPolling();
                    const VAccountToken token(d.value("access_token"_L1).toString());
                    if (not token.IsValid())
                    {
                        emit Failed(tr("The account server returned an unreadable token."));
                        return;
                    }
                    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
                    settings->SetAccountToken(token.Raw());
                    settings->SetAccountEmail(token.Email());
                    settings->SetAccountName(token.Name());
                    emit LoggedIn();
                    return;
                }
                if (error == "authorization_pending"_L1 || error == "slow_down"_L1)
                {
                    if (error == "slow_down"_L1)
                    {
                        m_intervalS += 5; // RFC 8628 section 3.5
                    }
                    m_poll.start(m_intervalS * 1000);
                    return;
                }
                StopPolling();
                if (error == "access_denied"_L1)
                {
                    emit Failed(tr("The sign-in was refused in the browser."));
                }
                else if (error == "expired_token"_L1)
                {
                    emit Failed(tr("The sign-in code expired before it was confirmed. Please try again."));
                }
                else
                {
                    emit Failed(tr("Sign-in failed: %1").arg(error.isEmpty() ? reply->errorString() : error));
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VAccountClient::StopPolling()
{
    m_poll.stop();
    m_deviceCode.clear();
    ++m_generation;
}
