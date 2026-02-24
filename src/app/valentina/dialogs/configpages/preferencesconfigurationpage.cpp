/************************************************************************
 **
 **  @file   preferencesconfigurationpage.cpp
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

#include "preferencesconfigurationpage.h"
#include "../../core/vapplication.h"
#include "../qmuparser/qmudef.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/dialogs/vshortcutdialog.h"
#include "../vmisc/literals.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractshortcutmanager.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vwidgets/vmousewheelwidgetadjustmentguard.h"
#include "ui_preferencesconfigurationpage.h"
#include "vcommonsettings.h"

#include <QDir>
#include <QDirIterator>
#include <QPushButton>
#include <QStyleHints>
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
PreferencesConfigurationPage::PreferencesConfigurationPage(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::PreferencesConfigurationPage)
{
    ui->setupUi(this);
    RetranslateUi();

    // Prevent stealing focus when scrolling
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->autoTime);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->langCombo);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxPieceLbelLanguage);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->unitCombo);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->labelCombo);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxThemeMode);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxPointerMode);

    ui->tabWidget->setCurrentIndex(0);

    // Tab General
    ui->autoSaveCheck->setChecked(VAbstractValApplication::VApp()->ValentinaSettings()->GetAutosaveState());

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { m_langChanged = true; });

    InitPieceLabelLanguages(ui->comboBoxPieceLbelLanguage);
    connect(ui->comboBoxPieceLbelLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { m_pieceLabelLangChanged = true; });

    //-------------------- Decimal separator setup
    ui->osOptionCheck->setChecked(VAbstractValApplication::VApp()->ValentinaSettings()->GetOsSeparator());

    //----------------------- Unit setup
    // set default unit
    if (const qint32 indexUnit =
            ui->unitCombo->findData(VAbstractValApplication::VApp()->ValentinaSettings()->GetUnit());
        indexUnit != -1)
    {
        ui->unitCombo->setCurrentIndex(indexUnit);
    }

    connect(ui->unitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { m_unitChanged = true; });

    //----------------------- Label language
    SetLabelComboBox(VApplication::LabelLanguages());

    int index = ui->labelCombo->findData(VAbstractValApplication::VApp()->ValentinaSettings()->GetLabelLanguage());
    if (index != -1)
    {
        ui->labelCombo->setCurrentIndex(index);
    }
    connect(ui->labelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { m_labelLangChanged = true; });

    //----------------------------- Pattern Editing
    connect(ui->resetWarningsButton, &QPushButton::released, this,
            []()
            {
                VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

                settings->SetConfirmItemDelete(true);
                settings->SetConfirmFormatRewriting(true);
                settings->SetAskContinueIfLayoutStale(true);
            });

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();

    ui->checkBoxFreeCurve->setChecked(settings->IsFreeCurveMode());
    ui->checkBoxZoomFitBestCurrentPP->setChecked(settings->IsDoubleClickZoomFitBestCurrentPP());
    ui->checkBoxInteractiveTools->setChecked(settings->IsInteractiveTools());
    ui->checkBoxTranslateFormula->setChecked(settings->IsTranslateFormula());
    ui->checkBoxGarbageCollector->setChecked(settings->IsCollectGarbage());

    //----------------------- Toolbar
    ui->toolBarStyleCheck->setChecked(settings->GetToolBarStyle());
    ui->radioButtonToolboxIconSizeNormal->setChecked(!settings->GetToolboxIconSizeSmall());

    // Theme
    SetThemeModeComboBox();
    index = ui->comboBoxThemeMode->findData(static_cast<int>(settings->GetThemeMode()));
    if (index != -1)
    {
        ui->comboBoxThemeMode->setCurrentIndex(index);
    }

    // Pointer mode
    SetPointerModeComboBox();
    index = ui->comboBoxPointerMode->findData(static_cast<int>(settings->GetPointerMode()));
    if (index != -1)
    {
        ui->comboBoxPointerMode->setCurrentIndex(index);
    }

    // Use tool groups
    ui->checkBoxUseToolGroups->setChecked(settings->IsUseToolGroups());

    // Native dialogs
    ui->checkBoxDontUseNativeDialog->setChecked(settings->IsDontUseNativeDialog());

    //----------------------- Update
    ui->checkBoxAutomaticallyCheckUpdates->setChecked(settings->IsAutomaticallyCheckUpdates());

    // Tab Shortcuts
    InitShortcuts();
    connect(ui->pushButtonRestoreDefaults, &QPushButton::clicked, this, [this]() { InitShortcuts(true); });
    connect(ui->shortcutsTable, &QTableWidget::cellDoubleClicked, this,
            &PreferencesConfigurationPage::ShortcutCellDoubleClicked);

    // Tab Scrolling
    ui->spinBoxDuration->setMinimum(VCommonSettings::scrollingDurationMin);
    ui->spinBoxDuration->setMaximum(VCommonSettings::scrollingDurationMax);
    ui->spinBoxDuration->setValue(settings->GetScrollingDuration());

    ui->spinBoxUpdateInterval->setMinimum(VCommonSettings::scrollingUpdateIntervalMin);
    ui->spinBoxUpdateInterval->setMaximum(VCommonSettings::scrollingUpdateIntervalMax);
    ui->spinBoxUpdateInterval->setValue(settings->GetScrollingUpdateInterval());

    ui->doubleSpinBoxSensor->setMinimum(VCommonSettings::sensorMouseScaleMin);
    ui->doubleSpinBoxSensor->setMaximum(VCommonSettings::sensorMouseScaleMax);
    ui->doubleSpinBoxSensor->setValue(settings->GetSensorMouseScale());

    ui->doubleSpinBoxWheel->setMinimum(VCommonSettings::wheelMouseScaleMin);
    ui->doubleSpinBoxWheel->setMaximum(VCommonSettings::wheelMouseScaleMax);
    ui->doubleSpinBoxWheel->setValue(settings->GetWheelMouseScale());

    ui->doubleSpinBoxAcceleration->setMinimum(VCommonSettings::scrollingAccelerationMin);
    ui->doubleSpinBoxAcceleration->setMaximum(VCommonSettings::scrollingAccelerationMax);
    ui->doubleSpinBoxAcceleration->setValue(settings->GetScrollingAcceleration());

    // Tab Privacy
    ui->checkBoxSendUsageStatistics->setChecked(settings->IsCollectStatistic());

#if !defined(CRASH_REPORTING)
    ui->groupBoxCrashReports->setDisabled(true);
    ui->checkBoxSendCrashReports->setChecked(false);
    ui->lineEditCrashUserEmail->setText(QString());
#else
    ui->checkBoxSendCrashReports->setChecked(settings->IsSendCrashReport());

    connect(ui->checkBoxSendCrashReports, CHECKBOX_STATE_CHANGED, this, [this]() { m_sendCrashReportsChanged = true; });

    QRegularExpression const rx(QStringLiteral("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b"),
                                QRegularExpression::CaseInsensitiveOption);
    ui->lineEditCrashUserEmail->setValidator(new QRegularExpressionValidator(rx, this));
    ui->lineEditCrashUserEmail->setText(settings->GetCrashEmail());
    connect(ui->lineEditCrashUserEmail, &QLineEdit::editingFinished, this,
            [this]() { m_crashUserEmailChanged = true; });
#endif
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesConfigurationPage::~PreferencesConfigurationPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto PreferencesConfigurationPage::Apply() -> QStringList
{
    // Tab General
    QStringList preferences;
    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->SetAutosaveState(ui->autoSaveCheck->isChecked());
    settings->SetAutosaveTime(ui->autoTime->value());

    QTimer *autoSaveTimer = VApplication::VApp()->getAutoSaveTimer();
    SCASSERT(autoSaveTimer)

    ui->autoSaveCheck->isChecked() ? autoSaveTimer->start(std::chrono::minutes{ui->autoTime->value()})
                                   : autoSaveTimer->stop();

    settings->SetOsSeparator(ui->osOptionCheck->isChecked());
    settings->SetToolBarStyle(ui->toolBarStyleCheck->isChecked());
    settings->SetToolboxIconSizeSmall(ui->radioButtonToolboxIconSizeSmall->isChecked());

    if (auto themeMode = static_cast<VThemeMode>(ui->comboBoxThemeMode->currentData().toInt());
        settings->GetThemeMode() != themeMode)
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

    settings->SetPointerMode(static_cast<VToolPointerMode>(ui->comboBoxPointerMode->currentData().toInt()));

    if (settings->IsUseToolGroups() != ui->checkBoxUseToolGroups->isChecked())
    {
        settings->SetUseToolGroups(ui->checkBoxUseToolGroups->isChecked());
    }

    if (settings->IsDontUseNativeDialog() != ui->checkBoxDontUseNativeDialog->isChecked())
    {
        settings->SetDontUseNativeDialog(ui->checkBoxDontUseNativeDialog->isChecked());
    }

    settings->SetFreeCurveMode(ui->checkBoxFreeCurve->isChecked());
    settings->SetDoubleClickZoomFitBestCurrentPP(ui->checkBoxZoomFitBestCurrentPP->isChecked());
    settings->SetInteractiveTools(ui->checkBoxInteractiveTools->isChecked());
    settings->SetTranslateFormula(ui->checkBoxTranslateFormula->isChecked());
    settings->SetCollectGarbage(ui->checkBoxGarbageCollector->isChecked());

    if (m_pieceLabelLangChanged)
    {
        const auto locale = qvariant_cast<QString>(ui->comboBoxPieceLbelLanguage->currentData());
        settings->SetPieceLabelLocale(locale);
        m_pieceLabelLangChanged = false;
    }

    if (m_langChanged)
    {
        const auto locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        VGAnalytics::Instance()->SetGUILanguage(settings->GetLocale());
        m_langChanged = false;

        VAbstractApplication::VApp()->LoadTranslation(locale);
    }

    if (m_unitChanged)
    {
        const auto unit = qvariant_cast<QString>(ui->unitCombo->currentData());
        settings->SetUnit(unit);
        m_unitChanged = false;
        preferences.append(tr("default unit"));
    }

    if (m_labelLangChanged)
    {
        const auto locale = qvariant_cast<QString>(ui->labelCombo->currentData());
        settings->SetLabelLanguage(locale);
        m_labelLangChanged = false;
    }

    if (settings->IsAutomaticallyCheckUpdates() != ui->checkBoxAutomaticallyCheckUpdates->isChecked())
    {
        settings->SetAutomaticallyCheckUpdates(ui->checkBoxAutomaticallyCheckUpdates->isChecked());
    }

    // Tab Shortcuts
    if (VAbstractShortcutManager *manager = VAbstractValApplication::VApp()->GetShortcutManager())
    {
        const auto &shortcutsList = manager->GetShortcutsList();
        for (int i = 0; i < m_transientShortcuts.length(); i++)
        {
            settings->SetActionShortcuts(VAbstractShortcutManager::ShortcutActionToString(shortcutsList.value(i).type),
                                         m_transientShortcuts.value(i));
        }

        manager->UpdateShortcuts();
    }

    // Tab Scrolling
    settings->SetScrollingDuration(ui->spinBoxDuration->value());
    settings->SetScrollingUpdateInterval(ui->spinBoxUpdateInterval->value());
    settings->SetSensorMouseScale(ui->doubleSpinBoxSensor->value());
    settings->SetWheelMouseScale(ui->doubleSpinBoxWheel->value());
    settings->SetScrollingAcceleration(ui->doubleSpinBoxAcceleration->value());

    // Tab Privacy
    settings->SetCollectStatistic(ui->checkBoxSendUsageStatistics->isChecked());
    VGAnalytics::Instance()->Enable(ui->checkBoxSendUsageStatistics->isChecked());

#if defined(CRASH_REPORTING)
    if (m_sendCrashReportsChanged)
    {
        settings->SeSendCrashReport(ui->checkBoxSendCrashReports->isChecked());
        m_sendCrashReportsChanged = false;
        preferences.append(tr("send crash report"));
    }

    if (m_crashUserEmailChanged)
    {
        settings->SetCrashEmail(ui->lineEditCrashUserEmail->text());
        m_crashUserEmailChanged = false;
        preferences.append(tr("user email in case of crash"));
    }
#endif

    return preferences;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::changeEvent(QEvent *event)
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
void PreferencesConfigurationPage::ShortcutCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    auto *shortcutDialog = new VShortcutDialog(row, this);
    connect(shortcutDialog, &VShortcutDialog::ShortcutsListChanged, this,
            [this](int index, const QStringList &stringListShortcuts)
            {
                m_transientShortcuts.replace(index, stringListShortcuts);
                UpdateShortcutsTable();
            });
    shortcutDialog->open();
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::SetLabelComboBox(const QStringList &list)
{
    for (const auto &name : list)
    {
        ui->labelCombo->addItem(QLocale(name).nativeLanguageName(), name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::SetThemeModeComboBox()
{
    ui->comboBoxThemeMode->clear();
    ui->comboBoxThemeMode->addItem(tr("System", "theme"), static_cast<int>(VThemeMode::System));
    ui->comboBoxThemeMode->addItem(tr("Dark", "theme"), static_cast<int>(VThemeMode::Dark));
    ui->comboBoxThemeMode->addItem(tr("Light", "theme"), static_cast<int>(VThemeMode::Light));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::SetPointerModeComboBox()
{
    ui->comboBoxPointerMode->clear();
    ui->comboBoxPointerMode->addItem(tr("Tool icon cursor"), static_cast<int>(VToolPointerMode::ToolIcon));
    ui->comboBoxPointerMode->addItem(tr("Arrow cursor"), static_cast<int>(VToolPointerMode::Arrow));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::InitUnits()
{
    ui->unitCombo->addItem(tr("Centimeters"), unitCM);
    ui->unitCombo->addItem(tr("Millimiters"), unitMM);
    ui->unitCombo->addItem(tr("Inches"), unitINCH);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::RetranslateUi()
{
    ui->osOptionCheck->setText(tr("With OS options") + QStringLiteral(" (%1)").arg(LocaleDecimalPoint(QLocale())));

    {
        const QSignalBlocker blocker(ui->unitCombo);
        const auto unit = qvariant_cast<QString>(ui->unitCombo->currentData());
        ui->unitCombo->clear();
        InitUnits();
        ui->unitCombo->setCurrentIndex(ui->unitCombo->findData(unit));
    }

    {
        QSignalBlocker blocker(ui->comboBoxPieceLbelLanguage);
        const auto code = qvariant_cast<QString>(ui->comboBoxPieceLbelLanguage->currentData());
        InitPieceLabelLanguages(ui->comboBoxPieceLbelLanguage);
        ui->comboBoxPieceLbelLanguage->setCurrentIndex(-1);
        blocker.unblock();
        ui->comboBoxPieceLbelLanguage->setCurrentIndex(ui->comboBoxPieceLbelLanguage->findData(code));
    }

    RetranslateShortcutsTable();
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::InitShortcuts(bool defaults)
{
    VAbstractShortcutManager *manager = VAbstractValApplication::VApp()->GetShortcutManager();
    if (manager == nullptr)
    {
        return;
    }

    manager->UpdateShortcuts();
    m_transientShortcuts.clear();
    ui->shortcutsTable->clearContents();
    const auto &shortcutsList = manager->GetShortcutsList();
    ui->shortcutsTable->setRowCount(static_cast<int>(shortcutsList.length()));

    for (int i = 0; i < shortcutsList.length(); i++)
    {
        const VAbstractShortcutManager::VSShortcut &shortcut = shortcutsList.value(i);

        // Add shortcut to transient shortcut list
        if (defaults)
        {
            m_transientShortcuts.append(shortcut.defaultShortcuts);
        }
        else
        {
            m_transientShortcuts.append(shortcut.shortcuts);
        }

        // Add shortcut to table widget
        auto *nameItem = new QTableWidgetItem();
        nameItem->setText(VAbstractShortcutManager::ReadableName(shortcut.type));
        ui->shortcutsTable->setItem(i, 0, nameItem);

        auto *shortcutsItem = new QTableWidgetItem();
        shortcutsItem->setText(VAbstractShortcutManager::StringListToReadableString(m_transientShortcuts.value(i)));
        ui->shortcutsTable->setItem(i, 1, shortcutsItem);
    }
    UpdateShortcutsTable();
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::UpdateShortcutsTable()
{
    for (int i = 0; i < m_transientShortcuts.length(); i++)
    {
        const QStringList &shortcuts = m_transientShortcuts.value(i);
        ui->shortcutsTable->item(i, 1)->setText(VAbstractShortcutManager::StringListToReadableString(shortcuts));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::RetranslateShortcutsTable()
{
    VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager();
    if (manager == nullptr)
    {
        return;
    }

    const auto &shortcutsList = manager->GetShortcutsList();
    for (int i = 0; i < shortcutsList.length(); i++)
    {
        const VAbstractShortcutManager::VSShortcut &shortcut = shortcutsList.value(i);

        if (QTableWidgetItem *it = ui->shortcutsTable->item(i, 0))
        {
            it->setText(VAbstractShortcutManager::ReadableName(shortcut.type));
        }
    }
}
