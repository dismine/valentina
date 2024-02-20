/************************************************************************
 **
 **  @file   vpdialogabout.h
 **  @author Ronan Le Tiec
 **  @date   11 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VPDIALOGABOUT_H
#define VPDIALOGABOUT_H

#include <QDialog>
#include <memory>

namespace Ui
{
class VPDialogAbout;
}

class VPDialogAbout : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit VPDialogAbout(QWidget *parent = nullptr);
    ~VPDialogAbout() override;

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPDialogAbout) // NOLINT
    std::unique_ptr<Ui::VPDialogAbout> ui;
    bool m_isInitialized;

    static void FontPointSize(QWidget *w, int pointSize);

    void RetranslateUi();
};

#endif // VPDIALOGABOUT_H
