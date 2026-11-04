/************************************************************************
 **
 **  @file   vnestingprogressdialog.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 10, 2026
 **
 **  @brief  Online nesting progress: a gauge per stop condition and the group being nested.
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

#include "vnestingprogressdialog.h"
#include "ui_vnestingprogressdialog.h"
#include "vnestinghttpclient.h"

#include <QLocale>
#include <QProgressBar>
#include <QPushButton>
#include <QStyle>
#include <QStyleFactory>
#include <QtMath>

//---------------------------------------------------------------------------------------------------------------------
VNestingProgressDialog::VNestingProgressDialog(const QJsonObject &request, bool unlimited, QWidget *parent)
  : QDialog(parent),
    ui(std::make_unique<Ui::VNestingProgressDialog>())
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
#if defined(Q_OS_MAC)
    // the native macOS style (Qt 6.9 on macOS 26+) paints no progress bar at all: same fix as MainWindow::ToolBarStyle
    QStyle *fusion = QStyleFactory::create(QStringLiteral("fusion"));
    fusion->setParent(this);
    for (QProgressBar *bar : {ui->progressBarTokens, ui->progressBarTarget, ui->progressBarGroups})
    {
        bar->setStyle(fusion);
    }
#endif

    const int budget = qRound(request.value(QLatin1String("timeBudgetS")).toDouble());
    ui->labelTokens->setText(tr("Time:"));
    ui->labelTokens->setVisible(budget > 0);
    ui->progressBarTokens->setVisible(budget > 0);
    ui->progressBarTokens->setRange(0, qMax(budget, 1));
    ui->progressBarTokens->setValue(0);
    ui->progressBarTokens->setFormat(tr("%v of %m s"));

    ui->labelTarget->setVisible(false);
    ui->progressBarTarget->setVisible(false);
    ui->labelGroups->setVisible(false);
    ui->progressBarGroups->setVisible(false);

    ui->labelConditions->setText(ConditionsText(request));
    ui->labelStatus->setText(tr("Sending the nesting job…"));

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(unlimited ? tr("Stop and keep the layout") : tr("Cancel"));
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &VNestingProgressDialog::canceled);
}

//---------------------------------------------------------------------------------------------------------------------
VNestingProgressDialog::~VNestingProgressDialog() = default;

//---------------------------------------------------------------------------------------------------------------------
void VNestingProgressDialog::Update(const QString &status, int position, const QJsonObject &progress)
{
    ui->labelStatus->setText(VNestingHttpClient::ProgressText(status, position, progress));
    if (progress.contains(QLatin1String("utilisation")))
    {
        ui->gaugeUtilisation->SetValue(qRound(progress.value(QLatin1String("utilisation")).toDouble() * 100));
    }
    if (progress.contains(QLatin1String("elapsed_s")))
    {
        ui->progressBarTokens->setValue(
            qMin(qRound(progress.value(QLatin1String("elapsed_s")).toDouble()), ui->progressBarTokens->maximum()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingProgressDialog::SetStopping()
{
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestingProgressDialog::ConditionsText(const QJsonObject &request) -> QString
{
    const QLocale locale;
    const qreal seconds = request.value(QLatin1String("timeBudgetS")).toDouble();
    if (seconds > 0)
    {
        return tr("Stops when %1 seconds have elapsed.").arg(locale.toString(seconds, 'f', 0));
    }
    return tr("The job has no time budget.");
}
