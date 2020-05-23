/************************************************************************
 **
 **  @file   puzzlemainwindow.cpp
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
#include "puzzlemainwindow.h"

#include <QFileDialog>
#include <QCloseEvent>

#include "ui_puzzlemainwindow.h"
#include "dialogs/vpdialogabout.h"
#include "xml/vpuzzlelayoutfilewriter.h"
#include "xml/vpuzzlelayoutfilereader.h"
#include "puzzleapplication.h"
#include "../vlayout/vrawlayout.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/projectversion.h"
#include "../ifc/xml/vlayoutconverter.h"
#include "../ifc/exception/vexception.h"

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pWindow, "p.window")

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::PuzzleMainWindow(const VPuzzleCommandLinePtr &cmd, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PuzzleMainWindow),
    m_cmd(cmd)
{

    m_layout = new VPuzzleLayout();

    // ----- for test purposes, to be removed------------------
    m_layout->SetLayoutMarginsConverted(2, 2, 2, 2);
    m_layout->SetLayoutSizeConverted(30.0, 45);
    m_layout->SetPiecesGapConverted(1);
    m_layout->SetUnit(Unit::Cm);
    m_layout->SetWarningSuperpositionOfPieces(true);
    // --------------------------------------------------------

    ui->setupUi(this);

    InitMenuBar();
    InitProperties();
    InitPieceCarrousel();
    InitMainGraphics();


    SetPropertiesData();

    ReadSettings();
}

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::~PuzzleMainWindow()
{
    delete ui;
    delete m_pieceCarrousel;
}

//---------------------------------------------------------------------------------------------------------------------
bool PuzzleMainWindow::LoadFile(QString path)
{
    try
    {
        VLayoutConverter converter(path);
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

    QScopedPointer<VPuzzleLayoutFileReader> fileReader(new VPuzzleLayoutFileReader());

    if(m_layout == nullptr)
    {
        m_layout = new VPuzzleLayout();
    }

    fileReader->ReadFile(m_layout, &file);

    // TODO / FIXME : better return value and error handling

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool PuzzleMainWindow::SaveFile(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);

    VPuzzleLayoutFileWriter *fileWriter = new VPuzzleLayoutFileWriter();
    fileWriter->WriteFile(m_layout, &file);

    // TODO / FIXME : better return value and error handling

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::ImportRawLayouts(const QStringList &rawLayouts)
{
    VRawLayout rawLayoutReader;

    for(auto &path : rawLayouts)
    {
        VRawLayoutData data;
        if (rawLayoutReader.ReadFile(path, data))
        {
            for (int i = 0; i < data.pieces.size(); ++i)
            {
                VLayoutPiece rawPiece = data.pieces.at(i);

                // We translate the piece, so that the origin of the bounding rect of the piece is at (0,0)
                // It makes positioning later on easier.
                QRectF boundingRect = rawPiece.DetailBoundingRect();
                QPointF topLeft = boundingRect.topLeft();
                rawPiece.Translate(-topLeft.x(), -topLeft.y());



                // TODO / FIXME: make a few tests, on the data to check for validity. If not
                //
                // If seam allowance enabled, but the path is empty — invalid.
                // If seam line path not hidden, but the path is empty — invalid.
                // If seam allowance is built-in, but the seam line path is empty — invalid.


                // TODO for feature "Update piece" : CreateOrUpdate() function indstead of CreatePiece()
                VPuzzlePiece *piece = CreatePiece(rawPiece);
                m_layout->GetUnplacedPiecesLayer()->AddPiece(piece);
            }

            m_pieceCarrousel->Refresh();
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
VPuzzlePiece* PuzzleMainWindow::CreatePiece(const VLayoutPiece &rawPiece)
{
    VPuzzlePiece *piece = new VPuzzlePiece();
    piece->SetName(rawPiece.GetName());
    piece->SetUuid(rawPiece.GetUUID());

    piece->SetCuttingLine(rawPiece.GetMappedSeamAllowancePoints());
    piece->SetSeamLine(rawPiece.GetMappedContourPoints());

    piece->SetIsGrainlineEnabled(rawPiece.IsGrainlineEnabled());
    if(rawPiece.IsGrainlineEnabled())
    {
        piece->SetGrainlineAngle(rawPiece.GrainlineAngle());
        piece->SetGrainline(rawPiece.GetGrainline());
    }

    // TODO : set all the information we need for the piece!

    //
    connect(piece, &VPuzzlePiece::SelectionChanged, this, &PuzzleMainWindow::on_PieceSelectionChanged);
    connect(piece, &VPuzzlePiece::PositionChanged, this, &PuzzleMainWindow::on_PiecePositionChanged);
    connect(piece, &VPuzzlePiece::RotationChanged, this, &PuzzleMainWindow::on_PieceRotationChanged);


    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitMenuBar()
{
    // most of the actions are connected through name convention (auto-connection)


    // -------------------- connects the actions for the file menu
    connect(ui->actionExit, &QAction::triggered, this, &PuzzleMainWindow::close);

    // -------------------- connects the actions for the edit menu
    // TODO : initialise the undo / redo

    // -------------------- connects the actions for the windows menu
    // TODO : initialise the entries for the different windows

    // Add dock properties action
    QAction* actionDockWidgetToolOptions = ui->dockWidgetProperties->toggleViewAction();
    ui->menuWindows->addAction(actionDockWidgetToolOptions);

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitProperties()
{
    InitPropertyTabCurrentPiece();
    InitPropertyTabLayout();
    InitPropertyTabLayers();
    InitPropertyTabTiles();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitPropertyTabCurrentPiece()
{

    // ------------------------------ placement -----------------------------------
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_CurrentPiecePositionEdited);
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_CurrentPiecePositionEdited);
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitPropertyTabLayout()
{
    // -------------------- init the unit combobox ---------------------
   ui->comboBoxLayoutUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
   ui->comboBoxLayoutUnit->addItem(tr("Millimiters"), QVariant(UnitsToStr(Unit::Mm)));
   ui->comboBoxLayoutUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));

   // set default unit - TODO when we have the setting for the unit
//    const qint32 indexUnit = -1;//ui->comboBoxLayoutUnit->findData(qApp->ValentinaSettings()->GetUnit());
//    if (indexUnit != -1)
//    {
//        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
//    }

    // some of the UI Elements are connected to the slots via auto-connect
   // see https://doc.qt.io/qt-5/designer-using-a-ui-file.html#widgets-and-dialogs-with-auto-connect

    // -------------------- layout width, length, orientation  ------------------------
    connect(ui->doubleSpinBoxLayoutWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_LayoutSizeChanged);
    connect(ui->doubleSpinBoxLayoutLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_LayoutSizeChanged);
    connect(ui->radioButtonLayoutPortrait, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::on_LayoutOrientationChanged);
    connect(ui->radioButtonLayoutLandscape, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::on_LayoutOrientationChanged);

    // -------------------- margins  ------------------------
    connect(ui->doubleSpinBoxLayoutMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_LayoutMarginChanged);
    connect(ui->doubleSpinBoxLayoutMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_LayoutMarginChanged);
    connect(ui->doubleSpinBoxLayoutMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_LayoutMarginChanged);
    connect(ui->doubleSpinBoxLayoutMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_LayoutMarginChanged);

    // ------------------- follow grainline -----------------------
    connect(ui->radioButtonLayoutFollowGrainlineNo, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::on_LayoutFollowGrainlineChanged);
    connect(ui->radioButtonLayoutFollowGrainlineVertical, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::on_LayoutFollowGrainlineChanged);
    connect(ui->radioButtonLayoutFollowGrainlineHorizontal, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::on_LayoutFollowGrainlineChanged);

    // -------------------- export ---------------------------

    // TODO init the file format export combobox

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitPropertyTabTiles()
{
    // for the MVP we don't want the tiles tab.
    // we remove it. As soon as we need it, update this code
    ui->tabWidgetProperties->removeTab(2); // remove tiles
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitPropertyTabLayers()
{
    // for the MVP we don't want the layers tab.
    // we remove it. As soon as we need it, update this code
    ui->tabWidgetProperties->removeTab(3); // remove layers
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitPieceCarrousel()
{
    m_pieceCarrousel = new VPieceCarrousel(m_layout, ui->dockWidgetPieceCarrousel);
    ui->dockWidgetPieceCarrousel->setWidget(m_pieceCarrousel);

    connect(ui->dockWidgetPieceCarrousel, QOverload<Qt::DockWidgetArea>::of(&QDockWidget::dockLocationChanged), this,
              &PuzzleMainWindow::on_PieceCarrouselLocationChanged);
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetPropertiesData()
{
    if(m_layout == nullptr)
    {
       // TODO : hide the tabs when there is no layout
    }
    else
    {
        SetPropertyTabCurrentPieceData();
        SetPropertyTabLayoutData();
        SetPropertyTabTilesData();
        SetPropertyTabLayersData();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetPropertyTabCurrentPieceData()
{
    if(m_selectedPieces.count() == 0)
    {
        // show the content "no piece selected"

         ui->containerCurrentPieceNoData->setVisible(true);
         ui->containerCurrentPieceData->setVisible(false);
         ui->containerCurrentPieceMultipleData->setVisible(false);
    }
    else if(m_selectedPieces.count() == 1)
    {
        // show the content "selected piece data"
        ui->containerCurrentPieceNoData->setVisible(false);
        ui->containerCurrentPieceData->setVisible(true);
        ui->containerCurrentPieceMultipleData->setVisible(false);

        VPuzzlePiece *selectedPiece = m_selectedPieces.first();

        // set the value to the current piece
        ui->lineEditCurrentPieceName->setText(selectedPiece->GetName());

        ui->checkBoxCurrentPieceShowSeamline->setChecked(selectedPiece->GetShowSeamLine());
        ui->checkBoxCurrentPieceMirrorPiece->setChecked(selectedPiece->GetPieceMirrored());

        QPointF pos = selectedPiece->GetPosition();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceBoxPositionX,
                              UnitConvertor(pos.x(), Unit::Px, m_layout->GetUnit()));
        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceBoxPositionY,
                              UnitConvertor(pos.y(), Unit::Px, m_layout->GetUnit()));

        qreal angle = selectedPiece->GetRotation();
        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceAngle, angle);
    }
    else
    {
        // show the content "multiple pieces selected"

        ui->containerCurrentPieceNoData->setVisible(false);
        ui->containerCurrentPieceData->setVisible(false);
        ui->containerCurrentPieceMultipleData->setVisible(true);

        // if needed in the future, we can show some properties that coul be edited for all the pieces
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetPropertyTabLayoutData()
{
    // set Unit
    int index = ui->comboBoxLayoutUnit->findData(QVariant(UnitsToStr(m_layout->GetUnit())));
    if(index != -1)
    {
        ui->comboBoxLayoutUnit->blockSignals(true); // FIXME: is there a better way to block the signals?
        ui->comboBoxLayoutUnit->setCurrentIndex(index);
        ui->comboBoxLayoutUnit->blockSignals(false);
    }

    // set Width / Length
    QSizeF size = m_layout->GetLayoutSizeConverted();
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutWidth, size.width());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutLength, size.height());

    // Set Orientation
    if(size.width() <= size.height())
    {
        ui->radioButtonLayoutPortrait->setChecked(true);
    }
    else
    {
        ui->radioButtonLayoutLandscape->setChecked(true);
    }

    // set margins
    QMarginsF margins = m_layout->GetLayoutMarginsConverted();
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginLeft, margins.left());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginTop, margins.top());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginRight, margins.right());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginBottom, margins.bottom());

    // set pieces gap
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutPiecesGap, m_layout->GetPiecesGapConverted());

    // set the checkboxes
    SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesOutOfBound, m_layout->GetWarningPiecesOutOfBound());
    SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesSuperposition, m_layout->GetWarningSuperpositionOfPieces());
    SetCheckBoxValue(ui->checkBoxLayoutStickyEdges, m_layout->GetStickyEdges());
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetPropertyTabTilesData()
{

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetPropertyTabLayersData()
{

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitMainGraphics()
{
    m_graphicsView = new VPuzzleMainGraphicsView(m_layout, this);
    ui->centralWidget->layout()->addWidget(m_graphicsView);

    m_graphicsView->RefreshLayout();
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetDoubleSpinBoxValue(QDoubleSpinBox *spinBox, qreal value)
{
    spinBox->blockSignals(true);
    spinBox->setValue(value);
    spinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SetCheckBoxValue(QCheckBox *checkbox, bool value)
{
    checkbox->blockSignals(true);
    checkbox->setChecked(value);
    checkbox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::ReadSettings()
{
    qCDebug(pWindow, "Reading settings.");
    const VPuzzleSettings *settings = qApp->PuzzleSettings();

    if (settings->status() == QSettings::NoError)
    {
        restoreGeometry(settings->GetGeometry());
        restoreState(settings->GetWindowState());
        restoreState(settings->GetToolbarsState(), APP_VERSION);

        ui->dockWidgetProperties->setVisible(settings->IsDockWidgetPropertiesActive());
        ui->dockWidgetPropertiesContents->setVisible(settings->IsDockWidgetPropertiesContentsActive());

        // Scene antialiasing
        m_graphicsView->SetAntialiasing(settings->GetGraphicalOutput());

        // Stack limit
//        qApp->getUndoStack()->setUndoLimit(settings->GetUndoCount());
    }
    else
    {
        qWarning() << tr("Cannot read settings from a malformed .INI file.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::WriteSettings()
{
    VPuzzleSettings *settings = qApp->PuzzleSettings();
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
bool PuzzleMainWindow::MaybeSave()
{
    // TODO: Implement maybe save check
//    if (this->isWindowModified())
//    {
//        if (curFile.isEmpty() && ui->tableWidget->rowCount() == 0)
//        {
//            return true;// Don't ask if file was created without modifications.
//        }

//        QScopedPointer<QMessageBox> messageBox(new QMessageBox(tr("Unsaved changes"),
//                                                               tr("Measurements have been modified.\n"
//                                                                  "Do you want to save your changes?"),
//                                                               QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No,
//                                                               QMessageBox::Cancel, this, Qt::Sheet));

//        messageBox->setDefaultButton(QMessageBox::Yes);
//        messageBox->setEscapeButton(QMessageBox::Cancel);

//        messageBox->setButtonText(QMessageBox::Yes, curFile.isEmpty() || mIsReadOnly ? tr("Save…") : tr("Save"));
//        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

//        messageBox->setWindowModality(Qt::ApplicationModal);
//        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

//        switch (ret)
//        {
//        case QMessageBox::Yes:
//            if (mIsReadOnly)
//            {
//                return FileSaveAs();
//            }
//            else
//            {
//                return FileSave();
//            }
//        case QMessageBox::No:
//            return true;
//        case QMessageBox::Cancel:
//            return false;
//        default:
//            break;
//        }
//    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionNew_triggered()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::New");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO


}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::closeEvent(QCloseEvent *event)
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
void PuzzleMainWindow::on_actionOpen_triggered()
{
    qCDebug(pWindow, "Openning puzzle layout file.");

    const QString filter(tr("Layout files") + QLatin1String(" (*.vlt)"));

    //Get list last open files
    QStringList recentFiles = qApp->PuzzleSettings()->GetRecentFileList();
    QString dir;
    if (recentFiles.isEmpty())
    {
        dir = QDir::homePath();
    }
    else
    {
        //Absolute path to last open file
        dir = QFileInfo(recentFiles.first()).absolutePath();
    }
    qCDebug(pWindow, "Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter, nullptr);

    if (filePath.isEmpty())
    {
        return;
    }


    // TODO : if m_layout == nullptr, open in current window
    // otherwise open in new window

    // TODO : if layout file has a lock, warning message


    if(!LoadFile(filePath))
    {
        return;
    }

    // Updates the list of recent files
    recentFiles.removeAll(filePath);
    recentFiles.prepend(filePath);
    while (recentFiles.size() > MaxRecentFiles)
    {
        recentFiles.removeLast();
    }
    qApp->PuzzleSettings()->SetRecentFileList(recentFiles);

    // updates the properties with the loaded data
    SetPropertiesData();

    // TODO : update the Carrousel and the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionSave_triggered()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::Save");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionSaveAs_triggered()
{
    // TODO / FIXME : See valentina how the save is done over there. we need to add the extension .vlt, check for empty file names etc.

    //Get list last open files
    QStringList recentFiles = qApp->PuzzleSettings()->GetRecentFileList();
    QString dir;
    if (recentFiles.isEmpty())
    {
        dir = QDir::homePath();
    }
    else
    {
        //Absolute path to last open file
        dir = QFileInfo(recentFiles.first()).absolutePath();
    }

    QString filters(tr("Layout files") + QLatin1String("(*.vlt)"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"),
                                                    dir + QLatin1String("/") + tr("Layout") + QLatin1String(".vlt"),
                                                    filters, nullptr
#ifdef Q_OS_LINUX
                                                    , QFileDialog::DontUseNativeDialog
#endif
                                                    );

    SaveFile(fileName);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionImportRawLayout_triggered()
{
    // TODO: here the code is probably just bad, to be edited

    QString dir;
    if (true)
    {
        dir = QDir::homePath();
    }
    else
    {
        // TODO / FIXME get the default path for raw layouts
    }

    const QString filter(tr("Raw Layout files") + QLatin1String(" (*.rld)"));

    qCDebug(pWindow, "Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter, nullptr);

    QStringList rawLayouts = QStringList();
    rawLayouts.append(filePath);

    ImportRawLayouts(rawLayouts);

    // TODO / FIXME : better error handling

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionCloseLayout_triggered()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CloseLayout");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_actionAboutPuzzle_triggered()
{
    auto *aboutDialog = new VPDialogAbout(this);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    aboutDialog->show();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_comboBoxLayoutUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    QVariant comboBoxValue = ui->comboBoxLayoutUnit->currentData();

    if(comboBoxValue == QVariant(UnitsToStr(Unit::Cm)))
    {
        m_layout->SetUnit(Unit::Cm);
    }
    else if(comboBoxValue == QVariant(UnitsToStr(Unit::Mm)))
    {
        m_layout->SetUnit(Unit::Mm);
    }
    else if(comboBoxValue == QVariant(UnitsToStr(Unit::Inch)))
    {
        m_layout->SetUnit(Unit::Inch);
    }

    SetPropertyTabLayoutData();
    SetPropertyTabCurrentPieceData();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_comboBoxLayoutTemplate_currentIndexChanged(int index)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutTemplateChanged");
    int ret = msgBox.exec();

    Q_UNUSED(index);
    Q_UNUSED(ret);


    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_LayoutSizeChanged()
{
    m_layout->SetLayoutSizeConverted(ui->doubleSpinBoxLayoutWidth->value(), ui->doubleSpinBoxLayoutLength->value());

    // updates orientation - no need to block signals because the signal reacts on "clicked"
    if(ui->doubleSpinBoxLayoutWidth->value() <= ui->doubleSpinBoxLayoutLength->value())
    {
        //portrait
        ui->radioButtonLayoutPortrait->setChecked(true);
    }
    else
    {
        //landscape
        ui->radioButtonLayoutLandscape->setChecked(true);
    }

    // TODO Undo / Redo

    m_graphicsView->RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_LayoutOrientationChanged()
{
    // swap the width and length
    qreal width_before = ui->doubleSpinBoxLayoutWidth->value();
    qreal length_before = ui->doubleSpinBoxLayoutLength->value();

    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutWidth, length_before);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutLength, width_before);

    m_layout->SetLayoutSizeConverted(ui->doubleSpinBoxLayoutWidth->value(), ui->doubleSpinBoxLayoutLength->value());

    // TODO Undo / Redo

    m_graphicsView->RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_pushButtonLayoutRemoveUnusedLength_clicked()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutRemoveUnusedLength");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_LayoutMarginChanged()
{
    m_layout->SetLayoutMarginsConverted(
                ui->doubleSpinBoxLayoutMarginLeft->value(),
                ui->doubleSpinBoxLayoutMarginTop->value(),
                ui->doubleSpinBoxLayoutMarginRight->value(),
                ui->doubleSpinBoxLayoutMarginBottom->value()
            );

    // TODO Undo / Redo

    m_graphicsView->RefreshLayout();
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_LayoutFollowGrainlineChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutFollowGrainlineChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_doubleSpinBoxLayoutPiecesGap_valueChanged(double value)
{
    m_layout->SetPiecesGapConverted(value);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_checkBoxLayoutWarningPiecesSuperposition_toggled(bool checked)
{
    m_layout->SetWarningSuperpositionOfPieces(checked);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_checkBoxLayoutWarningPiecesOutOfBound_toggled(bool checked)
{
    m_layout->SetWarningPiecesOutOfBound(checked);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_checkBoxLayoutStickyEdges_toggled(bool checked)
{
    m_layout->SetStickyEdges(checked);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_pushButtonLayoutExport_clicked()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutExport");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_checkBoxCurrentPieceShowSeamline_toggled(bool checked)
{
    if(m_selectedPieces.count() == 1)
    {
        m_selectedPieces.first()->SetShowSeamLine(checked);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_checkBoxCurrentPieceMirrorPiece_toggled(bool checked)
{
    if(m_selectedPieces.count() == 1)
    {
        m_selectedPieces.first()->SetPieceMirrored(checked);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_doubleSpinBoxCurrentPieceAngle_valueChanged(double value)
{
    if(m_selectedPieces.count() == 1)
    {
        VPuzzlePiece *piece = m_selectedPieces.first();
        piece->SetRotation(value);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_CurrentPiecePositionEdited()
{
    if(m_selectedPieces.count() == 1)
    {
        VPuzzlePiece *piece = m_selectedPieces.first();
        QPointF pos(UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionX->value(), m_layout->GetUnit(), Unit::Px),
                    UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionY->value(), m_layout->GetUnit(), Unit::Px));
        piece->SetPosition(pos);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_PieceCarrouselLocationChanged(Qt::DockWidgetArea area)
{
    if(area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
    {
        m_pieceCarrousel->SetOrientation(Qt::Horizontal);
    }
    else if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
    {
        m_pieceCarrousel->SetOrientation(Qt::Vertical);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_PieceSelectionChanged()
{
    m_selectedPieces = m_layout->GetSelectedPieces();

    // update the property of the piece currently selected
    SetPropertyTabCurrentPieceData();
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_PiecePositionChanged()
{
    if(m_selectedPieces.count() == 1)
    {
        VPuzzlePiece *piece = m_selectedPieces.first();
        QPointF pos = piece->GetPosition();

        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceBoxPositionX,
                              UnitConvertor(pos.x(), Unit::Px, m_layout->GetUnit()));
        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceBoxPositionY,
                              UnitConvertor(pos.y(), Unit::Px, m_layout->GetUnit()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_PieceRotationChanged()
{
    if(m_selectedPieces.count() == 1)
    {
        VPuzzlePiece *piece = m_selectedPieces.first();
        qreal angle = piece->GetRotation();

        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceAngle, angle);
    }
}
