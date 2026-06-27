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

const QString DialogCredits::sponsors = QStringLiteral(R"(Jarosław Grzesik
Irina Konetzka
Gabrielle Squelin
Svitlana Cherhava
Aurelija Vindigienė
Зоряна Касьян
Наталія Субботська
Любов Корнійчук
Nicolas
Тамара Крупка
Castera Line
Тетяна Надточій
manu iena
kasatkina.o
кира
Аліна Стельмах-Гордієнко
Анастасія Микитюк
Лія Самосієнко
Edith Burgos
Hanna Panasiuk
Ludmila
Олена Дунаєвська
Поліна Якубовська
osteppy
Валентина2025
Альона
dyvosvit
Наталя Думінська
Поліна Михайлюта
Oleksandra
марго
Марина Прокопенко
Оля Коваль
tetiana17
Boris Korotach
Ruben Bakker
zubeikomaria
Alyssa SANTINI
Dariia28
Flavie Giraudeau
Fiono4ka
meryrose
Alena Minak
BaudinoMarieNoelle
Constance Cellier
zosia81
tammapatterns
DaphneLS
Tetiana Olkhovska
arven
Вікторія Гуцало
Jennifer Jean Joseph
Bodnarchuk
DurielLightbane
Марія Харишин
Kseniya Pashkova
Ірина
msandovaldesigns
Tetiana Shevchuk
svetlana valvi
Nina Staufert
Софія
angelinaa22
LailaS
ivanna sweet
Daniel Stefan Popescu
Alina Damineva
Carlos Flores
Ryan Hawker
Eliane SIMON
Bortnovska
Liubov
Tetiana Dubyna
Olena Schramka
Dmytrashko.tanya
Denarmin
CarlosRomero
ukranastasio
vio31
Jennifet
ElinLundh
Yuliya
Tanya_Pom08
Victoria
Марія Сцібель
Patternporter
GentleOne
Anna Yefimenko
Edwardo
Inesa
YaroslavaDolinska
Женя Канівець
yesalesya
Anita Guiso
Олена__Карась
Олеся Бурчак
Kozakova_Natalie1996
Vitaliia
MashaKid
iamksena92
Vanessa Focardi
Fartukoff2020
Juls0912
Dinara
AЗозуля
Nadiia Puchko
G.VOVK
Elina Kondratenko
Лозова.Таня
Galina84
David Hagar
jules delarue
Klaudiusz Piotr Kusz
hello
Busybee1977
scratchthedog
Kristina_Kuvshyn
saifudinka
Sacha-Hugo Lagaude
andrii yarovitsyn
Muriel Rogger
Maika
Andre
Volodymyr Chernega
Елена Солнышкина
Anna Rashevska
Cansuakaydin
Mafalda
Slavovna
cecile LALLEMAND
Anastasiia1303
wlkORety
Tetyana Mazur
Evvgenia
Anastasiia_Mostsepan
LEONARDOVITH
ElodieNguyen
ulabaka
The Sewing Site AI
emilialange
Veronika M
craftsbycatmarg
chaulieu
Valeriya89
Олена
Людмила_Данилюк
fv
a.dor
Ivette11
Горгуля_85
jonatan
Slazeret
GiovannyM
OlenaErakova
abulxoyr Ибн АбдулХаким
JessicaGoodyearDesign
thebadjarvis
Alena German
ProgneSports
qsrosales
Людмила Абрамова
Kyowashi
mdrivero
anastasiianf
Piotr Jasiek
SuzanA
Kateryna_Myasoyedova
Марина Ярёменко
shlomomintz
tim-jacobs
Tara Neplenbroek
АннаДенесюк
Monica Marques Carvalho
Himnish Sujan
Оля Калун
Svit1104
Diana Holub
Personalizzandia
Crema
milamila
Marina Orlova
Oksana Oksana
nian
Gorebuy
chiarabassetti.studio
Amirati
Оксана Чорна
martos
Анастасія Бровкіна
Bellamaesdesigns
Merli
Aasokolov13
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
