/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2016-2022 A. Stebich (librecad@mail.lordofbikes.de)        **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef DRW_ENTITIES_H
#define DRW_ENTITIES_H

#include "drw_base.h"
#include <QtGlobal>
#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>

class dxfReader;
class DRW_Polyline;

namespace DRW
{

//! Entity's type.
enum ETYPE
{
    E3DFACE,
    //        E3DSOLID, //encripted propietry data
    //        ACAD_PROXY_ENTITY,
    ARC,
    ATTDEF,
    //        ATTRIB,
    BLOCK, // and ENDBLK
           //        BODY, //encripted propietry data
    CIRCLE,
    DIMENSION,
    DIMALIGNED,
    DIMLINEAR,
    DIMRADIAL,
    DIMDIAMETRIC,
    DIMANGULAR,
    DIMANGULAR3P,
    DIMORDINATE,
    ELLIPSE,
    HATCH,
    //        HELIX,
    IMAGE,
    INSERT,
    LEADER,
    //        LIGHT,
    LINE,
    LWPOLYLINE,
    //        MESH,
    //        MLINE,
    //        MLEADERSTYLE,
    //        MLEADER,
    MTEXT,
    //        OLEFRAME,
    //        OLE2FRAME,
    POINT,
    POLYLINE,
    RAY,
    //        REGION, //encripted propietry data
    //        SECTION,
    //        SEQEND,//not needed?? used in polyline and insert/attrib and dwg
    //        SHAPE,
    SOLID,
    SPLINE,
    //        SUN,
    //        SURFACE, //encripted propietry data can be four types
    //        TABLE,
    TEXT,
    //        TOLERANCE,
    TRACE,
    UNDERLAY,
    VERTEX,
    VIEWPORT,
    //        WIPEOUT, //WIPEOUTVARIABLE
    XLINE,
    ASTMNOTCH, // ASTM NOTCH
    UNKNOWN
};

} // namespace DRW
// only in DWG: MINSERT, 5 types of vertex, 4 types of polylines: 2d, 3d, pface & mesh
// shape, dictionary, MLEADER, MLEADERSTYLE

#define SETENTFRIENDS friend class dxfRW;

//! Base class for entities
/*!
 *  Base class for entities
 *  @author Rallaz
 */
class DRW_Entity
{
    SETENTFRIENDS

public:
    // initializes default values
    DRW_Entity()
      : eType(DRW::UNKNOWN),
        handle(DRW::NoHandle),
        appData(),
        parentHandle(DRW::NoHandle),
        space(DRW::ModelSpace),
        layer("0"),
        lineType("BYLAYER"),
        material(DRW::MaterialByLayer),
        color(DRW::ColorByLayer),
        lWeight(DRW_LW_Conv::widthByLayer),
        ltypeScale(1.0),
        visible(true),
        numProxyGraph(0),
        proxyGraphics(),
        color24(-1),
        colorName(),
        transparency(DRW::Opaque),
        plotStyle(DRW::DefaultPlotStyle),
        shadow(DRW::CastAndReceieveShadows),
        haveExtrusion(false),
        extData(),
        extAxisX(),
        extAxisY(),
        curr(nullptr)
    {
    }

    DRW_Entity(const DRW_Entity &e)
      : eType(e.eType),
        handle(e.handle),
        appData(),
        parentHandle(e.parentHandle),
        space(e.space),
        layer(e.layer),
        lineType(e.lineType),
        material(e.material),
        color(e.color),
        lWeight(e.lWeight),
        ltypeScale(e.ltypeScale),
        visible(e.visible),
        numProxyGraph(e.numProxyGraph),
        proxyGraphics(),
        color24(e.color24),
        colorName(),
        transparency(e.transparency),
        plotStyle(e.plotStyle),
        shadow(e.shadow),
        haveExtrusion(e.haveExtrusion),
        extData(),
        extAxisX(),
        extAxisY(),
        curr(nullptr /*e.curr*/)
    {
        for (auto it = e.extData.begin(); it != e.extData.end(); ++it)
        {
            extData.push_back(new DRW_Variant(*(*it)));
        }
    }

    virtual ~DRW_Entity()
    {
        for (auto it = extData.begin(); it != extData.end(); ++it)
            delete *it;

        extData.clear();
    }

    void reset()
    {
        for (auto it = extData.begin(); it != extData.end(); ++it)
            delete *it;
        extData.clear();
    }

    virtual void applyExtrusion() = 0;

    void setWidthMm(double millimeters)
    {
        if (millimeters < 0.0)
        {
            lWeight = DRW_LW_Conv::widthByLayer;
            return;
        }
        if (millimeters > 2.11)
            millimeters = 2.11;
        lWeight = DRW_LW_Conv::dxfInt2lineWidth(int(floor(millimeters * 100.0)));
    }

protected:
    // parses dxf pair to read entity
    virtual auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool;
    // calculates extrusion axis (normal vector)
    void calculateAxis(DRW_Coord extPoint);
    // apply extrusion to @extPoint and return data in @point
    void extrudePoint(DRW_Coord extPoint, DRW_Coord *point) const;

    // parses dxf 102 groups to read entity
    auto parseDxfGroups(int code, const std::unique_ptr<dxfReader> &reader) -> bool;

public:
    DRW::ETYPE eType;                          /*!< enum: entity type, code 0 */
    duint32 handle;                            /*!< entity identifier, code 5 */
    std::list<std::list<DRW_Variant>> appData; /*!< list of application data, code 102 */
    duint32 parentHandle;                      /*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
    DRW::Space space;                          /*!< space indicator, code 67*/
    UTF8STRING layer;                          /*!< layer name, code 8 */
    UTF8STRING lineType;                       /*!< line type, code 6 */
    duint32 material;                          /*!< hard pointer id to material object, code 347 */
    int color;                                 /*!< entity color, code 62 */
    DRW_LW_Conv::lineWidth lWeight;            /*!< entity lineweight, code 370 */
    double ltypeScale;                         /*!< linetype scale, code 48 */
    bool visible;                              /*!< entity visibility, code 60 */
    int numProxyGraph;                         /*!< Number of bytes in proxy graphics, code 92 */
    std::string proxyGraphics;                 /*!< proxy graphics bytes, code 310 */
    int color24;                               /*!< 24-bit color, code 420 */
    std::string colorName;                     /*!< color name, code 430 */
    int transparency;                          /*!< transparency, code 440 */
    int plotStyle;                             /*!< hard pointer id to plot style object, code 390 */
    DRW::ShadowMode shadow;                    /*!< shadow mode, code 284 */
    bool haveExtrusion;                        /*!< set to true if the entity have extrusion*/
    std::vector<DRW_Variant *> extData;        /*!< FIFO list of extended data, codes 1000 to 1071*/

private:
    auto operator=(const DRW_Entity &) -> DRW_Entity &Q_DECL_EQ_DELETE;
    DRW_Coord extAxisX;
    DRW_Coord extAxisY;
    DRW_Variant *curr;
};

