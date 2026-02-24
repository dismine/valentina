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
#include "../../vpapplication.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/dialogs/vshortcutdialog.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractshortcutmanager.h"
#include "../vwidgets/vmousewheelwidgetadjustmentguard.h"
#include "ui_puzzlepreferencesconfigurationpage.h"

#include <QStyleHints>

//---------------------------------------------------------------------------------------------------------------------
PuzzlePreferencesConfigurationPage::PuzzlePreferencesConfigurationPage(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::PuzzlePreferencesConfigurationPage>())
{
    ui->setupUi(this);

    // Prevent stealing focus when scrolling
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->langCombo);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxThemeMode);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->undoCount);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->spinBoxDuration);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->spinBoxUpdateInterval);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxSensor);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxWheel);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->doubleSpinBoxAcceleration);

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]() { m_langChanged = true; });

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    // Theme
    SetThemeModeComboBox();
    if (int const index = ui->comboBoxThemeMode->findData(static_cast<int>(settings->GetThemeMode())); index != -1)
    {
        ui->comboBoxThemeMode->setCurrentIndex(index);
    }

    // Native dialogs
    ui->checkBoxDontUseNativeDialog->setChecked(settings->IsDontUseNativeDialog());

    //----------------------- Toolbar
    ui->toolBarStyleCheck->setChecked(settings->GetToolBarStyle());

    // Undo
    ui->undoCount->setValue(settings->GetUndoCount());

    // Graphical output
    ui->graphOutputCheck->setChecked(settings->GetGraphicalOutput());
    ui->checkBoxOpenGLRender->setChecked(settings->IsOpenGLRender());

    // Font
    ui->checkBoxSingleStrokeOutlineFont->setChecked(settings->GetSingleStrokeOutlineFont());
    ui->checkBoxSingleLineFonts->setChecked(settings->GetSingleLineFonts());

    //----------------------- Update
    ui->checkBoxAutomaticallyCheckUpdates->setChecked(settings->IsAutomaticallyCheckUpdates());

    // Tab Shortcuts
    InitShortcuts();
    connect(ui->pushButtonRestoreDefaults, &QPushButton::clicked, this, [this]() { InitShortcuts(true); });
    connect(ui->shortcutsTable, &QTableWidget::cellDoubleClicked, this,
            &PuzzlePreferencesConfigurationPage::ShortcutCellDoubleClicked);

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
PuzzlePreferencesConfigurationPage::~PuzzlePreferencesConfigurationPage() = default;

//---------------------------------------------------------------------------------------------------------------------
auto PuzzlePreferencesConfigurationPage::Apply() -> QStringList
{
    QStringList preferences;
    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    settings->SetToolBarStyle(ui->toolBarStyleCheck->isChecked());

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

    if (settings->IsDontUseNativeDialog() != ui->checkBoxDontUseNativeDialog->isChecked())
    {
        settings->SetDontUseNativeDialog(ui->checkBoxDontUseNativeDialog->isChecked());
    }

    if (m_langChanged)
    {
        const auto locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        VGAnalytics::Instance()->SetGUILanguage(settings->GetLocale());
        m_langChanged = false;

        VAbstractApplication::VApp()->LoadTranslation(locale);
        QCoreApplication::processEvents(); // force to call changeEvent
    }

    /* Maximum number of commands in undo stack may only be set when the undo stack is empty, since setting it on a
     * non-empty stack might delete the command at the current index. Calling setUndoLimit() on a non-empty stack
     * prints a warning and does nothing.*/
    if (settings->GetUndoCount() != ui->undoCount->value())
    {
        preferences.append(tr("undo limit"));
        settings->SetUndoCount(ui->undoCount->value());
    }

    // Scene antialiasing
    if (settings->GetGraphicalOutput() != ui->graphOutputCheck->isChecked())
    {
        preferences.append(tr("antialiasing"));
        settings->SetGraphicalOutput(ui->graphOutputCheck->isChecked());
    }

    if (settings->IsOpenGLRender() != ui->checkBoxOpenGLRender->isChecked())
    {
        preferences.append(tr("scene render"));
        settings->SetOpenGLRender(ui->checkBoxOpenGLRender->isChecked());
    }

    if (settings->IsAutomaticallyCheckUpdates() != ui->checkBoxAutomaticallyCheckUpdates->isChecked())
    {
        settings->SetAutomaticallyCheckUpdates(ui->checkBoxAutomaticallyCheckUpdates->isChecked());
    }

    if (settings->GetSingleStrokeOutlineFont() != ui->checkBoxSingleStrokeOutlineFont->isChecked())
    {
        settings->SetSingleStrokeOutlineFont(ui->checkBoxSingleStrokeOutlineFont->isChecked());
    }
    settings->SetSingleLineFonts(ui->checkBoxSingleLineFonts->isChecked());

    // Tab Shortcuts
    if (VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager())
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
void PuzzlePreferencesConfigurationPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        RetranslateShortcutsTable();
        ui->retranslateUi(this);
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesConfigurationPage::ShortcutCellDoubleClicked(int row, int column)
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
void PuzzlePreferencesConfigurationPage::SetThemeModeComboBox()
{
    ui->comboBoxThemeMode->clear();
    ui->comboBoxThemeMode->addItem(tr("System", "theme"), static_cast<int>(VThemeMode::System));
    ui->comboBoxThemeMode->addItem(tr("Dark", "theme"), static_cast<int>(VThemeMode::Dark));
    ui->comboBoxThemeMode->addItem(tr("Light", "theme"), static_cast<int>(VThemeMode::Light));
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesConfigurationPage::InitShortcuts(bool defaults)
{
    VAbstractShortcutManager *manager = VAbstractApplication::VApp()->GetShortcutManager();
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
void PuzzlePreferencesConfigurationPage::UpdateShortcutsTable() const
{
    for (int i = 0; i < m_transientShortcuts.length(); i++)
    {
        const QStringList &shortcuts = m_transientShortcuts.value(i);
        ui->shortcutsTable->item(i, 1)->setText(VAbstractShortcutManager::StringListToReadableString(shortcuts));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PuzzlePreferencesConfigurationPage::RetranslateShortcutsTable() const
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
