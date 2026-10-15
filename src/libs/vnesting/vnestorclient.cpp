/************************************************************************
 **
 **  @file   vpnestorclient.cpp
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

#include "vnestorclient.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QtMath>

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int pollMs = 2000;
}

//---------------------------------------------------------------------------------------------------------------------
VNestorClient::VNestorClient(QObject *parent)
  : QObject(parent)
{
    m_poll.setSingleShot(true);
    connect(&m_poll, &QTimer::timeout, this, &VNestorClient::Poll);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::Submit(const QJsonObject &request)
{
    DropJob(false);
    const int run = ++m_run;
    QNetworkRequest req = Request(QStringLiteral("/jobs"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = m_qnam.post(req, QJsonDocument(request).toJson(QJsonDocument::Compact));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, run]()
            {
                reply->deleteLater();
                const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                const QByteArray raw = reply->readAll();
                const QJsonObject body = Body(raw);
                if (run != m_run)
                {
                    // cancelled while the POST was in flight: the job exists on the server, drop it there too
                    if (status == 202)
                    {
                        QNetworkReply *drop = m_qnam.deleteResource(
                            Request(QStringLiteral("/jobs/") + body.value("id"_L1).toString()));
                        connect(drop, &QNetworkReply::finished, drop, &QNetworkReply::deleteLater);
                    }
                    return;
                }
                if (status != 202)
                {
                    emit Failed(status, ErrorText(reply, raw));
                    return;
                }
                m_jobId = body.value("id"_L1).toString();
                UploadFiles(m_jobId);
                emit Progress(body.value("status"_L1).toString(),
                              body.value("position"_L1).toInt(),
                              QJsonObject{{QStringLiteral("eta_s"), body.value("eta_s"_L1)}});
                m_poll.start(pollMs);
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::SetFiles(const QList<QPair<QString, QByteArray>> &files, const QJsonObject &meta)
{
    m_files = files;
    m_meta = meta;
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::UploadFiles(const QString &jobId)
{
    if (m_files.isEmpty())
    {
        return;
    }
    const QString boundary = QStringLiteral("nestor-") + QString::number(QRandomGenerator::global()->generate64(), 36);
    QNetworkRequest req = Request(QStringLiteral("/jobs/") + jobId + QStringLiteral("/files"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("multipart/form-data; boundary=") + boundary);
    QNetworkReply *reply = m_qnam.put(req, NestingFilesBody(m_files, m_meta, boundary));
    m_files.clear();
    m_meta = QJsonObject();
    // best effort: the job does not depend on it (spec 17.1)
    connect(reply,
            &QNetworkReply::finished,
            reply,
            [reply]()
            {
                if (reply->error() != QNetworkReply::NoError)
                {
                    qWarning() << "nestor: file upload failed:" << reply->errorString();
                }
                reply->deleteLater();
            });
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::NestingFilesBody(const QList<QPair<QString, QByteArray>> &files,
                                     const QJsonObject &meta,
                                     const QString &boundary) -> QByteArray
{
    QByteArray body;
    const QByteArray dash = "--" + boundary.toLatin1();
    for (const auto &file : files)
    {
        QString name = QFileInfo(file.first).fileName();
        name.remove('"');
        body += dash + "\r\nContent-Disposition: form-data; name=\"files\"; filename=\"" + name.toUtf8()
                + "\"\r\nContent-Type: application/octet-stream\r\n\r\n" + file.second + "\r\n";
    }
    body += dash + "\r\nContent-Disposition: form-data; name=\"meta\"\r\n\r\n"
            + QJsonDocument(meta).toJson(QJsonDocument::Compact) + "\r\n";
    body += dash + "--\r\n";
    return body;
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::DeleteMyFiles()
{
    QNetworkReply *reply = m_qnam.deleteResource(Request(QStringLiteral("/me/files")));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply]()
            {
                reply->deleteLater();
                const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if (status == 204)
                {
                    emit FilesDeleted();
                }
                else
                {
                    emit Failed(status, ErrorText(reply, reply->readAll()));
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::Cancel()
{
    DropJob(true);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::Stop()
{
    if (m_jobId.isEmpty() || m_stopping)
    {
        Cancel();
        return;
    }
    m_stopping = true;
    const QString job = m_jobId;
    QNetworkReply *reply = m_qnam.post(Request(QStringLiteral("/jobs/") + job + QStringLiteral("/stop")), QByteArray());
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, job]()
            {
                reply->deleteLater();
                if (job == m_jobId && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
                {
                    Cancel(); // not running (still queued, or already ending): nothing to keep
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::DropJob(bool notify)
{
    m_poll.stop();
    ++m_run;
    m_stopping = false;
    if (m_jobId.isEmpty())
    {
        if (notify)
        {
            emit Cancelled(QJsonObject()); // nothing submitted yet (POST in flight): the summary has nothing to tell
        }
        return;
    }
    QNetworkReply *reply = m_qnam.deleteResource(Request(QStringLiteral("/jobs/") + m_jobId));
    m_jobId.clear();
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, notify]()
            {
                reply->deleteLater();
                if (notify)
                {
                    // 200 carries the cost of a queued or running job, 204 (purge) or an error carries nothing
                    emit Cancelled(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200
                                       ? Body(reply->readAll())
                                       : QJsonObject());
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::Duration(qreal seconds) -> QString
{
    const int whole = qRound(seconds);
    return QStringLiteral("%1:%2").arg(whole / 60).arg(whole % 60, 2, 10, QLatin1Char('0'));
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::SummaryText(const QJsonObject &result) -> QString
{
    const QLocale locale;
    QStringList lines{tr("Marker length %1 m, utilisation %2 %.")
                          .arg(locale.toString(result.value("marker_length_mm"_L1).toDouble() / 1000, 'f', 2),
                               locale.toString(result.value("utilisation"_L1).toDouble() * 100, 'f', 1))};
    const int rolls = static_cast<int>(result.value("rolls"_L1).toArray().size());
    if (rolls > 1)
    {
        lines << tr("Laid on %n roll(s).", "", rolls);
    }
    const QString stopped = result.value("stopped"_L1).toString();
    const QString why = stopped == "target"_L1      ? tr("the target length was reached")
                        : stopped == "converged"_L1 ? tr("no further improvement was found")
                        : stopped == "user"_L1      ? tr("you stopped it")
                        : stopped == "abandoned"_L1 ? tr("the service stopped it after losing contact with this app")
                                                    : tr("the time budget was used");
    lines << tr("Time %1, %2.").arg(Duration(result.value("elapsed_s"_L1).toDouble()), why);
    lines << tr("%n token(s) used.", "", result.value("tokens"_L1).toInt());
    return lines.join('\n');
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::CancelSummaryText(const QJsonObject &summary) -> QString
{
    if (summary.isEmpty())
    {
        return tr("The job was cancelled.");
    }
    const QLocale locale;
    QStringList lines{tr("The job was cancelled after %1.").arg(Duration(summary.value("elapsed_s"_L1).toDouble()))};
    const QJsonObject progress = summary.value("progress"_L1).toObject();
    if (not progress.isEmpty())
    {
        lines << tr("Last result: %1 m at %2 % utilisation.")
                     .arg(locale.toString(progress.value("marker_length_mm"_L1).toDouble() / 1000, 'f', 2),
                          locale.toString(progress.value("utilisation"_L1).toDouble() * 100, 'f', 1));
    }
    lines << tr("%n token(s) used.", "", summary.value("tokens"_L1).toInt());
    return lines.join('\n');
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::RunningProgress(const QJsonObject &body) -> QJsonObject
{
    // the service's progress point plus the bill so far (spec 20), for ProgressText
    QJsonObject progress = body.value("progress"_L1).toObject();
    progress.insert(QStringLiteral("tokens"), body.value("tokens"_L1));
    if (body.value("budget_tokens"_L1).toInt() > 0)
    {
        progress.insert(QStringLiteral("budget_tokens"), body.value("budget_tokens"_L1));
    }
    return progress;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::ProgressText(const QString &status, int position, const QJsonObject &progress) -> QString
{
    if (status == "stopping"_L1)
    {
        return tr("Stopping, waiting for the final layout…");
    }
    if (status != "running"_L1)
    {
        const QString base = tr("Waiting in the service queue (position %1)…").arg(position + 1);
        const QJsonObject eta = progress.value("eta_s"_L1).toObject(); // spec 16.2
        if (eta.isEmpty())
        {
            return base;
        }
        const int lo = qCeil(eta.value("min"_L1).toDouble() / 60.0);
        const int hi = qCeil(eta.value("max"_L1).toDouble() / 60.0);
        if (hi <= 1)
        {
            return base + ' ' + tr("Starting in less than a minute.");
        }
        if (lo == hi)
        {
            return base + ' ' + tr("Starting in about %n minute(s).", "", hi);
        }
        return base + ' ' + tr("Starting in %1 to %2 minutes.").arg(lo).arg(hi);
    }
    const QLocale locale;
    QString text = progress.contains("marker_length_mm"_L1)
                       ? tr("Nesting in progress… current length %1 m, utilisation %2 %")
                             .arg(locale.toString(progress.value("marker_length_mm"_L1).toDouble() / 1000, 'f', 2),
                                  locale.toString(progress.value("utilisation"_L1).toDouble() * 100, 'f', 1))
                       : tr("Nesting in progress…");
    if (progress.contains("tokens"_L1))
    {
        const int tokens = progress.value("tokens"_L1).toInt();
        const int budget = progress.value("budget_tokens"_L1).toInt();
        text += ' '
                + (budget > 0 ? tr("%1 of %2 tokens spent.").arg(tokens).arg(budget)
                              : tr("%n token(s) spent.", "", tokens));
    }
    return text;
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::FetchInfo()
{
    QNetworkReply *reply = m_qnam.get(Request(QStringLiteral("/health")));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply]()
            {
                reply->deleteLater();
                if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)
                {
                    emit Info(Body(reply->readAll()));
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VNestorClient::Poll()
{
    if (m_jobId.isEmpty())
    {
        return;
    }
    const QString job = m_jobId; // a reply for a cancelled or superseded job is ignored
    QNetworkReply *reply = m_qnam.get(Request(QStringLiteral("/jobs/") + job));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, job]()
            {
                reply->deleteLater();
                if (job != m_jobId)
                {
                    return;
                }
                const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                const QByteArray raw = reply->readAll();
                if (status != 200)
                {
                    m_jobId.clear();
                    emit Failed(status,
                                status == 404
                                    ? tr("The service lost the job (it may have restarted). Please try again.")
                                    : ErrorText(reply, raw));
                    return;
                }
                const QJsonObject body = Body(raw);
                const QString state = body.value("status"_L1).toString();
                if (state == "done"_L1)
                {
                    m_jobId.clear();
                    emit Finished(body.value("result"_L1).toObject());
                }
                else if (state == "failed"_L1)
                {
                    m_jobId.clear();
                    emit Failed(200, body.value("error"_L1).toString());
                }
                else
                {
                    emit Progress(m_stopping ? QStringLiteral("stopping") : state,
                                  body.value("position"_L1).toInt(-1),
                                  state == "queued"_L1 ? QJsonObject{{QStringLiteral("eta_s"), body.value("eta_s"_L1)}}
                                                       : RunningProgress(body));
                    m_poll.start(pollMs);
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::Request(const QString &path) -> QNetworkRequest
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    return BearerRequest(QUrl(QUrl(settings->GetNestorServiceUrl()).toString(QUrl::StripTrailingSlash) + path),
                         settings->GetAccountToken());
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::BearerRequest(const QUrl &url, const QString &token) -> QNetworkRequest
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
    request.setRawHeader("Authorization", "Bearer " + token.toLatin1());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    return request;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::Body(const QByteArray &raw) -> QJsonObject
{
    return QJsonDocument::fromJson(raw).object();
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestorClient::ErrorText(QNetworkReply *reply, const QByteArray &raw) -> QString
{
    // nestor answers errors with a short text or {"detail": "..."}; fall back to Qt's description
    const QString detail = QJsonDocument::fromJson(raw).object().value("detail"_L1).toString();
    if (not detail.isEmpty())
    {
        return detail;
    }
    return raw.isEmpty() ? reply->errorString() : QString::fromUtf8(raw);
}