//! Class to handle point entity
/*!
 *  Class to handle point entity
 *  @author Rallaz
 */
class DRW_Point : public DRW_Entity
{
    SETENTFRIENDS

public:
    DRW_Point() { eType = DRW::POINT; }

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    DRW_Coord basePoint{};                  /*!<  base point, code 10, 20 & 30 */
    double thickness{0};                    /*!<  thickness, code 39 */
    DRW_Coord extPoint{DRW_Coord(0, 0, 1)}; /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
    // TNick: we're not handling code 50 - Angle of the X axis for
    // the UCS in effect when the point was drawn
};

class DRW_ASTMNotch : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_ASTMNotch() { eType = DRW::ASTMNOTCH; }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    double angle{0}; /*!< angle, code 50 */
};

class DRW_ATTDEF : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_ATTDEF() { eType = DRW::ATTDEF; }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    DRW_Coord adjustmentPoint{}; /*!< alignment point, code 11, 21 & 31 */
    double height{0};            /*!< height text, code 40 */
    UTF8STRING text{};           /*!< text string, code 1 */
    UTF8STRING name{};           /*!< name, code 2 */
    UTF8STRING promptString{};   /*!< prompt string, code 3 */
    int flags{0};                /*!< flags, code 70 */
    int horizontalAdjustment{0}; /*!< Horizontal text justification, code 72, default 0 */
};

//! Class to handle line entity
/*!
 *  Class to handle line entity
 *  @author Rallaz
 */
class DRW_Line : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Line()
      : secPoint()
    {
        eType = DRW::LINE;
    }

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    DRW_Coord secPoint; /*!< second point, code 11, 21 & 31 */
};

//! Class to handle ray entity
/*!
 *  Class to handle ray entity
 *  @author Rallaz
 */
class DRW_Ray : public DRW_Line
{
    SETENTFRIENDS

public:
    DRW_Ray() { eType = DRW::RAY; }
};

//! Class to handle xline entity
/*!
 *  Class to handle xline entity
 *  @author Rallaz
 */
class DRW_Xline : public DRW_Ray
{
public:
    DRW_Xline() { eType = DRW::XLINE; }
};

//! Class to handle circle entity
/*!
 *  Class to handle circle entity
 *  @author Rallaz
 */
class DRW_Circle : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Circle()
      : radious()
    {
        eType = DRW::CIRCLE;
    }

    void applyExtrusion() override;

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    double radious; /*!< radius, code 40 */
};

//! Class to handle arc entity
/*!
 *  Class to handle arc entity
 *  @author Rallaz
 */
class DRW_Arc : public DRW_Circle
{
    SETENTFRIENDS

public:
    DRW_Arc()
      : staangle(),
        endangle(),
        isccw(1)
    {
        eType = DRW::ARC;
    }

    void applyExtrusion() override;

    //! center point in OCS
    auto center() const -> const DRW_Coord & { return basePoint; }
    //! the radius of the circle
    auto radius() const -> double { return radious; }
    //! start angle in radians
    auto startAngle() const -> double { return staangle; }
    //! end angle in radians
    auto endAngle() const -> double { return endangle; }
    //! thickness
    auto thick() const -> double { return thickness; }
    //! extrusion
    auto extrusion() const -> const DRW_Coord & { return extPoint; }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    double staangle; /*!< start angle, code 50 in radians*/
    double endangle; /*!< end angle, code 51 in radians */
    int isccw;       /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

//! Class to handle ellipse entity
/*!
 *  Class to handle ellipse and elliptic arc entity
 *  Note: start/end parameter are in radians for ellipse entity but
 *  for hatch boundary are in degrees
 *  @author Rallaz
 */
class DRW_Ellipse : public DRW_Line
{
    SETENTFRIENDS

public:
    DRW_Ellipse()
      : ratio(),
        staparam(),
        endparam(),
        isccw(1)
    {
        eType = DRW::ELLIPSE;
    }

    void toPolyline(DRW_Polyline *pol, int parts = 128);
    void applyExtrusion() override;

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

private:
    void correctAxis();

public:
    double ratio;    /*!< ratio, code 40 */
    double staparam; /*!< start parameter, code 41, 0.0 for full ellipse*/
    double endparam; /*!< end parameter, code 42, 2*PI for full ellipse */
    int isccw;       /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

//! Class to handle trace entity
/*!
 *  Class to handle trace entity
 *  @author Rallaz
 */
class DRW_Trace : public DRW_Line
{
    SETENTFRIENDS

public:
    DRW_Trace()
      : thirdPoint(),
        fourPoint()
    {
        eType = DRW::TRACE;
    }

    void applyExtrusion() override;

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    DRW_Coord thirdPoint; /*!< third point, code 12, 22 & 32 */
    DRW_Coord fourPoint;  /*!< four point, code 13, 23 & 33 */
};

//! Class to handle solid entity
/*!
 *  Class to handle solid entity
 *  @author Rallaz
 */
class DRW_Solid : public DRW_Trace
{
    SETENTFRIENDS

public:
    DRW_Solid() { eType = DRW::SOLID; }

    //! first corner (2D)
    auto firstCorner() const -> const DRW_Coord & { return basePoint; }
    //! second corner (2D)
    auto secondCorner() const -> const DRW_Coord & { return secPoint; }
    //! third corner (2D)
    auto thirdCorner() const -> const DRW_Coord & { return thirdPoint; }
    //! fourth corner (2D)
    auto fourthCorner() const -> const DRW_Coord & { return fourPoint; }
    //! thickness
    auto thick() const -> double { return thickness; }
    //! elevation
    auto elevation() const -> double { return basePoint.z; }
    //! extrusion
    auto extrusion() const -> const DRW_Coord & { return extPoint; }
};

//! Class to handle 3dface entity
/*!
 *  Class to handle 3dface entity
 *  @author Rallaz
 */
class DRW_3Dface : public DRW_Trace
{
    SETENTFRIENDS

public:
    enum InvisibleEdgeFlags
    {
        NoEdge = 0x00,
        FirstEdge = 0x01,
        SecodEdge = 0x02,
        ThirdEdge = 0x04,
        FourthEdge = 0x08,
        AllEdges = 0x0F
    };

    DRW_3Dface()
      : invisibleflag(0)
    {
        eType = DRW::E3DFACE;
    }

    void applyExtrusion() override {}

