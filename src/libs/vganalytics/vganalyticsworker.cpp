/************************************************************************
 **
 **  @file   vganalyticsworker.cpp
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
#include "vganalyticsworker.h"
#include "vganalytics.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QGuiApplication>
#include <QScreen>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QLatin1String VGAnalyticsWorker::dateTimeFormat("yyyy,MM,dd-hh:mm::ss:zzz");

namespace
{
constexpr int fourHours = 4 * 60 * 60 * 1000;
}

//---------------------------------------------------------------------------------------------------------------------
VGAnalyticsWorker::VGAnalyticsWorker(QObject *parent)
  : QObject(parent),
    m_logLevel(VGAnalytics::Error)
{
    m_appName = QCoreApplication::applicationName();
    m_appVersion = QCoreApplication::applicationVersion();
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    m_request.setHeader(QNetworkRequest::UserAgentHeader, GetUserAgent());

    m_guiLanguage = QLocale::system().name().toLower().replace('_'_L1, '-'_L1);

    m_screensNumber = QString::number(QGuiApplication::screens().size());

    QScreen *screen = QGuiApplication::primaryScreen();
    QSize logicalSize = screen->size();
    qreal devicePixelRatio = screen->devicePixelRatio();
    m_screenPixelRatio = QString::number(devicePixelRatio);

    int screenWidth = qRound(logicalSize.width() * devicePixelRatio);
    int screenHeight = qRound(logicalSize.height() * devicePixelRatio);

    m_screenResolution = QStringLiteral("%1x%2").arg(screenWidth).arg(screenHeight);
    m_screenScaleFactor = screen->logicalDotsPerInchX() / 96.0;

    m_timer.setInterval(m_timerInterval);
    connect(&m_timer, &QTimer::timeout, this, &VGAnalyticsWorker::PostMessage);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalyticsWorker::Enable(bool state)
{
    // state change to the same is not valid.
    if (m_isEnabled == state)
    {
        return;
    }

    m_isEnabled = state;
    if (m_isEnabled)
    {
        // enable -> start doing things :)
        m_timer.start();
    }
    else
    {
        // disable -> stop the timer
        m_timer.stop();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalyticsWorker::LogMessage(enum VGAnalytics::LogLevel level, const QString &message) const
{
    if (m_logLevel > level)
    {
        return;
    }

    qDebug() << "[Analytics]" << message;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Try to gain information about the system where this application
 * is running. It needs to get the name and version of the operating
 * system, the language and screen resolution.
 * All this information will be send in POST messages.
 * @return agent        A QString with all the information formatted for a POST message.
 */
