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
#include "ui_puzzlemainwindow.h"

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::PuzzleMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PuzzleMainWindow)
{
    ui->setupUi(this);

    InitMenuBar();

    InitPropertyTabs();

}

//---------------------------------------------------------------------------------------------------------------------
PuzzleMainWindow::~PuzzleMainWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
bool PuzzleMainWindow::LoadFile(const QString &path)
{
    Q_UNUSED(path)
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitMenuBar()
{
    // connects the actions for the file menu
    connect(ui->actionNew, &QAction::triggered, this, &PuzzleMainWindow::New);
    connect(ui->actionOpen, &QAction::triggered, this, &PuzzleMainWindow::Open);
    connect(ui->actionSave, &QAction::triggered, this, &PuzzleMainWindow::Save);
    connect(ui->actionSaveAs, &QAction::triggered, this, &PuzzleMainWindow::SaveAs);
    connect(ui->actionImportRawLayout, &QAction::triggered, this, &PuzzleMainWindow::ImportRawLayout);
    connect(ui->actionExit, &QAction::triggered, this, &PuzzleMainWindow::close);

    // connects the actions for the edit menu
    // TODO : initialise the undo / redo

    // connects the actions for the windows menu
    // TODO : initialise the entries for the different windows
    connect(ui->actionCloseLayout, &QAction::triggered, this, &PuzzleMainWindow::CloseLayout);

    // connects the action for the Help Menu
    connect(ui->actionAboutQt, &QAction::triggered, this, &PuzzleMainWindow::AboutQt);
    connect(ui->actionAboutPuzzle, &QAction::triggered, this, &PuzzleMainWindow::AboutPuzzle);

}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::InitPropertyTabs()
{
    // for the MVP we don't want the tiles and current layer tabs.
    // we remove them. As soon as we need them, delete / update this code
    ui->tabWidgetProperties->removeTab(3); // remove layers
    ui->tabWidgetProperties->removeTab(2); // remove tiles

}


//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::New()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::New");
    int ret = msgBox.exec();

    // TODO


}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::Open()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::Open");
    int ret = msgBox.exec();

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::Save()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::Save");
    int ret = msgBox.exec();

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::SaveAs()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::SaveAs");
    int ret = msgBox.exec();

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::ImportRawLayout()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::ImportRawLayout");
    int ret = msgBox.exec();

    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzleMainWindow::CloseLayout()
{
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::CloseLayout");
    int ret = msgBox.exec();

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
    // just for test purpuses, to be removed:
    QMessageBox msgBox;
    msgBox.setText("TODO PuzzleMainWindow::AboutPuzzle");
    int ret = msgBox.exec();

    // TODO
}


