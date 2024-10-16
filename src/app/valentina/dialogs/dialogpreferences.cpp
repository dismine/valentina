/************************************************************************
 **
 **  @file   dialogpreferences.cpp
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

#include "dialogpreferences.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vvalentinasettings.h"
#include "../vtools/dialogs/dialogtoolbox.h"
#include "configpages/preferencesconfigurationpage.h"
#include "configpages/preferencespathpage.h"
#include "configpages/preferencespatternpage.h"
#include "ui_dialogpreferences.h"

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>

//---------------------------------------------------------------------------------------------------------------------
DialogPreferences::DialogPreferences(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogPreferences),
    m_configurePage(new PreferencesConfigurationPage),
    m_patternPage(new PreferencesPatternPage),
    m_pathPage(new PreferencesPathPage)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    connect(bOk, &QPushButton::clicked, this, &DialogPreferences::Ok);

    QPushButton *bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(bApply != nullptr)
    connect(bApply, &QPushButton::clicked, this, &DialogPreferences::Apply);

    ui->pagesWidget->insertWidget(0, m_configurePage);
    ui->pagesWidget->insertWidget(1, m_patternPage);
    ui->pagesWidget->insertWidget(2, m_pathPage);

    connect(ui->contentsWidget, &QListWidget::currentItemChanged, this, &DialogPreferences::PageChanged);
    ui->pagesWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPreferences::~DialogPreferences()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::showEvent(QShowEvent *event)
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

    if (QSize const sz = VAbstractApplication::VApp()->Settings()->GetPreferenceDialogSize(); not sz.isEmpty())
    {
        resize(sz);
    }

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::resizeEvent(QResizeEvent *event)
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
void DialogPreferences::changeEvent(QEvent *event)
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
void DialogPreferences::PageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    int const rowIndex = ui->contentsWidget->row(current);
    ui->pagesWidget->setCurrentIndex(rowIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::Apply()
{
    QStringList preferences;

    preferences += m_configurePage->Apply();
    preferences += m_patternPage->Apply();
    preferences += m_pathPage->Apply();

    if (not preferences.isEmpty())
    {
        const QString text =
            tr("Followed %n option(s) require restart to take effect: %1.", "", static_cast<int>(preferences.size()))
                .arg(preferences.join(QStringLiteral(", ")));
        QMessageBox::information(this, QCoreApplication::applicationName(), text);
    }

    m_patternPage->InitDefaultSeamAllowance();

    VAbstractValApplication::VApp()->ValentinaSettings()->GetOsSeparator() ? setLocale(QLocale())
                                                                           : setLocale(QLocale::c());
    emit UpdateProperties();
    setResult(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::Ok()
{
    Apply();
    done(QDialog::Accepted);
}
