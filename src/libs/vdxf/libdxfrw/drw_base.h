/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef DRW_BASE_H
#define DRW_BASE_H

#define DRW_VERSION "0.6.3"

#include <QtGlobal>
#include <cmath>
#include <list>
#include <string>
#include <unordered_map>

// Header <ciso646> is removed in C++20.
#if defined(Q_CC_MSVC) && __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#ifdef DRW_ASSERTS
#define drw_assert(a) assert(a)
#else
#define drw_assert(a)
#endif

#define UTF8STRING std::string
#define DRW_UNUSED(x) (void)x

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#define DRW_WIN
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define DRW_WIN
#elif defined(__MWERKS__) && defined(__INTEL__)
#define DRW_WIN
#else
#define DRW_POSIX
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#define M_PIx2 6.283185307179586 // 2*PI
#define ARAD 57.29577951308232

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

#ifndef Q_DISABLE_ASSIGN_MOVE
#define Q_DISABLE_ASSIGN_MOVE(Class)                                                                                   \
    Q_DISABLE_ASSIGN(Class)                                                                                            \
    Class(Class &&) = delete;                                                                                          \
    Class &operator=(Class &&) = delete;
#endif

using dint8 = signed char; /* 8 bit signed */
using dint16 = short;      /* 16 bit signed */
using dint32 = int;        /* 32 bit signed */
using dint64 = long long;  /* 64 bit signed */

using duint8 = unsigned char;       /* 8 bit unsigned */
using duint16 = unsigned short;     /* 16 bit unsigned */
using duint32 = unsigned int;       /* 32 bit unsigned */
using duint64 = unsigned long long; /* 64 bit unsigned */

using dfloat32 = float;        /* 32 bit floating point */
using ddouble64 = double;      /* 64 bit floating point */
using ddouble80 = long double; /* 80 bit floating point */

