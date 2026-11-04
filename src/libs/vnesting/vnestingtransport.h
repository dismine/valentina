/************************************************************************
 **
 **  @file   vnestingtransport.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 October, 2026
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

#ifndef VNESTINGTRANSPORT_H
#define VNESTINGTRANSPORT_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class QProcess;
class QTimer;
class VNestingHttpClient;

/**
 * Carries one version 1 job to the engine the user chose, and returns one version 1 result.
 * A remote nesting API uses the HTTP client. A local executable reads the job on stdin and writes the result on stdout.
 * The job bytes are the same either way.
 */
class VNestingTransport : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VNestingTransport(QObject *parent = nullptr);
    ~VNestingTransport() override = default;

    /** @brief Use this engine instead of the one stored in settings. */
    void SetEngine(bool localExecutable, const QString &httpUrl, const QString &executable);

    void Submit(const QJsonObject &job);
    void Cancel();

signals:
    void Progress(const QString &status, int position, const QJsonObject &progress);
    void Finished(const QJsonObject &result);
    void Failed(int httpStatus, const QString &message);
    void Cancelled(const QJsonObject &summary);

private:
    Q_DISABLE_COPY_MOVE(VNestingTransport) // NOLINT

    VNestingHttpClient *m_http;
    QProcess *m_process{nullptr};
    QTimer *m_clock{nullptr};
    bool m_local{false};
    bool m_override{false};
    bool m_killedByBudget{false};
    QString m_httpUrl{};
    QString m_executable{};
    int m_run{0};
    int m_elapsed{0};
    qreal m_budgetS{0};

    void SubmitHttp(const QJsonObject &job);
    void SubmitExecutable(const QJsonObject &job, qreal timeBudgetS);
    void ReadEngine();
    void Tick();
};

#endif // VNESTINGTRANSPORT_H
