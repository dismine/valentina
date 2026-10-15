/************************************************************************
 **
 **  @file   vnestingprogressdialog.h
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

#ifndef VNESTINGPROGRESSDIALOG_H
#define VNESTINGPROGRESSDIALOG_H

#include <memory>
#include <QDialog>
#include <QJsonObject>

namespace Ui
{
class VNestingProgressDialog;
}

/**
 * @brief The "nesting in progress" window: the service's status line, a utilisation gauge, one gauge per stop
 * condition the request asked for (token budget, target length) and, when nesting group by group, which group is
 * being placed. `canceled` fires on the button, like QProgressDialog.
 */
class VNestingProgressDialog : public QDialog
{
    Q_OBJECT // NOLINT

public:
    VNestingProgressDialog(const QJsonObject &request, bool unlimited, QWidget *parent = nullptr);
    ~VNestingProgressDialog() override;

    /** @brief Update What VNestorClient::Progress carries. */
    void Update(const QString &status, int position, const QJsonObject &progress);
    /** @brief SetStopping After a first "stop and keep" press: the button now cancels for good. */
    void SetStopping();

    /** @brief ConditionsText One line saying when the job ends, from the request's options. */
    static auto ConditionsText(const QJsonObject &request) -> QString;

signals:
    void canceled();

private:
    Q_DISABLE_COPY_MOVE(VNestingProgressDialog) // NOLINT
    std::unique_ptr<Ui::VNestingProgressDialog> ui;
    qreal m_targetMm{0};
};

#endif // VNESTINGPROGRESSDIALOG_H