namespace DRW
{

//! Version numbers for the DXF Format.
enum Version
{
    UNKNOWNV, //!< UNKNOWN VERSION.
    MC00,     //!< DWG Release 1.1
    AC12,     //!< DWG Release 1.2
    AC14,     //!< DWG Release 1.4
    AC150,    //!< DWG Release 2.0
    AC210,    //!< DWG Release 2.10
    AC1002,   //!< DWG Release 2.5
    AC1003,   //!< DWG Release 2.6
    AC1004,   //!< DWG Relase 9
    AC1006,   //!< DWG Release 10 (R10)
    AC1009,   //!< DWG Release 11/12 (LT R1/R2) (R11 & R12)
    AC1012,   //!< DWG Release 13 (LT95) (R13)
    AC1014,   //!< DWG Release 14/14.01 (LT97/LT98) (R14)
    AC1015,   //!< AutoCAD 2000/2000i/2002 (ACAD 2000)
    AC1018,   //!< AutoCAD 2004/2005/2006 (ACAD 2004)
    AC1021,   //!< AutoCAD 2007/2008/2009 (ACAD 2007)
    AC1024,   //!< AutoCAD 2010/2011/2012 (ACAD 2010)
    AC1027,   //!< AutoCAD 2013/2014/2015/2016/2017 (ACAD 2013)
    AC1032,   //!< AutoCAD 2018/2019/2020 (ACAD 2018)
};

const std::unordered_map<const char *, DRW::Version> dwgVersionStrings{
    {"MC0.0", DRW::MC00},    {"AC1.2", DRW::AC12},    {"AC1.4", DRW::AC14},    {"AC1.50", DRW::AC150},
    {"AC2.10", DRW::AC210},  {"AC1002", DRW::AC1002}, {"AC1003", DRW::AC1003}, {"AC1004", DRW::AC1004},
    {"AC1006", DRW::AC1006}, {"AC1009", DRW::AC1009}, {"AC1012", DRW::AC1012}, {"AC1014", DRW::AC1014},
    {"AC1015", DRW::AC1015}, {"AC1018", DRW::AC1018}, {"AC1021", DRW::AC1021}, {"AC1024", DRW::AC1024},
    {"AC1027", DRW::AC1027}, {"AC1032", DRW::AC1032},
};

enum error
{
    BAD_NONE,             /*!< No error. */
    BAD_UNKNOWN,          /*!< UNKNOWN. */
    BAD_OPEN,             /*!< error opening file. */
    BAD_VERSION,          /*!< unsupported version. */
    BAD_READ_METADATA,    /*!< error reading matadata. */
    BAD_READ_FILE_HEADER, /*!< error in file header read process. */
    BAD_READ_HEADER,      /*!< error in header vars read process. */
    BAD_READ_HANDLES,     /*!< error in object map read process. */
    BAD_READ_CLASSES,     /*!< error in classes read process. */
    BAD_READ_TABLES,      /*!< error in tables read process. */
    BAD_READ_BLOCKS,      /*!< error in block read process. */
    BAD_READ_ENTITIES,    /*!< error in entities read process. */
    BAD_READ_OBJECTS,     /*!< error in objects read process. */
    BAD_READ_SECTION,     /*!< error in sections read process. */
    BAD_CODE_PARSED,      /*!< error in any parseCodes() method. */
};

enum class DebugLevel
{
    None,
    Debug
};

/**
 * Interface for debug printers.
 *
 * The base class is silent and ignores all debugging.
 */
class DebugPrinter
{
public:
    virtual void printS(const std::string &s) { (void)s; }
    virtual void printI(long long int i) { (void)i; }
    virtual void printUI(long long unsigned int i) { (void)i; }
    virtual void printD(double d) { (void)d; }
    virtual void printH(long long int i) { (void)i; }
    virtual void printB(int i) { (void)i; }
    virtual void printHL(int c, int s, int h)
    {
        (void)c;
        (void)s;
        (void)h;
    }
    virtual void printPT(double x, double y, double z)
    {
        (void)x;
        (void)y;
        (void)z;
    }
    virtual ~DebugPrinter() = default;
};

/**
 * Sets a custom debug printer to use when outputting debug messages.
 *
 * Ownership of `printer` is transferred.
 */
void setCustomDebugPrinter(DebugPrinter *printer);

//! Special codes for colors
enum ColorCodes
{
    black = 250,
    green = 3,
    red = 1,
    brown = 15,
    yellow = 2,
    cyan = 4,
    magenta = 6,
    gray = 8,
    blue = 5,
    l_blue = 163,
    l_green = 121,
    l_cyan = 131,
    l_red = 23,
    l_magenta = 221,
    l_gray = 252,
    white = 7,
    ColorByLayer = 256,
    ColorByBlock = 0
};

//! Spaces
enum Space
{
    ModelSpace = 0,
    PaperSpace = 1
};

//! Special kinds of handles
enum HandleCodes
{
    NoHandle = 0
};

//! Shadow mode
enum ShadowMode
{
    CastAndReceieveShadows = 0,
    CastShadows = 1,
    ReceiveShadows = 2,
    IgnoreShadows = 3
};

//! Special kinds of materials
enum MaterialCodes
{
    MaterialByLayer = 0
};

//! Special kinds of plot styles
enum PlotStyleCodes
{
    DefaultPlotStyle = 0
};

//! Special kinds of transparencies
enum TransparencyCodes
{
    Opaque = 0,
    Transparent = -1
};

} // namespace DRW

Q_REQUIRED_RESULT static inline auto DRW_FuzzyComparePossibleNulls(double p1, double p2) -> bool;
static inline auto DRW_FuzzyComparePossibleNulls(double p1, double p2) -> bool
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

//! Class to handle 3D coordinate point
/*!
 *  Class to handle 3D coordinate point
 *  @author Rallaz
 */
class DRW_Coord
{
public:
    DRW_Coord() = default;
    DRW_Coord(double ix, double iy)
      : x(ix),
        y(iy)
    {
    }
    DRW_Coord(double ix, double iy, double iz)
      : x(ix),
        y(iy),
        z(iz)
    {
    }

    DRW_Coord(const DRW_Coord &data)
      : x(data.x),
        y(data.y),
        z(data.z)
    {
    }

    auto operator=(const DRW_Coord &data) -> DRW_Coord &
    {
        if (&data == this)
        {
            return *this;
        }

        x = data.x;
        y = data.y;
        z = data.z;
        return *this;
    }
    /*!< convert to unitary vector */
    void unitize()
    {
#if __cplusplus < 201703
        double dist = std::hypot(std::hypot(x, y), z);
#else
        double dist = std::hypot(x, y, z);
#endif
        if (std::isnormal(dist))
        {
            x = x / dist;
            y = y / dist;
            z = z / dist;
        }
    }

public:
    double x{0};
    double y{0};
    double z{0};
};

