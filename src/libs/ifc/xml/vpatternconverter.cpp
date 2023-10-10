/************************************************************************
 **
 **  @file   vpatternconverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 12, 2014
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

#include "vpatternconverter.h"

#include <QDir>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QFileInfo>
#include <QGlobalStatic>
#include <QLatin1String>
#include <QList>
#include <QUuid>
#include <algorithm>

#include "../exception/vexception.h"
#include "../exception/vexceptionemptyparameter.h"
#include "../exception/vexceptionwrongid.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/def.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

class QDomElement;

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VPatternConverter::PatternMinVerStr = QStringLiteral("0.1.4");                     // NOLINT
const QString VPatternConverter::PatternMaxVerStr = QStringLiteral("0.9.3");                     // NOLINT
const QString VPatternConverter::CurrentSchema = QStringLiteral("://schema/pattern/v0.9.3.xsd"); // NOLINT

// VPatternConverter::PatternMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
// VPatternConverter::PatternMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// The list of all string we use for conversion
// Better to use global variables because repeating QStringLiteral blows up code size
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strUnit, ("unit"_L1))                               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strVersion, ("version"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strName, ("name"_L1))                               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strBase, ("base"_L1))                               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strFormula, ("formula"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strId, ("id"_L1))                                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strKGrowth, ("kgrowth"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strKSize, ("ksize"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPoint, ("point"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strLength, ("length"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAngle, ("angle"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strC1Radius, ("c1Radius"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strC2Radius, ("c2Radius"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCRadius, ("cRadius"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strArc, ("arc"_L1))                                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAngle1, ("angle1"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAngle2, ("angle2"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strRadius, ("radius"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPathPoint, ("pathPoint"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strKAsm1, ("kAsm1"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strKAsm2, ("kAsm2"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPath, ("path"_L1))                               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strType, ("type"_L1))                               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCutArc, ("cutArc"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSpline, ("spline"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSplinePath, ("splinePath"_L1))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCutSpline, ("cutSpline"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCutSplinePath, ("cutSplinePath"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strColor, ("color"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strMeasurements, ("measurements"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strIncrement, ("increment"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strIncrements, ("increments"_L1))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPreviewCalculations, ("previewCalculations"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strModeling, ("modeling"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strTools, ("tools"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strIdTool, ("idTool"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strIdObject, ("idObject"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strChildren, ("children"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strChild, ("child"_L1))                             // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPointOfIntersectionCurves, ("pointOfIntersectionCurves"_L1))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCurveIntersectAxis, ("curveIntersectAxis"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCurve, ("curve"_L1))                                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCurve1, ("curve1"_L1))                                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCurve2, ("curve2"_L1))                                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strModelingPath, ("modelingPath"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strModelingSpline, ("modelingSpline"_L1))                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPointFromArcAndTangent, ("pointFromArcAndTangent"_L1)) // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPointOfIntersectionArcs, ("pointOfIntersectionArcs"_L1))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strFirstArc, ("firstArc"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSecondArc, ("secondArc"_L1))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strDetail, ("detail"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSupplement, ("supplement"_L1)) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const QString, strClosed, ("closed"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strWidth, ("width"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strHeight, ("height"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strNode, ("node"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strNodes, ("nodes"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strData, ("data"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPatternInfo, ("patternInfo"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strGrainline, ("grainline"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strReverse, ("reverse"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strMx, ("mx"_L1))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strMy, ("my"_L1))                   // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const QString, strForbidFlipping, ("forbidFlipping"_L1)) // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const QString, strInLayout, ("inLayout"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSeamAllowance, ("seamAllowance"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strUUID, ("uuid"_L1))                   // NOLINT
// Q_GLOBAL_STATIC_WITH_ARGS(const QString, strNodeType, ("nodeType"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strDet, ("det"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strTypeObject, ("typeObject"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strReadOnly, ("readOnly"_L1))                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPatternLabel, ("patternLabel"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strImage, ("image"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAuthor, ("author"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strDescription, ("description"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strNotes, ("notes"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strGradation, ("gradation"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPatternName, ("patternName"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPatternNum, ("patternNumber"_L1))          // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCompanyName, ("company"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCustomerName, ("customer"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strLine, ("line"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strText, ("text"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strBold, ("bold"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strItalic, ("italic"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAlignment, ("alignment"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strFSIncrement, ("sfIncrement"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strShowDate, ("showDate"_L1))                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strShowMeasurements, ("showMeasurements"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSize, ("size"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strMCP, ("mcp"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strLetter, ("letter"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strMaterial, ("material"_L1))                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strUserDefined, ("userDef"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPlacement, ("placement"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCutNumber, ("cutNumber"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strQuantity, ("quantity"_L1))                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strExtension, ("extension"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strContentType, ("contentType"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strFirstToCountour, ("firstToCountour"_L1))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strFirstToContour, ("firstToContour"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strLastToCountour, ("lastToCountour"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strLastToContour, ("lastToContour"_L1))       // NOLINT

QT_WARNING_POP
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
VPatternConverter::VPatternConverter(const QString &fileName)
  : VAbstractConverter(fileName)
{
    m_ver = GetFormatVersion(GetFormatVersionStr());
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::XSDSchemas() -> QHash<unsigned int, QString>
{
    static const auto schemas = QHash<unsigned, QString>{
        std::make_pair(FormatVersion(0, 1, 4), QStringLiteral("://schema/pattern/v0.1.4.xsd")),
        std::make_pair(FormatVersion(0, 2, 0), QStringLiteral("://schema/pattern/v0.2.0.xsd")),
        std::make_pair(FormatVersion(0, 2, 1), QStringLiteral("://schema/pattern/v0.2.1.xsd")),
        std::make_pair(FormatVersion(0, 2, 2), QStringLiteral("://schema/pattern/v0.2.2.xsd")),
        std::make_pair(FormatVersion(0, 2, 3), QStringLiteral("://schema/pattern/v0.2.3.xsd")),
        std::make_pair(FormatVersion(0, 2, 4), QStringLiteral("://schema/pattern/v0.2.4.xsd")),
        std::make_pair(FormatVersion(0, 2, 5), QStringLiteral("://schema/pattern/v0.2.5.xsd")),
        std::make_pair(FormatVersion(0, 2, 6), QStringLiteral("://schema/pattern/v0.2.6.xsd")),
        std::make_pair(FormatVersion(0, 2, 7), QStringLiteral("://schema/pattern/v0.2.7.xsd")),
        std::make_pair(FormatVersion(0, 3, 0), QStringLiteral("://schema/pattern/v0.3.0.xsd")),
        std::make_pair(FormatVersion(0, 3, 1), QStringLiteral("://schema/pattern/v0.3.1.xsd")),
        std::make_pair(FormatVersion(0, 3, 2), QStringLiteral("://schema/pattern/v0.3.2.xsd")),
        std::make_pair(FormatVersion(0, 3, 3), QStringLiteral("://schema/pattern/v0.3.3.xsd")),
        std::make_pair(FormatVersion(0, 3, 4), QStringLiteral("://schema/pattern/v0.3.4.xsd")),
        std::make_pair(FormatVersion(0, 3, 5), QStringLiteral("://schema/pattern/v0.3.5.xsd")),
        std::make_pair(FormatVersion(0, 3, 6), QStringLiteral("://schema/pattern/v0.3.6.xsd")),
        std::make_pair(FormatVersion(0, 3, 7), QStringLiteral("://schema/pattern/v0.3.7.xsd")),
        std::make_pair(FormatVersion(0, 3, 8), QStringLiteral("://schema/pattern/v0.3.8.xsd")),
        std::make_pair(FormatVersion(0, 3, 9), QStringLiteral("://schema/pattern/v0.3.9.xsd")),
        std::make_pair(FormatVersion(0, 4, 0), QStringLiteral("://schema/pattern/v0.4.0.xsd")),
        std::make_pair(FormatVersion(0, 4, 1), QStringLiteral("://schema/pattern/v0.4.1.xsd")),
        std::make_pair(FormatVersion(0, 4, 2), QStringLiteral("://schema/pattern/v0.4.2.xsd")),
        std::make_pair(FormatVersion(0, 4, 3), QStringLiteral("://schema/pattern/v0.4.3.xsd")),
        std::make_pair(FormatVersion(0, 4, 4), QStringLiteral("://schema/pattern/v0.4.4.xsd")),
        std::make_pair(FormatVersion(0, 4, 5), QStringLiteral("://schema/pattern/v0.4.5.xsd")),
        std::make_pair(FormatVersion(0, 4, 6), QStringLiteral("://schema/pattern/v0.4.6.xsd")),
        std::make_pair(FormatVersion(0, 4, 7), QStringLiteral("://schema/pattern/v0.4.7.xsd")),
        std::make_pair(FormatVersion(0, 4, 8), QStringLiteral("://schema/pattern/v0.4.8.xsd")),
        std::make_pair(FormatVersion(0, 5, 0), QStringLiteral("://schema/pattern/v0.5.0.xsd")),
        std::make_pair(FormatVersion(0, 5, 1), QStringLiteral("://schema/pattern/v0.5.1.xsd")),
        std::make_pair(FormatVersion(0, 6, 0), QStringLiteral("://schema/pattern/v0.6.0.xsd")),
        std::make_pair(FormatVersion(0, 6, 1), QStringLiteral("://schema/pattern/v0.6.1.xsd")),
        std::make_pair(FormatVersion(0, 6, 2), QStringLiteral("://schema/pattern/v0.6.2.xsd")),
        std::make_pair(FormatVersion(0, 6, 3), QStringLiteral("://schema/pattern/v0.6.3.xsd")),
        std::make_pair(FormatVersion(0, 6, 4), QStringLiteral("://schema/pattern/v0.6.4.xsd")),
        std::make_pair(FormatVersion(0, 6, 5), QStringLiteral("://schema/pattern/v0.6.5.xsd")),
        std::make_pair(FormatVersion(0, 6, 6), QStringLiteral("://schema/pattern/v0.6.6.xsd")),
        std::make_pair(FormatVersion(0, 7, 0), QStringLiteral("://schema/pattern/v0.7.0.xsd")),
        std::make_pair(FormatVersion(0, 7, 1), QStringLiteral("://schema/pattern/v0.7.1.xsd")),
        std::make_pair(FormatVersion(0, 7, 2), QStringLiteral("://schema/pattern/v0.7.2.xsd")),
        std::make_pair(FormatVersion(0, 7, 3), QStringLiteral("://schema/pattern/v0.7.3.xsd")),
        std::make_pair(FormatVersion(0, 7, 4), QStringLiteral("://schema/pattern/v0.7.4.xsd")),
        std::make_pair(FormatVersion(0, 7, 5), QStringLiteral("://schema/pattern/v0.7.5.xsd")),
        std::make_pair(FormatVersion(0, 7, 6), QStringLiteral("://schema/pattern/v0.7.6.xsd")),
        std::make_pair(FormatVersion(0, 7, 7), QStringLiteral("://schema/pattern/v0.7.7.xsd")),
        std::make_pair(FormatVersion(0, 7, 8), QStringLiteral("://schema/pattern/v0.7.8.xsd")),
        std::make_pair(FormatVersion(0, 7, 9), QStringLiteral("://schema/pattern/v0.7.9.xsd")),
        std::make_pair(FormatVersion(0, 7, 10), QStringLiteral("://schema/pattern/v0.7.10.xsd")),
        std::make_pair(FormatVersion(0, 7, 11), QStringLiteral("://schema/pattern/v0.7.11.xsd")),
        std::make_pair(FormatVersion(0, 7, 12), QStringLiteral("://schema/pattern/v0.7.12.xsd")),
        std::make_pair(FormatVersion(0, 7, 13), QStringLiteral("://schema/pattern/v0.7.13.xsd")),
        std::make_pair(FormatVersion(0, 8, 0), QStringLiteral("://schema/pattern/v0.8.0.xsd")),
        std::make_pair(FormatVersion(0, 8, 1), QStringLiteral("://schema/pattern/v0.8.1.xsd")),
        std::make_pair(FormatVersion(0, 8, 2), QStringLiteral("://schema/pattern/v0.8.2.xsd")),
        std::make_pair(FormatVersion(0, 8, 3), QStringLiteral("://schema/pattern/v0.8.3.xsd")),
        std::make_pair(FormatVersion(0, 8, 4), QStringLiteral("://schema/pattern/v0.8.4.xsd")),
        std::make_pair(FormatVersion(0, 8, 5), QStringLiteral("://schema/pattern/v0.8.5.xsd")),
        std::make_pair(FormatVersion(0, 8, 6), QStringLiteral("://schema/pattern/v0.8.6.xsd")),
        std::make_pair(FormatVersion(0, 8, 7), QStringLiteral("://schema/pattern/v0.8.7.xsd")),
        std::make_pair(FormatVersion(0, 8, 8), QStringLiteral("://schema/pattern/v0.8.8.xsd")),
        std::make_pair(FormatVersion(0, 8, 9), QStringLiteral("://schema/pattern/v0.8.9.xsd")),
        std::make_pair(FormatVersion(0, 8, 10), QStringLiteral("://schema/pattern/v0.8.10.xsd")),
        std::make_pair(FormatVersion(0, 8, 11), QStringLiteral("://schema/pattern/v0.8.11.xsd")),
        std::make_pair(FormatVersion(0, 8, 12), QStringLiteral("://schema/pattern/v0.8.12.xsd")),
        std::make_pair(FormatVersion(0, 8, 13), QStringLiteral("://schema/pattern/v0.8.13.xsd")),
        std::make_pair(FormatVersion(0, 9, 0), QStringLiteral("://schema/pattern/v0.9.0.xsd")),
        std::make_pair(FormatVersion(0, 9, 1), QStringLiteral("://schema/pattern/v0.9.1.xsd")),
        std::make_pair(FormatVersion(0, 9, 2), QStringLiteral("://schema/pattern/v0.9.2.xsd")),
        std::make_pair(FormatVersion(0, 9, 3), CurrentSchema)};

    return schemas;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::Save()
{
    try
    {
        TestUniqueId();
    }
    catch (const VExceptionWrongId &e)
    {
        Q_UNUSED(e)
        throw VException(tr("Error no unique id."));
    }

    VAbstractConverter::Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (FormatVersion(0, 1, 4)):
            ToV0_2_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 2, 0)):
            ToV0_2_1();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 2, 1)):
        case (FormatVersion(0, 2, 2)):
        case (FormatVersion(0, 2, 3)):
            ToV0_2_4();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 2, 4)):
        case (FormatVersion(0, 2, 5)):
        case (FormatVersion(0, 2, 6)):
        case (FormatVersion(0, 2, 7)):
            ToV0_3_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 3, 0)):
            ToV0_3_1();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 3, 1)):
        case (FormatVersion(0, 3, 2)):
        case (FormatVersion(0, 3, 3)):
        case (FormatVersion(0, 3, 4)):
        case (FormatVersion(0, 3, 5)):
        case (FormatVersion(0, 3, 6)):
        case (FormatVersion(0, 3, 7)):
        case (FormatVersion(0, 3, 8)):
        case (FormatVersion(0, 3, 9)):
            ToV0_4_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 4, 0)):
        case (FormatVersion(0, 4, 1)):
        case (FormatVersion(0, 4, 2)):
        case (FormatVersion(0, 4, 3)):
            ToV0_4_4();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 4, 4)):
        case (FormatVersion(0, 4, 5)):
        case (FormatVersion(0, 4, 6)):
        case (FormatVersion(0, 4, 7)):
        case (FormatVersion(0, 4, 8)):
        case (FormatVersion(0, 5, 0)):
        case (FormatVersion(0, 5, 1)):
            ToV0_6_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 6, 0)):
        case (FormatVersion(0, 6, 1)):
            ToV0_6_2();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 6, 2)):
        case (FormatVersion(0, 6, 3)):
        case (FormatVersion(0, 6, 4)):
        case (FormatVersion(0, 6, 5)):
        case (FormatVersion(0, 6, 6)):
        case (FormatVersion(0, 7, 0)):
        case (FormatVersion(0, 7, 1)):
        case (FormatVersion(0, 7, 2)):
        case (FormatVersion(0, 7, 3)):
        case (FormatVersion(0, 7, 4)):
        case (FormatVersion(0, 7, 5)):
        case (FormatVersion(0, 7, 6)):
        case (FormatVersion(0, 7, 7)):
        case (FormatVersion(0, 7, 8)):
        case (FormatVersion(0, 7, 9)):
        case (FormatVersion(0, 7, 10)):
        case (FormatVersion(0, 7, 11)):
        case (FormatVersion(0, 7, 12)):
        case (FormatVersion(0, 7, 13)):
        case (FormatVersion(0, 8, 0)):
        case (FormatVersion(0, 8, 1)):
        case (FormatVersion(0, 8, 2)):
        case (FormatVersion(0, 8, 3)):
        case (FormatVersion(0, 8, 4)):
        case (FormatVersion(0, 8, 5)):
        case (FormatVersion(0, 8, 6)):
        case (FormatVersion(0, 8, 7)):
        case (FormatVersion(0, 8, 8)):
            ToV0_8_8();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 8, 9)):
        case (FormatVersion(0, 8, 10)):
        case (FormatVersion(0, 8, 11)):
        case (FormatVersion(0, 8, 12)):
        case (FormatVersion(0, 8, 13)):
            ToV0_9_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 9, 0)):
            ToV0_9_1();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 9, 1)):
            ToV0_9_2();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 9, 2)):
            ToV0_9_3();
            ValidateXML(CurrentSchema);
            Q_FALLTHROUGH();
        case (FormatVersion(0, 9, 3)):
            break;
        default:
            InvalidVersion(m_ver);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(PatternMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::IsReadOnly() const -> bool
{
    // Check if attribute readOnly was not changed in file format
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMaxVer == FormatVersion(0, 9, 3), "Check attribute readOnly.");

    // Possibly in future attribute readOnly will change position etc.
    // For now position is the same for all supported format versions.
    // But don't forget to keep all versions of attribute until we support that format versions

    const QDomElement pattern = documentElement();

    if (pattern.isNull())
    {
        return false;
    }

    return GetParametrBool(pattern, *strReadOnly, falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::Schemas() const -> QHash<unsigned int, QString>
{
    return XSDSchemas();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.0"));
    TagUnitToV0_2_0();
    TagIncrementToV0_2_0();
    ConvertMeasurementsToV0_2_0();
    TagMeasurementsToV0_2_0(); // Alwayse last!!!
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_2_1()
{
    // TODO. Delete if minimal supported version is 0.2.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 1), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.2.1"));
    ConvertMeasurementsToV0_2_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_2_4()
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 4), "Time to refactor the code.");

    FixToolUnionToV0_2_4();
    SetVersion(QStringLiteral("0.2.4"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 3, 0), "Time to refactor the code.");

    // Cutting path do not create anymore subpaths
    FixCutPoint();
    FixCutPoint();
    SetVersion(QStringLiteral("0.3.0"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 3, 1), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.1"));
    RemoveColorToolCutV0_3_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.0"));
    TagRemoveAttributeTypeObjectInV0_4_0();
    TagDetailToV0_4_0();
    TagUnionDetailsToV0_4_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_4_4()
{
    // TODO. Delete if minimal supported version is 0.4.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 4), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.4"));
    LabelTagToV0_4_4(*strData);
    LabelTagToV0_4_4(*strPatternInfo);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_6_0()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.0"));
    QDomElement label = AddTagPatternLabelV0_5_1();
    PortPatternLabeltoV0_6_0(label);
    PortPieceLabelstoV0_6_0();
    RemoveUnusedTagsV0_6_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_6_2()
{
    // TODO. Delete if minimal supported version is 0.6.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 2), "Time to refactor the code.");
    SetVersion(QStringLiteral("0.6.2"));
    AddTagPreviewCalculationsV0_6_2();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_8_8()
{
    // TODO. Delete if minimal supported version is 0.8.8
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 8, 8), "Time to refactor the code.");
    SetVersion(QStringLiteral("0.8.8"));
    RemoveGradationV0_8_8();
    AddPieceUUIDV0_8_8();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_9_0()
{
    // TODO. Delete if minimal supported version is 0.9.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 0), "Time to refactor the code.");

    ConvertImageToV0_9_0();

    SetVersion(QStringLiteral("0.9.0"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_9_1()
{
    // TODO. Delete if minimal supported version is 0.9.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 1), "Time to refactor the code.");

    ConvertMeasurementsPathToV0_9_1();
    SetVersion(QStringLiteral("0.9.1"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_9_2()
{
    // TODO. Delete if minimal supported version is 0.9.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 2), "Time to refactor the code.");

    ConvertPathAttributesToV0_9_2();
    SetVersion(QStringLiteral("0.9.2"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ToV0_9_3()
{
    // TODO. Delete if minimal supported version is 0.9.3
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 3), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.9.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagUnitToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QDomElement patternElement = documentElement();
    patternElement.insertAfter(CreateElementWithText(*strUnit, MUnitV0_1_4()),
                               patternElement.firstChildElement(*strVersion));
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagIncrementToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    const QSet<QString> names = FixIncrementsToV0_2_0();

    FixPointExpressionsToV0_2_0(names);
    FixArcExpressionsToV0_2_0(names);
    FixPathPointExpressionsToV0_2_0(names);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertMeasurementsToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_2_0();
    ConvertPointExpressionsToV0_2_0(names);
    ConvertArcExpressionsToV0_2_0(names);
    ConvertPathPointExpressionsToV0_2_0(names);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::FixIncrementsToV0_2_0() -> QSet<QString>
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QSet<QString> names;
    const QDomElement incr = TagIncrementsV0_1_4();
    QDomNode domNode = incr.firstChild();
    while (not domNode.isNull())
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            if (not domElement.isNull())
            {
                if (domElement.tagName() == *strIncrement)
                {
                    try
                    {
                        const QString name = GetParametrString(domElement, *strName);
                        names.insert(name);
                        domElement.setAttribute(*strName, '#'_L1 + name);

                        const QString base = GetParametrString(domElement, *strBase);
                        domElement.setAttribute(*strFormula, base);
                    }
                    catch (VExceptionEmptyParameter &e)
                    {
                        VException excep("Can't get increment.");
                        excep.AddMoreInformation(e.ErrorMessage());
                        throw excep;
                    }
                    domElement.removeAttribute(*strId);
                    domElement.removeAttribute(*strKGrowth);
                    domElement.removeAttribute(*strKSize);
                    domElement.removeAttribute(*strBase);
                }
            }
        }
        domNode = domNode.nextSibling();
    }
    return names;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixPointExpressionsToV0_2_0(const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(*strPoint);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, *strLength);
            dom.setAttribute(*strLength, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strAngle);
            dom.setAttribute(*strAngle, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
        try
        {
            formula = GetParametrString(dom, *strC1Radius);
            dom.setAttribute(*strC1Radius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strC2Radius);
            dom.setAttribute(*strC2Radius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strCRadius);
            dom.setAttribute(*strCRadius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixArcExpressionsToV0_2_0(const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(*strArc);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, *strAngle1);
            dom.setAttribute(*strAngle1, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strAngle2);
            dom.setAttribute(*strAngle2, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strRadius);
            dom.setAttribute(*strRadius, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strLength);
            dom.setAttribute(*strLength, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixPathPointExpressionsToV0_2_0(const QSet<QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(*strPathPoint);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, *strKAsm1);
            dom.setAttribute(*strKAsm1, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strKAsm2);
            dom.setAttribute(*strKAsm2, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strAngle);
            dom.setAttribute(*strAngle, FixIncrementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertPointExpressionsToV0_2_0(const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(*strPoint);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, *strLength);
            dom.setAttribute(*strLength, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strAngle);
            dom.setAttribute(*strAngle, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
        try
        {
            formula = GetParametrString(dom, *strC1Radius);
            dom.setAttribute(*strC1Radius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strC2Radius);
            dom.setAttribute(*strC2Radius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strCRadius);
            dom.setAttribute(*strCRadius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertArcExpressionsToV0_2_0(const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(*strArc);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, *strAngle1);
            dom.setAttribute(*strAngle1, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strAngle2);
            dom.setAttribute(*strAngle2, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strRadius);
            dom.setAttribute(*strRadius, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strLength);
            dom.setAttribute(*strLength, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertPathPointExpressionsToV0_2_0(const QMap<QString, QString> &names)
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QString formula;
    const QDomNodeList list = elementsByTagName(*strPathPoint);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        try
        {
            formula = GetParametrString(dom, *strKAsm1);
            dom.setAttribute(*strKAsm1, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strKAsm2);
            dom.setAttribute(*strKAsm2, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }

        try
        {
            formula = GetParametrString(dom, *strAngle);
            dom.setAttribute(*strAngle, FixMeasurementInFormulaToV0_2_0(formula, names));
        }
        catch (VExceptionEmptyParameter &e)
        {
            Q_UNUSED(e)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::FixMeasurementInFormulaToV0_2_0(const QString &formula, const QMap<QString, QString> &names)
    -> QString
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, false, false)); // Eval formula
    QMap<vsizetype, QString> tokens = cal->GetTokens(); // Tokens (variables, measurements)
    cal.reset();

    QList<vsizetype> tKeys = tokens.keys(); // Take all tokens positions
    QList<QString> tValues = tokens.values();

    QString newFormula = formula; // Local copy for making changes
    for (int i = 0; i < tValues.size(); ++i)
    {
        if (not names.contains(tValues.at(i)))
        {
            continue;
        }

        vsizetype bias = 0;
        Replace(newFormula, names.value(tValues.at(i)), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        { // Translated token has different length than original. Position next tokens need to be corrected.
            CorrectionsPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::FixIncrementInFormulaToV0_2_0(const QString &formula, const QSet<QString> &names) -> QString
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    qmu::QmuTokenParser *cal = new qmu::QmuTokenParser(formula, false, false); // Eval formula
    QMap<vsizetype, QString> tokens = cal->GetTokens();                        // Tokens (variables, measurements)
    delete cal;

    QList<vsizetype> tKeys = tokens.keys(); // Take all tokens positions
    QList<QString> tValues = tokens.values();

    QString newFormula = formula; // Local copy for making changes
    for (vsizetype i = 0; i < tValues.size(); ++i)
    {
        if (not names.contains(tValues.at(i)))
        {
            continue;
        }

        vsizetype bias = 0;
        Replace(newFormula, "#" + tValues.at(i), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        { // Translated token has different length than original. Position next tokens need to be corrected.
            CorrectionsPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagMeasurementsToV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    QDomElement ms = TagMeasurementsV0_1_4();
    const QString path = GetParametrString(ms, *strPath);

    ms.removeAttribute(*strUnit);
    ms.removeAttribute(*strType);
    ms.removeAttribute(*strPath);

    ms.appendChild(createTextNode(QFileInfo(m_convertedFileName).absoluteDir().relativeFilePath(path)));
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertMeasurementsToV0_2_1()
{
    // TODO. Delete if minimal supported version is 0.2.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 1), "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_2_1();

    // Structure did not change. We can use the same code.
    ConvertPointExpressionsToV0_2_0(names);
    ConvertArcExpressionsToV0_2_0(names);
    ConvertPathPointExpressionsToV0_2_0(names);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveColorToolCutV0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 3, 1), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strPoint);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(*strType);
            if (type == *strCutArc || type == *strCutSpline || type == *strCutSplinePath)
            {
                element.removeAttribute(*strColor);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::MUnitV0_1_4() const -> QString
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    const QDomElement element = TagMeasurementsV0_1_4();
    try
    {
        return GetParametrString(element, *strUnit);
    }
    catch (VExceptionEmptyParameter &e)
    {
        VException excep("Can't get unit.");
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::TagMeasurementsV0_1_4() const -> QDomElement
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strMeasurements);
    const QDomElement element = list.at(0).toElement();
    if (not element.isElement())
    {
        VException excep("Can't get tag measurements.");
        throw excep;
    }
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::TagIncrementsV0_1_4() const -> QDomElement
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strIncrements);
    const QDomElement element = list.at(0).toElement();
    if (not element.isElement())
    {
        VException excep("Can't get tag measurements.");
        throw excep;
    }
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixToolUnionToV0_2_4()
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 4), "Time to refactor the code.");

    QDomElement root = documentElement();
    const QDomNodeList modelings = root.elementsByTagName(*strModeling);
    for (int i = 0; i < modelings.size(); ++i)
    {
        ParseModelingToV0_2_4(modelings.at(i).toElement());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ParseModelingToV0_2_4(const QDomElement &modeling)
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 4), "Time to refactor the code.");

    QDomElement node = modeling.firstChild().toElement();
    while (not node.isNull())
    {
        if (node.tagName() == *strTools)
        {
            const quint32 toolId = node.attribute(*strId).toUInt();
            QVector<quint32> children;
            QDomElement childNode = node.nextSibling().toElement();
            while (not childNode.isNull())
            {
                if (childNode.hasAttribute(*strIdTool) && childNode.attribute(*strIdTool).toUInt() == toolId)
                {
                    children.append(childNode.attribute(*strIdObject).toUInt());
                }
                else
                {
                    break;
                }
                childNode = childNode.nextSibling().toElement();
            }

            if (not children.isEmpty())
            {
                SaveChildrenToolUnionToV0_2_4(toolId, children);
            }
            node = childNode;
            continue;
        }
        node = node.nextSibling().toElement();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::SaveChildrenToolUnionToV0_2_4(quint32 id, const QVector<quint32> &children)
{
    // TODO. Delete if minimal supported version is 0.2.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 4), "Time to refactor the code.");

    QDomElement toolUnion = elementById(id);
    if (toolUnion.isNull())
    {
        return;
    }

    QDomElement tagChildren = createElement(*strChildren);

    for (auto child : children)
    {
        tagChildren.appendChild(CreateElementWithText(*strChild, QString().setNum(child)));
    }

    toolUnion.appendChild(tagChildren);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::OldNamesToNewNames_InV0_2_0() -> QMap<QString, QString>
{
    // TODO. Delete if minimal supported version is 0.2.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 0), "Time to refactor the code.");

    // old name, new name
    QMap<QString, QString> names;

    names.insert(QStringLiteral("cervicale_height"), QStringLiteral("height_neck_back"));
    names.insert(QStringLiteral("height_scapular_point"), QStringLiteral("height_scapula"));
    names.insert(QStringLiteral("height_back_angle_axilla"), QStringLiteral("height_armpit"));
    names.insert(QStringLiteral("waist_height"), QStringLiteral("height_waist_side"));
    names.insert(QStringLiteral("hip_height"), QStringLiteral("height_hip"));
    names.insert(QStringLiteral("knee_height"), QStringLiteral("height_knee"));
    names.insert(QStringLiteral("ankle_height"), QStringLiteral("height_ankle"));
    names.insert(QStringLiteral("high_hip_height"), QStringLiteral("height_highhip"));
    names.insert(QStringLiteral("front_waist_to_floor"), QStringLiteral("height_waist_front"));
    names.insert(QStringLiteral("height_nipple_point"), QStringLiteral("height_bustpoint"));

    QString name = QStringLiteral("height_shoulder_tip");
    names.insert(QStringLiteral("shoulder_height"), name);
    names.insert(QStringLiteral("height_shoulder_point"), name);

    name = QStringLiteral("height_neck_front");
    names.insert(QStringLiteral("height_clavicular_point"), name);
    names.insert(QStringLiteral("height_front_neck_base_point"), name);

    names.insert(QStringLiteral("height_base_neck_side_point"), QStringLiteral("height_neck_side"));

    name = QStringLiteral("height_neck_back_to_knee");
    names.insert(QStringLiteral("neck_to_knee_point"), name);
    names.insert(QStringLiteral("cervicale_to_knee_height"), name);

    names.insert(QStringLiteral("waist_to_knee_height"), QStringLiteral("height_waist_side_to_knee"));
    names.insert(QStringLiteral("waist_to_hip_height"), QStringLiteral("height_waist_side_to_hip"));
    names.insert(QStringLiteral("body_position"), QStringLiteral("indent_neck_back"));

    name = QStringLiteral("neck_mid_circ");
    names.insert(QStringLiteral("half_girth_neck_for_shirts"), name);
    names.insert(QStringLiteral("mid_neck_girth"), name);

    names.insert(QStringLiteral("neck_base_girth"), QStringLiteral("neck_circ"));
    names.insert(QStringLiteral("upper_chest_girth"), QStringLiteral("highbust_circ"));
    names.insert(QStringLiteral("bust_girth"), QStringLiteral("bust_circ"));
    names.insert(QStringLiteral("under_bust_girth"), QStringLiteral("lowbust_circ"));
    names.insert(QStringLiteral("waist_girth"), QStringLiteral("waist_circ"));
    names.insert(QStringLiteral("high_hip_girth"), QStringLiteral("highhip_circ"));
    names.insert(QStringLiteral("hips_excluding_protruding_abdomen"), QStringLiteral("hip_circ"));
    names.insert(QStringLiteral("hip_girth"), QStringLiteral("hip_circ_with_abdomen"));

    name = QStringLiteral("neck_arc_f");
    names.insert(QStringLiteral("half_girth_neck"), name);
    names.insert(QStringLiteral("front_neck_arc"), name);

    name = QStringLiteral("highbust_arc_f");
    names.insert(QStringLiteral("half_girth_chest_first"), name);
    names.insert(QStringLiteral("front_upper_chest_arc"), name);

    names.insert(QStringLiteral("half_girth_chest_second"), QStringLiteral("bust_arc_f"));
    names.insert(QStringLiteral("half_girth_chest_third"), QStringLiteral("lowbust_arc_f"));

    name = QStringLiteral("waist_arc_f");
    names.insert(QStringLiteral("half_girth_waist"), name);
    names.insert(QStringLiteral("front_waist_arc"), name);

    names.insert(QStringLiteral("front_upper_hip_arc"), QStringLiteral("highhip_arc_f"));

    name = QStringLiteral("hip_arc_f");
    names.insert(QStringLiteral("half_girth_hips_excluding_protruding_abdomen"), name);
    names.insert(QStringLiteral("front_hip_arc"), name);

    names.insert(QStringLiteral("back_neck_arc"), QStringLiteral("neck_arc_b"));
    names.insert(QStringLiteral("back_upper_chest_arc"), QStringLiteral("highbust_arc_b"));
    names.insert(QStringLiteral("back_waist_arc"), QStringLiteral("waist_arc_b"));
    names.insert(QStringLiteral("back_upper_hip_arc"), QStringLiteral("highhip_arc_b"));
    names.insert(QStringLiteral("back_hip_arc"), QStringLiteral("hip_arc_b"));
    names.insert(QStringLiteral("half_girth_hips_considering_protruding_abdomen"),
                 QStringLiteral("hip_with_abdomen_arc_f"));
    names.insert(QStringLiteral("shoulder_girth"), QStringLiteral("body_armfold_circ"));
    names.insert(QStringLiteral("trunk_length"), QStringLiteral("body_torso_circ"));
    names.insert(QStringLiteral("front_waist_length"), QStringLiteral("neck_front_to_waist_f"));
    names.insert(QStringLiteral("center_front_waist_length"), QStringLiteral("neck_front_to_waist_flat_f"));
    names.insert(QStringLiteral("side_waist_length"), QStringLiteral("armpit_to_waist_side"));
    names.insert(QStringLiteral("waist_to_neck_side"), QStringLiteral("neck_side_to_waist_b"));

    name = QStringLiteral("neck_side_to_waist_f");
    names.insert(QStringLiteral("neck_to_front_waist_line"), name);
    names.insert(QStringLiteral("front_shoulder_to_waist_length"), name);

    names.insert(QStringLiteral("back_shoulder_to_waist_length"), QStringLiteral("neck_side_to_waist_b"));
    names.insert(QStringLiteral("center_back_waist_length"), QStringLiteral("neck_back_to_waist_b"));

    name = QStringLiteral("neck_front_to_highbust_f");
    names.insert(QStringLiteral("neck_to_first_line_chest_circumference"), name);
    names.insert(QStringLiteral("front_neck_to_upper_chest_height"), name);

    names.insert(QStringLiteral("front_neck_to_bust_height"), QStringLiteral("neck_front_to_bust_f"));
    names.insert(QStringLiteral("front_waist_to_upper_chest"), QStringLiteral("highbust_to_waist_f"));
    names.insert(QStringLiteral("front_waist_to_lower_breast"), QStringLiteral("lowbust_to_waist_f"));
    names.insert(QStringLiteral("neck_to_back_line_chest_circumference"), QStringLiteral("neck_back_to_highbust_b"));
    names.insert(QStringLiteral("depth_waist_first"), QStringLiteral("waist_to_highhip_f"));
    names.insert(QStringLiteral("depth_waist_second"), QStringLiteral("waist_to_hip_f"));
    names.insert(QStringLiteral("shoulder_slope_degrees"), QStringLiteral("shoulder_slope_neck_side_angle"));
    names.insert(QStringLiteral("shoulder_drop"), QStringLiteral("shoulder_slope_neck_side_length"));
    names.insert(QStringLiteral("across_front_shoulder_width"), QStringLiteral("shoulder_tip_to_shoulder_tip_f"));
    names.insert(QStringLiteral("upper_front_chest_width"), QStringLiteral("across_chest_f"));
    names.insert(QStringLiteral("chest_width"), QStringLiteral("across_chest_f"));
    names.insert(QStringLiteral("front_chest_width"), QStringLiteral("armfold_to_armfold_f"));

    name = QStringLiteral("shoulder_tip_to_shoulder_tip_b");
    names.insert(QStringLiteral("arc_behind_shoulder_girdle"), name);
    names.insert(QStringLiteral("across_back_shoulder_width"), name);

    names.insert(QStringLiteral("upper_back_width"), QStringLiteral("across_back_b"));
    names.insert(QStringLiteral("back_width"), QStringLiteral("armfold_to_armfold_b"));
    names.insert(QStringLiteral("neck_transverse_diameter"), QStringLiteral("neck_width"));
    names.insert(QStringLiteral("bustpoint_to_bustpoint"), QStringLiteral("bustpoint_to_bustpoint"));
    names.insert(QStringLiteral("neck_to_bustpoint"), QStringLiteral("bustpoint_to_neck_side"));
    names.insert(QStringLiteral("halter_bustpoint_to_bustpoint"), QStringLiteral("bustpoint_to_bustpoint_halter"));

    name = QStringLiteral("shoulder_tip_to_waist_front");
    names.insert(QStringLiteral("front_slash_shoulder_height"), name);
    names.insert(QStringLiteral("front_shoulder_slope_length"), name);

    names.insert(QStringLiteral("front_waist_slope"), QStringLiteral("neck_front_to_waist_side"));

    name = QStringLiteral("neck_side_to_armfold_f");
    names.insert(QStringLiteral("height_armhole_slash"), name);
    names.insert(QStringLiteral("chest_slope"), name);

    name = QStringLiteral("shoulder_tip_to_waist_back");
    names.insert(QStringLiteral("slash_shoulder_height"), name);
    names.insert(QStringLiteral("back_shoulder_slope_length"), name);

    names.insert(QStringLiteral("back_waist_slope"), QStringLiteral("neck_back_to_waist_side"));
    names.insert(QStringLiteral("back_slope"), QStringLiteral("neck_side_to_armfold_b"));
    names.insert(QStringLiteral("arm_length"), QStringLiteral("arm_shoulder_tip_to_wrist"));
    names.insert(QStringLiteral("shoulder_to_elbow_length"), QStringLiteral("arm_shoulder_tip_to_elbow"));
    names.insert(QStringLiteral("underarm_length"), QStringLiteral("arm_armpit_to_wrist"));
    names.insert(QStringLiteral("upper_arm_girth"), QStringLiteral("arm_upper_circ"));
    names.insert(QStringLiteral("wrist_girth"), QStringLiteral("arm_wrist_circ"));
    names.insert(QStringLiteral("armscye_girth"), QStringLiteral("armscye_circ"));
    names.insert(QStringLiteral("anteroposterior_diameter_hands"), QStringLiteral("armscye_width"));
    names.insert(QStringLiteral("neck_to_third_finger"), QStringLiteral("arm_neck_side_to_finger_tip"));
    names.insert(QStringLiteral("neck_to_radial_point"), QStringLiteral("arm_neck_side_to_outer_elbow"));
    names.insert(QStringLiteral("shoulder_and_arm_length"), QStringLiteral("arm_neck_side_to_wrist"));
    names.insert(QStringLiteral("crotch_height"), QStringLiteral("leg_crotch_to_floor"));
    names.insert(QStringLiteral("side_waist_to_floor"), QStringLiteral("leg_waist_side_to_floor"));
    names.insert(QStringLiteral("waist_to_knee"), QStringLiteral("leg_waist_side_to_knee"));
    names.insert(QStringLiteral("thigh_girth"), QStringLiteral("leg_thigh_upper_circ"));
    names.insert(QStringLiteral("mid_thigh_girth"), QStringLiteral("leg_thigh_mid_circ"));
    names.insert(QStringLiteral("knee_girth"), QStringLiteral("leg_knee_circ"));
    names.insert(QStringLiteral("calf_girth"), QStringLiteral("leg_calf_circ"));
    names.insert(QStringLiteral("ankle_girth"), QStringLiteral("leg_ankle_circ"));
    names.insert(QStringLiteral("girth_knee_flexed_feet"), QStringLiteral("leg_knee_circ_bent"));
    names.insert(QStringLiteral("arc_through_groin_area"), QStringLiteral("crotch_length"));
    names.insert(QStringLiteral("waist_to_plane_seat"), QStringLiteral("rise_length_side_sitting"));
    names.insert(QStringLiteral("rise_height"), QStringLiteral("rise_length_diag"));
    names.insert(QStringLiteral("hand_vertical_diameter"), QStringLiteral("hand_length"));
    names.insert(QStringLiteral("hand_width"), QStringLiteral("hand_palm_width"));
    names.insert(QStringLiteral("hand_girth"), QStringLiteral("hand_circ"));
    names.insert(QStringLiteral("girth_foot_instep"), QStringLiteral("foot_instep_circ"));
    names.insert(QStringLiteral("head_height"), QStringLiteral("head_length"));
    names.insert(QStringLiteral("head_and_neck_length"), QStringLiteral("head_crown_to_neck_back"));
    names.insert(QStringLiteral("neck_to_neck_base"), QStringLiteral("head_chin_to_neck_back"));
    names.insert(QStringLiteral("arc_length_upper_body"), QStringLiteral("waist_to_waist_halter"));
    names.insert(QStringLiteral("cervicale_to_wrist_length"), QStringLiteral("arm_neck_back_to_wrist_bent"));
    names.insert(QStringLiteral("strap_length"), QStringLiteral("highbust_b_over_shoulder_to_highbust_f"));
    names.insert(QStringLiteral("arc_through_shoulder_joint"), QStringLiteral("armscye_arc"));
    names.insert(QStringLiteral("head_girth"), QStringLiteral("head_circ"));
    names.insert(QStringLiteral("elbow_girth"), QStringLiteral("arm_elbow_circ"));
    names.insert(QStringLiteral("height_under_buttock_folds"), QStringLiteral("height_gluteal_fold"));
    names.insert(QStringLiteral("scye_depth"), QStringLiteral("neck_back_to_highbust_b"));
    names.insert(QStringLiteral("back_waist_to_upper_chest"), QStringLiteral("across_back_to_waist_b"));

    return names;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::OldNamesToNewNames_InV0_2_1() -> QMap<QString, QString>
{
    // TODO. Delete if minimal supported version is 0.2.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 2, 1), "Time to refactor the code.");

    // old name, new name
    QMap<QString, QString> names;

    names.insert(QStringLiteral("rise_length_side"), QStringLiteral("rise_length_side_sitting"));
    names.insert(QStringLiteral("size"), QStringLiteral("bust_arc_f"));

    return names;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixCutPoint()
{
    const QStringList types = QStringList() << *strCutSplinePath << *strCutSpline << *strCutArc;

    const QDomNodeList list = elementsByTagName(*strPoint);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(*strType);
            switch (types.indexOf(type))
            {
                case 0: // strCutSplinePath
                {
                    const quint32 id = element.attribute(*strId).toUInt();
                    quint32 curve = element.attribute(*strSplinePath).toUInt();
                    FixSubPaths(i, id, curve);
                    break;
                }
                case 1: // strCutSpline
                {
                    const quint32 id = element.attribute(*strId).toUInt();
                    quint32 curve = element.attribute(*strSpline).toUInt();
                    FixSubPaths(i, id, curve);
                    break;
                }
                case 2: // strCutArc
                {
                    const quint32 id = element.attribute(*strId).toUInt();
                    quint32 curve = element.attribute(*strArc).toUInt();
                    FixSubPaths(i, id, curve);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::FixSubPaths(int i, quint32 id, quint32 baseCurve)
{
    const QStringList pointTypes = QStringList() << *strCutSplinePath << *strCutSpline << *strPointOfIntersectionCurves
                                                 << *strCurveIntersectAxis << *strPointFromArcAndTangent
                                                 << *strPointOfIntersectionArcs;

    const QDomNodeList listPoints = elementsByTagName(*strPoint);
    for (int j = i + 1; j < listPoints.size(); ++j)
    {
        QDomElement element = listPoints.at(j).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(*strType);
            switch (pointTypes.indexOf(type))
            {
                case 0: // strCutSplinePath
                {
                    const quint32 spl = element.attribute(*strSplinePath).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strSplinePath, baseCurve);
                    }
                    break;
                }
                case 1: // strCutSpline
                {
                    const quint32 spl = element.attribute(*strSpline).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strSpline, baseCurve);
                    }
                    break;
                }
                case 2: // strPointOfIntersectionCurves
                {
                    quint32 spl = element.attribute(*strCurve1).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strCurve1, baseCurve);
                    }

                    spl = element.attribute(*strCurve2).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strCurve2, baseCurve);
                    }
                    break;
                }
                case 3: // strCurveIntersectAxis
                {
                    const quint32 spl = element.attribute(*strCurve).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strCurve, baseCurve);
                    }
                    break;
                }
                case 4: // strPointFromArcAndTangent
                {
                    const quint32 spl = element.attribute(*strArc).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strArc, baseCurve);
                    }
                    break;
                }
                case 5: // strPointOfIntersectionArcs
                {
                    quint32 arc = element.attribute(*strFirstArc).toUInt();
                    if (arc == id + 1 || arc == id + 2)
                    {
                        element.setAttribute(*strFirstArc, baseCurve);
                    }

                    arc = element.attribute(*strSecondArc).toUInt();
                    if (arc == id + 1 || arc == id + 2)
                    {
                        element.setAttribute(*strSecondArc, baseCurve);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    const QStringList splTypes = QStringList() << *strModelingPath << *strModelingSpline;

    const QDomNodeList listSplines = elementsByTagName(*strSpline);
    for (int j = 0; j < listSplines.size(); ++j)
    {
        QDomElement element = listSplines.at(j).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(*strType);
            switch (splTypes.indexOf(type))
            {
                case 0: // strModelingPath
                case 1: // strModelingSpline
                {
                    const quint32 spl = element.attribute(*strIdObject).toUInt();
                    if (spl == id + 1 || spl == id + 2)
                    {
                        element.setAttribute(*strIdObject, baseCurve);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    const QDomNodeList listArcs = elementsByTagName(*strArc);
    for (int j = 0; j < listArcs.size(); ++j)
    {
        QDomElement element = listArcs.at(j).toElement();
        if (not element.isNull())
        {
            const QString type = element.attribute(*strType);
            if (type == *strModeling)
            {
                const quint32 arc = element.attribute(*strIdObject).toUInt();
                if (arc == id + 1 || arc == id + 2)
                {
                    element.setAttribute(*strIdObject, baseCurve);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagRemoveAttributeTypeObjectInV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strModeling);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement modeling = list.at(i).toElement();
        if (not modeling.isNull())
        {
            QDomNode domNode = modeling.firstChild();
            while (not domNode.isNull())
            {
                QDomElement domElement = domNode.toElement();
                if (not domElement.isNull())
                {
                    if (domElement.hasAttribute(*strTypeObject))
                    {
                        domElement.removeAttribute(*strTypeObject);
                    }
                }
                domNode = domNode.nextSibling();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagDetailToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strDetail);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        if (not dom.isNull())
        {
            dom.setAttribute(*strSeamAllowance, dom.attribute(*strSupplement, QChar('0')));
            dom.removeAttribute(*strSupplement);

            dom.setAttribute(*strVersion, QChar('1'));

            const QStringList tags = QStringList() << *strNode << *strData << *strPatternInfo << *strGrainline;

            QDomElement tagData;
            QDomElement tagPatternInfo;
            QDomElement tagGrainline;
            QDomElement tagNodes = createElement(*strNodes);

            const QDomNodeList childList = dom.childNodes();
            for (qint32 i = 0; i < childList.size(); ++i)
            {
                const QDomElement element = childList.at(i).toElement();
                if (not element.isNull())
                {
                    switch (tags.indexOf(element.tagName()))
                    {
                        case 0: // strNode
                        {
                            QDomElement tagNode = createElement(*strNode);

                            tagNode.setAttribute(*strIdObject, element.attribute(*strIdObject, NULL_ID_STR));

                            if (element.hasAttribute(*strReverse))
                            {
                                tagNode.setAttribute(*strReverse, element.attribute(*strReverse, QChar('0')));
                            }

                            if (element.hasAttribute(*strMx))
                            {
                                tagNode.setAttribute(*strMx, element.attribute(*strMx, QChar('0')));
                            }

                            if (element.hasAttribute(*strMy))
                            {
                                tagNode.setAttribute(*strMy, element.attribute(*strMy, QChar('0')));
                            }

                            tagNode.setAttribute(*strType, element.attribute(*strType, QString()));

                            tagNodes.appendChild(tagNode);

                            break;
                        }
                        case 1: // strData
                            tagData = element.cloneNode().toElement();
                            break;
                        case 2: // strPatternInfo
                            tagPatternInfo = element.cloneNode().toElement();
                            break;
                        case 3: // strGrainline
                            tagGrainline = element.cloneNode().toElement();
                            break;
                        default:
                            break;
                    }
                }
            }

            RemoveAllChildren(dom);

            dom.appendChild(tagData);
            dom.appendChild(tagPatternInfo);
            dom.appendChild(tagGrainline);
            dom.appendChild(tagNodes);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::GetUnionDetailNodesV0_4_0(const QDomElement &detail) -> QDomElement
{
    QDomElement tagNodes = createElement(*strNodes);

    if (not detail.isNull())
    {
        const QDomNodeList childList = detail.childNodes();
        for (qint32 i = 0; i < childList.size(); ++i)
        {
            const QDomElement node = childList.at(i).toElement();
            if (not node.isNull())
            {
                QDomElement tagNode = createElement(*strNode);

                tagNode.setAttribute(*strIdObject, node.attribute(*strIdObject, NULL_ID_STR));

                if (node.hasAttribute(*strReverse))
                {
                    tagNode.setAttribute(*strReverse, node.attribute(*strReverse, QChar('0')));
                }

                tagNode.setAttribute(*strType, node.attribute(*strType, QString()));

                tagNodes.appendChild(tagNode);
            }
        }
    }

    return tagNodes;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::GetUnionChildrenNodesV0_4_0(const QDomElement &detail) -> QDomElement
{
    QDomElement tagNodes = createElement(*strNodes);

    if (not detail.isNull())
    {
        const QDomNodeList childList = detail.childNodes();
        for (qint32 i = 0; i < childList.size(); ++i)
        {
            const QDomElement node = childList.at(i).toElement();
            if (not node.isNull())
            {
                QDomElement tagNode = node.cloneNode().toElement();
                tagNodes.appendChild(tagNode);
            }
        }
    }

    return tagNodes;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::LabelTagToV0_4_4(const QString &tagName)
{
    // TODO. Delete if minimal supported version is 0.4.4
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 4), "Time to refactor the code.");

    Unit unit = Unit::Cm;
    const QStringList units = QStringList() << "mm"
                                            << "cm"
                                            << "inch";
    switch (units.indexOf(UniqueTagText(*strUnit)))
    {
        case 0: // mm
            unit = Unit::Mm;
            break;
        case 1: // cm
            unit = Unit::Cm;
            break;
        case 2: // in
            unit = Unit::Inch;
            break;
        default:
            break;
    }

    auto ConvertData = [unit](QDomElement &dom, const QString &attribute)
    {
        if (dom.hasAttribute(attribute))
        {
            QString valStr = dom.attribute(attribute, QChar('1'));
            bool ok = false;
            qreal val = valStr.toDouble(&ok);
            if (not ok)
            {
                val = 1;
            }
            dom.setAttribute(attribute, QString().setNum(FromPixel(val, unit)));
        }
    };

    const QDomNodeList list = elementsByTagName(tagName);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        if (not dom.isNull())
        {
            ConvertData(dom, *strWidth);
            ConvertData(dom, *strHeight);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternConverter::AddTagPatternLabelV0_5_1() -> QDomElement
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strPatternLabel);
    if (list.isEmpty())
    {
        const QStringList tags = QStringList() << *strUnit << *strImage << *strAuthor << *strDescription << *strNotes
                                               << *strGradation << *strPatternName << *strPatternNum << *strCompanyName
                                               << *strCustomerName << *strPatternLabel;

        QDomElement element = createElement(*strPatternLabel);
        QDomElement pattern = documentElement();
        for (vsizetype i = tags.indexOf(element.tagName()) - 1; i >= 0; --i)
        {
            const QDomNodeList list = elementsByTagName(tags.at(i));
            if (not list.isEmpty())
            {
                pattern.insertAfter(element, list.at(0));
                break;
            }
        }
        return element;
    }
    return list.at(0).toElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::PortPatternLabeltoV0_6_0(QDomElement &label)
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");

    if (not UniqueTagText(*strCompanyName).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%author%", true, false, 0, 4);
    }
    else
    {
        const QString author = UniqueTagText(*strAuthor);
        if (not author.isEmpty())
        {
            AddLabelTemplateLineV0_6_0(label, author, true, false, 0, 4);
        }
    }

    if (not UniqueTagText(*strPatternName).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%patternName%", false, false, 0, 2);
    }

    if (not UniqueTagText(*strPatternNum).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%patternNumber%", false, false, 0, 0);
    }

    if (not UniqueTagText(*strCustomerName).isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, "%customer%", false, true, 0, 0);
    }

    const QString sizeField = UniqueTagText(*strSize);
    if (not sizeField.isEmpty())
    {
        AddLabelTemplateLineV0_6_0(label, sizeField, false, false, 0, 0);
    }

    if (UniqueTagText(*strShowMeasurements) == trueStr)
    {
        AddLabelTemplateLineV0_6_0(label, "%mFileName%.%mExt%", false, false, 0, 0);
    }

    if (UniqueTagText(*strShowDate) == trueStr)
    {
        AddLabelTemplateLineV0_6_0(label, "%date%", false, true, 0, 0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::AddLabelTemplateLineV0_6_0(QDomElement &label, const QString &text, bool bold, bool italic,
                                                   int alignment, int fontSizeIncrement)
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");

    QDomElement tagLine = createElement(*strLine);

    SetAttribute(tagLine, *strText, text);
    SetAttribute(tagLine, *strBold, bold);
    SetAttribute(tagLine, *strItalic, italic);
    SetAttribute(tagLine, *strAlignment, alignment);
    SetAttribute(tagLine, *strFSIncrement, fontSizeIncrement);

    label.appendChild(tagLine);
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::PortPieceLabelstoV0_6_0()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");

    const QDomNodeList nodeList = elementsByTagName(*strData);
    for (int i = 0; i < nodeList.size(); ++i)
    {
        QDomElement dataTag = nodeList.at(i).toElement();
        QDomNodeList nodeListMCP = dataTag.childNodes();
        const int count = nodeListMCP.count();
        try
        {
            if (not GetParametrString(dataTag, *strLetter, QString()).isEmpty())
            {
                AddLabelTemplateLineV0_6_0(dataTag, "%pLetter%", true, false, Qt::AlignHCenter, 6);
            }
        }
        catch (const VExceptionEmptyParameter &)
        {
        }

        AddLabelTemplateLineV0_6_0(dataTag, "%pName%", true, false, Qt::AlignHCenter, 2);

        int firstLineCutNumber = 1;
        bool firstLine = true;

        for (int iMCP = 0; iMCP < count; ++iMCP)
        {
            QDomElement domMCP = nodeListMCP.at(iMCP).toElement();

            QString line;

            const int material = static_cast<int>(GetParametrUInt(domMCP, *strMaterial, QChar('0')));
            switch (material)
            {
                case 0:
                    line.append("%mFabric%");
                    break;
                case 1:
                    line.append("%mLining%");
                    break;
                case 2:
                    line.append("%mInterfacing%");
                    break;
                case 3:
                    line.append("%mInterlining%");
                    break;
                case 4:
                default:
                    line.append(GetParametrString(domMCP, *strUserDefined, "User material"));
                    break;
            }

            line.append(", %wCut% ");

            const int cutNumber = static_cast<int>(GetParametrUInt(domMCP, *strCutNumber, QChar('1')));

            if (firstLine)
            {
                firstLineCutNumber = cutNumber;
                dataTag.setAttribute(*strQuantity, cutNumber);
                line.append("%pQuantity%");
                firstLine = false;
            }
            else
            {
                if (firstLineCutNumber != cutNumber)
                {
                    line.append(QString::number(cutNumber));
                }
                else
                {
                    line.append("%pQuantity%");
                }
            }

            if (GetParametrUInt(domMCP, *strPlacement, QChar('0')) == 1)
            {
                line.append(" %wOnFold%");
            }

            AddLabelTemplateLineV0_6_0(dataTag, line, false, false, Qt::AlignHCenter, 0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveUnusedTagsV0_6_0()
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");

    RemoveUniqueTagV0_6_0(*strAuthor);
    RemoveUniqueTagV0_6_0(*strSize);
    RemoveUniqueTagV0_6_0(*strShowDate);
    RemoveUniqueTagV0_6_0(*strShowMeasurements);

    QDomNodeList nodeList = elementsByTagName(*strData);
    for (int i = 0; i < nodeList.size(); ++i)
    {
        QDomElement child = nodeList.at(i).firstChildElement(*strMCP);
        while (not child.isNull())
        {
            nodeList.at(i).removeChild(child);
            child = nodeList.at(i).firstChildElement(*strMCP);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveUniqueTagV0_6_0(const QString &tag)
{
    // TODO. Delete if minimal supported version is 0.6.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 0), "Time to refactor the code.");

    const QDomNodeList nodeList = elementsByTagName(tag);
    if (nodeList.isEmpty())
    {
        return;
    }

    QDomElement pattern = documentElement();
    pattern.removeChild(nodeList.at(0));
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::AddTagPreviewCalculationsV0_6_2()
{
    // TODO. Delete if minimal supported version is 0.6.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 6, 2), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strIncrements);
    if (not list.isEmpty())
    {
        QDomElement pattern = documentElement();
        pattern.insertAfter(createElement(*strPreviewCalculations), list.at(0));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::RemoveGradationV0_8_8()
{
    QDomElement patternElement = documentElement();
    if (patternElement.isElement())
    {
        QDomElement gradationTag = patternElement.firstChildElement(*strGradation);
        if (gradationTag.isElement())
        {
            patternElement.removeChild(gradationTag);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::AddPieceUUIDV0_8_8()
{
    // TODO. Delete if minimal supported version is 0.8.8
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 8, 8), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strDetail);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();

        if (not dom.isNull())
        {
            dom.setAttribute(*strUUID, QUuid::createUuid().toString());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertImageToV0_9_0()
{
    // TODO. Delete if minimal supported version is 0.9.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strImage);
    if (not list.isEmpty())
    {
        QDomElement img = list.at(0).toElement();
        if (not img.isNull())
        {
            QString extension = img.attribute(*strExtension);
            img.removeAttribute(*strExtension);

            if (not extension.isEmpty())
            {
                QMap<QString, QString> mimeTypes{{"BMP", "image/bmp"}, {"JPG", "image/jpeg"}, {"PNG", "image/png"}};

                if (mimeTypes.contains(extension))
                {
                    img.setAttribute(*strContentType, mimeTypes.value(extension));
                }
            }

            const QString content = img.text();
            if (not content.isEmpty())
            {
                auto SplitString = [content]()
                {
                    const int n = 80;
                    QStringList list;
                    QString tmp(content);

                    while (not tmp.isEmpty())
                    {
                        list.append(tmp.left(n));
                        tmp.remove(0, n);
                    }

                    return list;
                };

                QStringList data = SplitString();
                setTagText(img, data.join("\n"));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertMeasurementsPathToV0_9_1()
{
    // TODO. Delete if minimal supported version is 0.9.1
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 1), "Time to refactor the code.");

    const QDomNodeList nodeList = this->elementsByTagName(*strMeasurements);
    if (nodeList.isEmpty())
    {
        return;
    }

    const QDomNode domNode = nodeList.at(0);
    if (not domNode.isNull() && domNode.isElement())
    {
        QDomElement domElement = domNode.toElement();
        if (not domElement.isNull())
        {
            const QString path = domElement.text();
            if (path.isEmpty())
            {
                return;
            }

            // Clean text
            RemoveAllChildren(domElement);

            domElement.setAttribute(*strPath, path);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::ConvertPathAttributesToV0_9_2()
{
    // TODO. Delete if minimal supported version is 0.9.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 9, 2), "Time to refactor the code.");

    const QDomNodeList paths = this->elementsByTagName(*strPath);
    for (int i = 0; i < paths.size(); ++i)
    {
        QDomElement domElement = paths.at(i).toElement();

        if (domElement.isNull())
        {
            continue;
        }

        if (domElement.hasAttribute(*strFirstToCountour))
        {
            domElement.setAttribute(*strFirstToContour, domElement.attribute(*strFirstToCountour));
            domElement.removeAttribute(*strFirstToCountour);
        }

        if (domElement.hasAttribute(*strLastToCountour))
        {
            domElement.setAttribute(*strLastToContour, domElement.attribute(*strLastToCountour));
            domElement.removeAttribute(*strLastToCountour);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternConverter::TagUnionDetailsToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName(*strTools);
    for (int i = 0; i < list.size(); ++i)
    {
        // Tag 'tools' used only for union details, so no need to check any additional attributes
        QDomElement toolDOM = list.at(i).toElement();
        if (not toolDOM.isNull())
        {
            const QStringList tags = QStringList() << *strDet << *strChildren;

            QVector<QDomElement> nodes;
            QDomElement tagChildrenNodes = createElement(*strChildren);

            const QDomNodeList childList = toolDOM.childNodes();
            for (qint32 i = 0; i < childList.size(); ++i)
            {
                const QDomElement element = childList.at(i).toElement();
                if (not element.isNull())
                {
                    switch (tags.indexOf(element.tagName()))
                    {
                        case 0: // strDet
                            nodes.append(GetUnionDetailNodesV0_4_0(element));
                            break;
                        case 1: // strChildren
                            tagChildrenNodes.appendChild(GetUnionChildrenNodesV0_4_0(element));
                            break;
                        default:
                            break;
                    }
                }
            }

            RemoveAllChildren(toolDOM);

            for (auto &node : nodes)
            {
                QDomElement tagDet = createElement(*strDet);
                tagDet.appendChild(node);
                toolDOM.appendChild(tagDet);
            }
            toolDOM.appendChild(tagChildrenNodes);
        }
    }
}