auto VGAnalyticsWorker::GetUserAgent() const -> QString
{
    return QStringLiteral("%1/%2").arg(m_appName, m_appVersion);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * The message queue contains a list of QueryBuffer object.
 * QueryBuffer holds a QUrlQuery object and a QDateTime object.
 * These both object are freed from the buffer object and
 * inserted as QString objects in a QList.
 * @return dataList     The list with concartinated queue data.
 */
auto VGAnalyticsWorker::PersistMessageQueue() -> QList<QString>
{
    QList<QString> dataList;
    dataList.reserve(m_messageQueue.size() * 2);

    for (const auto &buffer : m_messageQueue)
    {
        dataList << QJsonDocument(buffer.postQuery).toJson();
        dataList << buffer.time.toString(dateTimeFormat);
    }
    return dataList;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Reads persistent messages from a file.
 * Gets all message data as a QList<QString>.
 * Two lines in the list build a QueryBuffer object.
 */
void VGAnalyticsWorker::ReadMessagesFromFile(const QList<QString> &dataList)
{
    QListIterator<QString> iter(dataList);
    while (iter.hasNext())
    {
        QString queryString = iter.next();
        QString dateString = iter.next();
        QDateTime dateTime = QDateTime::fromString(dateString, dateTimeFormat);
        QueryBuffer buffer;

        QJsonDocument jsonDocument = QJsonDocument::fromJson(queryString.toUtf8());
        if (jsonDocument.isNull())
        {
            qDebug() << "===> please check the string " << queryString.toUtf8();
        }
        QJsonObject jsonObject = jsonDocument.object();

        buffer.postQuery = jsonObject;
        buffer.time = dateTime;
        m_messageQueue.enqueue(buffer);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Takes a QUrlQuery object and wrapp it together with
 * a QTime object into a QueryBuffer struct. These struct
 * will be stored in the message queue.
 */
void VGAnalyticsWorker::EnqueQueryWithCurrentTime(const QJsonObject &query)
{
    QueryBuffer buffer;
    buffer.postQuery = query;
    buffer.time = QDateTime::currentDateTime();

    m_messageQueue.enqueue(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * This function is called by a timer interval.
 * The function tries to send a messages from the queue.
 * If message was successfully send then this function
 * will be called back to send next message.
 * If message queue contains more than one message then
 * the connection will kept open.
 * The message POST is asyncroniously when the server
 * answered a signal will be emitted.
 */
auto VGAnalyticsWorker::PostMessage() -> QNetworkReply *
{
    if (m_messageQueue.isEmpty())
    {
        // queue empty -> try sending later
        m_timer.start();
        return nullptr;
    }

    // queue has messages -> stop timer and start sending
    m_timer.stop();

    QString connection = QStringLiteral("close");
    if (m_messageQueue.count() > 1)
    {
        connection = "keep-alive"_L1;
    }

    QueryBuffer buffer = m_messageQueue.head();
    QDateTime sendTime = QDateTime::currentDateTime();
    qint64 timeDiff = buffer.time.msecsTo(sendTime);

    if (timeDiff > fourHours)
    {
        // too old.
        m_messageQueue.dequeue();
        return PostMessage();
    }

    QByteArray requestJson = QJsonDocument(buffer.postQuery).toJson(QJsonDocument::Compact);
    m_request.setRawHeader("Connection", connection.toUtf8());
    m_request.setHeader(QNetworkRequest::ContentLengthHeader, requestJson.length());

    if (m_measurementId.isEmpty())
    {
        LogMessage(VGAnalytics::Error, QStringLiteral("google analytics measurement id was not set!"));
    }
    if (m_apiSecret.isEmpty())
    {
        LogMessage(VGAnalytics::Error, QStringLiteral("google analytics api seceret was not set!"));
    }

    QString requestUrl = QStringLiteral("https://www.google-analytics.com/mp/collect?measurement_id=%1&api_secret=%2");
    requestUrl = requestUrl.arg(m_measurementId, m_apiSecret);
    m_request.setUrl(QUrl(requestUrl));

    LogMessage(VGAnalytics::Debug, "GA Query string = " + requestJson);

    // Create a new network access manager if we don't have one yet
    if (networkManager == nullptr)
    {
        networkManager = new QNetworkAccessManager(this);
    }

    QNetworkReply *reply = networkManager->post(m_request, requestJson);
    connect(reply, &QNetworkReply::finished, this, &VGAnalyticsWorker::PostMessageFinished);
    return reply;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * NetworkAccsessManager has finished to POST a message.
 * If POST message was successfully send then the message
 * query should be removed from queue.
 * SIGNAL "postMessage" will be emitted to send next message
 * if there is any.
 * If message couldn't be send then next try is when the
 * timer emits its signal.
 */
void VGAnalyticsWorker::PostMessageFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());

    int httpStausCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStausCode < 200 || httpStausCode > 299)
    {
        LogMessage(VGAnalytics::Error, QStringLiteral("Error posting message: %1").arg(reply->errorString()));

        // An error ocurred. Try sending later.
        m_timer.start();
        return;
    }

    LogMessage(VGAnalytics::Debug, QStringLiteral("Message sent"));

    m_messageQueue.dequeue();
    PostMessage();
    reply->deleteLater();
}