    //! first corner in WCS
    auto firstCorner() const -> const DRW_Coord & { return basePoint; }
    //! second corner in WCS
    auto secondCorner() const -> const DRW_Coord & { return secPoint; }
    //! third corner in WCS
    auto thirdCorner() const -> const DRW_Coord & { return thirdPoint; }
    //! fourth corner in WCS
    auto fourthCorner() const -> const DRW_Coord & { return fourPoint; }
    //! edge visibility flags
    auto edgeFlags() const -> InvisibleEdgeFlags { return static_cast<InvisibleEdgeFlags>(invisibleflag); }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    int invisibleflag; /*!< invisible edge flag, code 70 */
};

//! Class to handle block entries
/*!
 *  Class to handle block entries
 *  @author Rallaz
 */
class DRW_Block : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Block()
      : name("*U0"),
        flags(0)
    {
        eType = DRW::BLOCK;
        layer = '0';
    }

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    UTF8STRING name; /*!< block name, code 2 */
    int flags;       /*!< block type, code 70 */
};

//! Class to handle insert entries
/*!
 *  Class to handle insert entries
 *  @author Rallaz
 */
class DRW_Insert : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Insert()
      : name(),
        xscale(1),
        yscale(1),
        zscale(1),
        angle(0),
        colcount(1),
        rowcount(1),
        colspace(0),
        rowspace(0)
    {
        eType = DRW::INSERT;
    }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    UTF8STRING name; /*!< block name, code 2 */
    double xscale;   /*!< x scale factor, code 41 */
    double yscale;   /*!< y scale factor, code 42 */
    double zscale;   /*!< z scale factor, code 43 */
    double angle;    /*!< rotation angle in radians, code 50 */
    int colcount;    /*!< column count, code 70 */
    int rowcount;    /*!< row count, code 71 */
    double colspace; /*!< column space, code 44 */
    double rowspace; /*!< row space, code 45 */
};

//! Class to handle lwpolyline entity
/*!
 *  Class to handle lwpolyline entity
 *  @author Rallaz
 */
class DRW_LWPolyline : public DRW_Entity
{
    SETENTFRIENDS

public:
    DRW_LWPolyline()
      : vertexnum(),
        flags(0),
        width(0.0),
        elevation(0.0),
        thickness(0.0),
        extPoint(DRW_Coord(0, 0, 1)),
        vertex(nullptr),
        vertlist()
    {
        eType = DRW::LWPOLYLINE;
    }

    DRW_LWPolyline(const DRW_LWPolyline &p)
      : DRW_Entity(p),
        vertexnum(),
        flags(p.flags),
        width(p.width),
        elevation(p.elevation),
        thickness(p.thickness),
        extPoint(p.extPoint),
        vertex(nullptr),
        vertlist()
    {
        this->eType = DRW::LWPOLYLINE;
        for (unsigned i = 0; i < p.vertlist.size(); i++) // RLZ ok or new
            this->vertlist.push_back(new DRW_Vertex2D(*(p.vertlist.at(i))));
    }

    ~DRW_LWPolyline()
    {
        for (DRW_Vertex2D *item : vertlist)
            delete item;
    }
    void applyExtrusion() override;
    void addVertex(const DRW_Vertex2D &v)
    {
        auto *vert = new DRW_Vertex2D();
        vert->x = v.x;
        vert->y = v.y;
        vert->stawidth = v.stawidth;
        vert->endwidth = v.endwidth;
        vert->bulge = v.bulge;
        vertlist.push_back(vert);
    }
    auto addVertex() -> DRW_Vertex2D *
    {
        auto *vert = new DRW_Vertex2D();
        vert->stawidth = 0;
        vert->endwidth = 0;
        vert->bulge = 0;
        vertlist.push_back(vert);
        return vert;
    }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    int vertexnum;                        /*!< number of vertex, code 90 */
    int flags;                            /*!< polyline flag, code 70, default 0 */
    double width;                         /*!< constant width, code 43 */
    double elevation;                     /*!< elevation, code 38 */
    double thickness;                     /*!< thickness, code 39 */
    DRW_Coord extPoint;                   /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
    DRW_Vertex2D *vertex;                 /*!< current vertex to add data */
    std::vector<DRW_Vertex2D *> vertlist; /*!< vertex list */

private:
    auto operator=(const DRW_LWPolyline &) -> DRW_LWPolyline &Q_DECL_EQ_DELETE;
};

//! Class to handle insert entries
/*!
 *  Class to handle insert entries
 *  @author Rallaz
 */
class DRW_Text : public DRW_Line
{
    SETENTFRIENDS

public:
    //! Vertical alignments.
    enum VAlign
    {
        VBaseLine = 0, /*!< Top = 0 */
        VBottom,       /*!< Bottom = 1 */
        VMiddle,       /*!< Middle = 2 */
        VTop           /*!< Top = 3 */
    };

    //! Horizontal alignments.
    enum HAlign
    {
        HLeft = 0, /*!< Left = 0 */
        HCenter,   /*!< Centered = 1 */
        HRight,    /*!< Right = 2 */
        HAligned,  /*!< Aligned = 3 (if VAlign==0) */
        HMiddle,   /*!< middle = 4 (if VAlign==0) */
        HFit       /*!< fit into point = 5 (if VAlign==0) */
    };

    DRW_Text()
      : height(),
        text(),
        angle(0),
        widthscale(1),
        oblique(0),
        style("STANDARD"),
        textgen(0),
        alignH(HLeft),
        alignV(VBaseLine)
    {
        eType = DRW::TEXT;
    }

    void applyExtrusion() override {} // RLZ TODO

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    double height;     /*!< height text, code 40 */
    UTF8STRING text;   /*!< text string, code 1 */
    double angle;      /*!< rotation angle in degrees (360), code 50 */
    double widthscale; /*!< width factor, code 41 */
    double oblique;    /*!< oblique angle, code 51 */
    UTF8STRING style;  /*!< style name, code 7 */
    int textgen;       /*!< text generation, code 71 */
    HAlign alignH;     /*!< horizontal align, code 72 */
    VAlign alignV;     /*!< vertical align, code 73 */
};

//! Class to handle insert entries
/*!
 *  Class to handle insert entries
 *  @author Rallaz
 */
class DRW_MText : public DRW_Text
{
    SETENTFRIENDS

public:
    //! Attachments.
    enum Attach
    {
        TopLeft = 1,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleCenter,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    DRW_MText()
      : interlin(1),
        hasXAxisVec(false) // if true needed to recalculate angle
    {
        eType = DRW::MTEXT;
        alignV = static_cast<VAlign>(TopLeft);
        textgen = 1;
    }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;
    void updateAngle(); // recalculate angle if 'haveXAxis' is true

public:
    double interlin; /*!< width factor, code 44 */

private:
    bool hasXAxisVec;
};

//! Class to handle vertex
/*!
 *  Class to handle vertex  for polyline entity
 *  @author Rallaz
 */
class DRW_Vertex final : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Vertex()
    {
        eType = DRW::VERTEX;
    }

    DRW_Vertex(double sx, double sy, double sz = 0.0, double b = 0.0)
      : bulge(b)

