/***************************************************************************************************
 **
 **  Copyright (c) 2012 Linas Valiukas and others.
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#include "fvupdatewindow.h"

#include <QApplication>
#include <QLabel>
#include <QPointer>
#include <QPushButton>

#include "fvavailableupdate.h"
#include "fvupdater.h"
#include "ui_fvupdatewindow.h"

//---------------------------------------------------------------------------------------------------------------------
FvUpdateWindow::FvUpdateWindow(QWidget *parent)
  : QDialog(parent),
    m_ui(new Ui::FvUpdateWindow)
{
    m_ui->setupUi(this);

    // Delete on close
    setAttribute(Qt::WA_DeleteOnClose, true);

    // Set the "new version is available" string
    const QString newVersString =
        m_ui->newVersionIsAvailableLabel->text().arg(QGuiApplication::applicationDisplayName());
    m_ui->newVersionIsAvailableLabel->setText(newVersString);

    // Connect buttons
    connect(m_ui->installUpdateButton, &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::InstallUpdate);
    connect(m_ui->skipThisVersionButton, &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::SkipUpdate);
    connect(m_ui->remindMeLaterButton, &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::RemindMeLater);
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdateWindow::~FvUpdateWindow()
{
    delete m_ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto FvUpdateWindow::UpdateWindowWithCurrentProposedUpdate() -> bool
{
    QPointer<FvAvailableUpdate> const proposedUpdate = FvUpdater::sharedUpdater()->GetProposedUpdate();
    if (proposedUpdate.isNull())
    {
        return false;
    }

    QString downloadString;
    if (FvUpdater::IsTestBuild())
    {
        downloadString = tr("New %1 test version is now available. Would you like to download it now?")
                             .arg(QGuiApplication::applicationDisplayName());
    }
    else
    {
        downloadString = m_ui->wouldYouLikeToDownloadLabel->text().arg(QGuiApplication::applicationDisplayName(),
                                                                       proposedUpdate->GetEnclosureVersion(),
                                                                       QCoreApplication::applicationVersion());
    }
    m_ui->wouldYouLikeToDownloadLabel->setText(downloadString);

    return true;
}
