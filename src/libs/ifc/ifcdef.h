/************************************************************************
 **
 **  @file   ifcdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 12, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef IFCDEF_H
#define IFCDEF_H

#include <QtGlobal>
#ifdef Q_OS_WIN
//   extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#include <qt_windows.h>
#endif /*Q_OS_WIN*/

#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>

#include "../vmisc/def.h"
#include "../vmisc/typedef.h"

extern const QString CustomMSign;
extern const QString CustomIncrSign;

// Detect whether the compiler supports C++11 noexcept exception specifications.
#if defined(__clang__)
#if __has_feature(cxx_noexcept)
#define V_NOEXCEPT_EXPR(x) noexcept(x) // Clang 3.0 and above have noexcept
#endif
#elif defined(__GNUC__)
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#define V_NOEXCEPT_EXPR(x) noexcept(x) // GCC 4.7 and following have noexcept
#endif
#elif defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
#define V_NOEXCEPT_EXPR(x) noexcept(x) // Visual Studio 2015 and following have noexcept
#else
#define V_NOEXCEPT_EXPR(x)
#endif

extern const QString AttrType;
extern const QString AttrMx;
extern const QString AttrMy;
extern const QString AttrName;
extern const QString AttrShortName;
extern const QString AttrUUID;
extern const QString AttrGradationLabel;
extern const QString AttrMx1;
extern const QString AttrMy1;
extern const QString AttrName1;
extern const QString AttrMx2;
extern const QString AttrMy2;
extern const QString AttrName2;
extern const QString AttrBaseLineP1;
extern const QString AttrBaseLineP2;
extern const QString AttrDartP1;
extern const QString AttrDartP2;
extern const QString AttrDartP3;
extern const QString AttrX;
extern const QString AttrY;
extern const QString AttrTypeLine;
extern const QString AttrCut;
extern const QString AttrLength;
extern const QString AttrBasePoint;
extern const QString AttrFirstPoint;
extern const QString AttrSecondPoint;
extern const QString AttrThirdPoint;
extern const QString AttrCenter;
extern const QString AttrRadius;
extern const QString AttrRadius1;
extern const QString AttrRadius2;
extern const QString AttrAngle;
extern const QString AttrAngle1;
extern const QString AttrAngle2;
extern const QString AttrRotationAngle;
extern const QString AttrLength1;
extern const QString AttrLength2;
extern const QString AttrP1Line;
extern const QString AttrP2Line;
extern const QString AttrP1Line1;
extern const QString AttrP2Line1;
extern const QString AttrP1Line2;
extern const QString AttrP2Line2;
extern const QString AttrPShoulder;
extern const QString AttrPoint1;
extern const QString AttrPoint2;
extern const QString AttrPoint3;
extern const QString AttrPoint4;
extern const QString AttrKAsm1;  // TODO. Delete if minimal supported version is 0.2.7
extern const QString AttrKAsm2;  // TODO. Delete if minimal supported version is 0.2.7
extern const QString AttrKCurve; // TODO. Delete if minimal supported version is 0.2.7
extern const QString AttrDuplicate;
extern const QString AttrAScale;
extern const QString AttrPathPoint;
extern const QString AttrPSpline;
extern const QString AttrAxisP1;
extern const QString AttrAxisP2;
extern const QString AttrCurve;
extern const QString AttrCurve1;
extern const QString AttrCurve2;
extern const QString AttrLineColor;
extern const QString AttrColor;
extern const QString AttrPenStyle;
extern const QString AttrFirstArc;
extern const QString AttrSecondArc;
extern const QString AttrCrossPoint;
extern const QString AttrVCrossPoint;
extern const QString AttrHCrossPoint;
extern const QString AttrAxisType;
extern const QString AttrC1Center;
extern const QString AttrC2Center;
extern const QString AttrC1Radius;
extern const QString AttrC2Radius;
extern const QString AttrCCenter;
extern const QString AttrTangent;
extern const QString AttrCRadius;
extern const QString AttrArc;
extern const QString AttrSuffix;
extern const QString AttrItem;
extern const QString AttrIdObject;
extern const QString AttrInLayout;
extern const QString AttrForbidFlipping;
extern const QString AttrForceFlipping;
extern const QString AttrSymmetricalCopy;
extern const QString AttrFollowGrainline;
extern const QString AttrSewLineOnDrawing;
extern const QString AttrClosed;
extern const QString AttrShowLabel;
extern const QString AttrShowLabel1;
extern const QString AttrShowLabel2;
extern const QString AttrWidth;
extern const QString AttrHeight;
extern const QString AttrPlaceLabelType;
extern const QString AttrVersion;
extern const QString AttrFirstToContour;
extern const QString AttrLastToContour;
extern const QString AttrNotes;
extern const QString AttrAlias;
extern const QString AttrAlias1;
extern const QString AttrAlias2;
extern const QString AttrCurve1Alias1;
extern const QString AttrCurve1Alias2;
extern const QString AttrCurve2Alias1;
extern const QString AttrCurve2Alias2;
extern const QString AttrLayoutVersion;
extern const QString AttrKMVersion;
extern const QString AttrNotMirrored;

