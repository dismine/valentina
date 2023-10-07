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
#include "../vmisc/defglobal.h"
#include "vganalyticsworker.h"

#include <QDataStream>
#include <QDebug>
#include <QGuiApplication>
#include <QHash>
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
#include <QString>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/mach_host.h>
#elif defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#endif

#if (defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)
// DO NOT WORK WITH GCC 4.8
#else
#if __cplusplus >= 201402L
using namespace std::chrono_literals;
#else
#include "../vmisc/bpstd/chrono.hpp"
using namespace bpstd::literals::chrono_literals;
#endif // __cplusplus >= 201402L
#endif //(defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{

//---------------------------------------------------------------------------------------------------------------------
auto GetSystemMemorySize() -> qint64
{
#ifdef Q_OS_WIN
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memory_status))
    {
        return static_cast<qint64>(memory_status.ullTotalPhys);
    }

    return -1;
#elif defined(Q_OS_MACOS)
    vm_size_t pageSize;
    vm_statistics64_data_t vmStats;

    mach_port_t machPort = mach_host_self();
    mach_msg_type_number_t count = sizeof(vmStats) / sizeof(natural_t);
    host_page_size(machPort, &pageSize);

    host_statistics64(machPort, HOST_VM_INFO, reinterpret_cast<host_info64_t>(&vmStats), &count);

    qulonglong freeMemory = static_cast<qulonglong>(vmStats.free_count) * static_cast<qulonglong>(pageSize);
    qulonglong totalMemoryUsed =
        (static_cast<qulonglong>(vmStats.active_count) + static_cast<qulonglong>(vmStats.inactive_count) +
         static_cast<qulonglong>(vmStats.wire_count)) *
        static_cast<qulonglong>(pageSize);
    return static_cast<qint64>(freeMemory + totalMemoryUsed);
#elif defined(Q_OS_LINUX)
    struct sysinfo info; // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    sysinfo(&info);
    return static_cast<qint64>(info.totalram) * static_cast<qint64>(info.mem_unit);
#else
    // Unsupported platform
    return -1;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto GetSystemMemorySizeGB() -> double
{
    qint64 totalMemoryBytes = GetSystemMemorySize();
    if (totalMemoryBytes != -1)
    {
        return static_cast<double>(totalMemoryBytes) / (1024 * 1024 * 1024); // Convert bytes to gigabytes
    }

    return -1.0; // Unsupported platform or error
}

//---------------------------------------------------------------------------------------------------------------------
auto TotalMemory() -> QString
{
    double size = GetSystemMemorySizeGB();
    return !qFuzzyCompare(size, -1.0) ? QStringLiteral("%1 GB").arg(size) : QStringLiteral("Unknown RAM");
}
} // namespace

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
void VGAnalytics::SetRepoRevision(QString rev)
{
    d->m_repoRevision = !rev.isEmpty() ? rev.remove(QStringLiteral("Git:")) : QStringLiteral("Unknown");
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::RepoRevision() const -> QString
{
    return d->m_repoRevision;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetGUILanguage(const QString &language)
{
    d->m_guiLanguage = language.toLower().replace('_'_L1, '-'_L1);
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
void VGAnalytics::SendPatternToolUsedEvent(qint64 engagementTimeMsec, const QString &toolName)
{
    QHash<QString, QJsonValue> params{
        // {QStringLiteral("category"), ""},
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("tool_name"), toolName},
    };

    SendEvent(QStringLiteral("vapp_pattern_tool_used"), params);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendPatternFormatVersion(qint64 engagementTimeMsec, const QString &version)
{
    QHash<QString, QJsonValue> params{
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("format_version"), version},
    };

    SendEvent(QStringLiteral("vapp_pattern_format_version"), params);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendIndividualMeasurementsFormatVersion(qint64 engagementTimeMsec, const QString &version)
{
    QHash<QString, QJsonValue> params{
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("format_version"), version},
    };

    SendEvent(QStringLiteral("vapp_individual_measurements_format_version"), params);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendMultisizeMeasurementsFormatVersion(qint64 engagementTimeMsec, const QString &version)
{
    QHash<QString, QJsonValue> params{
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("format_version"), version},
    };

    SendEvent(QStringLiteral("vapp_multisize_measurements_format_version"), params);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendLayoutFormatVersion(qint64 engagementTimeMsec, const QString &version)
{
    QHash<QString, QJsonValue> params{
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("format_version"), version},
    };

    SendEvent(QStringLiteral("vapp_layout_format_version"), params);
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
        {QStringLiteral("screens_number"), d->m_screensNumber},
        {QStringLiteral("screen_pixel_ratio"), d->m_screenPixelRatio},
        {QStringLiteral("screen_size"), d->m_screenResolution},
        {QStringLiteral("screen_scale_factor"), d->m_screenScaleFactor},
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and
        // session_id must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("gui_language"), d->m_guiLanguage},
        {QStringLiteral("country_code"), CountryCode()},
        {QStringLiteral("kernel_type"), QSysInfo::kernelType()},
        {QStringLiteral("total_memory"), TotalMemory()},
    };
    return params;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::CountryCode() -> QString
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("https://api.country.is")));
    QNetworkReply *reply = manager.get(request);

    QTimer timer;
    timer.setSingleShot(true);
    timer.start(V_SECONDS(5)); // Set the timeout to 5 seconds

    QEventLoop eventLoop;

    QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);

    eventLoop.exec();

    QString country = QStringLiteral("Unknown");

    if (timer.isActive())
    {
        // The API response was received before the timeout
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray responseData = reply->readAll();
            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &error);

            if (error.error == QJsonParseError::NoError && jsonDoc.isObject())
            {
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains(QStringLiteral("country")))
                {
                    country = jsonObj[QStringLiteral("country")].toString().toLower();
                }
            }
        }
        else
        {
            qDebug() << "Error:" << reply->errorString();
        }
    }
    else
    {
        // Timeout occurred
        qDebug() << "Request timed out";
        reply->abort();
    }

    // Clean up the reply
    reply->deleteLater();

    return country;
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
