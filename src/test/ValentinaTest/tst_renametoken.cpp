/************************************************************************
 **
 **  @file   tst_renametoken.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 5, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#include "tst_renametoken.h"
#include "../vtools/undocommands/renametoken.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_RenameToken::TST_RenameToken(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenLabel_data()
{
    QTest::addColumn<QString>("token");
    QTest::addColumn<QString>("oldLabel");
    QTest::addColumn<QString>("newLabel");
    QTest::addColumn<QString>("expected");

    // Two-label prefix — label1 replaced
    QTest::newRow("Line label1") << u"Line_A_B"_s << u"A"_s << u"C"_s << u"Line_C_B"_s;
    QTest::newRow("AngleLine label1") << u"AngleLine_A_B"_s << u"A"_s << u"C"_s << u"AngleLine_C_B"_s;
    QTest::newRow("Spl label1") << u"Spl_A_B"_s << u"A"_s << u"C"_s << u"Spl_C_B"_s;
    QTest::newRow("SplPath label1") << u"SplPath_A_B"_s << u"A"_s << u"C"_s << u"SplPath_C_B"_s;
    QTest::newRow("C1LengthSpl label1") << u"C1LengthSpl_A_B"_s << u"A"_s << u"C"_s << u"C1LengthSpl_C_B"_s;

    // Two-label prefix — label1 replaced with numeric suffix
    QTest::newRow("Line label1 with suffix") << u"Line_A_B_2"_s << u"A"_s << u"C"_s << u"Line_C_B_2"_s;
    QTest::newRow("Spl label1 with suffix") << u"Spl_A_B_3"_s << u"A"_s << u"C"_s << u"Spl_C_B_3"_s;

    // Two-label prefix — label2 replaced
    QTest::newRow("Line label2") << u"Line_A_B"_s << u"B"_s << u"D"_s << u"Line_A_D"_s;
    QTest::newRow("AngleLine label2") << u"AngleLine_A_B"_s << u"B"_s << u"D"_s << u"AngleLine_A_D"_s;
    QTest::newRow("Spl label2") << u"Spl_A_B"_s << u"B"_s << u"D"_s << u"Spl_A_D"_s;

    // Two-label prefix — label2 replaced with numeric suffix
    QTest::newRow("Line label2 with suffix") << u"Line_A_B_2"_s << u"B"_s << u"D"_s << u"Line_A_D_2"_s;
    QTest::newRow("Spl label2 with suffix") << u"Spl_A_B_3"_s << u"B"_s << u"D"_s << u"Spl_A_D_3"_s;

    // One-label prefix — label replaced (with numeric ID)
    QTest::newRow("Arc label") << u"Arc_A_5"_s << u"A"_s << u"B"_s << u"Arc_B_5"_s;
    QTest::newRow("RadiusArc label") << u"RadiusArc_A_5"_s << u"A"_s << u"B"_s << u"RadiusArc_B_5"_s;
    QTest::newRow("Angle1Arc label") << u"Angle1Arc_A_5"_s << u"A"_s << u"B"_s << u"Angle1Arc_B_5"_s;
    QTest::newRow("ElArc label") << u"ElArc_A_5"_s << u"A"_s << u"B"_s << u"ElArc_B_5"_s;
    QTest::newRow("Radius1ElArc label") << u"Radius1ElArc_A_5"_s << u"A"_s << u"B"_s << u"Radius1ElArc_B_5"_s;

    // One-label prefix — label replaced with duplicate
    QTest::newRow("Arc label with duplicate") << u"Arc_A_5_2"_s << u"A"_s << u"B"_s << u"Arc_B_5_2"_s;
    QTest::newRow("ElArc label with duplicate") << u"ElArc_A_5_3"_s << u"A"_s << u"B"_s << u"ElArc_B_5_3"_s;

    // Duplicate-label guard: renaming one label to match the other must leave token unchanged
    QTest::newRow("Guard: label1 would equal label2")
        << u"Spl_A_B"_s << u"A"_s << u"B"_s << u"Spl_A_B"_s;
    QTest::newRow("Guard: label2 would equal label1")
        << u"Spl_A_B"_s << u"B"_s << u"A"_s << u"Spl_A_B"_s;
    QTest::newRow("Guard: SplPath label2 would equal label1")
        << u"SplPath_A343_B"_s << u"B"_s << u"A343"_s << u"SplPath_A343_B"_s;
    QTest::newRow("Guard: Line label1 would equal label2")
        << u"Line_A_B"_s << u"A"_s << u"B"_s << u"Line_A_B"_s;
    QTest::newRow("Guard: label1 with suffix would equal label2")
        << u"Spl_A_B_2"_s << u"A"_s << u"B"_s << u"Spl_A_B_2"_s;
    QTest::newRow("Guard: label2 with suffix would equal label1")
        << u"Spl_A_B_2"_s << u"B"_s << u"A"_s << u"Spl_A_B_2"_s;

    // No match — returns token unchanged
    QTest::newRow("No match different label") << u"Line_X_Y"_s << u"A"_s << u"C"_s << u"Line_X_Y"_s;
    QTest::newRow("No match unknown prefix") << u"Unknown_A_B"_s << u"A"_s << u"C"_s << u"Unknown_A_B"_s;
    QTest::newRow("No match bare name") << u"SomeVar"_s << u"SomeVar"_s << u"NewVar"_s << u"SomeVar"_s;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenLabel()
{
    QFETCH(QString, token);
    QFETCH(QString, oldLabel);
    QFETCH(QString, newLabel);
    QFETCH(QString, expected);

    QCOMPARE(::ReplaceTokenLabel(token, oldLabel, newLabel), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenPair_data()
{
    QTest::addColumn<QString>("token");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("oldFirst");
    QTest::addColumn<QString>("oldSecond");
    QTest::addColumn<QString>("newFirst");
    QTest::addColumn<QString>("newSecond");
    QTest::addColumn<quint32>("oldDuplicate");
    QTest::addColumn<quint32>("newDuplicate");
    QTest::addColumn<QString>("expected");

    // Line type
    QTest::newRow("Line_A_B renamed to Line_C_D")
        << u"Line_A_B"_s << static_cast<int>(RenameObjectType::Line)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(0) << quint32(0)
        << u"Line_C_D"_s;
    QTest::newRow("AngleLine_A_B renamed to AngleLine_C_D")
        << u"AngleLine_A_B"_s << static_cast<int>(RenameObjectType::Line)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(0) << quint32(0)
        << u"AngleLine_C_D"_s;

    // Spline type
    QTest::newRow("Spl_A_B renamed to Spl_C_D")
        << u"Spl_A_B"_s << static_cast<int>(RenameObjectType::Spline)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(0) << quint32(0)
        << u"Spl_C_D"_s;
    QTest::newRow("Spl_A_B_1 renamed to Spl_C_D_2 with duplicates")
        << u"Spl_A_B_1"_s << static_cast<int>(RenameObjectType::Spline)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(1) << quint32(2)
        << u"Spl_C_D_2"_s;
    QTest::newRow("C1LengthSpl_A_B renamed")
        << u"C1LengthSpl_A_B"_s << static_cast<int>(RenameObjectType::Spline)
        << u"A"_s << u"B"_s << u"X"_s << u"Y"_s << quint32(0) << quint32(0)
        << u"C1LengthSpl_X_Y"_s;

    // SplinePath type
    QTest::newRow("SplPath_A_B renamed to SplPath_C_D")
        << u"SplPath_A_B"_s << static_cast<int>(RenameObjectType::SplinePath)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(0) << quint32(0)
        << u"SplPath_C_D"_s;
    QTest::newRow("C1LengthSplPath_A_B renamed")
        << u"C1LengthSplPath_A_B"_s << static_cast<int>(RenameObjectType::SplinePath)
        << u"A"_s << u"B"_s << u"X"_s << u"Y"_s << quint32(0) << quint32(0)
        << u"C1LengthSplPath_X_Y"_s;

    // Duplicate-label guard: newPair with equal labels must leave token unchanged
    QTest::newRow("Guard: Line newPair has equal labels")
        << u"Line_A_B"_s << static_cast<int>(RenameObjectType::Line)
        << u"A"_s << u"B"_s << u"B"_s << u"B"_s << quint32(0) << quint32(0)
        << u"Line_A_B"_s;
    QTest::newRow("Guard: Spl newPair has equal labels")
        << u"Spl_A_B"_s << static_cast<int>(RenameObjectType::Spline)
        << u"A"_s << u"B"_s << u"C"_s << u"C"_s << quint32(0) << quint32(0)
        << u"Spl_A_B"_s;

    // No match
    QTest::newRow("Line no match wrong pair")
        << u"Line_X_Y"_s << static_cast<int>(RenameObjectType::Line)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(0) << quint32(0)
        << u"Line_X_Y"_s;
    QTest::newRow("Spl wrong type for Line token")
        << u"Line_A_B"_s << static_cast<int>(RenameObjectType::Spline)
        << u"A"_s << u"B"_s << u"C"_s << u"D"_s << quint32(0) << quint32(0)
        << u"Line_A_B"_s;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenPair()
{
    QFETCH(QString, token);
    QFETCH(int, type);
    QFETCH(QString, oldFirst);
    QFETCH(QString, oldSecond);
    QFETCH(QString, newFirst);
    QFETCH(QString, newSecond);
    QFETCH(quint32, oldDuplicate);
    QFETCH(quint32, newDuplicate);
    QFETCH(QString, expected);

    const ObjectPair_t oldPair{oldFirst, oldSecond};
    const ObjectPair_t newPair{newFirst, newSecond};
    const auto renameType = static_cast<RenameObjectType>(type);

    QCOMPARE(::ReplaceTokenPair(token, renameType, oldPair, newPair, oldDuplicate, newDuplicate), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenAlias_data()
{
    QTest::addColumn<QString>("token");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("oldAlias");
    QTest::addColumn<QString>("newAlias");
    QTest::addColumn<QString>("expected");

    // Arc type
    QTest::newRow("Arc alias renamed")
        << u"Arc_myAlias"_s << static_cast<int>(CurveAliasType::Arc)
        << u"myAlias"_s << u"newAlias"_s << u"Arc_newAlias"_s;
    QTest::newRow("RadiusArc alias renamed")
        << u"RadiusArc_myAlias"_s << static_cast<int>(CurveAliasType::Arc)
        << u"myAlias"_s << u"newAlias"_s << u"RadiusArc_newAlias"_s;
    QTest::newRow("Angle1Arc alias renamed")
        << u"Angle1Arc_myAlias"_s << static_cast<int>(CurveAliasType::Arc)
        << u"myAlias"_s << u"newAlias"_s << u"Angle1Arc_newAlias"_s;

    // ElArc type
    QTest::newRow("ElArc alias renamed")
        << u"ElArc_myAlias"_s << static_cast<int>(CurveAliasType::ElArc)
        << u"myAlias"_s << u"newAlias"_s << u"ElArc_newAlias"_s;
    QTest::newRow("RotationElArc alias renamed")
        << u"RotationElArc_myAlias"_s << static_cast<int>(CurveAliasType::ElArc)
        << u"myAlias"_s << u"newAlias"_s << u"RotationElArc_newAlias"_s;

    // Spline type
    QTest::newRow("Spl alias renamed")
        << u"Spl_myAlias"_s << static_cast<int>(CurveAliasType::Spline)
        << u"myAlias"_s << u"newAlias"_s << u"Spl_newAlias"_s;
    QTest::newRow("Angle1Spl alias renamed")
        << u"Angle1Spl_myAlias"_s << static_cast<int>(CurveAliasType::Spline)
        << u"myAlias"_s << u"newAlias"_s << u"Angle1Spl_newAlias"_s;

    // SplinePath type
    QTest::newRow("SplPath alias renamed")
        << u"SplPath_myAlias"_s << static_cast<int>(CurveAliasType::SplinePath)
        << u"myAlias"_s << u"newAlias"_s << u"SplPath_newAlias"_s;

    // All type — checks a few representative prefixes
    QTest::newRow("Arc prefix with All type")
        << u"Arc_myAlias"_s << static_cast<int>(CurveAliasType::All)
        << u"myAlias"_s << u"newAlias"_s << u"Arc_newAlias"_s;
    QTest::newRow("Spl prefix with All type")
        << u"Spl_myAlias"_s << static_cast<int>(CurveAliasType::All)
        << u"myAlias"_s << u"newAlias"_s << u"Spl_newAlias"_s;

    // No match
    QTest::newRow("Arc no match wrong alias")
        << u"Arc_otherAlias"_s << static_cast<int>(CurveAliasType::Arc)
        << u"myAlias"_s << u"newAlias"_s << u"Arc_otherAlias"_s;
    QTest::newRow("Spl no match wrong type")
        << u"Spl_myAlias"_s << static_cast<int>(CurveAliasType::Arc)
        << u"myAlias"_s << u"newAlias"_s << u"Spl_myAlias"_s;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenAlias()
{
    QFETCH(QString, token);
    QFETCH(int, type);
    QFETCH(QString, oldAlias);
    QFETCH(QString, newAlias);
    QFETCH(QString, expected);

    const auto curveType = static_cast<CurveAliasType>(type);

    QCOMPARE(::ReplaceTokenAlias(token, curveType, oldAlias, newAlias), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenSegmentCurve_data()
{
    QTest::addColumn<QString>("token");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("pointName");
    QTest::addColumn<QString>("leftSub");
    QTest::addColumn<QString>("rightSub");
    QTest::addColumn<QString>("expected");

    // Left substitution: prefix_<label>_pointName -> prefix_leftSub
    QTest::newRow("Spl left sub")
        << u"Spl_A_P"_s << static_cast<int>(CurveAliasType::Spline)
        << u"P"_s << u"A_P1"_s << QString()
        << u"Spl_A_P1"_s;
    QTest::newRow("Arc left sub")
        << u"Arc_A_P"_s << static_cast<int>(CurveAliasType::Arc)
        << u"P"_s << u"A_P1"_s << QString()
        << u"Arc_A_P1"_s;

    // Right substitution: prefix_pointName_<label> -> prefix_rightSub
    QTest::newRow("Spl right sub")
        << u"Spl_P_B"_s << static_cast<int>(CurveAliasType::Spline)
        << u"P"_s << QString() << u"P1_B"_s
        << u"Spl_P1_B"_s;
    QTest::newRow("Arc right sub")
        << u"Arc_P_B"_s << static_cast<int>(CurveAliasType::Arc)
        << u"P"_s << QString() << u"P1_B"_s
        << u"Arc_P1_B"_s;
    QTest::newRow("SplPath right sub")
        << u"SplPath_P_B"_s << static_cast<int>(CurveAliasType::SplinePath)
        << u"P"_s << QString() << u"P1_B"_s
        << u"SplPath_P1_B"_s;

    // No match — point not in token
    QTest::newRow("Spl no match point absent")
        << u"Spl_A_B"_s << static_cast<int>(CurveAliasType::Spline)
        << u"P"_s << u"A_P1"_s << u"P1_B"_s
        << u"Spl_A_B"_s;

    // Empty pointName — always returns token unchanged
    QTest::newRow("Empty pointName")
        << u"Spl_A_B"_s << static_cast<int>(CurveAliasType::Spline)
        << QString() << u"A_P1"_s << u"P1_B"_s
        << u"Spl_A_B"_s;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenSegmentCurve()
{
    QFETCH(QString, token);
    QFETCH(int, type);
    QFETCH(QString, pointName);
    QFETCH(QString, leftSub);
    QFETCH(QString, rightSub);
    QFETCH(QString, expected);

    const auto curveType = static_cast<CurveAliasType>(type);

    QCOMPARE(::ReplaceTokenSegmentCurve(token, curveType, pointName, leftSub, rightSub), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenArc_data()
{
    QTest::addColumn<QString>("token");
    QTest::addColumn<int>("type");
    QTest::addColumn<QString>("oldCenter");
    QTest::addColumn<QString>("newCenter");
    QTest::addColumn<quint32>("id");
    QTest::addColumn<quint32>("oldDuplicate");
    QTest::addColumn<quint32>("newDuplicate");
    QTest::addColumn<QString>("expected");

    // Arc type — no duplicate
    QTest::newRow("Arc center renamed")
        << u"Arc_A_5"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"Arc_B_5"_s;
    QTest::newRow("RadiusArc center renamed")
        << u"RadiusArc_A_5"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"RadiusArc_B_5"_s;
    QTest::newRow("Angle1Arc center renamed")
        << u"Angle1Arc_A_5"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"Angle1Arc_B_5"_s;
    QTest::newRow("Angle2Arc center renamed")
        << u"Angle2Arc_A_5"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"Angle2Arc_B_5"_s;

    // Arc type — with duplicate
    QTest::newRow("Arc center renamed with duplicate")
        << u"Arc_A_5_2"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(2) << quint32(2)
        << u"Arc_B_5_2"_s;
    QTest::newRow("Arc center renamed old dup to new dup")
        << u"Arc_A_5_1"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(1) << quint32(3)
        << u"Arc_B_5_3"_s;

    // ElArc type — no duplicate
    QTest::newRow("ElArc center renamed")
        << u"ElArc_A_7"_s << static_cast<int>(RenameArcType::ElArc)
        << u"A"_s << u"B"_s << quint32(7) << quint32(0) << quint32(0)
        << u"ElArc_B_7"_s;
    QTest::newRow("RotationElArc center renamed")
        << u"RotationElArc_A_7"_s << static_cast<int>(RenameArcType::ElArc)
        << u"A"_s << u"B"_s << quint32(7) << quint32(0) << quint32(0)
        << u"RotationElArc_B_7"_s;
    QTest::newRow("Radius1ElArc center renamed")
        << u"Radius1ElArc_A_7"_s << static_cast<int>(RenameArcType::ElArc)
        << u"A"_s << u"B"_s << quint32(7) << quint32(0) << quint32(0)
        << u"Radius1ElArc_B_7"_s;

    // ElArc type — with duplicate
    QTest::newRow("ElArc center renamed with duplicate")
        << u"ElArc_A_7_2"_s << static_cast<int>(RenameArcType::ElArc)
        << u"A"_s << u"B"_s << quint32(7) << quint32(2) << quint32(2)
        << u"ElArc_B_7_2"_s;

    // No match — wrong center label
    QTest::newRow("Arc no match wrong center")
        << u"Arc_X_5"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"Arc_X_5"_s;

    // No match — wrong id
    QTest::newRow("Arc no match wrong id")
        << u"Arc_A_99"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"Arc_A_99"_s;

    // No match — ElArc prefix with Arc type
    QTest::newRow("ElArc prefix with Arc type no match")
        << u"ElArc_A_5"_s << static_cast<int>(RenameArcType::Arc)
        << u"A"_s << u"B"_s << quint32(5) << quint32(0) << quint32(0)
        << u"ElArc_A_5"_s;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_RenameToken::ReplaceTokenArc()
{
    QFETCH(QString, token);
    QFETCH(int, type);
    QFETCH(QString, oldCenter);
    QFETCH(QString, newCenter);
    QFETCH(quint32, id);
    QFETCH(quint32, oldDuplicate);
    QFETCH(quint32, newDuplicate);
    QFETCH(QString, expected);

    const auto arcType = static_cast<RenameArcType>(type);

    QCOMPARE(::ReplaceTokenArc(token, arcType, oldCenter, newCenter, id, oldDuplicate, newDuplicate), expected);
}
