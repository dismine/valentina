/************************************************************************
 **
 **  @file   preferencespathpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "preferencespathpage.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vvalentinasettings.h"
#include "ui_preferencespathpage.h"

#include <QDir>
#include <QFileDialog>

//---------------------------------------------------------------------------------------------------------------------
PreferencesPathPage::PreferencesPathPage(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::PreferencesPathPage)
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

    connect(ui->defaultButton, &QPushButton::clicked, this, &PreferencesPathPage::DefaultPath);
    connect(ui->editButton, &QPushButton::clicked, this, &PreferencesPathPage::EditPath);
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesPathPage::~PreferencesPathPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto PreferencesPathPage::Apply() -> QStringList
{
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetPathIndividualMeasurements(ui->pathTable->item(0, 1)->text());
    settings->SetPathMultisizeMeasurements(ui->pathTable->item(1, 1)->text());
    settings->SetPathPattern(ui->pathTable->item(2, 1)->text());
    settings->SetPathLayout(ui->pathTable->item(3, 1)->text());
    settings->SetPathLabelTemplate(ui->pathTable->item(4, 1)->text());
    settings->SetPathManualLayouts(ui->pathTable->item(5, 1)->text());
    settings->SetPathSVGFonts(ui->pathTable->item(6, 1)->text());
    settings->SetPathFontCorrections(ui->pathTable->item(7, 1)->text());

    return {}; // No changes those require restart.
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::changeEvent(QEvent *event)
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
void PreferencesPathPage::DefaultPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;

    switch (row)
    {
        case 1: // multisize measurements
            path = VCommonSettings::GetDefPathMultisizeMeasurements();
            break;
        case 2: // pattern path
            path = VValentinaSettings::GetDefPathPattern();
            break;
        case 0: // individual measurements
            path = VCommonSettings::GetDefPathIndividualMeasurements();
            break;
        case 3: // layout path
            path = VValentinaSettings::GetDefPathLayout();
            break;
        case 4: // label templates
            path = VValentinaSettings::GetDefPathLabelTemplate();
            break;
        case 5: // manual layouts
            path = VCommonSettings::GetDefPathManualLayouts();
            break;
        case 6: // svg fonts
            path = VCommonSettings::GetDefPathSVGFonts();
            break;
        case 7: // font corrections
            path = VCommonSettings::GetDefPathFontCorrections();
            break;
        default:
            break;
    }

    item->setText(path);
    item->setToolTip(path);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::EditPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // individual measurements
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathIndividualMeasurements();
            break;
        case 1: // multisize measurements
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathMultisizeMeasurements();
            path = VCommonSettings::PrepareMultisizeTables(path);
            break;
        case 2: // pattern path
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathPattern();
            break;
        case 3: // layout path
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathLayout();
            break;
        case 4: // label templates
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathLabelTemplate();
            break;
        case 5: // manual layouts
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathManualLayouts();
            break;
        case 6: // svg fonts
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathSVGFonts();
            break;
        case 7: // font corrections
            path = VAbstractValApplication::VApp()->ValentinaSettings()->GetPathFontCorrections();
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

    if (usedNotExistedDir)
    {
        QDir(path).rmpath(QChar('.'));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::InitTable()
{
    ui->pathTable->clearContents();
    ui->pathTable->setRowCount(8);
    ui->pathTable->setColumnCount(2);

    const VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    {
        ui->pathTable->setItem(0, 0, new QTableWidgetItem(tr("My Individual Measurements")));
        auto *item = new QTableWidgetItem(settings->GetPathIndividualMeasurements());
        item->setToolTip(settings->GetPathIndividualMeasurements());
        ui->pathTable->setItem(0, 1, item);
    }

    {
        ui->pathTable->setItem(1, 0, new QTableWidgetItem(tr("My Multisize Measurements")));
        auto *item = new QTableWidgetItem(settings->GetPathMultisizeMeasurements());
        item->setToolTip(settings->GetPathMultisizeMeasurements());
        ui->pathTable->setItem(1, 1, item);
    }

    {
        ui->pathTable->setItem(2, 0, new QTableWidgetItem(tr("My Patterns")));
        auto *item = new QTableWidgetItem(settings->GetPathPattern());
        item->setToolTip(settings->GetPathPattern());
        ui->pathTable->setItem(2, 1, item);
    }

    {
        ui->pathTable->setItem(3, 0, new QTableWidgetItem(tr("My Layouts")));
        auto *item = new QTableWidgetItem(settings->GetPathLayout());
        item->setToolTip(settings->GetPathLayout());
        ui->pathTable->setItem(3, 1, item);
    }

    {
        ui->pathTable->setItem(4, 0, new QTableWidgetItem(tr("My label templates")));
        auto *item = new QTableWidgetItem(settings->GetPathLabelTemplate());
        item->setToolTip(settings->GetPathLabelTemplate());
        ui->pathTable->setItem(4, 1, item);
    }

    {
        ui->pathTable->setItem(5, 0, new QTableWidgetItem(tr("My manual layouts")));
        auto *item = new QTableWidgetItem(settings->GetPathManualLayouts());
        item->setToolTip(settings->GetPathManualLayouts());
        ui->pathTable->setItem(5, 1, item);
    }

    {
        ui->pathTable->setItem(6, 0, new QTableWidgetItem(tr("My SVG Fonts")));
        auto *item = new QTableWidgetItem(settings->GetPathSVGFonts());
        item->setToolTip(settings->GetPathSVGFonts());
        ui->pathTable->setItem(6, 1, item);
    }

    {
        ui->pathTable->setItem(7, 0, new QTableWidgetItem(tr("My font corrections")));
        auto *item = new QTableWidgetItem(settings->GetPathFontCorrections());
        item->setToolTip(settings->GetPathFontCorrections());
        ui->pathTable->setItem(7, 1, item);
    }

    ui->pathTable->verticalHeader()->setDefaultSectionSize(20);
    ui->pathTable->resizeColumnsToContents();
    ui->pathTable->resizeRowsToContents();
}
