/************************************************************************
 **
 **  @file   puzzlepreferencespathpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 5, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#include "puzzlepreferencespathpage.h"
#include "../../vpapplication.h"
#include "ui_puzzlepreferencespathpage.h"

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesPathPage::PuzzlePreferencesPathPage(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::PuzzlePreferencesPathPage)
{
    ui->setupUi(this);

    InitTable();

    connect(ui->pathTable, &QTableWidget::itemSelectionChanged, this,
            [this]()
            {
                ui->defaultButton->setEnabled(not ui->pathTable->selectedItems().isEmpty());
                ui->defaultButton->setDefault(false);

                ui->editButton->setEnabled(not ui->pathTable->selectedItems().isEmpty());
                ui->editButton->setDefault(true);
            });

    connect(ui->defaultButton, &QPushButton::clicked, this, &PuzzlePreferencesPathPage::DefaultPath);
    connect(ui->editButton, &QPushButton::clicked, this, &PuzzlePreferencesPathPage::EditPath);
}

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesPathPage::~PuzzlePreferencesPathPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesPathPage::Apply()
{
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    settings->SetPathSVGFonts(ui->pathTable->item(0, 1)->text());
    settings->SetPathFontCorrections(ui->pathTable->item(1, 1)->text());
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesPathPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        InitTable();
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesPathPage::DefaultPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // svg fonts
            path = VCommonSettings::GetDefPathSVGFonts();
            break;
        case 1: // font corrections
            path = VCommonSettings::GetDefPathFontCorrections();
            break;
        default:
            break;
    }

    item->setText(path);
    item->setToolTip(path);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesPathPage::EditPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // svg fonts
            path = VPApplication::VApp()->PuzzleSettings()->GetPathSVGFonts();
            break;
        case 1: // font corrections
            path = VPApplication::VApp()->PuzzleSettings()->GetPathFontCorrections();
            break;
        default:
            break;
    }

    bool usedNotExistedDir = false;
    QDir directory(path);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(QChar('.'));
    }

    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Open Directory"), path,
        VAbstractApplication::VApp()->NativeFileDialog(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    if (dir.isEmpty())
    {
        if (usedNotExistedDir)
        {
            QDir(path).rmpath(QChar('.'));
        }

        DefaultPath();
        return;
    }

    item->setText(dir);
    item->setToolTip(dir);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesPathPage::InitTable()
{
    ui->pathTable->clearContents();
    ui->pathTable->setRowCount(2);
    ui->pathTable->setColumnCount(2);

    const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    {
        ui->pathTable->setItem(0, 0, new QTableWidgetItem(tr("My SVG Fonts")));
        auto *item = new QTableWidgetItem(settings->GetPathSVGFonts());
        item->setToolTip(settings->GetPathSVGFonts());
        ui->pathTable->setItem(0, 1, item);
    }

    {
        ui->pathTable->setItem(1, 0, new QTableWidgetItem(tr("My font corrections")));
        auto *item = new QTableWidgetItem(settings->GetPathFontCorrections());
        item->setToolTip(settings->GetPathFontCorrections());
        ui->pathTable->setItem(1, 1, item);
    }

    ui->pathTable->verticalHeader()->setDefaultSectionSize(20);
    ui->pathTable->resizeColumnsToContents();
    ui->pathTable->resizeRowsToContents();
    ui->pathTable->horizontalHeader()->setStretchLastSection(true);
}
