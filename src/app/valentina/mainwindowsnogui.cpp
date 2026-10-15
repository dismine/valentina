/************************************************************************
 **
 **  @file   mainwindowsnogui.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "mainwindowsnogui.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../vdxf/libdxfrw/drw_base.h"
#include "../vformat/vmeasurements.h"
#include "../vganalytics/vganalytics.h"
#include "../vlayout/vlayoutexporter.h"
#include "../vlayout/vbank.h"
#include "../vlayout/vlayoutgenerator.h"
#include "../vlayout/vnestingframe.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vcommonsettings.h"
#include "../vnesting/vaccounttoken.h"
#include "../vnesting/vnestingprogressdialog.h"
#include "../vnesting/vnestorclient.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "core/vapplication.h"
#include "dialogs/dialoglayoutprogress.h"
#include "dialogs/dialoglayoutsettings.h"
#include "dialogs/dialogsavelayout.h"
#include <QAction>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
#include "../vmisc/codecs/qtextcodec.h"
#else
#include "../vmisc/vtextcodec.h"
using QTextCodec = VTextCodec;
#endif // WITH_TEXTCODEC
#else
#include <QTextCodec>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

#include <QDebug>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFile>
#include <QJsonObject>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QPageSize>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinterInfo>
#include <QProcess>
#include <QProgressDialog>
#include <QToolButton>
#include <QtConcurrent>
#include <QtSvg>
#include <functional>

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vMainNoGUIWindow, "v.mainnoguiwindow") // NOLINT

QT_WARNING_POP

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CreateLayoutPath(const QString &path) -> bool
{
    bool usedNotExistedDir = true;
    QDir dir(path);
    dir.setPath(path);
    if (not dir.exists(path))
    {
        usedNotExistedDir = dir.mkpath(QChar('.'));
    }
    return usedNotExistedDir;
}

//---------------------------------------------------------------------------------------------------------------------
void RemoveLayoutPath(const QString &path, bool usedNotExistedDir)
{
    if (usedNotExistedDir)
    {
        QDir const dir(path);
        dir.rmpath(QChar('.'));
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if !defined(V_NO_ASSERT)
Q_DECL_UNUSED void InsertGlobalContours(const QList<QGraphicsScene *> &scenes, const QList<QGraphicsItem *> &gcontours);
void InsertGlobalContours(const QList<QGraphicsScene *> &scenes, const QList<QGraphicsItem *> &gcontours)
{
    if (scenes.size() != gcontours.size())
    {
        return;
    }

    for (int i = 0; i < scenes.size(); ++i)
    {
        scenes.at(i)->addItem(gcontours.at(i));
    }
}
#endif
} // namespace

//---------------------------------------------------------------------------------------------------------------------
MainWindowsNoGUI::MainWindowsNoGUI(QWidget *parent)
  : VAbstractMainWindow(parent),
    m_watcher(new QFileSystemWatcher(this)),
    pattern(new VContainer(VAbstractApplication::VApp()->TrVars(),
                           VAbstractValApplication::VApp()->patternUnitsP(),
                           valentinaNamespace))
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ,
    m_taskbarButton(new QWinTaskbarButton(this))
#endif
{
    InitTempLayoutScene();

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_taskbarButton->setWindow(this->windowHandle());
    m_taskbarProgress = m_taskbarButton->progress();
    m_taskbarProgress->setMinimum(0);
#endif

    m_layoutSettings->SetParentWidget(this);
}

//---------------------------------------------------------------------------------------------------------------------
MainWindowsNoGUI::~MainWindowsNoGUI()
{
    delete m_unreadPatternMessage.data();
    delete m_mouseCoordinate.data();
    delete tempSceneLayout;
    delete pattern;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ToolLayoutSettings(bool checked)
{
    auto *tButton = qobject_cast<QAction *>(this->sender());
    SCASSERT(tButton != nullptr)

    if (checked)
    {
        VLayoutGenerator lGenerator;

        DialogLayoutSettings layout(&lGenerator, this);
        layout.SetOnlineRequestBuilder(
            [this, &layout]()
            {
                QVector<VLayoutPiece> sent;
                QStringList skippedNames;
                const QJsonObject request =
                    OnlineRequest(layout.GetPaperWidth(), layout.GetLayoutWidth(), layout.IsBoundaryTogetherWithNotches(),
                                  layout.IsNestQuantity(), layout.GetOnlineOptions(), sent, skippedNames);
                return sent.isEmpty() ? QJsonObject() : request;
            });
        if (layout.exec() == QDialog::Rejected)
        {
            tButton->setChecked(false);
            return;
        }
        m_layoutSettings->SetLayoutPrinterName(layout.SelectedPrinter());
        GenerateLayout(lGenerator);
        tButton->setChecked(false);
    }
    else
    {
        tButton->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::GenerateLayout(VLayoutGenerator &lGenerator) -> bool
{
    if (lGenerator.IsNestOnline())
    {
        return GenerateLayoutOnline(lGenerator);
    }

    lGenerator.SetDetails(listDetails);

    QElapsedTimer timer;
    timer.start();

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTimer *progressTimer = nullptr;
#endif

    QSharedPointer<DialogLayoutProgress> progress;
    if (VApplication::IsGUIMode())
    {
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_taskbarProgress->setVisible(true);
        m_taskbarProgress->setValue(0);
        m_taskbarProgress->setMaximum(lGenerator.GetNestingTime() * 60);
        progressTimer = new QTimer(this);
        connect(progressTimer, &QTimer::timeout, this,
                [this, timer]() { m_taskbarProgress->setValue(static_cast<int>(timer.elapsed() / 1000)); });
        progressTimer->start(1000);
#endif

        progress = QSharedPointer<DialogLayoutProgress>(
            new DialogLayoutProgress(timer, lGenerator.GetNestingTimeMSecs(), this));

        connect(progress.data(), &DialogLayoutProgress::Abort, &lGenerator, &VLayoutGenerator::Abort);
        connect(progress.data(), &DialogLayoutProgress::Timeout, &lGenerator, &VLayoutGenerator::Timeout);

        progress->Start();
    }
    else
    {
        // Because the progress bar dialog will not terminate nesting we must create separate timer for this
        auto *progressTimer = new QTimer(this);
        connect(progressTimer, &QTimer::timeout, this,
                [timer, &lGenerator, progressTimer]()
                {
                    const auto timeout = static_cast<int>(lGenerator.GetNestingTimeMSecs() - timer.elapsed());

                    if (timeout <= 1000)
                    {
                        lGenerator.Timeout();
                        progressTimer->stop();
                        progressTimer->deleteLater();
                    }
                });
        progressTimer->start(1s);
    }

    LayoutErrors nestingState = LayoutErrors::NoError;

    auto IsTimeout = [&progress, &lGenerator, timer, &nestingState]()
    {
        if (timer.hasExpired(lGenerator.GetNestingTimeMSecs()))
        {
            if (nestingState != LayoutErrors::EmptyPaperError)
            {
                nestingState = LayoutErrors::Timeout;
            }

            if (VApplication::IsGUIMode())
            {
                progress->Finished();
            }
            return true;
        }
        return false;
    };

    bool rotationUsed = false;
    int rotatate = 1;
    lGenerator.SetShift(-1); // Trigger first shift calulation
    lGenerator.SetRotate(false);
    vsizetype papersCount = INT_MAX;
    qreal efficiency = 0;
    bool hasResult = false;

    QCoreApplication::processEvents();

#ifdef LAYOUT_DEBUG
    const QString path = QDir::homePath() + QStringLiteral("/LayoutDebug");
    QDir debugDir(path);
    debugDir.removeRecursively();
    debugDir.mkpath(path);
#endif

    forever
    {
        if (IsTimeout())
        {
            break;
        }

        {
            QEventLoop wait;
            QFutureWatcher<void> fw;
            connect(qApp, &QCoreApplication::aboutToQuit, &fw,
                    [&fw]()
                    {
                        fw.cancel();
                        fw.waitForFinished();
                    });
            QObject::connect(&fw, &QFutureWatcher<void>::finished, &wait, &QEventLoop::quit);
            fw.setFuture(
                QtConcurrent::run([&lGenerator, timer, nestingState]()
                                  { lGenerator.Generate(timer, lGenerator.GetNestingTimeMSecs(), nestingState); }));
            wait.exec();
        }

        if (IsTimeout())
        {
            break;
        }

        nestingState = lGenerator.State();

        switch (nestingState)
        {
            case LayoutErrors::NoError:
                if (lGenerator.PapersCount() <= papersCount)
                {
                    const qreal layoutEfficiency = lGenerator.LayoutEfficiency();
                    if (efficiency < layoutEfficiency || lGenerator.PapersCount() < papersCount)
                    {
                        efficiency = layoutEfficiency;
                        if (VApplication::IsGUIMode())
                        {
                            progress->Efficiency(efficiency);
                        }

                        ApplyGeneratorResult(lGenerator);
                        papersCount = lGenerator.PapersCount();
                        hasResult = true;
                        qDebug() << "Layout efficiency: " << efficiency;
                    }
                    else
                    {
                        if (lGenerator.IsRotationNeeded())
                        {
                            lGenerator.SetRotate(true);
                            lGenerator.SetRotationNumber(++rotatate);
                            rotationUsed = true;
                        }
                    }
                }
                else
                {
                    if (lGenerator.IsRotationNeeded())
                    {
                        lGenerator.SetRotate(true);
                        lGenerator.SetRotationNumber(++rotatate);
                        rotationUsed = true;
                    }
                }
                lGenerator.SetShift(lGenerator.GetShift() / 2.0);
                break;
            case LayoutErrors::EmptyPaperError:
                if (lGenerator.IsRotationNeeded())
                {
                    if (not rotationUsed)
                    {
                        lGenerator.SetRotate(true);
                        lGenerator.SetRotationNumber(++rotatate);
                        rotationUsed = true;
                    }
                    else
                    {
                        lGenerator.SetShift(lGenerator.GetShift() / 2.0);
                        rotationUsed = false;
                    }
                }
                else
                {
                    lGenerator.SetShift(lGenerator.GetShift() / 2.0);
                }
                break;
            case LayoutErrors::Timeout:
            case LayoutErrors::PrepareLayoutError:
            case LayoutErrors::ProcessStoped:
            case LayoutErrors::TerminatedByException:
            default:
                break;
        }

        if (nestingState == LayoutErrors::PrepareLayoutError || nestingState == LayoutErrors::ProcessStoped ||
            nestingState == LayoutErrors::TerminatedByException)
        {
            break;
        }

        if (nestingState == LayoutErrors::NoError && not qFuzzyIsNull(lGenerator.GetEfficiencyCoefficient()) &&
            efficiency >= lGenerator.GetEfficiencyCoefficient() &&
            (not lGenerator.IsPreferOneSheetSolution() || lGenerator.PapersCount() == 1))
        {
            break;
        }

        if (IsTimeout())
        {
            break;
        }
    }

    if (VApplication::IsGUIMode())
    {
        PrepareSceneList(PreviewQuatilty::Slow);
        progress->Finished();
    }

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (VApplication::IsGUIMode())
    {
        progressTimer->stop();
        m_taskbarProgress->setVisible(false);
    }
#endif

    if (VApplication::IsGUIMode())
    {
        QApplication::alert(this);
    }

    if (hasResult && nestingState != LayoutErrors::ProcessStoped)
    {
        return true;
    }

    ShowLayoutError(nestingState);
    if (not VApplication::IsGUIMode())
    {
        QCoreApplication::exit(V_EX_DATAERR);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ApplyGeneratorResult(const VLayoutGenerator &lGenerator)
{
    CleanLayout();
    m_layoutSettings->SetLayoutPapers(lGenerator.GetPapersItems());      // Blank sheets
    m_layoutSettings->SetLayoutDetails(lGenerator.GetAllDetailsItems()); // All details items
    detailsOnLayout = lGenerator.GetAllDetails();                        // All details items
    m_layoutSettings->SetLayoutShadows(CreateShadows(m_layoutSettings->LayoutPapers()));
    m_layoutSettings->SetLayoutPortrait(lGenerator.IsPortrait());
    m_layoutSettings->SetLayoutScenes(CreateScenes(m_layoutSettings->LayoutPapers(), m_layoutSettings->LayoutShadows(),
                                                   m_layoutSettings->LayoutDetails()));
    if (VApplication::IsGUIMode())
    {
        PrepareSceneList(PreviewQuatilty::Fast);
    }
    m_layoutSettings->SetIgnorePrinterMargins(not lGenerator.IsUsePrinterFields());
    m_layoutSettings->SetLayoutMargins(lGenerator.GetPrinterFields());
    m_layoutSettings->SetLayoutPaperSize(QSizeF(lGenerator.GetPaperWidth(), lGenerator.GetPaperHeight()));
    m_layoutSettings->SetAutoCropLength(lGenerator.GetAutoCropLength());
    m_layoutSettings->SetAutoCropWidth(lGenerator.GetAutoCropWidth());
    m_layoutSettings->SetUnitePages(lGenerator.IsUnitePages());
    m_layoutSettings->SetLayoutStale(false);
    m_layoutSettings->SetBoundaryTogetherWithNotches(lGenerator.IsBoundaryTogetherWithNotches());
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::OnlineRequest(qreal paperWidthPx, qreal layoutWidthPx, bool together, bool nestQuantity,
                                     const VNestingOptions &options, QVector<VLayoutPiece> &sent,
                                     QStringList &skippedNames) const -> QJsonObject
{
    // request: each piece in its layout base (forced flip, grainline along local y); quantities are expanded here,
    // like the local nester does, so that symmetrical copies alternate sides
    const QVector<VLayoutPiece> candidates = nestQuantity ? VBank::PrepareQuantity(listDetails) : listDetails;
    QVector<VNestingPieceInput> inputs;
    inputs.reserve(candidates.size());
    for (const auto &piece : candidates)
    {
        inputs.append({piece.GetName(), NestingContour(piece, together), NestingLayoutBase(piece),
                       piece.IsForbidFlipping(), piece.IsForceFlipping(), 1});
    }
    const qreal fabricWidthMm = paperWidthPx / NestingPxPerMm;
    const qreal gapMm = layoutWidthPx * 2 / NestingPxPerMm; // layout width is per side
    QVector<int> skipped;
    const QJsonObject request = NestingRequest(inputs, fabricWidthMm, gapMm, options, skipped);
    sent.clear();
    skippedNames.clear();
    for (vsizetype i = 0; i < candidates.size(); ++i)
    {
        if (skipped.contains(static_cast<int>(i)))
        {
            skippedNames.append(candidates.at(i).GetName());
        }
        else
        {
            sent.append(candidates.at(i));
        }
    }
    return request;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::GenerateLayoutOnline(VLayoutGenerator &lGenerator) -> bool
{
    if (not VApplication::IsGUIMode())
    {
        qCritical() << tr("Online nesting is not available from the command line.");
        QCoreApplication::exit(V_EX_USAGE);
        return false;
    }

    // access first: a valid, premium token (shared with Puzzle in common.ini)
    const VAccountToken token(VAbstractApplication::VApp()->Settings()->GetAccountToken());
    if (token.IsExpired())
    {
        QMessageBox box(QMessageBox::Information, tr("Online nesting"),
                        tr("Connect Valentina to your smart-pattern.com.ua account to use online nesting."),
                        QMessageBox::Cancel, this);
        QPushButton *open = box.addButton(tr("Open account settings"), QMessageBox::AcceptRole);
        box.exec();
        if (box.clickedButton() == open)
        {
            OpenAccountPreferences();
        }
        return false;
    }
    if (not token.IsPremium())
    {
        QMessageBox box(QMessageBox::Information, tr("Online nesting"),
                        tr("Online nesting is reserved to premium subscribers."), QMessageBox::Cancel, this);
        QPushButton *subscribe = box.addButton(tr("See the subscription"), QMessageBox::AcceptRole);
        box.exec();
        if (box.clickedButton() == subscribe)
        {
            QDesktopServices::openUrl(
                QUrl(QStringLiteral("https://smart-pattern.com.ua/en/catalogue/premium-subscription_66/")));
        }
        return false;
    }

    QVector<VLayoutPiece> sent;
    QStringList skippedNames;
    QJsonObject request =
        OnlineRequest(lGenerator.GetPaperWidth(), lGenerator.GetLayoutWidth(), lGenerator.IsBoundaryTogetherWithNotches(),
                      lGenerator.IsNestQuantity(), lGenerator.GetOnlineOptions(), sent, skippedNames);
    request.insert(QStringLiteral("app"), QStringLiteral("valentina")); // spec 18.2
    request.insert(QStringLiteral("app_version"), AppVersionStr());
    const QVector<int> quantities(sent.size(), 1);
    if (sent.isEmpty())
    {
        QMessageBox::warning(this, tr("Online nesting"), tr("No piece has a usable outline."));
        return false;
    }
    if (not skippedNames.isEmpty())
    {
        QMessageBox::warning(this, tr("Online nesting"),
                             tr("%n piece(s) without a usable outline will not be sent: %1", "",
                                static_cast<int>(skippedNames.size()))
                                 .arg(skippedNames.join(QStringLiteral(", "))));
    }

    // run the job to completion inside this call: the callers expect a synchronous answer
    if (m_nestor == nullptr)
    {
        m_nestor = new VNestorClient(this);
    }
    const bool unlimited = request.value(QLatin1String("time_s")).toInt() == 0; // spec 15: ends when the user says so
    VNestingProgressDialog progress(request, unlimited, this);
    progress.show();
    QEventLoop loop;
    QJsonObject result;
    bool cancelled = false;
    bool stopping = false;
    bool failed = false;
    int failStatus = 0;
    QString failMessage;
    QList<QMetaObject::Connection> conns;
    conns << connect(&progress, &VNestingProgressDialog::canceled, &loop,
                     [this, &cancelled, &loop, &progress, &stopping, unlimited]()
                     {
                         if (unlimited && not stopping)
                         {
                             // first press keeps the best layout (Finished follows, or Cancelled if there was nothing
                             // to keep); a second press cancels for good
                             stopping = true;
                             m_nestor->Stop();
                             progress.SetStopping();
                             return;
                         }
                         cancelled = true;
                         // one shot without Qt::SingleShotConnection (Qt 6 only): the lambda drops its own connection
                         auto once = QSharedPointer<QMetaObject::Connection>::create();
                         *once = connect(m_nestor, &VNestorClient::Cancelled, this,
                                         [this, once](const QJsonObject &summary)
                                         {
                                             disconnect(*once);
                                             QMessageBox::information(this, tr("Online nesting"),
                                                                      VNestorClient::CancelSummaryText(summary));
                                         });
                         m_nestor->Cancel();
                         loop.quit();
                     });
    conns << connect(m_nestor, &VNestorClient::Cancelled, &loop,
                     [this, &cancelled, &loop](const QJsonObject &summary)
                     {
                         // a Stop() that found nothing to keep; an explicit cancel answers after the loop has quit
                         cancelled = true;
                         loop.quit();
                         QMessageBox::information(this, tr("Online nesting"), VNestorClient::CancelSummaryText(summary));
                     });
    conns << connect(m_nestor, &VNestorClient::Progress, &progress,
                     [&progress](const QString &status, int position, const QJsonObject &running)
                     { progress.Update(status, position, running); });
    conns << connect(m_nestor, &VNestorClient::Failed, &loop,
                     [&failed, &failStatus, &failMessage, &loop](int status, const QString &message)
                     {
                         failed = true;
                         failStatus = status;
                         failMessage = message;
                         loop.quit();
                     });
    conns << connect(m_nestor, &VNestorClient::Finished, &loop,
                     [&result, &loop](const QJsonObject &r)
                     {
                         result = r;
                         loop.quit();
                     });
    if (VAbstractApplication::VApp()->Settings()->GetShareFiles())
    {
        // spec 17.1: the pattern as it is in memory, its measurement file, and what the job was asked for
        const QString patternPath = VAbstractValApplication::VApp()->GetPatternPath();
        QList<QPair<QString, QByteArray>> files{
            {patternPath.isEmpty() ? QStringLiteral("pattern.val") : patternPath, doc->toByteArray()}};
        QFile measurements(AbsoluteMPath(patternPath, doc->MPath()));
        if (not doc->MPath().isEmpty() && measurements.open(QIODevice::ReadOnly))
        {
            files.append({measurements.fileName(), measurements.readAll()});
        }
        const auto *app = VAbstractValApplication::VApp();
        m_nestor->SetFiles(files, QJsonObject{{"dimensions", QJsonObject{{"height", app->GetDimensionHeight()},
                                                                         {"size", app->GetDimensionSize()},
                                                                         {"hip", app->GetDimensionHip()},
                                                                         {"waist", app->GetDimensionWaist()}}},
                                              {"paper_width_px", lGenerator.GetPaperWidth()},
                                              {"layout_width_px", lGenerator.GetLayoutWidth()},
                                              {"nest_quantity", lGenerator.IsNestQuantity()}});
    }
    m_nestor->Submit(request);
    loop.exec();
    for (const auto &c : std::as_const(conns))
    {
        disconnect(c);
    }
    progress.close();

    if (cancelled)
    {
        return false;
    }
    if (failed)
    {
        if (failStatus == 401)
        {
            QMessageBox::warning(this, tr("Online nesting"),
                                 tr("The service rejected your session. Sign in again in Preferences > Account.\n%1")
                                     .arg(failMessage));
        }
        else if (failStatus == 403)
        {
            QMessageBox::warning(this, tr("Online nesting"), tr("Access refused: %1").arg(failMessage));
        }
        else if (failMessage.contains(QLatin1String("longer than the usable roll length")))
        {
            QMessageBox::warning(this, tr("Online nesting"),
                                 tr("A piece is longer than a roll minus the junction loss: %1").arg(failMessage));
        }
        else if (failMessage.contains(QLatin1String("could not construct an initial placement")))
        {
            QMessageBox::warning(this, tr("Online nesting"),
                                 tr("A piece does not fit across the fabric width. Check the paper width."));
        }
        else
        {
            QMessageBox::warning(this, tr("Online nesting"), tr("Nesting failed: %1").arg(failMessage));
        }
        return false;
    }
    if (not NestingPlanComplete(result, quantities))
    {
        QMessageBox::warning(this, tr("Online nesting"),
                             tr("The service returned an incomplete result; nothing was changed."));
        return false;
    }

    // one portrait paper per roll (or one of the marker's length), no printer margins
    const QVector<VLayoutPaper> papers =
        NestingResultPapers(sent, NestingPlacements(result), result, lGenerator.GetPaperWidth(),
                            lGenerator.GetLayoutWidth(), lGenerator.IsBoundaryTogetherWithNotches());
    int height = 0;
    for (const auto &paper : papers)
    {
        height = qMax(height, paper.GetHeight());
    }
    lGenerator.SetPapers(papers);
    lGenerator.SetPaperHeight(height);
    lGenerator.SetPrinterFields(false, QMarginsF());
    ApplyGeneratorResult(lGenerator);
    PrepareSceneList(PreviewQuatilty::Slow);
    QApplication::alert(this);
    QMessageBox::information(this, tr("Online nesting"), VNestorClient::SummaryText(result));
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ShowLayoutError(const LayoutErrors &state)
{
    switch (state)
    {
        case LayoutErrors::NoError:
            return;
        case LayoutErrors::PrepareLayoutError:
            qCritical() << tr("Couldn't prepare data for creation layout");
            break;
        case LayoutErrors::EmptyPaperError:
            qCritical() << tr("One or more pattern pieces are bigger than the paper format you selected. Please, "
                              "select a bigger paper format.");
            break;
        case LayoutErrors::Timeout:
            qCritical() << tr("Timeout.");
            break;
        case LayoutErrors::TerminatedByException:
            qCritical() << tr("Process has been stoped because of exception.");
            break;
        case LayoutErrors::ProcessStoped:
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportFMeasurementsToCSV()
{
    QString const fileName = CSVFilePath();

    if (fileName.isEmpty())
    {
        return;
    }

    DialogExportToCSV dialog(this);
    dialog.SetWithHeader(VAbstractApplication::VApp()->Settings()->GetCSVWithHeader());
    dialog.SetSelectedMib(VAbstractApplication::VApp()->Settings()->GetCSVCodec());
    dialog.SetSeparator(VAbstractApplication::VApp()->Settings()->GetCSVSeparator());

    if (dialog.exec() == QDialog::Accepted)
    {
        ExportFMeasurementsToCSVData(fileName, dialog.IsWithHeader(), dialog.GetSelectedMib(), dialog.GetSeparator());

        VAbstractApplication::VApp()->Settings()->SetCSVSeparator(dialog.GetSeparator());
        VAbstractApplication::VApp()->Settings()->SetCSVCodec(dialog.GetSelectedMib());
        VAbstractApplication::VApp()->Settings()->SetCSVWithHeader(dialog.IsWithHeader());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportData(const QVector<VLayoutPiece> &listDetails)
{
    if (const LayoutExportFormats format = m_dialogSaveLayout->Format();
        format == LayoutExportFormats::DXF_AAMA || format == LayoutExportFormats::DXF_ASTM ||
        format == LayoutExportFormats::RLD || format == LayoutExportFormats::HPGL ||
        format == LayoutExportFormats::HPGL2 || format == LayoutExportFormats::HPGL_PLT ||
        format == LayoutExportFormats::HPGL2_PLT)
    {
        if (m_dialogSaveLayout->Mode() == Draw::Layout)
        {
            for (int i = 0; i < detailsOnLayout.size(); ++i)
            {
                const QString index = detailsOnLayout.size() > 1 ? QString::number(i + 1) : QString();
                const QString name = m_dialogSaveLayout->Path() + '/'_L1 + m_dialogSaveLayout->FileName() + index
                                     + VLayoutExporter::ExportFormatSuffix(m_dialogSaveLayout->Format());

                auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutSettings->LayoutPapers().at(i));
                SCASSERT(paper != nullptr)

                ExportApparelLayout(detailsOnLayout.at(i), name, paper->rect().size().toSize());
            }
        }
        else
        {
            ExportDetailsAsApparelLayout(listDetails);
        }
    }
    else
    {
        if (m_dialogSaveLayout->Mode() == Draw::Layout)
        {
            ExportFlatLayout(m_layoutSettings->LayoutScenes(), m_layoutSettings->LayoutPapers(),
                             m_layoutSettings->LayoutShadows(), m_layoutSettings->LayoutDetails(),
                             m_layoutSettings->IgnorePrinterMargins(), m_layoutSettings->LayoutMargins());
        }
        else
        {
            ExportDetailsAsFlatLayout(listDetails);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportFlatLayout(const QList<QGraphicsScene *> &scenes, const QList<QGraphicsItem *> &papers,
                                        const QList<QGraphicsItem *> &shadows,
                                        const QList<QList<QGraphicsItem *>> &details, bool ignorePrinterFields,
                                        const QMarginsF &margins)
{
    const QString path = m_dialogSaveLayout->Path();
    bool const usedNotExistedDir = CreateLayoutPath(path);
    if (not usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    VAbstractValApplication::VApp()->ValentinaSettings()->SetPathLayoutExport(path);

    if (const LayoutExportFormats format = m_dialogSaveLayout->Format();
        format == LayoutExportFormats::PDFTiled && m_dialogSaveLayout->Mode() == Draw::Layout)
    {
        const QString name = path + '/'_L1 + m_dialogSaveLayout->FileName();
        PdfTiledFile(name);
    }
    else
    {
        ExportScene(scenes, papers, shadows, details, ignorePrinterFields, margins);
    }

    RemoveLayoutPath(path, usedNotExistedDir);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportDetailsAsFlatLayout(const QVector<VLayoutPiece> &listDetails)
{
    if (listDetails.isEmpty())
    {
        return;
    }

    QScopedPointer<QGraphicsScene> const scene(new QGraphicsScene());

    QList<QGraphicsItem *> list;
    list.reserve(listDetails.count());
    for (auto piece : listDetails)
    {
        QGraphicsItem *item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths(),
                                            m_dialogSaveLayout->IsBoundaryTogetherWithNotches(), false);
        qreal diff = 0;
        if (piece.IsForceFlipping())
        {
            const qreal x = item->boundingRect().x();
            piece.Mirror();
            item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths(),
                                 m_dialogSaveLayout->IsBoundaryTogetherWithNotches(), false);
            diff = item->boundingRect().x() - x;
        }

        if (piece.IsForceFlipping())
        {
            item->setPos(piece.GetMx() - diff, piece.GetMy());
        }
        else
        {
            item->setPos(piece.GetMx(), piece.GetMy());
        }
        list.append(item);
    }

    for (auto *item : list)
    {
        scene->addItem(item);
    }

    QList<QGraphicsItem *> papers; // Blank sheets
    QRect rect = scene->itemsBoundingRect().toRect();

    const int mx = rect.x();
    const int my = rect.y();

    QTransform matrix;
    matrix = matrix.translate(-mx, -my);

    for (auto *item : list)
    {
        item->setTransform(matrix);
    }

    rect = scene->itemsBoundingRect().toRect();

    auto *paper = new QGraphicsRectItem(rect);
    paper->setPen(QPen(Qt::black, 1));
    paper->setBrush(QBrush(Qt::white));
    papers.append(paper);

    QList<QList<QGraphicsItem *>> details; // All details
    details.append(list);

    QList<QGraphicsItem *> const shadows = CreateShadows(papers);
    QList<QGraphicsScene *> const scenes = CreateScenes(papers, shadows, details);

    const bool ignorePrinterFields = false;
    Q_RELAXED_CONSTEXPR qreal margin = ToPixel(1, Unit::Cm);
    ExportFlatLayout(scenes, papers, shadows, details, ignorePrinterFields, QMarginsF(margin, margin, margin, margin));

    qDeleteAll(scenes); // Scene will clear all other items
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportApparelLayout(const QVector<VLayoutPiece> &details, const QString &name,
                                           const QSize &size) const
{
    const QString path = m_dialogSaveLayout->Path();
    bool const usedNotExistedDir = CreateLayoutPath(path);
    if (not usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    VValentinaSettings  const*settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetPathLayoutExport(path);
    const LayoutExportFormats format = m_dialogSaveLayout->Format();

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutExporter exporter;

    QT_WARNING_POP

    exporter.SetFileName(name);
    exporter.SetImageRect(QRectF(0, 0, size.width(), size.height()));
    exporter.SetXScale(m_dialogSaveLayout->GetXScale());
    exporter.SetYScale(m_dialogSaveLayout->GetYScale());
    exporter.SetBinaryDxfFormat(m_dialogSaveLayout->IsBinaryDXFFormat());
    exporter.SetShowGrainline(m_dialogSaveLayout->IsShowGrainline());
    exporter.SetBoundaryTogetherWithNotches(m_dialogSaveLayout->IsBoundaryTogetherWithNotches());

    switch (format)
    {
        case LayoutExportFormats::DXF_ASTM:
            exporter.SetDxfVersion(DRW::AC1009);
            exporter.SetDxfApparelCompatibility(m_dialogSaveLayout->DxfCompatibility());
            exporter.ExportToASTMDXF(details);
            break;
        case LayoutExportFormats::DXF_AAMA:
            exporter.SetDxfVersion(DRW::AC1009);
            exporter.SetDxfApparelCompatibility(m_dialogSaveLayout->DxfCompatibility());
            exporter.ExportToAAMADXF(details);
            break;
        case LayoutExportFormats::RLD:
            exporter.ExportToRLD(details);
            break;
        case LayoutExportFormats::HPGL:
        case LayoutExportFormats::HPGL_PLT:
            exporter.SetSingleLineFont(settings->GetSingleLineFonts());
            exporter.SetSingleStrokeOutlineFont(settings->GetSingleStrokeOutlineFont());
            exporter.SetPenWidth(qCeil(settings->WidthHairLine()));
            exporter.ExportToHPGL(details);
            break;
        case LayoutExportFormats::HPGL2:
        case LayoutExportFormats::HPGL2_PLT:
            exporter.SetSingleLineFont(settings->GetSingleLineFonts());
            exporter.SetSingleStrokeOutlineFont(settings->GetSingleStrokeOutlineFont());
            exporter.SetPenWidth(qCeil(settings->WidthHairLine()));
            exporter.ExportToHPGL2(details);
            break;
        default:
            qDebug() << "Can't recognize file type." << Q_FUNC_INFO;
            break;
    }

    RemoveLayoutPath(path, usedNotExistedDir);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportDetailsAsApparelLayout(QVector<VLayoutPiece> listDetails)
{
    if (listDetails.isEmpty())
    {
        return;
    }

    QScopedPointer<QGraphicsScene> const scene(new QGraphicsScene());

    QList<QGraphicsItem *> list;
    list.reserve(listDetails.count());
    for (int i = 0; i < listDetails.count(); ++i)
    {
        VLayoutPiece piece = listDetails.at(i);
        QGraphicsItem *item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths(),
                                            m_dialogSaveLayout->IsBoundaryTogetherWithNotches(), false);
        qreal diff = 0;
        if (piece.IsForceFlipping())
        {
            const qreal x = item->boundingRect().x();
            piece.Mirror();
            delete item;
            item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths(),
                                 m_dialogSaveLayout->IsBoundaryTogetherWithNotches(), false);
            diff = item->boundingRect().x() - x;
        }

        QTransform moveMatrix = piece.GetMatrix();
        if (piece.IsForceFlipping())
        {
            item->setPos(piece.GetMx() - diff, piece.GetMy());
            moveMatrix = moveMatrix.translate(-piece.GetMx() + diff, piece.GetMy());
        }
        else
        {
            item->setPos(piece.GetMx(), piece.GetMy());
            moveMatrix = moveMatrix.translate(piece.GetMx(), piece.GetMy());
        }
        listDetails[i].SetMatrix(moveMatrix);
        list.append(item);
    }

    for (auto *item : list)
    {
        scene->addItem(item);
    }

    QRect rect = scene->itemsBoundingRect().toRect();

    const int mx = rect.x();
    const int my = rect.y();

    QTransform matrix;
    matrix = matrix.translate(-mx, -my);

    for (auto *item : list)
    {
        item->setTransform(matrix);
    }

    rect = scene->itemsBoundingRect().toRect();

    for (int i = 0; i < listDetails.count(); ++i)
    {
        QTransform moveMatrix = listDetails.at(i).GetMatrix();
        if (listDetails.at(i).IsForceFlipping())
        {
            moveMatrix = moveMatrix.translate(mx, -my);
        }
        else
        {
            moveMatrix = moveMatrix.translate(-mx, -my);
        }
        listDetails[i].SetMatrix(moveMatrix);
    }

    const QString name = m_dialogSaveLayout->Path() + '/' + m_dialogSaveLayout->FileName()
                         + VLayoutExporter::ExportFormatSuffix(m_dialogSaveLayout->Format());

    ExportApparelLayout(listDetails, name, rect.size());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintPreviewOrigin()
{
    m_layoutSettings->SetFileName(FileName());
    m_layoutSettings->PrintPreviewOrigin();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintPreviewTiled()
{
    m_layoutSettings->SetFileName(FileName());

    if (not m_dialogSaveLayout.isNull())
    {
        m_layoutSettings->SetTiledMargins(m_dialogSaveLayout->GetTiledMargins());
        m_layoutSettings->SetTiledPDFOrientation(m_dialogSaveLayout->GetTiledPageOrientation());

        VAbstractLayoutDialog::PaperSizeTemplate const tiledFormat = m_dialogSaveLayout->GetTiledPageFormat();
        m_layoutSettings->SetTiledPDFPaperSize(VAbstractLayoutDialog::GetTemplateSize(tiledFormat, Unit::Mm));
    }
    else
    {
        VValentinaSettings  const*settings = VAbstractValApplication::VApp()->ValentinaSettings();
        m_layoutSettings->SetTiledMargins(settings->GetTiledPDFMargins(Unit::Mm));
        m_layoutSettings->SetTiledPDFOrientation(settings->GetTiledPDFOrientation());
        m_layoutSettings->SetTiledPDFPaperSize(
            QSizeF(settings->GetTiledPDFPaperWidth(Unit::Mm), settings->GetTiledPDFPaperHeight(Unit::Mm)));
    }

    m_layoutSettings->SetWatermarkPath(
        AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->GetWatermarkPath()));
    m_layoutSettings->PrintPreviewTiled();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintOrigin()
{
    m_layoutSettings->SetFileName(FileName());
    m_layoutSettings->PrintOrigin();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintTiled()
{
    m_layoutSettings->SetFileName(FileName());

    if (not m_dialogSaveLayout.isNull())
    {
        m_layoutSettings->SetTiledMargins(m_dialogSaveLayout->GetTiledMargins());
        m_layoutSettings->SetTiledPDFOrientation(m_dialogSaveLayout->GetTiledPageOrientation());

        VAbstractLayoutDialog::PaperSizeTemplate const tiledFormat = m_dialogSaveLayout->GetTiledPageFormat();
        m_layoutSettings->SetTiledPDFPaperSize(VAbstractLayoutDialog::GetTemplateSize(tiledFormat, Unit::Mm));
    }
    else
    {
        VValentinaSettings  const*settings = VAbstractValApplication::VApp()->ValentinaSettings();
        m_layoutSettings->SetTiledMargins(settings->GetTiledPDFMargins(Unit::Mm));
        m_layoutSettings->SetTiledPDFOrientation(settings->GetTiledPDFOrientation());
        m_layoutSettings->SetTiledPDFPaperSize(
            QSizeF(settings->GetTiledPDFPaperWidth(Unit::Mm), settings->GetTiledPDFPaperHeight(Unit::Mm)));
    }

    m_layoutSettings->SetWatermarkPath(
        AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->GetWatermarkPath()));
    m_layoutSettings->PrintTiled();
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::PrepareDetailsForLayout(const QVector<DetailForLayout> &details) -> QVector<VLayoutPiece>
{
    if (details.isEmpty())
    {
        return {};
    }

    std::function<VLayoutPiece(const DetailForLayout &data)> const PrepareDetail = [](const DetailForLayout &data)
    {
        auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(data.id));
        SCASSERT(tool != nullptr)
        return VLayoutPiece::Create(data.piece, data.id, tool->getData(), data.buffer);
    };

    QProgressDialog progress(QCoreApplication::translate("MainWindowsNoGUI", "Preparing details for layout"), QString(),
                             0, static_cast<int>(details.size()));
    progress.setWindowModality(Qt::ApplicationModal);

    QFutureWatcher<VLayoutPiece> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<VLayoutPiece>::finished, &progress, &QProgressDialog::reset);
    QObject::connect(&futureWatcher, &QFutureWatcher<VLayoutPiece>::progressRangeChanged, &progress,
                     &QProgressDialog::setRange);
    QObject::connect(&futureWatcher, &QFutureWatcher<VLayoutPiece>::progressValueChanged, &progress,
                     &QProgressDialog::setValue);

    // Every worker below reads a tool's container snapshot; the main thread must not write
    // to any container until they are all done. See VContainer::FrozenScope.
    const VContainer::FrozenScope frozen;

    futureWatcher.setFuture(QtConcurrent::mapped(details, PrepareDetail));

    if (VApplication::IsGUIMode())
    {
        progress.exec();
    }

    futureWatcher.waitForFinished();

    QVector<VLayoutPiece> layoutDetails;
    layoutDetails.reserve(details.size());
    const QFuture<VLayoutPiece> future = futureWatcher.future();

    for (auto i = future.constBegin(); i != future.constEnd(); ++i)
    {
        layoutDetails.append(*i);
    }

    return layoutDetails;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::InitTempLayoutScene()
{
    tempSceneLayout = new VMainGraphicsScene();
    tempSceneLayout->setBackgroundBrush(QBrush(QColor(Qt::gray), Qt::SolidPattern));
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::RecentFileList() const -> QStringList
{
    return VAbstractValApplication::VApp()->ValentinaSettings()->GetRecentFileList();
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::ScenePreview(int i, QSize iconSize, PreviewQuatilty quality) const -> QIcon
{
    QImage image;

    if (auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutSettings->LayoutPapers().at(i)); paper)
    {
        if (quality == PreviewQuatilty::Fast)
        {
            image = QImage(iconSize, QImage::Format_RGB32);
            image.fill(Qt::white);
        }
        else
        {
            const QRectF r = paper->rect();
            // Create the image with the exact size of the shrunk scene
            image =
                QImage(QSize(static_cast<qint32>(r.width()), static_cast<qint32>(r.height())), QImage::Format_RGB32);

            if (not image.isNull())
            {
                image.fill(Qt::white);
                QPainter painter(&image);
                painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Normal));
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthMainLine(), Qt::SolidLine,
                                    Qt::RoundCap, Qt::RoundJoin));
                painter.setBrush(QBrush(Qt::NoBrush));
                m_layoutSettings->LayoutScenes().at(i)->render(&painter, r, r, Qt::IgnoreAspectRatio);
                painter.end();
            }
            else
            {
                qDebug() << "Cannot create image. Size " << r.size() << "too big";

                image = QImage(iconSize, QImage::Format_RGB32);
                image.fill(Qt::white);
            }
        }
    }
    else
    {
        image = QImage(iconSize, QImage::Format_RGB32);
        image.fill(Qt::white);
    }
    return {QBitmap::fromImage(image)};
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::CreateShadows(const QList<QGraphicsItem *> &papers) -> QList<QGraphicsItem *>
{
    QList<QGraphicsItem *> shadows;
    shadows.reserve(papers.size());

    for (auto *paper : papers)
    {
        qreal x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        if (auto *item = qgraphicsitem_cast<QGraphicsRectItem *>(paper))
        {
            item->rect().getCoords(&x1, &y1, &x2, &y2);
            auto *shadowPaper = new QGraphicsRectItem(QRectF(x1 + 4, y1 + 4, x2 + 4, y2 + 4));
            shadowPaper->setBrush(QBrush(Qt::black));
            shadows.append(shadowPaper);
        }
        else
        {
            shadows.append(nullptr);
        }
    }

    return shadows;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::CreateScenes(const QList<QGraphicsItem *> &papers, const QList<QGraphicsItem *> &shadows,
                                    const QList<QList<QGraphicsItem *>> &details) -> QList<QGraphicsScene *>
{
    QList<QGraphicsScene *> scenes;
    scenes.reserve(papers.size());
    for (int i = 0; i < papers.size(); ++i)
    {
        auto *scene = new VMainGraphicsScene();
        scene->SetNonInteractive(true);
        scene->setBackgroundBrush(QBrush(QColor(Qt::gray), Qt::SolidPattern));
        scene->addItem(shadows.at(i));
        scene->addItem(papers.at(i));

        const QList<QGraphicsItem *> &paperDetails = details.at(i);
        for (QGraphicsItem *detail : paperDetails)
        {
            scene->addItem(detail);
        }

        scenes.append(scene);
    }

    return scenes;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PdfTiledFile(const QString &name)
{
    if (not m_dialogSaveLayout.isNull())
    {
        m_layoutSettings->SetTiledMargins(m_dialogSaveLayout->GetTiledMargins());
        m_layoutSettings->SetTiledPDFOrientation(m_dialogSaveLayout->GetTiledPageOrientation());

        VAbstractLayoutDialog::PaperSizeTemplate const tiledFormat = m_dialogSaveLayout->GetTiledPageFormat();
        m_layoutSettings->SetTiledPDFPaperSize(VAbstractLayoutDialog::GetTemplateSize(tiledFormat, Unit::Mm));

        m_layoutSettings->SetXScale(m_dialogSaveLayout->GetXScale());
        m_layoutSettings->SetYScale(m_dialogSaveLayout->GetYScale());
    }
    else
    {
        VValentinaSettings  const*settings = VAbstractValApplication::VApp()->ValentinaSettings();
        m_layoutSettings->SetTiledMargins(QMarginsF(settings->GetTiledPDFMargins(Unit::Mm)));
        m_layoutSettings->SetTiledPDFOrientation(settings->GetTiledPDFOrientation());
        m_layoutSettings->SetTiledPDFPaperSize(
            QSizeF(settings->GetTiledPDFPaperWidth(Unit::Mm), settings->GetTiledPDFPaperHeight(Unit::Mm)));
        m_layoutSettings->SetXScale(1);
        m_layoutSettings->SetYScale(1);
    }

    m_layoutSettings->SetWatermarkPath(
        AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->GetWatermarkPath()));
    m_layoutSettings->PdfTiledFile(name);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportScene(const QList<QGraphicsScene *> &scenes, const QList<QGraphicsItem *> &papers,
                                   const QList<QGraphicsItem *> &shadows, const QList<QList<QGraphicsItem *>> &details,
                                   bool ignorePrinterFields, const QMarginsF &margins) const
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutExporter exporter;

    QT_WARNING_POP

    exporter.SetMargins(margins);
    exporter.SetXScale(m_dialogSaveLayout->GetXScale());
    exporter.SetYScale(m_dialogSaveLayout->GetYScale());
    exporter.SetTitle(tr("Pattern"));
    exporter.SetDescription(doc->GetDescription().toHtmlEscaped());
    exporter.SetIgnorePrinterMargins(ignorePrinterFields);
    exporter.SetBinaryDxfFormat(m_dialogSaveLayout->IsBinaryDXFFormat());
    exporter.SetShowGrainline(m_dialogSaveLayout->IsShowGrainline());
    exporter.SetBoundaryTogetherWithNotches(m_dialogSaveLayout->IsBoundaryTogetherWithNotches());

    for (int i = 0; i < scenes.size(); ++i)
    {
        auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
        if (paper != nullptr)
        {
            const QString index = scenes.size() > 1 ? QString::number(i + 1) : QString();
            const QString name = m_dialogSaveLayout->Path() + '/' + m_dialogSaveLayout->FileName() + index
                                 + VLayoutExporter::ExportFormatSuffix(m_dialogSaveLayout->Format());
            auto *brush = new QBrush();
            brush->setColor(QColor(Qt::white));
            QGraphicsScene *scene = scenes.at(i);
            scene->setBackgroundBrush(*brush);
            shadows[i]->setVisible(false);
            const qreal thinPen = 0.1;
            paper->setPen(QPen(QBrush(Qt::white, Qt::NoBrush), thinPen, Qt::NoPen));

            exporter.SetFileName(name);
            exporter.SetImageRect(paper->rect());

            QPen const defaultPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(), Qt::SolidLine,
                                  Qt::RoundCap, Qt::RoundJoin);

            switch (m_dialogSaveLayout->Format())
            {
                case LayoutExportFormats::SVG:
                    paper->setVisible(false);
                    exporter.SetPen(defaultPen);
                    exporter.ExportToSVG(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::PDF:
                    exporter.SetPen(defaultPen);
                    exporter.ExportToPDF(scene, details.at(i));
                    break;
                case LayoutExportFormats::PNG:
                    exporter.SetPen(defaultPen);
                    exporter.ExportToPNG(scene, details.at(i));
                    break;
                case LayoutExportFormats::OBJ:
                    paper->setVisible(false);
                    exporter.ExportToOBJ(scene);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::PS:
                    exporter.SetPen(defaultPen);
                    exporter.ExportToPS(scene, details.at(i));
                    break;
                case LayoutExportFormats::EPS:
                    exporter.SetPen(defaultPen);
                    exporter.ExportToEPS(scene, details.at(i));
                    break;
                case LayoutExportFormats::DXF_AC1006_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1006);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1009_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1009);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1012_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1012);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1014_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1014);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1015_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1015);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1018_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1018);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1021_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1021);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1024_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1024);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::DXF_AC1027_Flat:
                    paper->setVisible(false);
                    exporter.SetDxfVersion(DRW::AC1027);
                    exporter.ExportToFlatDXF(scene, details.at(i));
                    paper->setVisible(true);
                    break;
                case LayoutExportFormats::TIF:
                    exporter.SetPen(defaultPen);
                    exporter.ExportToTIF(scene, details.at(i));
                    break;
                default:
                    qDebug() << "Can't recognize file type." << Q_FUNC_INFO;
                    break;
            }
            paper->setPen(QPen(Qt::black, 1));
            brush->setColor(QColor(Qt::gray));
            brush->setStyle(Qt::SolidPattern);
            scenes[i]->setBackgroundBrush(*brush);
            shadows[i]->setVisible(true);
            delete brush;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::FileName() const -> QString
{
    QString fileName;
    VAbstractValApplication::VApp()->GetPatternPath().isEmpty()
        ? fileName = tr("unnamed")
        : fileName = VAbstractValApplication::VApp()->GetPatternPath();
    return QFileInfo(fileName).baseName();
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::ExportFMeasurementsToCSVData(const QString &fileName, bool withHeader, int mib,
                                                    const QChar &separator) const -> bool
{
    QxtCsvModel csv;

    csv.insertSingleColumn(0);
    csv.insertSingleColumn(1);
    csv.insertSingleColumn(2);

    if (withHeader)
    {
        csv.setHeaderText(0, tr("Name"));
        csv.setHeaderText(1, tr("Value"));
        csv.setHeaderText(2, tr("Description"));
    }

    const QVector<VFinalMeasurement> measurements = doc->GetFinalMeasurements();
    VContainer completeData = doc->GetCompleteData();
    completeData.FillPiecesAreas(VAbstractValApplication::VApp()->patternUnits());

    for (int i = 0; i < measurements.size(); ++i)
    {
        const VFinalMeasurement &m = measurements.at(i);

        csv.insertSingleRow(i);
        csv.setText(i, 0, m.name); // name

        if (not m.formula.isEmpty())
        {
            try
            {
                QScopedPointer<Calculator> const cal(new Calculator());
                const qreal result = cal->EvalFormula(completeData.DataVariables(), m.formula);

                csv.setText(i, 1, VAbstractApplication::VApp()->LocaleToString(result)); // value

                if (qIsInf(result) || qIsNaN(result))
                {
                    qCritical(
                        "%s\n\n%s", qUtf8Printable(tr("Export final measurements error.")),
                        qUtf8Printable(
                            tr("Value in line %1 is infinite or NaN. Please, check your calculations.").arg(i + 1)));
                    if (not VApplication::IsGUIMode())
                    {
                        QCoreApplication::exit(V_EX_DATAERR);
                    }
                    return false;
                }
            }
            catch (qmu::QmuParserError &e)
            {
                qCritical("%s\n\n%s", qUtf8Printable(tr("Export final measurements error.")),
                          qUtf8Printable(tr("Parser error at line %1: %2.").arg(i + 1).arg(e.GetMsg())));
                if (not VApplication::IsGUIMode())
                {
                    QCoreApplication::exit(V_EX_DATAERR);
                }
                return false;
            }
        }

        csv.setText(i, 2, m.description); // description
    }

    QString error;
    const bool success = csv.toCSV(fileName, error, withHeader, separator, QTextCodec::codecForMib(mib));

    if (not success)
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Export final measurements error.")),
                  qUtf8Printable(tr("File error %1.").arg(error)));
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_CANTCREAT);
        }
    }

    return success;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::OpenMeasurementFile(const QString &patternPath, QString &path) -> QSharedPointer<VMeasurements>
{
    if (path.isEmpty())
    {
        return {};
    }

    const QStringList patternMeasurements = doc->ListMeasurements();
    QString currentPath = path;
    while (true)
    {
        QSharedPointer<VMeasurements> m = LoadMeasurements(patternPath, currentPath, path);
        if (!m || m->Type() == MeasurementsType::Unknown)
        {
            return {};
        }

        const QSet<QString> missingMeasurements = ConvertToSet<QString>(patternMeasurements)
                                                      .subtract(ConvertToSet<QString>(m->ListAll()));
        if (missingMeasurements.isEmpty())
        {
            path = currentPath;
            return m;
        }

        if (!HandleMissingMeasurements(m, missingMeasurements, currentPath))
        {
            return {};
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::LoadMeasurements(const QString &patternPath, const QString &currentPath, const QString &oldPath)
    -> QSharedPointer<VMeasurements>
{
    try
    {
        auto m = QSharedPointer<VMeasurements>(new VMeasurements(pattern));
        m->setXMLContent(currentPath);

        if (m->Type() == MeasurementsType::Unknown)
        {
            throw VException(QCoreApplication::translate("MainWindowsNoGUI", "Measurement file has unknown format."));
        }

        if (currentPath != oldPath)
        {
            VAbstractValApplication::VApp()->SetMeasurementsType(m->Type());
            if (not doc->MPath().isEmpty())
            {
                m_watcher->removePath(AbsoluteMPath(patternPath, doc->MPath()));
            }
            doc->SetMPath(RelativeMPath(patternPath, currentPath));
            m_watcher->addPath(currentPath);
        }

        const QString formatVersion = ApplyConverter(m, currentPath);
        CollectStatistics(m->Type(), formatVersion);

        return m;
    }
    catch (const VException &e)
    {
        qCCritical(vMainNoGUIWindow,
                   "%s\n\n%s\n\n%s",
                   qUtf8Printable(QCoreApplication::translate("MainWindowsNoGUI", "File error.")),
                   qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));

        if (!VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }

        return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::ApplyConverter(const QSharedPointer<VMeasurements> &m, const QString &path) const -> QString
{
    std::unique_ptr<VAbstractMConverter> converter;
    if (m->Type() == MeasurementsType::Multisize)
    {
        converter = std::make_unique<VVSTConverter>(path);
    }
    else
    {
        converter = std::make_unique<VVITConverter>(path);
    }

    m->setXMLContent(converter->Convert());
    return converter->GetFormatVersionStr();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::CollectStatistics(MeasurementsType measurementsType, const QString &formatVersion) const
{
    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (!VApplication::IsGUIMode() || !settings->IsCollectStatistic())
    {
        return;
    }

    auto *statistic = VGAnalytics::Instance();
    if (settings->GetClientID().isEmpty())
    {
        const QString clientID = QUuid::createUuid().toString();
        settings->SetClientID(clientID);
        statistic->SetClientID(clientID);
    }
    statistic->Enable(true);
    qint64 const uptime = VAbstractApplication::VApp()->AppUptime();
    if (measurementsType == MeasurementsType::Multisize)
    {
        statistic->SendMultisizeMeasurementsFormatVersion(uptime, formatVersion);
    }
    else
    {
        statistic->SendIndividualMeasurementsFormatVersion(uptime, formatVersion);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::HandleMissingMeasurements(const QSharedPointer<VMeasurements> &m,
                                                 const QSet<QString> &missing,
                                                 QString &currentPath) -> bool
{
    const QStringList missingList = ConvertToList(missing);

    if (!VApplication::IsGUIMode())
    {
        qCCritical(vMainNoGUIWindow,
                   "%s\n\n%s\n\n%s",
                   qUtf8Printable(QCoreApplication::translate("MainWindowsNoGUI", "File error.")),
                   qUtf8Printable(tr("Measurement file doesn't include all required measurements.")),
                   qUtf8Printable(tr("Please, additionally provide: %1").arg(missingList.join(", "_L1))));
        return false;
    }

    if (const QString message
        = tr("Measurement file doesn't include all required measurements.\n\nMissing: %1\n\nWould you like "
             "to select another file?")
              .arg(missingList.join(", "));
        QMessageBox::question(this, tr("Missing Measurements"), message) != QMessageBox::Yes)
    {
        m->clear();
        return false;
    }

    return SelectNewFile(m, currentPath);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindowsNoGUI::SelectNewFile(const QSharedPointer<VMeasurements> &m, QString &currentPath) -> bool
{
    QString filter;
    if (m->Type() == MeasurementsType::Multisize)
    {
        filter = tr("Multisize measurements") + " (*.vst);;"_L1 + tr("Individual measurements") + " (*.vit)"_L1;
    }
    else
    {
        filter = tr("Individual measurements") + " (*.vit);;"_L1 + tr("Multisize measurements") + " (*.vst)"_L1;
    }

    const QString dirPath = QFileInfo(currentPath).absolutePath();
    QFileDialog dialog(this, tr("Select Measurement File"), dirPath, filter);
    dialog.selectFile(QFileInfo(currentPath).fileName());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog,
                     VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

    if (dialog.exec() == QDialog::Accepted)
    {
        currentPath = dialog.selectedFiles().value(0);

        if (const VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
            m->Type() == MeasurementsType::Multisize)
        {
            settings->SetPathMultisizeMeasurements(currentPath);
        }
        else
        {
            settings->SetPathIndividualMeasurements(currentPath);
        }
    }
    else
    {
        m->clear();
        return false;
    }

    return true;
}
