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
#include "../vformat/knownmeasurements/vknownmeasurementsdatabase.h"
#include "../vganalytics/vganalytics.h"
#include "../vmisc/dialogs/vshortcutdialog.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractshortcutmanager.h"
#include "../vwidgets/vmousewheelwidgetadjustmentguard.h"
#include "ui_tapepreferencesconfigurationpage.h"

#include <QScrollBar>
#include <QStyleHints>

//---------------------------------------------------------------------------------------------------------------------
TapePreferencesConfigurationPage::TapePreferencesConfigurationPage(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::TapePreferencesConfigurationPage>()),
    m_langChanged(false),
    m_systemChanged(false)
{
    ui->setupUi(this);
    RetranslateUi();

    // Prevent stealing focus when scrolling
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->langCombo);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxKnownMeasurements);
    VMouseWheelWidgetAdjustmentGuard::InstallEventFilter(ui->comboBoxThemeMode);

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
    ui->knownMeasurementsDescription->setFixedHeight(
        4 * QFontMetrics(ui->knownMeasurementsDescription->font()).lineSpacing());
    connect(ui->comboBoxKnownMeasurements, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                m_systemChanged = true;
                InitKnownMeasurementsDescription();
            });

    // set default pattern making system
    InitKnownMeasurements(ui->comboBoxKnownMeasurements);
    index = ui->comboBoxKnownMeasurements->findData(settings->GetKnownMeasurementsId());
    if (index != -1)
    {
        ui->comboBoxKnownMeasurements->setCurrentIndex(index);
    }
    InitKnownMeasurementsDescription();

    //----------------------------- Measurements Editing
    connect(ui->resetWarningsButton, &QPushButton::released, this,
            []() { MApplication::VApp()->TapeSettings()->SetConfirmFormatRewriting(true); });

    ui->checkBoxTranslateFormula->setChecked(settings->IsTranslateFormula());

    //----------------------- Toolbar
    ui->toolBarStyleCheck->setChecked(settings->GetToolBarStyle());

    //----------------------- Update
    ui->checkBoxAutomaticallyCheckUpdates->setChecked(settings->IsAutomaticallyCheckUpdates());

    // Tab Shortcuts
    InitShortcuts();
    connect(ui->pushButtonRestoreDefaults, &QPushButton::clicked, this, [this]() { InitShortcuts(true); });
    connect(ui->shortcutsTable, &QTableWidget::cellDoubleClicked, this,
            &TapePreferencesConfigurationPage::ShortcutCellDoubleClicked);

    // Tab Privacy
    ui->checkBoxSendUsageStatistics->setChecked(settings->IsCollectStatistic());

#if !defined(CRASH_REPORTING)
    ui->groupBoxCrashReports->setDisabled(true);
    ui->checkBoxSendCrashReports->setChecked(false);
    ui->lineEditCrashUserEmail->setText(QString());
#else
    ui->checkBoxSendCrashReports->setChecked(settings->IsSendCrashReport());

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    connect(ui->checkBoxSendCrashReports, &QCheckBox::checkStateChanged, this,
#else
    connect(ui->checkBoxSendCrashReports, &QCheckBox::stateChanged, this,
#endif
            [this]() { m_sendCrashReportsChanged = true; });

    QRegularExpression const rx(QStringLiteral("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b"),
                                QRegularExpression::CaseInsensitiveOption);
    ui->lineEditCrashUserEmail->setValidator(new QRegularExpressionValidator(rx, this));
    ui->lineEditCrashUserEmail->setText(settings->GetCrashEmail());
    connect(ui->lineEditCrashUserEmail, &QLineEdit::editingFinished, this,
            [this]() { m_crashUserEmailChanged = true; });
#endif
}

//---------------------------------------------------------------------------------------------------------------------
TapePreferencesConfigurationPage::~TapePreferencesConfigurationPage() = default;

