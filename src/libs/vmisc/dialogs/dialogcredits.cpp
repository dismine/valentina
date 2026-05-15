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

const QString DialogCredits::sponsors = QStringLiteral(R"(Альона
Аліна Стельмах-Гордієнко
Анастасія Микитюк
Анна Капошко
АннаДенесюк
Валентина2025
Віка Чебера
Вікторія Гуцало
Горгуля_85
Елена Солнышкина
Зоряна Касьян
кира
Лозова.Таня
Любов Корнійчук
Людмила Абрамова
Людмила_Данилюк
Лія Самосієнко
марго
Марина Прокопенко
Марина Ярёменко
Марія Харишин
Наталя Думінська
Наталія Субботська
Олена
Олена Дунаєвська
Олена Жемчужина
Олена Сівер
Оля Калун
Оля Коваль
Поліна Михайлюта
Поліна Якубовська
Софія
Тамара Крупка
Татьяна Донец
Тетяна Надточій
Юлия Горобец
Ірина
a.dor
abulxoyr Ибн АбдулХаким
Alena German
Alena Minak
Alina Damineva
Alyssa SANTINI
Anastasiia1303
Anastasiia_Mostsepan
anastasiianf
Andre
andrii yarovitsyn
angelinaa22
Anita Guiso
Anna Rashevska
Anna Yefimenko
arven
Aurelija Vindigienė
AЗозуля
BaudinoMarieNoelle@8469
Bodnarchuk
Boris Korotach
Bortnovska
Busybee1977
Cansuakaydin
Carlos Flores
CarlosRomero
Castera Line
cecile LALLEMAND
chaulieu
Constance Cellier
craftsbycatmarg
Daniel Stefan Popescu
DaphneLS
Dariia28
David Hagar
Denarmin
Dinara
Dmytrashko.tanya
DurielLightbane
dyvosvit
Edith Burgos
Eliane SIMON
Elina Kondratenko
ElinLundh
ElodieNguyen
emilialange
Evvgenia
fainakobita
Fartukoff2020
Fiono4ka
Flavie Giraudeau
Florine Vangeneberg
fv
G.VOVK
Gabrielle Squelin
Galina84
GiovannyM
Halyna Zinechko
Hanna Panasiuk
hello
Himnish Sujan
Innusha Tu
Irina Konetzka
ivanna sweet
Ivette11
Jarosław Grzesik
Jennifer Jean Joseph
Jennifet
JessicaGoodyearDesign
jonatan
jules delarue
Juls0912
kasatkina.o
Kateryna_Myasoyedova
Klaudiusz Piotr Kusz
Kristina_Kuvshyn
Kseniya Pashkova
Kyowashi
LailaS
LEONARDOVITH
Liubov
Ludmila@Yevchik
Mafalda
Maika
manu iena
mdrivero
meryrose
Monica Marques Carvalho
msandovaldesigns
Muriel Rogger
Nadiia Puchko
Nicolas
Nina Staufert
Olena Schramka
OlenaErakova
Olesia
Olga Guzyuk
olga39
osteppy
Patternporter
Piotr Jasiek
ProgneSports
qsrosales
Ruben Bakker
Ryan Hawker
Sacha-Hugo Lagaude
saifudinka
scratchthedog
shlomomintz
Slavovna
Slazeret
SuzanA
svetlana valvi
Svitlana Cherhava
tammapatterns
tanjashka
Tara Neplenbroek
Tatiana Onyshchak
Tetiana Dubyna
Tetiana Olkhovska
Tetiana Shevchuk
tetiana17
Tetyana Mazur
The Sewing Site AI
thebadjarvis
tim-jacobs
ukranastasio
ulabaka
Valeriya89
Vanessa Focardi
Veronika M
vikakarr
vio31
Vivrafashion
Volodymyr Chernega
wlkORety
Yuliya
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