//! Class to handle vertex
/*!
 *  Class to handle vertex for lwpolyline entity
 *  @author Rallaz
 */
class DRW_Vertex2D
{
public:
    DRW_Vertex2D() = default;
    DRW_Vertex2D(double sx, double sy, double b = 0.0)
      : x(sx),
        y(sy),
        bulge(b)
    {
    }

public:
    double x{0.};        /*!< x coordinate, code 10 */
    double y{0.};        /*!< y coordinate, code 20 */
    double stawidth{0.}; /*!< Start width, code 40 */
    double endwidth{0.}; /*!< End width, code 41 */
    double bulge{0.};    /*!< bulge, code 42 */
};

//! Class to handle header vars
/*!
 *  Class to handle header vars
 *  @author Rallaz
 */
class DRW_Variant
{
public:
    enum TYPE
    {
        STRING,
        INTEGER,
        DOUBLE,
        COORD,
        INVALID
    };
    // TODO: add INT64 support
    DRW_Variant()
      : content(),
        type(INVALID),
        code(),
        sdata(),
        vdata()
    {
    }

    DRW_Variant(int c, dint32 i)
      : content(),
        type(),
        code(c),
        sdata(),
        vdata()
    {
        addInt(i);
    }

    DRW_Variant(int c, duint32 i)
      : content(),
        type(),
        code(c),
        sdata(),
        vdata()
    {
        addInt(static_cast<dint32>(i)); // RLZ: verify if work with big numbers
    }

    DRW_Variant(int c, double d)
      : content(),
        type(),
        code(c),
        sdata(),
        vdata()
    {
        addDouble(d);
    }

    DRW_Variant(int c, const UTF8STRING &s)
      : content(),
        type(),
        code(c),
        sdata(),
        vdata()
    {
        addString(s);
    }

    DRW_Variant(int c, const DRW_Coord &crd)
      : content(),
        type(),
        code(c),
        sdata(),
        vdata()
    {
        addCoord(crd);
    }

    DRW_Variant(const DRW_Variant &d)
      : content(d.content),
        type(d.type),
        code(d.code),
        sdata(),
        vdata()
    {
        if (d.type == COORD)
        {
            vdata = d.vdata;
            content.v = &vdata;
        }

        if (d.type == STRING)
        {
            sdata = d.sdata;
            content.s = &sdata;
        }
    }

    ~DRW_Variant() = default;

    void addString(const UTF8STRING &s)
    {
        setType(STRING);
        sdata = s;
        content.s = &sdata;
    }
    void addInt(int i)
    {
        setType(INTEGER);
        content.i = i;
    }
    void addDouble(double d)
    {
        setType(DOUBLE);
        content.d = d;
    }
    void addCoord()
    {
        setType(COORD);
        vdata.x = 0.0;
        vdata.y = 0.0;
        vdata.z = 0.0;
        content.v = &vdata;
    }
    void addCoord(const DRW_Coord &v)
    {
        setType(COORD);
        vdata = v;
        content.v = &vdata;
    }
    void setType(TYPE t) { type = t; }
    void setCoordX(double d)
    {
        if (type == COORD)
            vdata.x = d;
    }
    void setCoordY(double d)
    {
        if (type == COORD)
            vdata.y = d;
    }
    void setCoordZ(double d)
    {
        if (type == COORD)
            vdata.z = d;
    }

private:
    using DRW_VarContent = union
    {
        UTF8STRING *s;
        dint32 i;
        double d;
        DRW_Coord *v;
    };

public:
    DRW_VarContent content;
    TYPE type{INVALID};
    int code{0}; /*!< dxf code of this value*/

private:
    auto operator=(const DRW_Variant &) -> DRW_Variant &Q_DECL_EQ_DELETE;
    std::string sdata;
    DRW_Coord vdata;
};

//! Class to convert between line width and integer
/*!
 *  Class to convert between line width and integer
 *  verifing valid values, if value is not valid
 *  returns widthDefault.
 *  @author Rallaz
 */
