/************************************************************************
 **
 **  @file   vnestingaccountpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
 **
 **  @brief  
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making 
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project 
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

#include "vnestingaccountpage.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_vnestingaccountpage.h"
#include "vaccountclient.h"
#include "vnestinghttpclient.h"

#include <QFileDialog>

//---------------------------------------------------------------------------------------------------------------------
VNestingAccountPage::VNestingAccountPage(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::VNestingAccountPage>()),
    m_client(new VAccountClient(this))
{
    ui->setupUi(this);
    ui->checkBoxShareFiles->setChecked(VAbstractApplication::VApp()->Settings()->GetShareFiles());
    connect(ui->buttonDeleteFiles,
            &QPushButton::clicked,
            this,
            [this]()
            {
                auto *http = new VNestingHttpClient(this);
                ui->buttonDeleteFiles->setEnabled(false);
                ui->labelDeleteFiles->setText(tr("Deleting…"));
                connect(http,
                        &VNestingHttpClient::FilesDeleted,
                        this,
                        [this, http]()
                        {
                            ui->buttonDeleteFiles->setEnabled(true);
                            ui->labelDeleteFiles->setText(tr("Your shared files were deleted."));
                            http->deleteLater();
                        });
                connect(http,
                        &VNestingHttpClient::Failed,
                        this,
                        [this, http](int, const QString &message)
                        {
                            ui->buttonDeleteFiles->setEnabled(true);
                            ui->labelDeleteFiles->setText(tr("Could not delete: %1").arg(message));
                            http->deleteLater();
                        });
                http->DeleteMyFiles();
            });

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    ui->lineEditHttpUrl->setText(settings->GetNestingServiceUrl());
    ui->lineEditExecutable->setText(settings->GetNestingExecutable());
    ui->lineEditToken->setText(settings->GetAccountToken());
    ui->comboBoxEngine->setCurrentIndex(settings->IsNestingLocalEngine() ? 1 : 0);
    connect(ui->comboBoxEngine,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            [this](int) { ShowEngineFields(); });
    connect(ui->buttonBrowseExecutable, &QPushButton::clicked, this, [this]()
            {
                const QString path =
                    QFileDialog::getOpenFileName(this, tr("Nesting program"), ui->lineEditExecutable->text());
                if (not path.isEmpty())
                {
                    ui->lineEditExecutable->setText(path);
                }
            });

    connect(ui->buttonTestService,
            &QPushButton::clicked,
            this,
            [this]()
            {
                ui->labelServiceStatus->setText(tr("Checking…"));
                ShowEngineFields();
                m_client->CheckService(ui->lineEditHttpUrl->text());
            });

    connect(m_client,
            &VAccountClient::ServiceChecked,
            this,
            [this](bool ok, const QString &message)
            {
                ui->labelServiceStatus->setText((ok ? QStringLiteral("✔ ") : QStringLiteral("✘ ")) + message);
                ShowEngineFields();
            });
    ShowEngineFields();
}

//---------------------------------------------------------------------------------------------------------------------
VNestingAccountPage::~VNestingAccountPage() = default;

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::Apply()
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    settings->SetShareFiles(ui->checkBoxShareFiles->isChecked());
    settings->SetNestingServiceUrl(ui->lineEditHttpUrl->text().trimmed());
    settings->SetNestingLocalEngine(ui->comboBoxEngine->currentIndex() == 1);
    settings->SetNestingExecutable(ui->lineEditExecutable->text().trimmed());
    settings->SetAccountToken(ui->lineEditToken->text().trimmed());
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        ShowEngineFields();
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::ShowEngineFields()
{
    const bool remote = ui->comboBoxEngine->currentIndex() == 0;
    ui->labelHttpUrl->setVisible(remote);
    ui->widgetHttpUrl->setVisible(remote);
    ui->labelUrlHint->setVisible(remote);
    ui->labelServiceStatus->setVisible(remote && not ui->labelServiceStatus->text().isEmpty());
    ui->labelExecutable->setVisible(not remote);
    ui->widgetExecutable->setVisible(not remote);
    ui->groupBoxAccount->setVisible(remote);
    ui->groupBoxBeta->setVisible(remote);
}
