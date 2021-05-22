/************************************************************************
 **
 **  @file   puzzlepreferencesconfigurationpage.cpp
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

#include "puzzlepreferencesconfigurationpage.h"
#include "ui_puzzlepreferencesconfigurationpage.h"
#include "../../vpapplication.h"

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesConfigurationPage::PuzzlePreferencesConfigurationPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PuzzlePreferencesConfigurationPage)
{
    ui->setupUi(this);

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_langChanged = true;
    });

    // Theme
    ui->darkModeCheck->setChecked(VPApplication::VApp()->PuzzleSettings()->GetDarkMode());

    // Native dialogs
    ui->checkBoxDontUseNativeDialog->setChecked(VPApplication::VApp()->PuzzleSettings()->IsDontUseNativeDialog());

    //----------------------- Toolbar
    ui->toolBarStyleCheck->setChecked(VPApplication::VApp()->PuzzleSettings()->GetToolBarStyle());
}

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesConfigurationPage::~PuzzlePreferencesConfigurationPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList PuzzlePreferencesConfigurationPage::Apply()
{
    QStringList preferences;
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    settings->SetToolBarStyle(ui->toolBarStyleCheck->isChecked());

    if (settings->GetDarkMode() != ui->darkModeCheck->isChecked())
    {
        settings->SetDarkMode(ui->darkModeCheck->isChecked());
        preferences.append(tr("dark mode"));
    }

    if (settings->IsDontUseNativeDialog() != ui->checkBoxDontUseNativeDialog->isChecked())
    {
        settings->SetDontUseNativeDialog(ui->checkBoxDontUseNativeDialog->isChecked());
    }

    if (m_langChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        m_langChanged = false;

        VAbstractApplication::VApp()->LoadTranslation(locale);
        qApp->processEvents();// force to call changeEvent
    }

    return preferences;
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesConfigurationPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}
