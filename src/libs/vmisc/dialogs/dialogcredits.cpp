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

const QString DialogCredits::sponsors = QStringLiteral(R"(Аліна Стельмах-Гордієнко
Зоряна Касьян
Ірина
кира
марго
Марина Прокопенко
Марія Харишин
Поліна Якубовська
Софія
Alyssa SANTINI
angelinaa22
Bodnarchuk
Boris Korotach
Castera Line
Constance Cellier
Dariia28
DurielLightbane
dyvosvit
Edith Burgos
Flavie Giraudeau
Gabrielle Squelin
Irina Konetzka
Jennifer Jean Joseph
kasatkina.o
msandovaldesigns
Nicolas
Nina Staufert
Ruben Bakker
svetlana valvi
Tetiana Shevchuk
tetiana17
zubeikomaria
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
