/************************************************************************
 **
 **  @file   dialogpuzzlepreferences.cpp
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

#include "dialogpuzzlepreferences.h"
#include "../vpapplication.h"
#include "configpages/puzzlepreferencesconfigurationpage.h"
#include "configpages/puzzlepreferenceslayoutpage.h"
#include "configpages/puzzlepreferencespathpage.h"
#include "ui_dialogpuzzlepreferences.h"

#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
DialogPuzzlePreferences::DialogPuzzlePreferences(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogPuzzlePreferences),
    m_configurationPage(new PuzzlePreferencesConfigurationPage),
    m_layoutPage(new PuzzlePreferencesLayoutPage),
    m_pathPage(new PuzzlePreferencesPathPage)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    connect(bOk, &QPushButton::clicked, this, &DialogPuzzlePreferences::Ok);

    QPushButton *bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(bApply != nullptr)
    connect(bApply, &QPushButton::clicked, this, &DialogPuzzlePreferences::Apply);

    ui->pagesWidget->insertWidget(0, m_configurationPage);
    ui->pagesWidget->insertWidget(1, m_layoutPage);
    ui->pagesWidget->insertWidget(2, m_pathPage);

    connect(ui->contentsWidget, &QListWidget::currentItemChanged, this, &DialogPuzzlePreferences::PageChanged);
    ui->pagesWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPuzzlePreferences::~DialogPuzzlePreferences()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPuzzlePreferences::showEvent(QShowEvent *event)
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
void DialogPuzzlePreferences::resizeEvent(QResizeEvent *event)
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
void DialogPuzzlePreferences::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    if (event->type() == QEvent::PaletteChange)
    {
        QStyle *style = QApplication::style();

        QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
        SCASSERT(bOk != nullptr)
        bOk->setIcon(style->standardIcon(QStyle::SP_DialogOkButton));

        QPushButton *bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
        SCASSERT(bApply != nullptr)
        bApply->setIcon(style->standardIcon(QStyle::SP_DialogApplyButton));

        QPushButton *bCancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
        SCASSERT(bCancel != nullptr)
        bCancel->setIcon(style->standardIcon(QStyle::SP_DialogCancelButton));
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPuzzlePreferences::Apply()
{
    QStringList preferences;

    preferences += m_configurationPage->Apply();
    preferences += m_layoutPage->Apply();
    m_pathPage->Apply();

    if (not preferences.isEmpty())
    {
        const QString text =
            tr("Followed %n option(s) require restart to take effect: %1.", "", static_cast<int>(preferences.size()))
                .arg(preferences.join(", "_L1));
        QMessageBox::information(this, QCoreApplication::applicationName(), text);
    }

    VPApplication::VApp()->PuzzleSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    emit UpdateProperties();
    setResult(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPuzzlePreferences::Ok()
{
    Apply();
    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPuzzlePreferences::PageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    int rowIndex = ui->contentsWidget->row(current);
    ui->pagesWidget->setCurrentIndex(rowIndex);
}
