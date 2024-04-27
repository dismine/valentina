/************************************************************************
 **
 **  @file   tst_vsvgpathtokenizer.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "tst_vsvgpathtokenizer.h"

#include "../vmisc/svgfont/vsvgpathtokenizer.h"
#include "def.h"

#include <QList>
#include <QtTest>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void CompareCommands(const VSVGPathCommand &actual, const VSVGPathCommand &expected)
{
    const QString msg =
        QStringLiteral("Commands mismatch. Expected: %1, actual: %2").arg(expected.m_command).arg(actual.m_command);
    QVERIFY2(actual.m_command == expected.m_command, qUtf8Printable(msg));

    const auto msg2 = QStringLiteral("Commands %1 arguments mismatch.").arg(actual.m_command);
    QVERIFY2(actual.m_arguments.size() == expected.m_arguments.size(),
             qUtf8Printable(msg2)); // First check if sizes are equal

    for (std::size_t i = 0; i < actual.m_arguments.size(); i++)
    {
        QVERIFY(VFuzzyComparePossibleNulls(actual.m_arguments.at(i), expected.m_arguments.at(i)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CompareCommandLists(const QList<VSVGPathCommand> &actual, const QList<VSVGPathCommand> &expected)
{
    QCOMPARE(actual.size(), expected.size()); // First check if sizes are equal

    for (int i = 0; i < actual.size(); i++)
    {
        CompareCommands(actual.at(i), expected.at(i));
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VSVGPathTokenizer::TST_VSVGPathTokenizer(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VSVGPathTokenizer::TokenizingSVGPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QList<VSVGPathCommand>>("expectedCommands");

    QTest::newRow("Empty path") << "" << QList<VSVGPathCommand>();

    {
        QList<VSVGPathCommand> const path = {{'M', {444, 665}}, {'L', {403, 592}}, {'L', {295, 362}},
                                             {'L', {232, 214}}, {'M', {204, 63}},  {'L', {182, 40.9}}};
        QTest::newRow("Valid path1") << "M 444 665 L 403 592 L 295 362 L 232 214 M 204 63 L 182 40.9" << path;
        QTest::newRow("Valid path1. No space") << "M444,665L403,592L295,362L232,214M204,63L182,40.9" << path;
    }

    {
        QList<VSVGPathCommand> const path = {{'M', {444, 665, 403, 592, 295, 362, 232, 214}},
                                             {'M', {204, 63, 182, 40.9}}};
        QTest::newRow("Valid path1. Implicit 'lineto'")
            << "M 444 665 403 592 295 362 232 214 M 204 63 182 40.9" << path;

        QTest::newRow("Valid path1. Implicit 'lineto'. No space")
            << "M444,665,403,592,295,362,232,214M204,63,182,40.9" << path;
    }

    {
        QList<VSVGPathCommand> const path = {{'M', {444, 403, 592, 295, 362, 232, 214}}, {'M', {204, 63, 182, 40.9}}};
        QTest::newRow("Invalid path1. Implicit 'lineto'")
            << "M 444 665r0 403 592 295 362 232 214 M 204 63 182 40.9" << path;
    }

    {
        QList<VSVGPathCommand> const path = {{'M', {89.2783, 316.097}},
                                             {'C', {400.408, 348.062, 712.222, 329.315, 1024.04, 321.159}},
                                             {'M', {1096.6, 543.884}},
                                             {'C', {1082.96, 623.613, 1099.42, 642.388, 1172.53, 710.927}},
                                             {'C', {1001.15, 691.624, 719.415, 750.709, 646.089, 638.373}},
                                             {'M', {-44.0188, -100.667}},
                                             {'C', {-44.0188, -100.667, 102.6, 292.235, 198.953, 419.023}},
                                             {'C', {295.306, 545.811, 398.471, 665.784, 479.046, 709.239}},
                                             {'C', {586.956, 762.684, 641.433, 740.563, 646.089, 692.366}},
                                             {'C', {638.726, 495.905, 445.602, 252.633, 504.355, 74.8129}},
                                             {'C', {532.714, -1.50505, 568.017, -23.3497, 654.525, -55.1095}},
                                             {'C', {741.033, -86.8694, 902.328, -79.7905, 1008.86, -51.7349}},
                                             {'C', {1115.39, -23.6793, 1277.14, 96.7478, 1277.14, 0.7478}}};
        QTest::newRow("Valid path2") << "M89.2783 316.097C400.408 348.062 712.222 329.315 1024.04 "
                                        "321.159M1096.6 543.884C1082.96 623.613 "
                                        "1099.42 642.388 1172.53 710.927C1001.15 691.624 719.415 "
                                        "750.709 646.089 638.373M-44.0188 "
                                        "-100.667C-44.0188 -100.667 102.6 292.235 198.953 "
                                        "419.023C295.306 545.811 398.471 665.784 479.046 "
                                        "709.239C586.956 762.684 641.433 740.563 646.089 "
                                        "692.366C638.726 495.905 445.602 252.633 504.355 "
                                        "74.8129C532.714 -1.50505 568.017 -23.3497 654.525 "
                                        "-55.1095C741.033 -86.8694 902.328 -79.7905 1008.86 "
                                        "-51.7349C1115.39 -23.6793 1277.14 96.7478 1277.14 .7478"
                                     << path;
    }

    {
        QList<VSVGPathCommand> const path = {
            {'M', {10, 30}},         {'A', {20, 20, 0, 0, 1, 50, 30}}, {'A', {20, 20, 0, 0, 1, 90, 30}},
            {'Q', {90, 60, 50, 90}}, {'Q', {10, 60, 10, 0.3}},         {'z', {0}}};
        QTest::newRow("Valid path3") << "M 10,30 A 20,20 0,0,1 50,30 A 20,20 0,0,1 90,30 Q 90,60 50,90 Q 10,60 10,.3 z"
                                     << path;
        QTest::newRow("Valid path3. No space")
            << "M10,30A20,20,0,0,1,50,30A20,20,0,0,1,90,30Q90,60,50,90Q10,60,10,.3z" << path;
    }

    {
        QList<VSVGPathCommand> const path = {
            {'M', {10, 10}},  {'h', {10}}, {'m', {0, 10}},   {'h', {10}}, {'m', {0, 10}},   {'h', {10}},
            {'M', {40, 20}},  {'h', {10}}, {'m', {0, 10}},   {'h', {10}}, {'m', {0, 10}},   {'h', {10}},
            {'m', {0, 10}},   {'h', {10}}, {'M', {50, 50}},  {'h', {10}}, {'m', {-20, 10}}, {'h', {10}},
            {'m', {-20, 10}}, {'h', {10}}, {'m', {-20, 10}}, {'h', {10}}};
        QTest::newRow("Valid path4") << "M 10,10 h 10 m  0,10 h 10 m  0,10 h 10 M 40,20 h 10 m  0,10 h 10 m  0,10 h 10 "
                                        "m  0,10 h 10 M 50,50 h 10 m-20,10 h 10 m-20,10 h 10 m-20,10 h 10"
                                     << path;
        QTest::newRow("Valid path4. No space")
            << "M10,10h10m0,10h10m0,10h10M40,20h10m0,10h10m0,10h10m0,10h10M50,50h10m-20,10h10m-20,10h10m-20,10h10"
            << path;
    }

    {
        QList<VSVGPathCommand> const path = {
            {'M', {109.547, 99.5815}}, {'L', {51.0984, -1.90735e-005}}, {'L', {0, 123.374}}};
        QTest::newRow("Valid path5") << "M 109.547,99.5815L 51.0984,-1.90735e-005L 0,123.374" << path;
        QTest::newRow("Valid path5. No space") << "M109.547,99.5815L51.0984-1.90735e-005L0,123.374" << path;
    }

    {
        QList<VSVGPathCommand> const path = {{'M', {198.44, 340.26}}, {'l', {-.4, 41.2}},    {'m', {13.93, -36.83}},
                                             {'l', {-.2, 44.12}},     {'m', {15.2, -42.25}}, {'l', {-.43, 46.2}},
                                             {'m', {13.96, -45.16}},  {'l', {-.22, 46.83}},  {'m', {15.4, -46.4}},
                                             {'v', {46.82}},          {'m', {14.57, -47.1}}, {'l', {.2, 46.8}},
                                             {'m', {12.92, -48.1}},   {'l', {.4, 46}},       {'m', {14.8, -48.5}},
                                             {'l', {.2, 44.1}},       {'m', {13.53, -49.6}}, {'l', {.4, 41.6}}};
        QTest::newRow("Valid path6")
            << "M198.44 340.26l-.4 41.2m13.93-36.83l-.2 44.12m15.2-42.25l-.43 46.2m13.96-45.16l-.22 "
               "46.83m15.4-46.4v46.82m14.57-47.1l.2 46.8m12.92-48.1l.4 46m14.8-48.5l.2 44.1m13.53-49.6l.4 41.6"
            << path;
    }

    {
        QList<VSVGPathCommand> const path = {
            {'M', {349.56, 354.82}},
            {'l', {94.5, -44.54}},
            {'c',
             {17.9,  -7.08, 20.4,   2.9,    36.2,  0,      12.08,  -.14,   22.07, -4.86, 21.24,  -16.23, -6.1,   -9.03,
              -11.8, -14.7, -22.48, -24.57, .56,   -10.54, -1.4,   -20.67, -5.4,  -30.8, -4.17,  -5.97,  -11.67, -8.6,
              -22.9, 2.08,  -11.38, 8.6,    -7.78, 20.12,  -22.07, 32.05,  -23.3, 11.6,  -44.54, 21.8,   -70.35, 34.6}},
            {'l', {-196.5, 67.9}},
            {'c', {-22.75, 12.1, -43.42, 23.3, -69.5, 33.3,  -21.24, 5.3,  -20.4, -7.7,  -46.2, .9,
                   -7.37,  5.8,  -7.23,  14.2, -1.26, 20.4,  6.93,   6.8,  11.8,  11.1,  17.9,  17.1,
                   1.94,   8.6,  2.22,   16,   3.33,  26.3,  4.16,   12.6, 13.33, 11.9,  21.23, 5.8,
                   3.47,   -4.3, 7.77,   -8.2, 10.4,  -12.9, 3.6,    -6.2, 7.64,  -13.3, 12.5,  -19.1}},
            {'l', {62.85, -33.3}},
            {'z', {0}}};
        QTest::newRow("Valid path6")
            << "M349.56 354.82l94.5-44.54c17.9-7.08 20.4 2.9 36.2 0 12.08-.14 22.07-4.86 "
               "21.24-16.23-6.1-9.03-11.8-14.7-22.48-24.57.56-10.54-1.4-20.67-5.4-30.8-4.17-5.97-11.67-8.6-22.9 "
               "2.08-11.38 "
               "8.6-7.78 20.12-22.07 32.05-23.3 11.6-44.54 21.8-70.35 34.6l-196.5 67.9c-22.75 12.1-43.42 23.3-69.5 "
               "33.3-21.24 5.3-20.4-7.7-46.2.9-7.37 5.8-7.23 14.2-1.26 20.4 6.93 6.8 11.8 11.1 17.9 17.1 1.94 8.6 2.22 "
               "16 "
               "3.33 26.3 4.16 12.6 13.33 11.9 21.23 5.8 3.47-4.3 7.77-8.2 10.4-12.9 3.6-6.2 7.64-13.3 "
               "12.5-19.1l62.85-33.3z"
            << path;
    }

    {
        QList<VSVGPathCommand> const path = {{'M', {167, 467}},
                                             {'v', {-243.688}},
                                             {'l', {33.2998, -44.6221}},
                                             {'l', {100, -22.3779}},
                                             {'h', {100}},
                                             {'l', {100, 22.3779}},
                                             {'l', {66.7002, 44.6221}},
                                             {'z', {0}},
                                             {'M', {201, 467}},
                                             {'l', {-0.700195, -243.688}},
                                             {'l', {33.2998, -44.6221}},
                                             {'l', {66.7002, -22.3779}},
                                             {'z', {0}},
                                             {'M', {567, 467}},
                                             {'v', {-467}},
                                             {'z', {0}},
                                             {'M', {600, 467}},
                                             {'l', {0.299805, -467}},
                                             {'z', {0}},
                                             {'M', {67, 467}},
                                             {'h', {233}},
                                             {'z', {0}},
                                             {'M', {467, 467}},
                                             {'h', {234}},
                                             {'z', {0}},
                                             {'M', {467, 0}},
                                             {'h', {233.3}},
                                             {'z', {0}}};
        QTest::newRow("Valid path7") << "M167 467v-243.688l33.2998 -44.6221l100 -22.3779h100l100 22.3779l66.7002 "
                                        "44.6221zM201 467l-0.700195 -243.688l33.2998 -44.6221l66.7002 -22.3779zM567 "
                                        "467v-467zM600 467l0.299805 -467zM67 467h233zM467 467h234zM467 0h233.3z"
                                     << path;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VSVGPathTokenizer::TokenizingSVGPath() const
{
    QFETCH(QString, path);
    QFETCH(QList<VSVGPathCommand>, expectedCommands);

    VSVGPathTokenizer const tokenizer(path);
    QList<VSVGPathCommand> const commands = tokenizer.GetCommands();
    CompareCommandLists(commands, expectedCommands);
}