//---------------------------------------------------------------------------------------------------------------------
auto TapePreferencesConfigurationPage::Apply() -> QStringList
{
    QStringList preferences;
    VTapeSettings *settings = MApplication::VApp()->TapeSettings();
    settings->SetOsSeparator(ui->osOptionCheck->isChecked());

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

    settings->SetTranslateFormula(ui->checkBoxTranslateFormula->isChecked());

    if (settings->IsDontUseNativeDialog() != ui->checkBoxDontUseNativeDialog->isChecked())
    {
        settings->SetDontUseNativeDialog(ui->checkBoxDontUseNativeDialog->isChecked());
    }

    if (m_systemChanged)
    {
        const auto id = ui->comboBoxKnownMeasurements->currentData().toUuid();
        settings->SetKnownMeasurementsId(id);
        m_systemChanged = false;
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

    if (settings->IsAutomaticallyCheckUpdates() != ui->checkBoxAutomaticallyCheckUpdates->isChecked())
    {
        settings->SetAutomaticallyCheckUpdates(ui->checkBoxAutomaticallyCheckUpdates->isChecked());
    }

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
void TapePreferencesConfigurationPage::ShortcutCellDoubleClicked(int row, int column)
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
void TapePreferencesConfigurationPage::RetranslateUi()
{
    ui->osOptionCheck->setText(tr("With OS options") + QStringLiteral(" (%1)").arg(LocaleDecimalPoint(QLocale())));

    {
        const auto code = qvariant_cast<QString>(ui->comboBoxKnownMeasurements->currentData());

        {
            const QSignalBlocker blocker(ui->comboBoxKnownMeasurements);
            ui->comboBoxKnownMeasurements->clear();
            InitKnownMeasurements(ui->comboBoxKnownMeasurements);
            ui->comboBoxKnownMeasurements->setCurrentIndex(-1);
        }

        ui->comboBoxKnownMeasurements->setCurrentIndex(ui->comboBoxKnownMeasurements->findData(code));
    }

    RetranslateShortcutsTable();
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::SetThemeModeComboBox()
{
    ui->comboBoxThemeMode->clear();
    ui->comboBoxThemeMode->addItem(tr("System", "theme"), static_cast<int>(VThemeMode::System));
    ui->comboBoxThemeMode->addItem(tr("Dark", "theme"), static_cast<int>(VThemeMode::Dark));
    ui->comboBoxThemeMode->addItem(tr("Light", "theme"), static_cast<int>(VThemeMode::Light));
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::InitShortcuts(bool defaults)
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
void TapePreferencesConfigurationPage::UpdateShortcutsTable()
{
    for (int i = 0; i < m_transientShortcuts.length(); i++)
    {
        const QStringList &shortcuts = m_transientShortcuts.value(i);
        ui->shortcutsTable->item(i, 1)->setText(VAbstractShortcutManager::StringListToReadableString(shortcuts));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::RetranslateShortcutsTable()
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

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::InitKnownMeasurements(QComboBox *combo)
{
    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    QHash<QUuid, VKnownMeasurementsHeader> const known = db->AllKnownMeasurements();

    const QSignalBlocker blocker(combo);
    combo->clear();

    SCASSERT(combo != nullptr)
    combo->addItem(tr("None"), QUuid());

    int index = 1;
    auto i = known.constBegin();
    while (i != known.constEnd())
    {
        QString name = i.value().name;

        if (i.value().name.isEmpty())
        {
            name = tr("Known measurements %1").arg(index);
            ++index;
        }

        combo->addItem(name, i.key());
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TapePreferencesConfigurationPage::InitKnownMeasurementsDescription()
{
    VKnownMeasurementsDatabase *db = MApplication::VApp()->KnownMeasurementsDatabase();
    QHash<QUuid, VKnownMeasurementsHeader> const known = db->AllKnownMeasurements();

    ui->knownMeasurementsDescription->clear();
    QUuid const id = ui->comboBoxKnownMeasurements->currentData().toUuid();
    if (!id.isNull() && known.contains(id))
    {
        ui->knownMeasurementsDescription->setPlainText(known.value(id).description);
    }
}
