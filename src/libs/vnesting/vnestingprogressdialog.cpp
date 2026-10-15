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
#include "vnestorclient.h"

#include <QLocale>
#include <QProgressBar>
#include <QPushButton>
#include <QStyleFactory>
#include <QtMath>

//---------------------------------------------------------------------------------------------------------------------
VNestingProgressDialog::VNestingProgressDialog(const QJsonObject &request, bool unlimited, QWidget *parent)
  : QDialog(parent),
    ui(std::make_unique<Ui::VNestingProgressDialog>()),
    m_targetMm(request.value(QLatin1String("target_length_mm")).toDouble())
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

    const int budget = request.value(QLatin1String("budget_tokens")).toInt();
    ui->labelTokens->setVisible(budget > 0);
    ui->progressBarTokens->setVisible(budget > 0);
    ui->progressBarTokens->setRange(0, qMax(budget, 1));
    ui->progressBarTokens->setValue(0);
    ui->progressBarTokens->setFormat(tr("%v of %m tokens"));

    ui->labelTarget->setVisible(m_targetMm > 0);
    ui->progressBarTarget->setVisible(m_targetMm > 0);
    ui->progressBarTarget->setValue(0);

    const bool groups = request.value(QLatin1String("group_by_group")).toBool();
    ui->labelGroups->setVisible(groups);
    ui->progressBarGroups->setVisible(groups);
    ui->progressBarGroups->setRange(0, 1);
    ui->progressBarGroups->setValue(0);
    ui->progressBarGroups->setFormat(tr("Starting…"));

    ui->labelConditions->setText(ConditionsText(request));
    ui->labelStatus->setText(tr("Sending pieces to the nesting service…"));

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(unlimited ? tr("Stop and keep the layout") : tr("Cancel"));
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &VNestingProgressDialog::canceled);
}

//---------------------------------------------------------------------------------------------------------------------
VNestingProgressDialog::~VNestingProgressDialog() = default;

//---------------------------------------------------------------------------------------------------------------------
void VNestingProgressDialog::Update(const QString &status, int position, const QJsonObject &progress)
{
    ui->labelStatus->setText(VNestorClient::ProgressText(status, position, progress));
    if (progress.contains(QLatin1String("utilisation")))
    {
        ui->gaugeUtilisation->SetValue(qRound(progress.value(QLatin1String("utilisation")).toDouble() * 100));
    }
    if (progress.contains(QLatin1String("tokens")))
    {
        ui->progressBarTokens->setValue(
            qMin(progress.value(QLatin1String("tokens")).toInt(), ui->progressBarTokens->maximum()));
    }
    const qreal lengthMm = progress.value(QLatin1String("marker_length_mm")).toDouble();
    if (m_targetMm > 0 && lengthMm > 0)
    {
        // full when the marker is down to the target
        const QLocale locale;
        ui->progressBarTarget->setValue(qMin(100, qRound(m_targetMm / lengthMm * 100)));
        ui->progressBarTarget->setFormat(
            tr("%1 m, target %2 m")
                .arg(locale.toString(lengthMm / 1000, 'f', 2), locale.toString(m_targetMm / 1000, 'f', 2)));
    }
    if (progress.contains(QLatin1String("groups")))
    {
        const int total = progress.value(QLatin1String("groups")).toInt();
        const int current = progress.value(QLatin1String("group")).toInt();
        ui->progressBarGroups->setRange(0, qMax(total, 1));
        ui->progressBarGroups->setValue(current);
        ui->progressBarGroups->setFormat(tr("Nesting group %1 of %2: %3")
                                             .arg(current)
                                             .arg(total)
                                             .arg(progress.value(QLatin1String("group_name")).toString()));
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
    QStringList ends;
    if (const int budget = request.value(QLatin1String("budget_tokens")).toInt(); budget > 0)
    {
        ends << tr("the budget of %n token(s) is spent", "", budget);
    }
    if (const qreal target = request.value(QLatin1String("target_length_mm")).toDouble(); target > 0)
    {
        ends << tr("the marker is no longer than %1 m").arg(locale.toString(target / 1000, 'f', 2));
    }
    if (request.value(QLatin1String("until_converged")).toBool())
    {
        ends << tr("the search no longer improves");
    }
    if (const int seconds = request.value(QLatin1String("time_s")).toInt(); seconds > 0)
    {
        ends << tr("%n minute(s) have elapsed", "", qCeil(seconds / 60.0));
    }
    if (ends.isEmpty())
    {
        return tr("Runs until you stop it.");
    }
    return tr("Stops when %1, or when you stop it.").arg(ends.join(tr(", or when ")));
}
