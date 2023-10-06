/************************************************************************
 **
 **  @file   vganalytics.h
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
#ifndef VGANALYTICS_H
#define VGANALYTICS_H

#include <QObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0)
#include <QLocale>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class QNetworkAccessManager;
class VGAnalyticsWorker;

// https://developers.google.com/analytics/devguides/collection/protocol/ga4/verify-implementation?client_type=gtag
class VGAnalytics : public QObject
{
    Q_OBJECT // NOLINT

public:
    ~VGAnalytics() override;

    static auto Instance() -> VGAnalytics *;

    enum LogLevel
    {
        Debug,
        Info,
        Error
    };

    Q_ENUM(LogLevel) // NOLINT

    void SetMeasurementId(const QString &measurementId);
    void SetApiSecret(const QString &apiSecret);
    void SetClientID(const QString &clientID);

    void SetLogLevel(LogLevel logLevel);
    auto LogLevel() const -> LogLevel;

    void SetRepoRevision(QString rev);
    auto RepoRevision() const -> QString;

    void SetGUILanguage(const QString &language);
    auto GUILanguage() const -> QString;

    void SetSendInterval(int milliseconds);
    auto SendInterval() const -> int;

    void Enable(bool state = true);
    auto IsEnabled() -> bool;

    /// Get or set the network access manager. If none is set, the class creates its own on the first request
    void SetNetworkAccessManager(QNetworkAccessManager *networkAccessManager);
    auto NetworkAccessManager() const -> QNetworkAccessManager *;

    static auto CountryCode() -> QString;

public slots:
    void SendAppFreshInstallEvent(qint64 engagementTimeMsec);
    void SendAppStartEvent(qint64 engagementTimeMsec);
    void SendAppCloseEvent(qint64 engagementTimeMsec);
    void SendPatternToolUsedEvent(qint64 engagementTimeMsec, const QString &toolName);
    void SendPatternFormatVersion(qint64 engagementTimeMsec, const QString &version);
    void SendIndividualMeasurementsFormatVersion(qint64 engagementTimeMsec, const QString &version);
    void SendMultisizeMeasurementsFormatVersion(qint64 engagementTimeMsec, const QString &version);
    void SendLayoutFormatVersion(qint64 engagementTimeMsec, const QString &version);

private:
    Q_DISABLE_COPY_MOVE(VGAnalytics) // NOLINT

    explicit VGAnalytics(QObject *parent = nullptr);

private:
    VGAnalyticsWorker *d;

    void SendEvent(const QString &eventName, const QHash<QString, QJsonValue> &params);

    auto InitAppStartEventParams(qint64 engagementTimeMsec) const -> QHash<QString, QJsonValue>;

    friend auto operator<<(QDataStream &outStream, const VGAnalytics &analytics) -> QDataStream &;
    friend auto operator>>(QDataStream &inStream, VGAnalytics &analytics) -> QDataStream &;
};

#endif // VGANALYTICS_H
