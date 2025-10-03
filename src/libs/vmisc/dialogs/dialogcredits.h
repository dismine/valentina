/************************************************************************
 **
 **  @file   dialogcredits.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef DIALOGCREDITS_H
#define DIALOGCREDITS_H

#include <QDialog>

namespace Ui
{
class DialogCredits;
}

class DialogCredits : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogCredits(QWidget *parent = nullptr);
    ~DialogCredits() override;

    static const QString sponsors;

private:
    Q_DISABLE_COPY_MOVE(DialogCredits)
    Ui::DialogCredits *ui;
};

#endif // DIALOGCREDITS_H
