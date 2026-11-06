/************************************************************************
 **
 **  @file   tst_vnestinghttpclient.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 October, 2026
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
#include "tst_vnestinghttpclient.h"
#include "../vlayout/vnestingframe.h"
#include "../vnesting/vcirculargauge.h"
#include "../vnesting/vnestingprogressdialog.h"
#include "../vnesting/vnestingtransport.h"
#include "../vnesting/vnestinghttpclient.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VNestingHttpClient::TST_VNestingHttpClient(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::SummaryOfFinishedJob()
{
    const QJsonObject result{{"marker_length_mm", 2345.6},
                             {"utilisation", 0.8125},
                             {"elapsed_s", 125.4},
                             {"tokens", 540},
                             {"stopped", "target"},
                             {"rolls", QJsonArray{QJsonObject{{"index", 0}}, QJsonObject{{"index", 1}}}}};
    const QString text = VNestingHttpClient::SummaryText(result);
    const QLocale locale;                                                     // numbers follow the user's locale
    QVERIFY2(text.contains(locale.toString(2.35, 'f', 2)), qPrintable(text)); // length in metres
    QVERIFY2(text.contains(locale.toString(81.3, 'f', 1)), qPrintable(text)); // utilisation in percent
    QVERIFY2(text.contains(QStringLiteral("2:05")), qPrintable(text));        // elapsed as m:ss
    QVERIFY2(text.contains(QStringLiteral("540")), qPrintable(text));         // tokens
    QVERIFY2(text.contains(QStringLiteral("2 roll")), qPrintable(text));
    QVERIFY2(not VNestingHttpClient::SummaryText(QJsonObject{{"marker_length_mm", 1000}, {"utilisation", 0.5}, {"tokens", 0}})
                     .contains(QStringLiteral("roll")),
             "no rolls, no roll count");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::SummaryOfCancelledJob()
{
    const QJsonObject summary{{"tokens", 180},
                              {"elapsed_s", 42.0},
                              {"progress", QJsonObject{{"marker_length_mm", 3000.0}, {"utilisation", 0.7}}}};
    const QString text = VNestingHttpClient::CancelSummaryText(summary);
    QVERIFY2(text.contains(QStringLiteral("0:42")), qPrintable(text));
    QVERIFY2(text.contains(QStringLiteral("180")), qPrintable(text));
    const QLocale locale;
    QVERIFY2(text.contains(locale.toString(3.0, 'f', 2)) && text.contains(locale.toString(70.0, 'f', 1)),
             qPrintable(text));
    // nothing known yet (queued, or the DELETE answered 204 or failed)
    const QString bare = VNestingHttpClient::CancelSummaryText(QJsonObject());
    QVERIFY2(not bare.isEmpty() && not bare.contains(QStringLiteral("utilisation")), qPrintable(bare));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::SummaryOfStoppedJob()
{
    // spec 15: an unlimited job ends on the user's request or when the service lost contact with the app
    const QJsonObject base{{"marker_length_mm", 1000.0}, {"utilisation", 0.5}, {"elapsed_s", 61.0}, {"tokens", 120}};
    QJsonObject user = base;
    user.insert(QStringLiteral("stopped"), QStringLiteral("user"));
    QJsonObject abandoned = base;
    abandoned.insert(QStringLiteral("stopped"), QStringLiteral("abandoned"));
    QJsonObject time = base;
    time.insert(QStringLiteral("stopped"), QStringLiteral("time"));
    QVERIFY2(VNestingHttpClient::SummaryText(user) != VNestingHttpClient::SummaryText(time), "a user stop has its own reason");
    QVERIFY2(VNestingHttpClient::SummaryText(abandoned) != VNestingHttpClient::SummaryText(time), "so has an abandoned job");
    QVERIFY2(VNestingHttpClient::SummaryText(abandoned) != VNestingHttpClient::SummaryText(user), "and they differ");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::StoppingProgressText()
{
    const QJsonObject progress{{"marker_length_mm", 2000.0}, {"utilisation", 0.7}};
    const QString stopping = VNestingHttpClient::ProgressText(QStringLiteral("stopping"), -1, progress);
    QVERIFY2(stopping != VNestingHttpClient::ProgressText(QStringLiteral("running"), -1, progress), qPrintable(stopping));
    QVERIFY2(not stopping.contains(QStringLiteral("queue")), qPrintable(stopping));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::QueuedProgressTextWithEta()
{
    // spec 16.2: a queued job says when it should start
    const QJsonObject eta{{"eta_s", QJsonObject{{"min", 90}, {"max", 400}}}};
    const QString text = VNestingHttpClient::ProgressText(QStringLiteral("queued"), 2, eta);
    QVERIFY2(text.contains(QStringLiteral("3")) && text.contains(QStringLiteral("2 "))
                 && text.contains(QStringLiteral("7")),
             qPrintable(text)); // position 3, 2 to 7 minutes
    const QJsonObject soonEta{{"eta_s", QJsonObject{{"min", 5}, {"max", 30}}}};
    const QString soon = VNestingHttpClient::ProgressText(QStringLiteral("queued"), 0, soonEta);
    QVERIFY2(soon != VNestingHttpClient::ProgressText(QStringLiteral("queued"), 0, QJsonObject()), qPrintable(soon));
    const QJsonObject sameEta{{"eta_s", QJsonObject{{"min", 100}, {"max", 100}}}};
    QVERIFY2(VNestingHttpClient::ProgressText(QStringLiteral("queued"), 0, sameEta).contains(QStringLiteral("2")),
             "about 2 minutes");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::RunningProgressTextShowsTokens()
{
    // spec 20: while running, the dialog shows the tokens spent so far, and the budget when there is one
    const QJsonObject progress{{"marker_length_mm", 4321.0},
                               {"utilisation", 0.82},
                               {"tokens", 12},
                               {"budget_tokens", 120}};
    const QString text = VNestingHttpClient::ProgressText(QStringLiteral("running"), 0, progress);
    QVERIFY2(text.contains(QStringLiteral("12")) && text.contains(QStringLiteral("120")), qPrintable(text));
    const QJsonObject unlimited{{"tokens", 7}};
    const QString noBudget = VNestingHttpClient::ProgressText(QStringLiteral("running"), 0, unlimited);
    QVERIFY2(noBudget.contains(QStringLiteral("7")) && not noBudget.contains(QStringLiteral("120")),
             qPrintable(noBudget));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::FilesBody()
{
    // spec 17.1: the multipart body of PUT /jobs/{id}/files, built without a network
    const QByteArray body = VNestingHttpClient::NestingFilesBody({{QStringLiteral("/tmp/dir/p.val"), QByteArray("<p/>")}},
                                                            QJsonObject{{"app", "valentina"}},
                                                            QStringLiteral("XX"));
    QVERIFY2(body.contains("--XX\r\nContent-Disposition: form-data; name=\"files\"; filename=\"p.val\"\r\n"),
             body.constData());
    QVERIFY2(body.contains("\r\n\r\n<p/>\r\n"), body.constData());
    QVERIFY2(body.contains("name=\"meta\"\r\n\r\n{\"app\":\"valentina\"}"), body.constData());
    QVERIFY2(body.endsWith("--XX--\r\n"), body.constData());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::ProgressDialogGauges()
{
    const QJsonObject request{{QStringLiteral("timeBudgetS"), 30}};
    VNestingProgressDialog dialog(request, false);
    auto *utilisation = dialog.findChild<VCircularGauge *>(QStringLiteral("gaugeUtilisation"));
    auto *tokens = dialog.findChild<QProgressBar *>(QStringLiteral("progressBarTokens"));
    auto *target = dialog.findChild<QProgressBar *>(QStringLiteral("progressBarTarget"));
    auto *groups = dialog.findChild<QProgressBar *>(QStringLiteral("progressBarGroups"));
    QVERIFY(utilisation != nullptr && tokens != nullptr && target != nullptr && groups != nullptr);
    QVERIFY(not tokens->isHidden());
    QVERIFY(target->isHidden());
    QVERIFY(groups->isHidden());
    dialog.Update(QStringLiteral("running"), -1,
                  QJsonObject{{QStringLiteral("elapsed_s"), 12}, {QStringLiteral("utilisation"), 0.5}});
    QCOMPARE(utilisation->Value(), 50);
    QVERIFY(VCircularGauge::Colour(0).hue() == 0 && VCircularGauge::Colour(100).hue() == 120
            && VCircularGauge::Colour(50).hue() == 60);
    QCOMPARE(tokens->maximum(), 30);
    QCOMPARE(tokens->value(), 12);
    const QString conditions = VNestingProgressDialog::ConditionsText(request);
    QVERIFY2(conditions.contains(QStringLiteral("30")), qPrintable(conditions));

    VNestingProgressDialog none(QJsonObject(), false);
    QVERIFY(none.findChild<QProgressBar *>(QStringLiteral("progressBarTokens"))->isHidden());
    QVERIFY(none.findChild<QProgressBar *>(QStringLiteral("progressBarTarget"))->isHidden());
    QVERIFY(none.findChild<QProgressBar *>(QStringLiteral("progressBarGroups"))->isHidden());
    QVERIFY(VNestingProgressDialog::ConditionsText(QJsonObject()) != conditions);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::LocalEngineCarriesTheJob()
{
    QVector<int> skipped;
    VNestingPieceInput piece;
    piece.name = QStringLiteral("A");
    piece.contourPx = {{0, 0}, {50 * NestingPxPerMm, 0}, {50 * NestingPxPerMm, 100 * NestingPxPerMm}, {0, 100 * NestingPxPerMm}};
    const QJsonObject job = NestingRequest({piece}, 1500, 2, {NestingRotations::Any, 15}, QRectF(), true, skipped);
    QVERIFY(skipped.isEmpty());

    VNestingTransport missing(this);
    missing.SetEngine(false, QString(), QString());
    QString early;
    connect(&missing, &VNestingTransport::Failed, this, [&early](int, const QString &message) { early = message; });
    missing.Submit(QJsonObject{{QStringLiteral("version"), 1}, {QStringLiteral("timeBudgetS"), 0}});
    QVERIFY2(not early.isEmpty(), qPrintable(early));

    early.clear();
    missing.SetEngine(false, QString(), QString());
    missing.Submit(job);
    QVERIFY2(not early.isEmpty(), qPrintable(early));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString scriptPath = dir.filePath(QStringLiteral("engine.py"));
    const QString dumpPath = dir.filePath(QStringLiteral("job.json"));
    QFile script(scriptPath);
    QVERIFY(script.open(QIODevice::WriteOnly));
    const QByteArray program = QStringLiteral(
                                   "#!/usr/bin/env python3\n"
                                   "import json, sys\n"
                                   "raw = sys.stdin.buffer.read()\n"
                                   "open(r'''%1''', 'wb').write(raw)\n"
                                   "job = json.loads(raw.decode())\n"
                                   "ids = [i['id'] for i in job.get('instances', [])]\n"
                                   "placements = []\n"
                                   "if ids:\n"
                                   "    placements.append({'instanceId': ids[0], 'flip': 'none', 'rotationDeg': 0,\n"
                                   "                       'translation': {'xMm': 1, 'yMm': 2}})\n"
                                   "sys.stdout.write(json.dumps({'status': 'incomplete', 'bins': [{'lengthUsedMm': 12,\n"
                                   "    'placements': placements}], 'unplaced': ids[1:]}))\n")
                                   .arg(dumpPath)
                                   .toUtf8();
    QCOMPARE(script.write(program), static_cast<qint64>(program.size()));
    script.close();

    // CreateProcess will not run a shebang script, so the program is the interpreter and the script is an argument.
#if defined(Q_OS_WIN)
    const QStringList interpreterNames{QStringLiteral("python"), QStringLiteral("python3")};
#else
    const QStringList interpreterNames{QStringLiteral("python3"), QStringLiteral("python")};
#endif
    QString interpreter;
    for (const QString &name : interpreterNames)
    {
        interpreter = QStandardPaths::findExecutable(name);
        if (not interpreter.isEmpty())
        {
            break;
        }
    }
    QVERIFY2(not interpreter.isEmpty(), "The local-engine stand-in needs python3 or python on PATH.");

    VNestingTransport transport(this);
    transport.SetEngine(true, QString(), interpreter, {scriptPath});
    QJsonObject result;
    QString failed;
    QEventLoop loop;
    connect(&transport, &VNestingTransport::Finished, &loop,
            [&result, &loop](const QJsonObject &value)
            {
                result = value;
                loop.quit();
            });
    connect(&transport, &VNestingTransport::Failed, &loop,
            [&failed, &loop](int, const QString &message)
            {
                failed = message;
                loop.quit();
            });
    transport.Submit(job);
    QTimer::singleShot(8000, &loop, &QEventLoop::quit);
    loop.exec();
    QVERIFY2(failed.isEmpty(), qPrintable(failed));
    QCOMPARE(result.value(QStringLiteral("status")).toString(), QStringLiteral("incomplete"));
    QFile dump(dumpPath);
    QVERIFY(dump.open(QIODevice::ReadOnly));
    const QByteArray bytes = dump.readAll();
    QCOMPARE(bytes, QJsonDocument(job).toJson(QJsonDocument::Compact));
    QVERIFY(not bytes.contains("gap_mm"));
    QVERIFY(not bytes.contains("budget_tokens"));
    QCOMPARE(result.value(QStringLiteral("bins")).toArray().at(0).toObject().value(QStringLiteral("lengthUsedMm")).toDouble(),
             12.0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingHttpClient::BearerRequestDoesNotFollowRedirects()
{
    // a redirect would carry the account token to whatever host answers: the client treats it as an error instead
    const QNetworkRequest request = VNestingHttpClient::BearerRequest(
        QUrl(QStringLiteral("https://nesting.example/jobs")), QStringLiteral("tok"));
    QCOMPARE(request.rawHeader("Authorization"), QByteArray("Bearer tok"));
    QCOMPARE(request.attribute(QNetworkRequest::RedirectPolicyAttribute).value<QNetworkRequest::RedirectPolicy>(),
             QNetworkRequest::ManualRedirectPolicy);
}
