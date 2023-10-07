/************************************************************************
 **
 **  @file   vpmainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vpmainwindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QLoggingCategory>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinterInfo>
#include <QSaveFile>
#include <QSvgGenerator>
#include <QTimer>
#include <QUndoStack>
#include <QtMath>
#include <chrono>
#include <thread>

#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vlayoutconverter.h"
#include "../vdxf/libdxfrw/drw_base.h"
#include "../vganalytics/vganalytics.h"
#include "../vlayout/dialogs/watermarkwindow.h"
#include "../vlayout/vlayoutexporter.h"
#include "../vlayout/vprintlayout.h"
#include "../vlayout/vrawlayout.h"
#include "../vmisc/dialogs/dialogaskcollectstatistic.h"
#include "../vmisc/dialogs/dialogselectlanguage.h"
#include "../vmisc/lambdaconstants.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vsysexits.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "dialogs/dialogpuzzlepreferences.h"
#include "dialogs/dialogsavemanuallayout.h"
#include "dialogs/vpdialogabout.h"
#include "layout/vppiece.h"
#include "layout/vpsheet.h"
#include "ui_vpmainwindow.h"
#include "undocommands/vpundoaddsheet.h"
#include "undocommands/vpundopiecemove.h"
#include "undocommands/vpundopiecerotate.h"
#include "undocommands/vpundopiecezvaluemove.h"
#include "vpapplication.h"
#include "vptilefactory.h"
#include "xml/vplayoutfilereader.h"
#include "xml/vplayoutfilewriter.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pWindow, "p.window") // NOLINT

QT_WARNING_POP

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
void SetDoubleSpinBoxValue(QDoubleSpinBox *spinBox, qreal value)
{
    spinBox->blockSignals(true);
    spinBox->setValue(value);
    spinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void SetCheckBoxValue(QCheckBox *checkbox, bool value)
{
    checkbox->blockSignals(true);
    checkbox->setChecked(value);
    checkbox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void SetLineEditValue(QLineEdit *lineEdit, const QString &value)
{
    lineEdit->blockSignals(true);
    lineEdit->setText(value);
    lineEdit->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void SetPlainTextEditValue(QPlainTextEdit *textEdit, const QString &value)
{
    textEdit->blockSignals(true);
    textEdit->setPlainText(value);
    textEdit->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
auto PiecesBoundingRect(const QList<VPPiecePtr> &selectedPieces) -> QRectF
{
    QRectF rect;
    for (const auto &item : selectedPieces)
    {
        if (not item.isNull())
        {
            rect = rect.united(item->MappedDetailBoundingRect());
        }
    }

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
Q_REQUIRED_RESULT auto PreparePrinter(const QPrinterInfo &info, QPrinter::PrinterMode mode = QPrinter::ScreenResolution)
    -> QSharedPointer<QPrinter>;
auto PreparePrinter(const QPrinterInfo &info, QPrinter::PrinterMode mode) -> QSharedPointer<QPrinter>
{
    QPrinterInfo tmpInfo = info;
    if (tmpInfo.isNull() || tmpInfo.printerName().isEmpty())
    {
        const QStringList list = QPrinterInfo::availablePrinterNames();
        if (list.isEmpty())
        {
            return {};
        }

        tmpInfo = QPrinterInfo::printerInfo(list.constFirst());
    }

    auto printer = QSharedPointer<QPrinter>(new QPrinter(tmpInfo, mode));
    printer->setResolution(static_cast<int>(PrintDPI));
    return printer;
}

//---------------------------------------------------------------------------------------------------------------------
void SetPrinterSheetPageSettings(const QSharedPointer<QPrinter> &printer, const VPSheetPtr &sheet, qreal xScale,
                                 qreal yScale)
{
    SCASSERT(not printer.isNull())

    QMarginsF margins;
    if (not sheet->IgnoreMargins())
    {
        margins = sheet->GetSheetMargins();
    }

    QPageLayout::Orientation sheetOrientation = sheet->GetSheetOrientation();

    QRectF imageRect = sheet->GetMarginsRect();
    qreal width = FromPixel(imageRect.width() * xScale + margins.left() + margins.right(), Unit::Mm);
    qreal height = FromPixel(imageRect.height() * yScale + margins.top() + margins.bottom(), Unit::Mm);

    QSizeF pageSize = sheetOrientation == QPageLayout::Portrait ? QSizeF(width, height) : QSizeF(height, width);
    if (not printer->setPageSize(QPageSize(pageSize, QPageSize::Millimeter)))
    {
        qWarning() << QObject::tr("Cannot set printer page size");
    }

    printer->setPageOrientation(sheetOrientation);
    printer->setFullPage(sheet->IgnoreMargins());

    if (not sheet->IgnoreMargins())
    {
        if (not printer->setPageMargins(UnitConvertor(margins, Unit::Px, Unit::Mm), QPageLayout::Millimeter))
        {
            qWarning() << QObject::tr("Cannot set printer margins");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SetPrinterTiledPageSettings(const QSharedPointer<QPrinter> &printer, const VPLayoutPtr &layout,
                                 const VPSheetPtr &sheet, QPageLayout::Orientation orientation, bool forSheet)
{
    SCASSERT(not printer.isNull())

    if (layout.isNull() || sheet.isNull())
    {
        return;
    }

    QSizeF tileSize = layout->LayoutSettings().GetTilesSize(Unit::Mm);
    QSizeF pageSize;

    if (not forSheet)
    {
        pageSize = orientation == QPageLayout::Portrait ? tileSize : tileSize.transposed();
    }
    else
    {
        QPageLayout::Orientation tileOrientation = layout->LayoutSettings().GetTilesOrientation();
        QPageLayout::Orientation sheetOrientation = sheet->GetSheetOrientation();

        if (tileOrientation != sheetOrientation)
        {
            pageSize = orientation == QPageLayout::Portrait ? tileSize.transposed() : tileSize;
        }
        else
        {
            pageSize = orientation == QPageLayout::Portrait ? tileSize : tileSize.transposed();
        }
    }

    if (not printer->setPageSize(QPageSize(pageSize, QPageSize::Millimeter)))
    {
        qWarning() << QObject::tr("Cannot set printer page size");
    }

    printer->setPageOrientation(orientation);
    printer->setFullPage(layout->LayoutSettings().IgnoreTilesMargins());

    if (not layout->LayoutSettings().IgnoreTilesMargins())
    {
        if (not printer->setPageMargins(layout->LayoutSettings().GetTilesMargins(Unit::Mm), QPageLayout::Millimeter))
        {
            qWarning() << QObject::tr("Cannot set printer margins");
        }
    }
}
} // namespace

struct VPExportData
{
    LayoutExportFormats format{LayoutExportFormats::SVG};
    QList<VPSheetPtr> sheets{};
    QString path{};
    QString fileName{};
    qreal xScale{1.};
    qreal yScale{1.};
    bool isBinaryDXF{false};
    bool textAsPaths{false};
    bool exportUnified{true};
    bool showTilesScheme{false};
    bool showGrainline{true};
};

//---------------------------------------------------------------------------------------------------------------------
VPMainWindow::VPMainWindow(const VPCommandLinePtr &cmd, QWidget *parent)
  : VAbstractMainWindow(parent),
    ui(new Ui::VPMainWindow),
    m_cmd(cmd),
    m_undoStack(new QUndoStack(this)),
    m_layout{VPLayout::CreateLayout(m_undoStack)},
    m_statusLabel(new QLabel(this)),
    m_layoutWatcher(new QFileSystemWatcher(this)),
    m_watermarkWatcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);

    connect(m_layout.data(), &VPLayout::PieceSelectionChanged, this, &VPMainWindow::on_PieceSelectionChanged);
    connect(m_layout.data(), &VPLayout::LayoutChanged, this, [this]() { LayoutWasSaved(false); });
    connect(m_layout.data(), &VPLayout::PieceTransformationChanged, this,
            [this]() { SetPropertyTabCurrentPieceData(); });
    connect(m_layout.data(), &VPLayout::ActiveSheetChanged, this,
            [this]()
            {
                m_layout->TileFactory()->RefreshTileInfos();
                m_graphicsView->RefreshLayout();
                SetPropertyTabSheetData();
            });

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) { LayoutWasSaved(clean); });

    // init status bar
    statusBar()->addPermanentWidget(m_statusLabel, 1);

    SetupMenu();
    InitProperties();
    InitCarrousel();

    InitMainGraphics();

    InitZoomToolBar();
    InitScaleToolBar();

    SetPropertiesData();

    ReadSettings();

#if defined(Q_OS_MAC)
    // Mac OS Dock Menu
    QMenu *menu = new QMenu(this);
    connect(menu, &QMenu::aboutToShow, this, &VPMainWindow::AboutToShowDockMenu);
    AboutToShowDockMenu();
    menu->setAsDockMenu();
#endif // defined(Q_OS_MAC)

    connect(m_layoutWatcher, &QFileSystemWatcher::fileChanged, this,
            [this](const QString &path)
            {
                QFileInfo checkFile(path);
                if (not checkFile.exists())
                {
                    for (int i = 0; i <= 1000; i = i + 10)
                    {
                        if (checkFile.exists())
                        {
                            break;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }

                if (not curFile.isEmpty() && curFile == path)
                {
                    UpdateWindowTitle();
                }

                m_layout->TileFactory()->RefreshTileInfos();

                if (path == m_layout->LayoutSettings().WatermarkPath())
                {
                    m_layoutWatcher->blockSignals(true);
                    m_layout->TileFactory()->RefreshWatermarkData();
                    m_layoutWatcher->blockSignals(false);
                }

                m_graphicsView->RefreshLayout();

                if (checkFile.exists())
                {
                    m_layoutWatcher->addPath(path);
                }
            });

    m_graphicsView->RefreshLayout();

    if (m_cmd->IsGuiEnabled())
    {
        QTimer::singleShot(V_SECONDS(1), this, &VPMainWindow::AskDefaultSettings);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPMainWindow::~VPMainWindow()
{
    delete m_undoStack;
    delete ui;
    delete m_carrousel;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::CurrentFile() const -> QString
{
    return curFile;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::LoadFile(const QString &path) -> bool
{
    if (not QFileInfo::exists(path))
    {
        qCCritical(pWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
        return false;
    }

    // Check if file already opened
    QList<VPMainWindow *> list = VPApplication::VApp()->MainWindows();
    auto w =
        std::find_if(list.begin(), list.end(), [path](VPMainWindow *window) { return window->CurrentFile() == path; });
    if (w != list.end())
    {
        (*w)->activateWindow();
        if (this != *w)
        {
            close();
            return false;
        }
        return true;
    }

    VlpCreateLock(lock, path);

    if (not lock->IsLocked())
    {
        if (not IgnoreLocking(lock->GetLockError(), path, m_cmd->IsGuiEnabled()))
        {
            return false;
        }
    }

    try
    {
        VLayoutConverter converter(path);
        m_curFileFormatVersion = converter.GetCurrentFormatVersion();
        m_curFileFormatVersionStr = converter.GetFormatVersionStr();

        QFile file(converter.Convert());
        file.open(QIODevice::ReadOnly);

        VPLayoutFileReader fileReader;
        m_layout->Clear();

        fileReader.ReadFile(m_layout, &file);

        if (fileReader.hasError())
        {
            qCCritical(pWindow, "%s\n\n%s", qUtf8Printable(tr("File error.")),
                       qUtf8Printable(tr("Unable to read a layout file. %1").arg(fileReader.errorString())));
            lock.reset();
            return false;
        }

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
            statistic->SendLayoutFormatVersion(uptime, converter.GetFormatVersionStr());
        }
    }
    catch (VException &e)
    {
        qCCritical(pWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")), qUtf8Printable(e.ErrorMessage()),
                   qUtf8Printable(e.DetailedInformation()));
        lock.reset();
        return false;
    }

    SetCurrentFile(path);

    m_layout->SetFocusedSheet();

    m_oldLayoutUnit = m_layout->LayoutSettings().GetUnit();

    // updates the properties with the loaded data
    SetPropertiesData();

    m_carrousel->Refresh();
    m_graphicsView->on_ActiveSheetChanged(m_layout->GetFocusedSheet());
    m_layout->TileFactory()->RefreshTileInfos();
    m_layout->TileFactory()->RefreshWatermarkData();
    m_layout->CheckPiecesPositionValidity();

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);

    ui->actionRemoveWatermark->setEnabled(not m_layout->LayoutSettings().WatermarkPath().isEmpty());
    ui->actionEditCurrentWatermark->setEnabled(not m_layout->LayoutSettings().WatermarkPath().isEmpty());

    if (not m_layout->LayoutSettings().WatermarkPath().isEmpty())
    {
        m_layoutWatcher->addPath(m_layout->LayoutSettings().WatermarkPath());
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::LayoutWasSaved(bool saved)
{
    setWindowModified(!saved);
    not IsLayoutReadOnly() ? ui->actionSave->setEnabled(!saved) : ui->actionSave->setEnabled(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetCurrentFile(const QString &fileName)
{
    if (not curFile.isEmpty() && m_layoutWatcher->files().contains(curFile))
    {
        m_layoutWatcher->removePath(curFile);
    }

    curFile = fileName;
    if (not curFile.isEmpty())
    {
        if (not m_layoutWatcher->files().contains(curFile))
        {
            m_layoutWatcher->addPath(curFile);
        }
        auto *settings = VPApplication::VApp()->PuzzleSettings();
        QStringList files = settings->GetRecentFileList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
        {
            files.removeLast();
        }
        settings->SetRecentFileList(files);
        UpdateRecentFileActions();
    }

    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::SaveLayout(const QString &path, QString &error) -> bool
{
    bool success = false;
    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly))
    {
        VPLayoutFileWriter fileWriter;
        fileWriter.WriteFile(m_layout, &file);

        if (fileWriter.hasError())
        {
            error = tr("Fail to create layout.");
            return false;
        }

        success = file.commit();
    }

    if (success)
    {
        SetCurrentFile(path);
        LayoutWasSaved(true);
    }
    else
    {
        error = file.errorString();
    }
    return success;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ImportRawLayouts(const QStringList &rawLayouts)
{

    for (const auto &path : rawLayouts)
    {
        if (not ImportRawLayout(path))
        {
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitZoom()
{
    if (m_graphicsView != nullptr)
    {
        m_graphicsView->ZoomFitBest();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetupMenu()
{
    // most of the actions are connected through name convention (auto-connection)
    // File
    // -------------------- connects the actions for the file menu
    ui->actionNew->setShortcuts(QKeySequence::New);
    ui->actionSave->setShortcuts(QKeySequence::Save);
    ui->actionSaveAs->setShortcuts(QKeySequence::SaveAs);

    m_recentFileActs.fill(nullptr);
    for (auto &recentFileAct : m_recentFileActs)
    {
        auto *action = new QAction(this);
        recentFileAct = action;
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    if (auto *senderAction = qobject_cast<QAction *>(sender()))
                    {
                        const QString filePath = senderAction->data().toString();
                        if (not filePath.isEmpty())
                        {
                            LoadFile(filePath);
                        }
                    }
                });
        ui->menuFile->insertAction(ui->actionPreferences, recentFileAct);
        recentFileAct->setVisible(false);
    }

    m_separatorAct = new QAction(this);
    m_separatorAct->setSeparator(true);
    m_separatorAct->setVisible(false);
    ui->menuFile->insertAction(ui->actionPreferences, m_separatorAct);

    // Actions for recent files loaded by a puzzle window application.
    UpdateRecentFileActions();

    connect(ui->actionExit, &QAction::triggered, this, &VPMainWindow::close);
    ui->actionExit->setShortcuts(QKeySequence::Quit);

    // Layout
    connect(ui->actionExportLayout, &QAction::triggered, this, &VPMainWindow::on_ExportLayout);

    // Sheet
    connect(ui->actionExportSheet, &QAction::triggered, this, &VPMainWindow::on_ExportSheet);

    // Add dock properties action
    ui->menuSheet->addSeparator();
    QAction *actionDockWidgetToolOptions = ui->dockWidgetProperties->toggleViewAction();
    ui->menuSheet->addAction(actionDockWidgetToolOptions);
    ui->menuSheet->addSeparator();

    // Add Undo/Redo actions.
    undoAction = m_layout->UndoStack()->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo")));
    ui->menuSheet->addAction(undoAction);
    ui->toolBarUndoCommands->addAction(undoAction);

    redoAction = m_layout->UndoStack()->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-redo")));
    ui->menuSheet->addAction(redoAction);
    ui->toolBarUndoCommands->addAction(redoAction);

    // Z value
    connect(ui->actionZValueBottom, &QAction::triggered, this,
            [this]() { ZValueMove(static_cast<int>(ML::ZValueMove::Bottom)); });
    connect(ui->actionZValueDown, &QAction::triggered, this,
            [this]() { ZValueMove(static_cast<int>(ML::ZValueMove::Down)); });
    connect(ui->actionZValueUp, &QAction::triggered, this,
            [this]() { ZValueMove(static_cast<int>(ML::ZValueMove::Up)); });
    connect(ui->actionZValueTop, &QAction::triggered, this,
            [this]() { ZValueMove(static_cast<int>(ML::ZValueMove::Top)); });

    // Watermark
    connect(ui->actionWatermarkEditor, &QAction::triggered, this, &VPMainWindow::CreateWatermark);
    connect(ui->actionEditCurrentWatermark, &QAction::triggered, this, &VPMainWindow::EditCurrentWatermark);
    connect(ui->actionLoadWatermark, &QAction::triggered, this, &VPMainWindow::LoadWatermark);
    connect(ui->actionRemoveWatermark, &QAction::triggered, this, &VPMainWindow::RemoveWatermark);

    // Window
    connect(ui->menuWindow, &QMenu::aboutToShow, this,
            [this]()
            {
                ui->menuWindow->clear();
                CreateWindowMenu(ui->menuWindow);
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitProperties()
{
    ui->tabWidgetProperties->setCurrentIndex(0);

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    m_oldLayoutUnit = settings->LayoutUnit();

    InitPropertyTabCurrentPiece();
    InitPropertyTabCurrentSheet();
    InitPropertyTabTiles();
    InitPropertyTabLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabCurrentPiece()
{
    connect(ui->checkBoxCurrentPieceShowSeamline, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                QList<VPPiecePtr> selectedPieces = SelectedPieces();
                if (selectedPieces.size() == 1)
                {
                    const VPPiecePtr &selectedPiece = selectedPieces.constFirst();
                    if (not selectedPiece.isNull())
                    {
                        selectedPiece->SetHideMainPath(not checked);
                        LayoutWasSaved(false);
                        // nothing changed, but will force redraw
                        emit m_layout->PieceTransformationChanged(selectedPiece);
                    }
                }
            });

    connect(ui->checkBoxCurrentPieceMirrorPiece, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                QList<VPPiecePtr> selectedPieces = SelectedPieces();
                if (selectedPieces.size() == 1)
                {
                    const VPPiecePtr &selectedPiece = selectedPieces.constFirst();
                    if (not selectedPiece.isNull())
                    {
                        if (selectedPiece->IsMirror() != checked)
                        {
                            selectedPiece->Flip();
                            LayoutWasSaved(false);
                            emit m_layout->PieceTransformationChanged(selectedPiece);
                        }
                    }
                }
            });

    // Translate
    ui->comboBoxTranslateUnit->addItem(tr("Millimiters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxTranslateUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxTranslateUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));
    ui->comboBoxTranslateUnit->addItem(tr("Pixels"), QVariant(UnitsToStr(Unit::Px)));

    m_oldPieceTranslationUnit = Unit::Mm;
    ui->comboBoxTranslateUnit->blockSignals(true);
    ui->comboBoxTranslateUnit->setCurrentIndex(0);
    ui->comboBoxTranslateUnit->blockSignals(false);

    const int minTranslate = -1000;
    const int maxTranslate = 1000;

    ui->doubleSpinBoxCurrentPieceBoxPositionX->setMinimum(
        UnitConvertor(minTranslate, Unit::Cm, m_oldPieceTranslationUnit));
    ui->doubleSpinBoxCurrentPieceBoxPositionX->setMaximum(
        UnitConvertor(maxTranslate, Unit::Cm, m_oldPieceTranslationUnit));
    ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(0);

    ui->doubleSpinBoxCurrentPieceBoxPositionY->setMinimum(
        UnitConvertor(minTranslate, Unit::Cm, m_oldPieceTranslationUnit));
    ui->doubleSpinBoxCurrentPieceBoxPositionY->setMaximum(
        UnitConvertor(maxTranslate, Unit::Cm, m_oldPieceTranslationUnit));
    ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(0);

    connect(ui->comboBoxTranslateUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this V_LAMBDA_CONSTANTS(minTranslate, maxTranslate)]()
            {
                const Unit newUnit = TranslateUnit();
                const qreal oldTranslateX = ui->doubleSpinBoxCurrentPieceBoxPositionX->value();
                const qreal oldTranslateY = ui->doubleSpinBoxCurrentPieceBoxPositionY->value();

                ui->doubleSpinBoxCurrentPieceBoxPositionX->setMinimum(UnitConvertor(minTranslate, Unit::Cm, newUnit));
                ui->doubleSpinBoxCurrentPieceBoxPositionX->setMaximum(UnitConvertor(maxTranslate, Unit::Cm, newUnit));

                ui->doubleSpinBoxCurrentPieceBoxPositionY->setMinimum(UnitConvertor(minTranslate, Unit::Cm, newUnit));
                ui->doubleSpinBoxCurrentPieceBoxPositionY->setMaximum(UnitConvertor(maxTranslate, Unit::Cm, newUnit));

                ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(
                    UnitConvertor(oldTranslateX, m_oldPieceTranslationUnit, newUnit));
                ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(
                    UnitConvertor(oldTranslateY, m_oldPieceTranslationUnit, newUnit));

                m_oldPieceTranslationUnit = newUnit;
            });

    SetCheckBoxValue(ui->checkBoxRelativeTranslation, true);
    connect(ui->checkBoxRelativeTranslation, &QCheckBox::toggled, this, &VPMainWindow::on_RelativeTranslationChanged);

    // Rotate
    ui->doubleSpinBoxCurrentPieceAngle->setValue(0);

    ui->toolButtonCurrentPieceRotationAnticlockwise->setChecked(true);
    ui->checkBoxTransformSeparately->setChecked(false);

    QPushButton *bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(bApply != nullptr)
    connect(bApply, &QPushButton::clicked, this, &VPMainWindow::on_ApplyPieceTransformation);

    QPushButton *bReset = ui->buttonBox->button(QDialogButtonBox::Reset);
    SCASSERT(bReset != nullptr)
    connect(bReset, &QPushButton::clicked, this, &VPMainWindow::on_ResetPieceTransformationSettings);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabCurrentSheet()
{
    connect(ui->lineEditSheetName, &QLineEdit::textEdited, this,
            [this](const QString &text)
            {
                if (not m_layout.isNull())
                {
                    VPSheetPtr sheet = m_layout->GetFocusedSheet();
                    if (not sheet.isNull())
                    {
                        sheet->SetName(text);
                        LayoutWasSaved(false);

                        if (m_carrousel != nullptr)
                        {
                            m_carrousel->RefreshSheetNames();
                        }
                    }
                }
            });

    // -------------------- layout units ---------------------------
    ui->comboBoxLayoutUnit->addItem(tr("Millimiters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxLayoutUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxLayoutUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));
    ui->comboBoxLayoutUnit->addItem(tr("Pixels"), QVariant(UnitsToStr(Unit::Px)));

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    const qint32 indexUnit = ui->comboBoxLayoutUnit->findData(UnitsToStr(settings->LayoutUnit()));
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }

    connect(ui->comboBoxLayoutUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VPMainWindow::on_ConvertPaperSize);

    // -------------------- sheet template ---------------------------
    VAbstractLayoutDialog::InitTemplates(ui->comboBoxSheetTemplates);

    connect(ui->comboBoxSheetTemplates, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this] { SheetSize(SheetTemplate()); });

    const QString suffix = " " + UnitsToStr(LayoutUnit(), true);
    // -------------------- paper size ---------------------------
    InitPaperSizeData(suffix);

    // -------------------- margins  ------------------------
    InitMarginsData(suffix);

    ui->groupBoxSheetGrid->setVisible(false); // temporary hide

    connect(ui->pushButtonSheetExport, &QPushButton::clicked, this, &VPMainWindow::on_ExportSheet);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPaperSizeData(const QString &suffix)
{
    MinimumSheetPaperSize();

    ui->doubleSpinBoxSheetPaperWidth->setSuffix(suffix);
    ui->doubleSpinBoxSheetPaperHeight->setSuffix(suffix);

    connect(ui->doubleSpinBoxSheetPaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetSizeChanged);
    connect(ui->doubleSpinBoxSheetPaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetSizeChanged);
    connect(ui->toolButtonSheetPortraitOritation, &QToolButton::toggled, this,
            &VPMainWindow::on_SheetOrientationChanged);
    connect(ui->toolButtonSheetLandscapeOrientation, &QToolButton::toggled, this,
            &VPMainWindow::on_SheetOrientationChanged);

    connect(ui->toolButtonGrainlineHorizontalOrientation, &QToolButton::clicked, this,
            [this](bool checked)
            {
                VPSheetPtr sheet = m_layout->GetFocusedSheet();
                if (sheet.isNull())
                {
                    return;
                }

                if (checked)
                {
                    sheet->SetGrainlineType(GrainlineType::Horizontal);
                    ui->toolButtonGrainlineVerticalOrientation->setChecked(false);
                }
                else
                {
                    sheet->SetGrainlineType(GrainlineType::NotFixed);
                }

                RotatePiecesToGrainline();
                LayoutWasSaved(false);
            });

    connect(ui->toolButtonGrainlineVerticalOrientation, &QToolButton::clicked, this,
            [this](bool checked)
            {
                VPSheetPtr sheet = m_layout->GetFocusedSheet();
                if (sheet.isNull())
                {
                    return;
                }

                if (checked)
                {
                    sheet->SetGrainlineType(GrainlineType::Vertical);
                    ui->toolButtonGrainlineHorizontalOrientation->setChecked(false);
                }
                else
                {
                    sheet->SetGrainlineType(GrainlineType::NotFixed);
                }

                RotatePiecesToGrainline();
                LayoutWasSaved(false);
            });

    connect(ui->pushButtonSheetRemoveUnusedLength, &QPushButton::clicked, this,
            [this]()
            {
                if (not m_layout.isNull())
                {
                    VPSheetPtr sheet = m_layout->GetFocusedSheet();
                    if (not sheet.isNull())
                    {
                        sheet->RemoveUnusedLength();
                        LayoutWasSaved(false);
                        m_layout->TileFactory()->RefreshTileInfos();
                        m_graphicsView->RefreshLayout();
                        m_graphicsView->RefreshPieces();
                        SetPropertyTabSheetData();
                    }
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitMarginsData(const QString &suffix)
{
    ui->doubleSpinBoxSheetMarginLeft->setSuffix(suffix);
    ui->doubleSpinBoxSheetMarginRight->setSuffix(suffix);
    ui->doubleSpinBoxSheetMarginTop->setSuffix(suffix);
    ui->doubleSpinBoxSheetMarginBottom->setSuffix(suffix);

    connect(ui->doubleSpinBoxSheetMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);
    connect(ui->doubleSpinBoxSheetMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);
    connect(ui->doubleSpinBoxSheetMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);
    connect(ui->doubleSpinBoxSheetMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);

    connect(ui->checkBoxLayoutIgnoreFileds, &QCheckBox::stateChanged, this,
            [this](int state)
            {
                if (not m_layout.isNull())
                {
                    ui->doubleSpinBoxSheetMarginLeft->setDisabled(state != 0);
                    ui->doubleSpinBoxSheetMarginRight->setDisabled(state != 0);
                    ui->doubleSpinBoxSheetMarginTop->setDisabled(state != 0);
                    ui->doubleSpinBoxSheetMarginBottom->setDisabled(state != 0);

                    VPSheetPtr sheet = m_layout->GetFocusedSheet();
                    if (not sheet.isNull())
                    {
                        sheet->SetIgnoreMargins(state != 0);
                        LayoutWasSaved(false);
                        m_layout->TileFactory()->RefreshTileInfos();
                        m_graphicsView->RefreshLayout();

                        sheet->ValidatePiecesOutOfBound();
                    }
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabTiles()
{
    // -------------------- tiles template
    VAbstractLayoutDialog::InitTileTemplates(ui->comboBoxTileTemplates, true);

    connect(ui->comboBoxTileTemplates, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this] { TileSize(TileTemplate()); });

    // -------------------- paper size ---------------------------
    MinimumTilePaperSize();

    const QString suffix = " " + UnitsToStr(LayoutUnit(), true);

    ui->doubleSpinBoxTilePaperWidth->setSuffix(suffix);
    ui->doubleSpinBoxTilePaperHeight->setSuffix(suffix);

    connect(ui->doubleSpinBoxTilePaperWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_TilesSizeChanged);
    connect(ui->doubleSpinBoxTilePaperHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_TilesSizeChanged);
    connect(ui->toolButtonTilePortraitOrientation, &QToolButton::toggled, this,
            &VPMainWindow::on_TilesOrientationChanged);
    connect(ui->toolButtonTileLandscapeOrientation, &QToolButton::toggled, this,
            &VPMainWindow::on_TilesOrientationChanged);

    // -------------------- margins  ------------------------
    ui->doubleSpinBoxTileMarginLeft->setSuffix(suffix);
    ui->doubleSpinBoxTileMarginRight->setSuffix(suffix);
    ui->doubleSpinBoxTileMarginTop->setSuffix(suffix);
    ui->doubleSpinBoxTileMarginBottom->setSuffix(suffix);

    connect(ui->doubleSpinBoxTileMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_TilesMarginChanged);
    connect(ui->doubleSpinBoxTileMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_TilesMarginChanged);
    connect(ui->doubleSpinBoxTileMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_TilesMarginChanged);
    connect(ui->doubleSpinBoxTileMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_TilesMarginChanged);

    connect(ui->checkBoxTileIgnoreFileds, &QCheckBox::stateChanged, this,
            [this](int state)
            {
                if (not m_layout.isNull())
                {
                    ui->doubleSpinBoxTileMarginLeft->setDisabled(state != 0);
                    ui->doubleSpinBoxTileMarginRight->setDisabled(state != 0);
                    ui->doubleSpinBoxTileMarginTop->setDisabled(state != 0);
                    ui->doubleSpinBoxTileMarginBottom->setDisabled(state != 0);

                    m_layout->LayoutSettings().SetIgnoreTilesMargins(state != 0);
                    LayoutWasSaved(false);
                    m_layout->TileFactory()->RefreshTileInfos();
                    m_graphicsView->RefreshLayout();
                    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
                }
            });

    // -------------------- control  ------------------------
    connect(ui->checkBoxTilesShowTiles, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetShowTiles(checked);
                    LayoutWasSaved(false);
                    m_graphicsView->RefreshLayout();
                    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
                }
            });

    connect(ui->checkBoxTilesShowWatermark, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetShowWatermark(checked);
                    LayoutWasSaved(false);
                    m_graphicsView->RefreshLayout();
                    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
                }
            });

    connect(ui->checkBoxPrintTilesScheme, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetPrintTilesScheme(checked);
                    LayoutWasSaved(false);
                }
            });

    connect(ui->checkBoxShowTileNumber, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetShowTileNumber(checked);
                    LayoutWasSaved(false);
                    m_graphicsView->RefreshLayout();
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabLayout()
{
    connect(ui->lineEditLayoutName, &QLineEdit::textEdited, this,
            [this](const QString &text)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetTitle(text);
                    LayoutWasSaved(false);
                }
            });

    connect(ui->plainTextEditLayoutDescription, &QPlainTextEdit::textChanged, this,
            [this]()
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetDescription(ui->plainTextEditLayoutDescription->toPlainText());
                    LayoutWasSaved(false);
                }
            });

    connect(ui->checkBoxLayoutWarningPiecesSuperposition, &QCheckBox::toggled, this,
            &VPMainWindow::LayoutWarningPiecesSuperposition_toggled);
    connect(ui->checkBoxLayoutWarningPiecesOutOfBound, &QCheckBox::toggled, this,
            &VPMainWindow::LayoutWarningPiecesOutOfBound_toggled);

    connect(ui->checkBoxSheetStickyEdges, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetStickyEdges(checked);
                    LayoutWasSaved(false);
                }
            });

    connect(ui->checkBoxFollowGainline, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetFollowGrainline(checked);

                    if (checked)
                    {
                        RotatePiecesToGrainline();
                    }

                    LayoutWasSaved(false);
                }
            });

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    ui->doubleSpinBoxSheetPiecesGap->setMaximum(
        UnitConvertor(VPSettings::GetMaxLayoutPieceGap(), Unit::Px, settings->LayoutUnit()));
    ui->doubleSpinBoxSheetPiecesGap->setSuffix(" " + UnitsToStr(LayoutUnit(), true));
    connect(ui->doubleSpinBoxSheetPiecesGap, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double d)
            {
                if (not m_layout.isNull())
                {
                    m_layout->LayoutSettings().SetPiecesGapConverted(d);
                    LayoutWasSaved(false);
                }
            });

    connect(ui->toolButtonScaleConnected, &QToolButton::clicked, this,
            [this]()
            {
                m_scaleConnected = not m_scaleConnected;

                UpdateScaleConnection();
            });

    connect(ui->doubleSpinBoxHorizontalScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::HorizontalScaleChanged);
    connect(ui->doubleSpinBoxVerticalScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::VerticalScaleChanged);
    connect(ui->pushButtonLayoutExport, &QPushButton::clicked, this, &VPMainWindow::on_ExportLayout);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitCarrousel()
{
    m_carrousel = new VPCarrousel(m_layout, ui->dockWidgetCarrousel);
    ui->dockWidgetCarrousel->setWidget(m_carrousel);

    connect(ui->dockWidgetCarrousel, QOverload<Qt::DockWidgetArea>::of(&QDockWidget::dockLocationChanged), this,
            &VPMainWindow::on_CarrouselLocationChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertiesData()
{
    SetPropertyTabCurrentPieceData();
    SetPropertyTabSheetData();
    SetPropertyTabTilesData();
    SetPropertyTabLayoutData();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabCurrentPieceData()
{
    QList<VPPiecePtr> selectedPieces = SelectedPieces();

    ui->labelCurrentPieceNoPieceSelected->setVisible(false);

    if (selectedPieces.isEmpty())
    {
        // show the content "no piece selected"
        ui->labelCurrentPieceNoPieceSelected->setVisible(true);

        ui->groupBoxCurrentPieceInfo->setVisible(false);
        ui->groupBoxPieceTransformation->setVisible(false);
        ui->groupBoxCurrentPieceSeamline->setVisible(false);
        ui->groupBoxCurrentPieceGeometry->setVisible(false);
    }
    else if (selectedPieces.count() == 1)
    {
        ui->groupBoxCurrentPieceInfo->setVisible(true);
        ui->groupBoxPieceTransformation->setVisible(true);
        ui->groupBoxCurrentPieceSeamline->setVisible(true);
        ui->groupBoxCurrentPieceGeometry->setVisible(true);

        const VPPiecePtr &selectedPiece = selectedPieces.constFirst();

        // set the value to the current piece
        SetLineEditValue(ui->lineEditCurrentPieceName, selectedPiece->GetName());
        SetPlainTextEditValue(ui->plainTextEditCurrentPieceUUID, selectedPiece->GetUUID().toString());
        SetLineEditValue(ui->lineEditCurrentPieceGradationId, selectedPiece->GetGradationId());

        SetCheckBoxValue(ui->checkBoxCurrentPieceShowSeamline, not selectedPiece->IsHideMainPath());
        SetCheckBoxValue(ui->checkBoxCurrentPieceMirrorPiece, selectedPiece->IsMirror());

        const bool disableFlipping = selectedPiece->IsForbidFlipping() || selectedPiece->IsForceFlipping();
        ui->checkBoxCurrentPieceMirrorPiece->setDisabled(disableFlipping);

        if (not ui->checkBoxRelativeTranslation->isChecked())
        {
            QRectF rect = PiecesBoundingRect(selectedPieces);

            ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(
                UnitConvertor(rect.topLeft().x(), Unit::Px, TranslateUnit()));
            ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(
                UnitConvertor(rect.topLeft().y(), Unit::Px, TranslateUnit()));
        }
    }
    else
    {
        // show the content "multiple pieces selected"
        ui->groupBoxCurrentPieceInfo->setVisible(false);
        ui->groupBoxPieceTransformation->setVisible(true);
        ui->groupBoxCurrentPieceSeamline->setVisible(false);
        ui->groupBoxCurrentPieceGeometry->setVisible(false);

        if (not ui->checkBoxRelativeTranslation->isChecked())
        {
            QRectF rect = PiecesBoundingRect(selectedPieces);

            ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(
                UnitConvertor(rect.topLeft().x(), Unit::Px, TranslateUnit()));
            ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(
                UnitConvertor(rect.topLeft().y(), Unit::Px, TranslateUnit()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabSheetData()
{
    if (not m_layout.isNull())
    {
        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            ui->groupBoxSheetInfos->setDisabled(false);
            SetLineEditValue(ui->lineEditSheetName, sheet->GetName());

            ui->groupBoxPaperFormat->setDisabled(false);
            const qint32 indexUnit = ui->comboBoxLayoutUnit->findData(UnitsToStr(m_layout->LayoutSettings().GetUnit()));
            if (indexUnit != -1)
            {
                ui->comboBoxLayoutUnit->blockSignals(true);
                ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
                ui->comboBoxLayoutUnit->blockSignals(false);
            }
            else
            {
                ui->comboBoxLayoutUnit->setCurrentIndex(0);
            }

            const QString suffix = " " + UnitsToStr(LayoutUnit(), true);

            ui->doubleSpinBoxSheetPaperWidth->setSuffix(suffix);
            ui->doubleSpinBoxSheetPaperHeight->setSuffix(suffix);

            ui->doubleSpinBoxSheetMarginLeft->setSuffix(suffix);
            ui->doubleSpinBoxSheetMarginRight->setSuffix(suffix);
            ui->doubleSpinBoxSheetMarginTop->setSuffix(suffix);
            ui->doubleSpinBoxSheetMarginBottom->setSuffix(suffix);

            // set Width / Length
            QSizeF size = sheet->GetSheetSizeConverted();
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperWidth, size.width());
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperHeight, size.height());

            SheetPaperSizeChanged();
            FindSheetTemplate();

            // set margins
            ui->groupBoxSheetMargin->setDisabled(false);
            QMarginsF margins = sheet->GetSheetMarginsConverted();
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginLeft, margins.left());
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginTop, margins.top());
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginRight, margins.right());
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginBottom, margins.bottom());

            CorrectSheetMaxMargins();

            const bool ignoreMargins = sheet->IgnoreMargins();
            SetCheckBoxValue(ui->checkBoxLayoutIgnoreFileds, ignoreMargins);

            ui->doubleSpinBoxSheetMarginLeft->setDisabled(ignoreMargins);
            ui->doubleSpinBoxSheetMarginRight->setDisabled(ignoreMargins);
            ui->doubleSpinBoxSheetMarginTop->setDisabled(ignoreMargins);
            ui->doubleSpinBoxSheetMarginBottom->setDisabled(ignoreMargins);

            GrainlineType type = sheet->GetGrainlineType();
            ui->toolButtonGrainlineHorizontalOrientation->setChecked(type == GrainlineType::Horizontal);
            ui->toolButtonGrainlineVerticalOrientation->setChecked(type == GrainlineType::Vertical);

            // set placement grid
            ui->groupBoxSheetGrid->setDisabled(false);
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetGridColWidth,
                                  m_layout->LayoutSettings().GetGridColWidthConverted());
            SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetGridRowHeight,
                                  m_layout->LayoutSettings().GetGridRowHeightConverted());

            SetCheckBoxValue(ui->checkBoxSheetShowGrid, m_layout->LayoutSettings().GetShowGrid());

            ui->groupBoxSheetExport->setDisabled(false);

            return;
        }
    }

    ui->groupBoxSheetInfos->setDisabled(true);
    SetLineEditValue(ui->lineEditSheetName, QString());

    ui->groupBoxPaperFormat->setDisabled(true);

    ui->comboBoxLayoutUnit->blockSignals(true);
    ui->comboBoxLayoutUnit->setCurrentIndex(-1);
    ui->comboBoxLayoutUnit->blockSignals(false);

    ui->comboBoxSheetTemplates->blockSignals(true);
    ui->comboBoxSheetTemplates->setCurrentIndex(-1);
    ui->comboBoxSheetTemplates->blockSignals(false);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperWidth, 0);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperHeight, 0);

    ui->groupBoxSheetMargin->setDisabled(true);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginLeft, 0);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginTop, 0);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginRight, 0);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginBottom, 0);

    SetCheckBoxValue(ui->checkBoxLayoutIgnoreFileds, false);

    ui->groupBoxSheetGrid->setDisabled(true);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetGridColWidth, 0);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetGridRowHeight, 0);

    SetCheckBoxValue(ui->checkBoxSheetShowGrid, false);

    ui->groupBoxSheetExport->setDisabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabTilesData()
{
    if (not m_layout.isNull())
    {
        ui->groupBoxTilePaperFormat->setDisabled(false);
        // set Width / Length
        QSizeF size = m_layout->LayoutSettings().GetTilesSizeConverted();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperWidth, size.width());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperHeight, size.height());

        TilePaperSizeChanged();
        FindTileTemplate();

        // set margins
        ui->groupBoxTileMargins->setDisabled(false);
        QMarginsF margins = m_layout->LayoutSettings().GetTilesMarginsConverted();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginLeft, margins.left());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginTop, margins.top());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginRight, margins.right());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginBottom, margins.bottom());

        CorrectTileMaxMargins();

        const bool ignoreMargins = m_layout->LayoutSettings().IgnoreTilesMargins();
        SetCheckBoxValue(ui->checkBoxTileIgnoreFileds, ignoreMargins);

        ui->doubleSpinBoxTileMarginLeft->setDisabled(ignoreMargins);
        ui->doubleSpinBoxTileMarginRight->setDisabled(ignoreMargins);
        ui->doubleSpinBoxTileMarginTop->setDisabled(ignoreMargins);
        ui->doubleSpinBoxTileMarginBottom->setDisabled(ignoreMargins);

        const QString suffix = " " + UnitsToStr(LayoutUnit(), true);

        ui->doubleSpinBoxTilePaperWidth->setSuffix(suffix);
        ui->doubleSpinBoxTilePaperHeight->setSuffix(suffix);

        ui->doubleSpinBoxTileMarginLeft->setSuffix(suffix);
        ui->doubleSpinBoxTileMarginRight->setSuffix(suffix);
        ui->doubleSpinBoxTileMarginTop->setSuffix(suffix);
        ui->doubleSpinBoxTileMarginBottom->setSuffix(suffix);

        ui->groupBoxTilesControl->setDisabled(false);

        SetCheckBoxValue(ui->checkBoxTilesShowTiles, m_layout->LayoutSettings().GetShowTiles());
        SetCheckBoxValue(ui->checkBoxTilesShowWatermark, m_layout->LayoutSettings().GetShowWatermark());
        SetCheckBoxValue(ui->checkBoxPrintTilesScheme, m_layout->LayoutSettings().GetPrintTilesScheme());
        SetCheckBoxValue(ui->checkBoxShowTileNumber, m_layout->LayoutSettings().GetShowTileNumber());
    }
    else
    {
        ui->groupBoxTilePaperFormat->setDisabled(true);

        SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperWidth, 0);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperHeight, 0);

        ui->groupBoxTileMargins->setDisabled(true);

        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginLeft, 0);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginTop, 0);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginRight, 0);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginBottom, 0);

        SetCheckBoxValue(ui->checkBoxTileIgnoreFileds, false);

        ui->groupBoxTilesControl->setDisabled(true);

        SetCheckBoxValue(ui->checkBoxTilesShowTiles, false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabLayoutData()
{
    if (not m_layout.isNull())
    {
        // set the title and description
        ui->groupBoxLayoutInfos->setDisabled(false);
        SetLineEditValue(ui->lineEditLayoutName, m_layout->LayoutSettings().GetTitle());
        SetPlainTextEditValue(ui->plainTextEditLayoutDescription, m_layout->LayoutSettings().GetDescription());

        // set controls
        ui->groupBoxLayoutControl->setDisabled(false);
        SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesOutOfBound,
                         m_layout->LayoutSettings().GetWarningPiecesOutOfBound());
        SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesSuperposition,
                         m_layout->LayoutSettings().GetWarningSuperpositionOfPieces());
        SetCheckBoxValue(ui->checkBoxSheetStickyEdges, m_layout->LayoutSettings().GetStickyEdges());
        SetCheckBoxValue(ui->checkBoxFollowGainline, m_layout->LayoutSettings().GetFollowGrainline());

        // set pieces gap
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPiecesGap, m_layout->LayoutSettings().GetPiecesGapConverted());

        ui->doubleSpinBoxSheetPiecesGap->setSuffix(" " + UnitsToStr(LayoutUnit(), true));

        ui->groupBoxLayoutScale->setDisabled(false);

        const qreal xScale = m_layout->LayoutSettings().HorizontalScale();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxHorizontalScale, xScale * 100.);

        const qreal yScale = m_layout->LayoutSettings().VerticalScale();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxVerticalScale, yScale * 100.);

        m_scaleConnected = qFuzzyCompare(xScale, yScale);
        UpdateScaleConnection();

        ui->groupBoxLayoutExport->setDisabled(false);
    }
    else
    {
        ui->groupBoxLayoutInfos->setDisabled(true);
        SetLineEditValue(ui->lineEditLayoutName, QString());
        SetPlainTextEditValue(ui->plainTextEditLayoutDescription, QString());

        ui->groupBoxLayoutControl->setDisabled(true);

        ui->comboBoxLayoutUnit->blockSignals(true);
        ui->comboBoxLayoutUnit->setCurrentIndex(-1);
        ui->comboBoxLayoutUnit->blockSignals(false);

        // set controls
        SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesOutOfBound, false);
        SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesSuperposition, false);
        SetCheckBoxValue(ui->checkBoxSheetStickyEdges, false);
        SetCheckBoxValue(ui->checkBoxFollowGainline, false);

        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPiecesGap, 0);

        ui->groupBoxLayoutScale->setDisabled(true);

        ui->groupBoxLayoutExport->setDisabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitMainGraphics()
{
    m_graphicsView = new VPMainGraphicsView(m_layout, this);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->centralWidget->layout()->addWidget(m_graphicsView);

    m_graphicsView->RefreshLayout();

    connect(m_graphicsView, &VPMainGraphicsView::ScaleChanged, this, &VPMainWindow::on_ScaleChanged);
    connect(m_graphicsView, &VPMainGraphicsView::mouseMove, this, &VPMainWindow::on_MouseMoved);
    connect(m_layout.data(), &VPLayout::PieceSheetChanged, m_carrousel, &VPCarrousel::Refresh);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitZoomToolBar()
{
    if (not m_doubleSpinBoxScale.isNull())
    {
        delete m_doubleSpinBoxScale;
    }

    delete m_mouseCoordinate;

    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

    // connect the zoom buttons and shortcuts to the slots
    QList<QKeySequence> zoomInShortcuts;
    zoomInShortcuts.append(QKeySequence(QKeySequence::ZoomIn));
    zoomInShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(QKeyCombination(Qt::ControlModifier), Qt::Key_Plus | Qt::KeypadModifier));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_Plus + Qt::KeypadModifier));
#endif
    ui->actionZoomIn->setShortcuts(zoomInShortcuts);
    connect(ui->actionZoomIn, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomIn);

    QList<QKeySequence> zoomOutShortcuts;
    zoomOutShortcuts.append(QKeySequence(QKeySequence::ZoomOut));
    zoomOutShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(QKeyCombination(Qt::ControlModifier), Qt::Key_Minus | Qt::KeypadModifier));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_Minus + Qt::KeypadModifier));
#endif
    ui->actionZoomOut->setShortcuts(zoomOutShortcuts);
    connect(ui->actionZoomOut, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomOut);

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
    connect(ui->actionZoomOriginal, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomOriginal);

    QList<QKeySequence> zoomFitBestShortcuts;
    zoomFitBestShortcuts.append(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeySequence(Qt::ControlModifier | Qt::Key_Equal));
#else
        QKeySequence(Qt::ControlModifier + Qt::Key_Equal));
#endif
    ui->actionZoomFitBest->setShortcuts(zoomFitBestShortcuts);
    connect(ui->actionZoomFitBest, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomFitBest);

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitScaleToolBar()
{
    auto *zoomScale = new QLabel(tr("Scale:"), this);
    ui->toolBarScale->addWidget(zoomScale);

    m_doubleSpinBoxScale = new QDoubleSpinBox(this);
    m_doubleSpinBoxScale->setDecimals(1);
    m_doubleSpinBoxScale->setSuffix(QChar('%'));
    on_ScaleChanged(m_graphicsView->transform().m11());
    connect(m_doubleSpinBoxScale.data(), QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double d) { m_graphicsView->Zoom(d / 100.0); });
    ui->toolBarScale->addWidget(m_doubleSpinBoxScale);

    // define the mouse position
    ui->toolBarScale->addSeparator();

    m_mouseCoordinate =
        new QLabel(QStringLiteral("0, 0 (%1)").arg(UnitsToStr(m_layout->LayoutSettings().GetUnit(), true)));
    ui->toolBarScale->addWidget(m_mouseCoordinate);
    ui->toolBarScale->addSeparator();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::UpdateWindowTitle()
{
    QString showName;
    if (not curFile.isEmpty())
    {
        showName = StrippedName(curFile);
    }
    else
    {
        vsizetype index = VPApplication::VApp()->MainWindows().indexOf(this);
        if (index != -1)
        {
            showName = tr("untitled %1.vlt").arg(index + 1);
        }
        else
        {
            showName = tr("untitled.vlt");
        }
    }

    showName += "[*]"_L1;

    if (IsLayoutReadOnly())
    {
        showName += " ("_L1 + tr("read only") + ')'_L1;
    }

    setWindowTitle(showName);
    setWindowFilePath(curFile);

#if defined(Q_OS_MAC)
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() + "/../Resources/layout.icns"_L1);
    QIcon icon;
    if (not curFile.isEmpty())
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
void VPMainWindow::ReadSettings()
{
    qCDebug(pWindow, "Reading settings.");
    const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetGeometry());
        restoreState(settings->GetToolbarsState(), static_cast<int>(AppVersion()));

        // Text under tool buton icon
        ToolBarStyles();

        ui->dockWidgetProperties->setVisible(settings->IsDockWidgetPropertiesActive());
        ui->dockWidgetPropertiesContents->setVisible(settings->IsDockWidgetPropertiesContentsActive());

        // Scene antialiasing
        m_graphicsView->SetAntialiasing(settings->GetGraphicalOutput());

        // Stack limit
        m_undoStack->setUndoLimit(settings->GetUndoCount());
    }
    else
    {
        qWarning() << tr("Cannot read settings from a malformed .INI file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::WriteSettings()
{
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    settings->SetGeometry(saveGeometry());
    settings->SetToolbarsState(saveState(static_cast<int>(AppVersion())));

    settings->SetDockWidgetPropertiesActive(ui->dockWidgetProperties->isEnabled());
    settings->SetDockWidgetPropertiesContentsActive(ui->dockWidgetPropertiesContents->isEnabled());

    settings->sync();
    if (settings->status() == QSettings::AccessError)
    {
        qWarning() << tr("Cannot save settings. Access denied.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::MaybeSave() -> bool
{
    // TODO: Implement maybe save check
    if (this->isWindowModified())
    {
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(
            tr("Unsaved changes"), tr("Layout has been modified. Do you want to save your changes?"),
            QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel, this, Qt::Sheet));

        messageBox->setDefaultButton(QMessageBox::Yes);
        messageBox->setEscapeButton(QMessageBox::Cancel);

        messageBox->setButtonText(QMessageBox::Yes, curFile.isEmpty() || IsLayoutReadOnly() ? tr("Save…") : tr("Save"));
        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

        messageBox->setWindowModality(Qt::ApplicationModal);
        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

        switch (ret)
        {
            case QMessageBox::Yes:
                if (IsLayoutReadOnly())
                {
                    return on_actionSaveAs_triggered();
                }
                return on_actionSave_triggered();
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
void VPMainWindow::CreateWindowMenu(QMenu *menu)
{
    SCASSERT(menu != nullptr)

    QAction *action = menu->addAction(tr("&New Window"));
    connect(action, &QAction::triggered, this, []() { VPApplication::VApp()->NewMainWindow()->activateWindow(); });
    action->setMenuRole(QAction::NoRole);
    menu->addSeparator();

    const QList<VPMainWindow *> windows = VPApplication::VApp()->MainWindows();
    for (int i = 0; i < windows.count(); ++i)
    {
        VPMainWindow *window = windows.at(i);

        QString title = QStringLiteral("%1. %2").arg(i + 1).arg(window->windowTitle());
        const vsizetype index = title.lastIndexOf("[*]"_L1);
        if (index != -1)
        {
            window->isWindowModified() ? title.replace(index, 3, '*'_L1) : title.replace(index, 3, QString());
        }

        QAction *action = menu->addAction(title, this, &VPMainWindow::ShowWindow);
        action->setData(i);
        action->setCheckable(true);
        action->setMenuRole(QAction::NoRole);
        if (window->isActiveWindow())
        {
            action->setChecked(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::IsLayoutReadOnly() const -> bool
{
    if (curFile.isEmpty())
    {
        return false;
    }

    QFileInfo f(curFile);

    if (not f.exists())
    {
        return false;
    }

    // #ifdef Q_OS_WIN32
    //     qt_ntfs_permission_lookup++; // turn checking on
    // #endif /*Q_OS_WIN32*/

    bool fileWritable = f.isWritable();

    // #ifdef Q_OS_WIN32
    //     qt_ntfs_permission_lookup--; // turn it off again
    // #endif /*Q_OS_WIN32*/

    return not fileWritable;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ConnectToPreferences(const QSharedPointer<DialogPuzzlePreferences> &preferences)
{
    // Must be first
    connect(preferences.data(), &DialogPuzzlePreferences::UpdateProperties, this, &VPMainWindow::WindowsLocale);
    connect(preferences.data(), &DialogPuzzlePreferences::UpdateProperties, this, &VPMainWindow::ToolBarStyles);
    connect(preferences.data(), &DialogPuzzlePreferences::UpdateProperties, this,
            [this]()
            {
                if (not m_layout.isNull())
                {
                    m_layout->RefreshScenePieces();
                }
            });
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::SelectedPieces() const -> QList<VPPiecePtr>
{
    QList<VPPiecePtr> selectedPieces;
    if (not m_layout.isNull())
    {
        VPSheetPtr activeSheet = m_layout->GetFocusedSheet();
        if (not activeSheet.isNull())
        {
            selectedPieces = activeSheet->GetSelectedPieces();
        }
    }

    return selectedPieces;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::TranslateUnit() const -> Unit
{
    return StrToUnits(ui->comboBoxTranslateUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::LayoutUnit() const -> Unit
{
    return StrToUnits(ui->comboBoxLayoutUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::Template(VAbstractLayoutDialog::PaperSizeTemplate t) const -> QSizeF
{
    const Unit layoutUnit = LayoutUnit();

    switch (t)
    {
        case VAbstractLayoutDialog::PaperSizeTemplate::A0:
        case VAbstractLayoutDialog::PaperSizeTemplate::A1:
        case VAbstractLayoutDialog::PaperSizeTemplate::A2:
        case VAbstractLayoutDialog::PaperSizeTemplate::A3:
        case VAbstractLayoutDialog::PaperSizeTemplate::A4:
        case VAbstractLayoutDialog::PaperSizeTemplate::Letter:
        case VAbstractLayoutDialog::PaperSizeTemplate::Legal:
        case VAbstractLayoutDialog::PaperSizeTemplate::Tabloid:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll24in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll30in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll36in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll42in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll44in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll48in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll62in:
        case VAbstractLayoutDialog::PaperSizeTemplate::Roll72in:
            return VAbstractLayoutDialog::GetTemplateSize(t, layoutUnit);
        case VAbstractLayoutDialog::PaperSizeTemplate::Custom:
        default:
            break;
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::SheetTemplate() const -> QSizeF
{
    auto t = static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ui->comboBoxSheetTemplates->currentData().toInt());

    if (t == VAbstractLayoutDialog::PaperSizeTemplate::Custom)
    {
        return {ui->doubleSpinBoxSheetPaperWidth->value(), ui->doubleSpinBoxSheetPaperHeight->value()};
    }

    return Template(t);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::TileTemplate() const -> QSizeF
{
    auto t = static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ui->comboBoxTileTemplates->currentData().toInt());

    if (t == VAbstractLayoutDialog::PaperSizeTemplate::Custom)
    {
        return {ui->doubleSpinBoxTilePaperWidth->value(), ui->doubleSpinBoxTilePaperHeight->value()};
    }

    return Template(t);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SheetSize(const QSizeF &size)
{
    m_oldLayoutUnit = LayoutUnit();
    ui->doubleSpinBoxSheetPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));
    ui->doubleSpinBoxSheetPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));

    ui->doubleSpinBoxSheetPaperWidth->setValue(size.width());
    ui->doubleSpinBoxSheetPaperHeight->setValue(size.height());

    CorrectPaperDecimals();
    SheetPaperSizeChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::TileSize(const QSizeF &size)
{
    m_oldLayoutUnit = LayoutUnit();
    ui->doubleSpinBoxTilePaperWidth->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));
    ui->doubleSpinBoxTilePaperHeight->setMaximum(FromPixel(QIMAGE_MAX, m_oldLayoutUnit));

    ui->doubleSpinBoxTilePaperWidth->setValue(size.width());
    ui->doubleSpinBoxTilePaperHeight->setValue(size.height());

    CorrectPaperDecimals();
    TilePaperSizeChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CorrectPaperDecimals()
{
    int decimals = 0;

    switch (m_oldLayoutUnit)
    {
        case Unit::Cm:
            decimals = 2;
            break;
        case Unit::Mm:
            decimals = 1;
            break;
        case Unit::Px:
            decimals = 4;
            break;
        case Unit::Inch:
            decimals = 5;
            break;
        default:
            break;
    }

    ui->doubleSpinBoxSheetPaperWidth->setDecimals(decimals);
    ui->doubleSpinBoxSheetPaperHeight->setDecimals(decimals);

    ui->doubleSpinBoxTilePaperWidth->setDecimals(decimals);
    ui->doubleSpinBoxTilePaperHeight->setDecimals(decimals);

    ui->doubleSpinBoxSheetMarginLeft->setDecimals(decimals);
    ui->doubleSpinBoxSheetMarginRight->setDecimals(decimals);
    ui->doubleSpinBoxSheetMarginTop->setDecimals(decimals);
    ui->doubleSpinBoxSheetMarginBottom->setDecimals(decimals);

    ui->doubleSpinBoxTileMarginLeft->setDecimals(decimals);
    ui->doubleSpinBoxTileMarginRight->setDecimals(decimals);
    ui->doubleSpinBoxTileMarginTop->setDecimals(decimals);
    ui->doubleSpinBoxTileMarginBottom->setDecimals(decimals);

    ui->doubleSpinBoxSheetPiecesGap->setDecimals(decimals);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SheetPaperSizeChanged()
{
    bool portrait = ui->doubleSpinBoxSheetPaperHeight->value() >= ui->doubleSpinBoxSheetPaperWidth->value();

    ui->toolButtonSheetPortraitOritation->blockSignals(true);
    ui->toolButtonSheetPortraitOritation->setChecked(portrait);
    ui->toolButtonSheetPortraitOritation->blockSignals(false);

    ui->toolButtonSheetLandscapeOrientation->blockSignals(true);
    ui->toolButtonSheetLandscapeOrientation->setChecked(not portrait);
    ui->toolButtonSheetLandscapeOrientation->blockSignals(false);

    if (not m_layout.isNull())
    {
        if (m_layout->LayoutSettings().GetFollowGrainline())
        {
            RotatePiecesToGrainline();
        }

        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            sheet->ValidatePiecesOutOfBound();
        }
    }

    if (m_graphicsView->scene() != nullptr)
    {
        VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::TilePaperSizeChanged()
{
    bool portrait = ui->doubleSpinBoxTilePaperHeight->value() >= ui->doubleSpinBoxTilePaperWidth->value();

    ui->toolButtonTilePortraitOrientation->blockSignals(true);
    ui->toolButtonTilePortraitOrientation->setChecked(portrait);
    ui->toolButtonTilePortraitOrientation->blockSignals(false);

    ui->toolButtonTileLandscapeOrientation->blockSignals(true);
    ui->toolButtonTileLandscapeOrientation->setChecked(not portrait);
    ui->toolButtonTileLandscapeOrientation->blockSignals(false);

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::MinimumSheetPaperSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, m_oldLayoutUnit);
    ui->doubleSpinBoxSheetPaperWidth->setMinimum(value);
    ui->doubleSpinBoxSheetPaperHeight->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::MinimumTilePaperSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, m_oldLayoutUnit);
    ui->doubleSpinBoxTilePaperWidth->setMinimum(value);
    ui->doubleSpinBoxTilePaperHeight->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::FindTemplate(QComboBox *box, qreal width, qreal height)
{
    SCASSERT(box != nullptr)
    const Unit paperUnit = LayoutUnit();

    const int max = static_cast<int>(VAbstractLayoutDialog::PaperSizeTemplate::Custom);
    for (int i = 0; i < max; ++i)
    {
        const QSizeF tmplSize =
            VAbstractLayoutDialog::GetTemplateSize(static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(i), paperUnit);
        if (QSizeF(width, height) == tmplSize || QSizeF(height, width) == tmplSize)
        {
            box->blockSignals(true);
            const int index = box->findData(i);
            if (index != -1)
            {
                box->setCurrentIndex(index);
            }
            box->blockSignals(false);
            return;
        }
    }

    box->blockSignals(true);
    const int index = box->findData(max);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::FindSheetTemplate()
{
    const qreal width = ui->doubleSpinBoxSheetPaperWidth->value();
    const qreal height = ui->doubleSpinBoxSheetPaperHeight->value();
    FindTemplate(ui->comboBoxSheetTemplates, width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::FindTileTemplate()
{
    const qreal width = ui->doubleSpinBoxTilePaperWidth->value();
    const qreal height = ui->doubleSpinBoxTilePaperHeight->value();
    FindTemplate(ui->comboBoxTileTemplates, width, height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CorrectTileMaxMargins()
{
    const qreal tileWidth = ui->doubleSpinBoxTilePaperWidth->value();
    const qreal tileHeight = ui->doubleSpinBoxTilePaperHeight->value();

    // 80%/2 of paper size for each field
    const qreal tileWidthMargin = (tileWidth * 80.0 / 100.0) / 2.0;
    const qreal tileHeightMargin = (tileHeight * 80.0 / 100.0) / 2.0;

    ui->doubleSpinBoxTileMarginLeft->setMaximum(tileWidthMargin);
    ui->doubleSpinBoxTileMarginRight->setMaximum(tileWidthMargin);
    ui->doubleSpinBoxTileMarginTop->setMaximum(tileHeightMargin);
    ui->doubleSpinBoxTileMarginBottom->setMaximum(tileHeightMargin);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CorrectSheetMaxMargins()
{
    const qreal sheetWidth = ui->doubleSpinBoxSheetPaperWidth->value();
    const qreal sheetHeight = ui->doubleSpinBoxSheetPaperHeight->value();

    // 80%/2 of paper size for each field
    const qreal sheetWidthMargin = (sheetWidth * 80.0 / 100.0) / 2.0;
    const qreal sheetHeightMargin = (sheetHeight * 80.0 / 100.0) / 2.0;

    ui->doubleSpinBoxSheetMarginLeft->setMaximum(sheetWidthMargin);
    ui->doubleSpinBoxSheetMarginRight->setMaximum(sheetWidthMargin);
    ui->doubleSpinBoxSheetMarginTop->setMaximum(sheetHeightMargin);
    ui->doubleSpinBoxSheetMarginBottom->setMaximum(sheetHeightMargin);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CorrectMaxMargins()
{
    CorrectSheetMaxMargins();
    CorrectTileMaxMargins();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::RotatePiecesToGrainline()
{
    if (not m_layout->LayoutSettings().GetFollowGrainline())
    {
        return;
    }

    QList<VPSheetPtr> sheets = m_layout->GetAllSheets();
    for (const auto &sheet : sheets)
    {
        if (not sheet.isNull())
        {
            QList<VPPiecePtr> pieces = sheet->GetPieces();
            for (const auto &piece : pieces)
            {
                if (not piece.isNull() && piece->IsGrainlineEnabled())
                {
                    QT_WARNING_PUSH
                    QT_WARNING_DISABLE_GCC("-Wnoexcept")

                    VPTransformationOrigon origin;
                    origin.custom = true;

                    QT_WARNING_POP

                    piece->RotateToGrainline(origin);
                    emit m_layout->PieceTransformationChanged(piece);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ExportData(const VPExportData &data)
{
    if (data.format == LayoutExportFormats::DXF_AAMA || data.format == LayoutExportFormats::DXF_ASTM ||
        data.format == LayoutExportFormats::RLD || data.format == LayoutExportFormats::HPGL ||
        data.format == LayoutExportFormats::HPGL2)
    {
        for (int i = 0; i < data.sheets.size(); ++i)
        {
            QString name;

            if (data.sheets.size() > 1)
            {
                name = data.path + '/' + data.fileName + QString::number(i + 1) +
                       VLayoutExporter::ExportFormatSuffix(data.format);
            }
            else
            {
                name = data.path + '/' + data.fileName + VLayoutExporter::ExportFormatSuffix(data.format);
            }

            VPSheetPtr sheet = data.sheets.at(i);
            ExportApparelLayout(data, sheet->GetAsLayoutPieces(), name, sheet->GetSheetSize().toSize());
        }
    }
    else
    {
        ExportFlatLayout(data);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ExportApparelLayout(const VPExportData &data, const QVector<VLayoutPiece> &details,
                                       const QString &name, const QSize &size)
{
    const QString path = data.path;
    bool usedNotExistedDir = CreateLayoutPath(path);
    if (not usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    settings->SetPathManualLayouts(path);

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutExporter exporter;

    QT_WARNING_POP

    exporter.SetFileName(name);
    exporter.SetImageRect(QRectF(0, 0, size.width(), size.height()));
    exporter.SetXScale(data.xScale);
    exporter.SetYScale(data.yScale);
    exporter.SetBinaryDxfFormat(data.isBinaryDXF);
    exporter.SetShowGrainline(data.showGrainline);

    switch (data.format)
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
            exporter.SetPenWidth(settings->GetLayoutLineWidth());
            exporter.ExportToHPGL(details);
            break;
        case LayoutExportFormats::HPGL2:
            exporter.SetSingleLineFont(settings->GetSingleLineFonts());
            exporter.SetSingleStrokeOutlineFont(settings->GetSingleStrokeOutlineFont());
            exporter.SetPenWidth(settings->GetLayoutLineWidth());
            exporter.ExportToHPGL2(details);
            break;
        default:
            qDebug() << "Can't recognize file type." << Q_FUNC_INFO;
            break;
    }

    RemoveLayoutPath(path, usedNotExistedDir);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ExportFlatLayout(const VPExportData &data)
{
    const QString path = data.path;
    bool usedNotExistedDir = CreateLayoutPath(path);
    if (not usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    m_layout->RefreshScenePieces();

    VPApplication::VApp()->PuzzleSettings()->SetPathManualLayouts(path);

    if (data.format == LayoutExportFormats::PDFTiled)
    {
        ExportPdfTiledFile(data);
    }
    else if ((data.format == LayoutExportFormats::PDF || data.format == LayoutExportFormats::PS ||
              data.format == LayoutExportFormats::EPS) &&
             data.exportUnified)
    {
        ExportUnifiedPdfFile(data);
    }
    else
    {
        ExportScene(data);
    }

    RemoveLayoutPath(path, usedNotExistedDir);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ExportScene(const VPExportData &data)
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VLayoutExporter exporter;

    QT_WARNING_POP

    exporter.SetXScale(data.xScale);
    exporter.SetYScale(data.yScale);
    exporter.SetDescription(m_layout->LayoutSettings().GetDescription());
    exporter.SetBinaryDxfFormat(data.isBinaryDXF);
    exporter.SetShowGrainline(data.showGrainline);

    QList<VPSheetPtr> sheets = data.sheets;

    for (int i = 0; i < sheets.size(); ++i)
    {
        const VPSheetPtr &sheet = sheets.at(i);
        if (sheet.isNull())
        {
            continue;
        }

        sheet->SceneData()->PrepareForExport();
        sheet->SceneData()->SetTextAsPaths(data.textAsPaths);

        exporter.SetMargins(sheet->GetSheetMargins());
        exporter.SetTitle(sheet->GetName());
        exporter.SetIgnorePrinterMargins(sheet->IgnoreMargins());

        QString name;
        if (sheets.size() > 1)
        {
            name = data.path + '/' + data.fileName + QString::number(i + 1) +
                   VLayoutExporter::ExportFormatSuffix(data.format);
        }
        else
        {
            name = data.path + '/' + data.fileName + VLayoutExporter::ExportFormatSuffix(data.format);
        }

        exporter.SetFileName(name);
        exporter.SetImageRect(sheet->GetMarginsRect());

        switch (data.format)
        {
            case LayoutExportFormats::SVG:
                exporter.SetPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                exporter.ExportToSVG(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::PDF:
                exporter.SetPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                exporter.ExportToPDF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::PNG:
                exporter.SetPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                exporter.ExportToPNG(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::OBJ:
                exporter.ExportToOBJ(sheet->SceneData()->Scene());
                break;
            case LayoutExportFormats::PS:
                exporter.SetPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                exporter.ExportToPS(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::EPS:
                exporter.SetPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                exporter.ExportToEPS(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1006_Flat:
                exporter.SetDxfVersion(DRW::AC1006);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1009_Flat:
                exporter.SetDxfVersion(DRW::AC1009);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1012_Flat:
                exporter.SetDxfVersion(DRW::AC1012);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1014_Flat:
                exporter.SetDxfVersion(DRW::AC1014);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1015_Flat:
                exporter.SetDxfVersion(DRW::AC1015);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1018_Flat:
                exporter.SetDxfVersion(DRW::AC1018);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1021_Flat:
                exporter.SetDxfVersion(DRW::AC1021);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1024_Flat:
                exporter.SetDxfVersion(DRW::AC1024);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::DXF_AC1027_Flat:
                exporter.SetDxfVersion(DRW::AC1027);
                exporter.ExportToFlatDXF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            case LayoutExportFormats::TIF:
                exporter.SetPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                exporter.ExportToTIF(sheet->SceneData()->Scene(), sheet->SceneData()->GraphicsPiecesAsItems());
                break;
            default:
                qDebug() << "Can't recognize file type." << Q_FUNC_INFO;
                break;
        }

        sheet->SceneData()->CleanAfterExport();
        sheet->SceneData()->SetTextAsPaths(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ExportUnifiedPdfFile(const VPExportData &data)
{
    const QString name = data.path + '/' + data.fileName + VLayoutExporter::ExportFormatSuffix(data.format);

    if (data.format == LayoutExportFormats::PDF)
    {
        GenerateUnifiedPdfFile(data, name);
    }
    else if (data.format == LayoutExportFormats::PS)
    {
        QTemporaryFile tmp;
        if (tmp.open())
        {
            GenerateUnifiedPdfFile(data, tmp.fileName());
            VLayoutExporter::PdfToPs(QStringList{tmp.fileName(), name});
        }
    }
    else if (data.format == LayoutExportFormats::EPS)
    {
        QTemporaryFile tmp;
        if (tmp.open())
        {
            GenerateUnifiedPdfFile(data, tmp.fileName());
            VLayoutExporter::PdfToPs(QStringList{QStringLiteral("-eps"), tmp.fileName(), name});
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::GenerateUnifiedPdfFile(const VPExportData &data, const QString &name)
{
    QSharedPointer<QPrinter> printer(new QPrinter());
    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOutputFileName(name);
    printer->setDocName(QFileInfo(name).fileName());
    printer->setResolution(static_cast<int>(PrintDPI));

    QPainter painter;

    bool firstPage = true;

    for (const auto &sheet : data.sheets)
    {
        if (sheet.isNull())
        {
            continue;
        }

        SetPrinterSheetPageSettings(printer, sheet, data.xScale, data.yScale);

        if (firstPage)
        {
            if (not painter.begin(printer.data()))
            { // failed to open file
                qCritical() << qUtf8Printable(tr("Can't open file '%1'").arg(name));
                return;
            }

            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(), Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush(QBrush(Qt::NoBrush));
            painter.scale(data.xScale, data.yScale);
        }
        else
        {
            if (not printer->newPage())
            {
                qCritical() << tr("Failed in flushing page to disk, disk full?");
                return;
            }
        }

        sheet->SceneData()->PrepareForExport(); // Go first because recreates pieces
        VLayoutExporter::PrepareGrainlineForExport(sheet->SceneData()->GraphicsPiecesAsItems(), data.showGrainline);
        QRectF imageRect = sheet->GetMarginsRect();
        sheet->SceneData()->Scene()->render(&painter, VPrintLayout::SceneTargetRect(printer.data(), imageRect),
                                            imageRect, Qt::IgnoreAspectRatio);
        sheet->SceneData()->CleanAfterExport(); // Will restore the grainlines automatically

        firstPage = false;
    }

    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ExportPdfTiledFile(const VPExportData &data)
{
    QSharedPointer<QPrinter> printer(new QPrinter());
#ifdef Q_OS_MAC
    printer->setOutputFormat(QPrinter::NativeFormat);
#else
    printer->setOutputFormat(QPrinter::PdfFormat);
#endif
    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    printer->setResolution(static_cast<int>(PrintDPI));

    if (data.exportUnified)
    {
        const QString name = data.path + '/' + data.fileName + VLayoutExporter::ExportFormatSuffix(data.format);

        printer->setOutputFileName(name);
        printer->setDocName(QFileInfo(name).baseName());

        QPainter painter;
        bool firstPage = true;
        for (const auto &sheet : data.sheets)
        {
            if (not GeneratePdfTiledFile(sheet, data, &painter, printer, firstPage))
            {
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < data.sheets.size(); ++i)
        {
            const QString name = data.path + '/' + data.fileName + QString::number(i + 1) +
                                 VLayoutExporter::ExportFormatSuffix(data.format);

            printer->setOutputFileName(name);
            printer->setDocName(QFileInfo(name).baseName());

            QPainter painter;
            bool firstPage = true;
            if (not GeneratePdfTiledFile(data.sheets.at(i), data, &painter, printer, firstPage))
            {
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::GeneratePdfTiledFile(const VPSheetPtr &sheet, const VPExportData &data, QPainter *painter,
                                        const QSharedPointer<QPrinter> &printer, bool &firstPage) -> bool
{
    SCASSERT(not sheet.isNull())
    SCASSERT(painter != nullptr)
    SCASSERT(not printer.isNull())

    sheet->SceneData()->PrepareForExport(); // Go first because recreates pieces
    VLayoutExporter::PrepareGrainlineForExport(sheet->SceneData()->GraphicsPiecesAsItems(), data.showGrainline);
    m_layout->TileFactory()->RefreshTileInfos();
    m_layout->TileFactory()->RefreshWatermarkData();
    sheet->SceneData()->SetTextAsPaths(data.textAsPaths);

    auto Clean = qScopeGuard(
        [sheet]()
        {
            sheet->SceneData()->SetTextAsPaths(false);
            sheet->SceneData()->CleanAfterExport(); // Will restore the grainlines automatically
        });

    if (data.showTilesScheme)
    {
        SetPrinterTiledPageSettings(printer, m_layout, sheet, sheet->GetSheetOrientation(), true);
    }
    else
    {
        SetPrinterTiledPageSettings(printer, m_layout, sheet, m_layout->LayoutSettings().GetTilesOrientation(), false);
    }

    if (firstPage)
    {
        if (not painter->begin(printer.data()))
        { // failed to open file
            qCritical() << tr("Failed to open file, is it writable?");
            return false;
        }

        painter->setPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthMainLine(), Qt::SolidLine,
                             Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    if (data.showTilesScheme)
    {
        if (not DrawTilesScheme(printer.data(), painter, sheet, firstPage))
        {
            return false;
        }
        firstPage = false;
    }

    for (int row = 0; row < m_layout->TileFactory()->RowNb(sheet); row++) // for each row of the tiling grid
    {
        for (int col = 0; col < m_layout->TileFactory()->ColNb(sheet); col++) // for each column of tiling grid
        {
            if (not firstPage)
            {
                SetPrinterTiledPageSettings(printer, m_layout, sheet, m_layout->LayoutSettings().GetTilesOrientation(),
                                            false);

                if (not printer->newPage())
                {
                    qWarning("failed in flushing page to disk, disk full?");
                    return false;
                }
            }

            m_layout->TileFactory()->drawTile(painter, printer.data(), sheet, row, col);

            firstPage = false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::UpdateScaleConnection() const
{
    QIcon icon;
    icon.addFile(m_scaleConnected ? QStringLiteral(":/icon/32x32/link.png")
                                  : QStringLiteral(":/icon/32x32/broken_link.png"));
    ui->toolButtonScaleConnected->setIcon(icon);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::OpenWatermark(const QString &path)
{
    for (const auto &m_watermarkEditor : qAsConst(m_watermarkEditors))
    {
        if (not m_watermarkEditor.isNull() && not m_watermarkEditor->CurrentFile().isEmpty() &&
            m_watermarkEditor->CurrentFile() == AbsoluteMPath(curFile, path))
        {
            m_watermarkEditor->show();
            return;
        }
    }

    auto *watermark = new WatermarkWindow(curFile, this);
    connect(watermark, &WatermarkWindow::New, this, [this]() { OpenWatermark(); });
    connect(watermark, &WatermarkWindow::OpenAnother, this, [this](const QString &path) { OpenWatermark(path); });
    m_watermarkEditors.append(watermark);
    watermark->show();
    watermark->Open(path);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CleanWaterkmarkEditors()
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
auto VPMainWindow::DrawTilesScheme(QPrinter *printer, QPainter *painter, const VPSheetPtr &sheet, bool firstPage)
    -> bool
{
    SCASSERT(printer != nullptr)
    SCASSERT(painter != nullptr)

    if (sheet.isNull())
    {
        return false;
    }

    if (not firstPage)
    {
        printer->setPageOrientation(sheet->GetSheetOrientation());

        if (not printer->newPage())
        {
            qWarning("failed in flushing page to disk, disk full?");
            return false;
        }
    }

    sheet->SceneData()->PrepareTilesScheme();

    qreal xScale = m_layout->LayoutSettings().HorizontalScale();
    qreal yScale = m_layout->LayoutSettings().VerticalScale();

    qreal width = m_layout->TileFactory()->DrawingAreaWidth();
    qreal height = m_layout->TileFactory()->DrawingAreaHeight();

    QPageLayout::Orientation tileOrientation = m_layout->LayoutSettings().GetTilesOrientation();
    QPageLayout::Orientation sheetOrientation = sheet->GetSheetOrientation();

    QRectF sheetRect = sheet->GetMarginsRect();

    const int nbCol = m_layout->TileFactory()->ColNb(sheet);
    const int nbRow = m_layout->TileFactory()->RowNb(sheet);

    QRectF source = QRectF(sheetRect.topLeft(), QSizeF(nbCol * ((width - VPTileFactory::tileStripeWidth) / xScale),
                                                       nbRow * ((height - VPTileFactory::tileStripeWidth) / yScale)));
    QRectF target;

    if (tileOrientation != sheetOrientation)
    {
        QMarginsF margins;
        if (not m_layout->LayoutSettings().IgnoreTilesMargins())
        {
            margins = m_layout->LayoutSettings().GetTilesMargins();
        }

        QSizeF tilesSize = m_layout->LayoutSettings().GetTilesSize();
        target = QRectF(0, 0, tilesSize.height() - margins.left() - margins.right(),
                        tilesSize.width() - margins.top() - margins.bottom());
    }
    else
    {
        target = QRectF(0, 0, width, height);
    }

    target = VPrintLayout::SceneTargetRect(printer, target);

    sheet->SceneData()->Scene()->render(painter, target, source, Qt::KeepAspectRatio);

    VWatermarkData watermarkData = m_layout->TileFactory()->WatermarkData();
    if (watermarkData.opacity > 0)
    {
        if (watermarkData.showImage && not watermarkData.path.isEmpty())
        {
            VPTileFactory::PaintWatermarkImage(painter, target, watermarkData,
                                               m_layout->LayoutSettings().WatermarkPath(), false);
        }

        if (watermarkData.showText && not watermarkData.text.isEmpty())
        {
            VPTileFactory::PaintWatermarkText(painter, target, watermarkData);
        }
    }

    sheet->SceneData()->ClearTilesScheme();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::AskLayoutIsInvalid(const QList<VPSheetPtr> &sheets) -> bool
{
    if (not m_layout->LayoutSettings().GetWarningPiecesOutOfBound() &&
        not m_layout->LayoutSettings().GetWarningSuperpositionOfPieces())
    {
        return true;
    }

    for (const auto &sheet : sheets)
    {
        bool outOfBoundChecked = false;
        bool pieceSuperpositionChecked = false;

        QList<VPPiecePtr> pieces = sheet->GetPieces();
        for (const auto &piece : pieces)
        {
            if (not CheckPiecesOutOfBound(piece, outOfBoundChecked))
            {
                return false;
            }

            if (not CheckSuperpositionOfPieces(piece, pieceSuperpositionChecked))
            {
                return false;
            }
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::CheckPiecesOutOfBound(const VPPiecePtr &piece, bool &outOfBoundChecked) -> bool
{
    if (m_layout->LayoutSettings().GetWarningPiecesOutOfBound())
    {
        if (not outOfBoundChecked && not piece.isNull() && piece->OutOfBound())
        {
            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setWindowTitle(tr("The layout is invalid."));
            msgBox.setText(tr("The layout is invalid. Piece out of bound. Do you want to continue export?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            const int width = 500;
            auto *horizontalSpacer = new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
            auto *layout = qobject_cast<QGridLayout *>(msgBox.layout());
            SCASSERT(layout != nullptr)
            layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
            if (msgBox.exec() == QMessageBox::No)
            {
                return false;
            }

            outOfBoundChecked = true; // no need to ask more
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::CheckSuperpositionOfPieces(const VPPiecePtr &piece, bool &pieceSuperpositionChecked) -> bool
{
    if (m_layout->LayoutSettings().GetWarningSuperpositionOfPieces())
    {
        if (not pieceSuperpositionChecked && not piece.isNull() && piece->HasSuperpositionWithPieces())
        {
            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setWindowTitle(tr("The layout is invalid."));
            msgBox.setText(tr("The layout is invalid. Pieces superposition. Do you want to continue "
                              "export?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            const int width = 500;
            auto *horizontalSpacer = new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
            auto *layout = qobject_cast<QGridLayout *>(msgBox.layout());
            SCASSERT(layout != nullptr)
            layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
            if (msgBox.exec() == QMessageBox::No)
            {
                return false;
            }

            pieceSuperpositionChecked = true; // no need to ask more
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::PrintLayoutSheets(QPrinter *printer, const QList<VPSheetPtr> &sheets)
{
    SCASSERT(printer != nullptr)

    // Handle the fromPage(), toPage(), supportsMultipleCopies(), and numCopies() values from QPrinter.
    int firstPageNumber = printer->fromPage() - 1;
    if (firstPageNumber >= sheets.count())
    {
        return;
    }
    if (firstPageNumber == -1)
    {
        firstPageNumber = 0;
    }

    vsizetype lastPageNumber = printer->toPage() - 1;
    if (lastPageNumber == -1 || lastPageNumber >= sheets.count())
    {
        lastPageNumber = sheets.count() - 1;
    }

    const vsizetype numPages = lastPageNumber - firstPageNumber + 1;
    int copyCount = 1;
    if (not printer->supportsMultipleCopies())
    {
        copyCount = printer->copyCount();
    }

    QPainter painter;

    if (not painter.begin(printer))
    { // failed to open file
        qCritical() << tr("Failed to open file, is it writable?");
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthHairLine(), Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.scale(m_layout->LayoutSettings().HorizontalScale(), m_layout->LayoutSettings().VerticalScale());

    for (int i = 0; i < copyCount; ++i)
    {
        for (int j = 0; j < numPages; ++j)
        {
            vsizetype index =
                printer->pageOrder() == QPrinter::FirstPageFirst ? firstPageNumber + j : lastPageNumber - j;

            const VPSheetPtr &sheet = sheets.at(index);
            if (sheet.isNull())
            {
                continue;
            }

            if (not PrintLayoutSheetPage(printer, painter, sheet))
            {
                return;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::PrintLayoutSheetPage(QPrinter *printer, QPainter &painter, const VPSheetPtr &sheet) -> bool
{
    SCASSERT(printer != nullptr)

    printer->setPageOrientation(sheet->GetSheetOrientation());
    printer->setFullPage(sheet->IgnoreMargins());

    if (not sheet->IgnoreMargins())
    {
        QMarginsF margins = sheet->GetSheetMargins();
        if (not printer->setPageMargins(UnitConvertor(margins, Unit::Px, Unit::Mm), QPageLayout::Millimeter))
        {
            qWarning() << QObject::tr("Cannot set printer margins");
        }
    }

    if (not printer->newPage())
    {
        qCritical() << tr("Failed in flushing page to disk, disk full?");
        return false;
    }

    sheet->SceneData()->PrepareForExport();
    QRectF imageRect = sheet->GetMarginsRect();
    sheet->SceneData()->Scene()->render(&painter, VPrintLayout::SceneTargetRect(printer, imageRect), imageRect,
                                        Qt::IgnoreAspectRatio);
    sheet->SceneData()->CleanAfterExport();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::PrintLayoutTiledSheets(QPrinter *printer, const QList<VPSheetPtr> &sheets)
{
    SCASSERT(printer != nullptr)

    const QVector<VPLayoutPrinterPage> pages = PrepareLayoutTilePages(sheets);

    // Handle the fromPage(), toPage(), supportsMultipleCopies(), and numCopies() values from QPrinter.
    int firstPageNumber = printer->fromPage() - 1;
    if (firstPageNumber >= pages.count())
    {
        return;
    }
    if (firstPageNumber == -1)
    {
        firstPageNumber = 0;
    }

    vsizetype lastPageNumber = printer->toPage() - 1;
    if (lastPageNumber == -1 || lastPageNumber >= pages.count())
    {
        lastPageNumber = pages.count() - 1;
    }

    const vsizetype numPages = lastPageNumber - firstPageNumber + 1;
    int copyCount = 1;
    if (not printer->supportsMultipleCopies())
    {
        copyCount = printer->copyCount();
    }

    QPainter painter;
    bool firstPage = true;

    m_layout->TileFactory()->RefreshTileInfos();
    m_layout->TileFactory()->RefreshWatermarkData();

    for (int i = 0; i < copyCount; ++i)
    {
        for (int j = 0; j < numPages; ++j)
        {
            vsizetype index =
                printer->pageOrder() == QPrinter::FirstPageFirst ? firstPageNumber + j : lastPageNumber - j;
            const VPLayoutPrinterPage &page = pages.at(index);

            if (not PrintLayoutTiledSheetPage(printer, painter, page, firstPage))
            {
                return;
            }

            firstPage = false;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::PrepareLayoutTilePages(const QList<VPSheetPtr> &sheets) -> QVector<VPMainWindow::VPLayoutPrinterPage>
{
    QVector<VPLayoutPrinterPage> pages;

    for (const auto &sheet : sheets)
    {
        if (m_layout->LayoutSettings().GetPrintTilesScheme())
        {
            VPLayoutPrinterPage page;
            page.sheet = sheet;
            page.tilesScheme = true;

            pages.append(page);
        }

        for (int row = 0; row < m_layout->TileFactory()->RowNb(sheet); row++) // for each row of the tiling grid
        {
            for (int col = 0; col < m_layout->TileFactory()->ColNb(sheet); col++) // for each column of tiling grid
            {
                VPLayoutPrinterPage page;
                page.sheet = sheet;
                page.tilesScheme = false;
                page.tileRow = row;
                page.tileCol = col;

                pages.append(page);
            }
        }
    }

    return pages;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::PrintLayoutTiledSheetPage(QPrinter *printer, QPainter &painter, const VPLayoutPrinterPage &page,
                                             bool firstPage) -> bool
{
    page.sheet->SceneData()->PrepareForExport();

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    page.sheet->SceneData()->SetTextAsPaths(settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont());

    auto clean = qScopeGuard(
        [page]()
        {
            page.sheet->SceneData()->SetTextAsPaths(false);
            page.sheet->SceneData()->CleanAfterExport();
        });

    if (firstPage)
    {
        if (page.tilesScheme)
        {
            printer->setPageOrientation(page.sheet->GetSheetOrientation());
        }
        else
        {
            printer->setPageOrientation(m_layout->LayoutSettings().GetTilesOrientation());
        }

        if (not painter.begin(printer))
        { // failed to open file
            qCritical() << tr("Failed to open file, is it writable?");
            return false;
        }

        painter.setPen(QPen(Qt::black, VAbstractApplication::VApp()->Settings()->WidthMainLine(), Qt::SolidLine,
                            Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(QBrush(Qt::NoBrush));
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    if (page.tilesScheme)
    {
        if (not DrawTilesScheme(printer, &painter, page.sheet, firstPage))
        {
            return false;
        }

        firstPage = false;
    }

    if (not firstPage)
    {
        printer->setPageOrientation(m_layout->LayoutSettings().GetTilesOrientation());

        if (not printer->newPage())
        {
            qWarning("failed in flushing page to disk, disk full?");
            return false;
        }
    }

    m_layout->TileFactory()->drawTile(&painter, printer, page.sheet, page.tileRow, page.tileCol);

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ZValueMove(int move)
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    QList<VPPiecePtr> selectedPieces = sheet->GetSelectedPieces();
    if (selectedPieces.isEmpty())
    {
        return;
    }

    QList<VPPiecePtr> allPieces = sheet->GetPieces();
    if (allPieces.isEmpty() || (allPieces.size() == selectedPieces.size()))
    {
        return;
    }

    auto zMove = static_cast<ML::ZValueMove>(move);

    if (selectedPieces.size() == 1)
    {
        m_layout->UndoStack()->push(new VPUndoPieceZValueMove(selectedPieces.constFirst(), zMove));
    }
    else if (selectedPieces.size() > 1)
    {
        m_layout->UndoStack()->push(new VPUndoPiecesZValueMove(allPieces, zMove));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::ImportRawLayout(const QString &rawLayout) -> bool
{
    VRawLayout rawLayoutReader;
    VRawLayoutData data;
    if (rawLayoutReader.ReadFile(rawLayout, data))
    {
        return AddLayoutPieces(data.pieces);
    }

    qCCritical(
        pWindow, "%s\n",
        qPrintable(tr("Could not extract data from file '%1'. %2").arg(rawLayout, rawLayoutReader.ErrorString())));
    if (m_cmd != nullptr && not m_cmd->IsGuiEnabled())
    {
        m_cmd->ShowHelp(V_EX_DATAERR);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::AddLayoutPieces(const QVector<VLayoutPiece> &pieces) -> bool
{
    for (const auto &rawPiece : pieces)
    {
        for (quint16 i = 1; i <= rawPiece.GetQuantity(); ++i)
        {
            VPPiecePtr piece(new VPPiece(rawPiece));
            piece->SetCopyNumber(i);

            QString error;
            if (not piece->IsValid(error))
            {
                qCCritical(pWindow) << qPrintable(tr("Piece %1 invalid. %2").arg(piece->GetName(), error));

                if (m_cmd != nullptr && not m_cmd->IsGuiEnabled())
                {
                    QGuiApplication::exit(V_EX_DATAERR);
                    return false;
                }
            }

            piece->SetSheet(VPSheetPtr()); // just in case
            VPLayout::AddPiece(m_layout, piece);
        }
    }

    m_carrousel->Refresh();
    m_layout->CheckPiecesPositionValidity();
    LayoutWasSaved(false);

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::TranslatePieces()
{
    QList<VPPiecePtr> selectedPieces = SelectedPieces();
    if (selectedPieces.isEmpty())
    {
        return;
    }

    const qreal dx = UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionX->value(), TranslateUnit(), Unit::Px);
    const qreal dy = UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionY->value(), TranslateUnit(), Unit::Px);

    if (ui->checkBoxRelativeTranslation->isChecked())
    {
        if (ui->checkBoxTransformSeparately->isChecked())
        {
            if (selectedPieces.size() > 1)
            {
                m_layout->UndoStack()->beginMacro(tr("translate pieces"));
            }

            QRectF rect = PiecesBoundingRect(selectedPieces);
            for (const auto &piece : qAsConst(selectedPieces))
            {
                TranslatePieceRelatively(piece, rect, selectedPieces.size(), dx, dy);
            }

            if (selectedPieces.size() > 1)
            {
                m_layout->UndoStack()->endMacro();
            }
        }
        else
        {
            auto *command = new VPUndoPiecesMove(selectedPieces, dx, dy);
            m_layout->UndoStack()->push(command);
        }
    }
    else
    {
        QRectF rect = PiecesBoundingRect(selectedPieces);
        qreal pieceDx = dx - rect.topLeft().x();
        qreal pieceDy = dy - rect.topLeft().y();

        if (selectedPieces.size() == 1)
        {
            auto *command = new VPUndoPieceMove(selectedPieces.constFirst(), pieceDx, pieceDy);
            m_layout->UndoStack()->push(command);
        }
        else
        {
            auto *command = new VPUndoPiecesMove(selectedPieces, pieceDx, pieceDy);
            m_layout->UndoStack()->push(command);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::TranslatePieceRelatively(const VPPiecePtr &piece, const QRectF &rect, vsizetype selectedPiecesCount,
                                            qreal dx, qreal dy)
{
    if (not piece.isNull())
    {
        const QRectF pieceRect = piece->MappedDetailBoundingRect();
        qreal pieceDx = dx;
        qreal pieceDy = dy;

        if (not qFuzzyIsNull(rect.width()))
        {
            pieceDx += dx * ((pieceRect.topLeft().x() - rect.topLeft().x()) / rect.width()) * 2.;
        }

        if (not qFuzzyIsNull(rect.height()))
        {
            pieceDy += dy * ((pieceRect.topLeft().y() - rect.topLeft().y()) / rect.height()) * 2.;
        }

        auto *command = new VPUndoPieceMove(piece, pieceDx, pieceDy);
        m_layout->UndoStack()->push(command);

        if (m_layout->LayoutSettings().GetStickyEdges())
        {
            qreal stickyTranslateX = 0;
            qreal stickyTranslateY = 0;
            if (piece->StickyPosition(stickyTranslateX, stickyTranslateY))
            {
                bool allowMerge = selectedPiecesCount == 1;
                auto *stickyCommand = new VPUndoPieceMove(piece, stickyTranslateX, stickyTranslateY, allowMerge);
                m_layout->UndoStack()->push(stickyCommand);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::RotatePieces()
{
    qreal angle = ui->doubleSpinBoxCurrentPieceAngle->value();

    if (ui->toolButtonCurrentPieceRotationClockwise->isChecked())
    {
        angle *= -1;
    }

    QList<VPPiecePtr> selectedPieces = SelectedPieces();
    if (selectedPieces.isEmpty())
    {
        return;
    }

    if (ui->checkBoxTransformSeparately->isChecked())
    {
        m_layout->UndoStack()->beginMacro(tr("rotate pieces"));
        for (const auto &piece : selectedPieces)
        {
            if (not piece.isNull())
            {
                const QRectF rect = piece->MappedDetailBoundingRect();

                VPTransformationOrigon origin;
                origin.origin = rect.center();
                origin.custom = true;

                auto *command = new VPUndoPieceRotate(piece, origin, angle, angle);
                m_layout->UndoStack()->push(command);
            }
        }
        m_layout->UndoStack()->endMacro();
    }
    else
    {
        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (sheet.isNull())
        {
            return;
        }

        VPTransformationOrigon origin = sheet->TransformationOrigin();
        auto *command = new VPUndoPiecesRotate(selectedPieces, origin, angle, angle);
        m_layout->UndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitIcons()
{
    const QString resource = QStringLiteral("puzzleicon");

    auto SetTabIcon = [resource, this](QWidget *tab, const QString &iconName)
    {
        const int index = ui->tabWidgetProperties->indexOf(tab);
        if (index != -1)
        {
            ui->tabWidgetProperties->setTabIcon(index, VTheme::GetIconResource(resource, iconName));
        }
    };

    SetTabIcon(ui->tabCurrentPieceProperty, QStringLiteral("64x64/iconCurrentPiece.png"));
    SetTabIcon(ui->tabTilesProperty, QStringLiteral("64x64/iconTiles.png"));

    ui->toolButtonGrainlineHorizontalOrientation->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("32x32/horizontal_grainline.png")));
    ui->toolButtonGrainlineVerticalOrientation->setIcon(
        VTheme::GetIconResource(resource, QStringLiteral("32x32/vertical_grainline.png")));
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionNew_triggered() // NOLINT(readability-convert-member-functions-to-static)
{
    VPApplication::VApp()->NewMainWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ShowToolTip(const QString &toolTip)
{
    m_statusLabel->setText(toolTip);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::closeEvent(QCloseEvent *event)
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(5, 11, 1)
    // Workaround for Qt bug https://bugreports.qt.io/browse/QTBUG-43344
    static int numCalled = 0;
    if (numCalled++ >= 1)
    {
        return;
    }
#endif

    if (MaybeSave())
    {
        WriteSettings();
        event->accept();
        deleteLater();
    }
    else
    {
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        undoAction->setText(tr("&Undo"));
        redoAction->setText(tr("&Redo"));

        WindowsLocale();
        UpdateWindowTitle();
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
    }

    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::RecentFileList() const -> QStringList
{
    return VPApplication::VApp()->PuzzleSettings()->GetRecentFileList();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionOpen_triggered()
{
    qCDebug(pWindow, "Openning puzzle layout file.");

    const QString filter(tr("Layout files") + QStringLiteral(" (*.vlt)"));
    // Use standard path to individual measurements
    const QString pathTo = VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts();

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        VPApplication::VApp()->NewMainWindow()->LoadFile(mPath);
        VPApplication::VApp()->PuzzleSettings()->SetPathManualLayouts(QFileInfo(mPath).absolutePath());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::on_actionSave_triggered() -> bool
{
    if (curFile.isEmpty() || IsLayoutReadOnly())
    {
        return on_actionSaveAs_triggered();
    }

    if (m_curFileFormatVersion < VLayoutConverter::LayoutMaxVer &&
        not ContinueFormatRewrite(m_curFileFormatVersionStr, VLayoutConverter::LayoutMaxVerStr))
    {
        return false;
    }

    if (not CheckFilePermissions(curFile, this))
    {
        return false;
    }

    QString error;
    if (not SaveLayout(curFile, error))
    {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(tr("Could not save the file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return false;
    }

    m_curFileFormatVersion = VLayoutConverter::LayoutMaxVer;
    m_curFileFormatVersionStr = VLayoutConverter::LayoutMaxVerStr;

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::on_actionSaveAs_triggered() -> bool
{
    QString filters = tr("Layout files") + QStringLiteral(" (*.vlt)");
    QString suffix = QStringLiteral("vlt");
    QString fName = tr("layout") + '.'_L1 + suffix;

    QString dir;
    if (curFile.isEmpty())
    {
        dir = VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts();
    }
    else
    {
        dir = QFileInfo(curFile).absolutePath();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), dir + '/'_L1 + fName, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    if (fileName.isEmpty())
    {
        return false;
    }

    QFileInfo f(fileName);
    if (f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += '.'_L1 + suffix;
    }

    if (curFile.isEmpty())
    {
        VPApplication::VApp()->PuzzleSettings()->SetPathManualLayouts(QFileInfo(fileName).absolutePath());
    }

    if (not CheckFilePermissions(fileName, this))
    {
        return false;
    }

    if (QFileInfo::exists(fileName) && curFile != fileName)
    {
        // Temporary try to lock the file before saving
        VLockGuard<char> tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCCritical(pWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
            return false;
        }
    }

    QString error;
    bool result = SaveLayout(fileName, error);
    if (not result)
    {
        QMessageBox messageBox;
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return false;
    }

    m_curFileFormatVersion = VLayoutConverter::LayoutMaxVer;
    m_curFileFormatVersionStr = VLayoutConverter::LayoutMaxVerStr;

    UpdateWindowTitle();

    if (curFile == fileName && not lock.isNull())
    {
        lock->Unlock();
    }
    VlpCreateLock(lock, fileName);
    if (not lock->IsLocked())
    {
        qCCritical(pWindow, "%s",
                   qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
                                     "Expect collissions when run 2 copies of the program.")));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionImportRawLayout_triggered()
{
    const QString filter(tr("Raw Layout files") + QStringLiteral(" (*.rld)"));

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::homePath(), filter, nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());

    if (not filePath.isEmpty())
    {
        ImportRawLayouts({filePath});
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionAboutPuzzle_triggered()
{
    auto *aboutDialog = new VPDialogAbout(this);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    aboutDialog->show();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_LayoutUnitChanged(int index)
{
    Q_UNUSED(index);
    QVariant comboBoxValue = ui->comboBoxLayoutUnit->currentData();
    m_layout->LayoutSettings().SetUnit(StrToUnits(comboBoxValue.toString()));

    SetPropertyTabCurrentPieceData();
    SetPropertyTabSheetData();
    SetPropertyTabTilesData();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetSizeChanged()
{
    if (not m_layout.isNull())
    {
        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            sheet->SetSheetSizeConverted(ui->doubleSpinBoxSheetPaperWidth->value(),
                                         ui->doubleSpinBoxSheetPaperHeight->value());
        }

        FindSheetTemplate();
        SheetPaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);

        m_layout->TileFactory()->RefreshTileInfos();
        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetOrientationChanged(bool checked)
{
    if (checked && not m_layout.isNull())
    {
        const qreal width = ui->doubleSpinBoxSheetPaperWidth->value();
        const qreal height = ui->doubleSpinBoxSheetPaperHeight->value();

        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperWidth, height);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperHeight, width);

        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            sheet->SetSheetSizeConverted(height, width); // NOLINT(readability-suspicious-call-argument)
        }

        SheetPaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);
        m_layout->TileFactory()->RefreshTileInfos();
        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetMarginChanged()
{
    if (not m_layout.isNull())
    {
        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            sheet->SetSheetMarginsConverted(
                ui->doubleSpinBoxSheetMarginLeft->value(), ui->doubleSpinBoxSheetMarginTop->value(),
                ui->doubleSpinBoxSheetMarginRight->value(), ui->doubleSpinBoxSheetMarginBottom->value());

            LayoutWasSaved(false);

            sheet->ValidatePiecesOutOfBound();
        }

        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_checkBoxSheetShowGrid_toggled(bool checked)
{
    m_layout->LayoutSettings().SetShowGrid(checked);
    m_graphicsView->RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_doubleSpinBoxSheetGridColWidth_valueChanged(double value)
{
    m_layout->LayoutSettings().SetGridColWidthConverted(value);
    m_graphicsView->RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_doubleSpinBoxSheetGridRowHeight_valueChanged(double value)
{
    m_layout->LayoutSettings().SetGridRowHeightConverted(value);
    m_graphicsView->RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_TilesSizeChanged()
{
    if (not m_layout.isNull())
    {
        m_layout->LayoutSettings().SetTilesSizeConverted(ui->doubleSpinBoxTilePaperWidth->value(),
                                                         ui->doubleSpinBoxTilePaperHeight->value());
        FindTileTemplate();
        TilePaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);

        m_layout->TileFactory()->RefreshTileInfos();
        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_TilesOrientationChanged(bool checked)
{
    if (checked && not m_layout.isNull())
    {
        const qreal width = ui->doubleSpinBoxTilePaperWidth->value();
        const qreal height = ui->doubleSpinBoxTilePaperHeight->value();

        SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperWidth, height);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperHeight, width);

        m_layout->LayoutSettings().SetTilesSizeConverted(height, width); // NOLINT(readability-suspicious-call-argument)

        TilePaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);
        m_layout->TileFactory()->RefreshTileInfos();
        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_TilesMarginChanged()
{
    if (not m_layout.isNull())
    {
        m_layout->LayoutSettings().SetTilesMarginsConverted(
            ui->doubleSpinBoxTileMarginLeft->value(), ui->doubleSpinBoxTileMarginTop->value(),
            ui->doubleSpinBoxTileMarginRight->value(), ui->doubleSpinBoxTileMarginBottom->value());
        LayoutWasSaved(false);
        m_layout->TileFactory()->RefreshTileInfos();
        m_graphicsView->RefreshLayout();
    }

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_CarrouselLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
    {
        m_carrousel->SetOrientation(Qt::Horizontal);
    }
    else if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
    {
        m_carrousel->SetOrientation(Qt::Vertical);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_PieceSelectionChanged()
{
    // update the property of the piece currently selected
    SetPropertyTabCurrentPieceData();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_ScaleChanged(qreal scale)
{
    if (not m_doubleSpinBoxScale.isNull())
    {
        m_doubleSpinBoxScale->blockSignals(true);
        m_doubleSpinBoxScale->setMaximum(qFloor(VPMainGraphicsView::MaxScale() * 1000) / 10.0);
        m_doubleSpinBoxScale->setMinimum(qFloor(VPMainGraphicsView::MinScale() * 1000) / 10.0);
        m_doubleSpinBoxScale->setValue(qFloor(scale * 1000) / 10.0);
        m_doubleSpinBoxScale->setSingleStep(1);
        m_doubleSpinBoxScale->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_MouseMoved(const QPointF &scenePos)
{
    if (m_mouseCoordinate != nullptr)
    {
        m_mouseCoordinate->setText(
            QStringLiteral("%1, %2 (%3)")
                .arg(static_cast<qint32>(FromPixel(scenePos.x(), m_layout->LayoutSettings().GetUnit())))
                .arg(static_cast<qint32>(FromPixel(scenePos.y(), m_layout->LayoutSettings().GetUnit())))
                .arg(UnitsToStr(m_layout->LayoutSettings().GetUnit(), true)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ShowWindow() const
{
    if (auto *action = qobject_cast<QAction *>(sender()))
    {
        const QVariant v = action->data();
        if (v.canConvert<int>())
        {
            const int offset = qvariant_cast<int>(v);
            const QList<VPMainWindow *> windows = VPApplication::VApp()->MainWindows();
            windows.at(offset)->raise();
            windows.at(offset)->activateWindow();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPreferences_triggered() // NOLINT(readability-convert-member-functions-to-static)
{
    // Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
    QSharedPointer<DialogPuzzlePreferences> preferences = VPApplication::VApp()->PreferencesDialog();
    if (preferences.isNull())
    {
        auto CleanAfterDialog = qScopeGuard([&preferences]() { preferences.clear(); });

        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        preferences.reset(new DialogPuzzlePreferences());
        preferences->setWindowModality(Qt::ApplicationModal);
        VPApplication::VApp()->SetPreferencesDialog(preferences);

        const QList<VPMainWindow *> windows = VPApplication::VApp()->MainWindows();
        for (auto *window : windows)
        {
            window->ConnectToPreferences(preferences);
        }

        QGuiApplication::restoreOverrideCursor();

        preferences->exec();
    }
    else
    {
        preferences->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ToolBarStyles()
{
    ToolBarStyle(ui->mainToolBar);
    ToolBarStyle(ui->toolBarZoom);
    ToolBarStyle(ui->toolBarUndoCommands);
    ToolBarStyle(ui->toolBarZValue);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionAddSheet_triggered()
{
    VPSheetPtr sheet(new VPSheet(m_layout));
    sheet->SetName(tr("Sheet %1").arg(m_layout->GetAllSheets().size() + 1));
    m_layout->UndoStack()->push(new VPUndoAddSheet(sheet));
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_ApplyPieceTransformation()
{
    if (m_layout.isNull())
    {
        return;
    }
    const int index = ui->tabWidgetPieceTransformation->currentIndex();
    if (ui->tabWidgetPieceTransformation->indexOf(ui->tabTranslate) == index)
    {
        TranslatePieces();
    }
    else if (ui->tabWidgetPieceTransformation->indexOf(ui->tabRotate) == index)
    {
        RotatePieces();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_ResetPieceTransformationSettings()
{
    const int index = ui->tabWidgetPieceTransformation->currentIndex();
    if (ui->tabWidgetPieceTransformation->indexOf(ui->tabTranslate) == index)
    { // translate
        if (ui->checkBoxRelativeTranslation->isChecked())
        {
            ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(0);
            ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(0);
        }
        else
        {
            int unitIndex = ui->comboBoxTranslateUnit->findData(QVariant(UnitsToStr(Unit::Px)));
            if (unitIndex != -1)
            {
                ui->comboBoxTranslateUnit->setCurrentIndex(unitIndex);
            }
        }
    }
    else if (ui->tabWidgetPieceTransformation->indexOf(ui->tabRotate) == index)
    { // rotate
        ui->doubleSpinBoxCurrentPieceAngle->setValue(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_RelativeTranslationChanged(bool checked)
{
    if (checked)
    {
        ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(0);
        ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(0);
    }
    else
    {
        QRectF rect = PiecesBoundingRect(SelectedPieces());

        ui->doubleSpinBoxCurrentPieceBoxPositionX->setValue(
            UnitConvertor(rect.topLeft().x(), Unit::Px, TranslateUnit()));
        ui->doubleSpinBoxCurrentPieceBoxPositionY->setValue(
            UnitConvertor(rect.topLeft().y(), Unit::Px, TranslateUnit()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_ConvertPaperSize()
{
    const Unit layoutUnit = LayoutUnit();

    const qreal sheetWidth = ui->doubleSpinBoxSheetPaperWidth->value();
    const qreal sheetHeight = ui->doubleSpinBoxSheetPaperHeight->value();

    const qreal sheetLeftMargin = ui->doubleSpinBoxSheetMarginLeft->value();
    const qreal sheetRightMargin = ui->doubleSpinBoxSheetMarginRight->value();
    const qreal sheetTopMargin = ui->doubleSpinBoxSheetMarginTop->value();
    const qreal sheetBottomMargin = ui->doubleSpinBoxSheetMarginBottom->value();

    ui->doubleSpinBoxSheetPaperWidth->blockSignals(true);
    ui->doubleSpinBoxSheetPaperHeight->blockSignals(true);
    ui->doubleSpinBoxSheetPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
    ui->doubleSpinBoxSheetPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
    ui->doubleSpinBoxSheetPaperWidth->blockSignals(false);
    ui->doubleSpinBoxSheetPaperHeight->blockSignals(false);

    const qreal newSheetWidth = UnitConvertor(sheetWidth, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetHeight = UnitConvertor(sheetHeight, m_oldLayoutUnit, layoutUnit);

    const qreal newSheetLeftMargin = UnitConvertor(sheetLeftMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetRightMargin = UnitConvertor(sheetRightMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetTopMargin = UnitConvertor(sheetTopMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newSheetBottomMargin = UnitConvertor(sheetBottomMargin, m_oldLayoutUnit, layoutUnit);

    const qreal tileWidth = ui->doubleSpinBoxTilePaperWidth->value();
    const qreal tileHeight = ui->doubleSpinBoxTilePaperHeight->value();

    const qreal tileLeftMargin = ui->doubleSpinBoxTileMarginLeft->value();
    const qreal tileRightMargin = ui->doubleSpinBoxTileMarginRight->value();
    const qreal tileTopMargin = ui->doubleSpinBoxTileMarginTop->value();
    const qreal tileBottomMargin = ui->doubleSpinBoxTileMarginBottom->value();

    ui->doubleSpinBoxTilePaperWidth->blockSignals(true);
    ui->doubleSpinBoxTilePaperHeight->blockSignals(true);
    ui->doubleSpinBoxTilePaperWidth->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
    ui->doubleSpinBoxTilePaperHeight->setMaximum(FromPixel(QIMAGE_MAX, layoutUnit));
    ui->doubleSpinBoxTilePaperWidth->blockSignals(false);
    ui->doubleSpinBoxTilePaperHeight->blockSignals(false);

    const qreal newTileWidth = UnitConvertor(tileWidth, m_oldLayoutUnit, layoutUnit);
    const qreal newTileHeight = UnitConvertor(tileHeight, m_oldLayoutUnit, layoutUnit);

    const qreal newTileLeftMargin = UnitConvertor(tileLeftMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newTileRightMargin = UnitConvertor(tileRightMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newTileTopMargin = UnitConvertor(tileTopMargin, m_oldLayoutUnit, layoutUnit);
    const qreal newTileBottomMargin = UnitConvertor(tileBottomMargin, m_oldLayoutUnit, layoutUnit);

    qreal newGap = UnitConvertor(ui->doubleSpinBoxSheetPiecesGap->value(), m_oldLayoutUnit, layoutUnit);

    m_oldLayoutUnit = layoutUnit;
    m_layout->LayoutSettings().SetUnit(layoutUnit);
    CorrectPaperDecimals();
    MinimumSheetPaperSize();
    MinimumTilePaperSize();

    const QString suffix = " " + UnitsToStr(layoutUnit, true);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperWidth, newSheetWidth);
    ui->doubleSpinBoxSheetPaperWidth->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperHeight, newSheetHeight);
    ui->doubleSpinBoxSheetPaperHeight->setSuffix(suffix);

    on_SheetSizeChanged();

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginLeft, newSheetLeftMargin);
    ui->doubleSpinBoxSheetMarginLeft->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginRight, newSheetRightMargin);
    ui->doubleSpinBoxSheetMarginRight->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginTop, newSheetTopMargin);
    ui->doubleSpinBoxSheetMarginTop->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginBottom, newSheetBottomMargin);
    ui->doubleSpinBoxSheetMarginBottom->setSuffix(suffix);

    on_SheetMarginChanged();

    SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperWidth, newTileWidth);
    ui->doubleSpinBoxTilePaperWidth->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxTilePaperHeight, newTileHeight);
    ui->doubleSpinBoxTilePaperHeight->setSuffix(suffix);

    on_TilesSizeChanged();

    SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginLeft, newTileLeftMargin);
    ui->doubleSpinBoxTileMarginLeft->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginRight, newTileRightMargin);
    ui->doubleSpinBoxTileMarginRight->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginTop, newTileTopMargin);
    ui->doubleSpinBoxTileMarginTop->setSuffix(suffix);

    SetDoubleSpinBoxValue(ui->doubleSpinBoxTileMarginBottom, newTileBottomMargin);
    ui->doubleSpinBoxTileMarginBottom->setSuffix(suffix);

    on_TilesMarginChanged();

    ui->doubleSpinBoxSheetPiecesGap->setMaximum(
        UnitConvertor(VPSettings::GetMaxLayoutPieceGap(), Unit::Cm, layoutUnit));
    ui->doubleSpinBoxSheetPiecesGap->setValue(newGap);
    ui->doubleSpinBoxSheetPiecesGap->setSuffix(suffix);

    CorrectMaxMargins();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_ExportLayout()
{
    if (m_layout.isNull())
    {
        return;
    }

    QList<VPSheetPtr> sheets = m_layout->GetSheets();
    if (sheets.isEmpty())
    {
        return;
    }

    if (not AskLayoutIsInvalid(sheets))
    {
        return;
    }

    const QString layoutTitle = m_layout->LayoutSettings().GetTitle();
    const QString fileName = layoutTitle.isEmpty() ? QFileInfo(curFile).baseName() : layoutTitle;

    DialogSaveManualLayout dialog(sheets.size(), false, fileName, this);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    if (dialog.FileName().isEmpty())
    {
        return;
    }

    VPExportData data;
    data.format = dialog.Format();
    data.path = dialog.Path();
    data.fileName = dialog.FileName();
    data.sheets = sheets;
    data.xScale = m_layout->LayoutSettings().HorizontalScale();
    data.yScale = m_layout->LayoutSettings().VerticalScale();
    data.isBinaryDXF = dialog.IsBinaryDXFFormat();
    data.textAsPaths = dialog.IsTextAsPaths();
    data.exportUnified = dialog.IsExportUnified();
    data.showTilesScheme = dialog.IsTilesScheme();
    data.showGrainline = dialog.IsShowGrainline();

    ExportData(data);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_ExportSheet()
{
    if (m_layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    if (not AskLayoutIsInvalid(QList<VPSheetPtr>{sheet}))
    {
        return;
    }

    const QString sheetTitle = sheet->GetName();
    const QString defaultName = not curFile.isEmpty() ? QFileInfo(curFile).baseName() : tr("sheet");
    const QString fileName = sheetTitle.isEmpty() ? defaultName : sheetTitle;

    DialogSaveManualLayout dialog(1, false, fileName, this);

    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    if (dialog.FileName().isEmpty())
    {
        return;
    }

    VPExportData data;
    data.format = dialog.Format();
    data.path = dialog.Path();
    data.fileName = dialog.FileName();
    data.sheets = QList<VPSheetPtr>{sheet};
    data.xScale = m_layout->LayoutSettings().HorizontalScale();
    data.yScale = m_layout->LayoutSettings().VerticalScale();
    data.isBinaryDXF = dialog.IsBinaryDXFFormat();
    data.textAsPaths = dialog.IsTextAsPaths();
    data.exportUnified = dialog.IsExportUnified();
    data.showTilesScheme = dialog.IsTilesScheme();
    data.showGrainline = dialog.IsShowGrainline();

    ExportData(data);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintLayout_triggered()
{
    if (not m_layout->IsSheetsUniform())
    {
        qCritical() << tr("For printing multipages document all sheet should have the same size.");
        return;
    }

    if (not AskLayoutIsInvalid(m_layout->GetSheets()))
    {
        return;
    }

    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter(), QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    QList<VPSheetPtr> sheets = m_layout->GetSheets();
    const VPSheetPtr &firstSheet = sheets.constFirst();
    if (firstSheet.isNull())
    {
        qCritical() << tr("Unable to get sheet page settings");
    }

    qreal xScale = m_layout->LayoutSettings().HorizontalScale();
    qreal yScale = m_layout->LayoutSettings().VerticalScale();

    SetPrinterSheetPageSettings(printer, firstSheet, xScale, yScale);
    printer->setDocName(m_layout->LayoutSettings().GetTitle());
    printer->setOutputFileName(QString()); // Disable printing to file if was enabled.
    printer->setOutputFormat(QPrinter::NativeFormat);

    QPrintDialog dialog(printer.data(), this);
    // If only user couldn't change page margins we could use method setMinMax();
    dialog.setOption(QPrintDialog::PrintCurrentPage, false);

    if (dialog.exec() == QDialog::Accepted)
    {
        printer->setResolution(static_cast<int>(PrintDPI));
        on_printLayoutSheets(printer.data());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintPreviewLayout_triggered()
{
    if (not m_layout->IsSheetsUniform())
    {
        qCritical() << tr("For printing multipages document all sheet should have the same size.");
        return;
    }

    if (not AskLayoutIsInvalid(m_layout->GetSheets()))
    {
        return;
    }

    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter());
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    QList<VPSheetPtr> sheets = m_layout->GetSheets();
    const VPSheetPtr &firstSheet = sheets.constFirst();
    if (firstSheet.isNull())
    {
        qCritical() << tr("Unable to get sheet page settings");
        return;
    }

    qreal xScale = m_layout->LayoutSettings().HorizontalScale();
    qreal yScale = m_layout->LayoutSettings().VerticalScale();

    SetPrinterSheetPageSettings(printer, firstSheet, xScale, yScale);
    printer->setDocName(m_layout->LayoutSettings().GetTitle());
    printer->setResolution(static_cast<int>(PrintDPI));

    // display print preview dialog
    QPrintPreviewDialog preview(printer.data());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &VPMainWindow::on_printLayoutSheets);
    preview.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintTiledLayout_triggered()
{
    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter(), QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    QList<VPSheetPtr> sheets = m_layout->GetSheets();
    const VPSheetPtr &firstSheet = sheets.constFirst();
    if (firstSheet.isNull())
    {
        qCritical() << tr("Unable to get sheet page settings");
        return;
    }

    SetPrinterTiledPageSettings(printer, m_layout, firstSheet, m_layout->LayoutSettings().GetTilesOrientation(), false);
    printer->setDocName(m_layout->LayoutSettings().GetTitle());
    printer->setOutputFileName(QString()); // Disable printing to file if was enabled.
    printer->setOutputFormat(QPrinter::NativeFormat);

    QPrintDialog dialog(printer.data(), this);
    // If only user couldn't change page margins we could use method setMinMax();
    dialog.setOption(QPrintDialog::PrintCurrentPage, false);

    if (dialog.exec() == QDialog::Accepted)
    {
        printer->setResolution(static_cast<int>(PrintDPI));
        on_printLayoutTiledPages(printer.data());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintPreviewTiledLayout_triggered()
{
    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter(), QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    QList<VPSheetPtr> sheets = m_layout->GetSheets();
    const VPSheetPtr &firstSheet = sheets.constFirst();
    if (firstSheet.isNull())
    {
        qCritical() << tr("Unable to get sheet page settings");
        return;
    }

    SetPrinterTiledPageSettings(printer, m_layout, firstSheet, m_layout->LayoutSettings().GetTilesOrientation(), false);
    printer->setDocName(m_layout->LayoutSettings().GetTitle());
    printer->setResolution(static_cast<int>(PrintDPI));

    // display print preview dialog
    QPrintPreviewDialog preview(printer.data());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &VPMainWindow::on_printLayoutTiledPages);
    preview.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_printLayoutSheets(QPrinter *printer)
{
    PrintLayoutSheets(printer, m_layout->GetSheets());
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_printLayoutTiledPages(QPrinter *printer)
{
    PrintLayoutTiledSheets(printer, m_layout->GetSheets());
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintSheet_triggered()
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    if (not AskLayoutIsInvalid(QList<VPSheetPtr>{sheet}))
    {
        return;
    }

    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter(), QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    qreal xScale = m_layout->LayoutSettings().HorizontalScale();
    qreal yScale = m_layout->LayoutSettings().VerticalScale();

    SetPrinterSheetPageSettings(printer, sheet, xScale, yScale);
    printer->setDocName(sheet->GetName());
    printer->setOutputFileName(QString()); // Disable printing to file if was enabled.
    printer->setOutputFormat(QPrinter::NativeFormat);

    QPrintDialog dialog(printer.data(), this);
    // If only user couldn't change page margins we could use method setMinMax();
    dialog.setOption(QPrintDialog::PrintCurrentPage, false);

    if (dialog.exec() == QDialog::Accepted)
    {
        printer->setResolution(static_cast<int>(PrintDPI));
        on_printLayoutSheet(printer.data());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintPreviewSheet_triggered()
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    if (not AskLayoutIsInvalid(QList<VPSheetPtr>{sheet}))
    {
        return;
    }

    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter());
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    qreal xScale = m_layout->LayoutSettings().HorizontalScale();
    qreal yScale = m_layout->LayoutSettings().VerticalScale();

    SetPrinterSheetPageSettings(printer, sheet, xScale, yScale);
    printer->setDocName(sheet->GetName());
    printer->setResolution(static_cast<int>(PrintDPI));

    // display print preview dialog
    QPrintPreviewDialog preview(printer.data());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &VPMainWindow::on_printLayoutSheet);
    preview.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintTiledSheet_triggered()
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter(), QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    SetPrinterTiledPageSettings(printer, m_layout, sheet, m_layout->LayoutSettings().GetTilesOrientation(), false);
    printer->setDocName(sheet->GetName());
    printer->setOutputFileName(QString()); // Disable printing to file if was enabled.
    printer->setOutputFormat(QPrinter::NativeFormat);

    QPrintDialog dialog(printer.data(), this);
    // If only user couldn't change page margins we could use method setMinMax();
    dialog.setOption(QPrintDialog::PrintCurrentPage, false);

    if (dialog.exec() == QDialog::Accepted)
    {
        printer->setResolution(static_cast<int>(PrintDPI));
        on_printLayoutSheetTiledPages(printer.data());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPrintPreviewTiledSheet_triggered()
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    QSharedPointer<QPrinter> printer = PreparePrinter(QPrinterInfo::defaultPrinter(), QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    printer->setCreator(QGuiApplication::applicationDisplayName() + QChar(QChar::Space) +
                        QCoreApplication::applicationVersion());

    SetPrinterTiledPageSettings(printer, m_layout, sheet, m_layout->LayoutSettings().GetTilesOrientation(), false);
    printer->setDocName(sheet->GetName());
    printer->setResolution(static_cast<int>(PrintDPI));

    // display print preview dialog
    QPrintPreviewDialog preview(printer.data());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &VPMainWindow::on_printLayoutSheetTiledPages);
    preview.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_printLayoutSheet(QPrinter *printer)
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    PrintLayoutSheets(printer, QList<VPSheetPtr>{sheet});
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_printLayoutSheetTiledPages(QPrinter *printer)
{
    VPSheetPtr sheet = m_layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    PrintLayoutTiledSheets(printer, QList<VPSheetPtr>{sheet});
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CreateWatermark()
{
    CleanWaterkmarkEditors();
    OpenWatermark();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::EditCurrentWatermark()
{
    CleanWaterkmarkEditors();

    QString watermarkFile = m_layout->LayoutSettings().WatermarkPath();
    if (not watermarkFile.isEmpty())
    {
        OpenWatermark(watermarkFile);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::LoadWatermark()
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

    m_layout->LayoutSettings().SetWatermarkPath(filePath);
    LayoutWasSaved(false);
    m_layout->TileFactory()->RefreshWatermarkData();
    m_graphicsView->RefreshLayout();
    ui->actionRemoveWatermark->setEnabled(true);
    ui->actionEditCurrentWatermark->setEnabled(true);

    if (not m_layout->LayoutSettings().WatermarkPath().isEmpty())
    {
        m_layoutWatcher->addPath(m_layout->LayoutSettings().WatermarkPath());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::RemoveWatermark()
{
    m_layout->LayoutSettings().SetWatermarkPath(QString());
    LayoutWasSaved(false);
    m_layout->TileFactory()->RefreshWatermarkData();
    m_graphicsView->RefreshLayout();
    ui->actionRemoveWatermark->setEnabled(false);
    ui->actionEditCurrentWatermark->setEnabled(false);

    if (not m_layout->LayoutSettings().WatermarkPath().isEmpty())
    {
        m_layoutWatcher->removePath(m_layout->LayoutSettings().WatermarkPath());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::AskDefaultSettings()
{
    if (m_cmd->IsGuiEnabled())
    {
        auto *settings = VPApplication::VApp()->PuzzleSettings();
        if (not settings->IsLocaleSelected())
        {
            QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            DialogSelectLanguage dialog(this);
            QGuiApplication::restoreOverrideCursor();
            dialog.setWindowModality(Qt::WindowModal);
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

            const qint64 uptime = VPApplication::VApp()->AppUptime();
            freshID ? statistic->SendAppFreshInstallEvent(uptime) : statistic->SendAppStartEvent(uptime);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::HorizontalScaleChanged(double value)
{
    if (m_layout.isNull())
    {
        return;
    }

    m_layout->LayoutSettings().SetHorizontalScale(value / 100.);

    if (m_scaleConnected)
    {
        ui->doubleSpinBoxVerticalScale->blockSignals(true);
        ui->doubleSpinBoxVerticalScale->setValue(value);
        ui->doubleSpinBoxVerticalScale->blockSignals(false);

        m_layout->LayoutSettings().SetVerticalScale(value / 100.);
    }

    LayoutWasSaved(false);
    m_layout->TileFactory()->RefreshTileInfos();
    m_graphicsView->RefreshLayout();

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::VerticalScaleChanged(double value)
{
    if (m_layout.isNull())
    {
        return;
    }

    m_layout->LayoutSettings().SetVerticalScale(value / 100.);

    if (m_scaleConnected)
    {
        ui->doubleSpinBoxHorizontalScale->blockSignals(true);
        ui->doubleSpinBoxHorizontalScale->setValue(value);
        ui->doubleSpinBoxHorizontalScale->blockSignals(false);

        m_layout->LayoutSettings().SetHorizontalScale(value / 100.);
    }

    LayoutWasSaved(false);
    m_layout->TileFactory()->RefreshTileInfos();
    m_graphicsView->RefreshLayout();

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::LayoutWarningPiecesSuperposition_toggled(bool checked)
{
    if (not m_layout.isNull())
    {
        m_layout->LayoutSettings().SetWarningSuperpositionOfPieces(checked);
        LayoutWasSaved(false);
        if (checked)
        {
            VPSheetPtr sheet = m_layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                sheet->ValidateSuperpositionOfPieces();
            }
        }
        m_graphicsView->RefreshPieces();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::LayoutWarningPiecesOutOfBound_toggled(bool checked)
{
    if (not m_layout.isNull())
    {
        m_layout->LayoutSettings().SetWarningPiecesOutOfBound(checked);
        LayoutWasSaved(false);

        if (checked)
        {
            VPSheetPtr sheet = m_layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                sheet->ValidatePiecesOutOfBound();
            }
        }
        m_graphicsView->RefreshPieces();
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void VPMainWindow::AboutToShowDockMenu()
{
    if (QMenu *menu = qobject_cast<QMenu *>(sender()))
    {
        menu->clear();
        CreateWindowMenu(menu);

        menu->addSeparator();

        QAction *actionPreferences = menu->addAction(tr("Preferences"));
        actionPreferences->setMenuRole(QAction::NoRole);
        connect(actionPreferences, &QAction::triggered, this, &VPMainWindow::on_actionPreferences_triggered);
    }
}
#endif // defined(Q_OS_MAC)
