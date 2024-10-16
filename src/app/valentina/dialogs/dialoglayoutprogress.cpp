/************************************************************************
 **
 **  @file   dialoglayoutprogress.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "dialoglayoutprogress.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vvalentinasettings.h"
#include "ui_dialoglayoutprogress.h"

#include <QMessageBox>
#include <QMovie>
#include <QPushButton>
#include <QShowEvent>
#include <QTime>
#include <QtDebug>
#include <chrono>

using namespace std::chrono_literals;

//---------------------------------------------------------------------------------------------------------------------
DialogLayoutProgress::DialogLayoutProgress(QElapsedTimer timer, qint64 timeout, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogLayoutProgress),
    m_timer(timer),
    m_timeout(timeout),
    m_progressTimer(new QTimer(this))
{
    ui->setupUi(this);

    VAbstractValApplication::VApp()->ValentinaSettings()->GetOsSeparator() ? setLocale(QLocale())
                                                                           : setLocale(QLocale::c());

    ui->progressBar->setMaximum(static_cast<int>(timeout / 1000));
    ui->progressBar->setValue(0);

    const QString scheme =
        (VTheme::ColorSheme() == VColorSheme::Light ? QStringLiteral("light") : QStringLiteral("dark"));
    m_movie = new QMovie(QStringLiteral("://icon/%1/16x16/progress.gif").arg(scheme));
    ui->labelProgress->setMovie(m_movie);
    m_movie->start();

    QPushButton *bCancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
    SCASSERT(bCancel != nullptr)
    connect(bCancel, &QPushButton::clicked, this, [this]() { emit Abort(); });
    setModal(true);

    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    connect(m_progressTimer, &QTimer::timeout, this,
            [this]()
            {
                const qint64 elapsed = m_timer.elapsed();
                const auto timeout = static_cast<int>(m_timeout - elapsed);
                QTime t(0, 0);
                t = t.addMSecs(timeout);
                ui->labelTimeLeft->setText(tr("Time left: %1").arg(t.toString()));
                ui->progressBar->setValue(static_cast<int>(elapsed / 1000));

                if (timeout <= 1000)
                {
                    emit Timeout();
                    m_progressTimer->stop();
                }
            });
    m_progressTimer->start(1s);
}

//---------------------------------------------------------------------------------------------------------------------
DialogLayoutProgress::~DialogLayoutProgress()
{
    delete m_movie;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutProgress::Start()
{
    show();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutProgress::Finished()
{
    m_progressTimer->stop();
    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutProgress::Efficiency(qreal value)
{
    ui->labelMessage->setText(tr("Efficiency coefficient: %1%").arg(qRound(value * 10.) / 10.));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLayoutProgress::showEvent(QShowEvent *event)
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
