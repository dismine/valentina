/************************************************************************
 **
 **  @file   vpnestorclient.h
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

#ifndef VNESTORCLIENT_H
#define VNESTORCLIENT_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>

class QNetworkReply;

/** @brief Submits a nesting job to the online service and polls it until done, failed or cancelled. */
class VNestorClient : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VNestorClient(QObject *parent = nullptr);
    ~VNestorClient() override = default;

    void Submit(const QJsonObject &request);
    /** @brief Spec 17.1: files (name, content) and metadata to upload once the next Submit is accepted. */
    void SetFiles(const QList<QPair<QString, QByteArray>> &files, const QJsonObject &meta);
    /** @brief Spec 17.1: withdraws every file this account shared; emits FilesDeleted or Failed. */
    void DeleteMyFiles();
    /** @brief multipart/form-data body of PUT /jobs/{id}/files: `files` parts and a `meta` JSON part. */
    static auto NestingFilesBody(const QList<QPair<QString, QByteArray>> &files,
                                 const QJsonObject &meta,
                                 const QString &boundary) -> QByteArray;
    void Cancel();
    /** @brief Spec 15: asks the service to end the running job with its best layout; Finished follows. A job that
     * cannot be stopped (queued, lost) is cancelled instead, so Cancelled follows. */
    void Stop();
    /** @brief Reads /health: limits and prices (spec 12.1). Emits Info, or nothing if unreachable. */
    void FetchInfo();
    /** @brief Label for a progress dialog: queue position, or current length and utilisation while running. */
    static auto RunningProgress(const QJsonObject &body) -> QJsonObject;
    static auto ProgressText(const QString &status, int position, const QJsonObject &progress) -> QString;
    /** @brief End-of-job summary (spec 14): length, utilisation, rolls, time, tokens, why it stopped. */
    static auto SummaryText(const QJsonObject &result) -> QString;
    /** @brief Summary after a cancel, from the DELETE reply (may be empty). */
    static auto CancelSummaryText(const QJsonObject &summary) -> QString;

    /** @brief BearerRequest A request carrying the account token; redirects are not followed, they would leak it. */
    static auto BearerRequest(const QUrl &url, const QString &token) -> QNetworkRequest;

signals:
    void Progress(const QString &status, int position, const QJsonObject &progress);
    void Finished(const QJsonObject &result);
    void Failed(int httpStatus, const QString &message);
    void Info(const QJsonObject &info);
    /** @brief The service answered an explicit Cancel(): what the job cost so far (empty if nothing to tell). */
    void Cancelled(const QJsonObject &summary);
    void FilesDeleted();

private slots:
    void Poll();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VNestorClient) // NOLINT

    QNetworkAccessManager m_qnam{};
    QTimer m_poll{};
    QString m_jobId{};
    int m_run{0}; // bumped by Submit and Cancel: a reply from an older run is stale
    bool m_stopping{false};
    QList<QPair<QString, QByteArray>> m_files{};
    QJsonObject m_meta{};

    void DropJob(bool notify);
    void UploadFiles(const QString &jobId);
    static auto Request(const QString &path) -> QNetworkRequest;
    static auto Duration(qreal seconds) -> QString;
    static auto Body(const QByteArray &raw) -> QJsonObject;
    static auto ErrorText(QNetworkReply *reply, const QByteArray &raw) -> QString;
};

#endif // VNESTORCLIENT_H
