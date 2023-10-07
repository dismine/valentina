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
#include "../vlayout/vlayoutgenerator.h"
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
#include "../vmisc/vtextcodec.h"
#else
#include <QTextCodec>
#endif

#include <QDebug>
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

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

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
        QDir dir(path);
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
    pattern(new VContainer(VAbstractApplication::VApp()->TrVars(), VAbstractValApplication::VApp()->patternUnitsP(),
                           valentinaNamespace))
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    ,
    m_taskbarButton(new QWinTaskbarButton(this))
#endif
{
    InitTempLayoutScene();

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    m_taskbarButton->setWindow(this->windowHandle());
    m_taskbarProgress = m_taskbarButton->progress();
    m_taskbarProgress->setMinimum(0);
#endif

    m_layoutSettings->SetParentWidget(this);
}

//---------------------------------------------------------------------------------------------------------------------
MainWindowsNoGUI::~MainWindowsNoGUI()
{
    delete m_unreadPatternMessage;
    delete m_mouseCoordinate;
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
    lGenerator.SetDetails(listDetails);

    QElapsedTimer timer;
    timer.start();

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QTimer *progressTimer = nullptr;
#endif

    QSharedPointer<DialogLayoutProgress> progress;
    if (VApplication::IsGUIMode())
    {
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
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
                    const int timeout = static_cast<int>(lGenerator.GetNestingTimeMSecs() - timer.elapsed());

                    if (timeout <= 1000)
                    {
                        lGenerator.Timeout();
                        progressTimer->stop();
                        progressTimer->deleteLater();
                    }
                });
        progressTimer->start(1000);
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
            fw.setFuture(
                QtConcurrent::run([&lGenerator, timer, nestingState]()
                                  { lGenerator.Generate(timer, lGenerator.GetNestingTimeMSecs(), nestingState); }));
            QObject::connect(&fw, &QFutureWatcher<void>::finished, &wait, &QEventLoop::quit);
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

                        CleanLayout();
                        m_layoutSettings->SetLayoutPapers(lGenerator.GetPapersItems());      // Blank sheets
                        m_layoutSettings->SetLayoutDetails(lGenerator.GetAllDetailsItems()); // All details items
                        detailsOnLayout = lGenerator.GetAllDetails();                        // All details items
                        m_layoutSettings->SetLayoutShadows(CreateShadows(m_layoutSettings->LayoutPapers()));
                        m_layoutSettings->SetLayoutPortrait(lGenerator.IsPortrait());
                        m_layoutSettings->SetLayoutScenes(CreateScenes(m_layoutSettings->LayoutPapers(),
                                                                       m_layoutSettings->LayoutShadows(),
                                                                       m_layoutSettings->LayoutDetails()));
#if !defined(V_NO_ASSERT)
                        // Uncomment to debug, shows global contour
