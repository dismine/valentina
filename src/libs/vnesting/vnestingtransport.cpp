/************************************************************************
 **
 **  @file   vnestingtransport.cpp
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

#include "vnestingtransport.h"

#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "vnestinghttpclient.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QProcess>
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VNestingTransport::VNestingTransport(QObject *parent)
  : QObject(parent),
    m_http(new VNestingHttpClient(this)),
    m_clock(new QTimer(this))
{
    m_clock->setInterval(1000);
    connect(m_http, &VNestingHttpClient::Progress, this, &VNestingTransport::Progress);
    connect(m_http, &VNestingHttpClient::Finished, this, &VNestingTransport::Finished);
    connect(m_http, &VNestingHttpClient::Failed, this, &VNestingTransport::Failed);
    connect(m_http, &VNestingHttpClient::Cancelled, this, &VNestingTransport::Cancelled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::SetEngine(bool localExecutable, const QString &httpUrl, const QString &executable,
                                  const QStringList &arguments)
{
    m_override = true;
    m_local = localExecutable;
    m_httpUrl = httpUrl;
    m_executable = executable;
    m_arguments = arguments;
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::Submit(const QJsonObject &job)
{
    const qreal budget = job.value("timeBudgetS"_L1).toDouble();
    if (budget <= 0)
    {
        emit Failed(0, tr("The nesting job needs a positive time budget in seconds."));
        return;
    }

    bool local = m_local;
    QString executable = m_executable;
    QString httpUrl = m_httpUrl;
    QStringList arguments = m_arguments;
    if (not m_override)
    {
        const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        local = settings->IsNestingLocalEngine();
        executable = settings->GetNestingExecutable();
        httpUrl = settings->GetNestingServiceUrl();
        arguments.clear();
    }
    if (local)
    {
        m_executable = executable;
        SubmitExecutable(job, budget, arguments);
        return;
    }
    if (httpUrl.trimmed().isEmpty())
    {
        emit Failed(0, tr("Choose a nesting engine: a remote nesting API or a local program, on the Nesting page."));
        return;
    }
    m_httpUrl = httpUrl;
    SubmitHttp(job);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::Cancel()
{
    ++m_run;
    m_clock->stop();
    if (m_process != nullptr && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
        emit Cancelled(QJsonObject());
        return;
    }
    m_http->Cancel();
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::SubmitHttp(const QJsonObject &job)
{
    // The URL is the user's. The body is the job, with no service field added here.
    m_http->SetBaseUrl(m_httpUrl);
    m_http->Submit(job);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::SubmitExecutable(const QJsonObject &job, qreal timeBudgetS, const QStringList &arguments)
{
    ++m_run;
    m_clock->stop();
    if (m_process != nullptr)
    {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
    const QString program = m_executable.trimmed();
    if (program.isEmpty() || not QFileInfo::exists(program))
    {
        emit Failed(0, tr("Choose a local nesting program in Preferences, on the Nesting page."));
        return;
    }

    const int run = m_run;
    m_killedByBudget = false;
    m_elapsed = 0;
    m_budgetS = timeBudgetS;
    m_process = new QProcess(this);
    m_process->setProgram(program);
    // CreateProcess does not run a shebang script. A stand-in passes the interpreter here and the script in arguments.
    m_process->setArguments(arguments);
    const QByteArray bytes = QJsonDocument(job).toJson(QJsonDocument::Compact);
    connect(m_process, &QProcess::started, this, [this, bytes, run]()
            {
                if (run != m_run || m_process == nullptr)
                {
                    return;
                }
                m_process->write(bytes);
                m_process->closeWriteChannel();
            });
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, run](int, QProcess::ExitStatus)
            {
                if (run != m_run)
                {
                    return;
                }
                ReadEngine();
            });
    connect(m_process, &QProcess::errorOccurred, this, [this, run](QProcess::ProcessError error)
            {
                if (run != m_run || error != QProcess::FailedToStart)
                {
                    return;
                }
                m_clock->stop();
                emit Failed(0, tr("The nesting program did not start."));
            });

    connect(m_clock, &QTimer::timeout, this, &VNestingTransport::Tick, Qt::UniqueConnection);
    m_clock->start();
    emit Progress(QStringLiteral("running"), -1, QJsonObject{{QStringLiteral("elapsed_s"), 0}});
    m_process->start();
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::Tick()
{
    if (m_process == nullptr)
    {
        return;
    }
    ++m_elapsed;
    emit Progress(QStringLiteral("running"), -1, QJsonObject{{QStringLiteral("elapsed_s"), m_elapsed}});
    // The engine is required to stop by itself. This only frees the window if it does not.
    if (m_elapsed > m_budgetS + 2 && m_process->state() != QProcess::NotRunning)
    {
        m_killedByBudget = true;
        m_process->kill();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingTransport::ReadEngine()
{
    m_clock->stop();
    if (m_process == nullptr)
    {
        return;
    }
    QProcess *process = m_process;
    m_process = nullptr;
    const QByteArray out = process->readAllStandardOutput();
    const QByteArray err = process->readAllStandardError();
    const int code = process->exitCode();
    process->deleteLater();
    if (m_killedByBudget)
    {
        emit Failed(0, tr("The engine did not return within the time budget."));
        return;
    }
    const QJsonObject result = QJsonDocument::fromJson(out).object();
    const QString status = result.value(QStringLiteral("status")).toString();
    if (status == QLatin1String("complete") || status == QLatin1String("incomplete"))
    {
        emit Finished(result);
        return;
    }
    const QString detail = QString::fromUtf8(err.isEmpty() ? out : err).trimmed();
    emit Failed(code, detail.isEmpty() ? tr("The engine did not return a version 1 result.") : detail);
}
