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

#ifndef LIBDXFRW_H
#define LIBDXFRW_H

#include "drw_entities.h"
#include "drw_header.h"
#include "drw_interface.h"
#include "drw_objects.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"

#include <string>
#include <unordered_map>

// Header <ciso646> is removed in C++20.
#if __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

class dxfRW
{
public:
    explicit dxfRW(const std::string &name);
    ~dxfRW();

    static void setDebug(DRW::DebugLevel lvl);
    /// reads the file specified in constructor
    /*!
     * An interface must be provided. It is used by the class to signal various
     * components being added.
     * @param interface_ the interface to use
     * @param ext should the extrusion be applied to convert in 2D?
     * @return true for success
     */
    auto read(DRW_Interface *interface_, bool ext) -> bool;
    void setBinary(bool b) { binFile = b; }
    void AddXSpaceBlock(bool add) { m_xSpaceBlock = add; }

    auto write(DRW_Interface *interface_, DRW::Version ver, bool bin) -> bool;
    auto writeLineType(DRW_LType *ent) -> bool;
    auto writeLayer(DRW_Layer *ent) -> bool;
    auto writeView(DRW_View *ent) -> bool;
    auto writeDimstyle(DRW_Dimstyle *ent) -> bool;
    auto writeTextstyle(DRW_Textstyle *ent) -> bool;
    auto writeVport(DRW_Vport *ent) -> bool;
    auto writeAppId(DRW_AppId *ent) -> bool;
    auto writePoint(DRW_Point *ent) -> bool;
    auto writeASTMNotch(DRW_ASTMNotch *ent) -> bool;
    auto writeATTDEF(DRW_ATTDEF *ent) -> bool;
    auto writeLine(DRW_Line *ent) -> bool;
    auto writeRay(DRW_Ray *ent) -> bool;
    auto writeXline(DRW_Xline *ent) -> bool;
    auto writeCircle(DRW_Circle *ent) -> bool;
    auto writeArc(DRW_Arc *ent) -> bool;
    auto writeEllipse(DRW_Ellipse *ent) -> bool;
    auto writeTrace(DRW_Trace *ent) -> bool;
    auto writeSolid(DRW_Solid *ent) -> bool;
    auto write3dface(DRW_3Dface *ent) -> bool;
    auto writeLWPolyline(DRW_LWPolyline *ent) -> bool;
    auto writePolyline(DRW_Polyline *ent) -> bool;
    auto writeSpline(DRW_Spline *ent) -> bool;
    auto writeBlockRecord(const std::string &name) -> bool;
    auto writeBlock(DRW_Block *bk) -> bool;
    auto writeInsert(DRW_Insert *ent) -> bool;
    auto writeMText(DRW_MText *ent) -> bool;
    auto writeText(DRW_Text *ent) -> bool;
    auto writeHatch(DRW_Hatch *ent) -> bool;
    auto writeViewport(DRW_Viewport *ent) -> bool;
    auto writeImage(DRW_Image *ent, const std::string &name) -> DRW_ImageDef *;
    auto writeLeader(DRW_Leader *ent) -> bool;
    auto writeDimension(DRW_Dimension *ent) -> bool;
    void setEllipseParts(int parts) { elParts = parts; } /*!< set parts number when convert ellipse to polyline */
    auto writePlotSettings(DRW_PlotSettings *ent) -> bool;

    auto ErrorString() const -> std::string;
    auto getVersion() const -> DRW::Version;
    auto getError() const -> DRW::error;

private:
    Q_DISABLE_COPY_MOVE(dxfRW) // NOLINT
    /// used by read() to parse the content of the file
    auto processDxf() -> bool;
    auto processHeader() -> bool;
    auto processTables() -> bool;
    auto processBlocks() -> bool;
    auto processBlock() -> bool;
    auto processEntities(bool isblock) -> bool;
    auto processObjects() -> bool;

    auto processLType() -> bool;
    auto processLayer() -> bool;
    auto processDimStyle() -> bool;
    auto processTextStyle() -> bool;
    auto processVports() -> bool;
    auto processView() -> bool;
    auto processAppId() -> bool;

    auto processPoint() -> bool;
    auto processLine() -> bool;
    auto processRay() -> bool;
    auto processXline() -> bool;
    auto processCircle() -> bool;
    auto processArc() -> bool;
    auto processEllipse() -> bool;
    auto processTrace() -> bool;
    auto processSolid() -> bool;
    auto processInsert() -> bool;
    auto processLWPolyline() -> bool;
    auto processPolyline() -> bool;
    auto processVertex(DRW_Polyline *pl) -> bool;
    auto processText() -> bool;
    auto processMText() -> bool;
    auto processHatch() -> bool;
    auto processSpline() -> bool;
    auto process3dface() -> bool;
    auto processViewport() -> bool;
    auto processImage() -> bool;
    auto processImageDef() -> bool;
    auto processDimension() -> bool;
    auto processLeader() -> bool;
    auto processPlotSettings() -> bool;

    //    bool writeHeader();
    auto writeEntity(DRW_Entity *ent) -> bool;
    auto writeTables() -> bool;
    auto writeBlocks() -> bool;
    auto writeObjects() -> bool;
    auto writeExtData(const std::vector<DRW_Variant *> &ed) -> bool;
    static auto toHexStr(int n) -> std::string; // RLZ removeme
    auto writeAppData(const std::list<std::list<DRW_Variant>> &appData) -> bool;

    auto setError(DRW::error lastError) -> bool;

private:
    DRW::Version version{};
    DRW::error error{DRW::BAD_NONE};
    std::string fileName;
    std::string codePage{};
    bool binFile{false};
    bool m_xSpaceBlock{true};
    std::unique_ptr<dxfReader> reader{};
    std::unique_ptr<dxfWriter> writer{};
    DRW_Interface *iface{nullptr};
    DRW_Header header{};
    //    int section;
    std::string nextentity{};
    int entCount{};
    bool wlayer0{};
    bool dimstyleStd{};
    bool applyExt{false};
    bool writingBlock{};
    int elParts{128}; /*!< parts number when convert ellipse to polyline */
    std::unordered_map<std::string, int> blockMap{};
    std::unordered_map<std::string, int> textStyleMap{};
    std::vector<DRW_ImageDef *> imageDef{}; /*!< imageDef list */

    int currHandle{};

    std::string errorString{};
};

#endif // LIBDXFRW_H
