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
#include "vpsheet.h"

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pWindow, "p.window")

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
VPMainWindow::VPMainWindow(const VPCommandLinePtr &cmd, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VPMainWindow),
    m_cmd(cmd)
{
    m_layout = new VPLayout();

    // create a standard sheet
    VPSheet *sheet = new VPSheet(m_layout);
    sheet->SetName(QObject::tr("Sheet #1"));
    m_layout->AddSheet(sheet);
    m_layout->SetFocusedSheet();

    // ----- for test purposes, to be removed------------------
    sheet->SetSheetMarginsConverted(2, 2, 2, 2);
    sheet->SetSheetSizeConverted(30.0, 45);
    sheet->SetPiecesGapConverted(1);

    m_layout->SetUnit(Unit::Cm);
    m_layout->SetWarningSuperpositionOfPieces(true);
    // --------------------------------------------------------

    ui->setupUi(this);

    InitMenuBar();
    InitProperties();
    InitCarrousel();
    InitMainGraphics();


    SetPropertiesData();

    ReadSettings();
}

//---------------------------------------------------------------------------------------------------------------------
VPMainWindow::~VPMainWindow()
{
    delete ui;
    delete m_carrousel;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPMainWindow::LoadFile(QString path)
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

    QScopedPointer<VPLayoutFileReader> fileReader(new VPLayoutFileReader());

    if(m_layout == nullptr)
    {
        m_layout = new VPLayout();
    }

    fileReader->ReadFile(m_layout, &file);

    // TODO / FIXME : better return value and error handling

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPMainWindow::SaveFile(const QString &path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);

    VPLayoutFileWriter *fileWriter = new VPLayoutFileWriter();
    fileWriter->WriteFile(m_layout, &file);

    // TODO / FIXME : better return value and error handling

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ImportRawLayouts(const QStringList &rawLayouts)
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
                VPPiece *piece = CreatePiece(rawPiece);
                m_layout->GetUnplacedPieceList()->AddPiece(piece);
            }

            m_carrousel->Refresh();
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
VPPiece* VPMainWindow::CreatePiece(const VLayoutPiece &rawPiece)
{
    VPPiece *piece = new VPPiece();
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
    connect(piece, &VPPiece::SelectionChanged, this, &VPMainWindow::on_PieceSelectionChanged);
    connect(piece, &VPPiece::PositionChanged, this, &VPMainWindow::on_PiecePositionChanged);
    connect(piece, &VPPiece::RotationChanged, this, &VPMainWindow::on_PieceRotationChanged);


    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitMenuBar()
{
    // most of the actions are connected through name convention (auto-connection)


    // -------------------- connects the actions for the file menu
    connect(ui->actionExit, &QAction::triggered, this, &VPMainWindow::close);

    // -------------------- connects the actions for the edit menu
    // TODO : initialise the undo / redo

    // -------------------- connects the actions for the windows menu
    // TODO : initialise the entries for the different windows

    // Add dock properties action
    QAction* actionDockWidgetToolOptions = ui->dockWidgetProperties->toggleViewAction();
    ui->menuWindows->addAction(actionDockWidgetToolOptions);

}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitProperties()
{
    InitPropertyTabCurrentPiece();
    InitPropertyTabLayout();
    InitPropertyTabTiles();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabCurrentPiece()
{

    // ------------------------------ placement -----------------------------------
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_CurrentPiecePositionEdited);
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_CurrentPiecePositionEdited);
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabLayout()
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
    connect(ui->doubleSpinBoxSheetWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetSizeChanged);
    connect(ui->doubleSpinBoxSheetLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetSizeChanged);
    connect(ui->radioButtonSheetPortrait, QOverload<bool>::of(&QRadioButton::clicked), this,
            &VPMainWindow::on_SheetOrientationChanged);
    connect(ui->radioButtonSheetLandscape, QOverload<bool>::of(&QRadioButton::clicked), this,
            &VPMainWindow::on_SheetOrientationChanged);

    // -------------------- margins  ------------------------
    connect(ui->doubleSpinBoxSheetMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);
    connect(ui->doubleSpinBoxSheetMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);
    connect(ui->doubleSpinBoxSheetMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);
    connect(ui->doubleSpinBoxSheetMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &VPMainWindow::on_SheetMarginChanged);

    // ------------------- follow grainline -----------------------
    connect(ui->radioButtonSheetFollowGrainlineNo, QOverload<bool>::of(&QRadioButton::clicked), this,
            &VPMainWindow::on_SheetFollowGrainlineChanged);
    connect(ui->radioButtonSheetFollowGrainlineVertical, QOverload<bool>::of(&QRadioButton::clicked), this,
            &VPMainWindow::on_SheetFollowGrainlineChanged);
    connect(ui->radioButtonSheetFollowGrainlineHorizontal, QOverload<bool>::of(&QRadioButton::clicked), this,
            &VPMainWindow::on_SheetFollowGrainlineChanged);

    // -------------------- export ---------------------------

    // TODO init the file format export combobox

}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitPropertyTabTiles()
{
    // for the MVP we don't want the tiles tab.
    // we remove it. As soon as we need it, update this code
    ui->tabWidgetProperties->removeTab(2); // remove tiles
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
    if(m_layout == nullptr)
    {
       // TODO : hide the tabs when there is no layout
    }
    else
    {
        SetPropertyTabCurrentPieceData();
        SetPropertyTabSheetData();
        SetPropertyTabTilesData();
        SetPropertyTabLayoutData();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabCurrentPieceData()
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

        VPPiece *selectedPiece = m_selectedPieces.first();

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
void VPMainWindow::SetPropertyTabSheetData()
{
    // set Width / Length
    QSizeF size = m_layout->GetFocusedSheet()->GetSheetSizeConverted();
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetWidth, size.width());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetLength, size.height());

    // Set Orientation
    if(size.width() <= size.height())
    {
        ui->radioButtonSheetPortrait->setChecked(true);
    }
    else
    {
        ui->radioButtonSheetLandscape->setChecked(true);
    }

    // set margins
    QMarginsF margins = m_layout->GetFocusedSheet()->GetSheetMarginsConverted();
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginLeft, margins.left());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginTop, margins.top());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginRight, margins.right());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetMarginBottom, margins.bottom());

    // set pieces gap
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetPiecesGap, m_layout->GetFocusedSheet()->GetPiecesGapConverted());

    // set the checkboxes
    SetCheckBoxValue(ui->checkBoxSheetStickyEdges, m_layout->GetFocusedSheet()->GetStickyEdges());
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabLayoutData()
{
    // TODO FIXME : Set name and description

    // set Unit
    int index = ui->comboBoxLayoutUnit->findData(QVariant(UnitsToStr(m_layout->GetUnit())));
    if(index != -1)
    {
        ui->comboBoxLayoutUnit->blockSignals(true); // FIXME: is there a better way to block the signals?
        ui->comboBoxLayoutUnit->setCurrentIndex(index);
        ui->comboBoxLayoutUnit->blockSignals(false);
    }

    // set controls
    SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesOutOfBound, m_layout->GetWarningPiecesOutOfBound());
    SetCheckBoxValue(ui->checkBoxLayoutWarningPiecesSuperposition, m_layout->GetWarningSuperpositionOfPieces());
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetPropertyTabTilesData()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::InitMainGraphics()
{
    m_graphicsView = new VPMainGraphicsView(m_layout, this);
    ui->centralWidget->layout()->addWidget(m_graphicsView);

    m_graphicsView->RefreshLayout();
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetDoubleSpinBoxValue(QDoubleSpinBox *spinBox, qreal value)
{
    spinBox->blockSignals(true);
    spinBox->setValue(value);
    spinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::SetCheckBoxValue(QCheckBox *checkbox, bool value)
{
    checkbox->blockSignals(true);
    checkbox->setChecked(value);
    checkbox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::ReadSettings()
{
    qCDebug(pWindow, "Reading settings.");
    const VPSettings *settings = qApp->PuzzleSettings();

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
void VPMainWindow::WriteSettings()
{
    VPSettings *settings = qApp->PuzzleSettings();
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
bool VPMainWindow::MaybeSave()
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
void VPMainWindow::on_actionNew_triggered()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::New");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO


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
void VPMainWindow::on_actionOpen_triggered()
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
void VPMainWindow::on_actionSave_triggered()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::Save");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_actionSaveAs_triggered()
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
void VPMainWindow::on_actionImportRawLayout_triggered()
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
void VPMainWindow::on_actionCloseLayout_triggered()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::CloseLayout");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
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
void VPMainWindow::on_comboBoxLayoutUnit_currentIndexChanged(int index)
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

    SetPropertyTabSheetData();
    SetPropertyTabCurrentPieceData();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_comboBoxSheetTemplate_currentIndexChanged(int index)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::SheetTemplateChanged");
    int ret = msgBox.exec();

    Q_UNUSED(index);
    Q_UNUSED(ret);


    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetSizeChanged()
{
    m_layout->GetFocusedSheet()->SetSheetSizeConverted(ui->doubleSpinBoxSheetWidth->value(), ui->doubleSpinBoxSheetLength->value());

    // updates orientation - no need to block signals because the signal reacts on "clicked"
    if(ui->doubleSpinBoxSheetWidth->value() <= ui->doubleSpinBoxSheetLength->value())
    {
        //portrait
        ui->radioButtonSheetPortrait->setChecked(true);
    }
    else
    {
        //landscape
        ui->radioButtonSheetLandscape->setChecked(true);
    }

    // TODO Undo / Redo

    m_graphicsView->RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetOrientationChanged()
{
    // swap the width and length
    qreal width_before = ui->doubleSpinBoxSheetWidth->value();
    qreal length_before = ui->doubleSpinBoxSheetLength->value();

    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetWidth, length_before);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxSheetLength, width_before);

    m_layout->GetFocusedSheet()->SetSheetSizeConverted(ui->doubleSpinBoxSheetWidth->value(), ui->doubleSpinBoxSheetLength->value());

    // TODO Undo / Redo

    m_graphicsView->RefreshLayout();
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
    m_layout->GetFocusedSheet()->SetSheetMarginsConverted(
                ui->doubleSpinBoxSheetMarginLeft->value(),
                ui->doubleSpinBoxSheetMarginTop->value(),
                ui->doubleSpinBoxSheetMarginRight->value(),
                ui->doubleSpinBoxSheetMarginBottom->value()
            );

    // TODO Undo / Redo

    m_graphicsView->RefreshLayout();
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_SheetFollowGrainlineChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::on_SheetFollowGrainlineChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_doubleSpinBoxSheetPiecesGap_valueChanged(double value)
{
    m_layout->GetFocusedSheet()->SetPiecesGapConverted(value);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_checkBoxLayoutWarningPiecesSuperposition_toggled(bool checked)
{
    m_layout->SetWarningSuperpositionOfPieces(checked);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_checkBoxLayoutWarningPiecesOutOfBound_toggled(bool checked)
{
    m_layout->SetWarningPiecesOutOfBound(checked);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_checkBoxSheetStickyEdges_toggled(bool checked)
{
    m_layout->GetFocusedSheet()->SetStickyEdges(checked);

    // TODO Undo / Redo
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_pushButtonSheetExport_clicked()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO VPMainWindow::on_pushButtonSheetExport_clicked");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO

}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_checkBoxCurrentPieceShowSeamline_toggled(bool checked)
{
    if(m_selectedPieces.count() == 1)
    {
        m_selectedPieces.first()->SetShowSeamLine(checked);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_checkBoxCurrentPieceMirrorPiece_toggled(bool checked)
{
    if(m_selectedPieces.count() == 1)
    {
        m_selectedPieces.first()->SetPieceMirrored(checked);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_doubleSpinBoxCurrentPieceAngle_valueChanged(double value)
{
    if(m_selectedPieces.count() == 1)
    {
        VPPiece *piece = m_selectedPieces.first();
        piece->SetRotation(value);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_CurrentPiecePositionEdited()
{
    if(m_selectedPieces.count() == 1)
    {
        VPPiece *piece = m_selectedPieces.first();
        QPointF pos(UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionX->value(), m_layout->GetUnit(), Unit::Px),
                    UnitConvertor(ui->doubleSpinBoxCurrentPieceBoxPositionY->value(), m_layout->GetUnit(), Unit::Px));
        piece->SetPosition(pos);
    }
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
    m_selectedPieces = m_layout->GetSelectedPieces();

    // update the property of the piece currently selected
    SetPropertyTabCurrentPieceData();
}


//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_PiecePositionChanged()
{
    if(m_selectedPieces.count() == 1)
    {
        VPPiece *piece = m_selectedPieces.first();
        QPointF pos = piece->GetPosition();

        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceBoxPositionX,
                              UnitConvertor(pos.x(), Unit::Px, m_layout->GetUnit()));
        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceBoxPositionY,
                              UnitConvertor(pos.y(), Unit::Px, m_layout->GetUnit()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainWindow::on_PieceRotationChanged()
{
    if(m_selectedPieces.count() == 1)
    {
        VPPiece *piece = m_selectedPieces.first();
        qreal angle = piece->GetRotation();

        SetDoubleSpinBoxValue(ui->doubleSpinBoxCurrentPieceAngle, angle);
    }
}
