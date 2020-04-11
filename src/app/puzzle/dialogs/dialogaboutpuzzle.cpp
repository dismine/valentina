/************************************************************************
 **
 **  @file   dialogaboutpuzzle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
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

#include "dialogaboutpuzzle.h"
#include "ui_dialogaboutpuzzle.h"
#include "../version.h"
#include "../vmisc/def.h"
#include "../fervor/fvupdater.h"

#include <QDate>
#include <QDesktopServices>
#include <QMessageBox>
#include <QShowEvent>
#include <QUrl>
#include <QtDebug>

//---------------------------------------------------------------------------------------------------------------------
DialogAboutPuzzle::DialogAboutPuzzle(QWidget *parent)
    :QDialog(parent),
      ui(new Ui::DialogAboutPuzzle),
      isInitialized(false)
{
    ui->setupUi(this);

    //mApp->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    RetranslateUi();
    connect(ui->pushButton_Web_Site, &QPushButton::clicked, this, []()
    {
        if ( not QDesktopServices::openUrl(QUrl(VER_COMPANYDOMAIN_STR)))
        {
            qWarning() << tr("Cannot open your default browser");
        }
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogAboutPuzzle::close);
    connect(ui->pushButtonCheckUpdate, &QPushButton::clicked, []()
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
DialogAboutPuzzle::~DialogAboutPuzzle()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutPuzzle::changeEvent(QEvent *event)
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
void DialogAboutPuzzle::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutPuzzle::FontPointSize(QWidget *w, int pointSize)
{
    SCASSERT(w != nullptr)

    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutPuzzle::RetranslateUi()
{
    ui->label_Puzzle_Version->setText(QString("Puzzle %1").arg(APP_VERSION_STR));
    ui->labelBuildRevision->setText(tr("Build revision: %1").arg(BUILD_REVISION));
    ui->label_QT_Version->setText(buildCompatibilityString());

    const QDate date = QLocale::c().toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy"));
    ui->label_Puzzle_Built->setText(tr("Built on %1 at %2").arg(date.toString(), __TIME__));

    ui->label_Legal_Stuff->setText(QApplication::translate("InternalStrings",
                                                           "The program is provided AS IS with NO WARRANTY OF ANY "
                                                           "KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY "
                                                           "AND FITNESS FOR A PARTICULAR PURPOSE."));

    ui->pushButton_Web_Site->setText(tr("Web site : %1").arg(VER_COMPANYDOMAIN_STR));
}
