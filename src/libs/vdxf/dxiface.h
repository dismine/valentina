/******************************************************************************
**                                                                           **
**  Copyright (C) 2015 José F. Soriano, rallazz@gmail.com                    **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef DX_IFACE_H
#define DX_IFACE_H

#include "dxfdef.h"
#include "libdxfrw/drw_interface.h"
#include "libdxfrw/libdxfrw.h"

#include <Qt>

class QFont;

// class to store image data and path from DRW_ImageDef
class dx_ifaceImg : public DRW_Image
{
public:
    dx_ifaceImg()
      : path()
    {
    }

    explicit dx_ifaceImg(const DRW_Image &p)
      : DRW_Image(p),
        path()
    {
    }

    virtual ~dx_ifaceImg() = default;
    std::string path; // stores the image path
};

// container class to store entites.
class dx_ifaceBlock final : public DRW_Block
{
public:
    dx_ifaceBlock()
      : ent()
    {
    }

    explicit dx_ifaceBlock(const DRW_Block &p)
      : DRW_Block(p),
        ent()
    {
    }

    virtual ~dx_ifaceBlock()
    {
        for (auto it = ent.begin(); it != ent.end(); ++it)
            delete *it;
    }
    std::list<DRW_Entity *> ent; // stores the entities list
};

// container class to store full dxf data.
class dx_data
{
public:
    dx_data()
      : headerC(),
        lineTypes(),
        layers(),
        dimStyles(),
        VPorts(),
        textStyles(),
        appIds(),
        blocks(),
        images(),
        mBlock(new dx_ifaceBlock())
    {
    }

    ~dx_data()
    {
        // cleanup,
        for (auto it = blocks.begin(); it != blocks.end(); ++it)
            delete *it;
        delete mBlock;
    }

    DRW_Header headerC;                  // stores a copy of the header vars
    std::list<DRW_LType> lineTypes;      // stores a copy of all line types
    std::list<DRW_Layer> layers;         // stores a copy of all layers
    std::list<DRW_Dimstyle> dimStyles;   // stores a copy of all dimension styles
    std::list<DRW_Vport> VPorts;         // stores a copy of all vports
    std::list<DRW_Textstyle> textStyles; // stores a copy of all text styles
    std::list<DRW_AppId> appIds;         // stores a copy of all line types
    std::list<dx_ifaceBlock *> blocks;   // stores a copy of all blocks and the entities in it
    std::list<dx_ifaceImg *> images;     // temporary list to find images for link with DRW_ImageDef. Do not delete it!!

    dx_ifaceBlock *mBlock; // container to store model entities

private:
    Q_DISABLE_COPY_MOVE(dx_data) // NOLINT
};

class dx_iface final : public DRW_Interface
{
public:
    dx_iface(const std::string &file, DRW::Version v, VarMeasurement varMeasurement, VarInsunits varInsunits);
    virtual ~dx_iface();
    auto fileExport(bool binary) -> bool;
    void writeEntity(DRW_Entity *e);

    void AddXSpaceBlock(bool add) { dxfW->AddXSpaceBlock(add); }

    auto ErrorString() const -> std::string;

    // reimplement virtual DRW_Interface functions
    // writer part, send all in class dx_data to writer
    virtual void writeHeader(DRW_Header &data) override;
    virtual void writeBlocks() override;
    // only send the name, needed by the reader to prepare handles of blocks & blockRecords
    virtual void writeBlockRecords() override;
    // write entities of model space and first paper_space
    virtual void writeEntities() override;
    virtual void writeLTypes() override;
    virtual void writeLayers() override;
    virtual void writeTextstyles() override;
    virtual void writeVports() override;
    virtual void writeDimstyles() override;
    virtual void writeObjects() override;
    virtual void writeAppId() override;

    void AddEntity(DRW_Entity *e);
    auto AddFont(const QFont &f) -> UTF8STRING;
    void AddBlock(dx_ifaceBlock *block);

    static auto QtPenStyleToString(Qt::PenStyle style) -> UTF8STRING;

    void AddQtLTypes();
    void AddDefLayers();
    void AddAAMALayers();
    void AddDefHeaderData();
    void AddAAMAHeaderData();
    void AddASTMLayers();

private:
    Q_DISABLE_COPY_MOVE(dx_iface) // NOLINT
    dxfRW *dxfW;                  // pointer to writer, needed to send data
    dx_data cData;                // class to store or read data
    DRW::Version version;

    void InitHeader(VarMeasurement varMeasurement, VarInsunits varInsunits);
    void InitTextstyles();
    void InitVPorts();
    void InitAppId();

    static auto LocaleToISO() -> std::string;
};

#endif // DX_IFACE_H
