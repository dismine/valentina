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

#include <QFileDialog>
#include <QCloseEvent>
#include <QtMath>
#include <QSvgGenerator>
#include <QFileSystemWatcher>
#include <QSaveFile>
#include <QUndoStack>

#include "ui_vpmainwindow.h"
#include "dialogs/vpdialogabout.h"
#include "xml/vplayoutfilewriter.h"
#include "xml/vplayoutfilereader.h"
#include "vpapplication.h"
#include "../vlayout/vrawlayout.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/projectversion.h"
#include "../ifc/xml/vlayoutconverter.h"
#include "../ifc/exception/vexception.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "layout/vpsheet.h"
#include "dialogs/dialogpuzzlepreferences.h"
#include "undocommands/vpundoaddsheet.h"
#include "undocommands/vpundopiecerotate.h"
#include "undocommands/vpundopiecemove.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pWindow, "p.window")

QT_WARNING_POP

namespace
{
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
    for (const auto& item : selectedPieces)
    {
        if (not item.isNull())
        {
            rect = rect.united(item->MappedDetailBoundingRect());
        }
    }

    return rect;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
VPMainWindow::VPMainWindow(const VPCommandLinePtr &cmd, QWidget *parent) :
    VAbstractMainWindow(parent),
    ui(new Ui::VPMainWindow),
    m_cmd(cmd),
    m_undoStack(new QUndoStack(this)),
    m_layout{VPLayout::CreateLayout(m_undoStack)},
    m_statusLabel(new QLabel(this)),
    m_layoutWatcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);

    connect(m_layout.get(), &VPLayout::PieceSelectionChanged, this, &VPMainWindow::on_PieceSelectionChanged);
    connect(m_layout.get(), &VPLayout::LayoutChanged, this, [this]()
    {
        LayoutWasSaved(false);
    });
    connect(m_layout.get(), &VPLayout::PieceTransformationChanged, this, [this]()
    {
        SetPropertyTabCurrentPieceData();
    });

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean)
    {
        LayoutWasSaved(clean);
    });

    // init the tile factory
    m_tileFactory = new VPTileFactory(m_layout, VPApplication::VApp()->Settings());
    m_tileFactory->refreshTileInfos();

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
#endif //defined(Q_OS_MAC)

    connect(m_layoutWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path)
    {
        if (not curFile.isEmpty() && curFile == path)
        {
            UpdateWindowTitle();
        }
    });

    m_graphicsView->RefreshLayout();
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
auto VPMainWindow::LoadFile(QString path) -> bool
{
    if (not QFileInfo::exists(path))
    {
        qCCritical(pWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
        if (m_cmd->IsTestModeEnabled())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    // Check if file already opened
    QList<VPMainWindow*> list = VPApplication::VApp()->MainWindows();
    auto w = std::find_if(list.begin(), list.end(),
                          [path](VPMainWindow *window) { return window->CurrentFile() == path; });
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
        path = converter.Convert();
    }
    catch (VException &e)
    {
        qCCritical(pWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return false;
    }

    QFile file(path);
    file.open(QIODevice::ReadOnly);

    VPLayoutFileReader fileReader;
    m_layout->Clear();

    fileReader.ReadFile(m_layout, &file);

    if (fileReader.hasError())
    {
        qCCritical(pWindow, "%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(tr("Unable to read a layout file")));
        lock.reset();

        if (m_cmd->IsTestModeEnabled())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    SetCurrentFile(path);

    m_layout->SetFocusedSheet();

    m_oldLayoutUnit = m_layout->LayoutSettings().GetUnit();

    // updates the properties with the loaded data
    SetPropertiesData();

    m_carrousel->Refresh();
    m_graphicsView->RefreshLayout();
    m_graphicsView->RefreshPieces();
    m_tileFactory->refreshTileInfos();
    m_layout->CheckPiecesPositionValidity();
    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::LayoutWasSaved(bool saved)
{
    setWindowModified(!saved);
    not IsLayoutReadOnly() ? ui->actionSave->setEnabled(!saved): ui->actionSave->setEnabled(false);
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
    // cppcheck-suppress ConfigurationNotChecked
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
    VRawLayout rawLayoutReader;

    for(const auto &path : rawLayouts)
    {
        VRawLayoutData data;
        if (rawLayoutReader.ReadFile(path, data))
        {
            for (const auto& rawPiece : data.pieces)
            {
                // TODO / FIXME: make a few tests, on the data to check for validity. If not
                //
                // If seam allowance enabled, but the path is empty — invalid.
                // If seam line path not hidden, but the path is empty — invalid.
                // If seam allowance is built-in, but the seam line path is empty — invalid.


                // TODO for feature "Update piece" : CreateOrUpdate() function indstead of CreatePiece()
                VPPiecePtr piece(CreatePiece(rawPiece));
                piece->SetSheet(nullptr); // just in case
                VPLayout::AddPiece(m_layout, piece);
            }

            m_carrousel->Refresh();
            LayoutWasSaved(false);
        }
        else
        {
            qCCritical(pWindow, "%s\n", qPrintable(tr("Could not extract data from file '%1'. %2")
                                                    .arg(path, rawLayoutReader.ErrorString())));
            if (m_cmd != nullptr && not m_cmd->IsGuiEnabled())
            {
                m_cmd->ShowHelp(V_EX_DATAERR);
            }
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
VPPiece* VPMainWindow::CreatePiece(const VLayoutPiece &rawPiece)
{
    auto *piece = new VPPiece(rawPiece);


    // cutting line : GetMappedSeamAllowancePoints();
    // seamline : GetMappedContourPoints();

    // rawPiece.IsGrainlineEnabled() , GrainlineAngle , GetGrainline


    // TODO : set all the information we need for the piece!


    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetupMenu()
{
    // most of the actions are connected through name convention (auto-connection)

    // -------------------- connects the actions for the file menu
    ui->actionNew->setShortcuts(QKeySequence::New);
    ui->actionSave->setShortcuts(QKeySequence::Save);
    ui->actionSaveAs->setShortcuts(QKeySequence::SaveAs);

    connect(ui->actionExit, &QAction::triggered, this, &VPMainWindow::close);
    ui->actionExit->setShortcuts(QKeySequence::Quit);

    // -------------------- connects the actions for the windows menu
    // TODO : initialise the entries for the different windows

    // Add dock properties action
    QAction* actionDockWidgetToolOptions = ui->dockWidgetProperties->toggleViewAction();
    ui->menuEdit->addAction(actionDockWidgetToolOptions);

    auto *separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->menuEdit->addAction(separatorAct);

    // Add Undo/Redo actions.
    undoAction = m_layout->UndoStack()->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon::fromTheme("edit-undo"));
    ui->menuEdit->addAction(undoAction);
    ui->toolBarUndoCommands->addAction(undoAction);

    redoAction = m_layout->UndoStack()->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon::fromTheme("edit-redo"));
    ui->menuEdit->addAction(redoAction);
    ui->toolBarUndoCommands->addAction(redoAction);

    // File
    m_recentFileActs.fill(nullptr);
    for (auto & recentFileAct : m_recentFileActs)
    {
        auto *action = new QAction(this);
        recentFileAct = action;
        connect(action, &QAction::triggered, this, [this]()
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

    // Window
    connect(ui->menuWindow, &QMenu::aboutToShow, this, [this]()
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
    connect(ui->lineEditCurrentPieceName, &QLineEdit::textEdited, this, [this](const QString &text)
    {
        QList<VPPiecePtr> selectedPieces = SelectedPieces();
        if (selectedPieces.size() == 1)
        {
            VPPiecePtr selectedPiece = selectedPieces.first();
            if (not selectedPiece.isNull())
            {
                selectedPiece->SetName(text);
                LayoutWasSaved(false);
            }
        }
    });

    connect(ui->plainTextEditCurrentPieceUUID, &QPlainTextEdit::textChanged, this, [this]()
    {
        QList<VPPiecePtr> selectedPieces = SelectedPieces();
        if (selectedPieces.size() == 1)
        {
            VPPiecePtr selectedPiece = selectedPieces.first();
            if (not selectedPiece.isNull())
            {
                const QUuid temp = QUuid(ui->plainTextEditCurrentPieceUUID->toPlainText());
                if (not temp.isNull())
                {
                    selectedPiece->SetUUID(temp);
                }
                LayoutWasSaved(false);
            }
        }
    });

    connect(ui->checkBoxCurrentPieceShowSeamline, &QCheckBox::toggled, this, [this](bool checked)
    {
        QList<VPPiecePtr> selectedPieces = SelectedPieces();
        if (selectedPieces.size() == 1)
        {
            VPPiecePtr selectedPiece = selectedPieces.first();
            if (not selectedPiece.isNull())
            {
//                selectedPiece->SetShowSeamline(checked);
//                LayoutWasSaved(false);
            }
        }
    });

    connect(ui->checkBoxCurrentPieceMirrorPiece, &QCheckBox::toggled, this, [this](bool checked)
    {
        QList<VPPiecePtr> selectedPieces = SelectedPieces();
        if (selectedPieces.size() == 1)
        {
            VPPiecePtr selectedPiece = selectedPieces.first();
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

    connect(ui->comboBoxTranslateUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
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
    connect(ui->lineEditSheetName, &QLineEdit::textEdited, this, [this](const QString &text)
    {
        if (not m_layout.isNull())
        {
            VPSheetPtr sheet = m_layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                sheet->SetName(text);
                LayoutWasSaved(false);

                if(m_carrousel != nullptr)
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

    connect(ui->comboBoxLayoutUnit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VPMainWindow::on_ConvertPaperSize);

    // -------------------- sheet template ---------------------------
    VAbstractLayoutDialog::InitTemplates(ui->comboBoxSheetTemplates);

    connect(ui->comboBoxSheetTemplates, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]{SheetSize(SheetTemplate());});

    // -------------------- paper size ---------------------------
    MinimumSheetPaperSize();

    const QString suffix = " " + UnitsToStr(LayoutUnit(), true);

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

    // -------------------- margins  ------------------------
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

    connect(ui->checkBoxLayoutIgnoreFileds, &QCheckBox::stateChanged, this, [this](int state)
    {
        if (not m_layout.isNull())
        {
            ui->doubleSpinBoxSheetMarginLeft->setDisabled(state != 0);
            ui->doubleSpinBoxSheetMarginRight->setDisabled(state != 0);
            ui->doubleSpinBoxSheetMarginTop->setDisabled(state != 0);
            ui->doubleSpinBoxSheetMarginBottom->setDisabled(state != 0);

            m_layout->LayoutSettings().SetIgnoreMargins(state != 0);
            LayoutWasSaved(false);
            m_tileFactory->refreshTileInfos();
            m_graphicsView->RefreshLayout();

            VPSheetPtr sheet = m_layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                sheet->ValidatePiecesOutOfBound();
            }
        }
    });

    ui->groupBoxSheetGrid->setVisible(false); // temporary hide
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabTiles()
{
    // -------------------- tiles template
    VAbstractLayoutDialog::InitTileTemplates(ui->comboBoxTileTemplates, true);

    connect(ui->comboBoxTileTemplates, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]{TileSize(TileTemplate());});

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

    connect(ui->checkBoxTileIgnoreFileds, &QCheckBox::stateChanged, this, [this](int state)
    {
        if (not m_layout.isNull())
        {
            ui->doubleSpinBoxTileMarginLeft->setDisabled(state != 0);
            ui->doubleSpinBoxTileMarginRight->setDisabled(state != 0);
            ui->doubleSpinBoxTileMarginTop->setDisabled(state != 0);
            ui->doubleSpinBoxTileMarginBottom->setDisabled(state != 0);

            m_layout->LayoutSettings().SetIgnoreTilesMargins(state != 0);
            LayoutWasSaved(false);
            m_tileFactory->refreshTileInfos();
            m_graphicsView->RefreshLayout();
            VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
        }
    });

    // -------------------- control  ------------------------
    connect(ui->checkBoxTilesShowTiles, &QCheckBox::toggled, this, [this](bool checked)
    {
        if (not m_layout.isNull())
        {
            m_layout->LayoutSettings().SetShowTiles(checked);
            LayoutWasSaved(false);
            m_graphicsView->RefreshLayout();
            VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
        }
    });
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabLayout()
{
    connect(ui->lineEditLayoutName, &QLineEdit::textEdited, this, [this](const QString &text)
    {
        if (not m_layout.isNull())
        {
            m_layout->LayoutSettings().SetTitle(text);
            LayoutWasSaved(false);
        }
    });

    connect(ui->plainTextEditLayoutDescription, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (not m_layout.isNull())
        {
            m_layout->LayoutSettings().SetDescription(ui->plainTextEditLayoutDescription->toPlainText());
            LayoutWasSaved(false);
        }
    });

    connect(ui->checkBoxLayoutWarningPiecesSuperposition, &QCheckBox::toggled, this, [this](bool checked)
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
    });

    connect(ui->checkBoxLayoutWarningPiecesOutOfBound, &QCheckBox::toggled, this, [this](bool checked)
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
    });

    connect(ui->checkBoxSheetStickyEdges, &QCheckBox::toggled, this, [this](bool checked)
    {
        if (not m_layout.isNull())
        {
            m_layout->LayoutSettings().SetStickyEdges(checked);
            LayoutWasSaved(false);
        }
    });

    connect(ui->checkBoxFollowGainline, &QCheckBox::toggled, this, [this](bool checked)
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

    connect(ui->pushButtonLayoutExport, &QPushButton::clicked, this, [this]()
    {
        if (not m_layout.isNull())
        {
            // TODO export layout
        }
    });
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

    if(selectedPieces.isEmpty())
    {
        // show the content "no piece selected"
        ui->labelCurrentPieceNoPieceSelected->setVisible(true);

        ui->groupBoxCurrentPieceInfo->setVisible(false);
        ui->groupBoxPieceTransformation->setVisible(false);
        ui-> groupBoxCurrentPieceSeamline->setVisible(false);
        ui->groupBoxCurrentPieceGeometry->setVisible(false);
    }
    else if(selectedPieces.count() == 1)
    {
        ui->labelCurrentPieceNoPieceSelected->setVisible(false);

        ui->groupBoxCurrentPieceInfo->setVisible(true);
        ui->groupBoxPieceTransformation->setVisible(true);
        ui-> groupBoxCurrentPieceSeamline->setVisible(true);
        ui->groupBoxCurrentPieceGeometry->setVisible(true);

        VPPiecePtr selectedPiece = selectedPieces.first();

        // set the value to the current piece
        SetLineEditValue(ui->lineEditCurrentPieceName, selectedPiece->GetName());
        SetPlainTextEditValue(ui->plainTextEditCurrentPieceUUID, selectedPiece->GetUUID().toString());

//        SetCheckBoxValue(ui->checkBoxCurrentPieceShowSeamline, selectedPiece->GetShowSeamLine());
        SetCheckBoxValue(ui->checkBoxCurrentPieceMirrorPiece, selectedPiece->IsMirror());

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

        ui->labelCurrentPieceNoPieceSelected->setVisible(true);

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
        ui->groupBoxSheetInfos->setDisabled(false);
        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        SetLineEditValue(ui->lineEditSheetName, not sheet.isNull() ? sheet->GetName() : QString());

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
        QSizeF size = m_layout->LayoutSettings().GetSheetSizeConverted();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperWidth, size.width());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPaperHeight, size.height());

        SheetPaperSizeChanged();
        FindSheetTemplate();

        // set margins
        ui->groupBoxSheetMargin->setDisabled(false);
        QMarginsF margins = m_layout->LayoutSettings().GetSheetMarginsConverted();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginLeft, margins.left());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginTop, margins.top());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginRight, margins.right());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginBottom, margins.bottom());

        CorrectSheetMaxMargins();

        const bool ignoreMargins = m_layout->LayoutSettings().IgnoreMargins();
        SetCheckBoxValue(ui->checkBoxLayoutIgnoreFileds, ignoreMargins);

        ui->doubleSpinBoxSheetMarginLeft->setDisabled(ignoreMargins);
        ui->doubleSpinBoxSheetMarginRight->setDisabled(ignoreMargins);
        ui->doubleSpinBoxSheetMarginTop->setDisabled(ignoreMargins);
        ui->doubleSpinBoxSheetMarginBottom->setDisabled(ignoreMargins);

        // set placement grid
        ui->groupBoxSheetGrid->setDisabled(false);
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetGridColWidth,
                              m_layout->LayoutSettings().GetGridColWidthConverted());
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetGridRowHeight,
                              m_layout->LayoutSettings().GetGridRowHeightConverted());

        SetCheckBoxValue(ui->checkBoxSheetShowGrid, m_layout->LayoutSettings().GetShowGrid());

        ui->groupBoxSheetExport->setDisabled(false);
    }
    else
    {
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

        ui->groupBoxTilesExport->setDisabled(false);
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

        ui->groupBoxTilesExport->setDisabled(true);
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
        SetCheckBoxValue(ui->checkBoxSheetStickyEdges,
                         m_layout->LayoutSettings().GetStickyEdges());
        SetCheckBoxValue(ui->checkBoxFollowGainline,
                         m_layout->LayoutSettings().GetFollowGrainline());

        // set pieces gap
        SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPiecesGap, m_layout->LayoutSettings().GetPiecesGapConverted());

        ui->doubleSpinBoxSheetPiecesGap->setSuffix(" " + UnitsToStr(LayoutUnit(), true));

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

        ui->groupBoxLayoutExport->setDisabled(true);
    }
}



//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitMainGraphics()
{
    m_graphicsView = new VPMainGraphicsView(m_layout, m_tileFactory, this);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->centralWidget->layout()->addWidget(m_graphicsView);

    m_graphicsView->RefreshLayout();

    connect(m_graphicsView, &VPMainGraphicsView::ScaleChanged, this, &VPMainWindow::on_ScaleChanged);
    connect(m_graphicsView->GetScene(), &VMainGraphicsScene::mouseMove, this, &VPMainWindow::on_MouseMoved);
    connect(m_layout.get(), &VPLayout::PieceSheetChanged, m_carrousel, &VPCarrousel::Refresh);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitZoomToolBar()
{
    if (not m_doubleSpinBoxScale.isNull())
    {
        delete m_doubleSpinBoxScale;
    }

    delete m_mouseCoordinate;

    // connect the zoom buttons and shortcuts to the slots
    QList<QKeySequence> zoomInShortcuts;
    zoomInShortcuts.append(QKeySequence(QKeySequence::ZoomIn));
    zoomInShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Plus + Qt::KeypadModifier));
    ui->actionZoomIn->setShortcuts(zoomInShortcuts);
    connect(ui->actionZoomIn, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomIn);

    QList<QKeySequence> zoomOutShortcuts;
    zoomOutShortcuts.append(QKeySequence(QKeySequence::ZoomOut));
    zoomOutShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Minus + Qt::KeypadModifier));
    ui->actionZoomOut->setShortcuts(zoomOutShortcuts);
    connect(ui->actionZoomOut, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomOut);

    QList<QKeySequence> zoomOriginalShortcuts;
    zoomOriginalShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_0));
    zoomOriginalShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_0 + Qt::KeypadModifier));
    ui->actionZoomOriginal->setShortcuts(zoomOriginalShortcuts);
    connect(ui->actionZoomOriginal, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomOriginal);

    QList<QKeySequence> zoomFitBestShortcuts;
    zoomFitBestShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Equal));
    ui->actionZoomFitBest->setShortcuts(zoomFitBestShortcuts);
    connect(ui->actionZoomFitBest, &QAction::triggered, m_graphicsView, &VPMainGraphicsView::ZoomFitBest);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitScaleToolBar()
{
    auto* zoomScale = new QLabel(tr("Scale:"), this);
    ui->toolBarScale->addWidget(zoomScale);

    m_doubleSpinBoxScale = new QDoubleSpinBox(this);
    m_doubleSpinBoxScale->setDecimals(1);
    m_doubleSpinBoxScale->setSuffix(QChar('%'));
    on_ScaleChanged(m_graphicsView->transform().m11());
    connect(m_doubleSpinBoxScale.data(), QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double d){m_graphicsView->Zoom(d/100.0);});
    ui->toolBarScale->addWidget(m_doubleSpinBoxScale);


    // define the mouse position
    ui->toolBarScale->addSeparator();

    m_mouseCoordinate = new QLabel(QStringLiteral("0, 0 (%1)")
                                   .arg(UnitsToStr(m_layout->LayoutSettings().GetUnit(), true)));
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
        int index = VPApplication::VApp()->MainWindows().indexOf(this);
        if (index != -1)
        {
            showName = tr("untitled %1.vlt").arg(index+1);
        }
        else
        {
            showName = tr("untitled.vlt");
        }
    }

    showName += QLatin1String("[*]");

    if (IsLayoutReadOnly())
    {
        showName += QStringLiteral(" (") + tr("read only") + QChar(')');
    }

    setWindowTitle(showName);
    setWindowFilePath(curFile);

