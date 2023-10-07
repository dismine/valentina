/************************************************************************
 **
 **  @file   vpdialogabout.cpp
 **  @author Ronan Le Tiec
 **  @date   11 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vpdialogabout.h"
#include "../fervor/fvupdater.h"
#include "../vmisc/def.h"
#include "../vmisc/projectversion.h"
#include "ui_vpdialogabout.h"

#include <QDate>
#include <QDesktopServices>
#include <QMessageBox>
#include <QShowEvent>
#include <QUrl>
#include <QtDebug>

#if !defined(BUILD_REVISION) && defined(QBS_BUILD)
#include <vcsRepoState.h>
#define BUILD_REVISION VCS_REPO_STATE_REVISION
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VPDialogAbout::VPDialogAbout(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::VPDialogAbout),
    m_isInitialized(false)
{
    ui->setupUi(this);

    // mApp->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    RetranslateUi();
    connect(ui->pushButton_Web_Site, &QPushButton::clicked, this,
            []()
            {
                if (not QDesktopServices::openUrl(QUrl(QStringLiteral(VER_COMPANYDOMAIN_STR))))
                {
                    qWarning() << tr("Cannot open your default browser");
                }
            });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &VPDialogAbout::close);
    connect(ui->pushButtonCheckUpdate, &QPushButton::clicked,
            []()
            {
                // Set feed URL before doing anything else
                FvUpdater::sharedUpdater()->SetFeedURL(FvUpdater::CurrentFeedURL());
                FvUpdater::sharedUpdater()->CheckForUpdatesNotSilent();
            });

    // By default on Windows font point size 8 points we need 11 like on Linux.
    FontPointSize(ui->label_Legal_Stuff, 11);
    FontPointSize(ui->label_Puzzle_Built, 11);
    FontPointSize(ui->label_QT_Version, 11);
}

//---------------------------------------------------------------------------------------------------------------------
VPDialogAbout::~VPDialogAbout()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void VPDialogAbout::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        RetranslateUi();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPDialogAbout::showEvent(QShowEvent *event)
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

    setMaximumSize(size());
    setMinimumSize(size());

    m_isInitialized = true; // first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void VPDialogAbout::FontPointSize(QWidget *w, int pointSize)
{
    SCASSERT(w != nullptr)

    QFont font = w->font();
    font.setPointSize(qMax(pointSize, 1));
    w->setFont(font);
}

//---------------------------------------------------------------------------------------------------------------------
void VPDialogAbout::RetranslateUi()
{
    ui->label_Puzzle_Version->setText(QStringLiteral("Puzzle %1").arg(AppVersionStr()));
    ui->labelBuildRevision->setText(tr("Build revision: %1").arg(QStringLiteral(BUILD_REVISION)));
    ui->label_QT_Version->setText(buildCompatibilityString());

    const QDate date = QLocale::c().toDate(QStringLiteral(__DATE__).simplified(), "MMM d yyyy"_L1);
    ui->label_Puzzle_Built->setText(tr("Built on %1 at %2").arg(date.toString(), QStringLiteral(__TIME__)));

    ui->label_Legal_Stuff->setText(QApplication::translate("InternalStrings",
                                                           "The program is provided AS IS with NO WARRANTY OF ANY "
                                                           "KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY "
                                                           "AND FITNESS FOR A PARTICULAR PURPOSE."));

    ui->pushButton_Web_Site->setText(tr("Web site : %1").arg(QStringLiteral(VER_COMPANYDOMAIN_STR)));
}
