/************************************************************************
 **
 **  @file   tst_vdetail.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 1, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
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

#include "tst_vpiece.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/vabstractapplication.h"

#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VPiece::TST_VPiece(QObject *parent)
    :AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::ClearLoop()
{
    // Input data taken from real case
    // See file <root>/src/app/share/collection/jacketМ6_30-110.val
    // Check correct seam allowance
    const Unit unit = Unit::Mm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    qApp->setPatternUnit(unit);

    data->UpdateGObject(304, new VPointF(61.866708661417327, 446.92270866141735, "Ф1", 5.0000125984251973,
                                         9.9999874015748045));
    data->UpdateGObject(307, new VPointF(642.96276692900597, 581.21895343695326, "С1", 88.99993700787401,
                                         50.000125984251973));

    data->UpdateGObject(56, new VPointF(802.08718110236236, 850.6707401574804, "Г6", 20.733316535433072,
                                        18.132850393700789));
    data->UpdateGObject(57, new VPointF(690.47666217505162, 804.29700711628709, "З", -11.505637795275591,
                                        31.221543307086616));
    data->UpdateGObject(203, new VPointF(642.96276692900597, 581.21895343695326, "С1", 88.99993700787401,
                                         50.000125984251973));

    QVector<VFSplinePoint> points;

    {
        const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(203);
        VFSplinePoint p(*point.data(), 0.79455646129695412, 449.62747641208136, 1.6867283804609809, 269.62747641208136);
        points.append(p);
    }

    {
        const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(57);
        VFSplinePoint p(*point.data(), 0.4456850846354396, 120.24000000000034, 1.0255399999999999, 300.24000000000035);
        points.append(p);
    }

    {
        const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(56);
        VFSplinePoint p(*point.data(), 1.0085299999999999, 184.58891, 1, 4.5889100000000003);
        points.append(p);
    }

    {
        VSplinePath *path = new VSplinePath(points);
        path->SetApproximationScale(10);
        data->UpdateGObject(308, path);
    }

    data->UpdateGObject(309, new VPointF(799.45989815267649, 850.6707401574804, "Г8", -30.431206299212597,
                                         29.487155905511813));
    data->UpdateGObject(310, new VPointF(802.08718110236236, 1653.9337322834645, "Н5", 5.0000125984251973,
                                         9.9999874015748045));

    VPiece detail;
    detail.SetSeamAllowance(true);
    detail.SetSAWidth(7);
    detail.GetPath().Append(VPieceNode(304, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(307, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(308, Tool::NodeSplinePath));
    detail.GetPath().Append(VPieceNode(309, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(310, Tool::NodePoint));
    // Closed
    detail.GetPath()[0].SetFormulaSABefore(QChar('0'));
    detail.GetPath()[detail.GetPath().CountNodes()-1].SetFormulaSAAfter(QChar('0'));

    const QVector<QPointF> pointsEkv = detail.SeamAllowancePoints(data.data());

    QVector<QPointF> origPoints;
    origPoints.append(QPointF(42.46405659601942, 415.2845470563873));
    origPoints.append(QPointF(669.53545906065, 560.2060851721218));
    origPoints.append(QPointF(669.4001315082384, 584.8051149911265));
    origPoints.append(QPointF(669.3971506600346, 591.5358644500567));
    origPoints.append(QPointF(669.4350122627271, 598.0891335031142));
    origPoints.append(QPointF(669.512757618795, 604.4805090719773));
    origPoints.append(QPointF(669.6294287899757, 610.7123442956477));
    origPoints.append(QPointF(669.7840625660031, 616.7869939551731));
    origPoints.append(QPointF(669.9756903141416, 622.7068151490457));
    origPoints.append(QPointF(670.2033378543063, 628.4741680595723));
    origPoints.append(QPointF(670.4660253660076, 634.0914168000103));
    origPoints.append(QPointF(670.7627673353721, 639.5609303670992));
    origPoints.append(QPointF(671.0925725508636, 644.88508369749));
    origPoints.append(QPointF(671.4544441569424, 650.0662588248562));
    origPoints.append(QPointF(671.8473797781523, 655.1068461674331));
    origPoints.append(QPointF(672.2703717242233, 660.0092459230049));
    origPoints.append(QPointF(672.7224072914591, 664.7758696009033));
    origPoints.append(QPointF(673.2024691737167, 669.4091416739334));
    origPoints.append(QPointF(673.7095360004373, 673.9115013689292));
    origPoints.append(QPointF(674.2425830172843, 678.2854045794492));
    origPoints.append(QPointF(674.8005829289502, 682.5333259132697));
    origPoints.append(QPointF(675.3825069209878, 686.6577608536486));
    origPoints.append(QPointF(675.9873258816623, 690.6612280423046));
    origPoints.append(QPointF(676.6140118402975, 694.5462716540186));
    origPoints.append(QPointF(677.2615396418696, 698.3154638589637));
    origPoints.append(QPointF(677.9288888734632, 701.971407344139));
    origPoints.append(QPointF(678.615046057566, 705.5167378711579));
    origPoints.append(QPointF(679.3190071217258, 708.9541268298552));
    origPoints.append(QPointF(680.0397801538563, 712.2862837635739));
    origPoints.append(QPointF(680.7763884409013, 715.5159588051306));
    origPoints.append(QPointF(681.527873788316, 718.6459449906465));
    origPoints.append(QPointF(682.2933001036281, 721.6790803846292));
    origPoints.append(QPointF(683.0717572225838, 724.6182499679669));
    origPoints.append(QPointF(683.8623649413778, 727.4663872197892));
    origPoints.append(QPointF(684.6642772103369, 730.2264753386498));
    origPoints.append(QPointF(685.4688320979097, 732.8756855713231));
    origPoints.append(QPointF(686.7042297790314, 736.7448337001608));
    origPoints.append(QPointF(688.3922766601711, 741.6485656582661));
    origPoints.append(QPointF(690.1014239938168, 746.2438997099595));
    origPoints.append(QPointF(691.8365445991736, 750.5829691271946));
    origPoints.append(QPointF(693.5947375993189, 754.6929542771874));
    origPoints.append(QPointF(695.374143963642, 758.6015018634648));
    origPoints.append(QPointF(697.173895570668, 762.3365598413925));
    origPoints.append(QPointF(698.9939856776302, 765.9262007023783));
    origPoints.append(QPointF(700.8350559377793, 769.3984479446959));
    origPoints.append(QPointF(702.6981018570923, 772.781118895178));
    origPoints.append(QPointF(704.5751154394404, 776.0858639250417));
    origPoints.append(QPointF(707.471075434013, 781.0529292076153));
    origPoints.append(QPointF(711.3732953060202, 787.6314568397516));
    origPoints.append(QPointF(713.2933193388338, 790.9045911620659));
    origPoints.append(QPointF(713.7531687634975, 791.6875604681287));
    origPoints.append(QPointF(714.6678160659043, 793.1880160336813));
    origPoints.append(QPointF(715.5686285678618, 794.6014031881614));
    origPoints.append(QPointF(716.4776602288559, 795.9656869340208));
    origPoints.append(QPointF(717.3946261026655, 797.2821365366428));
    origPoints.append(QPointF(718.3193027545042, 798.5520602886423));
    origPoints.append(QPointF(719.2515331337012, 799.7768007322334));
    origPoints.append(QPointF(720.1912309193016, 800.9577290368121));
    origPoints.append(QPointF(721.1383841967548, 802.0962385692094));
    origPoints.append(QPointF(722.0930583323086, 803.1937377265572));
    origPoints.append(QPointF(723.0553979184381, 804.2516421248051));
    origPoints.append(QPointF(724.0256276824568, 805.2713662661918));
    origPoints.append(QPointF(725.0040522692885, 806.2543148301429));
    origPoints.append(QPointF(725.9910548373568, 807.2018737544394));
    origPoints.append(QPointF(726.9870944370721, 808.1154012887525));
    origPoints.append(QPointF(727.992702176212, 808.9962192129983));
    origPoints.append(QPointF(729.0084762114801, 809.8456044148575));
    origPoints.append(QPointF(730.0350756421151, 810.6647810168413));
    origPoints.append(QPointF(731.0732134148514, 811.4549132304943));
    origPoints.append(QPointF(732.1236483836559, 812.217099098663));
    origPoints.append(QPointF(733.1871766854633, 812.9523652550562));
    origPoints.append(QPointF(734.2646226265674, 813.6616628101706));
    origPoints.append(QPointF(735.3568292693044, 814.345864425637));
    origPoints.append(QPointF(736.4646489305214, 815.0057626152854));
    origPoints.append(QPointF(737.5889337869056, 815.6420692657007));
    origPoints.append(QPointF(738.7305267800202, 816.2554163399695));
    origPoints.append(QPointF(739.890252994274, 816.8463576948574));
    origPoints.append(QPointF(741.0689116577936, 817.4153719140883));
    origPoints.append(QPointF(742.2672688927632, 817.9628660401196));
    origPoints.append(QPointF(743.4860513115749, 818.4891800699529));
    origPoints.append(QPointF(744.7259405221598, 818.994592069369));
    origPoints.append(QPointF(745.9470287398851, 819.4637478954073));
    origPoints.append(QPointF(747.8833122349788, 820.1566508068018));
    origPoints.append(QPointF(750.5850392329681, 821.0169732509104));
    origPoints.append(QPointF(753.3408618817797, 821.7832053775238));
    origPoints.append(QPointF(756.1945063236712, 822.4686248864464));
    origPoints.append(QPointF(759.1474180444699, 823.0731242050008));
    origPoints.append(QPointF(762.2002204282861, 823.5965365148435));
    origPoints.append(QPointF(765.3528023393483, 824.0387539700189));
    origPoints.append(QPointF(768.6044162317405, 824.399818281789));
    origPoints.append(QPointF(771.9537790264915, 824.679985612587));
    origPoints.append(QPointF(775.3991699554742, 824.8797689928807));
    origPoints.append(QPointF(778.9385214242475, 824.999962059787));
    origPoints.append(QPointF(782.5695005442702, 825.041647992741));
    origPoints.append(QPointF(786.289580235633, 825.0061972612904));
    origPoints.append(QPointF(790.0960997334876, 824.8952573520692));
    origPoints.append(QPointF(793.9863149544959, 824.7107371151485));
    origPoints.append(QPointF(826.2455325542998, 822.6315469221747));
    origPoints.append(QPointF(828.6637938138197, 1697.2698270114065));
    origPoints.append(QPointF(42.46405659601942, 415.2845470563873));

    // Begin comparison
    Comparison(pointsEkv, origPoints);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::Issue620()
{
    // See file <root>/src/app/share/collection/bugs/Issue_#620.vit
    // Check main path
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    qApp->setPatternUnit(unit);

    data->UpdateGObject(1, new VPointF(30, 39.999874015748034, "A", 5.0000125984251973, 9.9999874015748045));
    data->UpdateGObject(2, new VPointF(333.80102715408322, 37.242158125518621, "A1", 5.0000125984251973,
                                       9.9999874015748045));
    data->UpdateGObject(3, new VPointF(345.43524385831239, 572.57275904711241, "A2", 5.0000125984251973,
                                       9.9999874015748045));
    VPointF *p4 = new VPointF(-43.770684129917051, 567.84465074396087, "A3", 5.0000125984251973,
                              9.9999874015748045);
    data->UpdateGObject(4, p4);

    VPointF *p5 = new VPointF(101.73836126698214, 289.83563666815587, "A4", 5.0000125984251973, 9.9999874015748045);
    data->UpdateGObject(5, p5);
    data->UpdateGObject(6, new VPointF(34.070501467722302, 568.79027240459118, "A5", 5.0000125984251973,
                                       9.9999874015748045));

    QVector<VSplinePoint> points;

    {
        const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(6);
        VSplinePoint p(*point.data(), 239.37700000000001, "239.377", 419.37700000000001, "59.3765",
                       0, QChar('0'), 109.55943307086613, "2.89876");
        points.append(p);
    }

    {
        const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(5);
        VSplinePoint p(*point.data(), 273.97199999999998, "273.972", 453.97199999999998, "93.9724",
                       88.161637795275595, "2.33261", 56.135055118110238, "1.48524");
        points.append(p);
    }

    {
        const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(1);
        VSplinePoint p(*point.data(), 337.32600000000002, "337.326", 157.32599999999999, "157.326",
                       71.189669291338589, "1.88356", 50.093858267716534, "1.3254");
        points.append(p);
    }

    {
        VSplinePath *path = new VSplinePath(points);
        path->SetApproximationScale(10);
        data->UpdateGObject(7, path);
    }

    {
        VSpline *spl = new VSpline(*p4, *p5, 59.932499999999997, "59.9325", 257.56999999999999,
                                   "257.57", 170.46425196850396, "4.5102", 150.6164409448819, "3.98506");
        spl->SetApproximationScale(10);
        data->UpdateGObject(8, spl);
    }

    VPiece detail;
    detail.SetSeamAllowance(false);
    detail.SetSAWidth(7);
    detail.GetPath().Append(VPieceNode(1, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(2, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(3, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(4, Tool::NodePoint));
    detail.GetPath().Append(VPieceNode(8, Tool::NodeSpline));
    detail.GetPath().Append(VPieceNode(7, Tool::NodeSplinePath));

    const QVector<QPointF> pointsEkv = detail.MainPathPoints(data.data());

    QVector<QPointF> origPoints;
    origPoints.append(QPointF(30.0, 39.999874015748034));
    origPoints.append(QPointF(333.8010271540832, 37.24215812551862));
    origPoints.append(QPointF(345.4352438583124, 572.5727590471124));
    origPoints.append(QPointF(-43.77068412991705, 567.8446507439609));
    origPoints.append(QPointF(-41.776027573529674, 564.4070486713748));
    origPoints.append(QPointF(-37.85646185008852, 557.7284453106807));
    origPoints.append(QPointF(-34.019306379498914, 551.2790100611998));
    origPoints.append(QPointF(-30.263131590262248, 545.0512397783091));
    origPoints.append(QPointF(-26.586507910879927, 539.0376313173863));
    origPoints.append(QPointF(-22.98800576985332, 533.2306815338081));
    origPoints.append(QPointF(-19.466195595683825, 527.6228872829522));
    origPoints.append(QPointF(-16.019647816872833, 522.2067454201957));
    origPoints.append(QPointF(-10.978854893973825, 514.4038559090675));
    origPoints.append(QPointF(-4.520178850519514, 504.6314487766784));
    origPoints.append(QPointF(1.6631798984757897, 495.490594593478));
    origPoints.append(QPointF(7.58265792500097, 486.92126820248467));
    origPoints.append(QPointF(13.249691801044914, 478.8634444467164));
    origPoints.append(QPointF(18.67571809859651, 471.25709816919147));
    origPoints.append(QPointF(26.41443785504145, 460.525117170489));
    origPoints.append(QPointF(33.62211724018606, 450.5466726362586));
    origPoints.append(QPointF(38.19847894365711, 444.14598470188895));
    origPoints.append(QPointF(42.591015928580234, 437.89664846085356));
    origPoints.append(QPointF(46.811164766944344, 431.73863875617076));
    origPoints.append(QPointF(50.87036203073829, 425.6119304308587));
    origPoints.append(QPointF(53.81161102143449, 420.9990273759227));
    origPoints.append(QPointF(55.73121775864344, 417.9028756631243));
    origPoints.append(QPointF(57.61701967395359, 414.7807851443662));
    origPoints.append(QPointF(59.47044633886358, 411.6252526750258));
    origPoints.append(QPointF(61.292927324872, 408.42877511048044));
    origPoints.append(QPointF(63.085892203477464, 405.18384930610733));
    origPoints.append(QPointF(64.85077054617858, 401.88297211728377));
    origPoints.append(QPointF(66.58899192447396, 398.5186403993869));
    origPoints.append(QPointF(68.30198590986222, 395.0833510077942));
    origPoints.append(QPointF(69.99118207384197, 391.5696007978828));
    origPoints.append(QPointF(71.65800998791181, 387.96988662503003));
    origPoints.append(QPointF(73.30389922357037, 384.2767053446131));
    origPoints.append(QPointF(74.93027935231623, 380.48255381200937));
    origPoints.append(QPointF(76.53857994564802, 376.57992888259605));
    origPoints.append(QPointF(78.13023057506439, 372.56132741175054));
    origPoints.append(QPointF(79.70666081206386, 368.4192462548499));
    origPoints.append(QPointF(81.26930022814513, 364.14618226727157));
    origPoints.append(QPointF(82.81957839480675, 359.7346323043928));
    origPoints.append(QPointF(84.35892488354736, 355.1770932215909));
    origPoints.append(QPointF(85.88876926586558, 350.466061874243));
    origPoints.append(QPointF(87.41054111325998, 345.59403511772655));
    origPoints.append(QPointF(88.92566999722922, 340.55350980741866));
    origPoints.append(QPointF(90.43558548927187, 335.33698279869674));
    origPoints.append(QPointF(91.94171716088657, 329.93695094693794));
    origPoints.append(QPointF(93.44549458357191, 324.34591110751967));
    origPoints.append(QPointF(94.94834732882651, 318.55636013581915));
    origPoints.append(QPointF(96.45170496814899, 312.56079488721366));
    origPoints.append(QPointF(97.95699707303794, 306.35171221708043));
    origPoints.append(QPointF(99.46565321499199, 299.92160898079675));
    origPoints.append(QPointF(100.97910296550974, 293.26298203373995));
    origPoints.append(QPointF(101.73836126698214, 289.83563666815587));
    origPoints.append(QPointF(101.37719348141695, 284.37000549925995));
    origPoints.append(QPointF(100.64077948858204, 271.46526165758365));
    origPoints.append(QPointF(99.80144652509331, 256.5210633117996));
    origPoints.append(QPointF(99.03846354637537, 244.15048365420762));
    origPoints.append(QPointF(98.45199860267363, 235.49121370797783));
    origPoints.append(QPointF(97.78603837592823, 226.52753219856694));
    origPoints.append(QPointF(97.02863496827689, 217.3050951399407));
    origPoints.append(QPointF(96.16784048185737, 207.86955854606484));
    origPoints.append(QPointF(95.19170701880736, 198.26657843090516));
    origPoints.append(QPointF(94.08828668126463, 188.54181080842733));
    origPoints.append(QPointF(92.8456315713669, 178.7409116925971));
    origPoints.append(QPointF(91.8100755249774, 171.36785871320973));
    origPoints.append(QPointF(91.07312073651642, 166.45311304837324));
    origPoints.append(QPointF(90.29463656241913, 161.54501601805435));
    origPoints.append(QPointF(89.4731295154528, 156.6492746239989));
    origPoints.append(QPointF(88.60710610838458, 151.77159586795244));
    origPoints.append(QPointF(87.69507285398177, 146.91768675166082));
    origPoints.append(QPointF(86.73553626501155, 142.0932542768697));
    origPoints.append(QPointF(85.7270028542411, 137.3040054453248));
    origPoints.append(QPointF(84.6679791344377, 132.55564725877187));
    origPoints.append(QPointF(83.55697161836854, 127.85388671895657));
    origPoints.append(QPointF(82.39248681880082, 123.20443082762469));
    origPoints.append(QPointF(81.17303124850177, 118.61298658652187));
    origPoints.append(QPointF(79.8971114202386, 114.08526099739389));
    origPoints.append(QPointF(78.56323384677853, 109.62696106198644));
    origPoints.append(QPointF(77.1699050408888, 105.24379378204526));
    origPoints.append(QPointF(75.71563151533661, 100.94146615931604));
    origPoints.append(QPointF(74.19891978288913, 96.72568519554451));
    origPoints.append(QPointF(72.61827635631366, 92.60215789247638));
    origPoints.append(QPointF(70.97220774837734, 88.5765912518574));
    origPoints.append(QPointF(69.25922047184744, 84.65469227543325));
    origPoints.append(QPointF(67.47782103949115, 80.84216796494968));
    origPoints.append(QPointF(65.62651596407568, 77.14472532215238));
    origPoints.append(QPointF(63.703811758368275, 73.56807134878709));
    origPoints.append(QPointF(61.70821493513614, 70.11791304659951));
    origPoints.append(QPointF(59.63823200714647, 66.79995741733536));
    origPoints.append(QPointF(57.49236948716651, 63.61991146274037));
    origPoints.append(QPointF(55.8298252766917, 61.333435118024795));
    origPoints.append(QPointF(54.698584178428256, 59.852194710865774));
    origPoints.append(QPointF(53.5475330955988, 58.408641910856005));
    origPoints.append(QPointF(52.37648534229922, 57.00349009321371));
    origPoints.append(QPointF(51.18525423262544, 55.63745263315711));
    origPoints.append(QPointF(49.973653080673344, 54.31124290590439));
    origPoints.append(QPointF(48.741495200538836, 53.025574286673795));
    origPoints.append(QPointF(47.48859390631783, 51.78116015068352));
    origPoints.append(QPointF(46.21476251210622, 50.5787138731518));
    origPoints.append(QPointF(44.919814331999916, 49.41894882929684));
    origPoints.append(QPointF(43.60356268009479, 48.30257839433685));
    origPoints.append(QPointF(42.26582087048678, 47.23031594349004));
    origPoints.append(QPointF(40.906402217271776, 46.20287485197463));
    origPoints.append(QPointF(39.525120034545665, 45.22096849500884));
    origPoints.append(QPointF(38.12178763640438, 44.285310247810884));
    origPoints.append(QPointF(36.69621833694379, 43.39661348559898));
    origPoints.append(QPointF(35.24822545025982, 42.55559158359132));
    origPoints.append(QPointF(33.77762229044836, 41.76295791700616));
    origPoints.append(QPointF(32.28422217160532, 41.01942586106168));
    origPoints.append(QPointF(30.7678384078266, 40.32570879097611));

    // Begin comparison
    Comparison(pointsEkv, origPoints);
}
