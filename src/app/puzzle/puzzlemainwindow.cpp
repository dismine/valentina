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

#include "ui_puzzlemainwindow.h"
#include "dialogs/dialogaboutpuzzle.h"
#include "xml/vpuzzlelayoutfilewriter.h"
#include "xml/vpuzzlelayoutfilereader.h"
#include "puzzleapplication.h"
#include "../vlayout/vrawlayout.h"
#include "../vmisc/vsysexits.h"
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
    pieceCarrousel(new VPieceCarrousel),
    m_cmd(cmd)
{

    // ----- for test purposes, to be removed------------------
    m_layout = new VPuzzleLayout();
    m_layout->SetLayoutMarginsConverted(1.5, 2.00, 4.21, 0.25);
    m_layout->SetLayoutSizeConverted(30.0, 29.7);
    m_layout->SetPiecesGapConverted(1.27);
    m_layout->SetUnit(Unit::Cm);
    m_layout->SetWarningSuperpositionOfPieces(true);
    // --------------------------------------------------------

    ui->setupUi(this);

    InitMenuBar();
    InitProperties();
    InitPieceCarrousel();


    SetPropertiesData();
}

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::~PuzzleMainWindow()
{
    delete ui;
    delete pieceCarrousel;
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
void PuzzleMainWindow::ImportRawLayouts(const QStringList &layouts)
{
    VRawLayout layoutReader;

    for(auto &path : layouts)
    {
        VRawLayoutData data;
        if (layoutReader.ReadFile(path, data))
        {
            // Do somethinmg with raw layout data
        }
        else
        {
            qCCritical(pWindow, "%s\n", qPrintable(tr("Could not extract data from file '%1'. %2")
                                                    .arg(path, layoutReader.ErrorString())));
            if (m_cmd != nullptr && not m_cmd->IsGuiEnabled())
            {
                m_cmd->ShowHelp(V_EX_DATAERR);
            }
        }
    }
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
            &PuzzleMainWindow::on_CurrentPiecePositionChanged);
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::on_CurrentPiecePositionChanged);
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
    ui->dockWidgetPieceCarrousel->setWidget(pieceCarrousel);

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
    if(m_selectedPiece == nullptr)
    {
        if(false) // check for multiple piece selection
        {
            // TODO in the future
        }
        else
        {
           // TODO : update current piece data to show a "no current piece selected"
        }
    }
    else
    {
        // TODO set the values of the piece currently selected
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
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::ImportRawLayout");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
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
    auto *aboutDialog = new DialogAboutPuzzle(this);
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
    // TODO update the QGraphicView
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_LayoutOrientationChanged()
{
    // swap the width and length
    qreal width_before = ui->doubleSpinBoxLayoutWidth->value();
    qreal length_before = ui->doubleSpinBoxLayoutLength->value();

    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutWidth, length_before);
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutLength, width_before);

    // TODO Undo / Redo
    // TODO update the QGraphicView
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
    // TODO update the QGraphicView
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
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CurrentPieceShowSeamlineChanged");
    int ret = msgBox.exec();

    Q_UNUSED(checked);
    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_checkBoxCurrentPieceMirrorPiece_toggled(bool checked)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CurrentPieceMirrorPieceChanged");
    int ret = msgBox.exec();

    Q_UNUSED(checked);
    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_doubleSpinBoxCurrentPieceAngle_valueChanged(double value)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CurrentPieceAngleChanged");
    int ret = msgBox.exec();

    Q_UNUSED(value);
    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_CurrentPiecePositionChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CurrentPiecePositionChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::on_PieceCarrouselLocationChanged(Qt::DockWidgetArea area)
{
    if(area == Qt::BottomDockWidgetArea || area == Qt::TopDockWidgetArea)
    {
        pieceCarrousel->setOrientation(Qt::Horizontal);
        ui->dockWidgetPieceCarrousel->setMaximumHeight(208);
        ui->dockWidgetPieceCarrousel->setMaximumWidth(10000);
    }
    else if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
    {
        pieceCarrousel->setOrientation(Qt::Vertical);
        ui->dockWidgetPieceCarrousel->setMaximumHeight(10000);
        ui->dockWidgetPieceCarrousel->setMaximumWidth(160);
    }
}

