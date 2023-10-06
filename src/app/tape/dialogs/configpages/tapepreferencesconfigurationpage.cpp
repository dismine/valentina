/************************************************************************
 **
 **  @file   tapepreferencesconfigurationpage.cpp
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

#include "tapepreferencesconfigurationpage.h"
#include "../../mapplication.h"
#include "../../vtapesettings.h"
#include "../qmuparser/qmudef.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/theme/vtheme.h"
#include "../vpatterndb/pmsystems.h"
#include "ui_tapepreferencesconfigurationpage.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#include <QStyleHints>

//---------------------------------------------------------------------------------------------------------------------
TapePreferencesConfigurationPage::TapePreferencesConfigurationPage(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::TapePreferencesConfigurationPage),
    m_langChanged(false),
    m_systemChanged(false)
{
    ui->setupUi(this);
    RetranslateUi();

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { m_langChanged = true; });

    VTapeSettings *settings = MApplication::VApp()->TapeSettings();

    //-------------------- Decimal separator setup
    ui->osOptionCheck->setChecked(settings->GetOsSeparator());

    // Theme
    SetThemeModeComboBox();
    int index = ui->comboBoxThemeMode->findData(static_cast<int>(settings->GetThemeMode()));
    if (index != -1)
    {
        ui->comboBoxThemeMode->setCurrentIndex(index);
    }

    // Native dialogs
    ui->checkBoxDontUseNativeDialog->setChecked(settings->IsDontUseNativeDialog());

    //---------------------- Pattern making system
    ui->systemBookValueLabel->setFixedHeight(4 * QFontMetrics(ui->systemBookValueLabel->font()).lineSpacing());
    connect(ui->systemCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                m_systemChanged = true;
                QString text =
                    VAbstractApplication::VApp()->TrVars()->PMSystemAuthor(ui->systemCombo->currentData().toString());
                ui->systemAuthorValueLabel->setText(text);
                ui->systemAuthorValueLabel->setToolTip(text);

                text = VAbstractApplication::VApp()->TrVars()->PMSystemBook(ui->systemCombo->currentData().toString());
                ui->systemBookValueLabel->setPlainText(text);
            });

    // set default pattern making system
    index = ui->systemCombo->findData(settings->GetPMSystemCode());
    if (index != -1)
    {
        ui->systemCombo->setCurrentIndex(index);
    }

    //----------------------------- Measurements Editing
    connect(ui->resetWarningsButton, &QPushButton::released, this,
            []() { MApplication::VApp()->TapeSettings()->SetConfirmFormatRewriting(true); });

    ui->checkBoxTranslateFormula->setChecked(settings->IsTranslateFormula());

    //----------------------- Toolbar
    ui->toolBarStyleCheck->setChecked(settings->GetToolBarStyle());

    //----------------------- Update
    ui->checkBoxAutomaticallyCheckUpdates->setChecked(settings->IsAutomaticallyCheckUpdates());

    // Tab Privacy
    ui->checkBoxSendUsageStatistics->setChecked(settings->IsCollectStatistic());
}

//---------------------------------------------------------------------------------------------------------------------
TapePreferencesConfigurationPage::~TapePreferencesConfigurationPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto TapePreferencesConfigurationPage::Apply() -> QStringList
{
    QStringList preferences;
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    settings->SetOsSeparator(ui->osOptionCheck->isChecked());

    settings->SetToolBarStyle(ui->toolBarStyleCheck->isChecked());

    auto themeMode = static_cast<VThemeMode>(ui->comboBoxThemeMode->currentData().toInt());
    if (settings->GetThemeMode() != themeMode)
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        if (themeMode == VThemeMode::System && VTheme::NativeDarkThemeAvailable())
        {
            if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
            {
                settings->SetThemeMode(VThemeMode::Light);
            }
            else
            {
                settings->SetThemeMode(VThemeMode::Dark);
            }

            VTheme::Instance()->ResetThemeSettings();
            QCoreApplication::processEvents();
        }
#endif

        settings->SetThemeMode(themeMode);
        VTheme::Instance()->ResetThemeSettings();
        QGuiApplication::restoreOverrideCursor();
    }

    settings->SetTranslateFormula(ui->checkBoxTranslateFormula->isChecked());

    if (settings->IsDontUseNativeDialog() != ui->checkBoxDontUseNativeDialog->isChecked())
    {
        settings->SetDontUseNativeDialog(ui->checkBoxDontUseNativeDialog->isChecked());
    }

    if (m_langChanged || m_systemChanged)
    {
        const auto locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        VGAnalytics::Instance()->SetGUILanguage(settings->GetLocale());
        m_langChanged = false;

        const auto code = qvariant_cast<QString>(ui->systemCombo->currentData());
        settings->SetPMSystemCode(code);
        m_systemChanged = false;

        VAbstractApplication::VApp()->LoadTranslation(locale);
        QCoreApplication::processEvents(); // force to call changeEvent

        // Part about measurments will not be updated automatically
        MApplication::VApp()->RetranslateTables();
        MApplication::VApp()->RetranslateGroups();
    }

    if (settings->IsAutomaticallyCheckUpdates() != ui->checkBoxAutomaticallyCheckUpdates->isChecked())
    {
        settings->SetAutomaticallyCheckUpdates(ui->checkBoxAutomaticallyCheckUpdates->isChecked());
    }

    // Tab Privacy
    settings->SetCollectStatistic(ui->checkBoxSendUsageStatistics->isChecked());
    VGAnalytics::Instance()->Enable(ui->checkBoxSendUsageStatistics->isChecked());

    return preferences;
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        RetranslateUi();
        ui->retranslateUi(this);
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::RetranslateUi()
{
    ui->osOptionCheck->setText(tr("With OS options") + QStringLiteral(" (%1)").arg(LocaleDecimalPoint(QLocale())));

    {
        const auto code = qvariant_cast<QString>(ui->systemCombo->currentData());
        ui->systemCombo->blockSignals(true);
        ui->systemCombo->clear();
        InitPMSystems(ui->systemCombo);
        ui->systemCombo->setCurrentIndex(-1);
        ui->systemCombo->blockSignals(false);
        ui->systemCombo->setCurrentIndex(ui->systemCombo->findData(code));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::SetThemeModeComboBox()
{
    ui->comboBoxThemeMode->clear();
    ui->comboBoxThemeMode->addItem(tr("System", "theme"), static_cast<int>(VThemeMode::System));
    ui->comboBoxThemeMode->addItem(tr("Dark", "theme"), static_cast<int>(VThemeMode::Dark));
    ui->comboBoxThemeMode->addItem(tr("Light", "theme"), static_cast<int>(VThemeMode::Light));
}
