/************************************************************************
 **
 **  @file   vshortcutdialog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2023
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
#ifndef VSHORTCUTDIALOG_H
#define VSHORTCUTDIALOG_H

#include <QDialog>

#include "../vabstractshortcutmanager.h"

namespace Ui
{
class VShortcutDialog;
}

class QAbstractButton;

class VShortcutDialog : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit VShortcutDialog(int index, QWidget *parent = nullptr);
    ~VShortcutDialog() override;

signals:
    void ShortcutsListChanged(int index, QStringList shortcutsStringList);

private slots:
    void ButtonBoxClicked(QAbstractButton *button);

private:
    Q_DISABLE_COPY_MOVE(VShortcutDialog) // NOLINT

    Ui::VShortcutDialog *ui;
    VAbstractShortcutManager::VSShortcut m_shortcutObject{};
    int m_index;

    void AcceptValidated();
    void done(int r) override;
};

#endif // VSHORTCUTDIALOG_H
