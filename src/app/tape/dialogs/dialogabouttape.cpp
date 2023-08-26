/************************************************************************
 **
 **  @file   dialogabouttape.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 7, 2015
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

#include "dialogabouttape.h"
#include "../fervor/fvupdater.h"
#include "../vmisc/def.h"
#include "../vmisc/projectversion.h"
#include "ui_dialogabouttape.h"

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

//---------------------------------------------------------------------------------------------------------------------
DialogAboutTape::DialogAboutTape(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogAboutTape),
    m_isInitialized(false)
{
    ui->setupUi(this);

    // mApp->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    RetranslateUi();
    connect(ui->pushButton_Web_Site, &QPushButton::clicked, this,
            []()
            {
                if (not QDesktopServices::openUrl(
                        QUrl(QStringLiteral("https://%1").arg(QStringLiteral(VER_COMPANYDOMAIN_STR)))))
                {
                    qWarning() << tr("Cannot open your default browser");
                }
            });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogAboutTape::close);
    connect(ui->pushButtonCheckUpdate, &QPushButton::clicked,
            []()
            {
                // Set feed URL before doing anything else
                FvUpdater::sharedUpdater()->SetFeedURL(FvUpdater::CurrentFeedURL());
                FvUpdater::sharedUpdater()->CheckForUpdatesNotSilent();
            });

    // By default on Windows font point size 8 points we need 11 like on Linux.
    FontPointSize(ui->label_Legal_Stuff, 11);
    FontPointSize(ui->label_Tape_Built, 11);
    FontPointSize(ui->label_QT_Version, 11);
}

//---------------------------------------------------------------------------------------------------------------------
DialogAboutTape::~DialogAboutTape()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutTape::changeEvent(QEvent *event)
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
void DialogAboutTape::showEvent(QShowEvent *event)
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
void DialogAboutTape::FontPointSize(QWidget *w, int pointSize)
{
    SCASSERT(w != nullptr)

    QFont font = w->font();
    font.setPointSize(qMax(pointSize, 1));
    w->setFont(font);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutTape::RetranslateUi()
{
    ui->label_Tape_Version->setText(QStringLiteral("Tape %1").arg(AppVersionStr()));
    ui->labelBuildRevision->setText(tr("Build revision: %1").arg(QStringLiteral(BUILD_REVISION)));
    ui->label_QT_Version->setText(buildCompatibilityString());

    const QDate date = QLocale::c().toDate(QStringLiteral(__DATE__).simplified(), QStringLiteral("MMM d yyyy"));
    ui->label_Tape_Built->setText(tr("Built on %1 at %2").arg(date.toString(), QStringLiteral(__TIME__)));

    ui->label_Legal_Stuff->setText(QApplication::translate("InternalStrings",
                                                           "The program is provided AS IS with NO WARRANTY OF ANY "
                                                           "KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY "
                                                           "AND FITNESS FOR A PARTICULAR PURPOSE."));

    ui->pushButton_Web_Site->setText(tr("Web site : %1").arg(QStringLiteral(VER_COMPANYDOMAIN_STR)));
}
