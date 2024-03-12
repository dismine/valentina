/************************************************************************
 **
 **  @file   dialogaskcollectstatistic.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "dialogaskcollectstatistic.h"
#include "ui_dialogaskcollectstatistic.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "../vabstractapplication.h"

//---------------------------------------------------------------------------------------------------------------------
DialogAskCollectStatistic::DialogAskCollectStatistic(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogAskCollectStatistic)
{
    ui->setupUi(this);

#if !defined(CRASH_REPORTING)
    ui->groupBoxCrashReports->setDisabled(true);
#endif

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    QRegularExpression const rx(QStringLiteral("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b"),
                                QRegularExpression::CaseInsensitiveOption);
    ui->lineEditCrashUserEmail->setValidator(new QRegularExpressionValidator(rx, this));
    ui->lineEditCrashUserEmail->setText(settings->GetCrashEmail());
}

//---------------------------------------------------------------------------------------------------------------------
DialogAskCollectStatistic::~DialogAskCollectStatistic()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogAskCollectStatistic::CollectStatistic() const -> bool
{
    return ui->checkBoxSendUsageStatistics->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogAskCollectStatistic::SendCrashReport() const -> bool
{
    return ui->checkBoxSendCrashReports->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogAskCollectStatistic::UserEmail() const -> QString
{
    return ui->lineEditCrashUserEmail->text();
}
