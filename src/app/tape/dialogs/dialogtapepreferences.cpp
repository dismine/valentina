/************************************************************************
 **
 **  @file   dialogtapepreferences.cpp
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

#include "dialogtapepreferences.h"
#include "../mapplication.h"
#include "../vtools/dialogs/dialogtoolbox.h"
#include "configpages/tapepreferencesconfigurationpage.h"
#include "ui_dialogtapepreferences.h"

#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>

//---------------------------------------------------------------------------------------------------------------------
DialogTapePreferences::DialogTapePreferences(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogTapePreferences),
    m_configurationPage(new TapePreferencesConfigurationPage)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    connect(bOk, &QPushButton::clicked, this, &DialogTapePreferences::Ok);

    QPushButton *bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(bApply != nullptr)
    connect(bApply, &QPushButton::clicked, this, &DialogTapePreferences::Apply);

    ui->pagesWidget->insertWidget(0, m_configurationPage);

    connect(ui->contentsWidget, &QListWidget::currentItemChanged, this, &DialogTapePreferences::PageChanged);
    ui->pagesWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------------------------------------------------
DialogTapePreferences::~DialogTapePreferences()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTapePreferences::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    QSize sz = VAbstractApplication::VApp()->Settings()->GetPreferenceDialogSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTapePreferences::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (m_isInitialized)
    {
        VAbstractApplication::VApp()->Settings()->SetPreferenceDialogSize(size());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTapePreferences::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitDialogButtonBoxIcons(ui->buttonBox);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTapePreferences::Apply()
{
    QStringList preferences;

    preferences += m_configurationPage->Apply();

    if (not preferences.isEmpty())
    {
        const QString text =
            tr("Followed %n option(s) require restart to take effect: %1.", "", static_cast<int>(preferences.size()))
                .arg(preferences.join(QStringLiteral(", ")));
        QMessageBox::information(this, QCoreApplication::applicationName(), text);
    }

    MApplication::VApp()->TapeSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    emit UpdateProperties();
    setResult(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTapePreferences::Ok()
{
    Apply();
    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTapePreferences::PageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    int rowIndex = ui->contentsWidget->row(current);
    ui->pagesWidget->setCurrentIndex(rowIndex);
}
