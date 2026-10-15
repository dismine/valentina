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
#include "vnestorclient.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QJsonObject>
#include <QLocale>

//---------------------------------------------------------------------------------------------------------------------
VNestingAccountPage::VNestingAccountPage(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::VNestingAccountPage>()),
    m_client(new VAccountClient(this))
{
    ui->setupUi(this);
    ui->checkBoxShareFiles->setChecked(VAbstractApplication::VApp()->Settings()->GetShareFiles());
    connect(ui->buttonCopyToken,
            &QPushButton::clicked,
            this,
            [this]()
            {
                QGuiApplication::clipboard()->setText(VAbstractApplication::VApp()->Settings()->GetAccountToken());
                ui->labelError->setText(tr("Account token copied to the clipboard."));
            });
    connect(ui->buttonDeleteFiles,
            &QPushButton::clicked,
            this,
            [this]()
            {
                auto *nestor = new VNestorClient(this);
                ui->buttonDeleteFiles->setEnabled(false);
                ui->labelDeleteFiles->setText(tr("Deleting…"));
                connect(nestor,
                        &VNestorClient::FilesDeleted,
                        this,
                        [this, nestor]()
                        {
                            ui->buttonDeleteFiles->setEnabled(true);
                            ui->labelDeleteFiles->setText(tr("Your shared files were deleted."));
                            nestor->deleteLater();
                        });
                connect(nestor,
                        &VNestorClient::Failed,
                        this,
                        [this, nestor](int, const QString &message)
                        {
                            ui->buttonDeleteFiles->setEnabled(true);
                            ui->labelDeleteFiles->setText(tr("Could not delete: %1").arg(message));
                            nestor->deleteLater();
                        });
                nestor->DeleteMyFiles();
            });

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    ui->lineEditIssuer->setText(settings->GetAccountIssuerUrl());
    ui->lineEditService->setText(settings->GetNestorServiceUrl());
    SetAdvancedVisible(false);
    connect(ui->groupBoxAdvanced, &QGroupBox::toggled, this, &VNestingAccountPage::SetAdvancedVisible);

    connect(ui->buttonSignIn,
            &QPushButton::clicked,
            this,
            [this]()
            {
                Apply(); // the issuer typed in Advanced must be the one we talk to
                m_signingIn = true;
                ui->labelError->clear();
                ShowState();
                m_client->StartDeviceLogin();
            });
    connect(ui->buttonCancel,
            &QPushButton::clicked,
            this,
            [this]()
            {
                m_client->CancelLogin();
                m_signingIn = false;
                ShowState();
            });
    connect(ui->buttonRefresh, &QPushButton::clicked, m_client, &VAccountClient::RefreshUserInfo);
    connect(ui->buttonSignOut, &QPushButton::clicked, m_client, &VAccountClient::Logout);
    connect(ui->buttonTestService,
            &QPushButton::clicked,
            this,
            [this]()
            {
                ui->labelServiceStatus->setText(tr("Checking…"));
                m_client->CheckService(ui->lineEditService->text());
            });

    connect(m_client,
            &VAccountClient::UserCodeReady,
            this,
            [this](const QString &code, const QUrl &url)
            {
                ui->labelCode->setText(tr("Confirm this code in your browser: <b style=\"font-size:x-large\">%1</b><br>"
                                          "If the browser did not open, go to <a href=\"%2\">%2</a>")
                                           .arg(code.toHtmlEscaped(), url.toString().toHtmlEscaped()));
                ui->labelCode->setOpenExternalLinks(true);
                QDesktopServices::openUrl(url);
            });
    connect(m_client,
            &VAccountClient::LoggedIn,
            this,
            [this]()
            {
                m_signingIn = false;
                ShowState();
                m_client->RefreshUserInfo();
            });
    connect(m_client,
            &VAccountClient::LoggedOut,
            this,
            [this]()
            {
                m_signingIn = false;
                ShowState();
            });
    connect(m_client,
            &VAccountClient::UserInfo,
            this,
            [this](const QJsonObject &info)
            {
                const bool premium = info.value(QStringLiteral("premium")).toBool();
                const QDate until = QDate::fromString(info.value(QStringLiteral("premium_until")).toString(),
                                                      Qt::ISODate);
                ui->labelPremium->setText(premium ? (until.isValid()
                                                         ? tr("Premium subscription: yes, until %1")
                                                               .arg(QLocale().toString(until, QLocale::ShortFormat))
                                                         : tr("Premium subscription: yes"))
                                                  : tr("Premium subscription: no"));
            });
    connect(m_client,
            &VAccountClient::ServiceChecked,
            this,
            [this](bool ok, const QString &message)
            { ui->labelServiceStatus->setText((ok ? QStringLiteral("✔ ") : QStringLiteral("✘ ")) + message); });
    connect(m_client,
            &VAccountClient::Failed,
            this,
            [this](const QString &message)
            {
                m_signingIn = false;
                ui->labelError->setText(message);
                ShowState();
            });

    ShowState();
    if (m_client->IsLoggedIn())
    {
        m_client->RefreshUserInfo();
    }
}

//---------------------------------------------------------------------------------------------------------------------
VNestingAccountPage::~VNestingAccountPage() = default;

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::Apply()
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    settings->SetShareFiles(ui->checkBoxShareFiles->isChecked());
    const QString issuer = ui->lineEditIssuer->text().trimmed();
    if (issuer != settings->GetAccountIssuerUrl())
    {
        // a token from another issuer is useless and would only produce 401s
        settings->SetAccountIssuerUrl(issuer);
        m_client->Logout();
    }
    settings->SetNestorServiceUrl(ui->lineEditService->text().trimmed());
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        ShowState();
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::ShowState()
{
    const VAccountToken token = m_client->Token();
    const bool in = m_client->IsLoggedIn();
    if (in)
    {
        ui->labelStatus->setText(tr("Signed in as %1 (%2).").arg(token.Name(), token.Email()));
    }
    else if (token.IsValid())
    {
        ui->labelStatus->setText(tr("Your session has expired. Please sign in again."));
    }
    else
    {
        ui->labelStatus->setText(tr("Connect Puzzle to your smart-pattern.com.ua account to use online nesting."));
    }
    if (not in)
    {
        ui->labelPremium->clear();
    }
    if (not m_signingIn)
    {
        ui->labelCode->clear();
    }
    ui->buttonSignIn->setVisible(not in && not m_signingIn);
    ui->buttonCancel->setVisible(m_signingIn);
    ui->buttonRefresh->setVisible(in);
    ui->buttonSignOut->setVisible(in);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingAccountPage::SetAdvancedVisible(bool visible)
{
    for (auto *w : {static_cast<QWidget *>(ui->labelIssuer),
                    static_cast<QWidget *>(ui->lineEditIssuer),
                    static_cast<QWidget *>(ui->labelService),
                    static_cast<QWidget *>(ui->lineEditService),
                    static_cast<QWidget *>(ui->buttonTestService),
                    static_cast<QWidget *>(ui->labelServiceStatus)})
    {
        w->setVisible(visible);
    }
}
