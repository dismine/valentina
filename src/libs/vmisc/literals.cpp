/************************************************************************
 **
 **  @file   literals.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 4, 2018
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2018 Valentina project
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
#include "literals.h"

#include <QString>
#include <QStringList>

const QString LONG_OPTION_NO_HDPI_SCALING = QStringLiteral("no-scaling");

const QString degreeSymbol = QStringLiteral("°");
const QString trueStr = QStringLiteral("true");
const QString falseStr = QStringLiteral("false");

// functions
const QString degTorad_F = QStringLiteral("degTorad");
const QString radTodeg_F = QStringLiteral("radTodeg");
const QString sin_F = QStringLiteral("sin");
const QString cos_F = QStringLiteral("cos");
const QString tan_F = QStringLiteral("tan");
const QString asin_F = QStringLiteral("asin");
const QString acos_F = QStringLiteral("acos");
const QString atan_F = QStringLiteral("atan");
const QString sinh_F = QStringLiteral("sinh");
const QString cosh_F = QStringLiteral("cosh");
const QString tanh_F = QStringLiteral("tanh");
const QString asinh_F = QStringLiteral("asinh");
const QString acosh_F = QStringLiteral("acosh");
const QString atanh_F = QStringLiteral("atanh");
const QString sinD_F = QStringLiteral("sinD");
const QString cosD_F = QStringLiteral("cosD");
const QString tanD_F = QStringLiteral("tanD");
const QString asinD_F = QStringLiteral("asinD");
const QString acosD_F = QStringLiteral("acosD");
const QString atanD_F = QStringLiteral("atanD");
const QString log2_F = QStringLiteral("log2");
const QString log10_F = QStringLiteral("log10");
const QString log_F = QStringLiteral("log");
const QString ln_F = QStringLiteral("ln");
const QString exp_F = QStringLiteral("exp");
const QString sqrt_F = QStringLiteral("sqrt");
const QString sign_F = QStringLiteral("sign");
const QString rint_F = QStringLiteral("rint");
const QString r2cm_F = QStringLiteral("r2cm");
const QString csrCm_F = QStringLiteral("csrCm");
const QString csrInch_F = QStringLiteral("csrInch");
const QString abs_F = QStringLiteral("abs");
const QString min_F = QStringLiteral("min");
const QString max_F = QStringLiteral("max");
const QString sum_F = QStringLiteral("sum");
const QString avg_F = QStringLiteral("avg");
const QString fmod_F = QStringLiteral("fmod");
const QString warning_F = QStringLiteral("warning");

auto BuilInFunctions() -> QStringList
{
    return {degTorad_F, radTodeg_F, sin_F,   cos_F,   tan_F,  asin_F, acos_F, atan_F,   sinh_F,  cosh_F,
            tanh_F,     asinh_F,    acosh_F, atanh_F, sinD_F, cosD_F, tanD_F, asinD_F,  acosD_F, atanD_F,
            log2_F,     log10_F,    log_F,   ln_F,    exp_F,  sqrt_F, sign_F, rint_F,   r2cm_F,  csrCm_F,
            csrInch_F,  abs_F,      min_F,   max_F,   sum_F,  avg_F,  fmod_F, warning_F};
}

const QString pl_size = QStringLiteral("size");
const QString pl_height = QStringLiteral("height");
const QString pl_hip = QStringLiteral("hip");
const QString pl_waist = QStringLiteral("waist");

const QString pl_sizeLabel = QStringLiteral("sizeLabel");
const QString pl_heightLabel = QStringLiteral("heightLabel");
const QString pl_hipLabel = QStringLiteral("hipLabel");
const QString pl_waistLabel = QStringLiteral("waistLabel");

const QString pl_dimensionX = QStringLiteral("dimensionX");
const QString pl_dimensionY = QStringLiteral("dimensionY");
const QString pl_dimensionW = QStringLiteral("dimensionW");
const QString pl_dimensionZ = QStringLiteral("dimensionZ");

const QString pl_dimensionXLabel = QStringLiteral("dimensionXLabel");
const QString pl_dimensionYLabel = QStringLiteral("dimensionYLabel");
const QString pl_dimensionWLabel = QStringLiteral("dimensionWLabel");
const QString pl_dimensionZLabel = QStringLiteral("dimensionZLabel");

const QString pl_date = QStringLiteral("date");
const QString pl_time = QStringLiteral("time");
const QString pl_birthDate = QStringLiteral("birthDate");
const QString pl_patternName = QStringLiteral("patternName");
const QString pl_patternNumber = QStringLiteral("patternNumber");
const QString pl_author = QStringLiteral("author");
const QString pl_customer = QStringLiteral("customer");
const QString pl_email = QStringLiteral("email");
const QString pl_userMaterial = QStringLiteral("userMaterial");
const QString pl_pExt = QStringLiteral("pExt");
const QString pl_pUnits = QStringLiteral("pUnits");
const QString pl_pFileName = QStringLiteral("pFileName");
const QString pl_mFileName = QStringLiteral("mFileName");
const QString pl_mExt = QStringLiteral("mExt");
const QString pl_mUnits = QStringLiteral("mUnits");
const QString pl_mSizeUnits = QStringLiteral("mSizeUnits");
const QString pl_areaUnits = QStringLiteral("areaUnits");
const QString pl_pLetter = QStringLiteral("pLetter");
const QString pl_pAnnotation = QStringLiteral("pAnnotation");
const QString pl_pOrientation = QStringLiteral("pOrientation");
const QString pl_pRotation = QStringLiteral("pRotation");
const QString pl_pTilt = QStringLiteral("pTilt");
const QString pl_pFoldPosition = QStringLiteral("pFoldPosition");
const QString pl_pName = QStringLiteral("pName");
const QString pl_pQuantity = QStringLiteral("pQuantity");
const QString pl_mFabric = QStringLiteral("mFabric");
const QString pl_mLining = QStringLiteral("mLining");
const QString pl_mInterfacing = QStringLiteral("mInterfacing");
const QString pl_mInterlining = QStringLiteral("mInterlining");
const QString pl_wCut = QStringLiteral("wCut");
const QString pl_wOnFold = QStringLiteral("wOnFold");
const QString pl_measurement = QStringLiteral("measurement_");
const QString pl_finalMeasurement = QStringLiteral("finalMeasurement_");
const QString pl_currentArea = QStringLiteral("currentArea");
const QString pl_currentSeamLineArea = QStringLiteral("currentSeamLineArea");

const QString cursorArrowOpenHand = QStringLiteral("://cursor/cursor-arrow-openhand.png");
const QString cursorArrowCloseHand = QStringLiteral("://cursor/cursor-arrow-closehand.png");

const QString strOne = QStringLiteral("one");
const QString strTwo = QStringLiteral("two");
const QString strThree = QStringLiteral("three");

const QString strStraightforward = QStringLiteral("straightforward");
const QString strBisector = QStringLiteral("bisector");
const QString strIntersection = QStringLiteral("intersection");
const QString strIntersectionOnlyLeft = QStringLiteral("intersectionLeft");
const QString strIntersectionOnlyRight = QStringLiteral("intersectionRight");
const QString strIntersection2 = QStringLiteral("intersection2");
const QString strIntersection2OnlyLeft = QStringLiteral("intersection2Left");
const QString strIntersection2OnlyRight = QStringLiteral("intersection2Right");
const QString strTypeIncrement = QStringLiteral("increment");
const QString strTypeSeparator = QStringLiteral("separator");
const QString strTypeMeasurement = QStringLiteral("measurement");
const QString strFoldTypeTwoArrowsTextAbove = QStringLiteral("2ArrowsTextAbove");
const QString strFoldTypeTwoArrowsTextUnder = QStringLiteral("2ArrowsTextUnder");
const QString strFoldTypeTwoArrows = QStringLiteral("2Arrows");
const QString strFoldTypeText = QStringLiteral("text");
const QString strFoldTypeThreeDots = QStringLiteral("3dots");
const QString strFoldTypeThreeX = QStringLiteral("3X");
const QString strFoldTypeNone = QStringLiteral("none");

const QString unitMM = QStringLiteral("mm");
const QString unitCM = QStringLiteral("cm");
const QString unitINCH = QStringLiteral("inch");
const QString unitPX = QStringLiteral("px");

const QString valentinaNamespace = QStringLiteral("valentina");
