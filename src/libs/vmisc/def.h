/************************************************************************
 **
 **  @file   def.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 4, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef DEF_H
#define DEF_H

#include <QtGlobal>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif /*Q_OS_WIN*/

#include <QLineF>
#include <QMargins>
#include <QPrinter>
#include <QString>
#include <QStringList>
#include <csignal>

#include "debugbreak.h"
#include "defglobal.h"

template <class T> class QSharedPointer;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COPY_CONSTRUCTOR_IMPL(className) className::className(const className &) = default;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COPY_CONSTRUCTOR_IMPL_2(className, baseClassName) className::className(const className &) = default;

// https://stackoverflow.com/questions/75008386/constructor-is-implicitly-deleted-because-its-exception-specification-does-not-m
#if (defined(Q_CC_GNU) && Q_CC_GNU < 1001) && !defined(Q_CC_CLANG)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_CONSTRUCTOR(className)                                                                                     \
    className() noexcept                                                                                               \
    {                                                                                                                  \
    }
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_CONSTRUCTOR(className) className() noexcept = default;
#endif

class QComboBox;
class QMarginsF;
class VTranslateMeasurements;
class QGraphicsItem;

constexpr qreal maxSceneSize = ((20.0 * 1000.0) / 25.4) * PrintDPI; // 20 meters in pixels

constexpr qreal defCurveApproximationScale = 0.5;
constexpr qreal minCurveApproximationScale = 0.2;
constexpr qreal maxCurveApproximationScale = 10.0;

constexpr int minLabelFontSize = 5;
constexpr int maxLabelFontSize = 100;

enum class NodeDetail : qint8
{
    Contour,
    Modeling
};
enum class SceneObject : qint8
{
    Point,
    Line,
    Spline,
    Arc,
    ElArc,
    SplinePath,
    Detail,
    Unknown
};
enum class MeasurementsType : qint8
{
    Multisize,
    Individual,
    Unknown
};
enum class Unit : qint8
{
    Mm = 0,
    Cm,
    Inch,
    Px,
    LAST_UNIT_DO_NOT_USE
};
enum class Source : qint8
{
    FromGui,
    FromFile,
    FromTool
};
enum class SelectionType : bool
{
    ByMousePress,
    ByMouseRelease
};
enum class PageOrientation : bool
{
    Portrait = true,
    Landscape = false
};
enum class Draw : qint8
{
    Calculation,
    Modeling,
    Layout
};

enum class PieceNodeAngle : quint8
{
    ByLength = 0,
    ByPointsIntersection = 1,
    ByFirstEdgeSymmetry = 2,
    BySecondEdgeSymmetry = 3,
    ByFirstEdgeRightAngle = 4,
    BySecondEdgeRightAngle = 5,
    ByLengthCurve = 6, // used only in runtime
    LAST_ONE_DO_NOT_USE
};

enum class PassmarkLineType : quint8
{
    OneLine = 0, // Default
    TwoLines = 1,
    ThreeLines = 2,
    TMark = 3,
    ExternalVMark = 4,
    InternalVMark = 5,
    UMark = 6,
    BoxMark = 7,
    CheckMark = 8,
    LAST_ONE_DO_NOT_USE
};

auto PassmarkLineTypeToString(PassmarkLineType type) -> QString;
auto StringToPassmarkLineType(const QString &value) -> PassmarkLineType;

enum class PassmarkAngleType : quint8
{
    Straightforward = 0, // Default
    Bisector,
    Intersection,
    IntersectionOnlyLeft,
    IntersectionOnlyRight,
    Intersection2,
    Intersection2OnlyLeft,
    Intersection2OnlyRight,
    LAST_ONE_DO_NOT_USE
};

auto PassmarkAngleTypeToString(PassmarkAngleType type) -> QString;
auto StringToPassmarkAngleType(const QString &value) -> PassmarkAngleType;

auto StrToUnits(const QString &unit) -> Unit;
auto UnitsToStr(const Unit &unit, const bool translate = false) -> QString;

enum class PiecePathIncludeType : quint8
{
    AsMainPath = 0,
    AsCustomSA = 1
};

constexpr unsigned int defFoldLineFontSize{12};

enum class FoldLineType : quint8
{
    TwoArrowsTextAbove = 0, // Default
    TwoArrowsTextUnder,
    TwoArrows,
    Text,
    ThreeDots,
    ThreeX,
    None,
    LAST_ONE_DO_NOT_USE
};

auto FoldLineTypeToString(FoldLineType type) -> QString;
auto StringToFoldLineType(const QString &value) -> FoldLineType;