//                        gcontours = lGenerator.GetGlobalContours(); // uncomment for debugging
//                        InsertGlobalContours(scenes, gcontours); // uncomment for debugging
#endif
                        if (VApplication::IsGUIMode())
                        {
                            PrepareSceneList(PreviewQuatilty::Fast);
                        }
                        m_layoutSettings->SetIgnorePrinterMargins(not lGenerator.IsUsePrinterFields());
                        m_layoutSettings->SetLayoutMargins(lGenerator.GetPrinterFields());
                        m_layoutSettings->SetLayoutPaperSize(
                            QSizeF(lGenerator.GetPaperWidth(), lGenerator.GetPaperHeight()));
                        m_layoutSettings->SetAutoCropLength(lGenerator.GetAutoCropLength());
                        m_layoutSettings->SetAutoCropWidth(lGenerator.GetAutoCropWidth());
                        m_layoutSettings->SetUnitePages(lGenerator.IsUnitePages());
                        m_layoutSettings->SetLayoutStale(false);
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
            efficiency >= lGenerator.GetEfficiencyCoefficient())
        {
            if (not lGenerator.IsPreferOneSheetSolution() || lGenerator.PapersCount() == 1)
            {
                break;
            }
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

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
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
    QString fileName = CSVFilePath();

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
    const LayoutExportFormats format = m_dialogSaveLayout->Format();

    if (format == LayoutExportFormats::DXF_AAMA || format == LayoutExportFormats::DXF_ASTM ||
        format == LayoutExportFormats::RLD || format == LayoutExportFormats::HPGL ||
        format == LayoutExportFormats::HPGL2)
    {
        if (m_dialogSaveLayout->Mode() == Draw::Layout)
        {
            for (int i = 0; i < detailsOnLayout.size(); ++i)
            {
                const QString name = m_dialogSaveLayout->Path() + '/' + m_dialogSaveLayout->FileName() +
                                     QString::number(i + 1) +
                                     VLayoutExporter::ExportFormatSuffix(m_dialogSaveLayout->Format());

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
    bool usedNotExistedDir = CreateLayoutPath(path);
    if (not usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    VAbstractValApplication::VApp()->ValentinaSettings()->SetPathLayout(path);
    const LayoutExportFormats format = m_dialogSaveLayout->Format();

    if (format == LayoutExportFormats::PDFTiled && m_dialogSaveLayout->Mode() == Draw::Layout)
    {
        const QString name = path + '/'_L1 + m_dialogSaveLayout->FileName() + '1'_L1;
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

    QScopedPointer<QGraphicsScene> scene(new QGraphicsScene());

    QList<QGraphicsItem *> list;
    list.reserve(listDetails.count());
    for (auto piece : listDetails)
    {
        QGraphicsItem *item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths());
        qreal diff = 0;
        if (piece.IsForceFlipping())
        {
            const qreal x = item->boundingRect().x();
            piece.Mirror();
            item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths());
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

    QGraphicsRectItem *paper = new QGraphicsRectItem(rect);
    paper->setPen(QPen(Qt::black, 1));
    paper->setBrush(QBrush(Qt::white));
    papers.append(paper);

    QList<QList<QGraphicsItem *>> details; // All details
    details.append(list);

    QList<QGraphicsItem *> shadows = CreateShadows(papers);
    QList<QGraphicsScene *> scenes = CreateScenes(papers, shadows, details);

    const bool ignorePrinterFields = false;
    const qreal margin = ToPixel(1, Unit::Cm);
    ExportFlatLayout(scenes, papers, shadows, details, ignorePrinterFields, QMarginsF(margin, margin, margin, margin));

    qDeleteAll(scenes); // Scene will clear all other items
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportApparelLayout(const QVector<VLayoutPiece> &details, const QString &name,
                                           const QSize &size) const
{
    const QString path = m_dialogSaveLayout->Path();
    bool usedNotExistedDir = CreateLayoutPath(path);
    if (not usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetPathLayout(path);
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

    switch (format)
    {
        case LayoutExportFormats::DXF_ASTM:
            exporter.SetDxfVersion(DRW::AC1009);
            exporter.ExportToASTMDXF(details);
            break;
        case LayoutExportFormats::DXF_AAMA:
            exporter.SetDxfVersion(DRW::AC1009);
            exporter.ExportToAAMADXF(details);
            break;
        case LayoutExportFormats::RLD:
            exporter.ExportToRLD(details);
            break;
        case LayoutExportFormats::HPGL:
            exporter.SetSingleLineFont(settings->GetSingleLineFonts());
            exporter.SetSingleStrokeOutlineFont(settings->GetSingleStrokeOutlineFont());
            exporter.SetPenWidth(qCeil(settings->WidthHairLine()));
            exporter.ExportToHPGL(details);
            break;
        case LayoutExportFormats::HPGL2:
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

    QScopedPointer<QGraphicsScene> scene(new QGraphicsScene());

    QList<QGraphicsItem *> list;
    list.reserve(listDetails.count());
    for (int i = 0; i < listDetails.count(); ++i)
    {
        VLayoutPiece piece = listDetails.at(i);
        QGraphicsItem *item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths());
        qreal diff = 0;
        if (piece.IsForceFlipping())
        {
            const qreal x = item->boundingRect().x();
            piece.Mirror();
            delete item;
            item = piece.GetItem(m_dialogSaveLayout->IsTextAsPaths());
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

    const QString name = m_dialogSaveLayout->Path() + '/' + m_dialogSaveLayout->FileName() + QString::number(1) +
                         VLayoutExporter::ExportFormatSuffix(m_dialogSaveLayout->Format());

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

        VAbstractLayoutDialog::PaperSizeTemplate tiledFormat = m_dialogSaveLayout->GetTiledPageFormat();
        m_layoutSettings->SetTiledPDFPaperSize(VAbstractLayoutDialog::GetTemplateSize(tiledFormat, Unit::Mm));
    }
    else
    {
        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
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

        VAbstractLayoutDialog::PaperSizeTemplate tiledFormat = m_dialogSaveLayout->GetTiledPageFormat();
        m_layoutSettings->SetTiledPDFPaperSize(VAbstractLayoutDialog::GetTemplateSize(tiledFormat, Unit::Mm));
    }
    else
    {
        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
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

    std::function<VLayoutPiece(const DetailForLayout &data)> PrepareDetail = [](const DetailForLayout &data)
    {
        auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(data.id));
        SCASSERT(tool != nullptr)
        return VLayoutPiece::Create(data.piece, data.id, tool->getData());
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

    futureWatcher.setFuture(QtConcurrent::mapped(details, PrepareDetail));

    if (VApplication::VApp()->IsGUIMode())
    {
        progress.exec();
    }

    futureWatcher.waitForFinished();

    QVector<VLayoutPiece> layoutDetails;
    layoutDetails.reserve(details.size());
    const QFuture<VLayoutPiece> future = futureWatcher.future();

    QFuture<VLayoutPiece>::const_iterator i;
    for (i = future.constBegin(); i != future.constEnd(); ++i)
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
    auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(m_layoutSettings->LayoutPapers().at(i));
    if (paper)
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
        for (const auto &detail : paperDetails)
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

        VAbstractLayoutDialog::PaperSizeTemplate tiledFormat = m_dialogSaveLayout->GetTiledPageFormat();
        m_layoutSettings->SetTiledPDFPaperSize(VAbstractLayoutDialog::GetTemplateSize(tiledFormat, Unit::Mm));

        m_layoutSettings->SetXScale(m_dialogSaveLayout->GetXScale());
        m_layoutSettings->SetYScale(m_dialogSaveLayout->GetYScale());
    }
    else
    {
        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
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

    for (int i = 0; i < scenes.size(); ++i)
    {
        auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
        if (paper != nullptr)
        {
            const QString name = m_dialogSaveLayout->Path() + '/' + m_dialogSaveLayout->FileName() +
                                 QString::number(i + 1) +
                                 VLayoutExporter::ExportFormatSuffix(m_dialogSaveLayout->Format());
            auto *brush = new QBrush();
            brush->setColor(QColor(Qt::white));
            QGraphicsScene *scene = scenes.at(i);
            scene->setBackgroundBrush(*brush);
            shadows[i]->setVisible(false);
            const qreal thinPen = 0.1;
            paper->setPen(QPen(QBrush(Qt::white, Qt::NoBrush), thinPen, Qt::NoPen));

            exporter.SetFileName(name);
            exporter.SetImageRect(paper->rect());

            QPen defaultPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(), Qt::SolidLine,
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

    csv.insertColumn(0);
    csv.insertColumn(1);
    csv.insertColumn(2);

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

        csv.insertRow(i);
        csv.setText(i, 0, m.name); // name

        if (not m.formula.isEmpty())
        {
            try
            {
                QScopedPointer<Calculator> cal(new Calculator());
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
    const bool success = csv.toCSV(fileName, error, withHeader, separator, VTextCodec::codecForMib(mib));

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
auto MainWindowsNoGUI::OpenMeasurementFile(const QString &path) const -> QSharedPointer<VMeasurements>
{
    QSharedPointer<VMeasurements> m;
    if (path.isEmpty())
    {
        return m;
    }

    try
    {
        m = QSharedPointer<VMeasurements>(new VMeasurements(pattern));
        m->setXMLContent(path);

        if (m->Type() == MeasurementsType::Unknown)
        {
            throw VException(QCoreApplication::translate("MainWindowsNoGUI", "Measurement file has unknown format."));
        }

        if (m->Type() == MeasurementsType::Multisize)
        {
            VVSTConverter converter(path);
            m->setXMLContent(converter.Convert()); // Read again after conversion

            VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
            if (settings->IsCollectStatistic())
            {
                auto *statistic = VGAnalytics::Instance();

                QString clientID = settings->GetClientID();
                if (clientID.isEmpty())
                {
                    clientID = QUuid::createUuid().toString();
                    settings->SetClientID(clientID);
                    statistic->SetClientID(clientID);
                }

                statistic->Enable(true);

                const qint64 uptime = VAbstractApplication::VApp()->AppUptime();
                statistic->SendMultisizeMeasurementsFormatVersion(uptime, converter.GetFormatVersionStr());
            }
        }
        else
        {
            VVITConverter converter(path);
            m->setXMLContent(converter.Convert()); // Read again after conversion

            VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
            if (settings->IsCollectStatistic())
            {
                auto *statistic = VGAnalytics::Instance();

                QString clientID = settings->GetClientID();
                if (clientID.isEmpty())
                {
                    clientID = QUuid::createUuid().toString();
                    settings->SetClientID(clientID);
                    statistic->SetClientID(clientID);
                }

                statistic->Enable(true);

                const qint64 uptime = VAbstractApplication::VApp()->AppUptime();
                statistic->SendIndividualMeasurementsFormatVersion(uptime, converter.GetFormatVersionStr());
            }
        }

        if (not m->IsDefinedKnownNamesValid())
        {
            throw VException(QCoreApplication::translate("MainWindowsNoGUI",
                                                         "Measurement file contains invalid known measurement(s)."));
        }

        CheckRequiredMeasurements(m.data());
    }
    catch (VException &e)
    {
        qCCritical(vMainNoGUIWindow, "%s\n\n%s\n\n%s",
                   qUtf8Printable(QCoreApplication::translate("MainWindowsNoGUI", "File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        if (not m.isNull())
        {
            m->clear();
        }

        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return m;
    }
    return m;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::CheckRequiredMeasurements(const VMeasurements *m) const
{
    const QSet<QString> match =
        ConvertToSet<QString>(doc->ListMeasurements()).subtract(ConvertToSet<QString>(m->ListAll()));

    if (not match.isEmpty())
    {
        QList<QString> list = ConvertToList(match);
        for (int i = 0; i < list.size(); ++i)
        {
            list[i] = VAbstractApplication::VApp()->TrVars()->MToUser(list.at(i));
        }

        VException e(tr("Measurement file doesn't include all required measurements."));
        e.AddMoreInformation(tr("Please, additionally provide: %1").arg(QStringList(list).join(QStringLiteral(", "))));
        throw e;
    }
}
