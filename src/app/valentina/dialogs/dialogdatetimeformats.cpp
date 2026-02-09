/************************************************************************
 **
 **  @file   dialogdatetimeformats.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 8, 2017
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

#include "dialogdatetimeformats.h"
#include "ui_dialogdatetimeformats.h"

//---------------------------------------------------------------------------------------------------------------------
DialogDateTimeFormats::DialogDateTimeFormats(const QDate &date, const QStringList &predefinedFormats,
                                             const QStringList &userDefinedFormats, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogDateTimeFormats),
    m_dateMode(true),
    m_date(date),
    m_predefined(predefinedFormats)
{
    ui->setupUi(this);

    Init(predefinedFormats, userDefinedFormats);
}

//---------------------------------------------------------------------------------------------------------------------
DialogDateTimeFormats::DialogDateTimeFormats(const QTime &time, const QStringList &predefinedFormats,
                                             const QStringList &userDefinedFormats, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogDateTimeFormats),
    m_dateMode(false),
    m_time(time),
    m_predefined(predefinedFormats)
{
    ui->setupUi(this);

    Init(predefinedFormats, userDefinedFormats);
}

//---------------------------------------------------------------------------------------------------------------------
DialogDateTimeFormats::~DialogDateTimeFormats()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogDateTimeFormats::GetFormats() const -> QStringList
{
    QStringList formats;

    for (int i = 0; i < ui->listWidget->count(); ++i)
    {
        if (const QListWidgetItem *lineItem = ui->listWidget->item(i))
        {
            const QString format = lineItem->data(Qt::UserRole).toString();
            if (not format.isEmpty())
            {
                formats.append(lineItem->data(Qt::UserRole).toString());
            }
        }
    }

    return formats;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::AddLine()
{
    int row = ui->listWidget->count();
    ui->listWidget->insertItem(++row, new QListWidgetItem(tr("<empty>")));
    ui->listWidget->setCurrentRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::RemoveLine()
{
    {
        const QSignalBlocker blocker(ui->listWidget);
        if (QListWidgetItem *curLine = ui->listWidget->currentItem())
        {
            if (not m_predefined.contains(curLine->data(Qt::UserRole).toString()))
            {
                delete ui->listWidget->takeItem(ui->listWidget->currentRow());
            }
        }
    }
    ShowFormatDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::SaveFormat(const QString &text)
{
    if (QListWidgetItem *curLine = ui->listWidget->currentItem())
    {
        if (not GetFormats().contains(text))
        {
            const QString preview = m_dateMode ? m_date.toString(text) : m_time.toString(text);
            curLine->setText(preview);
            curLine->setData(Qt::UserRole, text);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::ShowFormatDetails()
{
    if (ui->listWidget->count() > 0)
    {
        if (const QListWidgetItem *line = ui->listWidget->currentItem())
        {
            const QSignalBlocker blocker(ui->lineEditFormat);
            ui->lineEditFormat->setText(line->data(Qt::UserRole).toString());
        }
    }

    SetupControls();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::Init(const QStringList &predefined, const QStringList &userDefined)
{
    SetFormatLines(predefined, userDefined);

    ui->lineEditFormat->setClearButtonEnabled(true);

    connect(ui->toolButtonRemove, &QToolButton::clicked, this, &DialogDateTimeFormats::RemoveLine);
    connect(ui->toolButtonAdd, &QToolButton::clicked, this, &DialogDateTimeFormats::AddLine);
    connect(ui->lineEditFormat, &QLineEdit::textEdited, this, &DialogDateTimeFormats::SaveFormat);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &DialogDateTimeFormats::ShowFormatDetails);

    ui->listWidget->setCurrentRow(0);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::SetFormatLines(const QStringList &predefined, const QStringList &userDefined)
{
    QSignalBlocker blocker(ui->listWidget);
    ui->listWidget->clear();

    int row = -1;

    for (const auto &item : predefined)
    {
        ui->listWidget->insertItem(++row, AddListLine(item));
    }

    for (const auto &item : userDefined)
    {
        ui->listWidget->insertItem(++row, AddListLine(item));
    }

    blocker.unblock();

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogDateTimeFormats::SetupControls()
{
    const bool enabled = ui->listWidget->count() > 0;

    if (not enabled)
    {
        const QSignalBlocker blocker(ui->lineEditFormat);
        ui->lineEditFormat->clear();
    }

    ui->toolButtonAdd->setEnabled(true);

    ui->lineEditFormat->setEnabled(enabled);

    const QListWidgetItem *line = ui->listWidget->currentItem();
    if (line != nullptr && m_predefined.contains(line->data(Qt::UserRole).toString()))
    {
        ui->toolButtonRemove->setEnabled(false);
        ui->lineEditFormat->setReadOnly(true);
    }
    else
    {
        ui->toolButtonRemove->setEnabled(enabled);
        ui->lineEditFormat->setReadOnly(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogDateTimeFormats::AddListLine(const QString &format) -> QListWidgetItem *
{
    const QString preview = m_dateMode ? m_date.toString(format) : m_time.toString(format);
    auto *item = new QListWidgetItem(preview);
    item->setData(Qt::UserRole, format);
    return item;
}