enum class PiecePathType : quint8
{
    PiecePath = 0,
    CustomSeamAllowance = 1,
    InternalPath = 2,
    Unknown = 3
};

using ToolVisHolderType = int;
enum class Tool : ToolVisHolderType
{
    Arrow,
    SinglePoint,
    DoublePoint,
    LinePoint,
    AbstractSpline,
    Cut,
    BasePoint,
    EndLine,
    Line,
    AlongLine,
    ShoulderPoint,
    Normal,
    Bisector,
    LineIntersect,
    Spline,
    CubicBezier,
    CutSpline,
    CutArc,
    Arc,
    ArcWithLength,
    SplinePath,
    CubicBezierPath,
    CutSplinePath,
    PointOfContact,
    Piece,
    PiecePath,
    NodePoint,
    NodeArc,
    NodeElArc,
    NodeSpline,
    NodeSplinePath,
    Height,
    Triangle,
    LineIntersectAxis,
    PointOfIntersectionArcs,
    PointOfIntersectionCircles,
    PointOfIntersectionCurves,
    CurveIntersectAxis,
    ArcIntersectAxis,
    PointOfIntersection,
    PointFromCircleAndTangent,
    PointFromArcAndTangent,
    TrueDarts,
    UnionDetails,
    Group,
    Rotation,
    FlippingByLine,
    FlippingByAxis,
    Move,
    Midpoint,
    EllipticalArc,
    Pin,
    InsertNode,
    PlaceLabel,
    DuplicateDetail,
    BackgroundImage,
    BackgroundImageControls,
    BackgroundPixmapImage,
    BackgroundSVGImage,
    ArcStart,
    ArcEnd,
    EllipticalArcWithLength,
    ParallelCurve,
    GraduatedCurve,
    LAST_ONE_DO_NOT_USE // add new stuffs above this, this constant must be last and never used
};

enum class Vis : ToolVisHolderType
{
    ControlPointSpline = static_cast<ToolVisHolderType>(Tool::LAST_ONE_DO_NOT_USE),
    GraphicsSimpleTextItem,
    SimplePoint,
    SimpleCurve,
    ScaledLine,
    ScaledEllipse,
    Line,
    Path,
    Operation,
    ToolAlongLine,
    ToolArc,
    ToolArcWithLength,
    ToolBisector,
    ToolCutArc,
    ToolEndLine,
    ToolHeight,
    ToolLine,
    ToolLineIntersect,
    ToolNormal,
    ToolPointOfContact,
    ToolPointOfIntersection,
    ToolPointOfIntersectionArcs,
    ToolPointOfIntersectionCircles,
    ToolPointOfIntersectionCurves,
    ToolPointFromCircleAndTangent,
    ToolPointFromArcAndTangent,
    ToolShoulderPoint,
    ToolSpline,
    ToolCubicBezier,
    ToolCubicBezierPath,
    ToolTriangle,
    ToolCutSpline,
    ToolSplinePath,
    ToolCutSplinePath,
    ToolLineIntersectAxis,
    ToolCurveIntersectAxis,
    ToolTrueDarts,
    ToolRotation,
    ToolFlippingByLine,
    ToolFlippingByAxis,
    ToolMove,
    ToolEllipticalArc,
    ToolPiece,
    ToolPiecePath,
    ToolSpecialPoint,
    ToolPlaceLabel,
    ToolDuplicateDetail,
    PieceSpecialPoints,
    NoBrush,
    CurvePathItem,
    GrainlineItem,
    PieceItem,
    TextGraphicsItem,
    ScenePoint,
    ToolEllipticalArcWithLength,
    ToolParallelCurve,
    ToolGraduatedCurve,
    LAST_ONE_DO_NOT_USE // add new stuffs above this, this constant must be last and never used
};

enum class Layout : ToolVisHolderType
{
    GrainlineItem = static_cast<ToolVisHolderType>(Vis::LAST_ONE_DO_NOT_USE),
    FoldLineItem,
    LAST_ONE_DO_NOT_USE // add new stuffs above this, this constant must be last and never used
};

enum class VarType : qint8
{
    Measurement,
    MeasurementSeparator,
    Increment,
    IncrementSeparator,
    LineLength,
    CurveLength,
    CurveCLength,
    LineAngle,
    CurveAngle,
    ArcRadius,
    PieceExternalArea,
    PieceSeamLineArea,
    Unknown
};

enum class IncrementType : qint8
{
    Increment,
    Separator
};

auto IncrementTypeToString(IncrementType type) -> QString;
auto StringToIncrementType(const QString &value) -> IncrementType;

