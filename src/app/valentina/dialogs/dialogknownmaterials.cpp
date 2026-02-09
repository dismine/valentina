/************************************************************************
 **
 **  @file   dialogknownmaterials.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2017
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

#include "dialogknownmaterials.h"
#include "ui_dialogknownmaterials.h"

//---------------------------------------------------------------------------------------------------------------------
DialogKnownMaterials::DialogKnownMaterials(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogKnownMaterials)
{
    ui->setupUi(this);

    ui->lineEditMaterial->setClearButtonEnabled(true);

    connect(ui->toolButtonAdd, &QToolButton::clicked, this, &DialogKnownMaterials::Add);
    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &DialogKnownMaterials::Remove);
    connect(ui->lineEditMaterial, &QLineEdit::textEdited, this, &DialogKnownMaterials::SaveText);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &DialogKnownMaterials::ShowDetails);
}

//---------------------------------------------------------------------------------------------------------------------
DialogKnownMaterials::~DialogKnownMaterials()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMaterials::SetList(const QStringList &list)
{
    QSignalBlocker blocker(ui->listWidget);
    ui->listWidget->clear();

    int row = -1;

    for (const auto &m : list)
    {
        if (not m.isEmpty())
        {
            ui->listWidget->insertItem(++row, new QListWidgetItem(m));
        }
    }

    blocker.unblock();

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogKnownMaterials::GetList() const -> QStringList
{
    QStringList list;

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *item = ui->listWidget->item(i))
        {
            if (not item->text().isEmpty())
            {
                list.append(item->text());
            }
        }
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMaterials::ShowDetails()
{
    if (ui->listWidget->count() > 0)
    {
        const QListWidgetItem *line = ui->listWidget->currentItem();
        if (line)
        {
            const QSignalBlocker blocker(ui->lineEditMaterial);
            ui->lineEditMaterial->setText(line->text());
        }
    }

    SetupControls();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMaterials::Add()
{
    int row = ui->listWidget->currentRow();
    ui->listWidget->insertItem(++row, new QListWidgetItem(tr("User material")));
    ui->listWidget->setCurrentRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMaterials::Remove()
{
    {
        const QSignalBlocker blocker(ui->listWidget);
        delete ui->listWidget->takeItem(ui->listWidget->currentRow());
    }
    ShowDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMaterials::SaveText(const QString &text)
{
    QListWidgetItem *curLine = ui->listWidget->currentItem();
    if (curLine)
    {
        curLine->setText(text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogKnownMaterials::SetupControls()
{
    const bool enabled = ui->listWidget->count() > 0;

    if (not enabled)
    {
        const QSignalBlocker blocker(ui->lineEditMaterial);
        ui->lineEditMaterial->clear();
    }

    ui->toolButtonAdd->setEnabled(true);

    ui->toolButtonRemove->setEnabled(enabled);
    ui->lineEditMaterial->setEnabled(enabled);
}