class DRW_LW_Conv
{
public:
    enum lineWidth
    {
        width00 = 0,       /*!< 0.00mm (dxf 0)*/
        width01 = 1,       /*!< 0.05mm (dxf 5)*/
        width02 = 2,       /*!< 0.09mm (dxf 9)*/
        width03 = 3,       /*!< 0.13mm (dxf 13)*/
        width04 = 4,       /*!< 0.15mm (dxf 15)*/
        width05 = 5,       /*!< 0.18mm (dxf 18)*/
        width06 = 6,       /*!< 0.20mm (dxf 20)*/
        width07 = 7,       /*!< 0.25mm (dxf 25)*/
        width08 = 8,       /*!< 0.30mm (dxf 30)*/
        width09 = 9,       /*!< 0.35mm (dxf 35)*/
        width10 = 10,      /*!< 0.40mm (dxf 40)*/
        width11 = 11,      /*!< 0.50mm (dxf 50)*/
        width12 = 12,      /*!< 0.53mm (dxf 53)*/
        width13 = 13,      /*!< 0.60mm (dxf 60)*/
        width14 = 14,      /*!< 0.70mm (dxf 70)*/
        width15 = 15,      /*!< 0.80mm (dxf 80)*/
        width16 = 16,      /*!< 0.90mm (dxf 90)*/
        width17 = 17,      /*!< 1.00mm (dxf 100)*/
        width18 = 18,      /*!< 1.06mm (dxf 106)*/
        width19 = 19,      /*!< 1.20mm (dxf 120)*/
        width20 = 20,      /*!< 1.40mm (dxf 140)*/
        width21 = 21,      /*!< 1.58mm (dxf 158)*/
        width22 = 22,      /*!< 2.00mm (dxf 200)*/
        width23 = 23,      /*!< 2.11mm (dxf 211)*/
        widthByLayer = 29, /*!< by layer (dxf -1) */
        widthByBlock = 30, /*!< by block (dxf -2) */
        widthDefault = 31  /*!< by default (dxf -3) */
    };

    static auto lineWidth2dxfInt(lineWidth lw) -> int
    {
        switch (lw)
        {
            case widthByLayer:
                return -1;
            case widthByBlock:
                return -2;
            case widthDefault:
                return -3;
            case width00:
                return 0;
            case width01:
                return 5;
            case width02:
                return 9;
            case width03:
                return 13;
            case width04:
                return 15;
            case width05:
                return 18;
            case width06:
                return 20;
            case width07:
                return 25;
            case width08:
                return 30;
            case width09:
                return 35;
            case width10:
                return 40;
            case width11:
                return 50;
            case width12:
                return 53;
            case width13:
                return 60;
            case width14:
                return 70;
            case width15:
                return 80;
            case width16:
                return 90;
            case width17:
                return 100;
            case width18:
                return 106;
            case width19:
                return 120;
            case width20:
                return 140;
            case width21:
                return 158;
            case width22:
                return 200;
            case width23:
                return 211;
            default:
                break;
        }
        return -3;
    }

    static auto dxfInt2lineWidth(int i) -> lineWidth
    {
        if (i < 0)
        {
            if (i == -1)
                return widthByLayer;
            if (i == -2)
                return widthByBlock;
            else if (i == -3)
                return widthDefault;
        }
        else if (i < 3)
        {
            return width00;
        }
        else if (i < 7)
        {
            return width01;
        }
        else if (i < 11)
        {
            return width02;
        }
        else if (i < 14)
        {
            return width03;
        }
        else if (i < 16)
        {
            return width04;
        }
        else if (i < 19)
        {
            return width05;
        }
        else if (i < 22)
        {
            return width06;
        }
        else if (i < 27)
        {
            return width07;
        }
        else if (i < 32)
        {
            return width08;
        }
        else if (i < 37)
        {
            return width09;
        }
        else if (i < 45)
        {
            return width10;
        }
        else if (i < 52)
        {
            return width11;
        }
        else if (i < 57)
        {
            return width12;
        }
        else if (i < 65)
        {
            return width13;
        }
        else if (i < 75)
        {
            return width14;
        }
        else if (i < 85)
        {
            return width15;
        }
        else if (i < 95)
        {
            return width16;
        }
        else if (i < 103)
        {
            return width17;
        }
        else if (i < 112)
        {
            return width18;
        }
        else if (i < 130)
        {
            return width19;
        }
        else if (i < 149)
        {
            return width20;
        }
        else if (i < 180)
        {
            return width21;
        }
        else if (i < 205)
        {
            return width22;
        }
        else
        {
            return width23;
        }
        // default by default
        return widthDefault;
    }
};

#endif

// EOF
