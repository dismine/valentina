/************************************************************************
 **
 **  @file   mainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "mainwindow.h"

#include <QAction>
#include <QComboBox>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QGlobalStatic>
#include <QImageReader>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QScrollBar>
#include <QSettings>
#include <QShowEvent>
#include <QStyleFactory>
#include <QTimer>
#include <QUndoStack>
#include <QUuid>
#include <QtConcurrent>
#include <QtDebug>
#include <QtGlobal>
#include <chrono>
#include <memory>
#include <thread>

#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptioninvalidhistory.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionundo.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/xml/utils.h"
#include "../ifc/xml/vbackgroundpatternimage.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../vformat/vmeasurements.h"
#include "../vformat/vpatternrecipe.h"
#include "../vganalytics/vganalytics.h"
#include "../vlayout/dialogs/dialoglayoutscale.h"
#include "../vlayout/dialogs/watermarkwindow.h"
#include "../vlayout/vlayoutexporter.h"
#include "../vmisc/customevents.h"
#include "../vmisc/def.h"
#include "../vmisc/dialogs/dialogaskcollectstatistic.h"
#include "../vmisc/dialogs/dialogselectlanguage.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vmodifierkey.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vtools/dialogs/support/dialogeditlabel.h"
#include "../vtools/dialogs/tools/dialogalongline.h"
#include "../vtools/dialogs/tools/dialogarc.h"
#include "../vtools/dialogs/tools/dialogarcwithlength.h"
#include "../vtools/dialogs/tools/dialogbisector.h"
#include "../vtools/dialogs/tools/dialogcubicbezier.h"
#include "../vtools/dialogs/tools/dialogcubicbezierpath.h"
#include "../vtools/dialogs/tools/dialogcurveintersectaxis.h"
#include "../vtools/dialogs/tools/dialogcutarc.h"
#include "../vtools/dialogs/tools/dialogcutspline.h"
#include "../vtools/dialogs/tools/dialogcutsplinepath.h"
#include "../vtools/dialogs/tools/dialogellipticalarc.h"
#include "../vtools/dialogs/tools/dialogendline.h"
#include "../vtools/dialogs/tools/dialogflippingbyaxis.h"
#include "../vtools/dialogs/tools/dialogflippingbyline.h"
#include "../vtools/dialogs/tools/dialoggroup.h"
#include "../vtools/dialogs/tools/dialogheight.h"
#include "../vtools/dialogs/tools/dialogline.h"
#include "../vtools/dialogs/tools/dialoglineintersect.h"
#include "../vtools/dialogs/tools/dialoglineintersectaxis.h"
#include "../vtools/dialogs/tools/dialogmove.h"
#include "../vtools/dialogs/tools/dialognormal.h"
#include "../vtools/dialogs/tools/dialogpointfromarcandtangent.h"
#include "../vtools/dialogs/tools/dialogpointfromcircleandtangent.h"
#include "../vtools/dialogs/tools/dialogpointofcontact.h"
#include "../vtools/dialogs/tools/dialogpointofintersection.h"
#include "../vtools/dialogs/tools/dialogpointofintersectionarcs.h"
#include "../vtools/dialogs/tools/dialogpointofintersectioncircles.h"
#include "../vtools/dialogs/tools/dialogpointofintersectioncurves.h"
#include "../vtools/dialogs/tools/dialogrotation.h"
#include "../vtools/dialogs/tools/dialogshoulderpoint.h"
#include "../vtools/dialogs/tools/dialogspline.h"
#include "../vtools/dialogs/tools/dialogsplinepath.h"
#include "../vtools/dialogs/tools/dialogtriangle.h"
#include "../vtools/dialogs/tools/dialogtruedarts.h"
#include "../vtools/dialogs/tools/dialoguniondetails.h"
#include "../vtools/dialogs/tools/piece/dialogduplicatedetail.h"
#include "../vtools/dialogs/tools/piece/dialoginsertnode.h"
#include "../vtools/dialogs/tools/piece/dialogpiecepath.h"
#include "../vtools/dialogs/tools/piece/dialogpin.h"
#include "../vtools/dialogs/tools/piece/dialogplacelabel.h"
#include "../vtools/dialogs/tools/piece/dialogseamallowance.h"
#include "../vtools/tools/backgroundimage/vbackgroundimagecontrols.h"
#include "../vtools/tools/backgroundimage/vbackgroundimageitem.h"
#include "../vtools/tools/backgroundimage/vbackgroundpixmapitem.h"
#include "../vtools/tools/backgroundimage/vbackgroundsvgitem.h"
#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyaxis.h"
#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyline.h"
#include "../vtools/tools/drawTools/operation/vtoolmove.h"
#include "../vtools/tools/drawTools/operation/vtoolrotation.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarcwithlength.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezier.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezierpath.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolellipticalarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolspline.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/tooldoublepoint/vtooltruedarts.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolalongline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolbisector.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolcurveintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolheight.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollineintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolshoulderpoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolbasepoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoollineintersect.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromarcandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromcircleandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofcontact.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersection.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectionarcs.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncircles.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../vtools/tools/drawTools/vtoolline.h"
#include "../vtools/tools/nodeDetails/vtoolpiecepath.h"
#include "../vtools/tools/nodeDetails/vtoolpin.h"
#include "../vtools/tools/nodeDetails/vtoolplacelabel.h"
#include "../vtools/tools/vtoolseamallowance.h"
#include "../vtools/tools/vtooluniondetails.h"
#include "../vtools/undocommands/image/addbackgroundimage.h"
#include "../vtools/undocommands/image/deletebackgroundimage.h"
#include "../vtools/undocommands/renamepp.h"
#include "../vtools/undocommands/undogroup.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vtoolbuttonpopup.h"
#include "../vwidgets/vwidgetpopup.h"
#include "core/vapplication.h"
#include "core/vtooloptionspropertybrowser.h"
#include "dialogs/dialogaboutapp.h"
#include "dialogs/dialogaddbackgroundimage.h"
#include "dialogs/dialogfinalmeasurements.h"
#include "dialogs/dialoghistory.h"
#include "dialogs/dialogincrements.h"
#include "dialogs/dialognewpattern.h"
#include "dialogs/dialogpatternproperties.h"
#include "dialogs/dialogpreferences.h"
#include "dialogs/vwidgetbackgroundimages.h"
#include "dialogs/vwidgetdetails.h"
#include "dialogs/vwidgetgroups.h"
#include "ui_mainwindow.h"
#include "vabstractapplication.h"
#include "vsinglelineoutlinechar.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "../vmisc/vtextcodec.h"
#else
#include <QTextCodec>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QSourceLocation>
#endif

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#if defined(Q_OS_MAC)
#include <QDrag>
#include <QMimeData>
#endif // defined(Q_OS_MAC)

#if (defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)
// DO NOT WORK WITH GCC 4.8
#else
#if __cplusplus >= 201402L
using namespace std::chrono_literals;
#else
#include "../vmisc/bpstd/chrono.hpp"
using namespace bpstd::literals::chrono_literals;
#endif // __cplusplus >= 201402L
#endif //(defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vMainWindow, "v.mainwindow") // NOLINT

QT_WARNING_POP

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, autosavePrefix, (".autosave"_L1)) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
void LogPatternToolUsed(bool checked, const QString &toolName)
{
    if (checked)
    {
        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
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

            const qint64 uptime = VAbstractValApplication::VApp()->AppUptime();
            statistic->SendPatternToolUsedEvent(uptime, toolName);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SortDetailsForLayout(const QHash<quint32, VPiece> *allDetails, const QString &nameRegex = QString())
    -> QVector<DetailForLayout>
{
    QVector<DetailForLayout> details;
    QHash<quint32, VPiece>::const_iterator i = allDetails->constBegin();

    if (nameRegex.isEmpty())
    {
        while (i != allDetails->constEnd())
        {
            if (i.value().IsInLayout())
            {
                details.append(DetailForLayout(i.key(), i.value()));
            }

            ++i;
        }
    }
    else
    {
        QRegularExpression nameRe(nameRegex);
        while (i != allDetails->constEnd())
        {
            if (nameRe.match(i.value().GetName()).hasMatch())
            {
                details.append(DetailForLayout(i.key(), i.value()));
            }

            ++i;
        }
    }

    return details;
}

//---------------------------------------------------------------------------------------------------------------------
void WarningNotUniquePieceName(const QHash<quint32, VPiece> *allDetails)
{
    QHash<quint32, VPiece>::const_iterator i = allDetails->constBegin();
    QSet<QString> uniqueNames;

    while (i != allDetails->constEnd())
    {
        const QString pieceName = i.value().GetName();
        if (not uniqueNames.contains(pieceName))
        {
            uniqueNames.insert(pieceName);
        }
        else
        {
            const QString errorMsg = QObject::tr("Piece name '%1' is not unique.").arg(pieceName);
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }

        ++i;
    }
}

} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MainWindow constructor.
 * @param parent parent widget.
 */