    {
        eType = DRW::VERTEX;
        basePoint.x = sx;
        basePoint.y = sy;
        basePoint.z = sz;
    }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    double stawidth{0}; /*!< Start width, code 40 */
    double endwidth{0}; /*!< End width, code 41 */
    double bulge{0};    /*!< bulge, code 42 */

    int flags{0};      /*!< vertex flag, code 70, default 0 */
    double tgdir{0};   /*!< curve fit tangent direction, code 50 */
    int vindex1{0};    /*!< polyface mesh vertex index, code 71, default 0 */
    int vindex2{0};    /*!< polyface mesh vertex index, code 72, default 0 */
    int vindex3{0};    /*!< polyface mesh vertex index, code 73, default 0 */
    int vindex4{0};    /*!< polyface mesh vertex index, code 74, default 0 */
    int identifier{0}; /*!< vertex identifier, code 91, default 0 */
};

//! Class to handle polyline entity
/*!
 *  Class to handle polyline entity
 *  @author Rallaz
 */
class DRW_Polyline : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Polyline()
      : flags(0),
        defstawidth(0.0),
        defendwidth(0.0),
        vertexcount(0),
        facecount(0),
        smoothM(0),
        smoothN(0),
        curvetype(0),
        vertlist(),
        handlesList(),
        firstEH(),
        lastEH()
    {
        eType = DRW::POLYLINE;
        basePoint.x = basePoint.y = 0.0;
    }

    DRW_Polyline(const DRW_Polyline &p)
      : DRW_Point(p),
        flags(p.flags),
        defstawidth(p.defstawidth),
        defendwidth(p.defendwidth),
        vertexcount(p.vertexcount),
        facecount(p.facecount),
        smoothM(p.smoothM),
        smoothN(p.smoothN),
        curvetype(p.curvetype),
        vertlist(),
        handlesList(),
        firstEH(),
        lastEH()
    {
        for (unsigned i = 0; i < p.vertlist.size(); i++) // RLZ ok or new
            this->vertlist.push_back(new DRW_Vertex(*(p.vertlist.at(i))));
    }

    ~DRW_Polyline()
    {
        for (DRW_Vertex *item : vertlist)
            delete item;
    }
    void addVertex(const DRW_Vertex &v)
    {
        auto *vert = new DRW_Vertex();
        vert->basePoint.x = v.basePoint.x;
        vert->basePoint.y = v.basePoint.y;
        vert->basePoint.z = v.basePoint.z;
        vert->stawidth = v.stawidth;
        vert->endwidth = v.endwidth;
        vert->bulge = v.bulge;
        vertlist.push_back(vert);
    }
    void appendVertex(DRW_Vertex *v) { vertlist.push_back(v); }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    int flags;          /*!< polyline flag, code 70, default 0 */
    double defstawidth; /*!< Start width, code 40, default 0 */
    double defendwidth; /*!< End width, code 41, default 0 */
    int vertexcount;    /*!< polygon mesh M vertex or  polyface vertex num, code 71, default 0 */
    int facecount;      /*!< polygon mesh N vertex or  polyface face num, code 72, default 0 */
    int smoothM;        /*!< smooth surface M density, code 73, default 0 */
    int smoothN;        /*!< smooth surface M density, code 74, default 0 */
    int curvetype;      /*!< curves & smooth surface type, code 75, default 0 */

    std::vector<DRW_Vertex *> vertlist; /*!< vertex list */

private:
    auto operator=(const DRW_Polyline &) -> DRW_Polyline &Q_DECL_EQ_DELETE;
    std::list<duint32> handlesList; // list of handles, only in 2004+
    duint32 firstEH;                // handle of first entity, only in pre-2004
    duint32 lastEH;                 // handle of last entity, only in pre-2004
};

//! Class to handle spline entity
/*!
 *  Class to handle spline entity
 *  @author Rallaz
 */
class DRW_Spline : public DRW_Entity
{
    SETENTFRIENDS

public:
    DRW_Spline()
      : degree()
    {
        eType = DRW::SPLINE;
    }

    DRW_Spline(const DRW_Spline &p)
      : DRW_Entity(p),
        normalVec(p.normalVec),
        tgStart(p.tgStart),
        tgEnd(p.tgEnd),
        flags(p.flags),
        degree(p.degree),
        nknots(p.nknots),
        ncontrol(p.ncontrol),
        nfit(p.nfit),
        tolknot(p.tolknot),
        tolcontrol(p.tolcontrol),
        tolfit(p.tolfit)
    {
        eType = DRW::SPLINE;

        std::copy(p.knotslist.begin(), p.knotslist.end(), std::back_inserter(knotslist));
        std::copy(p.weightlist.begin(), p.weightlist.end(), std::back_inserter(weightlist));
        std::transform(p.controllist.cbegin(), p.controllist.cend(), std::back_inserter(controllist),
                       [](DRW_Coord *v) { return new DRW_Coord(*v); });
        std::transform(p.fitlist.cbegin(), p.fitlist.cend(), std::back_inserter(fitlist),
                       [](DRW_Coord *v) { return new DRW_Coord(*v); });
    }

    ~DRW_Spline()
    {
        for (DRW_Coord *item : controllist)
        {
            delete item;
        }
        for (DRW_Coord *item : fitlist)
        {
            delete item;
        }
    }
    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    //    double ex;                /*!< normal vector x coordinate, code 210 */
    //    double ey;                /*!< normal vector y coordinate, code 220 */
    //    double ez;                /*!< normal vector z coordinate, code 230 */
    DRW_Coord normalVec{}; /*!< normal vector, code 210, 220, 230 */
    DRW_Coord tgStart{};   /*!< start tangent, code 12, 22, 32 */
                           //    double tgsx;              /*!< start tangent x coordinate, code 12 */
                           //    double tgsy;              /*!< start tangent y coordinate, code 22 */
                           //    double tgsz;              /*!< start tangent z coordinate, code 32 */
    DRW_Coord tgEnd{};     /*!< end tangent, code 13, 23, 33 */
                           //    double tgex;              /*!< end tangent x coordinate, code 13 */
                           //    double tgey;              /*!< end tangent y coordinate, code 23 */
                           //    double tgez;              /*!< end tangent z coordinate, code 33 */
    int flags{0};          /*!< spline flag, code 70 */
    int degree;          /*!< degree of the spline, code 71 */
    dint32 nknots{0};    /*!< number of knots, code 72, default 0 */
    dint32 ncontrol{0};  /*!< number of control points, code 73, default 0 */
    dint32 nfit{0};      /*!< number of fit points, code 74, default 0 */
    double tolknot{0.0000001};    /*!< knot tolerance, code 42, default 0.0000001 */
    double tolcontrol{0.0000001}; /*!< control point tolerance, code 43, default 0.0000001 */
    double tolfit{0.0000001};     /*!< fit point tolerance, code 44, default 0.0000001 */

