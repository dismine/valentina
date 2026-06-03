/************************************************************************
 **
 **  @file   dialogcredits.cpp
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
#include "dialogcredits.h"
#include "ui_dialogcredits.h"

const QString DialogCredits::sponsors = QStringLiteral(R"(АннаДенесюк
Людмила_Данилюк
Марина Ярёменко
Наталя Думінська
Олена Жемчужина
Олена__Карась
Оля Калун
Тетяна Надточій
anastasiianf
Anita Guiso
Anna Rashevska
Anna Yefimenko
Bortnovska
cecile LALLEMAND
chaulieu
Constance Cellier
Diana Holub
DurielLightbane
Edwardo
Eliane SIMON
Gabrielle Squelin
Himnish Sujan
Irina Konetzka
Jennifet
Kateryna_Myasoyedova
Kozakova_Natalie1996
Kyowashi
mdrivero
Monica Marques Carvalho
Oleksandra
Personalizzandia
Piotr Jasiek
Ryan Hawker
shlomomintz
SuzanA
Svit1104
tammapatterns
Tanya_Pom08
Tara Neplenbroek
thebadjarvis
tim-jacobs
vikakarr
Volodymyr Chernega
yesalesya
zosia81
)");

//---------------------------------------------------------------------------------------------------------------------
DialogCredits::DialogCredits(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DialogCredits)
{
    ui->setupUi(this);
    ui->plainTextEditSponsors->setPlainText(sponsors);
    ui->plainTextEditAuthors->setPlainText(QStringLiteral("Roman Telezhinskyi"));
}

//---------------------------------------------------------------------------------------------------------------------
DialogCredits::~DialogCredits()
{
    delete ui;
}
