/************************************************************************
 **
 **  @file   tst_vnestorclient.cpp
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
#include "tst_vnestorclient.h"
#include "../vnesting/vcirculargauge.h"
#include "../vnesting/vnestingoptionswidget.h"
#include "../vnesting/vnestingprogressdialog.h"
#include "../vnesting/vnestorclient.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VNestorClient::TST_VNestorClient(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::SummaryOfFinishedJob()
{
    const QJsonObject result{{"marker_length_mm", 2345.6},
                             {"utilisation", 0.8125},
                             {"elapsed_s", 125.4},
                             {"tokens", 540},
                             {"stopped", "target"},
                             {"rolls", QJsonArray{QJsonObject{{"index", 0}}, QJsonObject{{"index", 1}}}}};
    const QString text = VNestorClient::SummaryText(result);
    const QLocale locale;                                                     // numbers follow the user's locale
    QVERIFY2(text.contains(locale.toString(2.35, 'f', 2)), qPrintable(text)); // length in metres
    QVERIFY2(text.contains(locale.toString(81.3, 'f', 1)), qPrintable(text)); // utilisation in percent
    QVERIFY2(text.contains(QStringLiteral("2:05")), qPrintable(text));        // elapsed as m:ss
    QVERIFY2(text.contains(QStringLiteral("540")), qPrintable(text));         // tokens
    QVERIFY2(text.contains(QStringLiteral("2 roll")), qPrintable(text));
    QVERIFY2(not VNestorClient::SummaryText(QJsonObject{{"marker_length_mm", 1000}, {"utilisation", 0.5}, {"tokens", 0}})
                     .contains(QStringLiteral("roll")),
             "no rolls, no roll count");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::SummaryOfCancelledJob()
{
    const QJsonObject summary{{"tokens", 180},
                              {"elapsed_s", 42.0},
                              {"progress", QJsonObject{{"marker_length_mm", 3000.0}, {"utilisation", 0.7}}}};
    const QString text = VNestorClient::CancelSummaryText(summary);
    QVERIFY2(text.contains(QStringLiteral("0:42")), qPrintable(text));
    QVERIFY2(text.contains(QStringLiteral("180")), qPrintable(text));
    const QLocale locale;
    QVERIFY2(text.contains(locale.toString(3.0, 'f', 2)) && text.contains(locale.toString(70.0, 'f', 1)),
             qPrintable(text));
    // nothing known yet (queued, or the DELETE answered 204 or failed)
    const QString bare = VNestorClient::CancelSummaryText(QJsonObject());
    QVERIFY2(not bare.isEmpty() && not bare.contains(QStringLiteral("utilisation")), qPrintable(bare));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::ParseRollLengths()
{
    const QLocale en(QLocale::English, QLocale::UnitedStates);
    const QLocale fr(QLocale::French, QLocale::France);
    const QVector<qreal> expected{50000, 50000, 32500};
    // separators: spaces and semicolons everywhere, a comma followed by a space or the end everywhere
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("50, 50, 32.5"), en), expected);
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("50; 50; 32,5"), fr), expected);
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("50 50 32,5"), fr), expected);
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("50, 50, 32,5"), fr), expected);
    // a comma between digits is the locale's decimal mark, or a separator where the mark is a point
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("50,50"), fr), (QVector<qreal>{50500}));
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("50,5"), en), (QVector<qreal>{50000, 5000}));
    // group separators are refused: "1,000" is not one thousand metres
    QVERIFY(not VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("1,000"), en).contains(1000000));
    QVERIFY(not VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral("1.000"), fr).contains(1000000));
    QCOMPARE(VNestingOptionsWidget::ParseRollLengthsMm(QStringLiteral(" 50 ,abc, -3, 0, 50 "), en),
             (QVector<qreal>{50000, 50000}));
    QVERIFY(VNestingOptionsWidget::ParseRollLengthsMm(QString(), fr).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::SummaryOfStoppedJob()
{
    // spec 15: an unlimited job ends on the user's request or when the service lost contact with the app
    const QJsonObject base{{"marker_length_mm", 1000.0}, {"utilisation", 0.5}, {"elapsed_s", 61.0}, {"tokens", 120}};
    QJsonObject user = base;
    user.insert(QStringLiteral("stopped"), QStringLiteral("user"));
    QJsonObject abandoned = base;
    abandoned.insert(QStringLiteral("stopped"), QStringLiteral("abandoned"));
    QJsonObject time = base;
    time.insert(QStringLiteral("stopped"), QStringLiteral("time"));
    QVERIFY2(VNestorClient::SummaryText(user) != VNestorClient::SummaryText(time), "a user stop has its own reason");
    QVERIFY2(VNestorClient::SummaryText(abandoned) != VNestorClient::SummaryText(time), "so has an abandoned job");
    QVERIFY2(VNestorClient::SummaryText(abandoned) != VNestorClient::SummaryText(user), "and they differ");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::StoppingProgressText()
{
    const QJsonObject progress{{"marker_length_mm", 2000.0}, {"utilisation", 0.7}};
    const QString stopping = VNestorClient::ProgressText(QStringLiteral("stopping"), -1, progress);
    QVERIFY2(stopping != VNestorClient::ProgressText(QStringLiteral("running"), -1, progress), qPrintable(stopping));
    QVERIFY2(not stopping.contains(QStringLiteral("queue")), qPrintable(stopping));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::QueuedProgressTextWithEta()
{
    // spec 16.2: a queued job says when it should start
    const QJsonObject eta{{"eta_s", QJsonObject{{"min", 90}, {"max", 400}}}};
    const QString text = VNestorClient::ProgressText(QStringLiteral("queued"), 2, eta);
    QVERIFY2(text.contains(QStringLiteral("3")) && text.contains(QStringLiteral("2 "))
                 && text.contains(QStringLiteral("7")),
             qPrintable(text)); // position 3, 2 to 7 minutes
    const QJsonObject soonEta{{"eta_s", QJsonObject{{"min", 5}, {"max", 30}}}};
    const QString soon = VNestorClient::ProgressText(QStringLiteral("queued"), 0, soonEta);
    QVERIFY2(soon != VNestorClient::ProgressText(QStringLiteral("queued"), 0, QJsonObject()), qPrintable(soon));
    const QJsonObject sameEta{{"eta_s", QJsonObject{{"min", 100}, {"max", 100}}}};
    QVERIFY2(VNestorClient::ProgressText(QStringLiteral("queued"), 0, sameEta).contains(QStringLiteral("2")),
             "about 2 minutes");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::RunningProgressTextShowsTokens()
{
    // spec 20: while running, the dialog shows the tokens spent so far, and the budget when there is one
    const QJsonObject progress{{"marker_length_mm", 4321.0},
                               {"utilisation", 0.82},
                               {"tokens", 12},
                               {"budget_tokens", 120}};
    const QString text = VNestorClient::ProgressText(QStringLiteral("running"), 0, progress);
    QVERIFY2(text.contains(QStringLiteral("12")) && text.contains(QStringLiteral("120")), qPrintable(text));
    const QJsonObject unlimited{{"tokens", 7}};
    const QString noBudget = VNestorClient::ProgressText(QStringLiteral("running"), 0, unlimited);
    QVERIFY2(noBudget.contains(QStringLiteral("7")) && not noBudget.contains(QStringLiteral("120")),
             qPrintable(noBudget));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::FilesBody()
{
    // spec 17.1: the multipart body of PUT /jobs/{id}/files, built without a network
    const QByteArray body = VNestorClient::NestingFilesBody({{QStringLiteral("/tmp/dir/p.val"), QByteArray("<p/>")}},
                                                            QJsonObject{{"app", "valentina"}},
                                                            QStringLiteral("XX"));
    QVERIFY2(body.contains("--XX\r\nContent-Disposition: form-data; name=\"files\"; filename=\"p.val\"\r\n"),
             body.constData());
    QVERIFY2(body.contains("\r\n\r\n<p/>\r\n"), body.constData());
    QVERIFY2(body.contains("name=\"meta\"\r\n\r\n{\"app\":\"valentina\"}"), body.constData());
    QVERIFY2(body.endsWith("--XX--\r\n"), body.constData());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::ProgressDialogGauges()
{
    // the progress dialog shows a gauge per stop condition of the request, and the group being nested (spec 21.5)
    const QJsonObject request{{"budget_tokens", 240},
                              {"target_length_mm", 5000.0},
                              {"group_by_group", true},
                              {"until_converged", true}};
    VNestingProgressDialog dialog(request, false);
    auto *utilisation = dialog.findChild<VCircularGauge *>(QStringLiteral("gaugeUtilisation"));
    auto *tokens = dialog.findChild<QProgressBar *>(QStringLiteral("progressBarTokens"));
    auto *target = dialog.findChild<QProgressBar *>(QStringLiteral("progressBarTarget"));
    auto *groups = dialog.findChild<QProgressBar *>(QStringLiteral("progressBarGroups"));
    QVERIFY(utilisation != nullptr && tokens != nullptr && target != nullptr && groups != nullptr);
    QVERIFY(not tokens->isHidden() && not target->isHidden() && not groups->isHidden());
    dialog.Update(QStringLiteral("running"),
                  -1,
                  QJsonObject{{"marker_length_mm", 6250.0},
                              {"utilisation", 0.8216},
                              {"tokens", 12},
                              {"budget_tokens", 240},
                              {"group", 2},
                              {"groups", 5},
                              {"group_name", "robe 40"}});
    QCOMPARE(utilisation->Value(), 82);
    // red at 0 %, green at 100 %: the dial's colour is a verdict on the marker
    QVERIFY(VCircularGauge::Colour(0).hue() == 0 && VCircularGauge::Colour(100).hue() == 120
            && VCircularGauge::Colour(50).hue() == 60);
    QCOMPARE(tokens->maximum(), 240);
    QCOMPARE(tokens->value(), 12);
    QCOMPARE(target->value(), 80); // 5 m wanted, 6.25 m so far
    QCOMPARE(groups->maximum(), 5);
    QCOMPARE(groups->value(), 2);
    QVERIFY2(groups->format().contains(QStringLiteral("robe 40")), qPrintable(groups->format()));

    const QString conditions = VNestingProgressDialog::ConditionsText(request);
    QVERIFY2(conditions.contains(QStringLiteral("240")) && conditions.contains(QStringLiteral("5")),
             qPrintable(conditions));

    // nothing asked: the job runs until the user stops it, the gauges of the conditions stay hidden
    VNestingProgressDialog unlimited(QJsonObject(), true);
    QVERIFY(unlimited.findChild<QProgressBar *>(QStringLiteral("progressBarTokens"))->isHidden());
    QVERIFY(unlimited.findChild<QProgressBar *>(QStringLiteral("progressBarTarget"))->isHidden());
    QVERIFY(unlimited.findChild<QProgressBar *>(QStringLiteral("progressBarGroups"))->isHidden());
    QVERIFY(VNestingProgressDialog::ConditionsText(QJsonObject()) != conditions);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestorClient::BearerRequestDoesNotFollowRedirects()
{
    // a redirect would carry the account token to whatever host answers: the client treats it as an error instead
    const QNetworkRequest request = VNestorClient::BearerRequest(QUrl(QStringLiteral("https://nestor.test/jobs")),
                                                                 QStringLiteral("tok"));
    QCOMPARE(request.rawHeader("Authorization"), QByteArray("Bearer tok"));
    QCOMPARE(request.attribute(QNetworkRequest::RedirectPolicyAttribute).value<QNetworkRequest::RedirectPolicy>(),
             QNetworkRequest::ManualRedirectPolicy);
}