enum class MeasurementType : qint8
{
    Measurement,
    Separator
};

auto MeasurementTypeToString(MeasurementType type) -> QString;
auto StringToMeasurementType(const QString &value) -> MeasurementType;

enum class IMD : qint8 // Individual measurement dimension
{
    N, // None
    X, // height
    Y, // size (chest half circumference)
    W, // waist half circumference
    Z  // hip half circumference
};

struct VMeasurementCorrection
{
    qreal correction{0};
    QString alias{};
};

/* QImage supports a maximum of 32768x32768 px images (signed short).
 * This follows from the condition: width * height * colordepth < INT_MAX (4 billion) -> 32768 * 32768 * 4 = 4 billion.
 * The second condition is of course that malloc is able to allocate the requested memory.
 *
 * If you really need bigger images you will have to use another wrapper or split into multiple QImage's.
 */
#define QIMAGE_MAX 32768

/*
 * This macros SCASSERT (for Stop and Continue Assert) will break into the debugger on the line of the assert and allow
 * you to continue afterwards should you choose to.
 * idea: Q_ASSERT no longer pauses debugger - http://qt-project.org/forums/viewthread/13148
 * Usefull links:
 * 1. What's the difference between __PRETTY_FUNCTION__, __FUNCTION__, __func__? -
 *    https://stackoverflow.com/questions/4384765/whats-the-difference-between-pretty-function-function-func
 *
 * 2. Windows Predefined Macros - http://msdn.microsoft.com/library/b0084kay.aspx
 *
 * 3. Windows DebugBreak function - http://msdn.microsoft.com/en-us/library/ms679297%28VS.85%29.aspx
 *
 * 4. Continue to debug after failed assertion on Linux? [C/C++] -
 * https://stackoverflow.com/questions/1721543/continue-to-debug-after-failed-assertion-on-linux-c-c
 */
#ifndef V_NO_ASSERT

#define SCASSERT(cond)                                                                                                 \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        qCritical("ASSERT: %s in %s (%s:%u)", #cond, Q_FUNC_INFO, __FILE__, __LINE__);                                 \
        debug_break();                                                                                                 \
        abort();                                                                                                       \
    }

#else // define but disable this function if debugging is not set
#define SCASSERT(cond) qt_noop();
#endif /* V_NO_ASSERT */

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#ifdef Q_OS_MAC
// Fix QT issue on MacOS version 11.0 "Big Sur"
//     https://bugreports.qt.io/browse/QTBUG-87014
// Enables layer backing for Big Sur
// Prevents 100% CPU & RAM utilization
//
// The fix at
//
//     https://codereview.qt-project.org/c/qt/qtbase/+/322228/3/src/plugins/platforms/cocoa/qnsview_drawing.mm
//
// enables layer backing if we're running on Big Sur OR we're running on
// Catalina AND we were built with the Catalina SDK. Enable layer backing
// here by setting QT_MAC_WANTS_LAYER=1, but only if we're running on Big
// Sur and our version of Qt doesn't have a fix for QTBUG-87014.

// We'll assume that it will be fixed in 5.12.11, 5.15.3, and 6.0.1.
// Feel free to add other versions if needed.
#define MACOS_LAYER_BACKING_AFFECTED                                                                                   \
    ((QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) && QT_VERSION < QT_VERSION_CHECK(5, 15, 3)) ||                          \
     (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 1)))

#if MACOS_LAYER_BACKING_AFFECTED
#include <QOperatingSystemVersion>
void MacosEnableLayerBacking();
#endif // MACOS_LAYER_BACKING_AFFECTED
#endif // Q_OS_MAC

// Don't forget to syncronize with XSD schema.
const int userMaterialPlaceholdersQuantity = 20;

