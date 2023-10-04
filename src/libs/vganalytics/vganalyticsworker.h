/************************************************************************
 **
 **  @file   vganalyticsworker.h
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
#ifndef VGANALYTICSWORKER_H
#define VGANALYTICSWORKER_H

#include "vganalytics.h"

#include <QDateTime>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <QUrlQuery>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class QNetworkReply;

struct QueryBuffer
{
    QJsonObject postQuery{};
    QDateTime time{};
};

class VGAnalyticsWorker : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VGAnalyticsWorker(QObject *parent = nullptr);
    ~VGAnalyticsWorker() override = default;

    QNetworkAccessManager *networkManager{nullptr}; // NOLINT(misc-non-private-member-variables-in-classes)

    QQueue<QueryBuffer> m_messageQueue{};                      // NOLINT(misc-non-private-member-variables-in-classes)
    QTimer m_timer{};                                          // NOLINT(misc-non-private-member-variables-in-classes)
    QNetworkRequest m_request{};                               // NOLINT(misc-non-private-member-variables-in-classes)
    enum VGAnalytics::LogLevel m_logLevel{VGAnalytics::Debug}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_measurementId{};                         // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_apiSecret{};                             // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_clientID{};                              // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_userID{};                                // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_appName{};                               // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_appVersion{};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_guiLanguage{};                           // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_screensNumber{};                         // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_screenPixelRatio{};                      // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_screenResolution{QStringLiteral("0x0")}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_repoRevision{QStringLiteral("Unknown")}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool m_isEnabled{false};      // NOLINT(misc-non-private-member-variables-in-classes)
    int m_timerInterval{30000};   // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_screenScaleFactor{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    const static QLatin1String dateTimeFormat;

    void LogMessage(enum VGAnalytics::LogLevel level, const QString &message) const;

    auto GetUserAgent() const -> QString;
    auto PersistMessageQueue() -> QList<QString>;
    void ReadMessagesFromFile(const QList<QString> &dataList);

    void EnqueQueryWithCurrentTime(const QJsonObject &query);
    void Enable(bool state);

public slots:
    QNetworkReply *PostMessage(); // NOLINT(modernize-use-trailing-return-type)
    void PostMessageFinished();

private:
    Q_DISABLE_COPY_MOVE(VGAnalyticsWorker) // NOLINT
};

#endif // VGANALYTICSWORKER_H
