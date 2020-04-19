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

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::PuzzleMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PuzzleMainWindow),
    pieceCarrousel(new VPieceCarrousel),
    m_layout (nullptr)
{
    ui->setupUi(this);

    InitMenuBar();
    InitProperties();
    InitPieceCarrousel();


    // ----- for test purposes, to be removed------------------
    m_layout = new VPuzzleLayout();
    m_layout->SetLayoutMarginsConverted(1.5, 2.00, 4.21, 0.25);
    m_layout->SetLayoutSizeConverted(21.0, 29.7);
    m_layout->SetPiecesGapConverted(1);
    m_layout->SetUnit(Unit::Cm);
    SetPropertiesData();
}

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::~PuzzleMainWindow()
{
    delete ui;
    delete pieceCarrousel;
}

//---------------------------------------------------------------------------------------------------------------------
bool PuzzleMainWindow::LoadFile(const QString &path)
{
    Q_UNUSED(path)
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::ImportRawLayouts(const QStringList &layouts)
{
    Q_UNUSED(layouts)
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitMenuBar()
{
    // -------------------- connects the actions for the file menu
    connect(ui->actionNew, &QAction::triggered, this, &PuzzleMainWindow::New);
    connect(ui->actionOpen, &QAction::triggered, this, &PuzzleMainWindow::Open);
    connect(ui->actionSave, &QAction::triggered, this, &PuzzleMainWindow::Save);
    connect(ui->actionSaveAs, &QAction::triggered, this, &PuzzleMainWindow::SaveAs);
    connect(ui->actionImportRawLayout, &QAction::triggered, this, &PuzzleMainWindow::ImportRawLayout);
    connect(ui->actionExit, &QAction::triggered, this, &PuzzleMainWindow::close);

    // -------------------- connects the actions for the edit menu
    // TODO : initialise the undo / redo

    // -------------------- connects the actions for the windows menu
    // TODO : initialise the entries for the different windows
    connect(ui->actionCloseLayout, &QAction::triggered, this, &PuzzleMainWindow::CloseLayout);

    // Add dock properties action
    QAction* actionDockWidgetToolOptions = ui->dockWidgetProperties->toggleViewAction();
    ui->menuWindows->addAction(actionDockWidgetToolOptions);

    // connects the action for the Help Menu
    connect(ui->actionAboutQt, &QAction::triggered, this, &PuzzleMainWindow::AboutQt);
    connect(ui->actionAboutPuzzle, &QAction::triggered, this, &PuzzleMainWindow::AboutPuzzle);

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
    // ------------------------------ seamline ------------------------------------
    connect(ui->checkBoxCurrentPieceShowSeamline,  QOverload<bool>::of(&QCheckBox::toggled), this,
            &PuzzleMainWindow::CurrentPieceShowSeamlineChanged);

    // ------------------------------ geometry ------------------------------------
    connect(ui->checkBoxCurrentPieceMirrorPiece,  QOverload<bool>::of(&QCheckBox::toggled), this,
            &PuzzleMainWindow::CurrentPieceMirrorPieceChanged);

    // ------------------------------ rotation ------------------------------------
    connect(ui->doubleSpinBoxCurrentPieceAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::CurrentPieceAngleChanged);

    // ------------------------------ placement -----------------------------------
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::CurrentPiecePositionChanged);
    connect(ui->doubleSpinBoxCurrentPieceBoxPositionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::CurrentPiecePositionChanged);

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

    connect(ui->comboBoxLayoutUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &PuzzleMainWindow::LayoutUnitChanged);


    // -------------------- init the template combobox ---------------------

    // TODO

    connect(ui->comboBoxLayoutTemplate, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &PuzzleMainWindow::LayoutTemplateChanged);

    // -------------------- layout width, length, orientation  ------------------------
    connect(ui->doubleSpinBoxLayoutWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutSizeChanged);
    connect(ui->doubleSpinBoxLayoutLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutSizeChanged);
    connect(ui->radioButtonLayoutPortrait, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::LayoutOrientationChanged);
    connect(ui->radioButtonLayoutLandscape, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::LayoutOrientationChanged);
    connect(ui->pushButtonLayoutRemoveUnusedLength, QOverload<bool>::of(&QPushButton::clicked), this,
            &PuzzleMainWindow::LayoutRemoveUnusedLength);

    // -------------------- margins  ------------------------
    connect(ui->doubleSpinBoxLayoutMarginTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutMarginChanged);
    connect(ui->doubleSpinBoxLayoutMarginRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutMarginChanged);
    connect(ui->doubleSpinBoxLayoutMarginBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutMarginChanged);
    connect(ui->doubleSpinBoxLayoutMarginLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutMarginChanged);

    // ------------------- follow grainline -----------------------
    connect(ui->radioButtonLayoutFollowGrainlineNo, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::LayoutFollowGrainlineChanged);
    connect(ui->radioButtonLayoutFollowGrainlineVertical, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::LayoutFollowGrainlineChanged);
    connect(ui->radioButtonLayoutFollowGrainlineHorizontal, QOverload<bool>::of(&QRadioButton::clicked), this,
            &PuzzleMainWindow::LayoutFollowGrainlineChanged);

    // -------------------- pieces gap and checkboxes ---------------
    connect(ui->doubleSpinBoxLayoutPiecesGap, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &PuzzleMainWindow::LayoutPiecesGapChanged);
    connect(ui->checkBoxLayoutWarningPiecesSuperposition,  QOverload<bool>::of(&QCheckBox::toggled), this,
            &PuzzleMainWindow::LayoutWarningPiecesSuperpositionChanged);
    connect(ui->checkBoxLayoutWarningPiecesOutOfBound,  QOverload<bool>::of(&QCheckBox::toggled), this,
            &PuzzleMainWindow::LayoutWarningPiecesOutOfBoundChanged);
    connect(ui->checkBoxLayoutStickyEdges,  QOverload<bool>::of(&QCheckBox::toggled), this,
            &PuzzleMainWindow::LayoutStickyEdgesChanged);

    // -------------------- export ---------------------------

    // TODO init the file format export combobox

    connect(ui->pushButtonLayoutExport, QOverload<bool>::of(&QPushButton::clicked), this,
            &PuzzleMainWindow::LayoutExport);
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
              &PuzzleMainWindow::PieceCarrouselLocationChanged);
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
    int index = ui->comboBoxLayoutUnit->findData(QVariant(UnitsToStr(m_layout->getUnit())));
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

    // set margins
    QMarginsF margins = m_layout->GetLayoutMarginsConverted();
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginLeft, margins.left());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginTop, margins.top());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginRight, margins.right());
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutMarginBottom, margins.bottom());

    // set pieces gap
    SetDoubleSpinBoxValue(ui->doubleSpinBoxLayoutPiecesGap, m_layout->GetPiecesGapConverted());
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
void PuzzleMainWindow::New()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::New");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO


}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::Open()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::Open");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::Save()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::Save");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SaveAs()
{
    // TODO / FIXME : See valentina how the save is done over there. we need to add the extension .vlt, check for empty file names etc.


    QString filters(tr("Pattern files") + QLatin1String("(*.val)"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"),
                                                    /*dir +*/ QLatin1String("/") + tr("Layout") + QLatin1String(".vlt"),
                                                    filters, nullptr
#ifdef Q_OS_LINUX
                                                    , QFileDialog::DontUseNativeDialog
#endif
                                                    );


    QFile file(fileName);
    file.open(QIODevice::WriteOnly);

    VPuzzleLayoutFileWriter *fileWriter = new VPuzzleLayoutFileWriter();
    fileWriter->WriteFile(m_layout, &file);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::ImportRawLayout()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::ImportRawLayout");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::CloseLayout()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CloseLayout");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::AboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::AboutPuzzle()
{
    auto *aboutDialog = new DialogAboutPuzzle(this);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    aboutDialog->show();
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutUnitChanged(int index)
{

    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutUnitChanged");
    int ret = msgBox.exec();

    Q_UNUSED(index);
    Q_UNUSED(ret);


   // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutTemplateChanged(int index)
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
void PuzzleMainWindow::LayoutSizeChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutSizeChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutOrientationChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutOrientationChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutRemoveUnusedLength()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutRemoveUnusedLength");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutMarginChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutMarginChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutFollowGrainlineChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutFollowGrainlineChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutPiecesGapChanged(double value)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutPieceGapChanged");
    int ret = msgBox.exec();

    Q_UNUSED(value);
    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutWarningPiecesSuperpositionChanged(bool checked)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutWarningPiecesSuperpositionChanged");
    int ret = msgBox.exec();

    Q_UNUSED(checked);
    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutWarningPiecesOutOfBoundChanged(bool checked)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutWarningPiecesOutOfBoundChanged");
    int ret = msgBox.exec();

    Q_UNUSED(checked);
    Q_UNUSED(ret);

    // TODO

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutStickyEdgesChanged(bool checked)
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutStickyEdgesChanged");
    int ret = msgBox.exec();

    Q_UNUSED(checked);
    Q_UNUSED(ret);


    // TODO

}



//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::LayoutExport()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::LayoutExport");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::CurrentPieceShowSeamlineChanged(bool checked)
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
void PuzzleMainWindow::CurrentPieceMirrorPieceChanged(bool checked)
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
void PuzzleMainWindow::CurrentPieceAngleChanged(double value)
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
void PuzzleMainWindow::CurrentPiecePositionChanged()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CurrentPiecePositionChanged");
    int ret = msgBox.exec();

    Q_UNUSED(ret);

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::PieceCarrouselLocationChanged(Qt::DockWidgetArea area)
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