MainWindow::MainWindow(QWidget *parent)
  : MainWindowsNoGUI(parent),
    ui(new Ui::MainWindow),
    m_watcher(new QFileSystemWatcher(this)),
    m_dialogTable(nullptr),
    m_dialogHistory(nullptr),
    m_dialogFMeasurements(nullptr),
    m_measurementsSyncTimer(new QTimer(this)),
    m_progressBar(new QProgressBar(this)),
    m_statusLabel(new QLabel(this)),
    m_gradation(new QTimer(this))
{
    CreateActions();
    InitScenes();

    connect(m_gradation, &QTimer::timeout, this, &MainWindow::GradationChanged);

    doc = new VPattern(pattern, m_sceneDraw, m_sceneDetails);
    connect(doc, &VPattern::ClearMainWindow, this, &MainWindow::Clear);
    connect(doc, &VPattern::patternChanged, this, &MainWindow::PatternChangesWereSaved);
    connect(doc, &VPattern::UndoCommand, this, &MainWindow::FullParseFile);
    connect(doc, &VPattern::SetEnabledGUI, this, &MainWindow::SetEnabledGUI);
    connect(doc, &VPattern::CheckLayout, this,
            [this]()
            {
                if (pattern->DataPieces()->count() == 0)
                {
                    if (not ui->actionDraw->isChecked())
                    {
                        ActionDraw(true);
                    }
                }
            });
    connect(doc, &VPattern::SetCurrentPP, this, &MainWindow::GlobalChangePP);
    connect(doc, &VPattern::MadeProgress, this, &MainWindow::ShowProgress);
    VAbstractValApplication::VApp()->setCurrentDocument(doc);

    InitDocksContain();
    CreateMenus();
    ToolBarDraws();
    ToolBarStages();
    ToolBarDrawTools();
    InitToolButtons();

    connect(ui->actionAddBackgroundImage, &QAction::triggered, this, &MainWindow::ActionAddBackgroundImage);
    connect(ui->actionExportFontCorrections, &QAction::triggered, this, &MainWindow::ActionExportFontCorrections);

    m_progressBar->setVisible(false);
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    m_taskbarProgress->setVisible(false);
#endif
    m_statusLabel->setText(tr("Create new pattern piece to start working."));
    statusBar()->addPermanentWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_progressBar, 1);

    ToolBarTools();

    connect(VAbstractApplication::VApp()->getUndoStack(), &QUndoStack::cleanChanged, this,
            &MainWindow::PatternChangesWereSaved);

    InitAutoSave();

    ReadSettings();

    setCurrentFile(QString());
    WindowsLocale();

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::ShowPaper);
    ui->dockWidgetLayoutPages->setVisible(false);

    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::MeasurementsChanged);

    m_measurementsSyncTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(m_measurementsSyncTimer, &QTimer::timeout, this,
            [this]()
            {
                if (isActiveWindow())
                {
                    static bool asking = false;
                    if (not asking && m_mChanges && not m_mChangesAsked)
                    {
                        asking = true;
                        m_mChangesAsked = true;
                        m_measurementsSyncTimer->stop();
                        const auto answer = QMessageBox::question(
                            this, tr("Measurements"),
                            tr("Measurements were changed. Do you want to sync measurements now?"),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                        if (answer == QMessageBox::Yes)
                        {
                            SyncMeasurements();
                        }
                        asking = false;
                    }
                }
            });

#if defined(Q_OS_MAC)
    // On Mac deafault icon size is 32x32.
    ui->toolBarDraws->setIconSize(QSize(24, 24));
    ui->toolBarOption->setIconSize(QSize(24, 24));
    ui->toolBarStages->setIconSize(QSize(24, 24));
    ui->toolBarTools->setIconSize(QSize(24, 24));

    setUnifiedTitleAndToolBarOnMac(true);

    // Mac OS Dock Menu
    QMenu *menu = new QMenu(this);

    QAction *actionNewPattern = menu->addAction(tr("New pattern"));
    actionNewPattern->setMenuRole(QAction::NoRole);
    connect(actionNewPattern, &QAction::triggered, this, &MainWindow::on_actionNew_triggered);

    QAction *actionOpenPattern = menu->addAction(tr("Open pattern"));
    actionOpenPattern->setMenuRole(QAction::NoRole);
    connect(actionOpenPattern, &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);

    QAction *actionOpenTape = menu->addAction(tr("Create/Edit measurements"));
    actionOpenTape->setMenuRole(QAction::NoRole);
    connect(actionOpenTape, &QAction::triggered, this, &MainWindow::CreateMeasurements);

    QAction *actionPreferences = menu->addAction(tr("Preferences"));
    actionPreferences->setMenuRole(QAction::NoRole);
    connect(actionPreferences, &QAction::triggered, this, &MainWindow::Preferences);

    menu->setAsDockMenu();
#endif // defined(Q_OS_MAC)

    connect(ui->toolButtonMessagesZoomInFont, &QToolButton::clicked, this,
            [this]()
            {
                VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
                QFont f = ui->plainTextEditPatternMessages->font();
                if (f.pointSize() < VValentinaSettings::GetDefMaxPatternMessageFontSize())
                {
                    f.setPointSize(qMax(f.pointSize() + 1, 1));
                    ui->plainTextEditPatternMessages->setFont(f);
                    settings->SetPatternMessageFontSize(f.pointSize());
                }
            });

    connect(ui->toolButtonMessagesZoomOutFont, &QToolButton::clicked, this,
            [this]()
            {
                VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
                QFont f = ui->plainTextEditPatternMessages->font();
                if (f.pointSize() > VValentinaSettings::GetDefMinPatternMessageFontSize())
                {
                    f.setPointSize(qMax(f.pointSize() - 1, 1));
                    ui->plainTextEditPatternMessages->setFont(f);
                    settings->SetPatternMessageFontSize(f.pointSize());
                }
            });

    connect(ui->lineEditMessagesFilter, &QLineEdit::textChanged, this,
            [this](const QString &text) { ui->plainTextEditPatternMessages->SetFilter(text); });

    connect(ui->toolButtonClearMessages, &QToolButton::clicked, this, &MainWindow::ClearPatternMessages);

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    ui->toolButtonAutoRefresh->setChecked(settings->GetAutoRefreshPatternMessage());
    connect(ui->toolButtonAutoRefresh, &QToolButton::clicked, this,
            [](bool checked)
            { VAbstractValApplication::VApp()->ValentinaSettings()->SetAutoRefreshPatternMessage(checked); });

    connect(doc, &VPattern::PreParseState, this,
            [this]()
            {
                if (ui->toolButtonAutoRefresh->isChecked())
                {
                    ClearPatternMessages();
                }
            });

    if (VApplication::IsGUIMode())
    {
        QTimer::singleShot(V_SECONDS(1), this, &MainWindow::AskDefaultSettings);
    }

    ui->actionExportFontCorrections->setEnabled(settings->GetSingleStrokeOutlineFont());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::AddPP(const QString &PPName)
{
    if (not doc->appendPP(PPName))
    {
        qCDebug(vMainWindow, "Error creating pattern piece with the name %s.", qUtf8Printable(PPName));
        return;
    }

    if (m_comboBoxDraws->count() == 0)
    {
        m_sceneDraw->InitOrigins();
        m_sceneDetails->InitOrigins();
    }

    m_comboBoxDraws->blockSignals(true);
    m_comboBoxDraws->addItem(PPName);

    pattern->ClearCalculationGObjects();
    // Create single point
    emit ui->view->itemClicked(nullptr); // hide options previous tool
    const QString label = doc->GenerateLabel(LabelType::NewPatternPiece);
    const QPointF startPosition = StartPositionNewPP();

    VToolBasePointInitData initData;
    initData.scene = m_sceneDraw;
    initData.doc = doc;
    initData.data = pattern;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.x = startPosition.x();
    initData.y = startPosition.y();
    initData.name = label;
    initData.nameActivPP = PPName;

    auto *spoint = VToolBasePoint::Create(initData);
    emit ui->view->itemClicked(spoint);

    SetEnableTool(true);
    SetEnableWidgets(true);

    const qint32 index = m_comboBoxDraws->findText(PPName);
    if (index != -1)
    { // -1 for not found
        m_comboBoxDraws->setCurrentIndex(index);
    }
    else
    {
        m_comboBoxDraws->setCurrentIndex(0);
    }
    m_comboBoxDraws->blockSignals(false);

    // Show best for new PP
    VMainGraphicsView::NewSceneRect(ui->view->scene(), ui->view, spoint);
    // ZoomFitBestCurrent();
    QApplication::postEvent(this, new FitBestCurrentEvent());

    ui->actionNewDraw->setEnabled(true);
    m_statusLabel->setText(QString());
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::StartPositionNewPP() const -> QPointF
{
    const qreal originX = 30.0;
    const qreal originY = 40.0;
    const qreal margin = 40.0;
    if (m_comboBoxDraws->count() > 1)
    {
        const QRectF rect = m_sceneDraw->VisibleItemsBoundingRect();
        if (rect.width() <= rect.height())
        {
            return {rect.width() + margin, originY};
        }

        return {originX, rect.height() + margin};
    }

    return {originX, originY};
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitScenes()
{
    m_sceneDraw = new VMainGraphicsScene(this);
    currentScene = m_sceneDraw;
    VAbstractValApplication::VApp()->setCurrentScene(&currentScene);
    connect(this, &MainWindow::EnableItemMove, m_sceneDraw, &VMainGraphicsScene::EnableItemMove);
    connect(this, &MainWindow::ItemsSelection, m_sceneDraw, &VMainGraphicsScene::ItemsSelection);

    connect(this, &MainWindow::EnableLabelSelection, m_sceneDraw, &VMainGraphicsScene::ToggleLabelSelection);
    connect(this, &MainWindow::EnablePointSelection, m_sceneDraw, &VMainGraphicsScene::TogglePointSelection);
    connect(this, &MainWindow::EnableLineSelection, m_sceneDraw, &VMainGraphicsScene::ToggleLineSelection);
    connect(this, &MainWindow::EnableArcSelection, m_sceneDraw, &VMainGraphicsScene::ToggleArcSelection);
    connect(this, &MainWindow::EnableElArcSelection, m_sceneDraw, &VMainGraphicsScene::ToggleElArcSelection);
    connect(this, &MainWindow::EnableSplineSelection, m_sceneDraw, &VMainGraphicsScene::ToggleSplineSelection);
    connect(this, &MainWindow::EnableSplinePathSelection, m_sceneDraw, &VMainGraphicsScene::ToggleSplinePathSelection);

    connect(this, &MainWindow::EnableLabelHover, m_sceneDraw, &VMainGraphicsScene::ToggleLabelHover);
    connect(this, &MainWindow::EnablePointHover, m_sceneDraw, &VMainGraphicsScene::TogglePointHover);
    connect(this, &MainWindow::EnableLineHover, m_sceneDraw, &VMainGraphicsScene::ToggleLineHover);
    connect(this, &MainWindow::EnableArcHover, m_sceneDraw, &VMainGraphicsScene::ToggleArcHover);
    connect(this, &MainWindow::EnableElArcHover, m_sceneDraw, &VMainGraphicsScene::ToggleElArcHover);
    connect(this, &MainWindow::EnableSplineHover, m_sceneDraw, &VMainGraphicsScene::ToggleSplineHover);
    connect(this, &MainWindow::EnableSplinePathHover, m_sceneDraw, &VMainGraphicsScene::ToggleSplinePathHover);

    connect(m_sceneDraw, &VMainGraphicsScene::mouseMove, this, &MainWindow::MouseMove);
    connect(m_sceneDraw, &VMainGraphicsScene::AddBackgroundImage, this, &MainWindow::PlaceBackgroundImage);

    m_sceneDetails = new VMainGraphicsScene(this);
    connect(this, &MainWindow::EnableItemMove, m_sceneDetails, &VMainGraphicsScene::EnableItemMove);

    connect(this, &MainWindow::EnableNodeLabelSelection, m_sceneDetails, &VMainGraphicsScene::ToggleNodeLabelSelection);
    connect(this, &MainWindow::EnableNodePointSelection, m_sceneDetails, &VMainGraphicsScene::ToggleNodePointSelection);
    connect(this, &MainWindow::EnableDetailSelection, m_sceneDetails, &VMainGraphicsScene::ToggleDetailSelection);

    connect(this, &MainWindow::EnableNodeLabelHover, m_sceneDetails, &VMainGraphicsScene::ToggleNodeLabelHover);
    connect(this, &MainWindow::EnableNodePointHover, m_sceneDetails, &VMainGraphicsScene::ToggleNodePointHover);
    connect(this, &MainWindow::EnableDetailHover, m_sceneDetails, &VMainGraphicsScene::ToggleDetailHover);

    connect(m_sceneDetails, &VMainGraphicsScene::mouseMove, this, &MainWindow::MouseMove);

    ui->view->setScene(currentScene);

    m_sceneDraw->setTransform(ui->view->transform());
    m_sceneDetails->setTransform(ui->view->transform());

    connect(ui->view, &VMainGraphicsView::MouseRelease, this, [this]() { EndVisualization(true); });
    connect(ui->view, &VMainGraphicsView::ScaleChanged, this, &MainWindow::ScaleChanged);
    connect(ui->view, &VMainGraphicsView::ZoomFitBestCurrent, this, [this]() { ZoomFitBestCurrent(); });
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHorizontalStretch(12);
    ui->view->setSizePolicy(policy);
    VAbstractValApplication::VApp()->setSceneView(ui->view);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::LoadMeasurements(const QString &path) -> bool
{
    m_m = OpenMeasurementFile(path);

    if (m_m->isNull())
    {
        return false;
    }

    try
    {
        VAbstractValApplication::VApp()->SetMeasurementsType(m_m->Type());
        if (m_m->Type() == MeasurementsType::Individual)
        {
            VAbstractValApplication::VApp()->SetCustomerName(m_m->Customer());
            VAbstractValApplication::VApp()->SetCustomerBirthDate(m_m->BirthDate());
            VAbstractValApplication::VApp()->SetCustomerEmail(m_m->Email());
        }
        else if (m_m->Type() == MeasurementsType::Multisize)
        {
            auto DimensionBase = [](qreal current, qreal tableBase)
            { return not VFuzzyComparePossibleNulls(current, -1) ? current : tableBase; };

            m_currentDimensionA = DimensionBase(doc->GetDimensionAValue(), m_m->DimensionABase());
            m_currentDimensionB = DimensionBase(doc->GetDimensionBValue(), m_m->DimensionBBase());
            m_currentDimensionC = DimensionBase(doc->GetDimensionCValue(), m_m->DimensionCBase());
        }
        ToolBarOption();
        SetDimensionBases();
        pattern->ClearVariables({VarType::Measurement, VarType::MeasurementSeparator});
        m_m->StoreNames(false);
        m_m->ReadMeasurements(m_currentDimensionA, m_currentDimensionB, m_currentDimensionC);
    }
    catch (VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    StoreDimensions();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::UpdateMeasurements(const QString &path, qreal baseA, qreal baseB, qreal baseC) -> bool
{
    return UpdateMeasurements(OpenMeasurementFile(path), baseA, baseB, baseC);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::UpdateMeasurements(const QSharedPointer<VMeasurements> &mFile, qreal baseA, qreal baseB, qreal baseC)
    -> bool
{
    m_m = mFile;

    if (m_m->isNull())
    {
        return false;
    }

    if (VAbstractValApplication::VApp()->GetMeasurementsType() != m_m->Type())
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Measurement files types have not match.")));
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_DATAERR);
        }
        return false;
    }

    try
    {
        ReadMeasurements(baseA, baseB, baseC);
    }
    catch (VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ReadMeasurements(qreal baseA, qreal baseB, qreal baseC)
{
    pattern->ClearVariables({VarType::Measurement, VarType::MeasurementSeparator});

    if (not m_m->Dimensions().isEmpty())
    {
        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

        auto InitDimensionLabel =
            [this](const MeasurementDimension_p &dimension, QPointer<QLabel> &name, QPointer<QComboBox> &control)
        {
            if (dimension.isNull())
            {
                return;
            }

            if (name == nullptr)
            {
                name = new QLabel(dimension->Name() + ':'_L1);
            }
            else
            {
                name->setText(dimension->Name() + ':'_L1);
            }
            name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_m->IsFullCircumference()));

            if (control == nullptr)
            {
                control = new QComboBox;
                control->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            }
        };

        MeasurementDimension_p dimension = dimensions.at(0);
        InitDimensionLabel(dimension, m_dimensionALabel, m_dimensionA);
        InitDimensionGradation(0, dimension, m_dimensionA);

        if (dimensions.size() > 1)
        {
            dimension = dimensions.at(1);
            InitDimensionLabel(dimension, m_dimensionBLabel, m_dimensionB);
            InitDimensionGradation(1, dimension, m_dimensionB);

            if (dimensions.size() > 2)
            {
                dimension = dimensions.at(2);
                InitDimensionLabel(dimension, m_dimensionCLabel, m_dimensionC);
                InitDimensionGradation(2, dimension, m_dimensionC);
            }
        }
    }

    m_m->StoreNames(false);
    m_m->ReadMeasurements(baseA, baseB, baseC);
    if (m_m->Type() == MeasurementsType::Individual)
    {
        VAbstractValApplication::VApp()->SetCustomerName(m_m->Customer());
        VAbstractValApplication::VApp()->SetCustomerBirthDate(m_m->BirthDate());
        VAbstractValApplication::VApp()->SetCustomerEmail(m_m->Email());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetToolButton set tool and show dialog.
 * @param checked true if tool button checked.
 * @param t tool type.
 * @param cursor path tool cursor icon.
 * @param toolTip first tooltipe.
 * @param closeDialogSlot function what handle after close dialog.
 */
template <typename Dialog, typename Func>
void MainWindow::SetToolButton(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                               Func closeDialogSlot)
{
    if (checked)
    {
        CancelTool();
        emit EnableItemMove(false);
        m_currentTool = m_lastUsedTool = t;

        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
        if (settings->GetPointerMode() == VToolPointerMode::ToolIcon)
        {
            const QString resource = QStringLiteral("toolcursor");
            auto cursorResource = VTheme::GetResourceName(resource, cursor);
            if (qApp->devicePixelRatio() >= 2) // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
            {
                // Try to load HiDPI versions of the cursors if availible
                auto hiDPICursor = QString(cursor).replace(".png"_L1, "@2x.png"_L1);
                auto cursorHiDPIResource = VTheme::GetResourceName(resource, hiDPICursor);
                if (QFileInfo::exists(cursorHiDPIResource))
                {
                    cursorResource = cursorHiDPIResource;
                }
            }
            QPixmap pixmap(cursorResource);
            QCursor cur(pixmap, 2, 2);
            ui->view->viewport()->setCursor(cur);
            ui->view->setCurrentCursorShape(); // Hack to fix problem with a cursor
        }

        m_statusLabel->setText(toolTip);
        ui->view->setShowToolOptions(false);
        m_dialogTool = new Dialog(pattern, doc, 0, this);

        // This check helps to find missed tools in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Check if need to extend.");

        switch (t)
        {
            case Tool::Midpoint:
                m_dialogTool->Build(t);
                break;
            case Tool::PiecePath:
            case Tool::Pin:
            case Tool::InsertNode:
            case Tool::PlaceLabel:
                m_dialogTool->SetPiecesList(doc->GetActivePPPieces());
                break;
            case Tool::Rotation:
            case Tool::Move:
            case Tool::FlippingByAxis:
            case Tool::FlippingByLine:
            case Tool::Group:
                m_dialogTool->SetGroupCategories(doc->GetGroupCategories());
                break;
            case Tool::Piece:
                m_dialogTool->SetPatternDoc(doc);
                break;
            default:
                break;
        }

        auto *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        connect(scene, &VMainGraphicsScene::ChoosedObject, m_dialogTool.data(), &DialogTool::ChosenObject);
        connect(scene, &VMainGraphicsScene::SelectedObject, m_dialogTool.data(), &DialogTool::SelectedObject);
        connect(m_dialogTool.data(), &DialogTool::DialogClosed, this, closeDialogSlot);
        connect(m_dialogTool.data(), &DialogTool::ToolTip, this, &MainWindow::ShowToolTip);
        emit ui->view->itemClicked(nullptr);
    }
    else
    {
        if (auto *tButton = qobject_cast<QAction *>(this->sender()))
        {
            tButton->setChecked(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename Dialog, typename Func, typename Func2>
/**
 * @brief SetToolButtonWithApply set tool and show dialog.
 * @param checked true if tool button checked.
 * @param t tool type.
 * @param cursor path tool cursor icon.
 * @param toolTip first tooltipe.
 * @param closeDialogSlot function to handle close of dialog.
 * @param applyDialogSlot function to handle apply in dialog.
 */
void MainWindow::SetToolButtonWithApply(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                                        Func closeDialogSlot, Func2 applyDialogSlot)
{
    if (checked)
    {
        SetToolButton<Dialog>(checked, t, cursor, toolTip, closeDialogSlot);

        connect(m_dialogTool.data(), &DialogTool::DialogApplied, this, applyDialogSlot);
    }
    else
    {
        if (auto *tButton = qobject_cast<QAction *>(this->sender()))
        {
            tButton->setChecked(true);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClosedDialog handle close dialog
 * @param result result working dialog.
 */
template <typename DrawTool> void MainWindow::ClosedDialog(int result)
{
    SCASSERT(not m_dialogTool.isNull())
    if (result == QDialog::Accepted)
    {
        auto *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        auto *tool = dynamic_cast<QGraphicsItem *>(DrawTool::Create(m_dialogTool, scene, doc, pattern));
        // Do not check for nullptr! See issue #719.
        emit ui->view->itemClicked(tool);
    }
    ArrowTool(true);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClosedDialogWithApply handle close dialog that has apply button
 * @param result result working dialog.
 */
template <typename DrawTool> void MainWindow::ClosedDialogWithApply(int result, VMainGraphicsScene *scene)
{
    SCASSERT(not m_dialogTool.isNull())
    if (result == QDialog::Accepted)
    {
        ApplyDialog<DrawTool>(scene);
    }
    // If before Cancel was used Apply we have an item
    auto *vtool = qobject_cast<DrawTool *>(m_dialogTool->GetAssociatedTool()); // Don't check for nullptr here
    if (m_dialogTool->GetAssociatedTool() != nullptr)
    {
        SCASSERT(vtool != nullptr)
        vtool->DialogLinkDestroy();
        connect(vtool, &DrawTool::ToolTip, this, &MainWindow::ShowToolTip);
    }
    ArrowTool(true);
    ui->view->itemClicked(vtool); // Don't check for nullptr here
    if (vtool)
    {
        vtool->setFocus();
    }
    // If insert not to the end of file call lite parse
    if (doc->getCursor() > 0)
    {
        doc->LiteParseTree(Document::LiteParse);
        if (m_dialogHistory)
        {
            m_dialogHistory->UpdateHistory();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ApplyDialog handle apply in dialog
 */
template <typename DrawTool> void MainWindow::ApplyDialog(VMainGraphicsScene *scene)
{
    SCASSERT(not m_dialogTool.isNull())

    // Only create tool if not already created with apply
    if (m_dialogTool->GetAssociatedTool() == nullptr)
    {
        SCASSERT(scene != nullptr)

        m_dialogTool->SetAssociatedTool(DrawTool::Create(m_dialogTool, scene, doc, pattern));
    }
    else
    { // Or update associated tool with data
        auto *vtool = qobject_cast<DrawTool *>(m_dialogTool->GetAssociatedTool());
        SCASSERT(vtool != nullptr)
        vtool->FullUpdateFromGuiApply();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool> void MainWindow::ClosedDrawDialogWithApply(int result)
{
    ClosedDialogWithApply<DrawTool>(result, m_sceneDraw);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool> void MainWindow::ApplyDrawDialog()
{
    ApplyDialog<DrawTool>(m_sceneDraw);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool> void MainWindow::ClosedDetailsDialogWithApply(int result)
{
    ClosedDialogWithApply<DrawTool>(result, m_sceneDetails);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool> void MainWindow::ApplyDetailsDialog()
{
    ApplyDialog<DrawTool>(m_sceneDetails);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolEndLine handler tool endLine.
 * @param checked true - button checked.
 */
void MainWindow::ToolEndLine(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogEndLine>(checked, Tool::EndLine, QStringLiteral("segment_cursor.png"),
                                          tr("Select point"), &MainWindow::ClosedDrawDialogWithApply<VToolEndLine>,
                                          &MainWindow::ApplyDrawDialog<VToolEndLine>);
    LogPatternToolUsed(checked, QStringLiteral("Segment tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolLine handler tool line.
 * @param checked true - button checked.
 */
void MainWindow::ToolLine(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogLine>(checked, Tool::Line, QStringLiteral("line_cursor.png"), tr("Select first point"),
                                       &MainWindow::ClosedDrawDialogWithApply<VToolLine>,
                                       &MainWindow::ApplyDrawDialog<VToolLine>);
    LogPatternToolUsed(checked, QStringLiteral("Line tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolAlongLine handler tool alongLine.
 * @param checked true - button checked.
 */
void MainWindow::ToolAlongLine(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogAlongLine>(checked, Tool::AlongLine, QStringLiteral("along_line_cursor.png"),
                                            tr("Select point"), &MainWindow::ClosedDrawDialogWithApply<VToolAlongLine>,
                                            &MainWindow::ApplyDrawDialog<VToolAlongLine>);
    LogPatternToolUsed(checked, QStringLiteral("Along line tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolMidpoint(bool checked)
{
    ToolSelectPointByRelease();
    // Reuse DialogAlongLine and VToolAlongLine but with different cursor
    SetToolButtonWithApply<DialogAlongLine>(checked, Tool::Midpoint, QStringLiteral("midpoint_cursor.png"),
                                            tr("Select point"), &MainWindow::ClosedDrawDialogWithApply<VToolAlongLine>,
                                            &MainWindow::ApplyDrawDialog<VToolAlongLine>);
    LogPatternToolUsed(checked, QStringLiteral("Midpoint tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolShoulderPoint handler tool shoulderPoint.
 * @param checked true - button checked.
 */
void MainWindow::ToolShoulderPoint(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogShoulderPoint>(
        checked, Tool::ShoulderPoint, QStringLiteral("shoulder_cursor.png"), tr("Select point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolShoulderPoint>, &MainWindow::ApplyDrawDialog<VToolShoulderPoint>);
    LogPatternToolUsed(checked, QStringLiteral("Shoulder point tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolNormal handler tool normal.
 * @param checked true - button checked.
 */
void MainWindow::ToolNormal(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogNormal>(
        checked, Tool::Normal, QStringLiteral("normal_cursor.png"), tr("Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolNormal>, &MainWindow::ApplyDrawDialog<VToolNormal>);
    LogPatternToolUsed(checked, QStringLiteral("Normal tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolBisector handler tool bisector.
 * @param checked true - button checked.
 */
void MainWindow::ToolBisector(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogBisector>(
        checked, Tool::Bisector, QStringLiteral("bisector_cursor.png"), tr("Select first point of angle"),
        &MainWindow::ClosedDrawDialogWithApply<VToolBisector>, &MainWindow::ApplyDrawDialog<VToolBisector>);
    LogPatternToolUsed(checked, QStringLiteral("Bisector tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolLineIntersect handler tool lineIntersect.
 * @param checked true - button checked.
 */
void MainWindow::ToolLineIntersect(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogLineIntersect>(
        checked, Tool::LineIntersect, QStringLiteral("intersect_cursor.png"), tr("Select first point of first line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLineIntersect>, &MainWindow::ApplyDrawDialog<VToolLineIntersect>);
    LogPatternToolUsed(checked, QStringLiteral("Line intersect tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolSpline handler tool spline.
 * @param checked true - button checked.
 */
void MainWindow::ToolSpline(bool checked)
{
    ToolSelectPointByPress();
    SetToolButtonWithApply<DialogSpline>(
        checked, Tool::Spline, QStringLiteral("spline_cursor.png"), tr("Select first point curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolSpline>, &MainWindow::ApplyDrawDialog<VToolSpline>);
    LogPatternToolUsed(checked, QStringLiteral("Spline tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolCubicBezier(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogCubicBezier>(
        checked, Tool::CubicBezier, QStringLiteral("cubic_bezier_cursor.png"), tr("Select first curve point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCubicBezier>, &MainWindow::ApplyDrawDialog<VToolCubicBezier>);
    LogPatternToolUsed(checked, QStringLiteral("Cubic bezier tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolCutSpline handler tool CutSpline.
 * @param checked true - button is checked
 */
void MainWindow::ToolCutSpline(bool checked)
{
    ToolSelectSpline();
    SetToolButtonWithApply<DialogCutSpline>(
        checked, Tool::CutSpline, QStringLiteral("spline_cut_point_cursor.png"), tr("Select simple curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSpline>, &MainWindow::ApplyDrawDialog<VToolCutSpline>);
    LogPatternToolUsed(checked, QStringLiteral("Cut spline tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolArc handler tool arc.
 * @param checked true - button checked.
 */
void MainWindow::ToolArc(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogArc>(
        checked, Tool::Arc, QStringLiteral("arc_cursor.png"), tr("Select point of center of arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolArc>, &MainWindow::ApplyDrawDialog<VToolArc>);
    LogPatternToolUsed(checked, QStringLiteral("Arc tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolEllipticalArc handler tool arc.
 * @param checked true - button checked.
 */
void MainWindow::ToolEllipticalArc(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogEllipticalArc>(checked, Tool::EllipticalArc, QStringLiteral("el_arc_cursor.png"),
                                                tr("Select point of center of elliptical arc"),
                                                &MainWindow::ClosedDrawDialogWithApply<VToolEllipticalArc>,
                                                &MainWindow::ApplyDrawDialog<VToolEllipticalArc>);
    LogPatternToolUsed(checked, QStringLiteral("Elliptical arc tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolSplinePath handler tool splinePath.
 * @param checked true - button checked.
 */
void MainWindow::ToolSplinePath(bool checked)
{
    ToolSelectPointByPress();
    SetToolButtonWithApply<DialogSplinePath>(
        checked, Tool::SplinePath, QStringLiteral("splinePath_cursor.png"), tr("Select point of curve path"),
        &MainWindow::ClosedDrawDialogWithApply<VToolSplinePath>, &MainWindow::ApplyDrawDialog<VToolSplinePath>);
    LogPatternToolUsed(checked, QStringLiteral("Spline path tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolCubicBezierPath(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogCubicBezierPath>(
        checked, Tool::CubicBezierPath, QStringLiteral("cubic_bezier_path_cursor.png"),
        tr("Select point of cubic bezier path"), &MainWindow::ClosedDrawDialogWithApply<VToolCubicBezierPath>,
        &MainWindow::ApplyDrawDialog<VToolCubicBezierPath>);
    LogPatternToolUsed(checked, QStringLiteral("Cubic bezier path tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolCutSplinePath handler tool CutSplinePath.
 * @param checked true - button is checked
 */
void MainWindow::ToolCutSplinePath(bool checked)
{
    ToolSelectSplinePath();
    SetToolButtonWithApply<DialogCutSplinePath>(
        checked, Tool::CutSplinePath, QStringLiteral("splinePath_cut_point_cursor.png"), tr("Select curve path"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSplinePath>, &MainWindow::ApplyDrawDialog<VToolCutSplinePath>);
    LogPatternToolUsed(checked, QStringLiteral("Cut spline path tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolPointOfContact handler tool pointOfContact.
 * @param checked true - button checked.
 */
void MainWindow::ToolPointOfContact(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogPointOfContact>(
        checked, Tool::PointOfContact, QStringLiteral("point_of_contact_cursor.png"), tr("Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointOfContact>, &MainWindow::ApplyDrawDialog<VToolPointOfContact>);
    LogPatternToolUsed(checked, QStringLiteral("Point of contact tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolDetail handler tool detail.
 * @param checked true - button checked.
 */
void MainWindow::ToolDetail(bool checked)
{
    ToolSelectAllDrawObjects();
    SetToolButtonWithApply<DialogSeamAllowance>(checked, Tool::Piece, QStringLiteral("new_detail_cursor.png"),
                                                tr("Select main path objects clockwise."),
                                                &MainWindow::ClosedDetailsDialogWithApply<VToolSeamAllowance>,
                                                &MainWindow::ApplyDetailsDialog<VToolSeamAllowance>);
    LogPatternToolUsed(checked, QStringLiteral("Piece tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPiecePath(bool checked)
{
    ToolSelectAllDrawObjects();
    SetToolButton<DialogPiecePath>(
        checked, Tool::PiecePath, QStringLiteral("path_cursor.png"),
        tr("Select path objects, <b>%1</b> - reverse direction curve").arg(VModifierKey::Shift()),
        &MainWindow::ClosedDialogPiecePath);
    LogPatternToolUsed(checked, QStringLiteral("Piece path tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPin(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButton<DialogPin>(checked, Tool::Pin, QStringLiteral("pin_cursor.png"), tr("Select pin point"),
                             &MainWindow::ClosedDialogPin);
    LogPatternToolUsed(checked, QStringLiteral("Pin tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPlaceLabel(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButton<DialogPlaceLabel>(checked, Tool::PlaceLabel, QStringLiteral("place_label_cursor.png"),
                                    tr("Select placelabel center point"), &MainWindow::ClosedDialogPlaceLabel);
    LogPatternToolUsed(checked, QStringLiteral("Place label tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolHeight handler tool height.
 * @param checked true - button checked.
 */
void MainWindow::ToolHeight(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogHeight>(checked, Tool::Height, QStringLiteral("height_cursor.png"),
                                         tr("Select base point"), &MainWindow::ClosedDrawDialogWithApply<VToolHeight>,
                                         &MainWindow::ApplyDrawDialog<VToolHeight>);
    LogPatternToolUsed(checked, QStringLiteral("Height tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolTriangle handler tool triangle.
 * @param checked true - button checked.
 */
void MainWindow::ToolTriangle(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogTriangle>(
        checked, Tool::Triangle, QStringLiteral("triangle_cursor.png"), tr("Select first point of axis"),
        &MainWindow::ClosedDrawDialogWithApply<VToolTriangle>, &MainWindow::ApplyDrawDialog<VToolTriangle>);
    LogPatternToolUsed(checked, QStringLiteral("Triangle tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolPointOfIntersection handler tool pointOfIntersection.
 * @param checked true - button checked.
 */
void MainWindow::ToolPointOfIntersection(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogPointOfIntersection>(
        checked, Tool::PointOfIntersection, QStringLiteral("point_of_intersection_cursor.png"),
        tr("Select point for X value (vertical)"), &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersection>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersection>);
    LogPatternToolUsed(checked, QStringLiteral("Point of intersection tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolUnionDetails handler tool unionDetails.
 * @param checked true - button checked.
 */
void MainWindow::ToolUnionDetails(bool checked)
{
    ToolSelectDetail();
    SetToolButton<DialogUnionDetails>(checked, Tool::UnionDetails, QStringLiteral("union_cursor.png"),
                                      tr("Select detail"), &MainWindow::ClosedDialogUnionDetails);
    LogPatternToolUsed(checked, QStringLiteral("Union details tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClosedDialogUnionDetails actions after closing DialogUnionDetails.
 * @param result result of dialog working.
 */
void MainWindow::ClosedDialogUnionDetails(int result)
{
    ClosedDialog<VToolUnionDetails>(result); // Avoid error: Template function as signal or slot
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolDuplicateDetail(bool checked)
{
    ToolSelectDetail();
    SetToolButton<DialogDuplicateDetail>(checked, Tool::DuplicateDetail, QStringLiteral("duplicate_detail_cursor.png"),
                                         tr("Select detail"), &MainWindow::ClosedDialogDuplicateDetail);
    LogPatternToolUsed(checked, QStringLiteral("Duplicate detail tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogDuplicateDetail(int result)
{
    SCASSERT(not m_dialogTool.isNull())
    if (result == QDialog::Accepted)
    {
        auto *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        auto *tool = dynamic_cast<QGraphicsItem *>(VToolSeamAllowance::Duplicate(m_dialogTool, scene, doc));
        // Do not check for nullptr! See issue #719.
        emit ui->view->itemClicked(tool);
    }
    ArrowTool(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolGroup(bool checked)
{
    ToolSelectGroupObjects();
    const QString tooltip = tr("Select one or more objects, hold <b>%1</b> - for multiple selection, "
                               "<b>%2</b> - finish creation")
                                .arg(VModifierKey::Control(), VModifierKey::EnterKey());
    SetToolButton<DialogGroup>(checked, Tool::Group, QStringLiteral("group_plus_cursor.png"), tooltip,
                               &MainWindow::ClosedDialogGroup);
    LogPatternToolUsed(checked, QStringLiteral("Group tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolRotation(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("Select one or more objects, hold <b>%1</b> - for multiple selection, "
                               "<b>%2</b> - confirm selection")
                                .arg(VModifierKey::Control(), VModifierKey::EnterKey());
    SetToolButtonWithApply<DialogRotation>(checked, Tool::Rotation, QStringLiteral("rotation_cursor.png"), tooltip,
                                           &MainWindow::ClosedDrawDialogWithApply<VToolRotation>,
                                           &MainWindow::ApplyDrawDialog<VToolRotation>);
    LogPatternToolUsed(checked, QStringLiteral("Rotation tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolFlippingByLine(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("Select one or more objects, hold <b>%1</b> - for multiple selection, "
                               "<b>%2</b> - confirm selection")
                                .arg(VModifierKey::Control(), VModifierKey::EnterKey());
    SetToolButtonWithApply<DialogFlippingByLine>(
        checked, Tool::FlippingByLine, QStringLiteral("flipping_line_cursor.png"), tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolFlippingByLine>, &MainWindow::ApplyDrawDialog<VToolFlippingByLine>);
    LogPatternToolUsed(checked, QStringLiteral("Flipping by line tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolFlippingByAxis(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("Select one or more objects, hold <b>%1</b> - for multiple selection, "
                               "<b>%2</b> - confirm selection")
                                .arg(VModifierKey::Control(), VModifierKey::EnterKey());
    SetToolButtonWithApply<DialogFlippingByAxis>(
        checked, Tool::FlippingByAxis, QStringLiteral("flipping_axis_cursor.png"), tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolFlippingByAxis>, &MainWindow::ApplyDrawDialog<VToolFlippingByAxis>);
    LogPatternToolUsed(checked, QStringLiteral("Flipping by axis tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolMove(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("Select one or more objects, hold <b>%1</b> - for multiple selection, "
                               "<b>%2</b> - confirm selection")
                                .arg(VModifierKey::Control(), VModifierKey::EnterKey());
    SetToolButtonWithApply<DialogMove>(checked, Tool::Move, QStringLiteral("move_cursor.png"), tooltip,
                                       &MainWindow::ClosedDrawDialogWithApply<VToolMove>,
                                       &MainWindow::ApplyDrawDialog<VToolMove>);
    LogPatternToolUsed(checked, QStringLiteral("Move tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogGroup(int result)
{
    SCASSERT(m_dialogTool != nullptr)
    if (result == QDialog::Accepted)
    {
        const QPointer<DialogGroup> dialog = qobject_cast<DialogGroup *>(m_dialogTool);
        SCASSERT(not dialog.isNull())
        const QDomElement group =
            doc->CreateGroup(pattern->getNextId(), dialog->GetName(), dialog->GetTags(), dialog->GetGroup());
        if (not group.isNull())
        {
            auto *addGroup = new AddGroup(group, doc);
            connect(addGroup, &AddGroup::UpdateGroups, m_groupsWidget, &VWidgetGroups::UpdateGroups);
            VAbstractApplication::VApp()->getUndoStack()->push(addGroup);
        }
    }
    ArrowTool(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogPiecePath(int result)
{
    SCASSERT(m_dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        VToolPiecePath::Create(m_dialogTool, m_sceneDetails, doc, pattern);
    }
    ArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogPin(int result)
{
    SCASSERT(m_dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        VToolPin::Create(m_dialogTool, doc, pattern);
    }
    ArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogPlaceLabel(int result)
{
    SCASSERT(m_dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        VToolPlaceLabel::Create(m_dialogTool, doc, pattern);
    }
    ArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogInsertNode(int result)
{
    SCASSERT(m_dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        const QPointer<DialogInsertNode> dTool = qobject_cast<DialogInsertNode *>(m_dialogTool);
        SCASSERT(not dTool.isNull())
        VToolSeamAllowance::InsertNodes(dTool->GetNodes(), dTool->GetPieceId(), m_sceneDetails, pattern, doc);
    }
    ArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ZoomFitBestCurrent()
{
    if (m_drawMode)
    {
        const QRectF rect = doc->ActiveDrawBoundingRect();
        if (rect.isEmpty())
        {
            return;
        }

        ui->view->fitInView(rect, Qt::KeepAspectRatio);
        QTransform transform = ui->view->transform();

        qreal factor = transform.m11();
        factor = qMax(factor, VMainGraphicsView::MinScale());
        factor = qMin(factor, VMainGraphicsView::MaxScale());

        transform.setMatrix(factor, transform.m12(), transform.m13(), transform.m21(), factor, transform.m23(),
                            transform.m31(), transform.m32(), transform.m33());
        ui->view->setTransform(transform);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::PlaceBackgroundImage(const QPointF &pos, const QString &fileName)
{
    DialogAddBackgroundImage dialog(fileName, this);
    if (dialog.exec() == QDialog::Rejected)
    {
        qCritical() << tr("Unable to add background image");
        return;
    }

    VBackgroundPatternImage image = VBackgroundPatternImage::FromFile(fileName, dialog.BuiltIn());
    image.SetName(dialog.Name());

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    image.SetOpacity(settings->GetBackgroundImageDefOpacity() / 100.);

    QTransform m;
    m.translate(pos.x(), pos.y());

    QTransform imageMatrix = image.Matrix();
    imageMatrix *= m;

    image.SetMatrix(m);

    if (not image.IsValid())
    {
        qCritical() << tr("Invalid image. Error: %1").arg(image.ErrorString());
        return;
    }

    auto *addBackgroundImage = new AddBackgroundImage(image, doc);
    connect(addBackgroundImage, &AddBackgroundImage::AddItem, this, &MainWindow::AddBackgroundImageItem);
    connect(addBackgroundImage, &AddBackgroundImage::DeleteItem, this, &MainWindow::DeleteBackgroundImageItem);
    VApplication::VApp()->getUndoStack()->push(addBackgroundImage);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::RemoveBackgroundImage(const QUuid &id)
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(id);

    auto *deleteBackgroundImage = new DeleteBackgroundImage(image, doc);
    connect(deleteBackgroundImage, &DeleteBackgroundImage::AddItem, this, &MainWindow::AddBackgroundImageItem);
    connect(deleteBackgroundImage, &DeleteBackgroundImage::DeleteItem, this, &MainWindow::DeleteBackgroundImageItem);
    VApplication::VApp()->getUndoStack()->push(deleteBackgroundImage);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolCutArc handler tool cutArc.
 * @param checked true - button checked.
 */
void MainWindow::ToolCutArc(bool checked)
{
    ToolSelectArc();
    SetToolButtonWithApply<DialogCutArc>(checked, Tool::CutArc, QStringLiteral("arc_cut_cursor.png"), tr("Select arc"),
                                         &MainWindow::ClosedDrawDialogWithApply<VToolCutArc>,
                                         &MainWindow::ApplyDrawDialog<VToolCutArc>);
    LogPatternToolUsed(checked, QStringLiteral("Cut arc tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolLineIntersectAxis(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogLineIntersectAxis>(
        checked, Tool::LineIntersectAxis, QStringLiteral("line_intersect_axis_cursor.png"),
        tr("Select first point of line"), &MainWindow::ClosedDrawDialogWithApply<VToolLineIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolLineIntersectAxis>);
    LogPatternToolUsed(checked, QStringLiteral("Line intersect axis tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolCurveIntersectAxis(bool checked)
{
    ToolSelectAllDrawObjects();
    SetToolButtonWithApply<DialogCurveIntersectAxis>(
        checked, Tool::CurveIntersectAxis, QStringLiteral("curve_intersect_axis_cursor.png"), tr("Select curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCurveIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolCurveIntersectAxis>);
    LogPatternToolUsed(checked, QStringLiteral("Curve intersect axis tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolArcIntersectAxis(bool checked)
{
    ToolSelectAllDrawObjects();
    // Reuse ToolCurveIntersectAxis but with different cursor and tool tip
    SetToolButtonWithApply<DialogCurveIntersectAxis>(checked, Tool::ArcIntersectAxis,
                                                     QStringLiteral("arc_intersect_axis_cursor.png"), tr("Select arc"),
                                                     &MainWindow::ClosedDrawDialogWithApply<VToolCurveIntersectAxis>,
                                                     &MainWindow::ApplyDrawDialog<VToolCurveIntersectAxis>);
    LogPatternToolUsed(checked, QStringLiteral("Arc intersect axis tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPointOfIntersectionArcs(bool checked)
{
    ToolSelectArc();
    SetToolButtonWithApply<DialogPointOfIntersectionArcs>(
        checked, Tool::PointOfIntersectionArcs, QStringLiteral("point_of_intersection_arcs_cursor.png"),
        tr("Select first an arc"), &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersectionArcs>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersectionArcs>);
    LogPatternToolUsed(checked, QStringLiteral("Point of intersection arcs tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPointOfIntersectionCircles(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogPointOfIntersectionCircles>(
        checked, Tool::PointOfIntersectionCircles, QStringLiteral("point_of_intersection_circles_cursor.png"),
        tr("Select first circle center"), &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersectionCircles>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersectionCircles>);
    LogPatternToolUsed(checked, QStringLiteral("Point of intersection circles tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPointOfIntersectionCurves(bool checked)
{
    ToolSelectCurve();
    SetToolButtonWithApply<DialogPointOfIntersectionCurves>(
        checked, Tool::PointOfIntersectionCurves, QStringLiteral("intersection_curves_cursor.png"),
        tr("Select first curve"), &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersectionCurves>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersectionCurves>);
    LogPatternToolUsed(checked, QStringLiteral("Point of intersection curves tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPointFromCircleAndTangent(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogPointFromCircleAndTangent>(
        checked, Tool::PointFromCircleAndTangent, QStringLiteral("point_from_circle_and_tangent_cursor.png"),
        tr("Select point on tangent"), &MainWindow::ClosedDrawDialogWithApply<VToolPointFromCircleAndTangent>,
        &MainWindow::ApplyDrawDialog<VToolPointFromCircleAndTangent>);
    LogPatternToolUsed(checked, QStringLiteral("Point from circle and tangent tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolPointFromArcAndTangent(bool checked)
{
    ToolSelectPointArc();
    SetToolButtonWithApply<DialogPointFromArcAndTangent>(
        checked, Tool::PointFromArcAndTangent, QStringLiteral("point_from_arc_and_tangent_cursor.png"),
        tr("Select point on tangent"), &MainWindow::ClosedDrawDialogWithApply<VToolPointFromArcAndTangent>,
        &MainWindow::ApplyDrawDialog<VToolPointFromArcAndTangent>);
    LogPatternToolUsed(checked, QStringLiteral("Point from arc and tangent tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolArcWithLength(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogArcWithLength>(
        checked, Tool::ArcWithLength, QStringLiteral("arc_with_length_cursor.png"),
        tr("Select point of the center of the arc"), &MainWindow::ClosedDrawDialogWithApply<VToolArcWithLength>,
        &MainWindow::ApplyDrawDialog<VToolArcWithLength>);
    LogPatternToolUsed(checked, QStringLiteral("Arc with length tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolTrueDarts(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogTrueDarts>(
        checked, Tool::TrueDarts, QStringLiteral("true_darts_cursor.png"), tr("Select the first base line point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolTrueDarts>, &MainWindow::ApplyDrawDialog<VToolTrueDarts>);
    LogPatternToolUsed(checked, QStringLiteral("True darts tool"));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolInsertNode(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("Select one or more objects, hold <b>%1</b> - for multiple selection, "
                               "<b>%2</b> - finish creation")
                                .arg(VModifierKey::Control(), VModifierKey::EnterKey());
    SetToolButton<DialogInsertNode>(checked, Tool::InsertNode, QStringLiteral("insert_node_cursor.png"), tooltip,
                                    &MainWindow::ClosedDialogInsertNode);
    LogPatternToolUsed(checked, QStringLiteral("Insert node tool"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.Tip show tools tooltip.
 * @param toolTip tooltip text.
 */
void MainWindow::ShowToolTip(const QString &toolTip)
{
    m_statusLabel->setText(toolTip);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief triggers the update of the visibility groups
 */
void MainWindow::UpdateVisibilityGroups()
{
    m_groupsWidget->UpdateGroups();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief triggers the update of the details list
 */
void MainWindow::UpdateDetailsList()
{
    m_detailsWidget->UpdateList();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showEvent handle after show window.
 * @param event show event.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    m_taskbarButton->setWindow(windowHandle());
#endif

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    MinimumScrollBar();

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        undoAction->setText(tr("&Undo"));
        redoAction->setText(tr("&Redo"));
        statusBar()->showMessage(tr("Changes applied."), 5000);
        m_patternPieceLabel->setText(tr("Pattern Piece:"));

        if (VApplication::VApp()->GetDrawMode() == Draw::Calculation)
        {
            ui->dockWidgetGroups->setWindowTitle(tr("Groups of visibility"));
            ui->dockWidgetGroups->setToolTip(tr("Contains all visibility groups"));
        }
        else
        {
            ui->dockWidgetGroups->setWindowTitle(tr("Details"));
            ui->dockWidgetGroups->setToolTip(tr("Show which details will go in layout"));
        }

        ToolBarOption();

        UpdateWindowTitle();
        emit m_sceneDetails->LanguageChanged();
    }

    if (event->type() == QEvent::PaletteChange)
    {
        SetupDrawToolsIcons();
    }

    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle after close window.
 * @param event close event.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(5, 11, 1)
    // Workaround for Qt bug https://bugreports.qt.io/browse/QTBUG-43344
    static int numCalled = 0;
    if (numCalled++ >= 1)
    {
        return;
    }
#endif

    qCDebug(vMainWindow, "Closing main window");
    if (MaybeSave())
    {
        FileClosedCorrect();

        event->accept();
        QApplication::closeAllWindows();
    }
    else
    {
        qCDebug(vMainWindow, "Closing canceled.");
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::customEvent(QEvent *event)
{
    if (event->type() == UNDO_EVENT)
    {
        VAbstractApplication::VApp()->getUndoStack()->undo();
    }
    else if (event->type() == FIT_BEST_CURRENT_EVENT)
    {
        ZoomFitBestCurrent();
    }
    else if (event->type() == WARNING_MESSAGE_EVENT)
    {
        PrintPatternMessage(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CleanLayout()
{
    m_layoutSettings->CleanLayout();
    gcontours.clear();
    ui->listWidget->clear();
    SetLayoutModeActions();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::PrepareSceneList(PreviewQuatilty quality)
{
    ui->listWidget->clear();
    for (int i = 1; i <= m_layoutSettings->LayoutScenes().size(); ++i)
    {
        auto *item = new QListWidgetItem(ScenePreview(i - 1, ui->listWidget->iconSize(), quality), QString::number(i));
        ui->listWidget->addItem(item);
    }

    if (not m_layoutSettings->LayoutScenes().isEmpty())
    {
        ui->listWidget->setCurrentRow(0);
        SetLayoutModeActions();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator)
{
    QxtCsvModel csv;

    csv.insertColumn(0);
    csv.insertColumn(1);
    csv.insertColumn(2);

    if (withHeader)
    {
        csv.setHeaderText(0, tr("Name"));
        csv.setHeaderText(1, tr("The calculated value"));
        csv.setHeaderText(2, tr("Formula"));
    }

    const QMap<QString, QSharedPointer<VIncrement>> increments = pattern->DataIncrements();

    qint32 currentRow = -1;

    auto SavePreviewCalculation = [&currentRow, &csv, increments](bool save)
    {
        QMap<QString, QSharedPointer<VIncrement>>::const_iterator i;
        QMap<quint32, QString> map;
        // Sorting QHash by id
        for (i = increments.constBegin(); i != increments.constEnd(); ++i)
        {
            const QSharedPointer<VIncrement> &incr = i.value();
            if (incr->IsPreviewCalculation() == save)
            {
                map.insert(incr->GetIndex(), i.key());
            }
        }

        QMapIterator<quint32, QString> iMap(map);
        while (iMap.hasNext())
        {
            iMap.next();
            QSharedPointer<VIncrement> incr = increments.value(iMap.value());
            currentRow++;

            csv.insertRow(currentRow);
            csv.setText(currentRow, 0, incr->GetName()); // name
            csv.setText(currentRow, 1,
                        VAbstractApplication::VApp()->LocaleToString(*incr->GetValue())); // calculated value

            QString formula = VTranslateVars::TryFormulaToUser(
                incr->GetFormula(), VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            csv.setText(currentRow, 2, formula); // formula
        }
    };

    SavePreviewCalculation(false);
    SavePreviewCalculation(true);

    QString error;
    csv.toCSV(fileName, error, withHeader, separator, VTextCodec::codecForMib(mib));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolBarStyle(QToolBar *bar) const
{
    MainWindowsNoGUI::ToolBarStyle(bar);

#if defined(Q_OS_MAC)
    // Temporary fix issue with toolbar black background on mac with OpenGL render
    if (VAbstractValApplication::VApp()->getSceneView() &&
        VAbstractValApplication::VApp()->getSceneView()->IsOpenGLRender())
    {
        bar->setStyle(QStyleFactory::create("fusion"));
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ScaleChanged(qreal scale)
{
    if (not m_doubleSpinBoxScale.isNull())
    {
        m_doubleSpinBoxScale->blockSignals(true);
        m_doubleSpinBoxScale->setMaximum(qFloor(VMainGraphicsView::MaxScale() * 1000) / 10.0);
        m_doubleSpinBoxScale->setMinimum(qFloor(VMainGraphicsView::MinScale() * 1000) / 10.0);
        m_doubleSpinBoxScale->setValue(qFloor(scale * 1000) / 10.0);
        m_doubleSpinBoxScale->setSingleStep(1);
        m_doubleSpinBoxScale->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadIndividual()
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    const QString filter = tr("Individual measurements") + QStringLiteral(" (*.vit);;") + tr("Multisize measurements") +
                           QStringLiteral(" (*.vst)");
    // Use standard path to individual measurements
    const QString path = settings->GetPathIndividualMeasurements();

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        if (LoadMeasurements(mPath))
        {
            if (not doc->MPath().isEmpty())
            {
                m_watcher->removePath(AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath()));
            }
            ui->actionUnloadMeasurements->setEnabled(true);
            doc->SetMPath(RelativeMPath(VAbstractValApplication::VApp()->GetPatternPath(), mPath));
            m_watcher->addPath(mPath);
            PatternChangesWereSaved(false);
            ui->actionEditCurrent->setEnabled(true);
            statusBar()->showMessage(tr("Measurements loaded"), 5000);
            doc->LiteParseTree(Document::FullLiteParse);

            UpdateWindowTitle();
        }

        settings->SetPathIndividualMeasurements(QFileInfo(mPath).absolutePath());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadMultisize()
{
    const QString filter = tr("Multisize measurements") + QStringLiteral(" (*.vst);;") + tr("Individual measurements") +
                           QStringLiteral("(*.vit)");
    // Use standard path to multisize measurements
    QString path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathMultisizeMeasurements();
    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (mPath.isEmpty())
    {
        return;
    }

    if (!LoadMeasurements(mPath))
    {
        return;
    }

    if (not doc->MPath().isEmpty())
    {
        m_watcher->removePath(AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath()));
    }
    ui->actionUnloadMeasurements->setEnabled(true);
    doc->SetMPath(RelativeMPath(VAbstractValApplication::VApp()->GetPatternPath(), mPath));
    m_watcher->addPath(mPath);
    PatternChangesWereSaved(false);
    ui->actionEditCurrent->setEnabled(true);
    statusBar()->showMessage(tr("Measurements loaded"), 5000);
    doc->LiteParseTree(Document::FullLiteParse);

    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UnloadMeasurements()
{
    if (doc->MPath().isEmpty())
    {
        ui->actionUnloadMeasurements->setDisabled(true);
        return;
    }

    if (doc->ListMeasurements().isEmpty())
    {
        m_watcher->removePath(AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath()));

        const MeasurementsType oldType = VAbstractValApplication::VApp()->GetMeasurementsType();
        VAbstractValApplication::VApp()->SetMeasurementsType(MeasurementsType::Unknown);

        m_m.clear();

        VAbstractValApplication::VApp()->SetDimensionHeight(0);
        VAbstractValApplication::VApp()->SetDimensionSize(0);
        VAbstractValApplication::VApp()->SetDimensionHip(0);
        VAbstractValApplication::VApp()->SetDimensionWaist(0);

        VAbstractValApplication::VApp()->SetDimensionHeightLabel(QString());
        VAbstractValApplication::VApp()->SetDimensionSizeLabel(QString());
        VAbstractValApplication::VApp()->SetDimensionHipLabel(QString());
        VAbstractValApplication::VApp()->SetDimensionWaistLabel(QString());

        if (oldType == MeasurementsType::Multisize)
        {
            m_currentDimensionA = 0;
            m_currentDimensionB = 0;
            m_currentDimensionC = 0;

            ToolBarOption();
        }
        else if (oldType == MeasurementsType::Individual)
        {
            VAbstractValApplication::VApp()->SetCustomerBirthDate(QDate());
            VAbstractValApplication::VApp()->SetCustomerEmail(QString());
            VAbstractValApplication::VApp()->SetCustomerName(QString());
            VAbstractValApplication::VApp()->SetMeasurementsUnits(Unit::LAST_UNIT_DO_NOT_USE);
            VAbstractValApplication::VApp()->SetDimensionSizeUnits(Unit::LAST_UNIT_DO_NOT_USE);
        }

        doc->SetMPath(QString());
        emit doc->UpdatePatternLabel();
        PatternChangesWereSaved(false);
        ui->actionEditCurrent->setEnabled(false);
        ui->actionUnloadMeasurements->setDisabled(true);
        statusBar()->showMessage(tr("Measurements unloaded"), 5000);

        UpdateWindowTitle();
    }
    else
    {
        qCWarning(vMainWindow, "%s",
                  qUtf8Printable(tr("Couldn't unload measurements. Some of them are used in the pattern.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowMeasurements()
{
    if (not doc->MPath().isEmpty())
    {
        const QString absoluteMPath = AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath());

        QStringList arguments;
        arguments.append(absoluteMPath);
        arguments.append(QStringLiteral("-u"));
        arguments.append(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));

        if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
        {
            if (m_currentDimensionA > 0)
            {
                arguments.append(QStringLiteral("-a"));
                arguments.append(QString::number(m_currentDimensionA));
            }

            if (m_currentDimensionB > 0)
            {
                arguments.append(QStringLiteral("-b"));
                arguments.append(QString::number(m_currentDimensionB));
            }

            if (m_currentDimensionC > 0)
            {
                arguments.append(QStringLiteral("-c"));
                arguments.append(QString::number(m_currentDimensionC));
            }
        }

        if (isNoScaling)
        {
            arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        VApplication::StartDetachedProcess(VApplication::TapeFilePath(), arguments);
    }
    else
    {
        ui->actionEditCurrent->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::MeasurementsChanged(const QString &path)
{
    m_mChanges = false;
    QFileInfo checkFile(path);
    if (checkFile.exists())
    {
        m_mChanges = true;
        m_mChangesAsked = false;
        m_measurementsSyncTimer->start(V_MSECONDS(1500));
    }
    else
    {
        for (int i = 0; i <= 1000; i = i + 10)
        {
            if (checkFile.exists())
            {
                m_mChanges = true;
                m_mChangesAsked = false;
                m_measurementsSyncTimer->start(V_MSECONDS(1500));
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    UpdateWindowTitle();
    ui->actionSyncMeasurements->setEnabled(m_mChanges);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SyncMeasurements()
{
    if (m_mChanges)
    {
        const QString path = AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath());

        // Temporarily remove the path to prevent infinite synchronization after a format conversion.
        m_watcher->removePath(path);

        if (UpdateMeasurements(path, m_currentDimensionA, m_currentDimensionB, m_currentDimensionC))
        {
            const QString msg = tr("Measurements have been synced");
            qCDebug(vMainWindow, "%s", qUtf8Printable(msg));
            statusBar()->showMessage(msg, 5000);
            VWidgetPopup::PopupMessage(this, msg);
            doc->LiteParseTree(Document::FullLiteParse);
            StoreDimensions();
            m_mChanges = false;
            m_mChangesAsked = true;
            m_measurementsSyncTimer->stop();
            UpdateWindowTitle();
            ui->actionSyncMeasurements->setEnabled(m_mChanges);
        }
        else
        {
            qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't sync measurements.")));
        }

        if (not m_watcher->files().contains(path))
        {
            m_watcher->addPath(path);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CreateWatermark()
{
    CleanWaterkmarkEditors();
    OpenWatermark();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::EditCurrentWatermark()
{
    CleanWaterkmarkEditors();

    QString watermarkFile = doc->GetWatermarkPath();
    if (not watermarkFile.isEmpty())
    {
        OpenWatermark(watermarkFile);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadWatermark()
{
    const QString filter(tr("Watermark files") + QStringLiteral(" (*.vwm)"));
    QString dir = QDir::homePath();
    qDebug("Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter, nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());
    if (filePath.isEmpty())
    {
        return;
    }

    if (doc->SetWatermarkPath(filePath))
    {
        ui->actionRemoveWatermark->setEnabled(true);
        ui->actionEditCurrentWatermark->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::RemoveWatermark()
{
    if (doc->SetWatermarkPath(QString()))
    {
        ui->actionRemoveWatermark->setEnabled(false);
        ui->actionEditCurrentWatermark->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CleanWaterkmarkEditors()
{
    QMutableListIterator<QPointer<WatermarkWindow>> i(m_watermarkEditors);
    while (i.hasNext())
    {
        QPointer<WatermarkWindow> watermarkEditor = i.next();
        if (watermarkEditor.isNull())
        {
            i.remove();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::StoreMultisizeMDimensions()
{
    VAbstractValApplication::VApp()->SetMeasurementsUnits(m_m->Units());

    QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

    if (not dimensions.isEmpty())
    {
        if (not m_dimensionALabel.isNull())
        {
            m_dimensionALabel->setText(dimensions.at(0)->Name() + ':'_L1);
        }
    }

    if (dimensions.size() > 1)
    {
        if (not m_dimensionBLabel.isNull())
        {
            m_dimensionBLabel->setText(dimensions.at(1)->Name() + ':'_L1);
        }
    }

    if (dimensions.size() > 2)
    {
        if (not m_dimensionCLabel.isNull())
        {
            m_dimensionCLabel->setText(dimensions.at(2)->Name() + ':'_L1);
        }
    }

    StoreMultisizeMDimension(dimensions, 0, m_currentDimensionA);
    StoreMultisizeMDimension(dimensions, 1, m_currentDimensionB);
    StoreMultisizeMDimension(dimensions, 2, m_currentDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::StoreIndividualMDimensions()
{
    QMap<QString, QSharedPointer<VMeasurement>> measurements = pattern->DataMeasurements();

    StoreIndividualMDimension(measurements, IMD::X);
    StoreIndividualMDimension(measurements, IMD::Y);
    StoreIndividualMDimension(measurements, IMD::W);
    StoreIndividualMDimension(measurements, IMD::Z);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::StoreMultisizeMDimension(const QList<MeasurementDimension_p> &dimensions, int index, qreal currentBase)
{
    if (dimensions.size() > index)
    {
        const MeasurementDimension_p &dimension = dimensions.at(index);
        const DimesionLabels labels = dimension->Labels();

        switch (dimension->Type())
        {
            case MeasurementDimension::X:
                VAbstractValApplication::VApp()->SetDimensionHeight(currentBase);
                VAbstractValApplication::VApp()->SetDimensionHeightLabel(
                    VFuzzyValue(labels, currentBase, QString::number(currentBase)));
                break;
            case MeasurementDimension::Y:
            {
                const bool fc = m_m->IsFullCircumference();
                VAbstractValApplication::VApp()->SetDimensionSize(fc ? currentBase * 2 : currentBase);
                VAbstractValApplication::VApp()->SetDimensionSizeLabel(
                    VFuzzyValue(labels, currentBase, QString::number(fc ? currentBase * 2 : currentBase)));
                const bool measurement = dimension->IsBodyMeasurement();
                VAbstractValApplication::VApp()->SetDimensionSizeUnits(measurement ? m_m->Units()
                                                                                   : Unit::LAST_UNIT_DO_NOT_USE);
                break;
            }
            case MeasurementDimension::W:
            {
                const bool fc = m_m->IsFullCircumference();
                VAbstractValApplication::VApp()->SetDimensionWaist(fc ? currentBase * 2 : currentBase);
                VAbstractValApplication::VApp()->SetDimensionWaistLabel(
                    VFuzzyValue(labels, currentBase, QString::number(fc ? currentBase * 2 : currentBase)));
                break;
            }
            case MeasurementDimension::Z:
            {
                const bool fc = m_m->IsFullCircumference();
                VAbstractValApplication::VApp()->SetDimensionHip(fc ? currentBase * 2 : currentBase);
                VAbstractValApplication::VApp()->SetDimensionHipLabel(
                    VFuzzyValue(labels, currentBase, QString::number(fc ? currentBase * 2 : currentBase)));
                break;
            }
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::StoreIndividualMDimension(const QMap<QString, QSharedPointer<VMeasurement>> &measurements, IMD type)
{
    const QString name = VAbstractApplication::VApp()->TrVars()->VarToUser(m_m->MeasurementForDimension(type));
    const bool valid = not name.isEmpty() && measurements.contains(name);
    const qreal value = valid ? *measurements.value(name)->GetValue() : 0;
    switch (type)
    {
        case IMD::X:
            VAbstractValApplication::VApp()->SetDimensionHeight(value);
            VAbstractValApplication::VApp()->SetDimensionHeightLabel(QString::number(value));
            break;
        case IMD::Y:
            VAbstractValApplication::VApp()->SetDimensionSize(value);
            VAbstractValApplication::VApp()->SetDimensionSizeLabel(QString::number(value));
            break;
        case IMD::W:
            VAbstractValApplication::VApp()->SetDimensionWaist(value);
            VAbstractValApplication::VApp()->SetDimensionWaistLabel(QString::number(value));
            break;
        case IMD::Z:
            VAbstractValApplication::VApp()->SetDimensionHip(value);
            VAbstractValApplication::VApp()->SetDimensionHipLabel(QString::number(value));
            break;
        case IMD::N:
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::DimensionRestrictedValues(int index, const MeasurementDimension_p &dimension) -> QVector<qreal>
{
    VDimensionRestriction restriction;
    if (index == 0)
    {
        restriction = m_m->Restriction(0);
    }
    else if (index == 1)
    {
        restriction = m_m->Restriction(m_currentDimensionA);
    }
    else
    {
        restriction = m_m->Restriction(m_currentDimensionA, m_currentDimensionB);
    }

    const QVector<qreal> bases = dimension->ValidBases();

    qreal min = VFuzzyIndexOf(bases, restriction.GetMin()) != -1 ? restriction.GetMin() : dimension->MinValue();
    qreal max = VFuzzyIndexOf(bases, restriction.GetMax()) != -1 ? restriction.GetMax() : dimension->MaxValue();

    if (min > max)
    {
        min = dimension->MinValue();
        max = dimension->MaxValue();
    }

    return VAbstartMeasurementDimension::ValidBases(min, max, dimension->Step(), restriction.GetExcludeValues());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetDimensionBases()
{
    const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

    auto SetBase = [dimensions](int index, const QPointer<QComboBox> &control, double &value)
    {
        if (dimensions.size() > index)
        {
            SCASSERT(control != nullptr)

            control->blockSignals(true);

            const qint32 i = control->findData(value);
            if (i != -1)
            {
                control->setCurrentIndex(i);
            }
            else
            {
                value = control->currentData().toDouble();
            }

            control->blockSignals(false);
        }
    };

    SetBase(0, m_dimensionA, m_currentDimensionA);
    SetBase(1, m_dimensionB, m_currentDimensionB);
    SetBase(2, m_dimensionC, m_currentDimensionC);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::StoreDimensions()
{
    if (m_m->Type() == MeasurementsType::Multisize)
    {
        StoreMultisizeMDimensions();

        doc->SetPatternWasChanged(true);
        emit doc->UpdatePatternLabel();
    }
    else if (m_m->Type() == MeasurementsType::Individual)
    {
        StoreIndividualMDimensions();

        doc->SetPatternWasChanged(true);
        emit doc->UpdatePatternLabel();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ExportDraw(const QString &fileName)
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutExporter exporter;

    QT_WARNING_POP

    exporter.SetFileName(fileName);

    int verticalScrollBarValue = ui->view->verticalScrollBar()->value();
    int horizontalScrollBarValue = ui->view->horizontalScrollBar()->value();

    QTransform viewTransform = ui->view->transform();
    ui->view->ZoomFitBest(); // Resize all labels
    ui->view->repaint();
    ui->view->ZoomOriginal(); // Set to original scale

    // Enable all items on scene
    const QList<QGraphicsItem *> qItems = m_sceneDraw->items();
    for (auto *item : qItems)
    {
        item->setEnabled(true);
        if (item->type() == VGraphicsSimpleTextItem::Type)
        {
            auto *text = dynamic_cast<VGraphicsSimpleTextItem *>(item);
            text->RefreshColor(); // Regular update doesn't work on labels
        }
    }

    ui->view->repaint();

    m_sceneDraw->SetOriginsVisible(false);

    const QRectF rect = m_sceneDraw->VisibleItemsBoundingRect();
    m_sceneDraw->update(rect);
    exporter.SetImageRect(rect);
    exporter.SetOffset(rect.topLeft()); // Correct positions to fit SVG view rect

    exporter.ExportToSVG(m_sceneDraw, QList<QGraphicsItem *>());

    m_sceneDraw->SetOriginsVisible(true);

    // Restore scale, scrollbars and current active pattern piece
    ui->view->setTransform(viewTransform);
    VMainGraphicsView::NewSceneRect(ui->view->scene(), ui->view);
    ScaleChanged(ui->view->transform().m11());

    ui->view->verticalScrollBar()->setValue(verticalScrollBarValue);
    ui->view->horizontalScrollBar()->setValue(horizontalScrollBarValue);

    doc->ChangeActivPP(doc->GetNameActivPP(), Document::FullParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::NewBackgroundImageItem(const VBackgroundPatternImage &image)
{
    if (m_backgroudcontrols == nullptr)
    {
        m_backgroudcontrols = new VBackgroundImageControls(doc);
        connect(m_sceneDraw, &VMainGraphicsScene::ItemByMouseRelease, m_backgroudcontrols,
                &VBackgroundImageControls::DeactivateControls);
        m_sceneDraw->addItem(m_backgroudcontrols);
    }

    if (m_backgroundImages.contains(image.Id()))
    {
        VBackgroundImageItem *item = m_backgroundImages.value(image.Id());
        if (item != nullptr)
        {
            item->SetImage(image);
            item->setVisible(true);
        }
    }
    else
    {
        VBackgroundImageItem *item = InitBackgroundImageItem(image);
        if (item != nullptr)
        {
            m_backgroundImages.insert(image.Id(), item);
            m_sceneDraw->addItem(item);
        }
    }

    VMainGraphicsView::NewSceneRect(m_sceneDraw, ui->view);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::InitBackgroundImageItem(const VBackgroundPatternImage &image) -> VBackgroundImageItem *
{
    VBackgroundImageItem *item = nullptr;
    if (image.Type() == PatternImage::Raster)
    {
        item = new VBackgroundPixmapItem(image, doc);
    }
    else if (image.Type() == PatternImage::Vector || image.Type() == PatternImage::Unknown)
    {
        item = new VBackgroundSVGItem(image, doc);
    }

    if (item != nullptr)
    {
        connect(item, &VBackgroundImageItem::UpdateControls, m_backgroudcontrols,
                &VBackgroundImageControls::UpdateControls);
        connect(item, &VBackgroundImageItem::ActivateControls, m_backgroudcontrols,
                &VBackgroundImageControls::ActivateControls);
        connect(item, &VBackgroundImageItem::DeleteImage, this, &MainWindow::RemoveBackgroundImage);
        connect(this, &MainWindow::EnableBackgroundImageSelection, item, &VBackgroundImageItem::EnableSelection);
        connect(item, &VBackgroundImageItem::ShowImageInExplorer, this, &MainWindow::ShowBackgroundImageInExplorer);
        connect(item, &VBackgroundImageItem::SaveImage, this, &MainWindow::SaveBackgroundImage);
        connect(m_backgroudcontrols, &VBackgroundImageControls::ActiveImageChanged, m_backgroundImagesWidget,
                &VWidgetBackgroundImages::ImageSelected);
        connect(m_backgroundImagesWidget, &VWidgetBackgroundImages::SelectImage, m_backgroudcontrols,
                &VBackgroundImageControls::ActivateControls);
    }

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::SavePatternAs(const QString &fileName) -> bool
{
    // Need for restoring previous state in case of failure
    const bool wasModified = doc->IsModified(); // Need because SetReadOnly() will change internal state
    const bool readOnly = doc->IsReadOnly();

    doc->SetReadOnly(false); // Save as... disable read only state
    QString error;
    const bool result = SavePattern(fileName, error);
    if (not result)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();

        // Restoring previous state
        doc->SetReadOnly(readOnly);
        doc->SetModified(wasModified);

        return result;
    }

    const QString oldFilePath = VAbstractValApplication::VApp()->GetPatternPath();
    if (not oldFilePath.isEmpty())
    {
        qCDebug(vMainWindow, "Updating restore file list.");
        QStringList restoreFiles = VAbstractValApplication::VApp()->ValentinaSettings()->GetRestoreFileList();
        restoreFiles.removeAll(oldFilePath);
        VAbstractValApplication::VApp()->ValentinaSettings()->SetRestoreFileList(restoreFiles);
        QFile::remove(oldFilePath + *autosavePrefix);
    }

    m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
    m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;
    m_patternReadOnly = false;

    qCDebug(vMainWindow, "Locking file");
    if (VAbstractValApplication::VApp()->GetPatternPath() == fileName && not m_lock.isNull())
    {
        m_lock->Unlock();
    }
    VlpCreateLock(m_lock, fileName);

    if (m_lock->IsLocked())
    {
        qCDebug(vMainWindow, "Pattern file %s was locked.", qUtf8Printable(fileName));
    }
    else
    {
        qCDebug(vMainWindow, "Failed to lock %s", qUtf8Printable(fileName));
        qCDebug(vMainWindow, "Error type: %d", m_lock->GetLockError());
        qCCritical(vMainWindow, "%s",
                   qUtf8Printable(tr("Failed to lock. This file already opened in another window. Expect "
                                     "collissions when run 2 copies of the program.")));
    }

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::FullParsePattern() -> bool
{
    QFuture<void> futureTestUniqueId;

    auto WaitForFutureFinish = [](QFuture<void> &futureTestUniqueId)
    {
        try
        {
            futureTestUniqueId.waitForFinished();
        }
        catch (...)
        {
            // ignore
        }
    };

    auto HandleError = [this, WaitForFutureFinish](QFuture<void> &futureTestUniqueId)
    {
        SetEnabledGUI(false);
        if (VAbstractValApplication::VApp()->getOpeningPattern())
        {
            WaitForFutureFinish(futureTestUniqueId);
        }

        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_DATAERR);
        }
    };

    try
    {
        if (VAbstractValApplication::VApp()->getOpeningPattern())
        {
            futureTestUniqueId = QtConcurrent::run([this]() { doc->TestUniqueId(); });
        }

        SetEnabledGUI(true);
        doc->Parse(Document::FullParse);
        ParseBackgroundImages();

        if (VAbstractValApplication::VApp()->getOpeningPattern())
        {
            futureTestUniqueId.waitForFinished();
        }
    }
    catch (const VExceptionUndo &)
    {
        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        if (VAbstractValApplication::VApp()->getOpeningPattern())
        {
            try
            {
                futureTestUniqueId.waitForFinished();
            }
            catch (const VExceptionWrongId &e)
            {
                qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")),
                           qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
                SetEnabledGUI(false);
                if (not VApplication::IsGUIMode())
                {
                    QCoreApplication::exit(V_EX_DATAERR);
                }
                return false;
            }
        }
        QApplication::postEvent(this, new UndoEvent());
        return false;
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        HandleError(futureTestUniqueId);
        return false;
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        HandleError(futureTestUniqueId);
        return false;
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        HandleError(futureTestUniqueId);
        return false;
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        HandleError(futureTestUniqueId);
        return false;
    }
    catch (VException &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        HandleError(futureTestUniqueId);
        return false;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));
        HandleError(futureTestUniqueId);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void MainWindow::OpenAt(QAction *where)
{
    const QString path = VAbstractValApplication::VApp()->GetPatternPath().left(
                             VAbstractValApplication::VApp()->GetPatternPath().indexOf(where->text())) +
                         where->text();
    if (path == VAbstractValApplication::VApp()->GetPatternPath())
    {
        return;
    }
    QProcess process;
    process.start(QStringLiteral("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}
#endif // defined(Q_OS_MAC)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolBarOption enable option toolbar.
 */
void MainWindow::ToolBarOption()
{
    ui->toolBarOption->clear();

    if (not m_zoomScale.isNull())
    {
        delete m_zoomScale;
    }
    if (not m_doubleSpinBoxScale.isNull())
    {
        delete m_doubleSpinBoxScale;
    }

    if (not m_mouseCoordinate.isNull())
    {
        delete m_mouseCoordinate;
    }

    if (not m_unreadPatternMessage.isNull())
    {
        delete m_unreadPatternMessage;
    }

    InitDimensionControls();

    m_zoomScale = new QLabel(tr("Scale:"), this);
    ui->toolBarOption->addWidget(m_zoomScale);

    m_doubleSpinBoxScale = new QDoubleSpinBox(this);
    m_doubleSpinBoxScale->setDecimals(1);
    m_doubleSpinBoxScale->setSuffix(QChar('%'));
    ScaleChanged(ui->view->transform().m11());
    connect(m_doubleSpinBoxScale.data(), QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double d) { ui->view->Zoom(d / 100.0); });
    ui->toolBarOption->addWidget(m_doubleSpinBoxScale);

    ui->toolBarOption->addSeparator();

    m_mouseCoordinate =
        new QLabel(QStringLiteral("0, 0 (%1)").arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true)));
    ui->toolBarOption->addWidget(m_mouseCoordinate);

    ui->toolBarOption->addSeparator();
    m_unreadPatternMessage = new QLabel();
    ui->toolBarOption->addWidget(m_unreadPatternMessage);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolBarStages()
{
    const QString resource = QStringLiteral("icon");

    m_leftGoToStage = new QLabel(this);
    m_leftGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-next-skip.png")));
    ui->toolBarStages->insertWidget(ui->actionDetails, m_leftGoToStage);

    m_rightGoToStage = new QLabel(this);
    m_rightGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-next.png")));
    ui->toolBarStages->insertWidget(ui->actionLayout, m_rightGoToStage);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ToolBarDraws enable draw toolbar.
 */
void MainWindow::ToolBarDraws()
{
    m_patternPieceLabel = new QLabel(tr("Pattern Piece:"));
    ui->toolBarDraws->addWidget(m_patternPieceLabel);

    // By using Qt UI Designer we can't add QComboBox to toolbar
    m_comboBoxDraws = new QComboBox;
    ui->toolBarDraws->addWidget(m_comboBoxDraws);
    m_comboBoxDraws->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_comboBoxDraws->setEnabled(false);
    connect(m_comboBoxDraws, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) { ChangePP(index); });

    connect(ui->actionOptionDraw, &QAction::triggered, this,
            [this]()
            {
                QString draw = doc->GetNameActivPP();
                bool ok = PatternPieceName(draw);
                if (not ok)
                {
                    return;
                }
                VAbstractApplication::VApp()->getUndoStack()->push(new RenamePP(doc, draw, m_comboBoxDraws));
            });
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolBarTools()
{
    /*First we will try use Standard Shortcuts from Qt, but because keypad "-" and "+" not the same keys like in main
    keypad, shortcut Ctrl+"-" or "+" from keypad will not working with standard shortcut (QKeySequence::ZoomIn or
    QKeySequence::ZoomOut). For examle "+" is Qt::Key_Plus + Qt::KeypadModifier for keypad.
    Also for me don't work Qt:CTRL and work Qt::ControlModifier.*/

    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

    QList<QKeySequence> zoomInShortcuts;
    zoomInShortcuts.append(QKeySequence(QKeySequence::ZoomIn));
    zoomInShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(QKeyCombination(Qt::ControlModifier), Qt::Key_Plus | Qt::KeypadModifier));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_Plus + Qt::KeypadModifier));
#endif
    ui->actionZoomIn->setShortcuts(zoomInShortcuts);
    connect(ui->actionZoomIn, &QAction::triggered, ui->view, &VMainGraphicsView::ZoomIn);

    QList<QKeySequence> zoomOutShortcuts;
    zoomOutShortcuts.append(QKeySequence(QKeySequence::ZoomOut));
    zoomOutShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(QKeyCombination(Qt::ControlModifier), Qt::Key_Minus | Qt::KeypadModifier));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_Minus + Qt::KeypadModifier));
#endif
    ui->actionZoomOut->setShortcuts(zoomOutShortcuts);
    connect(ui->actionZoomOut, &QAction::triggered, ui->view, &VMainGraphicsView::ZoomOut);

    QList<QKeySequence> zoomOriginalShortcuts;
    zoomOriginalShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ControlModifier | Qt::Key_0));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_0));
#endif
    zoomOriginalShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(QKeyCombination(Qt::ControlModifier), Qt::Key_0 | Qt::KeypadModifier));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_0 + Qt::KeypadModifier));
#endif
    ui->actionZoomOriginal->setShortcuts(zoomOriginalShortcuts);
    connect(ui->actionZoomOriginal, &QAction::triggered, ui->view, &VMainGraphicsView::ZoomOriginal);

    QList<QKeySequence> zoomFitBestShortcuts;
    zoomFitBestShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ControlModifier | Qt::Key_Equal));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_Equal));
#endif
    ui->actionZoomFitBest->setShortcuts(zoomFitBestShortcuts);
    connect(ui->actionZoomFitBest, &QAction::triggered, ui->view, &VMainGraphicsView::ZoomFitBest);

    QList<QKeySequence> zoomFitBestCurrentShortcuts;
    zoomFitBestCurrentShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ControlModifier | Qt::Key_M));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_M));
#endif
    ui->actionZoomFitBestCurrent->setShortcuts(zoomFitBestCurrentShortcuts);
    connect(ui->actionZoomFitBestCurrent, &QAction::triggered, this, &MainWindow::ZoomFitBestCurrent);

    connect(ui->actionPreviousPatternPiece, &QAction::triggered, this, &MainWindow::PreviousPatternPiece);
    connect(ui->actionNextPatternPiece, &QAction::triggered, this, &MainWindow::NextPatternPiece);

    ui->actionIncreaseLabelFont->setShortcut(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ShiftModifier | Qt::Key_Plus));
#else
        QKeySequence(Qt::ShiftModifier + Qt::Key_Plus));
#endif
    connect(ui->actionIncreaseLabelFont, &QAction::triggered, this,
            [this]()
            {
                VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
                settings->SetPatternLabelFontSize(settings->GetPatternLabelFontSize() + 1);
                if (m_sceneDraw)
                {
                    m_sceneDraw->update();
                }

                if (m_sceneDetails)
                {
                    m_sceneDetails->update();
                }
            });

    ui->actionDecreaseLabelFont->setShortcut(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ShiftModifier | Qt::Key_Minus));
#else
        QKeySequence(Qt::ShiftModifier + Qt::Key_Minus));
#endif
    connect(ui->actionDecreaseLabelFont, &QAction::triggered, this,
            [this]()
            {
                VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
                settings->SetPatternLabelFontSize(settings->GetPatternLabelFontSize() - 1);
                if (m_sceneDraw)
                {
                    m_sceneDraw->update();
                }

                if (m_sceneDetails)
                {
                    m_sceneDetails->update();
                }
            });

    ui->actionOriginalLabelFont->setShortcut(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ShiftModifier | Qt::Key_0));
#else
        QKeySequence(Qt::ShiftModifier + Qt::Key_0));
#endif
    connect(ui->actionOriginalLabelFont, &QAction::triggered, this,
            [this]()
            {
                VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
                settings->SetPatternLabelFontSize(VCommonSettings::GetDefPatternLabelFontSize());
                if (m_sceneDraw)
                {
                    m_sceneDraw->update();
                }

                if (m_sceneDetails)
                {
                    m_sceneDetails->update();
                }
            });

    ui->actionHideLabels->setShortcut(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::AltModifier | Qt::Key_L));
#else
        QKeySequence(Qt::AltModifier + Qt::Key_L));
#endif
    ui->actionHideLabels->setChecked(VAbstractValApplication::VApp()->ValentinaSettings()->GetHideLabels());
    connect(ui->actionHideLabels, &QAction::triggered, this,
            [this](bool checked)
            {
                VAbstractValApplication::VApp()->ValentinaSettings()->SetHideLabels(checked);
                if (m_sceneDraw)
                {
                    m_sceneDraw->update();
                }

                if (m_sceneDetails)
                {
                    m_sceneDetails->update();
                }
            });

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolBarDrawTools()
{
    SetupDrawToolsIcons();

    VValentinaSettings *settings = VApplication::VApp()->ValentinaSettings();

    ui->toolBarPointTools->clear();
    if (settings->IsUseToolGroups())
    {
        // Point tools
        {
            auto *linePointToolMenu = new QMenu(this);
            linePointToolMenu->addAction(ui->actionEndLineTool);
            linePointToolMenu->addAction(ui->actionAlongLineTool);
            linePointToolMenu->addAction(ui->actionMidpointTool);

            auto *linePointTool = new VToolButtonPopup(this);
            linePointTool->setMenu(linePointToolMenu);
            linePointTool->setDefaultAction(ui->actionEndLineTool);

            ui->toolBarPointTools->addWidget(linePointTool);
        }

        {
            auto *angleLinePointToolMenu = new QMenu(this);
            angleLinePointToolMenu->addAction(ui->actionNormalTool);
            angleLinePointToolMenu->addAction(ui->actionBisectorTool);
            angleLinePointToolMenu->addAction(ui->actionHeightTool);

            auto *angleLinePointTool = new VToolButtonPopup(this);
            angleLinePointTool->setMenu(angleLinePointToolMenu);
            angleLinePointTool->setDefaultAction(ui->actionNormalTool);

            ui->toolBarPointTools->addWidget(angleLinePointTool);
        }

        {
            auto *lineIntersectionPointToolMenu = new QMenu(this);
            lineIntersectionPointToolMenu->addAction(ui->actionPointOfIntersectionTool);
            lineIntersectionPointToolMenu->addAction(ui->actionLineIntersectTool);

            auto *lineIntersectionPointTool = new VToolButtonPopup(this);
            lineIntersectionPointTool->setMenu(lineIntersectionPointToolMenu);
            lineIntersectionPointTool->setDefaultAction(ui->actionPointOfIntersectionTool);

            ui->toolBarPointTools->addWidget(lineIntersectionPointTool);
        }

        {
            auto *specialPointToolMenu = new QMenu(this);
            specialPointToolMenu->addAction(ui->actionShoulderPointTool);
            specialPointToolMenu->addAction(ui->actionTriangleTool);

            auto *specialPointTool = new VToolButtonPopup(this);
            specialPointTool->setMenu(specialPointToolMenu);
            specialPointTool->setDefaultAction(ui->actionShoulderPointTool);

            ui->toolBarPointTools->addWidget(specialPointTool);
        }

        {
            auto *axisPointToolMenu = new QMenu(this);
            axisPointToolMenu->addAction(ui->actionLineIntersectAxisTool);
            axisPointToolMenu->addAction(ui->actionCurveIntersectAxisTool);
            axisPointToolMenu->addAction(ui->actionArcIntersectAxisTool);

            auto *axisPointTool = new VToolButtonPopup(this);
            axisPointTool->setMenu(axisPointToolMenu);
            axisPointTool->setDefaultAction(ui->actionLineIntersectAxisTool);

            ui->toolBarPointTools->addWidget(axisPointTool);
        }

        {
            auto *curveSegmentPointToolMenu = new QMenu(this);
            curveSegmentPointToolMenu->addAction(ui->actionSplineCutPointTool);
            curveSegmentPointToolMenu->addAction(ui->actionSplinePathCutPointTool);
            curveSegmentPointToolMenu->addAction(ui->actionArcCutPointTool);

            auto *curveSegmentPointTool = new VToolButtonPopup(this);
            curveSegmentPointTool->setMenu(curveSegmentPointToolMenu);
            curveSegmentPointTool->setDefaultAction(ui->actionSplineCutPointTool);

            ui->toolBarPointTools->addWidget(curveSegmentPointTool);
        }

        {
            auto *curveIntersectionPointToolMenu = new QMenu(this);
            curveIntersectionPointToolMenu->addAction(ui->actionIntersectionCurvesTool);
            curveIntersectionPointToolMenu->addAction(ui->actionPointOfIntersectionArcsTool);
            curveIntersectionPointToolMenu->addAction(ui->actionPointOfIntersectionCirclesTool);

            auto *curveIntersectionPointTool = new VToolButtonPopup(this);
            curveIntersectionPointTool->setMenu(curveIntersectionPointToolMenu);
            curveIntersectionPointTool->setDefaultAction(ui->actionIntersectionCurvesTool);

            ui->toolBarPointTools->addWidget(curveIntersectionPointTool);
        }

        {
            auto *tangentPointToolMenu = new QMenu(this);
            tangentPointToolMenu->addAction(ui->actionPointFromArcAndTangentTool);
            tangentPointToolMenu->addAction(ui->actionPointFromCircleAndTangentTool);
            tangentPointToolMenu->addAction(ui->actionPointOfContactTool);

            auto *tangentPointTool = new VToolButtonPopup(this);
            tangentPointTool->setMenu(tangentPointToolMenu);
            tangentPointTool->setDefaultAction(ui->actionPointFromArcAndTangentTool);

            ui->toolBarPointTools->addWidget(tangentPointTool);
        }
    }
    else
    {
        ui->toolBarPointTools->addAction(ui->actionEndLineTool);
        ui->toolBarPointTools->addAction(ui->actionAlongLineTool);
        ui->toolBarPointTools->addAction(ui->actionMidpointTool);

        ui->toolBarPointTools->addAction(ui->actionNormalTool);
        ui->toolBarPointTools->addAction(ui->actionBisectorTool);
        ui->toolBarPointTools->addAction(ui->actionHeightTool);

        ui->toolBarPointTools->addAction(ui->actionPointOfIntersectionTool);
        ui->toolBarPointTools->addAction(ui->actionLineIntersectTool);

        ui->toolBarPointTools->addAction(ui->actionShoulderPointTool);
        ui->toolBarPointTools->addAction(ui->actionTriangleTool);

        ui->toolBarPointTools->addAction(ui->actionLineIntersectAxisTool);
        ui->toolBarPointTools->addAction(ui->actionCurveIntersectAxisTool);
        ui->toolBarPointTools->addAction(ui->actionArcIntersectAxisTool);

        ui->toolBarPointTools->addAction(ui->actionSplineCutPointTool);
        ui->toolBarPointTools->addAction(ui->actionSplinePathCutPointTool);
        ui->toolBarPointTools->addAction(ui->actionArcCutPointTool);

        ui->toolBarPointTools->addAction(ui->actionIntersectionCurvesTool);
        ui->toolBarPointTools->addAction(ui->actionPointOfIntersectionArcsTool);
        ui->toolBarPointTools->addAction(ui->actionPointOfIntersectionCirclesTool);

        ui->toolBarPointTools->addAction(ui->actionPointFromArcAndTangentTool);
        ui->toolBarPointTools->addAction(ui->actionPointFromCircleAndTangentTool);
        ui->toolBarPointTools->addAction(ui->actionPointOfContactTool);
    }

    ui->toolBarCurveTools->clear();
    if (settings->IsUseToolGroups())
    {
        // Curve tools
        {
            auto *curveToolMenu = new QMenu(this);
            curveToolMenu->addAction(ui->actionSplineTool);
            curveToolMenu->addAction(ui->actionCubicBezierTool);
            curveToolMenu->addAction(ui->actionSplinePathTool);
            curveToolMenu->addAction(ui->actionCubicBezierPathTool);
            curveToolMenu->addAction(ui->actionArcTool);
            curveToolMenu->addAction(ui->actionArcWithLengthTool);
            curveToolMenu->addAction(ui->actionEllipticalArcTool);

            auto *curvePointTool = new VToolButtonPopup(this);
            curvePointTool->setMenu(curveToolMenu);
            curvePointTool->setDefaultAction(ui->actionSplineTool);

            ui->toolBarCurveTools->addWidget(curvePointTool);
        }
    }
    else
    {
        ui->toolBarCurveTools->addAction(ui->actionSplineTool);
        ui->toolBarCurveTools->addAction(ui->actionCubicBezierTool);
        ui->toolBarCurveTools->addAction(ui->actionSplinePathTool);
        ui->toolBarCurveTools->addAction(ui->actionCubicBezierPathTool);
        ui->toolBarCurveTools->addAction(ui->actionArcTool);
        ui->toolBarCurveTools->addAction(ui->actionArcWithLengthTool);
        ui->toolBarCurveTools->addAction(ui->actionEllipticalArcTool);
    }

    ui->toolBarOperationTools->clear();
    if (settings->IsUseToolGroups())
    {
        // Group tools
        ui->toolBarOperationTools->addAction(ui->actionGroupTool);

        {
            auto *symmetryToolMenu = new QMenu(this);
            symmetryToolMenu->addAction(ui->actionFlippingByAxisTool);
            symmetryToolMenu->addAction(ui->actionFlippingByLineTool);

            auto *symmetryTool = new VToolButtonPopup(this);
            symmetryTool->setMenu(symmetryToolMenu);
            symmetryTool->setDefaultAction(ui->actionFlippingByAxisTool);

            ui->toolBarOperationTools->addWidget(symmetryTool);
        }

        {
            auto *transformToolMenu = new QMenu(this);
            transformToolMenu->addAction(ui->actionRotationTool);
            transformToolMenu->addAction(ui->actionMoveTool);

            auto *transformTool = new VToolButtonPopup(this);
            transformTool->setMenu(transformToolMenu);
            transformTool->setDefaultAction(ui->actionRotationTool);

            ui->toolBarOperationTools->addWidget(transformTool);
        }

        ui->toolBarOperationTools->addAction(ui->actionTrueDartsTool);
        ui->toolBarOperationTools->addAction(ui->actionExportDraw);
    }
    else
    {
        ui->toolBarOperationTools->addAction(ui->actionGroupTool);

        ui->toolBarOperationTools->addAction(ui->actionFlippingByAxisTool);
        ui->toolBarOperationTools->addAction(ui->actionFlippingByLineTool);

        ui->toolBarOperationTools->addAction(ui->actionRotationTool);
        ui->toolBarOperationTools->addAction(ui->actionMoveTool);

        ui->toolBarOperationTools->addAction(ui->actionTrueDartsTool);
        ui->toolBarOperationTools->addAction(ui->actionExportDraw);
    }

    ui->toolBarDetailTools->clear();
    if (settings->IsUseToolGroups())
    {
        // Detail tools
        ui->toolBarDetailTools->addAction(ui->actionNewDetailTool);
        {
            auto *detailToolMenu = new QMenu(this);
            detailToolMenu->addAction(ui->actionUnionDetailsTool);
            detailToolMenu->addAction(ui->actionDuplicateDetailTool);

            auto *detailTool = new VToolButtonPopup(this);
            detailTool->setMenu(detailToolMenu);
            detailTool->setDefaultAction(ui->actionUnionDetailsTool);

            ui->toolBarDetailTools->addWidget(detailTool);
        }

        {
            auto *internalDetailItemToolMenu = new QMenu(this);
            internalDetailItemToolMenu->addAction(ui->actionInternalPathTool);
            internalDetailItemToolMenu->addAction(ui->actionPinTool);
            internalDetailItemToolMenu->addAction(ui->actionInsertNodeTool);
            internalDetailItemToolMenu->addAction(ui->actionPlaceLabelTool);

            auto *detailTool = new VToolButtonPopup(this);
            detailTool->setMenu(internalDetailItemToolMenu);
            detailTool->setDefaultAction(ui->actionInternalPathTool);

            ui->toolBarDetailTools->addWidget(detailTool);
        }

        ui->toolBarDetailTools->addAction(ui->actionDetailExportAs);
    }
    else
    {
        ui->toolBarDetailTools->addAction(ui->actionNewDetailTool);

        ui->toolBarDetailTools->addAction(ui->actionUnionDetailsTool);
        ui->toolBarDetailTools->addAction(ui->actionDuplicateDetailTool);

        ui->toolBarDetailTools->addAction(ui->actionInternalPathTool);
        ui->toolBarDetailTools->addAction(ui->actionPinTool);
        ui->toolBarDetailTools->addAction(ui->actionInsertNodeTool);
        ui->toolBarDetailTools->addAction(ui->actionPlaceLabelTool);

        ui->toolBarDetailTools->addAction(ui->actionDetailExportAs);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitToolButtons()
{
    connect(ui->actionToolSelect, &QAction::triggered, this, &MainWindow::ArrowTool);

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Check if all tools were connected.");

    connect(ui->actionEndLineTool, &QAction::triggered, this, &MainWindow::ToolEndLine);
    connect(ui->actionLineTool, &QAction::triggered, this, &MainWindow::ToolLine);
    connect(ui->actionAlongLineTool, &QAction::triggered, this, &MainWindow::ToolAlongLine);
    connect(ui->actionShoulderPointTool, &QAction::triggered, this, &MainWindow::ToolShoulderPoint);
    connect(ui->actionNormalTool, &QAction::triggered, this, &MainWindow::ToolNormal);
    connect(ui->actionBisectorTool, &QAction::triggered, this, &MainWindow::ToolBisector);
    connect(ui->actionLineIntersectTool, &QAction::triggered, this, &MainWindow::ToolLineIntersect);
    connect(ui->actionSplineTool, &QAction::triggered, this, &MainWindow::ToolSpline);
    connect(ui->actionCubicBezierTool, &QAction::triggered, this, &MainWindow::ToolCubicBezier);
    connect(ui->actionArcTool, &QAction::triggered, this, &MainWindow::ToolArc);
    connect(ui->actionSplinePathTool, &QAction::triggered, this, &MainWindow::ToolSplinePath);
    connect(ui->actionCubicBezierPathTool, &QAction::triggered, this, &MainWindow::ToolCubicBezierPath);
    connect(ui->actionPointOfContactTool, &QAction::triggered, this, &MainWindow::ToolPointOfContact);
    connect(ui->actionNewDetailTool, &QAction::triggered, this, &MainWindow::ToolDetail);
    connect(ui->actionInternalPathTool, &QAction::triggered, this, &MainWindow::ToolPiecePath);
    connect(ui->actionHeightTool, &QAction::triggered, this, &MainWindow::ToolHeight);
    connect(ui->actionTriangleTool, &QAction::triggered, this, &MainWindow::ToolTriangle);
    connect(ui->actionPointOfIntersectionTool, &QAction::triggered, this, &MainWindow::ToolPointOfIntersection);
    connect(ui->actionSplineCutPointTool, &QAction::triggered, this, &MainWindow::ToolCutSpline);
    connect(ui->actionSplinePathCutPointTool, &QAction::triggered, this, &MainWindow::ToolCutSplinePath);
    connect(ui->actionUnionDetailsTool, &QAction::triggered, this, &MainWindow::ToolUnionDetails);
    connect(ui->actionDuplicateDetailTool, &QAction::triggered, this, &MainWindow::ToolDuplicateDetail);
    connect(ui->actionArcCutPointTool, &QAction::triggered, this, &MainWindow::ToolCutArc);
    connect(ui->actionLineIntersectAxisTool, &QAction::triggered, this, &MainWindow::ToolLineIntersectAxis);
    connect(ui->actionCurveIntersectAxisTool, &QAction::triggered, this, &MainWindow::ToolCurveIntersectAxis);
    connect(ui->actionArcIntersectAxisTool, &QAction::triggered, this, &MainWindow::ToolArcIntersectAxis);
    connect(ui->actionLayoutSettings, &QAction::triggered, this, &MainWindow::ToolLayoutSettings);
    connect(ui->actionPointOfIntersectionArcsTool, &QAction::triggered, this, &MainWindow::ToolPointOfIntersectionArcs);
    connect(ui->actionPointOfIntersectionCirclesTool, &QAction::triggered, this,
            &MainWindow::ToolPointOfIntersectionCircles);
    connect(ui->actionIntersectionCurvesTool, &QAction::triggered, this, &MainWindow::ToolPointOfIntersectionCurves);
    connect(ui->actionPointFromCircleAndTangentTool, &QAction::triggered, this,
            &MainWindow::ToolPointFromCircleAndTangent);
    connect(ui->actionPointFromArcAndTangentTool, &QAction::triggered, this, &MainWindow::ToolPointFromArcAndTangent);
    connect(ui->actionArcWithLengthTool, &QAction::triggered, this, &MainWindow::ToolArcWithLength);
    connect(ui->actionTrueDartsTool, &QAction::triggered, this, &MainWindow::ToolTrueDarts);
    connect(ui->actionGroupTool, &QAction::triggered, this, &MainWindow::ToolGroup);
    connect(ui->actionRotationTool, &QAction::triggered, this, &MainWindow::ToolRotation);
    connect(ui->actionFlippingByLineTool, &QAction::triggered, this, &MainWindow::ToolFlippingByLine);
    connect(ui->actionFlippingByAxisTool, &QAction::triggered, this, &MainWindow::ToolFlippingByAxis);
    connect(ui->actionMoveTool, &QAction::triggered, this, &MainWindow::ToolMove);
    connect(ui->actionMidpointTool, &QAction::triggered, this, &MainWindow::ToolMidpoint);
    connect(ui->actionExportDraw, &QAction::triggered, this, &MainWindow::ExportDrawAs);
    connect(ui->actionLayoutExportAs, &QAction::triggered, this, &MainWindow::ExportLayoutAs);
    connect(ui->actionDetailExportAs, &QAction::triggered, this, &MainWindow::ExportDetailsAs);
    connect(ui->actionEllipticalArcTool, &QAction::triggered, this, &MainWindow::ToolEllipticalArc);
    connect(ui->actionPinTool, &QAction::triggered, this, &MainWindow::ToolPin);
    connect(ui->actionInsertNodeTool, &QAction::triggered, this, &MainWindow::ToolInsertNode);
    connect(ui->actionPlaceLabelTool, &QAction::triggered, this, &MainWindow::ToolPlaceLabel);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseMove save mouse position and show user.
 * @param scenePos position mouse.
 */
void MainWindow::MouseMove(const QPointF &scenePos)
{
    if (not m_mouseCoordinate.isNull())
    {
        m_mouseCoordinate->setText(
            QStringLiteral("%1, %2 (%3)")
                .arg(static_cast<qint32>(VAbstractValApplication::VApp()->fromPixel(scenePos.x())))
                .arg(static_cast<qint32>(VAbstractValApplication::VApp()->fromPixel(scenePos.y())))
                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief CancelTool cancel tool.
 */
void MainWindow::CancelTool()
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were handled.");

    qCDebug(vMainWindow, "Canceling tool.");
    if (not m_dialogTool.isNull())
    {
        m_dialogTool->hide();
        m_dialogTool->deleteLater();
    }
    qCDebug(vMainWindow, "Dialog closed.");

    currentScene->setFocus(Qt::OtherFocusReason);
    currentScene->clearSelection();
    emit ui->view->itemClicked(nullptr); // Hide visualization to avoid a crash

    switch (m_currentTool)
    {
        case Tool::Arrow:
            ui->actionToolSelect->setChecked(false);
            m_statusLabel->setText(QString());

            // Crash: using CRTL+Z while using line tool.
            // related bug report:
            // https://bitbucket.org/dismine/valentina/issues/454/crash-using-crtl-z-while-using-line-tool
            undoAction->setEnabled(false);
            redoAction->setEnabled(false);
            VAbstractTool::m_suppressContextMenu = true;
            return;
        case Tool::BasePoint:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::LAST_ONE_DO_NOT_USE:
        case Tool::NodePoint:
        case Tool::NodeArc:
        case Tool::NodeElArc:
        case Tool::NodeSpline:
        case Tool::NodeSplinePath:
        case Tool::BackgroundImage:
        case Tool::BackgroundImageControls:
        case Tool::BackgroundPixmapImage:
        case Tool::BackgroundSVGImage:
            Q_UNREACHABLE(); //-V501
            // Nothing to do here because we can't create this tool from main window.
            break;
        case Tool::EndLine:
            ui->actionEndLineTool->setChecked(false);
            break;
        case Tool::Line:
            ui->actionLineTool->setChecked(false);
            break;
        case Tool::AlongLine:
            ui->actionAlongLineTool->setChecked(false);
            break;
        case Tool::Midpoint:
            ui->actionMidpointTool->setChecked(false);
            break;
        case Tool::ShoulderPoint:
            ui->actionShoulderPointTool->setChecked(false);
            break;
        case Tool::Normal:
            ui->actionNormalTool->setChecked(false);
            break;
        case Tool::Bisector:
            ui->actionBisectorTool->setChecked(false);
            break;
        case Tool::LineIntersect:
            ui->actionLineIntersectTool->setChecked(false);
            break;
        case Tool::Spline:
            ui->actionSplineTool->setChecked(false);
            break;
        case Tool::CubicBezier:
            ui->actionCubicBezierTool->setChecked(false);
            break;
        case Tool::Arc:
            ui->actionArcTool->setChecked(false);
            break;
        case Tool::ArcWithLength:
            ui->actionArcWithLengthTool->setChecked(false);
            break;
        case Tool::SplinePath:
            ui->actionSplinePathTool->setChecked(false);
            break;
        case Tool::CubicBezierPath:
            ui->actionCubicBezierPathTool->setChecked(false);
            break;
        case Tool::PointOfContact:
            ui->actionPointOfContactTool->setChecked(false);
            break;
        case Tool::Piece:
            ui->actionNewDetailTool->setChecked(false);
            break;
        case Tool::PiecePath:
            ui->actionInternalPathTool->setChecked(false);
            break;
        case Tool::Height:
            ui->actionHeightTool->setChecked(false);
            break;
        case Tool::Triangle:
            ui->actionTriangleTool->setChecked(false);
            break;
        case Tool::PointOfIntersection:
            ui->actionPointOfIntersectionTool->setChecked(false);
            break;
        case Tool::CutSpline:
            ui->actionSplineCutPointTool->setChecked(false);
            break;
        case Tool::CutSplinePath:
            ui->actionSplinePathCutPointTool->setChecked(false);
            break;
        case Tool::UnionDetails:
            ui->actionUnionDetailsTool->setChecked(false);
            break;
        case Tool::DuplicateDetail:
            ui->actionDuplicateDetailTool->setChecked(false);
            break;
        case Tool::CutArc:
            ui->actionArcCutPointTool->setChecked(false);
            break;
        case Tool::LineIntersectAxis:
            ui->actionLineIntersectAxisTool->setChecked(false);
            break;
        case Tool::CurveIntersectAxis:
            ui->actionCurveIntersectAxisTool->setChecked(false);
            break;
        case Tool::ArcIntersectAxis:
            ui->actionArcIntersectAxisTool->setChecked(false);
            break;
        case Tool::PointOfIntersectionArcs:
            ui->actionPointOfIntersectionArcsTool->setChecked(false);
            break;
        case Tool::PointOfIntersectionCircles:
            ui->actionPointOfIntersectionCirclesTool->setChecked(false);
            break;
        case Tool::PointOfIntersectionCurves:
            ui->actionIntersectionCurvesTool->setChecked(false);
            break;
        case Tool::PointFromCircleAndTangent:
            ui->actionPointFromCircleAndTangentTool->setChecked(false);
            break;
        case Tool::PointFromArcAndTangent:
            ui->actionPointFromArcAndTangentTool->setChecked(false);
            break;
        case Tool::TrueDarts:
            ui->actionTrueDartsTool->setChecked(false);
            break;
        case Tool::Group:
            ui->actionGroupTool->setChecked(false);
            break;
        case Tool::Rotation:
            ui->actionRotationTool->setChecked(false);
            break;
        case Tool::FlippingByLine:
            ui->actionFlippingByLineTool->setChecked(false);
            break;
        case Tool::FlippingByAxis:
            ui->actionFlippingByAxisTool->setChecked(false);
            break;
        case Tool::Move:
            ui->actionMoveTool->setChecked(false);
            break;
        case Tool::EllipticalArc:
            ui->actionEllipticalArcTool->setChecked(false);
            break;
        case Tool::Pin:
            ui->actionPinTool->setChecked(false);
            break;
        case Tool::InsertNode:
            ui->actionInsertNodeTool->setChecked(false);
            break;
        case Tool::PlaceLabel:
            ui->actionPlaceLabelTool->setChecked(false);
            break;
    }

    // Crash: using CRTL+Z while using line tool.
    // related bug report:
    // https://bitbucket.org/dismine/valentina/issues/454/crash-using-crtl-z-while-using-line-tool
    undoAction->setEnabled(VAbstractApplication::VApp()->getUndoStack()->canUndo());
    redoAction->setEnabled(VAbstractApplication::VApp()->getUndoStack()->canRedo());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetupDrawToolsIcons()
{
    const QString resource = QStringLiteral("toolicon");

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were handled.");

    ui->actionLineTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("line.png")));
    ui->actionEndLineTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("segment.png")));
    ui->actionAlongLineTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("along_line.png")));
    ui->actionMidpointTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("midpoint.png")));
    ui->actionNormalTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("normal.png")));
    ui->actionBisectorTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("bisector.png")));
    ui->actionHeightTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("height.png")));
    ui->actionPointOfIntersectionTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("point_of_intersection.png")));
    ui->actionLineIntersectTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("intersect.png")));
    ui->actionShoulderPointTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("shoulder.png")));
    ui->actionTriangleTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("triangle.png")));
    ui->actionLineIntersectAxisTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("line_intersect_axis.png")));
    ui->actionCurveIntersectAxisTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("curve_intersect_axis.png")));
    ui->actionArcIntersectAxisTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("arc_intersect_axis.png")));
    ui->actionSplineCutPointTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("spline_cut_point.png")));
    ui->actionSplinePathCutPointTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("splinePath_cut_point.png")));
    ui->actionArcCutPointTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("arc_cut.png")));
    ui->actionIntersectionCurvesTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("intersection_curves.png")));
    ui->actionPointOfIntersectionArcsTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("point_of_intersection_arcs.png")));
    ui->actionPointOfIntersectionCirclesTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("point_of_intersection_circles.png")));
    ui->actionPointFromArcAndTangentTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("point_from_arc_and_tangent.png")));
    ui->actionPointFromCircleAndTangentTool->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("point_from_circle_and_tangent.png")));
    ui->actionPointOfContactTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("point_of_contact.png")));
    ui->actionSplineTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("spline.png")));
    ui->actionCubicBezierTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("cubic_bezier.png")));
    ui->actionSplinePathTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("splinePath.png")));
    ui->actionCubicBezierPathTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("cubic_bezier_path.png")));
    ui->actionArcTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("arc.png")));
    ui->actionArcWithLengthTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("arc_with_length.png")));
    ui->actionEllipticalArcTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("el_arc.png")));
    ui->actionGroupTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("group_plus.png")));
    ui->actionFlippingByAxisTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("flipping_axis.png")));
    ui->actionFlippingByLineTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("flipping_line.png")));
    ui->actionRotationTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("rotation.png")));
    ui->actionMoveTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("move.png")));
    ui->actionTrueDartsTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("true_darts.png")));
    ui->actionNewDetailTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("new_detail.png")));
    ui->actionUnionDetailsTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("union.png")));
    ui->actionDuplicateDetailTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("duplicate_detail.png")));
    ui->actionInternalPathTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("path.png")));
    ui->actionPinTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("pin.png")));
    ui->actionInsertNodeTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("insert_node.png")));
    ui->actionPlaceLabelTool->setIcon(VTheme::GetIconResource(resource, QStringLiteral("place_label.png")));
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowTool enable arrow tool.
 */
void MainWindow::ArrowTool(bool checked)
{
    if (checked && m_currentTool != Tool::Arrow)
    {
        qCDebug(vMainWindow, "Arrow tool.");
        CancelTool();
        ui->actionToolSelect->setChecked(true);
        m_currentTool = Tool::Arrow;
        emit EnableItemMove(true);
        emit ItemsSelection(SelectionType::ByMouseRelease);
        VAbstractTool::m_suppressContextMenu = false;

        // Only true for rubber band selection
        emit EnableLabelSelection(true);
        emit EnablePointSelection(false);
        emit EnableLineSelection(false);
        emit EnableArcSelection(false);
        emit EnableElArcSelection(false);
        emit EnableSplineSelection(false);
        emit EnableSplinePathSelection(false);
        emit EnableNodeLabelSelection(true);
        emit EnableNodePointSelection(true);
        emit EnableDetailSelection(true); // Disable when done visualization details
        emit EnableBackgroundImageSelection(true);

        // Hovering
        emit EnableLabelHover(true);
        emit EnablePointHover(true);
        emit EnableLineHover(true);
        emit EnableArcHover(true);
        emit EnableElArcHover(true);
        emit EnableSplineHover(true);
        emit EnableSplinePathHover(true);
        emit EnableNodeLabelHover(true);
        emit EnableNodePointHover(true);
        emit EnableDetailHover(true);
        emit EnableImageBackgroundHover(true);

        ui->view->AllowRubberBand(true);
        ui->view->viewport()->unsetCursor();
        ui->view->viewport()->setCursor(QCursor(Qt::ArrowCursor));
        ui->view->setCurrentCursorShape(); // Hack to fix problem with a cursor
        m_statusLabel->setText(QString());
        ui->view->setShowToolOptions(true);
        qCDebug(vMainWindow, "Enabled arrow tool.");
    }
    else
    {
        ui->actionToolSelect->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyPressEvent handle key press events.
 * @param event key event.
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            ArrowTool(true);
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            EndVisualization();
            break;
        default:
            break;
    }
    QMainWindow::keyPressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveCurrentScene save scene options before set another.
 */
void MainWindow::SaveCurrentScene()
{
    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation ||
        VAbstractValApplication::VApp()->GetDrawMode() == Draw::Modeling)
    {
        auto *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        /*Save transform*/
        scene->setTransform(ui->view->transform());
        /*Save scroll bars value for previous scene.*/
        QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
        scene->setHorScrollBar(horScrollBar->value());
        QScrollBar *verScrollBar = ui->view->verticalScrollBar();
        scene->setVerScrollBar(verScrollBar->value());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RestoreCurrentScene restore scene options after change.
 */
void MainWindow::RestoreCurrentScene()
{
    auto *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
    SCASSERT(scene != nullptr)

    /*Set transform for current scene*/
    ui->view->setTransform(scene->transform());
    /*Set value for current scene scroll bar.*/
    QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
    horScrollBar->setValue(scene->getHorScrollBar());
    QScrollBar *verScrollBar = ui->view->verticalScrollBar();
    verScrollBar->setValue(scene->getVerScrollBar());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ActionDraw show draw scene.
 * @param checked true - button checked.
 */
void MainWindow::ActionDraw(bool checked)
{
    if (checked)
    {
        qCDebug(vMainWindow, "Show draw scene");
        ArrowTool(true);

        const QString resource = QStringLiteral("icon");

        m_leftGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-next-skip.png")));
        m_rightGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-next.png")));

        ui->actionDraw->setChecked(true);
        ui->actionDetails->setChecked(false);
        ui->actionLayout->setChecked(false);
        SaveCurrentScene();

        currentScene = m_sceneDraw;
        ui->view->setScene(currentScene);
        RestoreCurrentScene();

        VAbstractValApplication::VApp()->SetDrawMode(Draw::Calculation);
        m_comboBoxDraws->setCurrentIndex(m_currentDrawIndex); // restore current pattern peace
        m_drawMode = true;

        SetEnableTool(true);
        SetEnableWidgets(true);

        if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
        {
            ui->toolBarOption->setVisible(true);
        }

        ui->dockWidgetLayoutPages->setVisible(false);
        ui->dockWidgetToolOptions->setVisible(m_toolOptionsActive);

        ui->dockWidgetGroups->setWidget(m_groupsWidget);
        ui->dockWidgetGroups->setWindowTitle(tr("Groups of visibility"));
        ui->dockWidgetGroups->setVisible(m_groupsActive);
        ui->dockWidgetGroups->setToolTip(tr("Contains all visibility groups"));

        ui->dockWidgetBackgroundImages->setVisible(m_backgroundImagesActive);
    }
    else
    {
        ui->actionDraw->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ActionDetails show details scene.
 * @param checked true - button checked.
 */
void MainWindow::ActionDetails(bool checked)
{
    if (checked)
    {
        ArrowTool(true);

        if (m_drawMode)
        {
            m_currentDrawIndex = m_comboBoxDraws->currentIndex(); // save current pattern piece
            m_drawMode = false;
        }
        m_comboBoxDraws->setCurrentIndex(m_comboBoxDraws->count() - 1); // Need to get data about all details

        const QString resource = QStringLiteral("icon");
        m_leftGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-previous.png")));
        m_rightGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-next.png")));

        ui->actionDraw->setChecked(false);
        ui->actionDetails->setChecked(true);
        ui->actionLayout->setChecked(false);

        if (not VAbstractValApplication::VApp()->getOpeningPattern())
        {
            if (pattern->DataPieces()->count() == 0)
            {
                QMessageBox::information(this, tr("Detail mode"),
                                         tr("You can't use Detail mode yet. "
                                            "Please, create at least one workpiece."),
                                         QMessageBox::Ok, QMessageBox::Ok);
                ActionDraw(true);
                return;
            }
        }

        m_detailsWidget->UpdateList();

        qCDebug(vMainWindow, "Show details scene");
        SaveCurrentScene();

        currentScene = m_sceneDetails;
        emit ui->view->itemClicked(nullptr);
        ui->view->setScene(currentScene);
        RestoreCurrentScene();

        VAbstractValApplication::VApp()->SetDrawMode(Draw::Modeling);
        SetEnableTool(true);
        SetEnableWidgets(true);

        if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
        {
            ui->toolBarOption->setVisible(true);
        }

        ui->dockWidgetLayoutPages->setVisible(false);

        ui->dockWidgetGroups->setWidget(m_detailsWidget);
        ui->dockWidgetGroups->setWindowTitle(tr("Details"));
        ui->dockWidgetGroups->setVisible(m_groupsActive);
        ui->dockWidgetGroups->setToolTip(tr("Show which details will go in layout"));

        ui->dockWidgetToolOptions->setVisible(m_toolOptionsActive);
        ui->dockWidgetBackgroundImages->setVisible(false);

        m_statusLabel->setText(QString());

        WarningNotUniquePieceName(pattern->DataPieces());
    }
    else
    {
        ui->actionDetails->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ActionLayout begin creation layout.
 * @param checked true - button checked.
 */
void MainWindow::ActionLayout(bool checked)
{
    if (not checked)
    {
        ui->actionLayout->setChecked(true);
        return;
    }

    ArrowTool(true);

    if (m_drawMode)
    {
        m_currentDrawIndex = m_comboBoxDraws->currentIndex(); // save current pattern piece
        m_drawMode = false;
    }
    m_comboBoxDraws->setCurrentIndex(m_comboBoxDraws->count() - 1); // Need to get data about all details

    const QString resource = QStringLiteral("icon");
    m_leftGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-previous.png")));
    m_rightGoToStage->setPixmap(VTheme::GetPixmapResource(resource, QStringLiteral("24x24/go-previous-skip.png")));

    ui->actionDraw->setChecked(false);
    ui->actionDetails->setChecked(false);
    ui->actionLayout->setChecked(true);

    QVector<DetailForLayout> details;
    if (not VAbstractValApplication::VApp()->getOpeningPattern())
    {
        const QHash<quint32, VPiece> *allDetails = pattern->DataPieces();
        if (allDetails->count() == 0)
        {
            QMessageBox::information(this, tr("Layout mode"),
                                     tr("You can't use Layout mode yet. "
                                        "Please, create at least one workpiece."),
                                     QMessageBox::Ok, QMessageBox::Ok);
            ActionDraw(true);
            return;
        }

        WarningNotUniquePieceName(allDetails);
        details = SortDetailsForLayout(allDetails);

        if (details.count() == 0)
        {
            QMessageBox::information(this, tr("Layout mode"),
                                     tr("You can't use Layout mode yet. Please, "
                                        "include at least one detail in layout."),
                                     QMessageBox::Ok, QMessageBox::Ok);
            VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation ? ActionDraw(true)
                                                                                : ActionDetails(true);
            return;
        }
    }

    m_comboBoxDraws->setCurrentIndex(-1); // Hide pattern pieces

    qCDebug(vMainWindow, "Show layout scene");

    SaveCurrentScene();

    try
    {
        listDetails = PrepareDetailsForLayout(details);
    }
    catch (VException &e)
    {
        listDetails.clear();
        QMessageBox::warning(this, tr("Layout mode"),
                             tr("You can't use Layout mode yet.") + QStringLiteral(" \n") + e.ErrorMessage(),
                             QMessageBox::Ok, QMessageBox::Ok);
        VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation ? ActionDraw(true) : ActionDetails(true);
        return;
    }

    currentScene = tempSceneLayout;
    emit ui->view->itemClicked(nullptr);
    ui->view->setScene(currentScene);

    VAbstractValApplication::VApp()->SetDrawMode(Draw::Layout);
    SetEnableTool(true);
    SetEnableWidgets(true);

    if (not m_mouseCoordinate.isNull())
    {
        m_mouseCoordinate->setText(QString());
    }

    if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
    {
        ui->toolBarOption->setVisible(false);
    }

    ui->dockWidgetLayoutPages->setVisible(true);
    ui->dockWidgetToolOptions->setVisible(false);
    ui->dockWidgetGroups->setVisible(false);
    ui->dockWidgetBackgroundImages->setVisible(false);

    ShowPaper(ui->listWidget->currentRow());

    if (m_layoutSettings->LayoutScenes().isEmpty() || m_layoutSettings->IsLayoutStale())
    {
        ui->actionLayoutSettings->activate(QAction::Trigger);
    }

    m_statusLabel->setText(QString());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief on_actionSaveAs_triggered save as pattern file.
 * @return true for successes saving.
 */
auto MainWindow::on_actionSaveAs_triggered() -> bool
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    QString patternPath = VAbstractValApplication::VApp()->GetPatternPath();
    QString dir = patternPath.isEmpty() ? settings->GetPathPattern() : QFileInfo(patternPath).absolutePath();

    QString newFileName = tr("pattern") + QStringLiteral(".val");
    if (not patternPath.isEmpty())
    {
        newFileName = StrippedName(patternPath);
    }

    QString filters(tr("Pattern files") + QStringLiteral("(*.val)"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), dir + '/'_L1 + newFileName, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return false;
    }

    QFileInfo f(fileName);
    if (f.suffix().isEmpty() && f.suffix() != "val"_L1)
    {
        fileName += ".val"_L1;
    }

    if (patternPath.isEmpty())
    {
        settings->SetPathPattern(QFileInfo(fileName).absolutePath());
    }

    if (f.exists() && patternPath != fileName)
    {
        // Temporary try to lock the file before saving
        // Also help to rewite current read-only pattern
        VLockGuard<char> tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCCritical(vMainWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
            return false;
        }
    }

    return SavePatternAs(fileName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief on_actionSave_triggered save pattern file.
 * @return true for successes saving.
 */
auto MainWindow::on_actionSave_triggered() -> bool
{
    if (VAbstractValApplication::VApp()->GetPatternPath().isEmpty() || m_patternReadOnly)
    {
        return on_actionSaveAs_triggered();
    }

    if (m_curFileFormatVersion < VPatternConverter::PatternMaxVer &&
        not ContinueFormatRewrite(m_curFileFormatVersionStr, VPatternConverter::PatternMaxVerStr))
    {
        return false;
    }

    if (not CheckFilePermissions(VAbstractValApplication::VApp()->GetPatternPath(), this))
    {
        return false;
    }

    QString error;
    bool result = SavePattern(VAbstractValApplication::VApp()->GetPatternPath(), error);
    if (result)
    {
        QFile::remove(VAbstractValApplication::VApp()->GetPatternPath() + *autosavePrefix);
        m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
        m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;
    }
    else
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(tr("Could not save the file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief on_actionOpen_triggered ask user select pattern file.
 */
void MainWindow::on_actionOpen_triggered()
{
    qCDebug(vMainWindow, "Openning new file.");
    const QString filter(tr("Pattern files") + QStringLiteral(" (*.val)"));
    // Get list last open files
    const QStringList files = VAbstractValApplication::VApp()->ValentinaSettings()->GetRecentFileList();
    QString dir;
    if (files.isEmpty())
    {
        dir = QDir::homePath();
    }
    else
    {
        // Absolute path to last open file
        dir = QFileInfo(files.constFirst()).absolutePath();
    }
    qCDebug(vMainWindow, "Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter, nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());
    if (filePath.isEmpty())
    {
        return;
    }
    LoadPattern(filePath);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::on_actionOpenPuzzle_triggered()
{
    QStringList arguments;
    if (isNoScaling)
    {
        arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
    }

    VApplication::StartDetachedProcess(VApplication::PuzzleFilePath(), arguments);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::on_actionCreateManualLayout_triggered()
{
    QTemporaryFile rldFile(QDir::tempPath() + "/puzzle.rld.XXXXXX");
    if (rldFile.open())
    {
        QVector<DetailForLayout> detailsInLayout = SortDetailsForLayout(pattern->DataPieces());

        if (detailsInLayout.count() == 0)
        {
            QMessageBox::information(this, tr("Layout mode"),
                                     tr("You don't have enough details to export. Please, "
                                        "include at least one detail in layout."),
                                     QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

        QVector<VLayoutPiece> listDetails;
        try
        {
            listDetails = PrepareDetailsForLayout(detailsInLayout);
        }
        catch (VException &e)
        {
            QMessageBox::warning(this, tr("Export details"),
                                 tr("Can't export details.") + QStringLiteral(" \n") + e.ErrorMessage(),
                                 QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

        DialogLayoutScale layoutScale(false, this);
        layoutScale.SetXScale(1);
        layoutScale.SetYScale(1);
        layoutScale.exec();

        VLayoutExporter exporter;
        exporter.SetFileName(rldFile.fileName());
        exporter.SetXScale(layoutScale.GetXScale());
        exporter.SetYScale(layoutScale.GetYScale());
        exporter.ExportToRLD(listDetails);

        QStringList arguments{"-r", rldFile.fileName()};
        if (isNoScaling)
        {
            arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        rldFile.setAutoRemove(false);

        VApplication::StartDetachedProcess(VApplication::PuzzleFilePath(), arguments);
    }
    else
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Unable to prepare raw layout data.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::on_actionUpdateManualLayout_triggered()
{
    const QString filter(tr("Manual layout files") + QStringLiteral(" (*.vlt)"));

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    // Use standard path to manual layouts
    const QString path = settings->GetPathManualLayouts();

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select manual layout"), path, filter, nullptr);

    if (filePath.isEmpty())
    {
        return;
    }

    settings->SetPathManualLayouts(QFileInfo(filePath).absolutePath());

    QTemporaryFile rldFile(QDir::tempPath() + "/puzzle.rld.XXXXXX");
    rldFile.setAutoRemove(false);
    if (rldFile.open())
    {
        QVector<DetailForLayout> detailsInLayout = SortDetailsForLayout(pattern->DataPieces());

        if (detailsInLayout.count() == 0)
        {
            QMessageBox::information(this, tr("Layout mode"),
                                     tr("You don't have enough details to export. Please, "
                                        "include at least one detail in layout."),
                                     QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

        QVector<VLayoutPiece> listDetails;
        try
        {
            listDetails = PrepareDetailsForLayout(detailsInLayout);
        }
        catch (VException &e)
        {
            QMessageBox::warning(this, tr("Export details"),
                                 tr("Can't export details.") + QStringLiteral(" \n") + e.ErrorMessage(),
                                 QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

        DialogLayoutScale layoutScale(false, this);
        layoutScale.SetXScale(1);
        layoutScale.SetYScale(1);
        layoutScale.exec();

        VLayoutExporter exporter;
        exporter.SetFileName(rldFile.fileName());
        exporter.SetXScale(layoutScale.GetXScale());
        exporter.SetYScale(layoutScale.GetYScale());
        exporter.ExportToRLD(listDetails);

        QStringList arguments{filePath, "-r", rldFile.fileName()};
        if (isNoScaling)
        {
            arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        rldFile.setAutoRemove(false);

        VApplication::StartDetachedProcess(VApplication::PuzzleFilePath(), arguments);
    }
    else
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Unable to prepare raw layout data.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionAddBackgroundImage()
{
    const QString fileName =
        QFileDialog::getOpenFileName(this, tr("Select background image"), QDir::homePath(), PrepareImageFilters(),
                                     nullptr, VAbstractApplication::VApp()->NativeFileDialog());
    if (not fileName.isEmpty())
    {
        QRect viewportRect(0, 0, ui->view->viewport()->width(), ui->view->viewport()->height());
        PlaceBackgroundImage(ui->view->mapToScene(viewportRect.center()), fileName);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionExportFontCorrections()
{
    // Use standard path to manual layouts
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    const QString dirPath = settings->GetPathFontCorrections();

    bool usedNotExistedDir = false;
    QDir directory(dirPath);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    auto RemoveUnsuded = qScopeGuard(
        [usedNotExistedDir, dirPath]()
        {
            if (usedNotExistedDir)
            {
                QDir directory(dirPath);
                directory.rmpath(QChar('.'));
            }
        });

    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select folder"), dirPath,
        VAbstractApplication::VApp()->NativeFileDialog(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    if (not dir.isEmpty())
    {
        VSingleLineOutlineChar corrector(settings->GetLabelFont());
        corrector.ExportCorrections(dir);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Clear reset to default window.
 */
void MainWindow::Clear()
{
    qCDebug(vMainWindow, "Reseting main window.");
    m_lock.reset();
    qCDebug(vMainWindow, "Unlocked pattern file.");
    ActionDraw(true);
    qCDebug(vMainWindow, "Returned to Draw mode.");
    setCurrentFile(QString()); // Keep before cleaning a pattern data to prevent a crash
    pattern->Clear();
    qCDebug(vMainWindow, "Clearing pattern.");
    if (not VAbstractValApplication::VApp()->GetPatternPath().isEmpty() && not doc->MPath().isEmpty())
    {
        m_watcher->removePath(AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath()));
    }
    doc->Clear();
    UpdateWindowTitle();
    UpdateVisibilityGroups();
    m_detailsWidget->UpdateList();
    m_backgroundImagesWidget->UpdateImages();
    qCDebug(vMainWindow, "Clearing scenes.");
    m_sceneDraw->clear();
    m_sceneDraw->SetAcceptDrop(false);
    m_sceneDetails->clear();
    ArrowTool(true);
    m_comboBoxDraws->clear();
    ui->actionDraw->setEnabled(false);
    ui->actionDetails->setEnabled(false);
    ui->actionLayout->setEnabled(false);
    ui->actionNewDraw->setEnabled(false);
    ui->actionOptionDraw->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionSaveAs->setEnabled(false);
    ui->actionPattern_properties->setEnabled(false);
    ui->actionZoomIn->setEnabled(false);
    ui->actionZoomOut->setEnabled(false);
    ui->actionZoomFitBest->setEnabled(false);
    ui->actionZoomFitBestCurrent->setEnabled(false);
    ui->actionZoomOriginal->setEnabled(false);
    ui->actionHistory->setEnabled(false);
    ui->actionExportRecipe->setEnabled(false);
    ui->actionTable->setEnabled(false);
    ui->actionExportIncrementsToCSV->setEnabled(false);
    ui->actionExportFinalMeasurementsToCSV->setEnabled(false);
    ui->actionFinalMeasurements->setEnabled(false);
    ui->actionLast_tool->setEnabled(false);
    ui->actionShowCurveDetails->setEnabled(false);
    ui->actionShowAccuracyRadius->setEnabled(false);
    ui->actionShowMainPath->setEnabled(false);
    ui->actionLoadIndividual->setEnabled(false);
    ui->actionLoadMultisize->setEnabled(false);
    ui->actionUnloadMeasurements->setEnabled(false);
    ui->actionEditCurrent->setEnabled(false);
    ui->actionPreviousPatternPiece->setEnabled(false);
    ui->actionNextPatternPiece->setEnabled(false);
    ui->actionAddBackgroundImage->setEnabled(false);
    SetEnableTool(false);
    VAbstractValApplication::VApp()->SetPatternUnits(Unit::Cm);
    VAbstractValApplication::VApp()->SetMeasurementsType(MeasurementsType::Unknown);
    ui->toolBarOption->clear();
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    CleanLayout();
    listDetails.clear(); // don't move to CleanLayout()
    VAbstractApplication::VApp()->getUndoStack()->clear();
    m_toolOptions->ClearPropertyBrowser();
    m_toolOptions->itemClicked(nullptr);
    m_progressBar->setVisible(false);
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    m_taskbarProgress->setVisible(false);
#endif
    m_statusLabel->setVisible(true);
    ui->actionIncreaseLabelFont->setEnabled(false);
    ui->actionDecreaseLabelFont->setEnabled(false);
    ui->actionOriginalLabelFont->setEnabled(false);
    ui->actionHideLabels->setEnabled(false);
    ui->plainTextEditPatternMessages->clear();
    ui->actionLoadWatermark->setEnabled(false);
    ui->actionRemoveWatermark->setEnabled(false);
    ui->actionEditCurrentWatermark->setEnabled(false);
    PatternChangesWereSaved(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::FileClosedCorrect()
{
    WriteSettings();

    // File was closed correct.
    QStringList restoreFiles = VAbstractValApplication::VApp()->ValentinaSettings()->GetRestoreFileList();
    restoreFiles.removeAll(VAbstractValApplication::VApp()->GetPatternPath());
    VAbstractValApplication::VApp()->ValentinaSettings()->SetRestoreFileList(restoreFiles);

    // Remove autosave file
    QFile autofile(VAbstractValApplication::VApp()->GetPatternPath() + *autosavePrefix);
    if (autofile.exists())
    {
        autofile.remove();
    }
    qCDebug(vMainWindow, "File %s closed correct.", qUtf8Printable(VAbstractValApplication::VApp()->GetPatternPath()));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::FullParseFile()
{
    qCDebug(vMainWindow, "Full parsing file");

    m_toolOptions->ClearPropertyBrowser();

    if (not FullParsePattern())
    {
        return;
    }

    QString patternPiece;
    if (m_comboBoxDraws->currentIndex() != -1)
    {
        patternPiece = m_comboBoxDraws->itemText(m_comboBoxDraws->currentIndex());
    }
    m_comboBoxDraws->blockSignals(true);
    m_comboBoxDraws->clear();

    QStringList patternPieceNames = doc->getPatternPieces();
    patternPieceNames.sort();
    m_comboBoxDraws->addItems(patternPieceNames);

    if (not m_drawMode)
    {
        m_comboBoxDraws->setCurrentIndex(m_comboBoxDraws->count() - 1);
    }
    else
    {
        const qint32 index = m_comboBoxDraws->findText(patternPiece);
        if (index != -1)
        {
            m_comboBoxDraws->setCurrentIndex(index);
        }
    }
    m_comboBoxDraws->blockSignals(false);
    ui->actionPattern_properties->setEnabled(true);

    GlobalChangePP(patternPiece);

    SetEnableTool(m_comboBoxDraws->count() > 0);
    m_detailsWidget->UpdateList();

    VMainGraphicsView::NewSceneRect(m_sceneDraw, VAbstractValApplication::VApp()->getSceneView());
    VMainGraphicsView::NewSceneRect(m_sceneDetails, VAbstractValApplication::VApp()->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::GlobalChangePP(const QString &patternPiece)
{
    const qint32 index = m_comboBoxDraws->findText(patternPiece);
    try
    {
        if (index != -1)
        { // -1 for not found
            ChangePP(index, false);
            m_comboBoxDraws->blockSignals(true);
            m_comboBoxDraws->setCurrentIndex(index);
            m_comboBoxDraws->blockSignals(false);
        }
        else
        {
            ChangePP(0, false);
        }
    }
    catch (VExceptionBadId &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Bad id.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::PreviousPatternPiece()
{
    int index = m_comboBoxDraws->currentIndex();

    if (index == -1 || m_comboBoxDraws->count() <= 1)
    {
        return;
    }

    if (index == 0)
    {
        index = m_comboBoxDraws->count() - 1;
    }
    else
    {
        --index;
    }

    m_comboBoxDraws->setCurrentIndex(index);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::NextPatternPiece()
{
    int index = m_comboBoxDraws->currentIndex();

    if (index == -1 || m_comboBoxDraws->count() <= 1)
    {
        return;
    }

    if (index == m_comboBoxDraws->count() - 1)
    {
        index = 0;
    }
    else
    {
        ++index;
    }

    m_comboBoxDraws->setCurrentIndex(index);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetEnabledGUI(bool enabled)
{
    if (m_guiEnabled != enabled)
    {
        if (not enabled)
        {
            ArrowTool(true);
            VAbstractApplication::VApp()->getUndoStack()->clear();
        }
        SetEnableWidgets(enabled);

        m_guiEnabled = enabled;

        SetEnableTool(enabled);
        ui->toolBarOption->setEnabled(enabled);
#ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
#endif
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetEnableWidgets enable action button.
 * @param enable enable value.
 */
void MainWindow::SetEnableWidgets(bool enable)
{
    const bool drawStage = (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation);
    const bool detailsStage = (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Modeling);
    const bool layoutStage = (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Layout);
    const bool designStage = (drawStage || detailsStage);
    const bool piecesStage = (detailsStage || layoutStage);

    const bool enableOnDrawStage = (enable && drawStage);
    const bool enableOnDesignStage = (enable && designStage);
    const bool enableOnDetailsStage = (enable && detailsStage);
    const bool enableOnPiecesStage = (enable && piecesStage);

    m_comboBoxDraws->setEnabled(enableOnDrawStage);
    ui->actionOptionDraw->setEnabled(enableOnDrawStage);
    ui->actionSave->setEnabled(isWindowModified() && enable && not m_patternReadOnly);
    ui->actionSaveAs->setEnabled(enable);
    ui->actionPattern_properties->setEnabled(enableOnDesignStage);
    ui->actionZoomIn->setEnabled(enable);
    ui->actionZoomOut->setEnabled(enable);
    ui->actionArrowTool->setEnabled(enableOnDesignStage);
    ui->actionHistory->setEnabled(enableOnDrawStage);
    ui->actionExportRecipe->setEnabled(enableOnDrawStage);
    ui->actionNewDraw->setEnabled(enableOnDrawStage);
    ui->actionDraw->setEnabled(enable);
    ui->actionDetails->setEnabled(enable);
    ui->actionLayout->setEnabled(enable);
    ui->actionTable->setEnabled(enableOnDesignStage);
    ui->actionExportIncrementsToCSV->setEnabled(enable);
    ui->actionExportFinalMeasurementsToCSV->setEnabled(enable);
    ui->actionFinalMeasurements->setEnabled(enable);
    ui->actionZoomFitBest->setEnabled(enable);
    ui->actionZoomFitBestCurrent->setEnabled(enableOnDrawStage);
    ui->actionZoomOriginal->setEnabled(enable);
    ui->actionShowCurveDetails->setEnabled(enableOnDrawStage);
    ui->actionShowAccuracyRadius->setEnabled(enableOnDesignStage);
    ui->actionShowMainPath->setEnabled(enableOnDetailsStage);
    ui->actionLoadIndividual->setEnabled(enableOnDesignStage);
    ui->actionLoadMultisize->setEnabled(enableOnDesignStage);
    ui->actionUnloadMeasurements->setEnabled(enableOnDesignStage);
    ui->actionPreviousPatternPiece->setEnabled(enableOnDrawStage);
    ui->actionNextPatternPiece->setEnabled(enableOnDrawStage);
    ui->actionAddBackgroundImage->setEnabled(enableOnDrawStage);
    ui->actionIncreaseLabelFont->setEnabled(enable);
    ui->actionDecreaseLabelFont->setEnabled(enable);
    ui->actionOriginalLabelFont->setEnabled(enable);
    ui->actionHideLabels->setEnabled(enable);
    ui->actionCreateManualLayout->setEnabled(enableOnPiecesStage);
    ui->actionUpdateManualLayout->setEnabled(enableOnPiecesStage);

    ui->actionLoadWatermark->setEnabled(enable);
    ui->actionRemoveWatermark->setEnabled(enable && not doc->GetWatermarkPath().isEmpty());
    ui->actionEditCurrentWatermark->setEnabled(enable && not doc->GetWatermarkPath().isEmpty());

    actionDockWidgetToolOptions->setEnabled(enableOnDesignStage);
    actionDockWidgetGroups->setEnabled(enableOnDesignStage);
    actionDockWidgetBackgroundImages->setEnabled(enableOnDrawStage);

    undoAction->setEnabled(enableOnDesignStage && VAbstractApplication::VApp()->getUndoStack()->canUndo());
    redoAction->setEnabled(enableOnDesignStage && VAbstractApplication::VApp()->getUndoStack()->canRedo());

    // Now we don't want allow user call context menu
    m_sceneDraw->SetDisableTools(!enable, doc->GetNameActivPP());
    ui->view->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief on_actionNew_triggered create new empty pattern.
 */
void MainWindow::on_actionNew_triggered()
{
    if (m_comboBoxDraws->count() == 0)
    {
        qCDebug(vMainWindow, "New PP.");
        QString patternPieceName = tr("Pattern piece %1").arg(m_comboBoxDraws->count() + 1);
        qCDebug(vMainWindow, "Generated PP name: %s", qUtf8Printable(patternPieceName));

        qCDebug(vMainWindow, "First PP");
        DialogNewPattern newPattern(pattern, patternPieceName, this);
        if (newPattern.exec() == QDialog::Accepted)
        {
            patternPieceName = newPattern.name();
            VAbstractValApplication::VApp()->SetPatternUnits(newPattern.PatternUnit());
            qCDebug(vMainWindow, "PP name: %s", qUtf8Printable(patternPieceName));
        }
        else
        {
            qCDebug(vMainWindow, "Creation a new pattern was canceled.");
            return;
        }

        // Set scene size to size scene view
        VMainGraphicsView::NewSceneRect(m_sceneDraw, ui->view);
        VMainGraphicsView::NewSceneRect(m_sceneDetails, ui->view);

        AddPP(patternPieceName);

        m_mouseCoordinate = new QLabel(
            QStringLiteral("0, 0 (%1)").arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true)));
        ui->toolBarOption->addWidget(m_mouseCoordinate);

        m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
        m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;

        ToolBarOption();

        m_sceneDraw->SetAcceptDrop(true);
    }
    else
    {
        OpenNewValentina();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief haveChange enable action save if we have unsaved change.
 */
void MainWindow::PatternChangesWereSaved(bool saved)
{
    if (m_guiEnabled)
    {
        const bool state = doc->IsModified() || !saved;
        setWindowModified(state);
        not m_patternReadOnly ? ui->actionSave->setEnabled(state) : ui->actionSave->setEnabled(false);
        m_layoutSettings->SetLayoutStale(true);
        isNeedAutosave = not saved;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::DimensionABaseChanged()
{
    const qreal oldValue = m_currentDimensionA;
    m_currentDimensionA = m_dimensionA->currentData().toDouble();
    doc->SetDimensionAValue(m_currentDimensionA);

    if (not VFuzzyComparePossibleNulls(oldValue, m_currentDimensionA))
    {
        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();
        if (dimensions.size() > 1)
        {
            MeasurementDimension_p dimension = dimensions.at(1);
            InitDimensionGradation(1, dimension, m_dimensionB);

            if (dimensions.size() > 2)
            {
                dimension = dimensions.at(2);
                InitDimensionGradation(2, dimension, m_dimensionC);
            }
        }

        m_gradation->start();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::DimensionBBaseChanged()
{
    const qreal oldValue = m_currentDimensionB;
    m_currentDimensionB = m_dimensionB->currentData().toDouble();
    doc->SetDimensionBValue(m_currentDimensionB);

    if (not VFuzzyComparePossibleNulls(oldValue, m_currentDimensionB))
    {
        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

        if (dimensions.size() > 2)
        {
            const MeasurementDimension_p &dimension = dimensions.at(2);
            InitDimensionGradation(2, dimension, m_dimensionC);
        }

        m_gradation->start();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::DimensionCBaseChanged()
{
    const qreal oldValue = m_currentDimensionC;
    m_currentDimensionC = m_dimensionC->currentData().toDouble();
    doc->SetDimensionCValue(m_currentDimensionC);

    if (not VFuzzyComparePossibleNulls(oldValue, m_currentDimensionC))
    {
        m_gradation->start();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::GradationChanged()
{
    m_gradation->stop();

    if (m_m->isNull())
    {
        m_m = OpenMeasurementFile(AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath()));
    }

    if (UpdateMeasurements(m_m, m_currentDimensionA, m_currentDimensionB, m_currentDimensionC))
    {
        doc->LiteParseTree(Document::FullLiteParse);
        StoreDimensions();
        emit m_sceneDetails->DimensionsChanged();
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't update measurements.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowProgress()
{
    if (m_progressBar->isVisible() && m_progressBar->value() + 1 <= m_progressBar->maximum())
    {
        const int newValue = m_progressBar->value() + 1;
        m_progressBar->setValue(newValue);
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        m_taskbarProgress->setValue(newValue);
#endif
        QCoreApplication::processEvents();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClearPatternMessages()
{
    ui->plainTextEditPatternMessages->clear();
    if (not m_unreadPatternMessage.isNull())
    {
        m_unreadPatternMessage->setText(QString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::AskDefaultSettings()
{
    if (VApplication::IsGUIMode())
    {
        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
        if (not settings->IsLocaleSelected())
        {
            QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            DialogSelectLanguage dialog(this);
            QGuiApplication::restoreOverrideCursor();
            if (dialog.exec() == QDialog::Accepted)
            {
                QString locale = dialog.Locale();
                settings->SetLocale(locale);
                VAbstractApplication::VApp()->LoadTranslation(locale);
            }
        }

        if (settings->IsAskCollectStatistic())
        {
            DialogAskCollectStatistic dialog(this);
            if (dialog.exec() == QDialog::Accepted)
            {
                settings->SetCollectStatistic(dialog.CollectStatistic());
            }

            settings->SetAskCollectStatistic(false);
        }

        if (settings->IsCollectStatistic())
        {
            auto *statistic = VGAnalytics::Instance();
            statistic->SetGUILanguage(settings->GetLocale());

            QString clientID = settings->GetClientID();
            bool freshID = false;
            if (clientID.isEmpty())
            {
                clientID = QUuid::createUuid().toString();
                settings->SetClientID(clientID);
                statistic->SetClientID(clientID);
                freshID = true;
            }

            statistic->Enable(true);

            const qint64 uptime = VAbstractValApplication::VApp()->AppUptime();
            freshID ? statistic->SendAppFreshInstallEvent(uptime) : statistic->SendAppStartEvent(uptime);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::AddBackgroundImageItem(const QUuid &id)
{
    NewBackgroundImageItem(doc->GetBackgroundImage(id));

    if (m_backgroundImagesWidget != nullptr)
    {
        m_backgroundImagesWidget->UpdateImages();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::DeleteBackgroundImageItem(const QUuid &id)
{
    if (m_backgroundImages.contains(id))
    {
        VBackgroundImageItem *item = m_backgroundImages.value(id);
        emit ui->view->itemClicked(nullptr); // Hide visualization to avoid a crash
        item->setVisible(false);             // Do not remove the item from scene to prevent crashes.
        if (m_backgroudcontrols != nullptr && m_backgroudcontrols->Id() == id)
        {
            m_backgroudcontrols->ActivateControls(QUuid());
        }

        if (m_backgroundImagesWidget != nullptr)
        {
            m_backgroundImagesWidget->UpdateImages();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowBackgroundImageInExplorer(const QUuid &id)
{
    ShowInGraphicalShell(doc->GetBackgroundImage(id).FilePath());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SaveBackgroundImage(const QUuid &id)
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(id);

    if (not image.IsValid())
    {
        qCritical() << tr("Unable to save image. Error: %1").arg(image.ErrorString());
        return;
    }

    if (image.ContentData().isEmpty())
    {
        qCritical() << tr("Unable to save image. No data.");
        return;
    }

    const QByteArray imageData = QByteArray::fromBase64(image.ContentData());
    QMimeType mime = MimeTypeFromByteArray(imageData);
    QString path = QDir::homePath() + QDir::separator() + tr("untitled");
    QStringList filters;

    if (mime.isValid())
    {
        QStringList suffixes = mime.suffixes();
        if (not suffixes.isEmpty())
        {
            path += '.' + suffixes.at(0);
        }

        filters.append(mime.filterString());
    }

    filters.append(tr("All files") + QStringLiteral(" (*.*)"));

    QString filter = filters.join(QStringLiteral(";;"));

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), path, filter, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());
    if (not filename.isEmpty())
    {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(imageData);
        }
        else
        {
            qCritical() << tr("Unable to save image. Error: %1").arg(file.errorString());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ParseBackgroundImages()
{
    // No memory leak. Scene should take care of these items
    m_backgroudcontrols = nullptr; // force creating new controls
    m_backgroundImages.clear();    // clear dangling pointers

    QVector<VBackgroundPatternImage> allImages = doc->GetBackgroundImages();
    for (const auto &image : allImages)
    {
        NewBackgroundImageItem(image);
    }
    m_backgroundImagesWidget->UpdateImages();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionHistory_triggered(bool checked)
{
    if (checked)
    {
        m_dialogHistory = new DialogHistory(pattern, doc, this);
        m_dialogHistory->setWindowFlags(Qt::Window);
        connect(this, &MainWindow::RefreshHistory, m_dialogHistory.data(), &DialogHistory::UpdateHistory);
        connect(m_dialogHistory.data(), &DialogHistory::DialogClosed, this,
                [this]()
                {
                    ui->actionHistory->setChecked(false);
                    delete m_dialogHistory;
                });
        // Fix issue #526. Dialog Detail is not on top after selection second object on Mac.
        m_dialogHistory->setWindowFlags(m_dialogHistory->windowFlags() | Qt::WindowStaysOnTopHint);
        m_dialogHistory->show();
    }
    else
    {
        ui->actionHistory->setChecked(true);
        m_dialogHistory->activateWindow();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionExportRecipe_triggered()
{
    QString filters(tr("Recipe files") + QStringLiteral("(*.vpr)"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export recipe"),
                                                    QDir::homePath() + '/' + tr("recipe") + QStringLiteral(".vpr"),
                                                    filters, nullptr, VAbstractApplication::VApp()->NativeFileDialog());
    if (fileName.isEmpty())
    {
        return;
    }

    try
    {
        VPatternRecipe recipe(doc);
        QString error;
        if (not recipe.SaveDocument(fileName, error))
        {
            qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Could not save recipe. %1").arg(error)));
        }
    }
    catch (const VExceptionInvalidHistory &e)
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Could not create recipe file. %1").arg(e.ErrorMessage())));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionNewDraw_triggered()
{
    qCDebug(vMainWindow, "New PP.");
    QString patternPieceName = tr("Pattern piece %1").arg(m_comboBoxDraws->count() + 1);
    qCDebug(vMainWindow, "Generated PP name: %s", qUtf8Printable(patternPieceName));

    qCDebug(vMainWindow, "PP count %d", m_comboBoxDraws->count());
    bool ok = PatternPieceName(patternPieceName);
    qCDebug(vMainWindow, "PP name: %s", qUtf8Printable(patternPieceName));
    if (not ok)
    {
        return;
    }

    AddPP(patternPieceName);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionTable_triggered()
{
    // Because of bug on Mac with Qt 5.11 closing this dialog causes a crash. Instead of closing we will keep
    // dialog in memory.
    if (m_dialogTable.isNull())
    {
        m_dialogTable = new DialogIncrements(pattern, doc, this);
        connect(m_dialogTable.data(), &DialogIncrements::UpdateProperties, m_toolOptions,
                &VToolOptionsPropertyBrowser::RefreshOptions);
        m_dialogTable->show();
    }
    else
    {
        m_dialogTable->FullUpdateFromFile();
        m_dialogTable->RestoreAfterClose(); // Redo some moves after close
        m_dialogTable->isVisible() ? m_dialogTable->activateWindow() : m_dialogTable->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionFinalMeasurements_triggered()
{
    if (m_dialogFMeasurements.isNull())
    {
        m_dialogFMeasurements = new DialogFinalMeasurements(doc, this);
        m_dialogFMeasurements->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_dialogFMeasurements.data(), &DialogFinalMeasurements::finished, this,
                [this](int result)
                {
                    if (result == QDialog::Accepted)
                    {
                        doc->SetFinalMeasurements(m_dialogFMeasurements->FinalMeasurements());
                        emit doc->UpdatePatternLabel();
                    }
                    m_dialogFMeasurements->close();
                });
        m_dialogFMeasurements->show();
    }
    else
    {
        m_dialogFMeasurements->activateWindow();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionShowMainPath_triggered(bool checked)
{
    VAbstractValApplication::VApp()->ValentinaSettings()->SetPieceShowMainPath(checked);
    const QList<quint32> ids = pattern->DataPieces()->keys();
    const bool updateChildren = false;
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    for (const auto &id : ids)
    {
        try
        {
            if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(id)))
            {
                tool->RefreshGeometry(updateChildren);
            }
        }
        catch (VExceptionBadId &)
        {
        }
    }
    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ActionOpenTape_triggered()
{
    QStringList arguments;
    if (isNoScaling)
    {
        arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
    }

    VApplication::StartDetachedProcess(VApplication::TapeFilePath(), arguments);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDimensionControls()
{
    if (not m_dimensionA.isNull())
    {
        delete m_dimensionA;
    }
    if (not m_dimensionALabel.isNull())
    {
        delete m_dimensionALabel;
    }

    if (not m_dimensionB.isNull())
    {
        delete m_dimensionB;
    }
    if (not m_dimensionBLabel.isNull())
    {
        delete m_dimensionBLabel;
    }

    if (not m_dimensionC.isNull())
    {
        delete m_dimensionC;
    }
    if (not m_dimensionCLabel.isNull())
    {
        delete m_dimensionCLabel;
    }

    if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
    {
        const QList<MeasurementDimension_p> dimensions = m_m->Dimensions().values();

        auto InitControl = [this, dimensions](int index, QPointer<QLabel> &name, QPointer<QComboBox> &control)
        {
            if (dimensions.size() > index)
            {
                const MeasurementDimension_p &dimension = dimensions.at(index);

                if (name.isNull())
                {
                    name = new QLabel(dimension->Name() + ':'_L1);
                }
                else
                {
                    name->setText(dimension->Name() + ':'_L1);
                }
                name->setToolTip(VAbstartMeasurementDimension::DimensionToolTip(dimension, m_m->IsFullCircumference()));

                if (control.isNull())
                {
                    control = new QComboBox;
                    control->setSizeAdjustPolicy(QComboBox::AdjustToContents);
                }

                InitDimensionGradation(index, dimension, control);

                ui->toolBarOption->addWidget(name);
                ui->toolBarOption->addWidget(control);
            }
        };

        InitControl(0, m_dimensionALabel, m_dimensionA);
        InitControl(1, m_dimensionBLabel, m_dimensionB);
        InitControl(2, m_dimensionCLabel, m_dimensionC);

        if (not m_dimensionA.isNull())
        {
            connect(m_dimensionA.data(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    &MainWindow::DimensionABaseChanged);
        }

        if (not m_dimensionB.isNull())
        {
            connect(m_dimensionB.data(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    &MainWindow::DimensionBBaseChanged);
        }

        if (not m_dimensionC.isNull())
        {
            connect(m_dimensionC.data(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    &MainWindow::DimensionCBaseChanged);
        }

        ui->toolBarOption->addSeparator();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDimensionGradation(int index, const MeasurementDimension_p &dimension,
                                        const QPointer<QComboBox> &control)
{
    SCASSERT(control != nullptr)

    qreal current = -1;
    if (control->currentIndex() != -1)
    {
        current = control->currentData().toDouble();
    }

    control->blockSignals(true);
    control->clear();

    const QVector<qreal> bases = DimensionRestrictedValues(index, dimension);
    const DimesionLabels labels = dimension->Labels();

    if (dimension->Type() == MeasurementDimension::X)
    {
        InitDimensionXGradation(bases, labels, control);
    }
    else if (dimension->Type() == MeasurementDimension::Y || dimension->Type() == MeasurementDimension::W ||
             dimension->Type() == MeasurementDimension::Z)
    {
        InitDimensionYWZGradation(bases, labels, control, dimension->IsBodyMeasurement());
    }

    // Calculate the width of the largest item using QFontMetrics
    QFontMetrics fontMetrics(control->font());
    int maxWidth = 0;
    for (int i = 0; i < control->count(); ++i)
    {
        int itemWidth = TextWidth(fontMetrics, control->itemText(i));
        if (itemWidth > maxWidth)
        {
            maxWidth = itemWidth;
        }
    }

    // Set the minimum width of the view to the largest item width
    control->view()->setMinimumWidth(maxWidth);

    // after initialization the current index is 0. The signal for changing the index will not be triggered if not make
    // it invalid first
    control->setCurrentIndex(-1);

    int i = control->findData(current);
    if (i != -1)
    {
        control->setCurrentIndex(i);
        control->blockSignals(false);
    }
    else
    {
        control->blockSignals(false);
        control->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDimensionXGradation(const QVector<qreal> &bases, const DimesionLabels &labels,
                                         const QPointer<QComboBox> &control)
{
    const QString unit = UnitsToStr(VAbstractValApplication::VApp()->MeasurementsUnits(), true);

    for (auto base : bases)
    {
        if (VFuzzyContains(labels, base) && not VFuzzyValue(labels, base).isEmpty())
        {
            control->addItem(VFuzzyValue(labels, base), base);
        }
        else
        {
            control->addItem(QStringLiteral("%1 %2").arg(base).arg(unit), base);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDimensionYWZGradation(const QVector<qreal> &bases, const DimesionLabels &labels,
                                           const QPointer<QComboBox> &control, bool bodyMeasurement)
{
    const bool fc = m_m->IsFullCircumference();
    const QString unit = UnitsToStr(VAbstractValApplication::VApp()->MeasurementsUnits(), true);

    for (auto base : bases)
    {
        if (VFuzzyContains(labels, base) && not VFuzzyValue(labels, base).isEmpty())
        {
            control->addItem(VFuzzyValue(labels, base), base);
        }
        else
        {
            if (bodyMeasurement)
            {
                control->addItem(QStringLiteral("%1 %2").arg(fc ? base * 2 : base).arg(unit), base);
            }
            else
            {
                control->addItem(QString::number(base), base);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetEnableTool enable button.
 * @param enable enable value.
 */
void MainWindow::SetEnableTool(bool enable)
{
    bool drawTools = false;
    bool modelingTools = false;
    bool layoutTools = false;

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    switch (VAbstractValApplication::VApp()->GetDrawMode())
    {
        case Draw::Calculation:
            drawTools = enable;
            break;
        case Draw::Modeling:
            modelingTools = enable;
            break;
        case Draw::Layout:
            layoutTools = enable;
            break;
    }
    QT_WARNING_POP

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were handled.");

    // Drawing Tools
    ui->actionEndLineTool->setEnabled(drawTools);
    ui->actionLineTool->setEnabled(drawTools);
    ui->actionAlongLineTool->setEnabled(drawTools);
    ui->actionShoulderPointTool->setEnabled(drawTools);
    ui->actionNormalTool->setEnabled(drawTools);
    ui->actionBisectorTool->setEnabled(drawTools);
    ui->actionLineIntersectTool->setEnabled(drawTools);
    ui->actionSplineTool->setEnabled(drawTools);
    ui->actionCubicBezierTool->setEnabled(drawTools);
    ui->actionArcTool->setEnabled(drawTools);
    ui->actionSplinePathTool->setEnabled(drawTools);
    ui->actionCubicBezierPathTool->setEnabled(drawTools);
    ui->actionPointOfContactTool->setEnabled(drawTools);
    ui->actionNewDetailTool->setEnabled(drawTools);
    ui->actionInternalPathTool->setEnabled(drawTools);
    ui->actionHeightTool->setEnabled(drawTools);
    ui->actionTriangleTool->setEnabled(drawTools);
    ui->actionPointOfIntersectionTool->setEnabled(drawTools);
    ui->actionSplineCutPointTool->setEnabled(drawTools);
    ui->actionSplinePathCutPointTool->setEnabled(drawTools);
    ui->actionArcCutPointTool->setEnabled(drawTools);
    ui->actionLineIntersectAxisTool->setEnabled(drawTools);
    ui->actionCurveIntersectAxisTool->setEnabled(drawTools);
    ui->actionArcIntersectAxisTool->setEnabled(drawTools);
    ui->actionPointOfIntersectionArcsTool->setEnabled(drawTools);
    ui->actionIntersectionCurvesTool->setEnabled(drawTools);
    ui->actionPointOfIntersectionCirclesTool->setEnabled(drawTools);
    ui->actionPointFromCircleAndTangentTool->setEnabled(drawTools);
    ui->actionPointFromArcAndTangentTool->setEnabled(drawTools);
    ui->actionArcWithLengthTool->setEnabled(drawTools);
    ui->actionTrueDartsTool->setEnabled(drawTools);
    ui->actionGroupTool->setEnabled(drawTools);
    ui->actionRotationTool->setEnabled(drawTools);
    ui->actionFlippingByLineTool->setEnabled(drawTools);
    ui->actionFlippingByAxisTool->setEnabled(drawTools);
    ui->actionMoveTool->setEnabled(drawTools);
    ui->actionMidpointTool->setEnabled(drawTools);
    ui->actionEllipticalArcTool->setEnabled(drawTools);
    ui->actionPinTool->setEnabled(drawTools);
    ui->actionInsertNodeTool->setEnabled(drawTools);
    ui->actionPlaceLabelTool->setEnabled(drawTools);
    ui->actionExportDraw->setEnabled(drawTools);

    ui->actionLast_tool->setEnabled(drawTools);

    ui->actionToolSelect->setEnabled(drawTools || modelingTools);
    ui->actionToolSelect->setChecked(drawTools || modelingTools);

    // Modeling Tools
    ui->actionUnionDetailsTool->setEnabled(modelingTools);
    ui->actionDetailExportAs->setEnabled(modelingTools);
    ui->actionDuplicateDetailTool->setEnabled(modelingTools);

    // Layout tools
    ui->actionLayoutSettings->setEnabled(layoutTools);
    ui->actionLayoutExportAs->setEnabled(layoutTools && not m_layoutSettings->LayoutScenes().isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetLayoutModeActions()
{
    const bool enabled = not m_layoutSettings->LayoutScenes().isEmpty();

    ui->actionLayoutExportAs->setEnabled(enabled);
    ui->actionExportAs->setEnabled(enabled);
    ui->actionPrintPreview->setEnabled(enabled);
    ui->actionPrintPreviewTiled->setEnabled(enabled);
    ui->actionPrint->setEnabled(enabled);
    ui->actionPrintTiled->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MinimumScrollBar set scroll bar to minimum.
 */
void MainWindow::MinimumScrollBar()
{
    QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
    horScrollBar->setValue(horScrollBar->minimum());
    QScrollBar *verScrollBar = ui->view->verticalScrollBar();
    verScrollBar->setValue(verScrollBar->minimum());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SavePattern save pattern file.
 * @param fileName pattern file name.
 * @return true if all is good.
 */
auto MainWindow::SavePattern(const QString &fileName, QString &error) -> bool
{
    qCDebug(vMainWindow, "Saving pattern file %s.", qUtf8Printable(fileName));
    QFileInfo tempInfo(fileName);

    const QString mPath = AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath());
    if (not mPath.isEmpty() && VAbstractValApplication::VApp()->GetPatternPath() != fileName)
    {
        doc->SetMPath(RelativeMPath(fileName, mPath));
    }

    const bool result = doc->SaveDocument(fileName, error);
    if (result)
    {
        if (tempInfo.suffix() != "autosave"_L1)
        {
            setCurrentFile(fileName);
            statusBar()->showMessage(tr("File saved"), 5000);
            qCDebug(vMainWindow, "File %s saved.", qUtf8Printable(fileName));
            PatternChangesWereSaved(result);
        }
    }
    else
    {
        doc->SetMPath(mPath);
        emit doc->UpdatePatternLabel();
        qCDebug(vMainWindow, "Could not save file %s. %s.", qUtf8Printable(fileName), qUtf8Printable(error));
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AutoSavePattern start safe saving.
 */
void MainWindow::AutoSavePattern()
{
    if (VApplication::IsGUIMode() && not VAbstractValApplication::VApp()->GetPatternPath().isEmpty() &&
        isWindowModified() && isNeedAutosave)
    {
        qCDebug(vMainWindow, "Autosaving pattern.");
        QString error;
        if (SavePattern(VAbstractValApplication::VApp()->GetPatternPath() + *autosavePrefix, error))
        {
            isNeedAutosave = false;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentFile the function is called to reset the state of a few variables when a file
 * is loaded or saved, or when the user starts editing a new file (in which case fileName is empty).
 * @param fileName file name.
 */
void MainWindow::setCurrentFile(const QString &fileName)
{
    qCDebug(vMainWindow, "Set current name to \"%s\"", qUtf8Printable(fileName));
    VAbstractValApplication::VApp()->SetPatternPath(fileName);
    doc->SetPatternWasChanged(true);
    emit doc->UpdatePatternLabel();
    VAbstractApplication::VApp()->getUndoStack()->setClean();

    if (not VAbstractValApplication::VApp()->GetPatternPath().isEmpty() && VApplication::IsGUIMode())
    {
        qCDebug(vMainWindow, "Updating recent file list.");
        VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
        QStringList files = settings->GetRecentFileList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
        {
            files.removeLast();
        }

        settings->SetRecentFileList(files);
        UpdateRecentFileActions();

        qCDebug(vMainWindow, "Updating restore file list.");
        QStringList restoreFiles = settings->GetRestoreFileList();
        restoreFiles.removeAll(fileName);
        restoreFiles.prepend(fileName);
        settings->SetRestoreFileList(restoreFiles);
    }

    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ReadSettings read setting for app.
 */
void MainWindow::ReadSettings()
{
    qCDebug(vMainWindow, "Reading settings.");
    const VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetGeometry());
        restoreState(settings->GetToolbarsState(), static_cast<int>(AppVersion()));

        m_groupsActive = settings->IsDockWidgetGroupsActive();
        m_toolOptionsActive = settings->IsDockWidgetToolOptionsActive();
        m_patternMessagesActive = settings->IsDockWidgetPatternMessagesActive();
        m_backgroundImagesActive = settings->IsDockWidgetBackgroundImagesActive();

        ui->dockWidgetGroups->setVisible(m_groupsActive);
        ui->dockWidgetToolOptions->setVisible(m_toolOptionsActive);
        ui->dockWidgetMessages->setVisible(m_patternMessagesActive);
        ui->dockWidgetBackgroundImages->setVisible(m_backgroundImagesActive);

        // Scene antialiasing
        ui->view->SetAntialiasing(settings->GetGraphicalOutput());

        // Stack limit
        VAbstractApplication::VApp()->getUndoStack()->setUndoLimit(settings->GetUndoCount());

        // Text under tool buton icon
        ToolBarStyles();

        ToolboxIconSize();

        QFont f = ui->plainTextEditPatternMessages->font();
        f.setPointSize(qMax(settings->GetPatternMessageFontSize(f.pointSize()), 1));
        ui->plainTextEditPatternMessages->setFont(f);
    }
    else
    {
        qWarning() << tr("Cannot read settings from a malformed .INI file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief WriteSettings save setting for app.
 */
void MainWindow::WriteSettings()
{
    ActionDraw(true);

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetGeometry(saveGeometry());
    settings->SetToolbarsState(saveState(static_cast<int>(AppVersion())));

    settings->SetDockWidgetGroupsActive(ui->dockWidgetGroups->isVisible());
    settings->SetDockWidgetToolOptionsActive(ui->dockWidgetToolOptions->isVisible());
    settings->SetDockWidgetPatternMessagesActive(ui->dockWidgetMessages->isVisible());
    settings->SetDockWidgetBackgroundImagesActive(actionDockWidgetBackgroundImages->isChecked());

    settings->sync();
    if (settings->status() == QSettings::AccessError)
    {
        qWarning() << tr("Cannot save settings. Access denied.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MaybeSave The function is called to save pending changes.
 * @return returns true in all cases, except when the user clicks Cancel.
 */
auto MainWindow::MaybeSave() -> bool
{
    if (this->isWindowModified() && m_guiEnabled)
    {
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(
            tr("Unsaved changes"), tr("The pattern has been modified. Do you want to save your changes?"),
            QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel, this, Qt::Sheet));

        messageBox->setDefaultButton(QMessageBox::Yes);
        messageBox->setEscapeButton(QMessageBox::Cancel);

        messageBox->setButtonText(QMessageBox::Yes,
                                  VAbstractValApplication::VApp()->GetPatternPath().isEmpty() || m_patternReadOnly
                                      ? tr("Save…")
                                      : tr("Save"));
        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

        messageBox->setWindowModality(Qt::ApplicationModal);
        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

        switch (ret)
        {
            case QMessageBox::Yes:
                if (m_patternReadOnly)
                {
                    return on_actionSaveAs_triggered();
                }
                else
                {
                    return on_actionSave_triggered();
                }
            case QMessageBox::No:
                return true;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CreateMenus()
{
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        ui->menuFile->insertAction(ui->actionPreferences, m_recentFileActs.at(i));
    }
    m_separatorAct = new QAction(this);
    m_separatorAct->setSeparator(true);
    ui->menuFile->insertAction(ui->actionPreferences, m_separatorAct);
    UpdateRecentFileActions();

    // Add Undo/Redo actions.
    undoAction = VAbstractApplication::VApp()->getUndoStack()->createUndoAction(this, tr("&Undo"));
    connect(undoAction, &QAction::triggered, m_toolOptions, &VToolOptionsPropertyBrowser::RefreshOptions);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo")));
    ui->menuPatternPiece->insertAction(ui->actionLast_tool, undoAction);
    ui->toolBarTools->addAction(undoAction);

    redoAction = VAbstractApplication::VApp()->getUndoStack()->createRedoAction(this, tr("&Redo"));
    connect(redoAction, &QAction::triggered, m_toolOptions, &VToolOptionsPropertyBrowser::RefreshOptions);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-redo")));
    ui->menuPatternPiece->insertAction(ui->actionLast_tool, redoAction);
    ui->toolBarTools->addAction(redoAction);

    m_separatorAct = new QAction(this);
    m_separatorAct->setSeparator(true);
    ui->menuPatternPiece->insertAction(ui->actionPattern_properties, m_separatorAct);

    AddDocks();
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
void MainWindow::LastUsedTool()
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were handled.");

    if (m_currentTool == m_lastUsedTool)
    {
        return;
    }

    switch (m_lastUsedTool)
    {
        case Tool::Arrow:
            ui->actionToolSelect->setChecked(true);
            ArrowTool(true);
            break;
        case Tool::BasePoint:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::LAST_ONE_DO_NOT_USE:
        case Tool::NodePoint:
        case Tool::NodeArc:
        case Tool::NodeElArc:
        case Tool::NodeSpline:
        case Tool::NodeSplinePath:
        case Tool::BackgroundImage:
        case Tool::BackgroundImageControls:
        case Tool::BackgroundPixmapImage:
        case Tool::BackgroundSVGImage:
            Q_UNREACHABLE(); //-V501
            // Nothing to do here because we can't create this tool from main window.
            break;
        case Tool::EndLine:
            ui->actionEndLineTool->setChecked(true);
            ToolEndLine(true);
            break;
        case Tool::Line:
            ui->actionLineTool->setChecked(true);
            ToolLine(true);
            break;
        case Tool::AlongLine:
            ui->actionAlongLineTool->setChecked(true);
            ToolAlongLine(true);
            break;
        case Tool::Midpoint:
            ui->actionMidpointTool->setChecked(true);
            ToolMidpoint(true);
            break;
        case Tool::ShoulderPoint:
            ui->actionShoulderPointTool->setChecked(true);
            ToolShoulderPoint(true);
            break;
        case Tool::Normal:
            ui->actionNormalTool->setChecked(true);
            ToolNormal(true);
            break;
        case Tool::Bisector:
            ui->actionBisectorTool->setChecked(true);
            ToolBisector(true);
            break;
        case Tool::LineIntersect:
            ui->actionLineIntersectTool->setChecked(true);
            ToolLineIntersect(true);
            break;
        case Tool::Spline:
            ui->actionSplineTool->setChecked(true);
            ToolSpline(true);
            break;
        case Tool::CubicBezier:
            ui->actionCubicBezierTool->setChecked(true);
            ToolCubicBezier(true);
            break;
        case Tool::Arc:
            ui->actionArcTool->setChecked(true);
            ToolArc(true);
            break;
        case Tool::SplinePath:
            ui->actionSplinePathTool->setChecked(true);
            ToolSplinePath(true);
            break;
        case Tool::CubicBezierPath:
            ui->actionCubicBezierPathTool->setChecked(true);
            ToolCubicBezierPath(true);
            break;
        case Tool::PointOfContact:
            ui->actionPointOfContactTool->setChecked(true);
            ToolPointOfContact(true);
            break;
        case Tool::Piece:
            ui->actionNewDetailTool->setChecked(true);
            ToolDetail(true);
            break;
        case Tool::PiecePath:
            ui->actionInternalPathTool->setChecked(true);
            ToolPiecePath(true);
            break;
        case Tool::Height:
            ui->actionHeightTool->setChecked(true);
            ToolHeight(true);
            break;
        case Tool::Triangle:
            ui->actionTriangleTool->setChecked(true);
            ToolTriangle(true);
            break;
        case Tool::PointOfIntersection:
            ui->actionPointOfIntersectionTool->setChecked(true);
            ToolPointOfIntersection(true);
            break;
        case Tool::PointOfIntersectionArcs:
            ui->actionPointOfIntersectionArcsTool->setChecked(true);
            ToolPointOfIntersectionArcs(true);
            break;
        case Tool::CutSpline:
            ui->actionSplineCutPointTool->setChecked(true);
            ToolCutSpline(true);
            break;
        case Tool::CutSplinePath:
            ui->actionSplinePathCutPointTool->setChecked(true);
            ToolCutSplinePath(true);
            break;
        case Tool::UnionDetails:
            ui->actionUnionDetailsTool->setChecked(true);
            ToolUnionDetails(true);
            break;
        case Tool::DuplicateDetail:
            ui->actionDuplicateDetailTool->setChecked(true);
            ToolDuplicateDetail(true);
            break;
        case Tool::CutArc:
            ui->actionArcCutPointTool->setChecked(true);
            ToolCutArc(true);
            break;
        case Tool::LineIntersectAxis:
            ui->actionLineIntersectAxisTool->setChecked(true);
            ToolLineIntersectAxis(true);
            break;
        case Tool::CurveIntersectAxis:
            ui->actionCurveIntersectAxisTool->setChecked(true);
            ToolCurveIntersectAxis(true);
            break;
        case Tool::ArcIntersectAxis:
            ui->actionArcIntersectAxisTool->setChecked(true);
            ToolArcIntersectAxis(true);
            break;
        case Tool::PointOfIntersectionCircles:
            ui->actionPointOfIntersectionCirclesTool->setChecked(true);
            ToolPointOfIntersectionCircles(true);
            break;
        case Tool::PointOfIntersectionCurves:
            ui->actionIntersectionCurvesTool->setChecked(true);
            ToolPointOfIntersectionCurves(true);
            break;
        case Tool::PointFromCircleAndTangent:
            ui->actionPointFromCircleAndTangentTool->setChecked(true);
            ToolPointFromCircleAndTangent(true);
            break;
        case Tool::PointFromArcAndTangent:
            ui->actionPointFromArcAndTangentTool->setChecked(true);
            ToolPointFromArcAndTangent(true);
            break;
        case Tool::ArcWithLength:
            ui->actionArcWithLengthTool->setChecked(true);
            ToolArcWithLength(true);
            break;
        case Tool::TrueDarts:
            ui->actionTrueDartsTool->setChecked(true);
            ToolTrueDarts(true);
            break;
        case Tool::Group:
            ui->actionGroupTool->setChecked(true);
            ToolGroup(true);
            break;
        case Tool::Rotation:
            ui->actionRotationTool->setChecked(true);
            ToolRotation(true);
            break;
        case Tool::FlippingByLine:
            ui->actionFlippingByLineTool->setChecked(true);
            ToolFlippingByLine(true);
            break;
        case Tool::FlippingByAxis:
            ui->actionFlippingByAxisTool->setChecked(true);
            ToolFlippingByAxis(true);
            break;
        case Tool::Move:
            ui->actionMoveTool->setChecked(true);
            ToolMove(true);
            break;
        case Tool::EllipticalArc:
            ui->actionEllipticalArcTool->setChecked(true);
            ToolEllipticalArc(true);
            break;
        case Tool::Pin:
            ui->actionPinTool->setChecked(true);
            ToolPin(true);
            break;
        case Tool::InsertNode:
            ui->actionInsertNodeTool->setChecked(true);
            ToolInsertNode(true);
            break;
        case Tool::PlaceLabel:
            ui->actionPlaceLabelTool->setChecked(true);
            ToolPlaceLabel(true);
            break;
    }
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::AddDocks()
{
    ui->menuWindow->addSeparator();

    // Add dock
    actionDockWidgetToolOptions = ui->dockWidgetToolOptions->toggleViewAction();
    connect(actionDockWidgetToolOptions, &QAction::triggered, this,
            [this](bool checked) { m_toolOptionsActive = checked; });
    ui->menuWindow->addAction(actionDockWidgetToolOptions);

    actionDockWidgetGroups = ui->dockWidgetGroups->toggleViewAction();
    connect(actionDockWidgetGroups, &QAction::triggered, this, [this](bool checked) { m_groupsActive = checked; });
    ui->menuWindow->addAction(actionDockWidgetGroups);

    QAction *action = ui->dockWidgetMessages->toggleViewAction();
    connect(action, &QAction::triggered, this, [this](bool checked) { m_patternMessagesActive = checked; });
    ui->menuWindow->addAction(action);

    actionDockWidgetBackgroundImages = ui->dockWidgetBackgroundImages->toggleViewAction();
    connect(actionDockWidgetBackgroundImages, &QAction::triggered, this,
            [this](bool checked) { m_backgroundImagesActive = checked; });
    ui->menuWindow->addAction(actionDockWidgetBackgroundImages);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDocksContain()
{
    qCDebug(vMainWindow, "Initialization property browser.");
    m_toolOptions = new VToolOptionsPropertyBrowser(ui->dockWidgetToolOptions);

    connect(ui->view, &VMainGraphicsView::itemClicked, m_toolOptions, &VToolOptionsPropertyBrowser::itemClicked);
    connect(doc, &VPattern::FullUpdateFromFile, m_toolOptions, &VToolOptionsPropertyBrowser::UpdateOptions);

    qCDebug(vMainWindow, "Initialization groups dock.");
    m_groupsWidget = new VWidgetGroups(doc, this);
    ui->dockWidgetGroups->setWidget(m_groupsWidget);
    connect(doc, &VAbstractPattern::UpdateGroups, this, &MainWindow::UpdateVisibilityGroups);

    m_detailsWidget = new VWidgetDetails(pattern, doc, this);
    connect(doc, &VPattern::FullUpdateFromFile, m_detailsWidget, &VWidgetDetails::UpdateList);
    connect(doc, &VPattern::UpdateInLayoutList, m_detailsWidget, &VWidgetDetails::UpdateList);
    connect(doc, &VPattern::ShowDetail, m_detailsWidget, &VWidgetDetails::SelectDetail);
    connect(m_detailsWidget, &VWidgetDetails::Highlight, m_sceneDetails, &VMainGraphicsScene::HighlightItem);
    m_detailsWidget->setVisible(false);

    m_backgroundImagesWidget = new VWidgetBackgroundImages(doc, this);
    ui->dockWidgetBackgroundImages->setWidget(m_backgroundImagesWidget);
    connect(m_backgroundImagesWidget, &VWidgetBackgroundImages::DeleteImage, this, &MainWindow::RemoveBackgroundImage);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::OpenNewValentina(const QString &fileName) const -> bool
{
    if (this->isWindowModified() || not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
    {
        VApplication::NewValentina(fileName);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CreateActions()
{
    ui->setupUi(this);

    connect(ui->actionArrowTool, &QAction::triggered, this, &MainWindow::ArrowTool);
    connect(ui->actionDraw, &QAction::triggered, this, &MainWindow::ActionDraw);
    connect(ui->actionDetails, &QAction::triggered, this, &MainWindow::ActionDetails);
    connect(ui->actionLayout, &QAction::triggered, this, &MainWindow::ActionLayout);
    connect(ui->actionHistory, &QAction::triggered, this, &MainWindow::ActionHistory_triggered);
    connect(ui->actionExportRecipe, &QAction::triggered, this, &MainWindow::ActionExportRecipe_triggered);
    connect(ui->actionNewDraw, &QAction::triggered, this, &MainWindow::ActionNewDraw_triggered);
    connect(ui->actionExportIncrementsToCSV, &QAction::triggered, this, &MainWindow::ExportDataToCSV);
    connect(ui->actionExportFinalMeasurementsToCSV, &QAction::triggered, this, &MainWindow::ExportFMeasurementsToCSV);
    connect(ui->actionTable, &QAction::triggered, this, &MainWindow::ActionTable_triggered);
    connect(ui->actionFinalMeasurements, &QAction::triggered, this, &MainWindow::ActionFinalMeasurements_triggered);

    connect(ui->actionAbout_Qt, &QAction::triggered, this, [this]() { QMessageBox::aboutQt(this, tr("About Qt")); });

    connect(ui->actionAbout_Valentina, &QAction::triggered, this,
            [this]()
            {
                auto *aboutDialog = new DialogAboutApp(this);
                aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
                aboutDialog->show();
            });

    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::Preferences);
    connect(ui->actionReportBug, &QAction::triggered, this,
            []()
            {
                qCDebug(vMainWindow, "Reporting bug");
                QDesktopServices::openUrl(
                    QUrl(QStringLiteral("https://gitlab.com/smart-pattern/valentina/issues/new")));
            });
    connect(ui->actionShop, &QAction::triggered, this,
            []()
            {
                qCDebug(vMainWindow, "Open shop");
                QDesktopServices::openUrl(QUrl(QStringLiteral("https://smart-pattern.com.ua/catalogue/")));
            });

    connect(ui->actionLast_tool, &QAction::triggered, this, &MainWindow::LastUsedTool);

    ui->actionInteractiveTools->setChecked(VAbstractValApplication::VApp()->ValentinaSettings()->IsInteractiveTools());
    connect(ui->actionInteractiveTools, &QAction::triggered, this,
            [](bool checked) { VAbstractValApplication::VApp()->ValentinaSettings()->SetInteractiveTools(checked); });

    connect(ui->actionPattern_properties, &QAction::triggered, this,
            [this]()
            {
                DialogPatternProperties proper(doc, pattern, this);
                connect(&proper, &DialogPatternProperties::UpddatePieces, m_sceneDetails,
                        &VMainGraphicsScene::UpdatePiecePassmarks);
                proper.exec();
            });

    ui->actionPattern_properties->setEnabled(false);
    connect(ui->actionClosePattern, &QAction::triggered, this,
            [this]()
            {
                if (MaybeSave())
                {
                    FileClosedCorrect();
                    Clear();
                }
            });

    ui->actionShowCurveDetails->setChecked(VAbstractValApplication::VApp()->ValentinaSettings()->IsShowCurveDetails());
    connect(ui->actionShowCurveDetails, &QAction::triggered, this,
            [this](bool checked)
            {
                emit ui->view->itemClicked(nullptr);
                m_sceneDraw->EnableDetailsMode(checked);
                VAbstractValApplication::VApp()->ValentinaSettings()->SetShowCurveDetails(checked);
            });

    ui->actionShowAccuracyRadius->setChecked(
        VAbstractValApplication::VApp()->ValentinaSettings()->GetShowAccuracyRadius());
    connect(ui->actionShowAccuracyRadius, &QAction::triggered, this,
            [this](bool checked)
            {
                VAbstractValApplication::VApp()->ValentinaSettings()->SetShowAccuracyRadius(checked);
                m_sceneDetails->update();
                m_sceneDraw->update();
            });

    ui->actionShowMainPath->setChecked(VAbstractValApplication::VApp()->ValentinaSettings()->IsPieceShowMainPath());
    connect(ui->actionShowMainPath, &QAction::triggered, this, &MainWindow::ActionShowMainPath_triggered);

    connect(ui->actionLoadIndividual, &QAction::triggered, this, &MainWindow::LoadIndividual);
    connect(ui->actionLoadMultisize, &QAction::triggered, this, &MainWindow::LoadMultisize);
    connect(ui->actionOpenTape, &QAction::triggered, this, &MainWindow::ActionOpenTape_triggered);
    connect(ui->actionEditCurrent, &QAction::triggered, this, &MainWindow::ShowMeasurements);
    connect(ui->actionExportAs, &QAction::triggered, this, &MainWindow::ExportLayoutAs);
    connect(ui->actionPrintPreview, &QAction::triggered, this, &MainWindow::PrintPreviewOrigin);
    connect(ui->actionPrintPreviewTiled, &QAction::triggered, this, &MainWindow::PrintPreviewTiled);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::PrintOrigin);
    connect(ui->actionPrintTiled, &QAction::triggered, this, &MainWindow::PrintTiled);

    // Actions for recent files loaded by a main window application.
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        auto *action = new QAction(this);
        action->setVisible(false);
        m_recentFileActs[i] = action;
        connect(m_recentFileActs[i], &QAction::triggered, this,
                [this]()
                {
                    if (auto *action = qobject_cast<QAction *>(sender()))
                    {
                        const QString filePath = action->data().toString();
                        if (not filePath.isEmpty())
                        {
                            LoadPattern(filePath);
                        }
                    }
                });
    }

    connect(ui->actionSyncMeasurements, &QAction::triggered, this, &MainWindow::SyncMeasurements);
    connect(ui->actionUnloadMeasurements, &QAction::triggered, this, &MainWindow::UnloadMeasurements);

    connect(ui->actionLabelTemplateEditor, &QAction::triggered, this,
            [this]()
            {
                DialogEditLabel editor(doc, pattern);
                editor.exec();
            });

    connect(ui->actionWatermarkEditor, &QAction::triggered, this, &MainWindow::CreateWatermark);
    connect(ui->actionEditCurrentWatermark, &QAction::triggered, this, &MainWindow::EditCurrentWatermark);
    connect(ui->actionLoadWatermark, &QAction::triggered, this, &MainWindow::LoadWatermark);
    connect(ui->actionRemoveWatermark, &QAction::triggered, this, &MainWindow::RemoveWatermark);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitAutoSave()
{
    // Autosaving file each 1 minutes
    delete m_autoSaveTimer;
    m_autoSaveTimer = nullptr;

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::AutoSavePattern);
    m_autoSaveTimer->stop();

    if (VAbstractValApplication::VApp()->ValentinaSettings()->GetAutosaveState())
    {
        const qint32 autoTime = VAbstractValApplication::VApp()->ValentinaSettings()->GetAutosaveTime();
        m_autoSaveTimer->start(autoTime * 60000);
        qCDebug(vMainWindow, "Autosaving each %d minutes.", autoTime);
    }
    VApplication::VApp()->setAutoSaveTimer(m_autoSaveTimer);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::PatternPieceName(QString &name) -> bool
{
    QScopedPointer<QInputDialog> dlg(new QInputDialog(this));
    dlg->setInputMode(QInputDialog::TextInput);
    dlg->setLabelText(tr("Pattern piece:"));
    dlg->setTextEchoMode(QLineEdit::Normal);
    dlg->setWindowTitle(tr("Enter a new label for the pattern piece."));
    dlg->resize(300, 100);
    dlg->setTextValue(name);
    QString nameDraw;
    while (true)
    {
        const bool bOk = dlg->exec();
        nameDraw = dlg->textValue();
        if (not bOk)
        {
            return false;
        }

        if (nameDraw.isEmpty())
        {
            continue;
        }

        if (m_comboBoxDraws->findText(nameDraw) == -1)
        {
            name = nameDraw;
            break; // unique name
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    CancelTool();

    delete doc;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LoadPattern open pattern file.
 * @param fileName name of file.
 */
auto MainWindow::LoadPattern(QString fileName, const QString &customMeasureFile) -> bool
{
    qCDebug(vMainWindow, "Loading new file %s.", qUtf8Printable(fileName));

    { // Convert to absolute path if need
        QFileInfo info(fileName);
        if (info.exists() && info.isRelative())
        {
            fileName = QFileInfo(QDir::currentPath() + '/'_L1 + fileName).canonicalFilePath();
        }
    }

    QFuture<VPatternConverter *> futureConverter = QtConcurrent::run(
        [fileName]()
        {
            std::unique_ptr<VPatternConverter> converter(new VPatternConverter(fileName));
            return converter.release();
        });

    // We have unsaved changes or load more then one file per time
    if (OpenNewValentina(fileName))
    {
        return false;
    }

    if (fileName.isEmpty())
    {
        qCDebug(vMainWindow, "Got empty file.");
        Clear();
        return false;
    }

    if (fileName.endsWith(QStringLiteral(".vit")) || fileName.endsWith(QStringLiteral(".vst")))
    {
        try
        {
            // Here comes undocumented Valentina's feature.
            // Because app bundle in Mac OS X doesn't allow setup assosiation for Tape we must do this through Valentina
            VMeasurements m(pattern);
            m.setXMLContent(fileName);

            if (m.Type() == MeasurementsType::Multisize || m.Type() == MeasurementsType::Individual)
            {
                QStringList arguments{fileName};
                if (isNoScaling)
                {
                    arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
                }

                VApplication::StartDetachedProcess(VApplication::TapeFilePath(), arguments);
                QCoreApplication::exit(V_EX_OK);
                return false; // stop continue processing
            }
        }
        catch (VException &e)
        {
            qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                       qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
            Clear();
            if (not VApplication::IsGUIMode())
            {
                QCoreApplication::exit(V_EX_NOINPUT);
            }
            return false;
        }
    }

    if (fileName.endsWith(QStringLiteral(".vlt")))
    {
        // Here comes undocumented Valentina's feature.
        // Because app bundle in Mac OS X doesn't allow setup assosiation for Puzzle we must do this through Valentina
        QStringList arguments{fileName};
        if (isNoScaling)
        {
            arguments.append(QStringLiteral("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        VApplication::StartDetachedProcess(VApplication::PuzzleFilePath(), arguments);
        QCoreApplication::exit(V_EX_OK);
        return false; // stop continue processing
    }

    qCDebug(vMainWindow, "Loking file");
    VlpCreateLock(m_lock, fileName);

    if (m_lock->IsLocked())
    {
        qCDebug(vMainWindow, "Pattern file %s was locked.", qUtf8Printable(fileName));
    }
    else
    {
        if (not IgnoreLocking(m_lock->GetLockError(), fileName, VApplication::IsGUIMode()))
        {
            return false;
        }
    }

    // On this stage scene empty. Fit scene size to view size
    VMainGraphicsView::NewSceneRect(m_sceneDraw, ui->view);
    VMainGraphicsView::NewSceneRect(m_sceneDetails, ui->view);

    VAbstractValApplication::VApp()->setOpeningPattern(); // Begin opening file
    try
    {
        // Quick reading measurements
        doc->setXMLContent(fileName);
        const unsigned currentFormatVersion = VDomDocument::GetFormatVersion(doc->GetFormatVersionStr());
        if (currentFormatVersion != VPatternConverter::PatternMaxVer)
        { // Because we rely on the fact that we know where is path to measurements optimization available only for
          // the latest format version
            QScopedPointer<VPatternConverter> converter(futureConverter.result());
            m_curFileFormatVersion = converter->GetCurrentFormatVersion();
            m_curFileFormatVersionStr = converter->GetFormatVersionStr();
            doc->setXMLContent(converter->Convert());

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
                statistic->SendPatternFormatVersion(uptime, m_curFileFormatVersionStr);
            }
        }

        if (!customMeasureFile.isEmpty())
        {
            doc->SetMPath(RelativeMPath(fileName, customMeasureFile));
        }
        VAbstractValApplication::VApp()->SetPatternUnits(doc->Units());

        QString path = AbsoluteMPath(fileName, doc->MPath());
        QString fixedMPath;

        // See issue #976. Pattern can lost link to measurements
        if (path.isEmpty() && doc->RequiresMeasurements())
        {
            const QString fakeName = QStringLiteral("unknown_measurements.vit");
            // Check if exist
            fixedMPath = CheckPathToMeasurements(fileName, fakeName);
            if (fixedMPath.isEmpty())
            {
                VAbstractValApplication::VApp()->setOpeningPattern(); // End opening file
                Clear();
                qCCritical(vMainWindow, "%s",
                           qUtf8Printable(tr("The measurements file '%1' could not be found.").arg(fakeName)));
                if (not VApplication::IsGUIMode())
                {
                    QCoreApplication::exit(V_EX_NOINPUT);
                }
                return false;
            }

            path = AbsoluteMPath(fileName, doc->MPath());
        }

        if (not path.isEmpty())
        {
            // Check if exist
            fixedMPath = CheckPathToMeasurements(fileName, path);
            if (fixedMPath.isEmpty())
            {
                VAbstractValApplication::VApp()->setOpeningPattern(); // End opening file
                Clear();
                qCCritical(vMainWindow, "%s",
                           qUtf8Printable(tr("The measurements file '%1' could not be found.").arg(path)));
                if (not VApplication::IsGUIMode())
                {
                    QCoreApplication::exit(V_EX_NOINPUT);
                }
                return false;
            }

            if (not LoadMeasurements(fixedMPath))
            {
                qCCritical(vMainWindow, "%s",
                           qUtf8Printable(tr("The measurements file '%1' could not be found or "
                                             "provides not enough information.")
                                              .arg(fixedMPath)));
                VAbstractValApplication::VApp()->setOpeningPattern(); // End opening file
                Clear();
                if (not VApplication::IsGUIMode())
                {
                    QCoreApplication::exit(V_EX_NOINPUT);
                }
                return false;
            }

            ui->actionUnloadMeasurements->setEnabled(true);
            m_watcher->addPath(fixedMPath);
            ui->actionEditCurrent->setEnabled(true);
        }

        if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Unknown)
        { // Show toolbar only if was not uploaded any measurements.
            ToolBarOption();
        }

        if (currentFormatVersion == VPatternConverter::PatternMaxVer)
        {
            // Real read
            QScopedPointer<VPatternConverter> converter(futureConverter.result());
            m_curFileFormatVersion = converter->GetCurrentFormatVersion();
            m_curFileFormatVersionStr = converter->GetFormatVersionStr();
            doc->setXMLContent(converter->Convert());
            if (!customMeasureFile.isEmpty())
            {
                doc->SetMPath(RelativeMPath(fileName, customMeasureFile));
            }
            else if (not path.isEmpty() && fixedMPath != path)
            {
                doc->SetMPath(RelativeMPath(fileName, fixedMPath));
            }
            VAbstractValApplication::VApp()->SetPatternUnits(doc->Units());
        }
    }
    catch (VException &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        VAbstractValApplication::VApp()->setOpeningPattern(); // End opening file
        Clear();
        if (not VApplication::IsGUIMode())
        {
            QCoreApplication::exit(V_EX_NOINPUT);
        }
        return false;
    }

    m_progressBar->setVisible(true);
    m_statusLabel->setVisible(false);
    m_progressBar->setValue(0);
    const int elements = doc->ElementsToParse();
    m_progressBar->setMaximum(elements);
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    m_taskbarProgress->setVisible(true);
    m_taskbarProgress->setMaximum(elements);
#endif

    FullParseFile();

    m_progressBar->setVisible(false);
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    m_taskbarProgress->setVisible(false);
#endif
    m_statusLabel->setVisible(true);

    if (m_guiEnabled)
    { // No errors occurred
        if (VApplication::IsGUIMode())
        {
            /* Collect garbage only after successfully parse. This way wrongly accused items have one more time to
             * restore a reference. */
            QTimer::singleShot(V_MSECONDS(100), Qt::CoarseTimer, this, [this]() { doc->GarbageCollector(true); });
        }

        m_patternReadOnly = doc->IsReadOnly();
        m_sceneDraw->SetAcceptDrop(true);
        SetEnableWidgets(true);
        setCurrentFile(fileName);
        qCDebug(vMainWindow, "File loaded.");

        // Fit scene size to best size for first show
        ZoomFirstShow();

        ActionDraw(true);

        VAbstractValApplication::VApp()->setOpeningPattern(); // End opening file
        m_statusLabel->setText(QString());
        return true;
    }

    VAbstractValApplication::VApp()->setOpeningPattern(); // End opening file
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::GetUnlokedRestoreFileList() -> QStringList
{
    QStringList restoreFiles;
    // Take all files that need to be restored
    QStringList files = VAbstractValApplication::VApp()->ValentinaSettings()->GetRestoreFileList();
    if (not files.empty())
    {
        restoreFiles.reserve(files.size());
        for (auto &file : files)
        {
            // Seeking file that realy need reopen
            VLockGuard<char> tmp(file);
            if (tmp.IsLocked())
            {
                restoreFiles.append(file);
            }
        }

        // Clearing list after filtering
        for (auto &file : restoreFiles)
        {
            files.removeAll(file);
        }

        // Clear all files that do not exist.
        QStringList filtered;
        for (auto &file : files)
        {
            if (QFileInfo::exists(file))
            {
                filtered.append(file);
            }
        }

        VAbstractValApplication::VApp()->ValentinaSettings()->SetRestoreFileList(filtered);
    }
    return restoreFiles;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolBarStyles()
{
    MainWindow::ToolBarStyle(ui->toolBarDraws);
    MainWindow::ToolBarStyle(ui->toolBarOption);
    MainWindow::ToolBarStyle(ui->toolBarStages);
    MainWindow::ToolBarStyle(ui->toolBarTools);
    MainWindow::ToolBarStyle(ui->mainToolBar);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolboxIconSize()
{
    auto SetIconSize = [](QToolBar *bar)
    {
        VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
        QSize size = settings->GetToolboxIconSizeSmall() ? QSize(24, 24) : QSize(32, 32);
        bar->setIconSize(size);
    };

    SetIconSize(ui->toolBarCurveTools);
    SetIconSize(ui->toolBarDetailTools);
    SetIconSize(ui->toolBarLineTools);
    SetIconSize(ui->toolBarOperationTools);
    SetIconSize(ui->toolBarPointTools);
    SetIconSize(ui->toolBarSelectingTools);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowPaper(int index)
{
    if (index < 0 || index >= m_layoutSettings->LayoutScenes().size())
    {
        ui->view->setScene(tempSceneLayout);
    }
    else
    {
        ui->view->setScene(m_layoutSettings->LayoutScenes().at(index));
    }

    ui->view->fitInView(ui->view->scene()->sceneRect(), Qt::KeepAspectRatio);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::Preferences()
{
    // Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
    static QPointer<DialogPreferences> guard; // Prevent any second run
    if (guard.isNull())
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        auto *preferences = new DialogPreferences(this);
        // QScopedPointer needs to be sure any exception will never block guard
        QScopedPointer<DialogPreferences> dlg(preferences);
        guard = preferences;
        connect(dlg.data(), &DialogPreferences::UpdateProperties, this, &MainWindow::WindowsLocale); // Must be first
        connect(dlg.data(), &DialogPreferences::UpdateProperties, m_toolOptions,
                &VToolOptionsPropertyBrowser::RefreshOptions);
        connect(dlg.data(), &DialogPreferences::UpdateProperties, this, &MainWindow::ToolBarStyles);
        connect(dlg.data(), &DialogPreferences::UpdateProperties, this, &MainWindow::ToolboxIconSize);
        connect(dlg.data(), &DialogPreferences::UpdateProperties, this, &MainWindow::ToolBarDrawTools);
        connect(dlg.data(), &DialogPreferences::UpdateProperties, this, [this]() { emit doc->FullUpdateFromFile(); });
        connect(dlg.data(), &DialogPreferences::UpdateProperties, ui->view,
                &VMainGraphicsView::ResetScrollingAnimation);
        QGuiApplication::restoreOverrideCursor();

        if (guard->exec() == QDialog::Accepted)
        {
            InitAutoSave();
            VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
            ui->actionExportFontCorrections->setEnabled(settings->GetSingleStrokeOutlineFont());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void MainWindow::CreateMeasurements()
{
    const QString tape = VApplication::VApp()->TapeFilePath();
    const QString workingDirectory = QFileInfo(tape).absoluteDir().absolutePath();

    QStringList arguments;
    if (isNoScaling)
    {
        arguments.append("--"_L1 + LONG_OPTION_NO_HDPI_SCALING);
    }

    QProcess::startDetached(tape, arguments, workingDirectory);
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ExportDrawAs()
{
    auto Uncheck = qScopeGuard([this] { ui->actionExportDraw->setChecked(false); });

    QString filters(tr("Scalable Vector Graphics files") + QStringLiteral("(*.svg)"));
    QString dir = QDir::homePath() + '/'_L1 + FileName() + QStringLiteral(".svg");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save draw"), dir, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return;
    }

    QFileInfo f(fileName);
    if (f.suffix().isEmpty() || f.suffix() != "svg"_L1)
    {
        fileName += ".svg"_L1;
    }

    ExportDraw(fileName);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ExportLayoutAs()
{
    auto Uncheck = qScopeGuard([this] { ui->actionLayoutExportAs->setChecked(false); });

    if (m_layoutSettings->IsLayoutStale())
    {
        if (VPrintLayout::ContinueIfLayoutStale(this) == QMessageBox::No)
        {
            return;
        }
    }

    try
    {
        m_dialogSaveLayout = QSharedPointer<DialogSaveLayout>(new DialogSaveLayout(
            static_cast<int>(m_layoutSettings->LayoutScenes().size()), Draw::Layout, FileName(), this));

        if (m_dialogSaveLayout->exec() == QDialog::Rejected)
        {
            m_dialogSaveLayout.clear();
            return;
        }

        ExportData(QVector<VLayoutPiece>());
        m_dialogSaveLayout.clear();
    }
    catch (const VException &e)
    {
        m_dialogSaveLayout.clear();
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Export error.")), qUtf8Printable(e.ErrorMessage()),
                  qUtf8Printable(e.DetailedInformation()));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ExportDetailsAs()
{
    auto Uncheck = qScopeGuard([this] { ui->actionDetailExportAs->setChecked(false); });

    QVector<DetailForLayout> detailsInLayout = SortDetailsForLayout(pattern->DataPieces());

    if (detailsInLayout.count() == 0)
    {
        QMessageBox::information(this, tr("Layout mode"),
                                 tr("You don't have enough details to export. Please, "
                                    "include at least one detail in layout."),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QVector<VLayoutPiece> listDetails;
    try
    {
        listDetails = PrepareDetailsForLayout(detailsInLayout);
    }
    catch (VException &e)
    {
        QMessageBox::warning(this, tr("Export details"),
                             tr("Can't export details.") + QStringLiteral(" \n") + e.ErrorMessage(), QMessageBox::Ok,
                             QMessageBox::Ok);
        return;
    }

    try
    {
        m_dialogSaveLayout =
            QSharedPointer<DialogSaveLayout>(new DialogSaveLayout(1, Draw::Modeling, FileName(), this));

        if (m_dialogSaveLayout->exec() == QDialog::Rejected)
        {
            m_dialogSaveLayout.clear();
            return;
        }

        ExportData(listDetails);
        m_dialogSaveLayout.clear();
    }
    catch (const VException &e)
    {
        m_dialogSaveLayout.clear();
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Export error.")), qUtf8Printable(e.ErrorMessage()),
                  qUtf8Printable(e.DetailedInformation()));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ReopenFilesAfterCrash(QStringList &args)
{
    const QStringList files = GetUnlokedRestoreFileList();
    if (files.empty())
    {
        return;
    }

    qCDebug(vMainWindow, "Reopen files after crash.");

    QStringList restoreFiles;
    restoreFiles.reserve(files.size());
    for (const auto &file : files)
    {
        if (QFile::exists(file + *autosavePrefix))
        {
            restoreFiles.append(file);
        }
    }

    if (restoreFiles.empty())
    {
        return;
    }

    QMessageBox::StandardButton reply;
    const QString mes = tr("Valentina didn't shut down correctly. Do you want reopen files (%1) you had open?")
                            .arg(restoreFiles.size());
    reply = QMessageBox::question(this, tr("Reopen files."), mes, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (reply != QMessageBox::Yes)
    {
        return;
    }

    qCDebug(vMainWindow, "User said Yes.");

    for (auto &file : restoreFiles)
    {
        QString error;
        if (VDomDocument::SafeCopy(file + *autosavePrefix, file, error))
        {
            QFile autoFile(file + *autosavePrefix);
            autoFile.remove();
            LoadPattern(file);
            args.removeAll(file); // Do not open file twice after we restore him.
        }
        else
        {
            qCDebug(vMainWindow, "Could not copy %s%s to %s %s", qUtf8Printable(file), qUtf8Printable(*autosavePrefix),
                    qUtf8Printable(file), qUtf8Printable(error));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::CheckPathToMeasurements(const QString &patternPath, const QString &path) -> QString
{
    if (path.isEmpty())
    {
        return path;
    }

    auto FindLocation = [this](const QString &filter, const QString &dirPath, const QString &selectedName)
    {
        QFileDialog dialog(this, tr("Open file"), dirPath, filter);
        dialog.selectFile(selectedName);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOption(QFileDialog::DontUseNativeDialog,
                         VAbstractApplication::VApp()->Settings()->IsDontUseNativeDialog());

        QString mPath;
        if (dialog.exec() == QDialog::Accepted)
        {
            mPath = dialog.selectedFiles().value(0);
        }

        return mPath;
    };

    QFileInfo table(path);
    if (table.exists())
    {
        return path;
    }

    if (not VApplication::IsGUIMode())
    {
        return {}; // console mode doesn't support fixing path to a measurement file
    }

    const QString text = tr("The measurements file <br/><br/> <b>%1</b> <br/><br/> could not be found. Do you "
                            "want to update the file location?")
                             .arg(path);
    QMessageBox::StandardButton res = QMessageBox::question(this, tr("Loading measurements file"), text,
                                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (res == QMessageBox::No)
    {
        return {};
    }

    MeasurementsType patternType;
    if (table.suffix() == "vst"_L1)
    {
        patternType = MeasurementsType::Multisize;
    }
    else
    {
        patternType = MeasurementsType::Individual; // or Unknown
    }

    auto DirPath = [patternPath, table](const QString &defPath, QString &selectedName)
    {
        QString dirPath;
        const QDir patternDir = QFileInfo(patternPath).absoluteDir();
        QString measurements = table.fileName();
        if (patternDir.exists(measurements))
        {
            selectedName = measurements;
            dirPath = patternDir.absolutePath();
        }
        else if (patternDir.exists(measurements.replace(' '_L1, '_'_L1)))
        {
            selectedName = measurements.replace(' '_L1, '_'_L1);
            dirPath = patternDir.absolutePath();
        }
        else
        {
            dirPath = defPath;
        }
        return dirPath;
    };

    QString mPath;
    if (patternType == MeasurementsType::Multisize)
    {
        const QString filter =
            tr("Multisize measurements") + " (*.vst);;"_L1 + tr("Individual measurements") + " (*.vit)"_L1;
        // Use standard path to multisize measurements
        QString selectedName;
        const QString dirPath =
            DirPath(VAbstractValApplication::VApp()->ValentinaSettings()->GetPathMultisizeMeasurements(), selectedName);
        mPath = FindLocation(filter, dirPath, selectedName);
        if (!mPath.isEmpty())
        {
            VAbstractValApplication::VApp()->ValentinaSettings()->SetPathMultisizeMeasurements(mPath);
        }
    }
    else
    {
        const QString filter =
            tr("Individual measurements") + " (*.vit);;"_L1 + tr("Multisize measurements") + " (*.vst)"_L1;
        // Use standard path to individual measurements
        QString selectedName;
        const QString dirPath = DirPath(
            VAbstractValApplication::VApp()->ValentinaSettings()->GetPathIndividualMeasurements(), selectedName);
        mPath = FindLocation(filter, dirPath, selectedName);
        if (!mPath.isEmpty())
        {
            VAbstractValApplication::VApp()->ValentinaSettings()->SetPathIndividualMeasurements(mPath);
        }
    }

    if (mPath.isEmpty())
    {
        return mPath;
    }

    QScopedPointer<VMeasurements> m(new VMeasurements(pattern));
    m->setXMLContent(mPath);

    patternType = m->Type();

    if (patternType == MeasurementsType::Unknown)
    {
        throw VException(tr("Measurement file has unknown format."));
    }

    if (patternType == MeasurementsType::Multisize)
    {
        VVSTConverter converter(mPath);
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
        VVITConverter converter(mPath);
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
        throw VException(tr("Measurement file contains invalid known measurement(s)."));
    }

    CheckRequiredMeasurements(m.data());

    VAbstractValApplication::VApp()->SetMeasurementsType(patternType);
    doc->SetMPath(RelativeMPath(patternPath, mPath));
    return mPath;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ChangePP(int index, bool zoomBestFit)
{
    if (index != -1)
    {
        doc->ChangeActivPP(m_comboBoxDraws->itemText(index));
        doc->setCurrentData();
        emit RefreshHistory();
        if (m_drawMode)
        {
            ArrowTool(true);
            if (zoomBestFit)
            {
                ZoomFitBestCurrent();
            }
        }
        m_toolOptions->itemClicked(nullptr); // hide options for tool in previous pattern piece
        m_groupsWidget->UpdateGroups();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::EndVisualization(bool click)
{
    if (not m_dialogTool.isNull())
    {
        m_dialogTool->ShowDialog(click);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ZoomFirstShow Fit scene size to best size for first show
 */
void MainWindow::ZoomFirstShow()
{
    /* If don't call ZoomFitBest() twice, after first scaling or moving pattern piece, scene change coordinate and whole
     * pattern will be moved. Looks very ugly. It is best solution that i have now.
     */

    if (not pattern->DataPieces()->empty())
    {
        ActionDetails(true);
        ui->view->ZoomFitBest();
    }

    if (not ui->actionDraw->isChecked())
    {
        ActionDraw(true);
    }
    ZoomFitBestCurrent();

    VMainGraphicsView::NewSceneRect(m_sceneDraw, ui->view);
    VMainGraphicsView::NewSceneRect(m_sceneDetails, ui->view);

    if (not pattern->DataPieces()->empty())
    {
        ActionDetails(true);
        ui->view->ZoomFitBest();
    }

    if (not ui->actionDraw->isChecked())
    {
        ActionDraw(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::DoExport(const VCommandLinePtr &expParams) -> bool
{
    QVector<DetailForLayout> details;
    if (not VAbstractValApplication::VApp()->getOpeningPattern())
    {
        const QHash<quint32, VPiece> *allDetails = pattern->DataPieces();
        if (allDetails->count() == 0)
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("You can't export empty scene.")));
            QCoreApplication::exit(V_EX_DATAERR);
            return false;
        }

        details = SortDetailsForLayout(allDetails, expParams->OptExportSuchDetails());

        if (details.count() == 0)
        {
            qCCritical(vMainWindow, "%s",
                       qUtf8Printable(tr("You can't export empty scene. Please, "
                                         "include at least one detail in layout.")));
            QCoreApplication::exit(V_EX_DATAERR);
            return false;
        }
    }
    listDetails = PrepareDetailsForLayout(details);

    const bool exportOnlyDetails = expParams->IsExportOnlyDetails();
    if (exportOnlyDetails)
    {
        try
        {
            m_dialogSaveLayout = QSharedPointer<DialogSaveLayout>(
                new DialogSaveLayout(1, Draw::Modeling, expParams->OptBaseName(), this));
            m_dialogSaveLayout->SetDestinationPath(expParams->OptDestinationPath());
            m_dialogSaveLayout->SelectFormat(static_cast<LayoutExportFormats>(expParams->OptExportType()));
            m_dialogSaveLayout->SetBinaryDXFFormat(expParams->IsBinaryDXF());
            m_dialogSaveLayout->SetShowGrainline(!expParams->IsNoGrainline());
            m_dialogSaveLayout->SetTextAsPaths(expParams->IsTextAsPaths());
            m_dialogSaveLayout->SetXScale(expParams->ExportXScale());
            m_dialogSaveLayout->SetYScale(expParams->ExportYScale());

            if (static_cast<LayoutExportFormats>(expParams->OptExportType()) == LayoutExportFormats::PDFTiled)
            {
                m_dialogSaveLayout->SetTiledExportMode(true);
                m_dialogSaveLayout->SetTiledMargins(expParams->TiledPageMargins());
                m_dialogSaveLayout->SetTiledPageFormat(expParams->OptTiledPaperSize());
                m_dialogSaveLayout->SetTiledPageOrientation(expParams->OptTiledPageOrientation());
            }

            ExportData(listDetails);
            m_dialogSaveLayout.clear();
        }
        catch (const VException &e)
        {
            m_dialogSaveLayout.clear();
            qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export error.")), qUtf8Printable(e.ErrorMessage()));
            QCoreApplication::exit(V_EX_DATAERR);
            return false;
        }
    }
    else
    {
        auto settings = expParams->DefaultGenerator();
        settings->SetTextAsPaths(expParams->IsTextAsPaths());

        if (GenerateLayout(*settings.get()))
        {
            try
            {
                m_dialogSaveLayout = QSharedPointer<DialogSaveLayout>(
                    new DialogSaveLayout(static_cast<int>(m_layoutSettings->LayoutScenes().size()), Draw::Layout,
                                         expParams->OptBaseName(), this));
                m_dialogSaveLayout->SetDestinationPath(expParams->OptDestinationPath());
                m_dialogSaveLayout->SelectFormat(static_cast<LayoutExportFormats>(expParams->OptExportType()));
                m_dialogSaveLayout->SetBinaryDXFFormat(expParams->IsBinaryDXF());
                m_dialogSaveLayout->SetShowGrainline(!expParams->IsNoGrainline());
                m_dialogSaveLayout->SetXScale(expParams->ExportXScale());
                m_dialogSaveLayout->SetYScale(expParams->ExportYScale());

                if (static_cast<LayoutExportFormats>(expParams->OptExportType()) == LayoutExportFormats::PDFTiled)
                {
                    m_dialogSaveLayout->SetTiledExportMode(true);
                    m_dialogSaveLayout->SetTiledMargins(expParams->TiledPageMargins());
                    m_dialogSaveLayout->SetTiledPageFormat(expParams->OptTiledPaperSize());
                    m_dialogSaveLayout->SetTiledPageOrientation(expParams->OptTiledPageOrientation());
                }

                ExportData(listDetails);
                m_dialogSaveLayout.clear();
            }
            catch (const VException &e)
            {
                m_dialogSaveLayout.clear();
                qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export error.")),
                           qUtf8Printable(e.ErrorMessage()));
                QCoreApplication::exit(V_EX_DATAERR);
                return false;
            }
        }
        else
        {
            QCoreApplication::exit(V_EX_DATAERR);
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DoFMExport process export final measurements
 * @param expParams command line options
 * @return true if succesfull
 */
auto MainWindow::DoFMExport(const VCommandLinePtr &expParams) -> bool
{
    QString filePath = expParams->OptExportFMTo();

    if (filePath.isEmpty())
    {
        qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export final measurements error.")),
                   qUtf8Printable(tr("Destination path is empty.")));
        QCoreApplication::exit(V_EX_DATAERR);
        return false;
    }

    QFileInfo info(filePath);
    if (info.isRelative())
    {
        filePath = QDir::currentPath() + '/'_L1 + filePath;
    }

    const QString codecName = expParams->OptCSVCodecName();
    int mib = VTextCodec::codecForLocale()->mibEnum();
    if (not codecName.isEmpty())
    {
        if (VTextCodec *codec = VTextCodec::codecForName(codecName.toLatin1()))
        {
            mib = codec->mibEnum();
        }
    }

    QChar separator = expParams->OptCSVSeparator();
    if (separator.isNull())
    {
        separator = VCommonSettings::GetDefCSVSeparator();
    }

    return ExportFMeasurementsToCSVData(filePath, expParams->IsCSVWithHeader(), mib, separator);
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::SetDimensionA(int value) -> bool
{
    if (not VApplication::IsGUIMode())
    {
        if (this->isWindowModified() || not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
        {
            if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
            {
                const qint32 index = m_dimensionA->findData(value);
                if (index != -1)
                {
                    m_dimensionA->setCurrentIndex(index);
                }
                else
                {
                    qCCritical(
                        vMainWindow, "%s",
                        qUtf8Printable(tr("Not supported dimension A value '%1' for this pattern file.").arg(value)));
                    return false;
                }
            }
            else
            {
                qCCritical(vMainWindow, "%s",
                           qUtf8Printable(tr("Couldn't set dimension A. Need a file with multisize measurements.")));
                return false;
            }
        }
        else
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set dimension A. File wasn't opened.")));
            return false;
        }
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::SetDimensionB(int value) -> bool
{
    if (not VApplication::IsGUIMode())
    {
        if (this->isWindowModified() || not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
        {
            if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
            {
                const qint32 index = m_dimensionB->findData(value);
                if (index != -1)
                {
                    m_dimensionB->setCurrentIndex(index);
                }
                else
                {
                    qCCritical(
                        vMainWindow, "%s",
                        qUtf8Printable(tr("Not supported dimension B value '%1' for this pattern file.").arg(value)));
                    return false;
                }
            }
            else
            {
                qCCritical(vMainWindow, "%s",
                           qUtf8Printable(tr("Couldn't set dimension B. Need a file with multisize measurements.")));
                return false;
            }
        }
        else
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set dimension B. File wasn't opened.")));
            return false;
        }
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::SetDimensionC(int value) -> bool
{
    if (not VApplication::IsGUIMode())
    {
        if (this->isWindowModified() || not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
        {
            if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize)
            {
                const qint32 index = m_dimensionC->findData(value);
                if (index != -1)
                {
                    m_dimensionC->setCurrentIndex(index);
                }
                else
                {
                    qCCritical(
                        vMainWindow, "%s",
                        qUtf8Printable(tr("Not supported dimension C value '%1' for this pattern file.").arg(value)));
                    return false;
                }
            }
            else
            {
                qCCritical(vMainWindow, "%s",
                           qUtf8Printable(tr("Couldn't set dimension C. Need a file with multisize measurements.")));
                return false;
            }
        }
        else
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set dimension C. File wasn't opened.")));
            return false;
        }
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ProcessCMD()
{
    const VCommandLinePtr cmd = VApplication::CommandLine();
    auto args = cmd->OptInputFileNames();

    isNoScaling = cmd->IsNoScalingEnabled();

    if (VApplication::IsGUIMode())
    {
        ReopenFilesAfterCrash(args);

        for (auto &arg : args)
        {
            LoadPattern(arg);
        }
    }
    else
    {
        if (args.size() != 1)
        {
            qCritical() << tr("Please, provide one input file.");
            QCoreApplication::exit(V_EX_NOINPUT);
            return;
        }

        VAbstractValApplication::VApp()->SetUserMaterials(cmd->OptUserMaterials());

        const bool loaded = LoadPattern(args.constFirst(), cmd->OptMeasurePath());

        if (not loaded)
        {
            return; // process only one input file
        }

        bool aSetted = true;
        bool bSetted = true;
        bool cSetted = true;
        if (cmd->IsSetDimensionA())
        {
            aSetted = SetDimensionA(cmd->OptDimensionA());
        }

        if (cmd->IsSetDimensionB())
        {
            bSetted = SetDimensionB(cmd->OptDimensionB());
        }

        if (cmd->IsSetDimensionC())
        {
            cSetted = SetDimensionB(cmd->OptDimensionC());
        }

        if (not(aSetted && bSetted && cSetted))
        {
            QCoreApplication::exit(V_EX_DATAERR);
            return;
        }

        if (not cmd->IsTestModeEnabled())
        {
            if (cmd->IsExportEnabled() && not DoExport(cmd))
            {
                return;
            }

            if (cmd->IsExportFMEnabled() && not DoFMExport(cmd))
            {
                return;
            }
        }

        QCoreApplication::exit(V_EX_OK); // close program after processing in console mode
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::GetPatternFileName() -> QString
{
    QString shownName = tr("untitled.val");
    if (not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
    {
        shownName = StrippedName(VAbstractValApplication::VApp()->GetPatternPath());
    }
    shownName += "[*]"_L1;
    return shownName;
}

//---------------------------------------------------------------------------------------------------------------------
auto MainWindow::GetMeasurementFileName() -> QString
{
    if (doc->MPath().isEmpty())
    {
        return {};
    }

    QString shownName = QStringLiteral(" [");
    shownName += StrippedName(AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), doc->MPath()));

    if (m_mChanges)
    {
        shownName += '*'_L1;
    }

    shownName += ']'_L1;
    return shownName;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateWindowTitle()
{
    bool isFileWritable = true;
    if (not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
    {
        // #ifdef Q_OS_WIN32
        //         qt_ntfs_permission_lookup++; // turn checking on
        // #endif /*Q_OS_WIN32*/
        isFileWritable = QFileInfo(VAbstractValApplication::VApp()->GetPatternPath()).isWritable();
        // #ifdef Q_OS_WIN32
        //         qt_ntfs_permission_lookup--; // turn it off again
        // #endif /*Q_OS_WIN32*/
    }

    if (not m_patternReadOnly && isFileWritable)
    {
        setWindowTitle(GetPatternFileName() + GetMeasurementFileName());
    }
    else
    {
        setWindowTitle(GetPatternFileName() + GetMeasurementFileName() + " ("_L1 + tr("read only") + ')'_L1);
    }
    setWindowFilePath(VAbstractValApplication::VApp()->GetPatternPath());

#if defined(Q_OS_MAC)
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() + "/../Resources/Valentina.icns"_L1);
    QIcon icon;
    if (not VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
    {
        if (not isWindowModified())
        {
            icon = fileIcon;
        }
        else
        {
            static QIcon darkIcon;

            if (darkIcon.isNull())
            {
                darkIcon = QIcon(darkenPixmap(fileIcon.pixmap(16, 16)));
            }
            icon = darkIcon;
        }
    }
    setWindowIcon(icon);
#endif // defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPoint()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);
    emit EnableImageBackgroundHover(false);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPointByRelease()
{
    ToolSelectPoint();
    emit ItemsSelection(SelectionType::ByMouseRelease);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPointByPress()
{
    ToolSelectPoint();
    emit ItemsSelection(SelectionType::ByMousePress);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectSpline()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(false);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectSplinePath()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(true);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectArc()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPointArc()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectCurve()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectAllDrawObjects()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectOperationObjects()
{
    // Only true for rubber band selection
    emit EnableLabelSelection(true);
    emit EnablePointSelection(true);
    emit EnableLineSelection(false);
    emit EnableArcSelection(true);
    emit EnableElArcSelection(true);
    emit EnableSplineSelection(true);
    emit EnableSplinePathSelection(true);
    emit EnableBackgroundImageSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);
    emit EnableImageBackgroundHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectGroupObjects()
{
    ToolSelectOperationObjects();
    // Only true for rubber band selection
    emit EnableLineSelection(true);

    // Hovering
    emit EnableLineHover(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectDetail()
{
    // Only true for rubber band selection
    emit EnableNodeLabelSelection(false);
    emit EnableNodePointSelection(false);
    emit EnableDetailSelection(true); // Disable when done visualization details

    // Hovering
    emit EnableNodeLabelHover(true);
    emit EnableNodePointHover(true);
    emit EnableDetailHover(true);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->AllowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::PrintPatternMessage(QEvent *event)
{
    SCASSERT(event != nullptr)
    auto *patternMessage =
        static_cast<WarningMessageEvent *>(event); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

    QString severity;

    switch (patternMessage->Severity())
    {
        case QtDebugMsg:
            severity = tr("DEBUG");
            break;
        case QtWarningMsg:
            severity = tr("WARNING");
            break;
        case QtCriticalMsg:
            severity = tr("CRITICAL");
            break;
        case QtFatalMsg:
            severity = tr("FATAL");
            break;
        case QtInfoMsg:
            severity = tr("INFO");
            break;
        default:
            break;
    }

    const QString time = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"));
    const QString message = QStringLiteral("%1: [%2] %3").arg(time, severity, patternMessage->Message());
    ui->plainTextEditPatternMessages->appendPlainText(message);
    if (not m_unreadPatternMessage.isNull())
    {
        m_unreadPatternMessage->setText(DialogWarningIcon() + tr("Pattern messages"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenWatermark(const QString &path)
{
    for (const auto &watermarkEditor : m_watermarkEditors)
    {
        if (not watermarkEditor.isNull() && not watermarkEditor->CurrentFile().isEmpty() &&
            watermarkEditor->CurrentFile() == AbsoluteMPath(VAbstractValApplication::VApp()->GetPatternPath(), path))
        {
            watermarkEditor->show();
            return;
        }
    }

    auto *watermark = new WatermarkWindow(VAbstractValApplication::VApp()->GetPatternPath(), this);
    connect(watermark, &WatermarkWindow::New, this, [this]() { OpenWatermark(); });
    connect(watermark, &WatermarkWindow::OpenAnother, this, [this](const QString &path) { OpenWatermark(path); });
    m_watermarkEditors.append(watermark);
    watermark->show();
    watermark->Open(path);
}
