/************************************************************************
 **
 **  @file   dialogselectlanguage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#ifndef DIALOGSELECTLANGUAGE_H
#define DIALOGSELECTLANGUAGE_H

#include <QDialog>

namespace Ui
{
class DialogSelectLanguage;
}

class DialogSelectLanguage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSelectLanguage(QWidget *parent = nullptr);
    ~DialogSelectLanguage();

    QString Locale() const;

private:
    Q_DISABLE_COPY(DialogSelectLanguage)
    Ui::DialogSelectLanguage *ui;
};

#endif // DIALOGSELECTLANGUAGE_H