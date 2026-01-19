/************************************************************************
 **
 **  @file   vlayoutdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2015
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

#ifndef VLAYOUTDEF_H
#define VLAYOUTDEF_H

#include <QPainterPath>
#include <QSize>
#include <QTransform>
#include <climits>

enum class LayoutExportFileFormat : qint8
{
    SVG,
    PDF,
    PNG,
    OBJ, /* Wavefront OBJ*/
    DXF,
    NC,
    RLD,
    TIF,
    HPGL,
    PLT,
    COUNT /*Use only for validation*/
};

enum class LayoutExportFormats : qint8
{
    SVG = 0,
    PDF = 1,
    PNG = 2,
    OBJ = 3, /* Wavefront OBJ*/
    PS = 4,
    EPS = 5,
    DXF_AC1006_Flat = 6,  /* R10. */
    DXF_AC1009_Flat = 7,  /* R11 & R12. */
    DXF_AC1012_Flat = 8,  /* R13. */
    DXF_AC1014_Flat = 9,  /* R14. */
    DXF_AC1015_Flat = 10, /* ACAD 2000. */
    DXF_AC1018_Flat = 11, /* ACAD 2004. */
    DXF_AC1021_Flat = 12, /* ACAD 2007. */
    DXF_AC1024_Flat = 13, /* ACAD 2010. */
    DXF_AC1027_Flat = 14, /* ACAD 2013. */
    DXF_AAMA = 16,        /* R11 & R12. */
    DXF_ASTM = 25,        /* R11 & R12. */
    PDFTiled = 33,
    NC = 34,  /*G-code. Reserved for future*/
    RLD = 35, /*Raw Layout Data*/
    TIF = 36,
    HPGL = 37,
    HPGL2 = 38,
    HPGL_PLT = 39,  /* Same as HPGL, but has .plt extension */
    HPGL2_PLT = 40, /* Same as HPGL2, but has .plt extension */
    COUNT           /*Use only for validation*/
};

enum class LayoutErrors : qint8
{
    NoError,
    PrepareLayoutError,
    ProcessStoped,
    EmptyPaperError,
    Timeout,
    TerminatedByException
};

enum class BestFrom : qint8
{
    Rotation = 0,
    Combine = 1
};

struct VBestSquareResData
{
    QSizeF bestSize{INT_MAX, INT_MAX};
    // cppcheck-suppress unusedStructMember
    int globalI{0}; // Edge of global contour
    // cppcheck-suppress unusedStructMember
    int detJ{0};            // Edge of detail
    QTransform resMatrix{}; // Matrix for rotation and translation detail
    // cppcheck-suppress unusedStructMember
    bool resMirror{false};
    BestFrom type{BestFrom::Rotation};
    // cppcheck-suppress unusedStructMember
    qreal depthPosition{INT_MAX};
    // cppcheck-suppress unusedStructMember
    qreal sidePosition{0};
};

struct VCachedPositions
{
    QRectF boundingRect{};              // NOLINT(misc-non-private-member-variables-in-classes)
    QPainterPath layoutAllowancePath{}; // NOLINT(misc-non-private-member-variables-in-classes)
};

enum class Cases : qint8
{
    CaseThreeGroup = 0,
    CaseTwoGroup,
    CaseDesc,
    UnknownCase
};

/* Warning! Debugging doesn't work stable in debug mode. If you need big allocation use release mode. Or disable
 * Address Sanitizer. See page https://bitbucket.org/dismine/valentina/wiki/developers/Address_Sanitizer
 */
// #define LAYOUT_DEBUG // Enable debug mode

// This block help rule debug mode. Don't turn all options at the same time!
#ifdef LAYOUT_DEBUG
// Nice looking
#define SHOW_VERTICES    // Show contour vertices
#define SHOW_DIRECTION   // Show contour direction
#define ARRANGED_DETAILS // Show already arranged details
#define SHOW_SHEET       // Show sheet rect
#define SHOW_CANDIDATE   // Show each position

// Debugging
// #   define SHOW_ROTATION         // For each position show rotation part
// #   define SHOW_COMBINE          // For each position show edge combine part
// #   define SHOW_MIRROR           // For each position show mirror part
// #   define SHOW_CANDIDATE_BEST   // For only correct positions that pass checks
#define SHOW_BEST // Show only best position for workpiece
#endif            // LAYOUT_DEBUG

#endif // VLAYOUTDEF_H
