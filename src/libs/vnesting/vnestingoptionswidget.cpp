/************************************************************************
 **
 **  @file   vnestingoptionswidget.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 October, 2026
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
#include "vnestingoptionswidget.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_vnestingoptionswidget.h"
#include "vaccounttoken.h"
#include "vnestorclient.h"

#include <QEvent>
#include <QLocale>
#include <QRegularExpression>
#include <QtMath>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VNestingOptionsWidget::VNestingOptionsWidget(QWidget *parent)
  : QWidget(parent),
    ui(std::make_unique<Ui::VNestingOptionsWidget>()),
    m_client(new VNestorClient(this))
{
    ui->setupUi(this);
    FillSpeeds();
    connect(ui->comboBoxSpeed, &QComboBox::currentIndexChanged, this, &VNestingOptionsWidget::UpdateQuote);
    connect(ui->comboBoxSpeed, &QComboBox::currentIndexChanged, this, &VNestingOptionsWidget::UpdatePreviewButton);
    connect(ui->checkBoxUnlimited, &QCheckBox::toggled, this, &VNestingOptionsWidget::UpdateQuote);
    connect(ui->checkBoxUnlimited, &QCheckBox::toggled, ui->spinBoxBudget, &QSpinBox::setDisabled);
    connect(ui->spinBoxBudget, &QSpinBox::valueChanged, this, &VNestingOptionsWidget::UpdateQuote);
    connect(ui->buttonPreview, &QPushButton::clicked, this, &VNestingOptionsWidget::StartPreview);
    connect(ui->lineEditRolls, &QLineEdit::textChanged, this, &VNestingOptionsWidget::ShowRolls);
    connect(m_client,
            &VNestorClient::Info,
            this,
            [this](const QJsonObject &info)
            {
                m_previewTimeS = info.value("preview_time_s"_L1).toInt(m_previewTimeS);
                ui->spinBoxBudget->setMaximum(info.value("max_budget_tokens"_L1).toInt(ui->spinBoxBudget->maximum()));
                const QJsonObject speeds = info.value("speeds"_L1).toObject();
                int i = 0;
                for (const auto speed : {NestingSpeed::Normal, NestingSpeed::Fast, NestingSpeed::Ultra})
                {
                    const QJsonObject sp = speeds.value(NestingSpeedString(speed)).toObject();
                    m_workers[i] = sp.value("workers"_L1).toInt(m_workers.at(i));
                    m_evalsPerS[i] = sp.value("evals_per_s"_L1).toDouble(m_evalsPerS.at(i));
                    m_evalsPerToken[i] = sp.value("evals_per_token"_L1).toInt(m_evalsPerToken.at(i));
                    ++i;
                }
                FillSpeeds();
                UpdateQuote();
                UpdatePreviewButton();
            });
    connect(m_client,
            &VNestorClient::Failed,
            this,
            [this](int status, const QString &message)
            {
                ui->buttonPreview->setEnabled(true);
                ui->labelPreview->setText(status == 429 ? tr("Preview refused: %1").arg(message)
                                                        : tr("Preview failed: %1").arg(message));
            });
    UpdateQuote();
    UpdatePreviewButton();
    ShowRolls();
    m_client->FetchInfo();
}

//---------------------------------------------------------------------------------------------------------------------
VNestingOptionsWidget::~VNestingOptionsWidget() = default;

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::SetRequestBuilder(std::function<QJsonObject()> builder)
{
    m_builder = std::move(builder);
    UpdatePreviewButton();
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestingOptionsWidget::Options() const -> VNestingOptions
{
    VNestingOptions options;
    options.budgetTokens = ui->checkBoxUnlimited->isChecked() ? 0 : ui->spinBoxBudget->value(); // 0: unlimited
    options.speed = Speed();
    options.targetLengthMm = ui->doubleSpinBoxTarget->value() * 1000; // typed in metres
    options.untilConverged = true; // the service always stops when it finds no further improvement
    options.rollsMm = ParseRollLengthsMm(ui->lineEditRolls->text(), QLocale());
    options.rollJunctionMm = ui->doubleSpinBoxJunction->value();
    return options;
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestingOptionsWidget::Speed() const -> NestingSpeed
{
    return static_cast<NestingSpeed>(ui->comboBoxSpeed->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::SetSpeed(NestingSpeed speed)
{
    ui->comboBoxSpeed->setCurrentIndex(qMax(ui->comboBoxSpeed->findData(static_cast<int>(speed)), 0));
}

//---------------------------------------------------------------------------------------------------------------------
auto VNestingOptionsWidget::ParseRollLengthsMm(const QString &text, const QLocale &locale) -> QVector<qreal>
{
    QVector<qreal> rolls;
    // a comma followed by a space or the end is a list separator in every locale; one between digits is the
    // decimal mark where the locale says so, a separator otherwise. Group separators ("1,000") are refused.
    QLocale strict = locale;
    strict.setNumberOptions(QLocale::RejectGroupSeparator);
    QString cleaned = text;
    cleaned.replace(QRegularExpression(QStringLiteral(",(?=\\s|$)")), QStringLiteral(" "));
    if (locale.decimalPoint() != QLatin1String(","))
    {
        cleaned.replace(',', ' ');
    }
    const QStringList tokens = cleaned.split(QRegularExpression(QStringLiteral("[;\\s]+")), Qt::SkipEmptyParts);
    for (const QString &token : tokens)
    {
        bool ok = false;
        qreal metres = strict.toDouble(token, &ok);
        if (not ok && not token.contains(','))
        {
            metres = token.toDouble(&ok); // "32.5" typed by a French user
        }
        if (ok && metres > 0)
        {
            rolls.append(metres * 1000);
        }
    }
    return rolls;
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::ShowRolls()
{
    // what was understood, so that a misread comma is seen before the job is paid for
    const QVector<qreal> rolls = ParseRollLengthsMm(ui->lineEditRolls->text(), QLocale());
    if (rolls.isEmpty())
    {
        ui->labelRollsParsed->setText(ui->lineEditRolls->text().trimmed().isEmpty() ? tr("One endless roll.")
                                                                                    : tr("No readable roll length."));
        return;
    }
    const QLocale locale;
    QStringList metres;
    for (const qreal mm : rolls)
    {
        metres << locale.toString(mm / 1000, 'f', 2);
    }
    ui->labelRollsParsed->setText(tr("%n roll(s): %1 m, then the last one again.", "", static_cast<int>(rolls.size()))
                                      .arg(metres.join(QStringLiteral(", "))));
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        FillSpeeds();
        UpdateQuote();
        UpdatePreviewButton();
        ShowRolls();
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::FillSpeeds()
{
    const NestingSpeed current = ui->comboBoxSpeed->count() > 0 ? Speed() : NestingSpeed::Normal;
    ui->comboBoxSpeed->blockSignals(true);
    ui->comboBoxSpeed->clear();
    const QString names[] = {tr("Normal"), tr("Fast"), tr("Ultra")};
    int i = 0;
    for (const auto speed : {NestingSpeed::Normal, NestingSpeed::Fast, NestingSpeed::Ultra})
    {
        ui->comboBoxSpeed->addItem(tr("%1 (%n thread(s))", "", m_workers.at(i)).arg(names[i]), static_cast<int>(speed));
        ++i;
    }
    ui->comboBoxSpeed->blockSignals(false);
    SetSpeed(current);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::UpdateQuote()
{
    if (ui->checkBoxUnlimited->isChecked())
    {
        ui->labelQuote->setText(tr("Billed on the work done, until you stop it."));
        return;
    }
    // "up to": a stop at the target length or on convergence is billed on the work actually done
    ui->labelQuote->setText(tr("Up to %n token(s).", "", ui->spinBoxBudget->value()));
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::UpdatePreviewButton()
{
    const VAccountToken token(VAbstractApplication::VApp()->Settings()->GetAccountToken());
    const auto speed = static_cast<int>(Speed());
    const int tokens = qCeil(m_previewTimeS * m_evalsPerS.at(speed) / m_evalsPerToken.at(speed));
    ui->buttonPreview->setText(tr("Preview (%n token(s))", "", tokens));
    ui->buttonPreview->setEnabled(m_builder != nullptr && not token.IsExpired() && token.IsPremium());
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::StartPreview()
{
    if (m_builder == nullptr)
    {
        return;
    }
    QJsonObject request = m_builder();
    if (request.isEmpty())
    {
        ui->labelPreview->setText(tr("There is no piece to place."));
        return;
    }
    request.insert(QStringLiteral("preview"), true);
    request.insert(QStringLiteral("time_s"), m_previewTimeS);
    ui->buttonPreview->setEnabled(false);
    ui->labelPreview->setText(tr("Preview in progress…"));
    disconnect(m_client, &VNestorClient::Finished, this, nullptr);
    connect(m_client,
            &VNestorClient::Finished,
            this,
            [this, request](const QJsonObject &result)
            {
                ui->buttonPreview->setEnabled(true);
                ShowPreview(request, result);
            });
    m_client->Submit(request);
}

//---------------------------------------------------------------------------------------------------------------------
void VNestingOptionsWidget::ShowPreview(const QJsonObject &request, const QJsonObject &result)
{
    const QLocale locale;
    const qreal width = request.value("fabric_width_mm"_L1).toDouble();
    const qreal area = NestingRequestArea(request);
    const qreal length = result.value("marker_length_mm"_L1).toDouble();
    const qreal utilisation = result.value("utilisation"_L1).toDouble();
    if (width <= 0 || length <= 0)
    {
        ui->labelPreview->setText(tr("The preview returned no layout."));
        return;
    }
    QStringList lines{tr("Quick placement: %1 m at %2 % utilisation.")
                          .arg(locale.toString(length / 1000, 'f', 2), locale.toString(utilisation * 100, 'f', 1)),
                      tr("%n token(s) spent.", "", result.value("tokens"_L1).toInt())};
    for (const int percent : {70, 75, 80, 85})
    {
        const qreal target = area / (width * percent / 100.0);
        if (target < length)
        {
            lines << tr("At %1 % utilisation: %2 m, saving %3 m.")
                         .arg(percent)
                         .arg(locale.toString(target / 1000, 'f', 2), locale.toString((length - target) / 1000, 'f', 2));
        }
    }
    ui->labelPreview->setText(lines.join('\n'));
}