#if defined(Q_OS_MAC)
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() +
                                  QLatin1String("/../Resources/layout.icns"));
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
#endif //defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ReadSettings()
{
    qCDebug(pWindow, "Reading settings.");
    const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetGeometry());
        restoreState(settings->GetWindowState());
        restoreState(settings->GetToolbarsState(), APP_VERSION);

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
    settings->SetWindowState(saveState());
    settings->SetToolbarsState(saveState(APP_VERSION));

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
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(tr("Unsaved changes"),
                                                               tr("Layout has been modified.\n"
                                                                  "Do you want to save your changes?"),
                                                               QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No,
                                                               QMessageBox::Cancel, this, Qt::Sheet));

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
void VPMainWindow::generateTiledPdf(QString fileName)
{
//    if(not fileName.isEmpty())
//    {
//        m_graphicsView->PrepareForExport();
//        m_tileFactory->refreshTileInfos();

//        PageOrientation tilesOrientation = m_layout->LayoutSettings().GetTilesOrientation();

//        // -------------  Set up the printer
//        QScopedPointer<QPrinter> printer(new QPrinter());

//        printer->setCreator(QGuiApplication::applicationDisplayName()+QChar(QChar::Space)+
//                            QCoreApplication::applicationVersion());
//        printer->setPageOrientation(QPageLayout::Portrait); // in the pdf file the pages should always be in portrait

//        // here we might need to so some rounding for the size.
//        printer->setPageSize(QPageSize(m_layout->LayoutSettings().GetTilesSize(Unit::Mm),
//                                                           QPageSize::Millimeter));
//        printer->setFullPage(true);

//        #ifdef Q_OS_MAC
//        printer->setOutputFormat(QPrinter::NativeFormat);
//        #else
//        printer->setOutputFormat(QPrinter::PdfFormat);
//        #endif

//        printer->setOutputFileName(fileName);
//        printer->setResolution(static_cast<int>(PrintDPI));
//        printer->setDocName(m_layout->GetFocusedSheet()->GetName());

//        // -------------  Set up the painter
//        QPainter painter;
//        if (not painter.begin(printer.data()))
//        { // failed to open file
//            qCritical() << tr("Failed to open file, is it writable?");
//            return;
//        }
//        painter.setFont( QFont( QStringLiteral("Arial"), 8, QFont::Normal ) );
//        painter.setRenderHint(QPainter::Antialiasing, true);
//        painter.setBrush ( QBrush ( Qt::NoBrush ) );

//        if(tilesOrientation == PageOrientation::Landscape)
//        {
//            // The landscape tiles have to be rotated, because the pages
//            // stay portrait in the pdf
//            painter.rotate(90);
//            painter.translate(0, -ToPixel(printer->pageRect(QPrinter::Millimeter).width(), Unit::Mm));
//        }

//        for(int row=0;row<m_tileFactory->getRowNb();row++)  // for each row of the tiling grid
//        {
//            for(int col=0;col<m_tileFactory->getColNb();col++) // for each column of tiling grid
//            {
//                if(not (row == 0 && col == 0))
//                {
//                    if (not printer->newPage())
//                    {
//                        qWarning("failed in flushing page to disk, disk full?");
//                        return;
//                    }
//                }

//                m_tileFactory->drawTile(&painter, m_graphicsView, row, col);
//            }
//        }

//        painter.end();

//        m_graphicsView->CleanAfterExport();
//    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::CreateWindowMenu(QMenu *menu)
{
    SCASSERT(menu != nullptr)

    QAction *action = menu->addAction(tr("&New Window"));
    connect(action, &QAction::triggered, this, []()
    {
        VPApplication::VApp()->NewMainWindow()->activateWindow();
    });
    action->setMenuRole(QAction::NoRole);
    menu->addSeparator();

    const QList<VPMainWindow*> windows = VPApplication::VApp()->MainWindows();
    for (int i = 0; i < windows.count(); ++i)
    {
        VPMainWindow *window = windows.at(i);

        QString title = QStringLiteral("%1. %2").arg(i+1).arg(window->windowTitle());
        const int index = title.lastIndexOf(QLatin1String("[*]"));
        if (index != -1)
        {
            window->isWindowModified() ? title.replace(index, 3, QChar('*')) : title.replace(index, 3, QString());
        }

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
        QAction *action = menu->addAction(title, this, SLOT(ShowWindow()));
#else
        QAction *action = menu->addAction(title, this, &VPMainWindow::ShowWindow);
#endif //QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
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

#ifdef Q_OS_WIN32
    qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/

    bool fileWritable = f.isWritable();

#ifdef Q_OS_WIN32
    qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

    return not fileWritable;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ConnectToPreferences(const QSharedPointer<DialogPuzzlePreferences> &preferences)
{
    // Must be first
    connect(preferences.get(), &DialogPuzzlePreferences::UpdateProperties, this, &VPMainWindow::WindowsLocale);
    connect(preferences.get(), &DialogPuzzlePreferences::UpdateProperties, this, &VPMainWindow::ToolBarStyles);
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
Unit VPMainWindow::TranslateUnit() const
{
    return StrToUnits(ui->comboBoxTranslateUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
Unit VPMainWindow::LayoutUnit() const
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
            return VAbstractLayoutDialog::GetTemplateSize(t, layoutUnit);
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
            return VAbstractLayoutDialog::GetTemplateSize(t, layoutUnit);
        default:
            break;
    }
    return QSizeF();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::SheetTemplate() const -> QSizeF
{
    auto t = static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ui->comboBoxSheetTemplates->currentData().toInt());
    return Template(t);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainWindow::TileTemplate() const -> QSizeF
{
    auto t = static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(ui->comboBoxTileTemplates->currentData().toInt());
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
    switch (m_oldLayoutUnit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            ui->doubleSpinBoxSheetPaperWidth->setDecimals(2);
            ui->doubleSpinBoxSheetPaperHeight->setDecimals(2);

            ui->doubleSpinBoxTilePaperWidth->setDecimals(2);
            ui->doubleSpinBoxTilePaperHeight->setDecimals(2);

            ui->doubleSpinBoxSheetMarginLeft->setDecimals(4);
            ui->doubleSpinBoxSheetMarginRight->setDecimals(4);
            ui->doubleSpinBoxSheetMarginTop->setDecimals(4);
            ui->doubleSpinBoxSheetMarginBottom->setDecimals(4);

            ui->doubleSpinBoxTileMarginLeft->setDecimals(4);
            ui->doubleSpinBoxTileMarginRight->setDecimals(4);
            ui->doubleSpinBoxTileMarginTop->setDecimals(4);
            ui->doubleSpinBoxTileMarginBottom->setDecimals(4);

            ui->doubleSpinBoxSheetPiecesGap->setDecimals(2);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxSheetPaperWidth->setDecimals(5);
            ui->doubleSpinBoxSheetPaperHeight->setDecimals(5);

            ui->doubleSpinBoxTilePaperWidth->setDecimals(5);
            ui->doubleSpinBoxTilePaperHeight->setDecimals(5);

            ui->doubleSpinBoxSheetMarginLeft->setDecimals(5);
            ui->doubleSpinBoxSheetMarginRight->setDecimals(5);
            ui->doubleSpinBoxSheetMarginTop->setDecimals(5);
            ui->doubleSpinBoxSheetMarginBottom->setDecimals(5);

            ui->doubleSpinBoxTileMarginLeft->setDecimals(5);
            ui->doubleSpinBoxTileMarginRight->setDecimals(5);
            ui->doubleSpinBoxTileMarginTop->setDecimals(5);
            ui->doubleSpinBoxTileMarginBottom->setDecimals(5);

            ui->doubleSpinBoxSheetPiecesGap->setDecimals(5);
            break;
        default:
            break;
    }
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

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
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
    for (int i=0; i < max; ++i)
    {
        const QSizeF tmplSize = VAbstractLayoutDialog::GetTemplateSize(
                    static_cast<VAbstractLayoutDialog::PaperSizeTemplate>(i), paperUnit);
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
    const qreal tileWidthMargin = (tileWidth*80.0/100.0)/2.0;
    const qreal tileHeightMargin = (tileHeight*80.0/100.0)/2.0;

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
    const qreal sheetWidthMargin = (sheetWidth*80.0/100.0)/2.0;
    const qreal sheetHeightMargin = (sheetHeight*80.0/100.0)/2.0;

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
    QList<VPSheetPtr> sheets = m_layout->GetSheets();
    for(const auto& sheet : sheets)
    {
        if (not sheet.isNull())
        {
            QList<VPPiecePtr> pieces = sheet->GetPieces();
            for(const auto& piece : pieces)
            {
                if (not piece.isNull() && piece->IsGrainlineEnabled())
                {
                    VPTransformationOrigon origin;
                    origin.custom = true;
                    piece->RotateToGrainline(origin);
                    emit m_layout->PieceTransformationChanged(piece);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionNew_triggered()
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

    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VPMainWindow::RecentFileList() const
{
    return VPApplication::VApp()->PuzzleSettings()->GetRecentFileList();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionOpen_triggered()
{
    qCDebug(pWindow, "Openning puzzle layout file.");

    const QString filter(tr("Layout files") + QLatin1String(" (*.vlt)"));
    //Use standard path to individual measurements
    const QString pathTo = VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts();

    bool usedNotExistedDir = false;
    QDir directory(pathTo);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), pathTo, filter, nullptr,
                                                       VAbstractApplication::VApp()->NativeFileDialog());

    if (not mPath.isEmpty())
    {
        VPApplication::VApp()->NewMainWindow()->LoadFile(mPath);
    }

    if (usedNotExistedDir)
    {
        QDir(pathTo).rmpath(QChar('.'));
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VPMainWindow::on_actionSave_triggered()
{
    if (curFile.isEmpty() || IsLayoutReadOnly())
    {
        return on_actionSaveAs_triggered();
    }

    if (m_curFileFormatVersion < VLayoutConverter::LayoutMaxVer
            && not ContinueFormatRewrite(m_curFileFormatVersionStr, VLayoutConverter::LayoutMaxVerStr))
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
bool VPMainWindow::on_actionSaveAs_triggered()
{
    QString filters = tr("Layout files") + QStringLiteral(" (*.vlt)");
    QString suffix = QStringLiteral("vlt");
    QString fName = tr("layout") + QChar('.') + suffix;

    QString dir;
    if (curFile.isEmpty())
    {
        dir = VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts();
    }
    else
    {
        dir = QFileInfo(curFile).absolutePath();
    }

    bool usedNotExistedDir = false;
    QDir directory(dir);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), dir + QChar('/') + fName, filters, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());

    auto RemoveTempDir = qScopeGuard([usedNotExistedDir, dir]()
    {
        if (usedNotExistedDir)
        {
            QDir(dir).rmpath(QChar('.'));
        }
    });

    if (fileName.isEmpty())
    {
        return false;
    }

    QFileInfo f( fileName );
    if (f.suffix().isEmpty() && f.suffix() != suffix)
    {
        fileName += QChar('.') + suffix;
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
        qCCritical(pWindow, "%s", qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
                                                    "Expect collissions when run 2 copies of the program.")));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionImportRawLayout_triggered()
{
    const QString filter(tr("Raw Layout files") + QLatin1String(" (*.rld)"));

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
        m_layout->LayoutSettings().SetSheetSizeConverted(
                    ui->doubleSpinBoxSheetPaperWidth->value(),
                    ui->doubleSpinBoxSheetPaperHeight->value());
        FindSheetTemplate();
        SheetPaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);

        m_tileFactory->refreshTileInfos();
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

        m_layout->LayoutSettings().SetSheetSizeConverted(height, width);

        SheetPaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);
        m_tileFactory->refreshTileInfos();
        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_pushButtonSheetRemoveUnusedLength_clicked()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::on_pushButtonSheetRemoveUnusedLength_clicked");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetMarginChanged()
{
    if (not m_layout.isNull())
    {
        m_layout->LayoutSettings().SetSheetMarginsConverted(
                    ui->doubleSpinBoxSheetMarginLeft->value(),
                    ui->doubleSpinBoxSheetMarginTop->value(),
                    ui->doubleSpinBoxSheetMarginRight->value(),
                    ui->doubleSpinBoxSheetMarginBottom->value());

        LayoutWasSaved(false);

        VPSheetPtr sheet = m_layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
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
        m_layout->LayoutSettings().SetTilesSizeConverted(
                    ui->doubleSpinBoxTilePaperWidth->value(),
                    ui->doubleSpinBoxTilePaperHeight->value());
        FindTileTemplate();
        TilePaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);

        m_tileFactory->refreshTileInfos();
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

        m_layout->LayoutSettings().SetTilesSizeConverted(height, width);

        TilePaperSizeChanged();
        CorrectMaxMargins();
        LayoutWasSaved(false);
        m_tileFactory->refreshTileInfos();
        m_graphicsView->RefreshLayout();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_TilesMarginChanged()
{
    if (not m_layout.isNull())
    {
        m_layout->LayoutSettings().SetTilesMarginsConverted(
                    ui->doubleSpinBoxTileMarginLeft->value(),
                    ui->doubleSpinBoxTileMarginTop->value(),
                    ui->doubleSpinBoxTileMarginRight->value(),
                    ui->doubleSpinBoxTileMarginBottom->value());
        LayoutWasSaved(false);
        m_tileFactory->refreshTileInfos();
        m_graphicsView->RefreshLayout();
    }

    VMainGraphicsView::NewSceneRect(m_graphicsView->scene(), m_graphicsView);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_pushButtonTilesExport_clicked()
{
    QString dir = QDir::homePath();
    QString filters(tr("PDF Files") + QLatin1String("(*.pdf)"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"),
                                                    dir + QLatin1String("/") + m_layout->GetFocusedSheet()->GetName() + QLatin1String(".pdf"),
                                                    filters, nullptr
#ifdef Q_OS_LINUX
                                                    , QFileDialog::DontUseNativeDialog
#endif
                                                    );

    generateTiledPdf(fileName);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_pushButtonSheetExport_clicked()
{
//    LayoutExportFormats format = static_cast<LayoutExportFormats>(ui->comboBoxSheetExportFormat->currentData().toInt());

//    VPExporter exporter;
//    exporter.Export(m_layout.get(), format, m_graphicsView);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_CarrouselLocationChanged(Qt::DockWidgetArea area)
{
    if(area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
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
        m_doubleSpinBoxScale->setMaximum(qFloor(VPMainGraphicsView::MaxScale()*1000)/10.0);
        m_doubleSpinBoxScale->setMinimum(qFloor(VPMainGraphicsView::MinScale()*1000)/10.0);
        m_doubleSpinBoxScale->setValue(qFloor(scale*1000)/10.0);
        m_doubleSpinBoxScale->setSingleStep(1);
        m_doubleSpinBoxScale->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_MouseMoved(const QPointF &scenePos)
{
    if (m_mouseCoordinate != nullptr)
    {
        m_mouseCoordinate->setText(QStringLiteral("%1, %2 (%3)")
                                   .arg(static_cast<qint32>(FromPixel(scenePos.x(),
                                                                      m_layout->LayoutSettings().GetUnit())))
                                   .arg(static_cast<qint32>(FromPixel(scenePos.y(),
                                                                      m_layout->LayoutSettings().GetUnit())))
                                   .arg(UnitsToStr(m_layout->LayoutSettings().GetUnit(), true)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ShowWindow() const
{
    if (auto *action = qobject_cast<QAction*>(sender()))
    {
        const QVariant v = action->data();
        if (v.canConvert<int>())
        {
            const int offset = qvariant_cast<int>(v);
            const QList<VPMainWindow*> windows = VPApplication::VApp()->MainWindows();
            windows.at(offset)->raise();
            windows.at(offset)->activateWindow();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionPreferences_triggered()
{
    // Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
    QSharedPointer<DialogPuzzlePreferences> preferences = VPApplication::VApp()->PreferencesDialog();
    if (preferences.isNull())
    {
        auto CleanAfterDialog = qScopeGuard([&preferences]()
        {
            preferences.clear();
        });

        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        preferences.reset(new DialogPuzzlePreferences());
        preferences->setWindowModality(Qt::ApplicationModal);
        VPApplication::VApp()->SetPreferencesDialog(preferences);

        const QList<VPMainWindow*> windows = VPApplication::VApp()->MainWindows();
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
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionAddSheet_triggered()
{
    VPSheetPtr sheet(new VPSheet(m_layout));
    sheet->SetName(tr("Sheet %1").arg(m_layout->GetSheets().size()+1));
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
    { // translate
        const qreal dx = UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionX->value(), TranslateUnit(), Unit::Px);
        const qreal dy = UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionY->value(), TranslateUnit(), Unit::Px);

        QList<VPPiecePtr> selectedPieces = SelectedPieces();
        if (selectedPieces.isEmpty())
        {
            return;
        }

        if (ui->checkBoxRelativeTranslation->isChecked())
        {
            if (ui->checkBoxTransformSeparately->isChecked())
            {
                if (selectedPieces.size() > 1)
                {
                    m_layout->UndoStack()->beginMacro(tr("translate pieces"));
                }

                QRectF rect = PiecesBoundingRect(selectedPieces);
                for (const auto& piece : selectedPieces)
                {
                    if (not piece.isNull())
                    {
                        const QRectF pieceRect = piece->MappedDetailBoundingRect();
                        qreal pieceDx = dx;
                        qreal pieceDy = dy;

                        if (not qFuzzyIsNull(rect.width()))
                        {
                            pieceDx += dx*((pieceRect.topLeft().x()-rect.topLeft().x())/rect.width())*2.;
                        }

                        if (not qFuzzyIsNull(rect.height()))
                        {
                            pieceDy += dy*((pieceRect.topLeft().y()-rect.topLeft().y())/rect.height())*2.;
                        }

                        auto *command = new VPUndoPieceMove(piece, pieceDx, pieceDy);
                        m_layout->UndoStack()->push(command);

                        if (m_layout->LayoutSettings().GetStickyEdges())
                        {
                            qreal stickyTranslateX = 0;
                            qreal stickyTranslateY = 0;
                            if (piece->StickyPosition(stickyTranslateX, stickyTranslateY))
                            {
                                bool allowMerge = selectedPieces.size() == 1;
                                auto *stickyCommand = new VPUndoPieceMove(piece, stickyTranslateX, stickyTranslateY,
                                                                          allowMerge);
                                m_layout->UndoStack()->push(stickyCommand);
                            }
                        }
                    }
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
                auto *command = new VPUndoPieceMove(selectedPieces.first(), pieceDx, pieceDy);
                m_layout->UndoStack()->push(command);
            }
            else
            {
                auto *command = new VPUndoPiecesMove(selectedPieces, pieceDx, pieceDy);
                m_layout->UndoStack()->push(command);
            }
        }
    }
    else if (ui->tabWidgetPieceTransformation->indexOf(ui->tabRotate) == index)
    { // rotate
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
            for (const auto& piece : selectedPieces)
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
#endif //defined(Q_OS_MAC)
