/************************************************************************
 **
 **  @file   vganalytics.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vganalytics.h"
#include "qguiapplication.h"
#include "qstringliteral.h"
#include "vganalyticsworker.h"

#include <QDataStream>
#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>

//---------------------------------------------------------------------------------------------------------------------
VGAnalytics::VGAnalytics(QObject *parent)
  : QObject{parent},
    d(new VGAnalyticsWorker(this))
{
}

//---------------------------------------------------------------------------------------------------------------------
VGAnalytics::~VGAnalytics()
{
    delete d;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::Instance() -> VGAnalytics *
{
    static VGAnalytics *instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    if (instance == nullptr)
    {
        instance = new VGAnalytics();
    }

    return instance;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetLogLevel(enum VGAnalytics::LogLevel logLevel)
{
    d->m_logLevel = logLevel;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::LogLevel() const -> enum VGAnalytics::LogLevel { return d->m_logLevel; }

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetRepoRevision(const QString &rev)
{
    d->m_repoRevision = rev;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::RepoRevision() const -> QString
{
    return d->m_repoRevision;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetGUILanguage(const QString &language)
{
    d->m_guiLanguage = language.toLower().replace(QChar('_'), QChar('-'));
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::GUILanguage() const -> QString
{
    return d->m_guiLanguage;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetSendInterval(int milliseconds)
{
    d->m_timer.setInterval(milliseconds);
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::SendInterval() const -> int
{
    return (d->m_timer.interval());
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::IsEnabled() -> bool
{
    return d->m_isEnabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetMeasurementId(const QString &measurementId)
{
    d->m_measurementId = measurementId;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetApiSecret(const QString &apiSecret)
{
    d->m_apiSecret = apiSecret;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetClientID(const QString &clientID)
{
    d->m_clientID = clientID;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::Enable(bool state)
{
    d->Enable(state);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    if (d->networkManager != networkAccessManager)
    {
        // Delete the old network manager if it was our child
        if (d->networkManager && d->networkManager->parent() == this)
        {
            d->networkManager->deleteLater();
        }

        d->networkManager = networkAccessManager;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::NetworkAccessManager() const -> QNetworkAccessManager *
{
    return d->networkManager;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendAppFreshInstallEvent(qint64 engagementTimeMsec)
{
    SendEvent(QStringLiteral("vapp_fresh_install"), InitAppStartEventParams(engagementTimeMsec));
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendAppStartEvent(qint64 engagementTimeMsec)
{
    SendEvent(QStringLiteral("vapp_start"), InitAppStartEventParams(engagementTimeMsec));
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendAppCloseEvent(qint64 engagementTimeMsec)
{
    QHash<QString, QJsonValue> params{
        // {QStringLiteral("category"), ""},
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
    };

    SendEvent(QStringLiteral("vapp_close"), params);
    QNetworkReply *reply = d->PostMessage();
    if (reply)
    {
        QTimer timer;
        const int timeoutSeconds = 3; // Wait for 3 seconds
        timer.setSingleShot(true);
        timer.start(timeoutSeconds * 1000);

        QEventLoop loop;
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec(); // wait for finished
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * A query for a POST message will be created to report this event. The
 * created query will be stored in a message queue.
 */
void VGAnalytics::SendEvent(const QString &eventName, const QHash<QString, QJsonValue> &params)
{
    /*
    // event body example
    {
        "client_id": "XXXXXXXXXX.YYYYYYYYYY",
        "events": [{
            "name": "refund",
            "params": {
                "currency": "USD",
                "value": "9.99",
                "transaction_id": "ABC-123"
            }
        }]
    }
    */

    QJsonObject root;
    root[QStringLiteral("client_id")] = d->m_clientID;

    QJsonObject event;
    event[QStringLiteral("name")] = eventName;

    QJsonObject eventParams;

    auto i = params.constBegin();
    while (i != params.constEnd())
    {
        eventParams[i.key()] = i.value();
        ++i;
    }

    // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id must
    // be supplied as part of the params for an event.
    // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    eventParams[QStringLiteral("session_id")] = qApp->sessionId();
    event[QStringLiteral("params")] = eventParams;

    QJsonArray eventArray;
    eventArray.append(event);
    root[QStringLiteral("events")] = eventArray;

    d->EnqueQueryWithCurrentTime(root);
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::InitAppStartEventParams(qint64 engagementTimeMsec) const -> QHash<QString, QJsonValue>
{

    QHash<QString, QJsonValue> params{
        // {QStringLiteral("category"), ""},
        {QStringLiteral("qt_version"), QT_VERSION_STR},
        {QStringLiteral("app_version"), d->m_appVersion},
        {QStringLiteral("word_size"), QStringLiteral("%1 bit").arg(QSysInfo::WordSize)},
        {QStringLiteral("cpu_architecture"), QSysInfo::currentCpuArchitecture()},
        {QStringLiteral("revision"), d->m_repoRevision},
        {QStringLiteral("os_version"), QSysInfo::prettyProductName()},
        {QStringLiteral("screen_size"), d->m_screenResolution},
        {QStringLiteral("screen_scale_factor"), d->m_screenScaleFactor},
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("gui_language"), d->m_guiLanguage},
    };
    return params;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Qut stream to persist class GAnalytics.
 */
auto operator<<(QDataStream &outStream, const VGAnalytics &analytics) -> QDataStream &
{
    outStream << analytics.d->PersistMessageQueue();

    return outStream;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * In stream to read GAnalytics from file.
 */
auto operator>>(QDataStream &inStream, VGAnalytics &analytics) -> QDataStream &
{
    QList<QString> dataList;
    inStream >> dataList;
    analytics.d->ReadMessagesFromFile(dataList);

    return inStream;
}
