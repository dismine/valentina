/************************************************************************
 **
 **  @file   literals.h
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
#ifndef LITERALS_H
#define LITERALS_H

#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>

class QString;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringList;
#endif

// From documantation: If you use QStringLiteral you should avoid declaring the same literal in multiple places: This
// furthermore blows up the binary sizes.

extern const QString LONG_OPTION_NO_HDPI_SCALING;

extern const QString editDeleteIcon;
extern const QString preferencesOtherIcon;
extern const QString degreeSymbol;
extern const QString trueStr;
extern const QString falseStr;

// functions
extern const QString degTorad_F;
extern const QString radTodeg_F;
extern const QString sin_F;
extern const QString cos_F;
extern const QString tan_F;
extern const QString asin_F;
extern const QString acos_F;
extern const QString atan_F;
extern const QString sinh_F;
extern const QString cosh_F;
extern const QString tanh_F;
extern const QString asinh_F;
extern const QString acosh_F;
extern const QString atanh_F;
extern const QString sinD_F;
extern const QString cosD_F;
extern const QString tanD_F;
extern const QString asinD_F;
extern const QString acosD_F;
extern const QString atanD_F;
extern const QString log2_F;
extern const QString log10_F;
extern const QString log_F;
extern const QString ln_F;
extern const QString exp_F;
extern const QString sqrt_F;
extern const QString sign_F;
extern const QString rint_F;
extern const QString r2cm_F;
extern const QString csrCm_F;
extern const QString csrInch_F;
extern const QString abs_F;
extern const QString min_F;
extern const QString max_F;
extern const QString sum_F;
extern const QString avg_F;
extern const QString fmod_F;
extern const QString warning_F;

auto BuilInFunctions() -> QStringList;

// Placeholders
extern const QString pl_height;
extern const QString pl_size;
extern const QString pl_waist;
extern const QString pl_hip;

extern const QString pl_heightLabel;
extern const QString pl_sizeLabel;
extern const QString pl_hipLabel;
extern const QString pl_waistLabel;

extern const QString pl_dimensionX;
extern const QString pl_dimensionY;
extern const QString pl_dimensionW;
extern const QString pl_dimensionZ;

extern const QString pl_dimensionXLabel;
extern const QString pl_dimensionYLabel;
extern const QString pl_dimensionWLabel;
extern const QString pl_dimensionZLabel;

extern const QString pl_date;
extern const QString pl_time;
extern const QString pl_birthDate;
extern const QString pl_patternName;
extern const QString pl_patternNumber;
extern const QString pl_author;
extern const QString pl_customer;
extern const QString pl_email;
extern const QString pl_userMaterial;
extern const QString pl_pExt;
extern const QString pl_pUnits;
extern const QString pl_pFileName;
extern const QString pl_mFileName;
extern const QString pl_mExt;
extern const QString pl_mUnits;
extern const QString pl_mSizeUnits;
extern const QString pl_areaUnits;
extern const QString pl_pLetter;
extern const QString pl_pAnnotation;
extern const QString pl_pOrientation;
extern const QString pl_pRotation;
extern const QString pl_pTilt;
extern const QString pl_pFoldPosition;
extern const QString pl_pName;
extern const QString pl_pQuantity;
extern const QString pl_mFabric;
extern const QString pl_mLining;
extern const QString pl_mInterfacing;
extern const QString pl_mInterlining;
extern const QString pl_wCut;
extern const QString pl_wOnFold;
extern const QString pl_measurement;
extern const QString pl_finalMeasurement;
extern const QString pl_currentArea;
extern const QString pl_currentSeamLineArea;

extern const QString cursorArrowOpenHand;
extern const QString cursorArrowCloseHand;

extern const QString strOne;
extern const QString strTwo;
extern const QString strThree;

extern const QString strStraightforward;
extern const QString strBisector;
extern const QString strIntersection;
extern const QString strIntersectionOnlyLeft;
extern const QString strIntersectionOnlyRight;
extern const QString strIntersection2;
extern const QString strIntersection2OnlyLeft;
extern const QString strIntersection2OnlyRight;
extern const QString strTypeIncrement;
extern const QString strTypeSeparator;
extern const QString strTypeMeasurement;
extern const QString strFoldTypeTwoArrowsTextAbove;
extern const QString strFoldTypeTwoArrowsTextUnder;
extern const QString strFoldTypeTwoArrows;
extern const QString strFoldTypeText;
extern const QString strFoldTypeThreeDots;
extern const QString strFoldTypeThreeX;
extern const QString strFoldTypeNone;

extern const QString unitMM;
extern const QString unitCM;
extern const QString unitINCH;
extern const QString unitPX;

extern const QString valentinaNamespace;

#endif // LITERALS_H