    std::vector<double> knotslist{};        /*!< knots list, code 40 */
    std::vector<double> weightlist{};       /*!< weight list, code 41 */
    std::vector<DRW_Coord *> controllist{}; /*!< control points list, code 10, 20 & 30 */
    std::vector<DRW_Coord *> fitlist{};     /*!< fit points list, code 11, 21 & 31 */

private:
    auto operator=(const DRW_Spline &) -> DRW_Spline &Q_DECL_EQ_DELETE;
    DRW_Coord *controlpoint{nullptr}; /*!< current control point to add data */
    DRW_Coord *fitpoint{nullptr};     /*!< current fit point to add data */
};

//! Class to handle hatch loop
/*!
 *  Class to handle hatch loop
 *  @author Rallaz
 */
class DRW_HatchLoop
{
public:
    explicit DRW_HatchLoop(int t)
      : type(t),
        numedges(0),
        objlist()
    {
    }

    ~DRW_HatchLoop()
    {
        // for(DRW_LWPolyline *item : pollist) delete item;
        for (DRW_Entity *item : objlist)
            delete item;
    }

    void update() { numedges = static_cast<int>(objlist.size()); }

public:
    int type;     /*!< boundary path type, code 92, polyline=2, default=0 */
    int numedges; /*!< number of edges (if not a polyline), code 93 */
                  // TODO: store lwpolylines as entities
    //     std::vector<DRW_LWPolyline *> pollist;  /*!< polyline list */
    std::vector<DRW_Entity *> objlist; /*!< entities list */
};

//! Class to handle hatch entity
/*!
 *  Class to handle hatch entity
 *  @author Rallaz
 */
// TODO: handle lwpolylines, splines and ellipses
class DRW_Hatch : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Hatch()
      : name(),
        solid(1),
        associative(0),
        hstyle(0),
        hpattern(1),
        doubleflag(0),
        loopsnum(0),
        angle(0.0),
        scale(0.0),
        deflines(0),
        looplist(),
        loop(nullptr),
        line(),
        arc(),
        ellipse(),
        spline(),
        pline(),
        pt(),
        plvert(),
        ispol()
    {
        eType = DRW::HATCH;
        basePoint.x = basePoint.y = basePoint.z = 0.0;
        clearEntities();
    }

    ~DRW_Hatch()
    {
        for (DRW_HatchLoop *item : looplist)
            delete item;
    }

    void appendLoop(DRW_HatchLoop *v) { looplist.push_back(v); }

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    UTF8STRING name; /*!< hatch pattern name, code 2 */
    int solid;       /*!< solid fill flag, code 70, solid=1, pattern=0 */
    int associative; /*!< associativity, code 71, associatve=1, non-assoc.=0 */
    int hstyle;      /*!< hatch style, code 75 */
    int hpattern;    /*!< hatch pattern type, code 76 */
    int doubleflag;  /*!< hatch pattern double flag, code 77, double=1, single=0 */
    int loopsnum;    /*!< namber of boundary paths (loops), code 91 */
    double angle;    /*!< hatch pattern angle, code 52 */
    double scale;    /*!< hatch pattern scale, code 41 */
    int deflines;    /*!< number of pattern definition lines, code 78 */

    std::vector<DRW_HatchLoop *> looplist; /*!< polyline list */

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_Hatch) // NOLINT
    void clearEntities()
    {
        pt = line = nullptr;
        pline = nullptr;
        arc = nullptr;
        ellipse = nullptr;
        spline = nullptr;
        plvert = nullptr;
    }

    void addLine()
    {
        clearEntities();
        if (loop)
        {
            pt = line = new DRW_Line;
            loop->objlist.push_back(line);
        }
    }

    void addArc()
    {
        clearEntities();
        if (loop)
        {
            pt = arc = new DRW_Arc;
            loop->objlist.push_back(arc);
        }
    }

    void addEllipse()
    {
        clearEntities();
        if (loop)
        {
            pt = ellipse = new DRW_Ellipse;
            loop->objlist.push_back(ellipse);
        }
    }

    void addSpline()
    {
        clearEntities();
        if (loop)
        {
            pt = nullptr;
            spline = new DRW_Spline;
            loop->objlist.push_back(spline);
        }
    }

    DRW_HatchLoop *loop; /*!< current loop to add data */
    DRW_Line *line;
    DRW_Arc *arc;
    DRW_Ellipse *ellipse;
    DRW_Spline *spline;
    DRW_LWPolyline *pline;
    DRW_Point *pt;
    DRW_Vertex2D *plvert;
    bool ispol;
};

//! Class to handle image entity
/*!
 *  Class to handle image entity
 *  @author Rallaz
 */
class DRW_Image : public DRW_Line
{
    SETENTFRIENDS

public:
    DRW_Image()
      : ref(),
        vVector(),
        sizeu(),
        sizev(),
        dz(),
        clip(0),
        brightness(50),
        contrast(50),
        fade(0)
    {
        eType = DRW::IMAGE;
    }

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    duint32 ref;       /*!< Hard reference to imagedef object, code 340 */
    DRW_Coord vVector; /*!< V-vector of single pixel, x coordinate, code 12, 22 & 32 */
                       //    double vx;                 /*!< V-vector of single pixel, x coordinate, code 12 */
                       //    double vy;                 /*!< V-vector of single pixel, y coordinate, code 22 */
                       //    double vz;                 /*!< V-vector of single pixel, z coordinate, code 32 */
    double sizeu;      /*!< image size in pixels, U value, code 13 */
    double sizev;      /*!< image size in pixels, V value, code 23 */
    double dz;         /*!< z coordinate, code 33 */
    int clip;          /*!< Clipping state, code 280, 0=off 1=on */
    int brightness;    /*!< Brightness value, code 281, (0-100) default 50 */
    int contrast;      /*!< Brightness value, code 282, (0-100) default 50 */
    int fade;          /*!< Brightness value, code 283, (0-100) default 0 */
};

//! Base class for dimension entity
/*!
 *  Base class for dimension entity
 *  @author Rallaz
 */
class DRW_Dimension : public DRW_Entity
{
    SETENTFRIENDS

public:
    DRW_Dimension()
      : type(0),
        name(),
        defPoint(),
        textPoint(),
        text(),
        style("STANDARD"),
        align(5),
        linesty(1),
        linefactor(1.0),
        rot(0.0),
        extPoint(DRW_Coord(0, 0, 1.0)),
        hdir(),
        clonePoint(),
        def1(),
        def2(),
        angle(0.0),
        oblique(0.0),
        circlePoint(),
        arcPoint(),
        length(0.0),
        hasActual(false),
        actual(0.0)
    {
        eType = DRW::DIMENSION;
    }

