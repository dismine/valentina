/************************************************************************
 **
 **  @file   tapepreferencespathpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 10, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "tapepreferencespathpage.h"
#include "../../mapplication.h"
#include "../../vtapesettings.h"
#include "ui_tapepreferencespathpage.h"

//---------------------------------------------------------------------------------------------------------------------
TapePreferencesPathPage::TapePreferencesPathPage(QWidget *parent)
  : QWidget(parent)
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

    connect(ui->defaultButton, &QPushButton::clicked, this, &TapePreferencesPathPage::DefaultPath);
    connect(ui->editButton, &QPushButton::clicked, this, &TapePreferencesPathPage::EditPath);
}

//---------------------------------------------------------------------------------------------------------------------
TapePreferencesPathPage::~TapePreferencesPathPage() = default;

//---------------------------------------------------------------------------------------------------------------------
auto TapePreferencesPathPage::Apply() -> QStringList
{
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    settings->SetPathKnownMeasurements(ui->pathTable->item(0, 1)->text());

    return {}; // No changes which require restart.
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesPathPage::changeEvent(QEvent *event)
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
void TapePreferencesPathPage::DefaultPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;

    switch (row)
    {
        case 0: // known measurements
            path = VCommonSettings::GetDefPathKnownMeasurements();
            break;
        default:
            break;
    }

    item->setText(path);
    item->setToolTip(path);
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesPathPage::EditPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // known measurements
            path = MApplication::VApp()->TapeSettings()->GetPathKnownMeasurements();
            break;
        default:
            break;
    }

    bool usedNotExistedDir = false;
    QDir const directory(path);
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

    if (usedNotExistedDir)
    {
        QDir(path).rmpath(QChar('.'));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesPathPage::InitTable()
{
    ui->pathTable->clearContents();
    ui->pathTable->setRowCount(1);
    ui->pathTable->setColumnCount(2);

    const VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    {
        ui->pathTable->setItem(0, 0, new QTableWidgetItem(tr("My known measurements")));
        auto *item = new QTableWidgetItem(settings->GetPathKnownMeasurements());
        item->setToolTip(settings->GetPathKnownMeasurements());
        ui->pathTable->setItem(0, 1, item);
    }

    ui->pathTable->verticalHeader()->setDefaultSectionSize(20);
    ui->pathTable->resizeColumnsToContents();
    ui->pathTable->resizeRowsToContents();
}