extern const QString TypeLineDefault;
extern const QString TypeLineNone;
extern const QString TypeLineLine;
extern const QString TypeLineDashLine;
extern const QString TypeLineDotLine;
extern const QString TypeLineDashDotLine;
extern const QString TypeLineDashDotDotLine;

auto StylesList() -> QStringList;
auto LineStyleToPenStyle(const QString &typeLine) -> Qt::PenStyle;
auto PenStyleToLineStyle(Qt::PenStyle penStyle) -> QString;
auto PenStylePic(QColor backgroundColor, QColor textColor, Qt::PenStyle style) -> QIcon;
auto LineStylesPics(QColor backgroundColor, QColor textColor) -> QMap<QString, QIcon>;
auto CurvePenStylesPics(QColor backgroundColor, QColor textColor) -> QMap<QString, QIcon>;

extern const QString ColorDefault;
extern const QString ColorBlack;
extern const QString ColorGreen;
extern const QString ColorBlue;
extern const QString ColorDarkRed;
extern const QString ColorDarkGreen;
extern const QString ColorDarkBlue;
extern const QString ColorYellow;
extern const QString ColorLightSalmon;
extern const QString ColorGoldenRod;
extern const QString ColorOrange;
extern const QString ColorDeepPink;
extern const QString ColorViolet;
extern const QString ColorDarkViolet;
extern const QString ColorMediumSeaGreen;
extern const QString ColorLime;
extern const QString ColorDeepSkyBlue;
extern const QString ColorCornFlowerBlue;

// variables name
// Hacks for avoiding the linker error "undefined reference to"
#define SPL_ "Spl_"
#define ARC_ "Arc_"
#define ELARC_ "ElArc_"

extern const QString measurement_;
extern const QString increment_;
extern const QString line_;
extern const QString angleLine_;
extern const QString arc_;
extern const QString elarc_;
extern const QString spl_;
extern const QString splPath;
extern const QString radius_V;
extern const QString radiusArc_;
extern const QString radius1ElArc_;
extern const QString radius2ElArc_;
extern const QString angle1_V;
extern const QString angle2_V;
extern const QString c1Length_V;
extern const QString c2Length_V;
extern const QString c1LengthSpl_;
extern const QString c2LengthSpl_;
extern const QString c1LengthSplPath;
extern const QString c2LengthSplPath;
extern const QString angle1Arc_;
extern const QString angle2Arc_;
extern const QString angle1ElArc_;
extern const QString angle2ElArc_;
extern const QString angle1Spl_;
extern const QString angle2Spl_;
extern const QString angle1SplPath;
extern const QString angle2SplPath;
extern const QString seg_;
extern const QString currentLength;
extern const QString currentSeamAllowance;
extern const QString rotation_V;
extern const QString rotationElArc_;
extern const QString pieceArea_;
extern const QString pieceSeamLineArea_;

auto BuilInVariables() -> QStringList;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct VLabelTemplateLine
{
    QString line;
    bool bold;
    bool italic;
    int alignment;
    int fontSizeIncrement;
};

struct VWatermarkData
{
    int opacity{20};             // NOLINT(misc-non-private-member-variables-in-classes)
    bool showText{true};         // NOLINT(misc-non-private-member-variables-in-classes)
    QString text{};              // NOLINT(misc-non-private-member-variables-in-classes)
    int textRotation{0};         // NOLINT(misc-non-private-member-variables-in-classes)
    QFont font{};                // NOLINT(misc-non-private-member-variables-in-classes)
    bool showImage{true};        // NOLINT(misc-non-private-member-variables-in-classes)
    QString path{};              // NOLINT(misc-non-private-member-variables-in-classes)
    int imageRotation{0};        // NOLINT(misc-non-private-member-variables-in-classes)
    bool grayscale{false};       // NOLINT(misc-non-private-member-variables-in-classes)
    bool invalidFile{false};     // NOLINT(misc-non-private-member-variables-in-classes)
    QColor textColor{Qt::black}; // NOLINT(misc-non-private-member-variables-in-classes)

    VWatermarkData() = default;
    VWatermarkData(VWatermarkData &&) noexcept = default;
    auto operator=(VWatermarkData &&) noexcept -> VWatermarkData & = default;
    VWatermarkData(const VWatermarkData &) = default;
    auto operator=(const VWatermarkData &) -> VWatermarkData & = default;
    ~VWatermarkData() = default;
};

QT_WARNING_POP

#endif // IFCDEF_H
