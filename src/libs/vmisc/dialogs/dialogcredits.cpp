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

const QString DialogCredits::sponsors = QStringLiteral(R"(Анастасія Бровкіна
АннаДенесюк
Людмила_Данилюк
Наталя Думінська
Оксана Чорна
Олена__Карась
Оля Калун
Amirati
Anita Guiso
Bellamaesdesigns
Bortnovska
cecile LALLEMAND
chaulieu
chiarabassetti.studio
Constance Cellier
Crema
Diana Holub
DurielLightbane
Edwardo
Eliane SIMON
Gabrielle Squelin
Gorebuy
Himnish Sujan
Irina Konetzka
Jennifet
Kozakova_Natalie1996
Kyowashi
Marina Orlova
martos
Merli
milamila
Monica Marques Carvalho
nian
Oksana Oksana
Oleksandra
Personalizzandia
Ryan Hawker
Svit1104
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
