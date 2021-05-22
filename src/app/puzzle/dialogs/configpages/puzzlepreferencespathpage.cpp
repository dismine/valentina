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
#include "ui_puzzlepreferencespathpage.h"
#include "../../vpapplication.h"

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesPathPage::PuzzlePreferencesPathPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PuzzlePreferencesPathPage)
{
    ui->setupUi(this);

    InitTable();

    connect(ui->pathTable, &QTableWidget::itemSelectionChanged, this, [this]()
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
    settings->SetPathIndividualMeasurements(ui->pathTable->item(0, 1)->text());
    settings->SetPathMultisizeMeasurements(ui->pathTable->item(1, 1)->text());
    settings->SetPathPattern(ui->pathTable->item(2, 1)->text());
    settings->SetPathTemplate(ui->pathTable->item(3, 1)->text());
    settings->SetPathManualLayouts(ui->pathTable->item(4, 1)->text());
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
        case 0: // individual measurements
            path = VCommonSettings::GetDefPathIndividualMeasurements();
            break;
        case 1: // multisize measurements
            path = VCommonSettings::GetDefPathMultisizeMeasurements();
            break;
        case 2: // pattern path
            path = VCommonSettings::GetDefPathPattern();
            break;
        case 3: // templates
            path = VCommonSettings::GetDefPathTemplate();
            break;
        case 4: // layouts
            path = VCommonSettings::GetDefPathManualLayouts();
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
        case 0: // individual measurements
            path = VPApplication::VApp()->PuzzleSettings()->GetPathIndividualMeasurements();
            break;
        case 1: // multisize measurements
            path = VPApplication::VApp()->PuzzleSettings()->GetPathMultisizeMeasurements();
            path = VCommonSettings::PrepareMultisizeTables(path);
            break;
        case 2: // pattern path
            path = VPApplication::VApp()->PuzzleSettings()->GetPathPattern();
            break;
        case 3: // templates
            path = VPApplication::VApp()->PuzzleSettings()->GetPathTemplate();
            break;
        case 4: // layouts
            path = VPApplication::VApp()->PuzzleSettings()->GetPathManualLayouts();
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
    ui->pathTable->setRowCount(5);
    ui->pathTable->setColumnCount(2);

    const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    {
        ui->pathTable->setItem(0, 0, new QTableWidgetItem(tr("My Individual Measurements")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathIndividualMeasurements());
        item->setToolTip(settings->GetPathIndividualMeasurements());
        ui->pathTable->setItem(0, 1, item);
    }

    {
        ui->pathTable->setItem(1, 0, new QTableWidgetItem(tr("My Multisize Measurements")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathMultisizeMeasurements());
        item->setToolTip(settings->GetPathMultisizeMeasurements());
        ui->pathTable->setItem(1, 1, item);
    }

    {
        ui->pathTable->setItem(2, 0, new QTableWidgetItem(tr("My Patterns")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathPattern());
        item->setToolTip(settings->GetPathPattern());
        ui->pathTable->setItem(2, 1, item);
    }

    {
        ui->pathTable->setItem(3, 0, new QTableWidgetItem(tr("My Templates")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathTemplate());
        item->setToolTip(settings->GetPathTemplate());
        ui->pathTable->setItem(3, 1, item);
    }

    {
        ui->pathTable->setItem(4, 0, new QTableWidgetItem(tr("My Layouts")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathManualLayouts());
        item->setToolTip(settings->GetPathManualLayouts());
        ui->pathTable->setItem(4, 1, item);
    }

    ui->pathTable->verticalHeader()->setDefaultSectionSize(20);
    ui->pathTable->resizeColumnsToContents();
    ui->pathTable->resizeRowsToContents();
    ui->pathTable->horizontalHeader()->setStretchLastSection(true);
}
