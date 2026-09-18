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

const QString DialogCredits::sponsors = QStringLiteral(R"(angela
Aasokolov13
Amirati
Ana Cristina Fonseca
Anastasia Banditka
Anita Guiso
AnnaNikolaevna11
Anne-Marlene Rüede
Assïa
Bellamaesdesigns
Bodnarchuk
Bortnovska
BryVos2
Constance Cellier
Crema
DurielLightbane
Edwardo
Eliane SIMON
Elsa-Sophie
Florence Meunier
Gabriel Albert
Gabrielle Squelin
Gorebuy
Ira
Irina Kobulnik
Irina Konetzka
JaneCrudace
Kateryna7
Kozakova_Natalie1996
Kyowashi
Lucy de Almeida Beck
Mariia BARTOSH
Marina Orlova
Merli
Mylène Labatut
Naomi1504
NeliaDeineka
Oksana Faltere
Oksana Oksana
Oleksandra
Olha Fesenko
Personalizzandia
Ryan Hawker
Simeoni Marie Laure
Svit1104
Tammy Sparks
Tanja Seibold
Tatiana0108
Teti
Vitaliia
Volodymyr Chernega
YanaIJFH
Yulia210283
alina.sereda
beasilva
brian
cecile LALLEMAND
chaulieu
chiarabassetti.studio
gnairda
joiger
lizochka90
lyubov.free
martos
matrjxz
milamila
nadiko
naommipaula
nian
otershak
thebadjarvis
valentynakrivoryka2301
vikakarr
yesalesya
zosia81
Іванна Нагорна
Александра
Алена Тріфонова
Анастасія Бровкіна
АннаДенесюк
Владимир Phantom
Вікторія Яріш
Дар'я
Елена Налбандян
Людмила Абрамова
Людмила_Данилюк
Маша Серикова
Наталия Уварова
Наталья Мельник
Оксана Чорна
Олена
Оля Калун
таня стратейчук
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