    DRW_Dimension(const DRW_Dimension &d)
      : DRW_Entity(d),
        type(d.type),
        name(d.name),
        defPoint(d.defPoint),
        textPoint(d.textPoint),
        text(d.text),
        style(d.style),
        align(d.align),
        linesty(d.linesty),
        linefactor(d.linefactor),
        rot(d.rot),
        extPoint(d.extPoint),
        hdir(),
        clonePoint(d.clonePoint),
        def1(d.def1),
        def2(d.def2),
        angle(d.angle),
        oblique(d.oblique),
        circlePoint(d.circlePoint),
        arcPoint(d.arcPoint),
        length(d.length),
        hasActual(d.hasActual),
        actual(d.actual)
    {
        eType = DRW::DIMENSION;
        // RLZ needed a def value for this: hdir = ???
    }
    ~DRW_Dimension() override = default;

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    auto getDefPoint() const -> DRW_Coord { return defPoint; } /*!< Definition point, code 10, 20 & 30 */
    void setDefPoint(const DRW_Coord &p) { defPoint = p; }
    auto getTextPoint() const -> DRW_Coord { return textPoint; } /*!< Middle point of text, code 11, 21 & 31 */
    void setTextPoint(const DRW_Coord &p) { textPoint = p; }
    auto getStyle() const -> std::string { return style; } /*!< Dimension style, code 3 */
    void setStyle(const std::string &s) { style = s; }
    auto getAlign() const -> int { return align; } /*!< attachment point, code 71 */
    void setAlign(const int a) { align = a; }
    auto getTextLineStyle() const -> int
    {
        return linesty;
    } /*!< Dimension text line spacing style, code 72, default 1 */
    void setTextLineStyle(const int l) { linesty = l; }
    auto getText() const -> std::string { return text; } /*!< Dimension text explicitly entered by the user, code 1 */
    void setText(const std::string &t) { text = t; }
    auto getTextLineFactor() const -> double
    {
        return linefactor;
    } /*!< Dimension text line spacing factor, code 41, default 1? */
    void setTextLineFactor(const double l) { linefactor = l; }
    auto getDir() const -> double
    {
        return rot;
    } /*!< rotation angle of the dimension text, code 53 (optional) default 0 */
    void setDir(const double d) { rot = d; }

    auto getExtrusion() const -> DRW_Coord { return extPoint; } /*!< extrusion, code 210, 220 & 230 */
    void setExtrusion(const DRW_Coord &p) { extPoint = p; }
    auto getName() const -> std::string { return name; } /*!< Name of the block that contains the entities, code 2 */
    void setName(const std::string &s) { name = s; }
    //    int getType(){ return type;}                      /*!< Dimension type, code 70 */
    auto hasActualMeasurement() const -> bool { return hasActual; }
    void setActualMeasurement(double value)
    {
        hasActual = true;
        actual = value;
    }
    auto getActualMeasurement() const -> double
    {
        return actual;
    } /*!< Actual measurement (optional; read-only value), code 42 */

protected:
    auto getPt2() const -> DRW_Coord { return clonePoint; }
    void setPt2(const DRW_Coord &p) { clonePoint = p; }
    auto getPt3() const -> DRW_Coord { return def1; }
    void setPt3(const DRW_Coord &p) { def1 = p; }
    auto getPt4() const -> DRW_Coord { return def2; }
    void setPt4(const DRW_Coord &p) { def2 = p; }
    auto getPt5() const -> DRW_Coord { return circlePoint; }
    void setPt5(const DRW_Coord &p) { circlePoint = p; }
    auto getPt6() const -> DRW_Coord { return arcPoint; }
    void setPt6(const DRW_Coord &p) { arcPoint = p; }
    auto getAn50() const -> double
    {
        return angle;
    } /*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
    void setAn50(const double d) { angle = d; }
    auto getOb52() const -> double { return oblique; } /*!< oblique angle, code 52 */
    void setOb52(const double d) { oblique = d; }
    auto getRa40() const -> double { return length; } /*!< Leader length, code 40 */
    void setRa40(const double d) { length = d; }

public:
    int type; /*!< Dimension type, code 70 */

private:
    auto operator=(const DRW_Dimension &) -> DRW_Dimension &Q_DECL_EQ_DELETE;
    std::string name;    /*!< Name of the block that contains the entities, code 2 */
    DRW_Coord defPoint;  /*!<  definition point, code 10, 20 & 30 (WCS) */
    DRW_Coord textPoint; /*!< Middle point of text, code 11, 21 & 31 (OCS) */
    UTF8STRING text;     /*!< Dimension text explicitly entered by the user, code 1 */
    UTF8STRING style;    /*!< Dimension style, code 3 */
    int align;           /*!< attachment point, code 71 */
    int linesty;         /*!< Dimension text line spacing style, code 72, default 1 */
    double linefactor;   /*!< Dimension text line spacing factor, code 41, default 1? (value range 0.25 to 4.00*/
    double rot;          /*!< rotation angle of the dimension text, code 53 */
    DRW_Coord extPoint;  /*!<  extrusion normal vector, code 210, 220 & 230 */

    double hdir;          /*!< horizontal direction for the dimension, code 51, default ? */
    DRW_Coord clonePoint; /*!< Insertion point for clones (Baseline & Continue), code 12, 22 & 32 (OCS) */
    DRW_Coord def1;       /*!< Definition point 1for linear & angular, code 13, 23 & 33 (WCS) */
    DRW_Coord def2;       /*!< Definition point 2, code 14, 24 & 34 (WCS) */
    double angle;         /*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
    double oblique;       /*!< oblique angle, code 52 */

    DRW_Coord circlePoint; /*!< Definition point for diameter, radius & angular dims code 15, 25 & 35 (WCS) */
    DRW_Coord arcPoint;    /*!< Point defining dimension arc, x coordinate, code 16, 26 & 36 (OCS) */
    double length;         /*!< Leader length, code 40 */
    bool hasActual;        /*!< Actual measurement has been read, code 42 */
    double actual;         /*!< Actual measurement (optional; read-only value), code 42 */
};

//! Class to handle  aligned dimension entity
/*!
 *  Class to handle aligned dimension entity
 *  @author Rallaz
 */
class DRW_DimAligned : public DRW_Dimension
{
    SETENTFRIENDS

public:
    DRW_DimAligned()
    {
        eType = DRW::DIMALIGNED;
        type = 1;
    }
    explicit DRW_DimAligned(const DRW_Dimension &d)
      : DRW_Dimension(d)
    {
        eType = DRW::DIMALIGNED;
    }

    auto getClonepoint() const -> DRW_Coord
    {
        return getPt2();
    } /*!< Insertion for clones (Baseline & Continue), 12, 22 & 32 */
    void setClonePoint(const DRW_Coord &c) { setPt2(c); }

