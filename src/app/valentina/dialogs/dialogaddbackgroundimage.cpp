/************************************************************************
 **
 **  @file   dialogaddbackgroundimage.cpp
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
#include "dialogaddbackgroundimage.h"
#include "qfileinfo.h"
#include "ui_dialogaddbackgroundimage.h"

//---------------------------------------------------------------------------------------------------------------------
DialogAddBackgroundImage::DialogAddBackgroundImage(const QString &fileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddBackgroundImage)
{
    ui->setupUi(this);
    ui->lineEditName->setText(QFileInfo(fileName).baseName());
}

//---------------------------------------------------------------------------------------------------------------------
DialogAddBackgroundImage::~DialogAddBackgroundImage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogAddBackgroundImage::Name() const -> QString
{
    return ui->lineEditName->text();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogAddBackgroundImage::BuiltIn() const -> bool
{
    return ui->checkBoxBuiltIn->isChecked();
}
