/************************************************************************
 **
 **  @file   dialogaddbackgroundimage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef DIALOGADDBACKGROUNDIMAGE_H
#define DIALOGADDBACKGROUNDIMAGE_H

#include <QDialog>

namespace Ui
{
class DialogAddBackgroundImage;
}

class DialogAddBackgroundImage : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogAddBackgroundImage(const QString &fileName, QWidget *parent = nullptr);
    ~DialogAddBackgroundImage() override;

    auto Name() const -> QString;
    auto BuiltIn() const -> bool;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogAddBackgroundImage) // NOLINT
    Ui::DialogAddBackgroundImage *ui;
};

#endif // DIALOGADDBACKGROUNDIMAGE_H