    auto getDimPoint() const -> DRW_Coord { return getDefPoint(); } /*!< dim line location point, code 10, 20 & 30 */
    void setDimPoint(const DRW_Coord &p) { setDefPoint(p); }
    auto getDef1Point() const -> DRW_Coord { return getPt3(); } /*!< Definition point 1, code 13, 23 & 33 */
    void setDef1Point(const DRW_Coord &p) { setPt3(p); }
    auto getDef2Point() const -> DRW_Coord { return getPt4(); } /*!< Definition point 2, code 14, 24 & 34 */
    void setDef2Point(const DRW_Coord &p) { setPt4(p); }
};

//! Class to handle  linear or rotated dimension entity
/*!
 *  Class to handle linear or rotated dimension entity
 *  @author Rallaz
 */
class DRW_DimLinear : public DRW_DimAligned
{
public:
    DRW_DimLinear()
    {
        eType = DRW::DIMLINEAR;
        type = 0;
    }
    explicit DRW_DimLinear(const DRW_Dimension &d)
      : DRW_DimAligned(d)
    {
        eType = DRW::DIMLINEAR;
    }

    auto getAngle() const -> double
    {
        return getAn50();
    } /*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
    void setAngle(const double d) { setAn50(d); }
    auto getOblique() const -> double { return getOb52(); } /*!< oblique angle, code 52 */
    void setOblique(const double d) { setOb52(d); }
};

//! Class to handle radial dimension entity
/*!
 *  Class to handle aligned, linear or rotated dimension entity
 *  @author Rallaz
 */
class DRW_DimRadial : public DRW_Dimension
{
    SETENTFRIENDS

public:
    DRW_DimRadial()
    {
        eType = DRW::DIMRADIAL;
        type = 4;
    }
    explicit DRW_DimRadial(const DRW_Dimension &d)
      : DRW_Dimension(d)
    {
        eType = DRW::DIMRADIAL;
    }

    auto getCenterPoint() const -> DRW_Coord { return getDefPoint(); } /*!< center point, code 10, 20 & 30 */
    void setCenterPoint(const DRW_Coord &p) { setDefPoint(p); }
    auto getDiameterPoint() const -> DRW_Coord
    {
        return getPt5();
    } /*!< Definition point for radius, code 15, 25 & 35 */
    void setDiameterPoint(const DRW_Coord &p) { setPt5(p); }
    auto getLeaderLength() const -> double { return getRa40(); } /*!< Leader length, code 40 */
    void setLeaderLength(const double d) { setRa40(d); }
};

//! Class to handle radial dimension entity
/*!
 *  Class to handle aligned, linear or rotated dimension entity
 *  @author Rallaz
 */
class DRW_DimDiametric : public DRW_Dimension
{
    SETENTFRIENDS

public:
    DRW_DimDiametric()
    {
        eType = DRW::DIMDIAMETRIC;
        type = 3;
    }
    explicit DRW_DimDiametric(const DRW_Dimension &d)
      : DRW_Dimension(d)
    {
        eType = DRW::DIMDIAMETRIC;
    }

    auto getDiameter1Point() const -> DRW_Coord
    {
        return getPt5();
    } /*!< First definition point for diameter, code 15, 25 & 35 */
    void setDiameter1Point(const DRW_Coord &p) { setPt5(p); }
    auto getDiameter2Point() const -> DRW_Coord
    {
        return getDefPoint();
    } /*!< Opposite point for diameter, code 10, 20 & 30 */
    void setDiameter2Point(const DRW_Coord &p) { setDefPoint(p); }
    auto getLeaderLength() const -> double { return getRa40(); } /*!< Leader length, code 40 */
    void setLeaderLength(const double d) { setRa40(d); }
};

//! Class to handle angular dimension entity
/*!
 *  Class to handle angular dimension entity
 *  @author Rallaz
 */
class DRW_DimAngular : public DRW_Dimension
{
    SETENTFRIENDS

public:
    DRW_DimAngular()
    {
        eType = DRW::DIMANGULAR;
        type = 2;
    }
    explicit DRW_DimAngular(const DRW_Dimension &d)
      : DRW_Dimension(d)
    {
        eType = DRW::DIMANGULAR;
    }

    auto getFirstLine1() const -> DRW_Coord { return getPt3(); } /*!< Definition point line 1-1, code 13, 23 & 33 */
    void setFirstLine1(const DRW_Coord &p) { setPt3(p); }
    auto getFirstLine2() const -> DRW_Coord { return getPt4(); } /*!< Definition point line 1-2, code 14, 24 & 34 */
    void setFirstLine2(const DRW_Coord &p) { setPt4(p); }
    auto getSecondLine1() const -> DRW_Coord { return getPt5(); } /*!< Definition point line 2-1, code 15, 25 & 35 */
    void setSecondLine1(const DRW_Coord &p) { setPt5(p); }
    auto getSecondLine2() const -> DRW_Coord
    {
        return getDefPoint();
    } /*!< Definition point line 2-2, code 10, 20 & 30 */
    void setSecondLine2(const DRW_Coord &p) { setDefPoint(p); }
    auto getDimPoint() const -> DRW_Coord { return getPt6(); } /*!< Dimension definition point, code 16, 26 & 36 */
    void setDimPoint(const DRW_Coord &p) { setPt6(p); }
};

//! Class to handle angular 3p dimension entity
/*!
 *  Class to handle angular 3p dimension entity
 *  @author Rallaz
 */
class DRW_DimAngular3p : public DRW_Dimension
{
    SETENTFRIENDS

public:
    DRW_DimAngular3p()
    {
        eType = DRW::DIMANGULAR3P;
        type = 5;
    }
    explicit DRW_DimAngular3p(const DRW_Dimension &d)
      : DRW_Dimension(d)
    {
        eType = DRW::DIMANGULAR3P;
    }

    auto getFirstLine() const -> DRW_Coord { return getPt3(); } /*!< Definition point line 1, code 13, 23 & 33 */
    void setFirstLine(const DRW_Coord &p) { setPt3(p); }
    auto getSecondLine() const -> DRW_Coord { return getPt4(); } /*!< Definition point line 2, code 14, 24 & 34 */
    void setSecondLine(const DRW_Coord &p) { setPt4(p); }
    auto getVertexPoint() const -> DRW_Coord { return getPt5(); } /*!< Vertex point, code 15, 25 & 35 */
    void SetVertexPoint(const DRW_Coord &p) { setPt5(p); }
    auto getDimPoint() const -> DRW_Coord { return getDefPoint(); } /*!< Dimension definition point, code 10, 20 & 30 */
    void setDimPoint(const DRW_Coord &p) { setDefPoint(p); }
};

//! Class to handle ordinate dimension entity
/*!
 *  Class to handle ordinate dimension entity
 *  @author Rallaz
 */
class DRW_DimOrdinate : public DRW_Dimension
{
    SETENTFRIENDS

public:
    DRW_DimOrdinate()
    {
        eType = DRW::DIMORDINATE;
        type = 6;
    }
    explicit DRW_DimOrdinate(const DRW_Dimension &d)
      : DRW_Dimension(d)
    {
        eType = DRW::DIMORDINATE;
    }