auto QPixmapFromCache(const QString &pixmapPath) -> QPixmap;
void SetItemOverrideCursor(QGraphicsItem *item, const QString &pixmapPath, int hotX = -1, int hotY = -1);

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto MmToPixel(T val) noexcept -> T
{
    return (val / 25.4) * PrintDPI;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto CmToPixel(T val) noexcept -> T
{
    return ((val * 10.0) / 25.4) * PrintDPI;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto InchToPixel(T val) noexcept -> T
{
    return val * PrintDPI;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto ToPixel(double val, const Unit &unit) noexcept -> double
{
    switch (unit)
    {
        case Unit::Mm:
            return MmToPixel(val);
        case Unit::Cm:
            return CmToPixel(val);
        case Unit::Inch:
            return InchToPixel(val);
        case Unit::Px:
            return val;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto PixelToInch(T pix) -> T
{
    return pix / PrintDPI;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto PixelToMm(T pix) -> T
{
    return PixelToInch(pix) * 25.4;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto PixelToCm(T pix) -> T
{
    return PixelToInch(pix) * 2.54;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto FromPixel(double pix, const Unit &unit) -> double
{
    switch (unit)
    {
        case Unit::Mm:
            return PixelToMm(pix);
        case Unit::Cm:
            return PixelToCm(pix);
        case Unit::Inch:
            return PixelToInch(pix);
        case Unit::Px:
            return pix;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto Inch2ToPixel2(T val) -> T
{
    return val * (PrintDPI * PrintDPI);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto Mm2ToPixel2(T val) -> T
{
    // There are 0.15500031 square inches in a square centimeter.
    return Inch2ToPixel2(val * 0.001550031);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto Cm2ToPixel2(T val) -> T
{
    // There are 0.15500031 square inches in a square centimeter.
    return Inch2ToPixel2(val * 0.15500031);
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto ToPixel2(double val, const Unit &unit) -> double
{
    switch (unit)
    {
        case Unit::Mm:
            return Mm2ToPixel2(val);
        case Unit::Cm:
            return Cm2ToPixel2(val);
        case Unit::Inch:
            return Inch2ToPixel2(val);
        case Unit::Px:
            return val;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto Pixel2ToInch2(T pix) -> T
{
    return pix / (PrintDPI * PrintDPI);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto Pixel2ToMm2(T pix) -> T
{
    // There are 0.15500031 square inches in a square centimeter.
    return Pixel2ToInch2(pix) / 0.001550031;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> constexpr auto Pixel2ToCm2(T pix) -> T
{
    // There are 0.15500031 square inches in a square centimeter.
    return Pixel2ToInch2(pix) / 0.15500031;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto FromPixel2(double pix, const Unit &unit) -> double
{
    switch (unit)
    {
        case Unit::Mm:
            return Pixel2ToMm2(pix);
        case Unit::Cm:
            return Pixel2ToCm2(pix);
        case Unit::Inch:
            return Pixel2ToInch2(pix);
        case Unit::Px:
            return pix;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto UnitConvertor(qreal value, const Unit &from, const Unit &to) -> qreal
{
    switch (from)
    {
        case Unit::Mm:
            switch (to)
            {
                case Unit::Mm:
                    return value;
                case Unit::Cm:
                    return value / 10.0;
                case Unit::Inch:
                    return value / 10.0 / 2.54;
                case Unit::Px:
                    return (value / 25.4) * PrintDPI;
                default:
                    break;
            }
            break;
        case Unit::Cm:
            switch (to)
            {
                case Unit::Mm:
                    return value * 10.0;
                case Unit::Cm:
                    return value;
                case Unit::Inch:
                    return value / 2.54;
                case Unit::Px:
                    return ((value * 10.0) / 25.4) * PrintDPI;
                default:
                    break;
            }
            break;
        case Unit::Inch:
            switch (to)
            {
                case Unit::Mm:
                    return value * 2.54 * 10.0;
                case Unit::Cm:
                    return value * 2.54;
                case Unit::Inch:
                    return value;
                case Unit::Px:
                    return value * PrintDPI;
                default:
                    break;
            }
            break;
        case Unit::Px:
            switch (to)
            {
                case Unit::Mm:
                    return (value / PrintDPI) * 25.4;
                case Unit::Cm:
                    return ((value / PrintDPI) * 25.4) / 10.0;
                case Unit::Inch:
                    return value / PrintDPI;
                case Unit::Px:
                    return value;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UnitConvertor Converts the values of the given margin from given unit to the new unit.
 * returns a new instand of QMarginsF.
 */
constexpr auto UnitConvertor(const QMarginsF &margins, const Unit &from, const Unit &to)
    -> QMarginsF
{
    const qreal left = UnitConvertor(margins.left(), from, to);
    const qreal top = UnitConvertor(margins.top(), from, to);
    const qreal right = UnitConvertor(margins.right(), from, to);
    const qreal bottom = UnitConvertor(margins.bottom(), from, to);

    return {left, top, right, bottom};
}

void InitLanguages(QComboBox *combobox);
void InitPieceLabelLanguages(QComboBox *combobox);
Q_REQUIRED_RESULT auto SupportedLocales() -> QStringList;

Q_REQUIRED_RESULT auto RelativeMPath(const QString &patternPath, const QString &absoluteMPath) -> QString;
Q_REQUIRED_RESULT auto AbsoluteMPath(const QString &patternPath, const QString &relativeMPath) -> QString;

auto SplitFilePaths(const QString &path) -> QStringList;

auto GetMinPrinterFields(const QSharedPointer<QPrinter> &printer) -> QMarginsF;
auto GetPrinterFields(const QSharedPointer<QPrinter> &printer) -> QMarginsF;

Q_REQUIRED_RESULT auto darkenPixmap(const QPixmap &pixmap) -> QPixmap;

void ShowInGraphicalShell(const QString &filePath);

Q_REQUIRED_RESULT constexpr static auto VFuzzyComparePossibleNulls(double p1, double p2) -> bool;
constexpr static auto VFuzzyComparePossibleNulls(double p1, double p2) -> bool
{
    if (qFuzzyIsNull(p1))
    {
        return qFuzzyIsNull(p2);
    }

    if (qFuzzyIsNull(p2))
    {
        return false;
    }

    return qFuzzyCompare(p1, p2);
}

/**
 * @brief The CustomSA struct contains record about custom seam allowanse (SA).
 */
struct CustomSARecord
{
    friend auto operator<<(QDataStream &out, const CustomSARecord &record) -> QDataStream &;
    friend auto operator>>(QDataStream &in, CustomSARecord &record) -> QDataStream &;

    quint32 startPoint{0};
    quint32 path{0};
    quint32 endPoint{0};
    bool reverse{false};
    PiecePathIncludeType includeType{PiecePathIncludeType::AsCustomSA};

private:
    static const quint32 streamHeader;
    static const quint16 classVersion;
};

Q_DECLARE_METATYPE(CustomSARecord)
Q_DECLARE_TYPEINFO(CustomSARecord, Q_MOVABLE_TYPE); // NOLINT
/****************************************************************************
** This file is derived from code bearing the following notice:
** The sole author of this file, Adam Higerd, has explicitly disclaimed all
** copyright interest and protection for the content within. This file has
** been placed in the public domain according to United States copyright
** statute and case law. In jurisdictions where this public domain dedication
** is not legally recognized, anyone who receives a copy of this file is
** permitted to use, modify, duplicate, and redistribute this file, in whole
** or in part, with no restrictions or conditions. In these jurisdictions,
** this file shall be copyright (C) 2006-2008 by Adam Higerd.
****************************************************************************/

#define QXT_DECLARE_PRIVATE(PUB)                                                                                       \
    friend class PUB##Private;                                                                                         \
    QxtPrivateInterface<PUB, PUB##Private> qxt_d;
#define QXT_DECLARE_PUBLIC(PUB) friend class PUB;
#define QXT_INIT_PRIVATE(PUB) qxt_d.setPublic(this);
#define QXT_D(PUB) PUB##Private &d = qxt_d()
#define QXT_P(PUB) PUB &p = qxt_p()

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

template <typename PUB> class QxtPrivate
{
public:
    QxtPrivate()
      : qxt_p_ptr(nullptr)
    {
    }
    virtual ~QxtPrivate() {}
    inline void QXT_setPublic(PUB *pub) { qxt_p_ptr = pub; }

protected:
    inline auto qxt_p() -> PUB & { return *qxt_p_ptr; }
    inline auto qxt_p() const -> const PUB & { return *qxt_p_ptr; }
    inline auto qxt_ptr() -> PUB * { return qxt_p_ptr; }
    inline auto qxt_ptr() const -> const PUB * { return qxt_p_ptr; }

private:
    Q_DISABLE_COPY_MOVE(QxtPrivate) // NOLINT
    PUB *qxt_p_ptr;
};

// cppcheck-suppress unknownMacro
QT_WARNING_POP

template <typename PUB, typename PVT> class QxtPrivateInterface
{
    friend class QxtPrivate<PUB>;

public:
    QxtPrivateInterface()
      : pvt(new PVT)
    {
    }
    ~QxtPrivateInterface() { delete pvt; }

    inline void setPublic(PUB *pub) { pvt->QXT_setPublic(pub); }
    inline auto operator()() -> PVT & { return *static_cast<PVT *>(pvt); }
    inline auto operator()() const -> const PVT & { return *static_cast<PVT *>(pvt); }
    inline auto operator->() -> PVT * { return static_cast<PVT *>(pvt); }
    inline auto operator->() const -> const PVT * { return static_cast<PVT *>(pvt); }

private:
    Q_DISABLE_COPY_MOVE(QxtPrivateInterface) // NOLINT
    QxtPrivate<PUB> *pvt;
};

#endif // DEF_H