    auto getOriginPoint() const -> DRW_Coord { return getDefPoint(); } /*!< Origin definition point, code 10, 20 & 30 */
    void setOriginPoint(const DRW_Coord &p) { setDefPoint(p); }
    auto getFirstLine() const -> DRW_Coord { return getPt3(); } /*!< Feature location point, code 13, 23 & 33 */
    void setFirstLine(const DRW_Coord &p) { setPt3(p); }
    auto getSecondLine() const -> DRW_Coord { return getPt4(); } /*!< Leader end point, code 14, 24 & 34 */
    void setSecondLine(const DRW_Coord &p) { setPt4(p); }
};

//! Class to handle leader entity
/*!
 *  Class to handle leader entity
 *  @author Rallaz
 */
class DRW_Leader : public DRW_Entity
{
    SETENTFRIENDS

public:
    DRW_Leader()
      : style(),
        arrow(1),
        leadertype(0),
        flag(3),
        hookline(),
        hookflag(0),
        textheight(),
        textwidth(),
        vertnum(0),
        coloruse(),
        annotHandle(),
        extrusionPoint(DRW_Coord(0, 0, 1.0)),
        horizdir(),
        offsetblock(),
        offsettext(),
        vertexlist(),
        vertexpoint()
    {
        eType = DRW::LEADER;
    }
    ~DRW_Leader()
    {
        for (DRW_Coord *item : vertexlist)
            delete item;
    }

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    UTF8STRING style;         /*!< Dimension style name, code 3 */
    int arrow;                /*!< Arrowhead flag, code 71, 0=Disabled; 1=Enabled */
    int leadertype;           /*!< Leader path type, code 72, 0=Straight line segments; 1=Spline */
    int flag;                 /*!< Leader creation flag, code 73, default 3 */
    int hookline;             /*!< Hook line direction flag, code 74, default 1 */
    int hookflag;             /*!< Hook line flag, code 75 */
    double textheight;        /*!< Text annotation height, code 40 */
    double textwidth;         /*!< Text annotation width, code 41 */
    int vertnum;              /*!< Number of vertices, code 76 */
    int coloruse;             /*!< Color to use if leader's DIMCLRD = BYBLOCK, code 77 */
    duint32 annotHandle;      /*!< Hard reference to associated annotation, code 340 */
    DRW_Coord extrusionPoint; /*!< Normal vector, code 210, 220 & 230 */
    DRW_Coord horizdir;       /*!< "Horizontal" direction for leader, code 211, 221 & 231 */
    DRW_Coord offsetblock;    /*!< Offset of last leader vertex from block, code 212, 222 & 232 */
    DRW_Coord offsettext;     /*!< Offset of last leader vertex from annotation, code 213, 223 & 233 */

    std::vector<DRW_Coord *> vertexlist; /*!< vertex points list, code 10, 20 & 30 */

private:
    Q_DISABLE_COPY_MOVE(DRW_Leader) // NOLINT
    DRW_Coord *vertexpoint;         /*!< current control point to add data */
};

//! Class to handle viewport entity
/*!
 *  Class to handle viewport entity
 *  @author Rallaz
 */
class DRW_Viewport : public DRW_Point
{
    SETENTFRIENDS

public:
    DRW_Viewport()
      : pswidth(205),
        psheight(156),
        vpstatus(0),
        vpID(),
        centerPX(128.5),
        centerPY(97.5),
        snapPX(),
        snapPY(),
        snapSpPX(),
        snapSpPY(),
        viewDir(),
        viewTarget(),
        viewLength(),
        frontClip(),
        backClip(),
        viewHeight(),
        snapAngle(),
        twistAngle(),
        frozenLyCount()
    {
        eType = DRW::VIEWPORT;
    }

    void applyExtrusion() override {}

protected:
    auto parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool override;

public:
    double pswidth;  /*!< Width in paper space units, code 40 */
    double psheight; /*!< Height in paper space units, code 41 */
    int vpstatus;    /*!< Viewport status, code 68 */
    int vpID;        /*!< Viewport ID, code 69 */
    double centerPX; /*!< view center point X, code 12 */
    double centerPY; /*!< view center point Y, code 22 */
    double snapPX;   /*!< Snap base point X, code 13 */
    double snapPY;   /*!< Snap base point Y, code 23 */
    double snapSpPX; /*!< Snap spacing X, code 14 */
    double snapSpPY; /*!< Snap spacing Y, code 24 */
    // TODO: complete in dxf
    DRW_Coord viewDir;    /*!< View direction vector, code 16, 26 & 36 */
    DRW_Coord viewTarget; /*!< View target point, code 17, 27, 37 */
    double viewLength;    /*!< Perspective lens length, code 42 */
    double frontClip;     /*!< Front clip plane Z value, code 43 */
    double backClip;      /*!< Back clip plane Z value, code 44 */
    double viewHeight;    /*!< View height in model space units, code 45 */
    double snapAngle;     /*!< Snap angle, code 50 */
    double twistAngle;    /*!< view twist angle, code 51 */

private:
    duint32 frozenLyCount;
}; // RLZ: missing 15,25, 72, 331, 90, 340, 1, 281, 71, 74, 110, 120, 130, 111, 121,131, 112,122, 132, 345,346, and
   // more...

// used  //DRW_Coord basePoint;      /*!<  base point, code 10, 20 & 30 */

// double thickness;         /*!< thickness, code 39 */
// DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
// enum DRW::ETYPE eType;     /*!< enum: entity type, code 0 */
// duint32 handle;            /*!< entity identifier, code 5 */
// std::list<std::list<DRW_Variant> > appData; /*!< list of application data, code 102 */
// duint32 parentHandle;      /*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
// DRW::Space space;          /*!< space indicator, code 67*/
// UTF8STRING layer;          /*!< layer name, code 8 */
// UTF8STRING lineType;       /*!< line type, code 6 */
// duint32 material;          /*!< hard pointer id to material object, code 347 */
// int color;                 /*!< entity color, code 62 */
// enum DRW_LW_Conv::lineWidth lWeight; /*!< entity lineweight, code 370 */
// double ltypeScale;         /*!< linetype scale, code 48 */
// bool visible;              /*!< entity visibility, code 60 */
// int numProxyGraph;         /*!< Number of bytes in proxy graphics, code 92 */
// std::string proxyGraphics; /*!< proxy graphics bytes, code 310 */
// int color24;               /*!< 24-bit color, code 420 */
// std::string colorName;     /*!< color name, code 430 */
// int transparency;          /*!< transparency, code 440 */
// int plotStyle;             /*!< hard pointer id to plot style object, code 390 */
// DRW::ShadowMode shadow;    /*!< shadow mode, code 284 */
// bool haveExtrusion;        /*!< set to true if the entity have extrusion*/

#endif

// EOF
