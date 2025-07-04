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

#include "libdxfrw.h"
#include "intern/drw_dbg.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <QScopedPointer>

#if ((defined(__clang__) && (__clang_major__ >= 9)) || (!defined(__clang__) && defined(__GNUC__) && __GNUC__ >= 9) \
     || (defined(_MSC_VER) && (_MSC_VER >= 1920)))
#include <filesystem>
#endif

#define FIRSTHANDLE 48

/*enum sections {
    secUnknown,
    secHeader,
    secTables,
    secBlocks,
    secEntities,
    secObjects
};*/

dxfRW::dxfRW(const std::string &name)
  : fileName(name)
{
    DRW_DBGSL(DRW_dbg::Level::None);
}

dxfRW::~dxfRW()
{
    for (auto &it : imageDef)
    {
        delete it;
    }

    imageDef.clear();
}

void dxfRW::setDebug(DRW::DebugLevel lvl)
{
    switch (lvl)
    {
        case DRW::DebugLevel::Debug:
            DRW_DBGSL(DRW_dbg::Level::Debug);
            break;
        default:
            DRW_DBGSL(DRW_dbg::Level::None);
    }
}

auto dxfRW::read(DRW_Interface *interface_, bool ext) -> bool
{
    drw_assert(fileName.empty() == false);
    applyExt = ext;
    std::ifstream filestr;
    if (nullptr == interface_)
    {
        return setError(DRW::BAD_UNKNOWN);
    }
    DRW_DBG("dxfRW::read 1def\n");

#if ((defined(__clang__) && (__clang_major__ < 9)) \
     || (!defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 1))) \
     || (defined(_MSC_VER) && (_MSC_VER < 1920)))
    filestr.open(fileName.c_str(), std::ios_base::in | std::ios::binary);
#else
#if __cplusplus >= 202002L // C++20 or newer
    auto filePath = std::filesystem::path(fileName);
#else                      // C++17 and older
    auto filePath = std::filesystem::u8path(fileName);
#endif

    filestr.open(filePath, std::ios_base::in | std::ios::binary);
#endif
    if (!filestr.is_open() || !filestr.good())
    {
        return setError(DRW::BAD_OPEN);
    }

    char line[22];
    char line2[22] = "AutoCAD Binary DXF\r\n";
    line2[20] = static_cast<char>(26);
    line2[21] = '\0';
    filestr.read(line, 22);
    filestr.close();
    iface = interface_;
    DRW_DBG("dxfRW::read 2\n");
    if (strncmp(line, line2, 21) == 0)
    {
#if ((defined(__clang__) && (__clang_major__ < 9)) \
     || (!defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 1))) \
     || (defined(_MSC_VER) && (_MSC_VER < 1920)))
        filestr.open(fileName.c_str(), std::ios_base::in | std::ios::binary);
#else
        filestr.open(filePath, std::ios_base::in | std::ios::binary);
#endif
        binFile = true;
        // skip sentinel
        filestr.seekg(22, std::ios::beg);
        reader = std::make_unique<dxfReaderBinary>(&filestr);
        DRW_DBG("dxfRW::read binary file\n");
    }
    else
    {
        binFile = false;
#if ((defined(__clang__) && (__clang_major__ < 9)) \
     || (!defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 1))) \
     || (defined(_MSC_VER) && (_MSC_VER < 1920)))
        filestr.open(fileName.c_str(), std::ios_base::in);
#else
        filestr.open(filePath, std::ios_base::in);
#endif
        reader = std::make_unique<dxfReaderAscii>(&filestr);
    }

    bool const isOk = processDxf();
    filestr.close();
    reader.reset();
    reader = nullptr;
    return isOk;
}

auto dxfRW::write(DRW_Interface *interface_, DRW::Version ver, bool bin) -> bool
{
    std::ofstream filestr;
    version = ver;
    binFile = bin;
    iface = interface_;

    if (binFile)
    {
#if ((defined(__clang__) && (__clang_major__ < 9)) \
     || (!defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 1))) \
     || (defined(_MSC_VER) && (_MSC_VER < 1920)))
        filestr.open(fileName.c_str(), std::ios_base::out | std::ios::binary | std::ios::trunc);
#else
#if __cplusplus >= 202002L // C++20 or newer
        auto filePath = std::filesystem::path(fileName);
#else                      // C++17 and older
        auto filePath = std::filesystem::u8path(fileName);
#endif

        filestr.open(filePath, std::ios_base::out | std::ios::binary | std::ios::trunc);
#endif

        if (!filestr.is_open())
        {
            errorString = std::string("Error opening file: ") + strerror(errno);
            writer.reset();
            return false;
        }

        // write sentinel
        filestr << "AutoCAD Binary DXF\r\n" << static_cast<char>(26) << '\0';
        writer = std::make_unique<dxfWriterBinary>(&filestr);
        DRW_DBG("dxfRW::read binary file\n");
    }
    else
    {
#if ((defined(__clang__) && (__clang_major__ < 9)) \
     || (!defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 9 || (__GNUC__ == 9 && __GNUC_MINOR__ < 1))) \
     || (defined(_MSC_VER) && (_MSC_VER < 1920)))
        filestr.open(fileName.c_str(), std::ios_base::out | std::ios::trunc);
#else
#if __cplusplus >= 202002L // C++20 or newer
        auto filePath = std::filesystem::path(fileName);
#else                      // C++17 and older
        auto filePath = std::filesystem::u8path(fileName);
#endif

        filestr.open(filePath, std::ios_base::out | std::ios::trunc);
#endif

        if (!filestr.is_open())
        {
            errorString = std::string("Error opening file: ") + strerror(errno);
            writer.reset();
            return false;
        }

        writer = std::make_unique<dxfWriterAscii>(&filestr);
        std::string const comm = std::string("dxfrw ") + std::string(DRW_VERSION);
        writer->writeString(999, comm);
    }

    this->header = DRW_Header();
    iface->writeHeader(header);
    writer->writeString(0, "SECTION");
    entCount = FIRSTHANDLE;
    header.write(writer, version);
    writer->writeString(0, "ENDSEC");
    if (ver > DRW::AC1009)
    {
        writer->writeString(0, "SECTION");
        writer->writeString(2, "CLASSES");
        writer->writeString(0, "ENDSEC");
    }
    writer->writeString(0, "SECTION");
    writer->writeString(2, "TABLES");
    writeTables();
    writer->writeString(0, "ENDSEC");
    writer->writeString(0, "SECTION");
    writer->writeString(2, "BLOCKS");
    writeBlocks();
    writer->writeString(0, "ENDSEC");

    writer->writeString(0, "SECTION");
    writer->writeString(2, "ENTITIES");
    iface->writeEntities();
    writer->writeString(0, "ENDSEC");

    if (version > DRW::AC1009)
    {
        writer->writeString(0, "SECTION");
        writer->writeString(2, "OBJECTS");
        writeObjects();
        writer->writeString(0, "ENDSEC");
    }
    writer->writeString(0, "EOF");
    filestr.flush();

    if (filestr.fail())
    {
        errorString = "Error writing to file!";
        return false;
    }

    filestr.close();
    writer.reset();
    return true;
}

auto dxfRW::writeEntity(DRW_Entity *ent) -> bool
{
    // A handle is arbitrary, but in your DXF file unique hex value as string like ‘10FF’. It is common to to use
    // uppercase letters for hex numbers. Handle can have up to 16 hexadecimal digits (8 bytes).
    //
    // For DXF R10 until R12 the usage of handles was optional. The header variable $HANDLING set to 1 indicate the
    // usage of handles, else $HANDLING is 0 or missing.
    //
    // For DXF R13 and later the usage of handles is mandatory and the header variable $HANDLING was removed.
    if (version < DRW::AC1012)
    {
        int varInt = 0;
        if (header.getInt("$HANDLING", &varInt) && varInt != 0)
        {
            ent->handle = static_cast<duint32>(++entCount);
            writer->writeString(5, toHexStr(static_cast<int>(ent->handle)));
        }
    }
    else
    {
        ent->handle = static_cast<duint32>(++entCount);
        writer->writeString(5, toHexStr(static_cast<int>(ent->handle)));
    }

    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbEntity");
    }

    if (ent->space == 1)
    {
        writer->writeInt16(67, 1);
    }

    if (version > DRW::AC1009)
    {
        writer->writeUtf8String(8, ent->layer);
        writer->writeUtf8String(6, ent->lineType);
    }
    else
    {
        writer->writeUtf8Caps(8, ent->layer);
        writer->writeUtf8Caps(6, ent->lineType);
    }
    writer->writeInt16(62, ent->color);
    if (version > DRW::AC1015 && ent->color24 >= 0)
    {
        writer->writeInt32(420, ent->color24);
    }
    if (version > DRW::AC1014)
    {
        writer->writeInt16(370, DRW_LW_Conv::lineWidth2dxfInt(ent->lWeight));
    }
    if (version >= DRW::AC1014)
    {
        writeAppData(ent->appData);
    }
    return true;
}

auto dxfRW::writeAppData(const std::list<std::list<DRW_Variant>> &appData) -> bool
{
    for (const auto &group : appData)
    {
        // Search for application name
        bool found = false;

        for (const auto &data : group)
        {
            if (data.code == 102 && data.type == DRW_Variant::STRING)
            {
                writer->writeString(102, "{" + *(data.content.s));
                found = true;
                break;
            }
        }

        if (found)
        {
            for (const auto &data : group)
            {
                if (data.code == 102)
                {
                    continue;
                }

                switch (data.type)
                {
                    case DRW_Variant::STRING:
                        writer->writeString(data.code, *(data.content.s));
                        break;

                    case DRW_Variant::INTEGER:
                        writer->writeInt32(data.code, data.content.i);
                        break;

                    case DRW_Variant::DOUBLE:
                        writer->writeDouble(data.code, data.content.i);
                        break;

                    default:
                        break;
                }
            }

            writer->writeString(102, "}");
        }
    }
    return true;
}

auto dxfRW::writeLineType(DRW_LType *ent) -> bool
{
    std::string strname = ent->name;

    transform(strname.begin(), strname.end(), strname.begin(), ::toupper);
    // do not write linetypes handled by library
    if (strname == "BYLAYER" || strname == "BYBLOCK" || strname == "CONTINUOUS")
    {
        return true;
    }
    writer->writeString(0, "LTYPE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, toHexStr(++entCount));
        if (version > DRW::AC1012)
        {
            writer->writeString(330, "5");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbLinetypeTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
        writer->writeUtf8Caps(2, ent->name);
    writer->writeInt16(70, ent->flags);
    writer->writeUtf8String(3, ent->desc);
    ent->update();
    writer->writeInt16(72, 65);
    writer->writeInt16(73, ent->size);
    writer->writeDouble(40, ent->length);

    for (unsigned int i = 0; i < ent->path.size(); i++)
    {
        writer->writeDouble(49, ent->path.at(i));
        if (version > DRW::AC1009)
        {
            writer->writeInt16(74, 0);
        }
    }
    return true;
}

auto dxfRW::writeLayer(DRW_Layer *ent) -> bool
{
    writer->writeString(0, "LAYER");
    if (!wlayer0 && ent->name == "0")
    {
        wlayer0 = true;
        if (version > DRW::AC1009)
        {
            writer->writeString(5, "10");
        }
    }
    else
    {
        if (version > DRW::AC1009)
        {
            writer->writeString(5, toHexStr(++entCount));
        }
    }
    if (version > DRW::AC1012)
    {
        writer->writeString(330, "2");
    }
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbLayerTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
    {
        writer->writeUtf8Caps(2, ent->name);
    }
    writer->writeInt16(70, ent->flags);
    writer->writeInt16(62, ent->color);
    if (version > DRW::AC1015 && ent->color24 >= 0)
    {
        writer->writeInt32(420, ent->color24);
    }
    if (version > DRW::AC1009)
    {
        writer->writeUtf8String(6, ent->lineType);
        if (!ent->plotF)
            writer->writeBool(290, ent->plotF);
        writer->writeInt16(370, DRW_LW_Conv::lineWidth2dxfInt(ent->lWeight));
        writer->writeString(390, "F");
    }
    else
        writer->writeUtf8Caps(6, ent->lineType);
    if (!ent->extData.empty())
    {
        writeExtData(ent->extData);
    }
    //    writer->writeString(347, "10012");
    return true;
}

bool dxfRW::writeView(const DRW_View *ent)
{
    writer->writeString(0, "VIEW");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, toHexStr(++entCount));
    }
    if (version > DRW::AC1012)
    {
        writer->writeString(
            330, "42"); // 	Soft-pointer ID/handle to owner object, fixme - check whether id is fixed as for layers?
    }
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbViewTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
    {
        writer->writeUtf8Caps(2, ent->name);
    }
    writer->writeInt16(70, ent->flags);

    writer->writeDouble(40, ent->size.y);
    writer->writeDouble(10, ent->center.x);
    writer->writeDouble(20, ent->center.y);
    writer->writeDouble(41, ent->size.x);

    writer->writeDouble(11, ent->viewDirectionFromTarget.x);
    writer->writeDouble(21, ent->viewDirectionFromTarget.y);
    if (!qFuzzyIsNull(ent->viewDirectionFromTarget.z))
    {
        writer->writeDouble(31, ent->viewDirectionFromTarget.z);
    }

    writer->writeDouble(12, ent->targetPoint.x);
    writer->writeDouble(22, ent->targetPoint.y);
    if (!qFuzzyIsNull(ent->targetPoint.z))
    {
        writer->writeDouble(32, ent->targetPoint.z);
    }

    writer->writeDouble(42, ent->lensLen);
    writer->writeDouble(43, ent->frontClippingPlaneOffset);
    writer->writeDouble(44, ent->backClippingPlaneOffset);
    writer->writeDouble(50, ent->twistAngle);
    writer->writeInt16(71, ent->viewMode);
    writer->writeInt16(281, static_cast<int>(ent->renderMode));

    writer->writeBool(72, ent->hasUCS);
    writer->writeBool(73, ent->cameraPlottable);

    /*
     * fixme - investigate deep whether we should support these attributes
    writer->writeString(332, "42"); // Soft-pointer ID/handle to background object (optional)
    writer->writeString(334, "42"); // Soft-pointer ID/handle to live section object (optional)
    writer->writeString(348, "42"); // Hard-pointer ID/handle to visual style object (optional)
    */

    if (ent->hasUCS)
    {
        writer->writeDouble(110, ent->ucsOrigin.x);
        writer->writeDouble(120, ent->ucsOrigin.y);
        if (!qFuzzyIsNull(ent->ucsOrigin.z))
        {
            writer->writeDouble(130, ent->ucsOrigin.z);
        }

        writer->writeDouble(111, ent->ucsXAxis.x);
        writer->writeDouble(121, ent->ucsXAxis.y);
        if (!qFuzzyIsNull(ent->ucsXAxis.z))
        {
            writer->writeDouble(131, ent->ucsXAxis.z);
        }

        writer->writeDouble(112, ent->ucsYAxis.x);
        writer->writeDouble(122, ent->ucsYAxis.y);
        if (!qFuzzyIsNull(ent->ucsYAxis.z))
        {
            writer->writeDouble(132, ent->ucsYAxis.z);
        }

        writer->writeInt16(79, ent->ucsOrthoType);
        writer->writeDouble(146, ent->ucsElevation);

        /*
     * fixme - investigate deep whether we should support these attributes
    //ID/handle of AcDbUCSTableRecord if UCS is a named UCS. If not present, then UCS is unnamed (appears only if code 72 is set to 1)
    writer->writeString(345, "42");
    // Soft-pointer ID/handle to live section object (optional)
    writer->writeString(346, "42");
    */
    }
    return true;
}

auto dxfRW::writeTextstyle(DRW_Textstyle *ent) -> bool
{
    writer->writeString(0, "STYLE");
    // stringstream cause crash in OS/X, bug#3597944
    std::string name = ent->name;
    transform(name.begin(), name.end(), name.begin(), toupper);
    if (!dimstyleStd && name == "STANDARD")
    {
        // stringstream cause crash in OS/X, bug#3597944
        std::string name = ent->name;
        transform(name.begin(), name.end(), name.begin(), toupper);
        if (name == "STANDARD")
            dimstyleStd = true;
    }
    if (version > DRW::AC1009)
    {
        writer->writeString(5, toHexStr(++entCount));
        textStyleMap[name] = entCount;
    }

    if (version > DRW::AC1012)
    {
        writer->writeString(330, "2");
    }
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbTextStyleTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
    {
        writer->writeUtf8Caps(2, ent->name);
    }
    writer->writeInt16(70, ent->flags);
    writer->writeDouble(40, ent->height);
    writer->writeDouble(41, ent->width);
    writer->writeDouble(50, ent->oblique);
    writer->writeInt16(71, ent->genFlag);
    writer->writeDouble(42, ent->lastHeight);
    if (version > DRW::AC1009)
    {
        writer->writeUtf8String(3, ent->font);
        writer->writeUtf8String(4, ent->bigFont);
        writer->writeUtf8String(1001, "ACAD");
        writer->writeUtf8String(1000, ent->font);
        if (ent->fontFamily != 0)
            writer->writeInt32(1071, ent->fontFamily);
    }
    else
    {
        writer->writeUtf8Caps(3, ent->font);
        writer->writeUtf8Caps(4, ent->bigFont);
    }
    return true;
}

auto dxfRW::writeVport(DRW_Vport *ent) -> bool
{
    if (!dimstyleStd)
    {
        ent->name = "*ACTIVE";
        dimstyleStd = true;
    }
    writer->writeString(0, "VPORT");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, toHexStr(++entCount));
        if (version > DRW::AC1012)
            writer->writeString(330, "2");
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbViewportTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
        writer->writeUtf8Caps(2, ent->name);
    writer->writeInt16(70, ent->flags);
    writer->writeDouble(10, ent->lowerLeft.x);
    writer->writeDouble(20, ent->lowerLeft.y);
    writer->writeDouble(11, ent->UpperRight.x);
    writer->writeDouble(21, ent->UpperRight.y);
    writer->writeDouble(12, ent->center.x);
    writer->writeDouble(22, ent->center.y);
    writer->writeDouble(13, ent->snapBase.x);
    writer->writeDouble(23, ent->snapBase.y);
    writer->writeDouble(14, ent->snapSpacing.x);
    writer->writeDouble(24, ent->snapSpacing.y);
    writer->writeDouble(15, ent->gridSpacing.x);
    writer->writeDouble(25, ent->gridSpacing.y);
    writer->writeDouble(16, ent->viewDir.x);
    writer->writeDouble(26, ent->viewDir.y);
    writer->writeDouble(36, ent->viewDir.z);
    writer->writeDouble(17, ent->viewTarget.x);
    writer->writeDouble(27, ent->viewTarget.y);
    writer->writeDouble(37, ent->viewTarget.z);
    writer->writeDouble(40, ent->height);
    writer->writeDouble(41, ent->width);
    writer->writeDouble(42, ent->lensHeight);
    writer->writeDouble(43, ent->frontClip);
    writer->writeDouble(44, ent->backClip);
    writer->writeDouble(50, ent->snapAngle);
    writer->writeDouble(51, ent->twistAngle);
    writer->writeInt16(71, ent->viewMode);
    writer->writeInt16(72, ent->circleZoom);
    writer->writeInt16(73, ent->fastZoom);
    writer->writeInt16(74, ent->ucsIcon);
    writer->writeInt16(75, ent->snap);
    writer->writeInt16(76, ent->grid);
    writer->writeInt16(77, ent->snapStyle);
    writer->writeInt16(78, ent->snapIsopair);
    if (version > DRW::AC1014)
    {
        writer->writeInt16(281, 0);
        writer->writeInt16(65, 1);
        writer->writeDouble(110, 0.0);
        writer->writeDouble(120, 0.0);
        writer->writeDouble(130, 0.0);
        writer->writeDouble(111, 1.0);
        writer->writeDouble(121, 0.0);
        writer->writeDouble(131, 0.0);
        writer->writeDouble(112, 0.0);
        writer->writeDouble(122, 1.0);
        writer->writeDouble(132, 0.0);
        writer->writeInt16(79, 0);
        writer->writeDouble(146, 0.0);
        if (version > DRW::AC1018)
        {
            writer->writeString(348, "10020");
            writer->writeInt16(60, ent->gridBehavior); // v2007 undocummented see DRW_Vport class
            writer->writeInt16(61, 5);
            writer->writeBool(292, true);
            writer->writeInt16(282, 1);
            writer->writeDouble(141, 0.0);
            writer->writeDouble(142, 0.0);
            writer->writeInt16(63, 250);
            writer->writeInt32(421, 3358443);
        }
    }
    return true;
}

auto dxfRW::writeDimstyle(DRW_Dimstyle *ent) -> bool
{
    writer->writeString(0, "DIMSTYLE");
    if (!dimstyleStd)
    {
        std::string name = ent->name;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        if (name == "STANDARD")
            dimstyleStd = true;
    }
    if (version > DRW::AC1009)
    {
        writer->writeString(105, toHexStr(++entCount));
    }
    if (version > DRW::AC1012)
    {
        writer->writeString(330, "A");
    }
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbDimStyleTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
        writer->writeUtf8Caps(2, ent->name);
    writer->writeInt16(70, ent->flags);
    if (version == DRW::AC1009 || !(ent->dimpost.empty()))
        writer->writeUtf8String(3, ent->dimpost);
    if (version == DRW::AC1009 || !(ent->dimapost.empty()))
        writer->writeUtf8String(4, ent->dimapost);
    if (version == DRW::AC1009 || !(ent->dimblk.empty()))
        writer->writeUtf8String(5, ent->dimblk);
    if (version == DRW::AC1009 || !(ent->dimblk1.empty()))
        writer->writeUtf8String(6, ent->dimblk1);
    if (version == DRW::AC1009 || !(ent->dimblk2.empty()))
        writer->writeUtf8String(7, ent->dimblk2);
    writer->writeDouble(40, ent->dimscale);
    writer->writeDouble(41, ent->dimasz);
    writer->writeDouble(42, ent->dimexo);
    writer->writeDouble(43, ent->dimdli);
    writer->writeDouble(44, ent->dimexe);
    writer->writeDouble(45, ent->dimrnd);
    writer->writeDouble(46, ent->dimdle);
    writer->writeDouble(47, ent->dimtp);
    writer->writeDouble(48, ent->dimtm);
    if (version > DRW::AC1018 || !qFuzzyIsNull(ent->dimfxl))
        writer->writeDouble(49, ent->dimfxl);
    writer->writeDouble(140, ent->dimtxt);
    writer->writeDouble(141, ent->dimcen);
    writer->writeDouble(142, ent->dimtsz);
    writer->writeDouble(143, ent->dimaltf);
    writer->writeDouble(144, ent->dimlfac);
    writer->writeDouble(145, ent->dimtvp);
    writer->writeDouble(146, ent->dimtfac);
    writer->writeDouble(147, ent->dimgap);
    if (version > DRW::AC1014)
    {
        writer->writeDouble(148, ent->dimaltrnd);
    }
    writer->writeInt16(71, ent->dimtol);
    writer->writeInt16(72, ent->dimlim);
    writer->writeInt16(73, ent->dimtih);
    writer->writeInt16(74, ent->dimtoh);
    writer->writeInt16(75, ent->dimse1);
    writer->writeInt16(76, ent->dimse2);
    writer->writeInt16(77, ent->dimtad);
    writer->writeInt16(78, ent->dimzin);
    if (version > DRW::AC1014)
    {
        writer->writeInt16(79, ent->dimazin);
    }
    writer->writeInt16(170, ent->dimalt);
    writer->writeInt16(171, ent->dimaltd);
    writer->writeInt16(172, ent->dimtofl);
    writer->writeInt16(173, ent->dimsah);
    writer->writeInt16(174, ent->dimtix);
    writer->writeInt16(175, ent->dimsoxd);
    writer->writeInt16(176, ent->dimclrd);
    writer->writeInt16(177, ent->dimclre);
    writer->writeInt16(178, ent->dimclrt);
    if (version > DRW::AC1014)
    {
        writer->writeInt16(179, ent->dimadec);
    }
    if (version > DRW::AC1009)
    {
        if (version < DRW::AC1015)
            writer->writeInt16(270, ent->dimunit);
        writer->writeInt16(271, ent->dimdec);
        writer->writeInt16(272, ent->dimtdec);
        writer->writeInt16(273, ent->dimaltu);
        writer->writeInt16(274, ent->dimalttd);
        writer->writeInt16(275, ent->dimaunit);
    }
    if (version > DRW::AC1014)
    {
        writer->writeInt16(276, ent->dimfrac);
        writer->writeInt16(277, ent->dimlunit);
        writer->writeInt16(278, ent->dimdsep);
        writer->writeInt16(279, ent->dimtmove);
    }
    if (version > DRW::AC1009)
    {
        writer->writeInt16(280, ent->dimjust);
        writer->writeInt16(281, ent->dimsd1);
        writer->writeInt16(282, ent->dimsd2);
        writer->writeInt16(283, ent->dimtolj);
        writer->writeInt16(284, ent->dimtzin);
        writer->writeInt16(285, ent->dimaltz);
        writer->writeInt16(286, ent->dimaltttz);
        if (version < DRW::AC1015)
            writer->writeInt16(287, ent->dimfit);
        writer->writeInt16(288, ent->dimupt);
    }
    if (version > DRW::AC1014)
    {
        writer->writeInt16(289, ent->dimatfit);
    }
    if (version > DRW::AC1018 && ent->dimfxlon != 0)
        writer->writeInt16(290, ent->dimfxlon);
    if (version > DRW::AC1009)
    {
        std::string txstyname = ent->dimtxsty;
        std::transform(txstyname.begin(), txstyname.end(), txstyname.begin(), ::toupper);
        if (textStyleMap.count(txstyname) > 0)
        {
            int const txstyHandle = (*(textStyleMap.find(txstyname))).second;
            writer->writeUtf8String(340, toHexStr(txstyHandle));
        }
    }
    if (version > DRW::AC1014 && blockMap.count(ent->dimldrblk) > 0)
    {
        int const blkHandle = (*(blockMap.find(ent->dimldrblk))).second;
        writer->writeUtf8String(341, toHexStr(blkHandle));
        writer->writeInt16(371, ent->dimlwd);
        writer->writeInt16(372, ent->dimlwe);
    }
    return true;
}

auto dxfRW::writeAppId(DRW_AppId *ent) -> bool
{
    std::string strname = ent->name;
    transform(strname.begin(), strname.end(), strname.begin(), ::toupper);
    // do not write mandatory ACAD appId, handled by library
    if (strname == "ACAD")
        return true;
    writer->writeString(0, "APPID");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, toHexStr(++entCount));
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "9");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbRegAppTableRecord");
        writer->writeUtf8String(2, ent->name);
    }
    else
    {
        writer->writeUtf8Caps(2, ent->name);
    }
    writer->writeInt16(70, ent->flags);
    return true;
}

auto dxfRW::writePoint(DRW_Point *ent) -> bool
{
    writer->writeString(0, "POINT");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbPoint");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
    }
    return true;
}

auto dxfRW::writeASTMNotch(DRW_ASTMNotch *ent) -> bool
{
    writePoint(ent);
    writer->writeDouble(50, ent->angle);
    if (not qFuzzyIsNull(ent->thickness))
    {
        writer->writeDouble(39, ent->thickness); // Defined, but not used in point
    }

    return true;
}

auto dxfRW::writeATTDEF(DRW_ATTDEF *ent) -> bool
{
    writer->writeString(0, "ATTDEF");

    writeEntity(ent);

    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbText");
    }

    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
    }

    writer->writeDouble(11, ent->adjustmentPoint.x);
    writer->writeDouble(21, ent->adjustmentPoint.y);
    if (not qFuzzyIsNull(ent->adjustmentPoint.z))
    {
        writer->writeDouble(31, ent->adjustmentPoint.z);
    }

    writer->writeDouble(40, ent->height);
    writer->writeString(1, ent->text);

    UTF8STRING name = ent->name;
    std::replace(name.begin(), name.end(), ' ', '_');
    writer->writeString(2, name);

    writer->writeString(3, ent->promptString);
    writer->writeInt16(70, ent->flags);
    writer->writeInt16(73, ent->horizontalAdjustment);

    return true;
}

auto dxfRW::writeLine(DRW_Line *ent) -> bool
{
    writer->writeString(0, "LINE");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbLine");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z) || not qFuzzyIsNull(ent->secPoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(11, ent->secPoint.x);
        writer->writeDouble(21, ent->secPoint.y);
        writer->writeDouble(31, ent->secPoint.z);
    }
    else
    {
        writer->writeDouble(11, ent->secPoint.x);
        writer->writeDouble(21, ent->secPoint.y);
    }
    return true;
}

auto dxfRW::writeRay(DRW_Ray *ent) -> bool
{
    writer->writeString(0, "RAY");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbRay");
    }
    DRW_Coord crd = ent->secPoint;
    crd.unitize();
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z) || not qFuzzyIsNull(ent->secPoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(11, crd.x);
        writer->writeDouble(21, crd.y);
        writer->writeDouble(31, crd.z);
    }
    else
    {
        writer->writeDouble(11, crd.x);
        writer->writeDouble(21, crd.y);
    }
    return true;
}

auto dxfRW::writeXline(DRW_Xline *ent) -> bool
{
    writer->writeString(0, "XLINE");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbXline");
    }
    DRW_Coord crd = ent->secPoint;
    crd.unitize();
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z) || not qFuzzyIsNull(ent->secPoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(11, crd.x);
        writer->writeDouble(21, crd.y);
        writer->writeDouble(31, crd.z);
    }
    else
    {
        writer->writeDouble(11, crd.x);
        writer->writeDouble(21, crd.y);
    }
    return true;
}

auto dxfRW::writeCircle(DRW_Circle *ent) -> bool
{
    writer->writeString(0, "CIRCLE");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbCircle");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
    }
    writer->writeDouble(40, ent->radious);
    return true;
}

auto dxfRW::writeArc(DRW_Arc *ent) -> bool
{
    writer->writeString(0, "ARC");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbCircle");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
    }
    writer->writeDouble(40, ent->radious);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbArc");
    }
    writer->writeDouble(50, ent->staangle * ARAD);
    writer->writeDouble(51, ent->endangle * ARAD);
    return true;
}

auto dxfRW::writeEllipse(DRW_Ellipse *ent) -> bool
{
    // verify axis/ratio and params for full ellipse
    ent->correctAxis();
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "ELLIPSE");
        writeEntity(ent);
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbEllipse");
        }
        writer->writeDouble(10, ent->basePoint.x);
        writer->writeDouble(20, ent->basePoint.y);
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(11, ent->secPoint.x);
        writer->writeDouble(21, ent->secPoint.y);
        writer->writeDouble(31, ent->secPoint.z);
        writer->writeDouble(40, ent->ratio);
        writer->writeDouble(41, ent->staparam);
        writer->writeDouble(42, ent->endparam);
    }
    else
    {
        DRW_Polyline pol;
        // RLZ: copy properties
        ent->toPolyline(&pol, elParts);
        writePolyline(&pol);
    }
    return true;
}

auto dxfRW::writeTrace(DRW_Trace *ent) -> bool
{
    writer->writeString(0, "TRACE");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbTrace");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    writer->writeDouble(30, ent->basePoint.z);
    writer->writeDouble(11, ent->secPoint.x);
    writer->writeDouble(21, ent->secPoint.y);
    writer->writeDouble(31, ent->secPoint.z);
    writer->writeDouble(12, ent->thirdPoint.x);
    writer->writeDouble(22, ent->thirdPoint.y);
    writer->writeDouble(32, ent->thirdPoint.z);
    writer->writeDouble(13, ent->fourPoint.x);
    writer->writeDouble(23, ent->fourPoint.y);
    writer->writeDouble(33, ent->fourPoint.z);
    return true;
}

auto dxfRW::writeSolid(DRW_Solid *ent) -> bool
{
    writer->writeString(0, "SOLID");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbTrace");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    writer->writeDouble(30, ent->basePoint.z);
    writer->writeDouble(11, ent->secPoint.x);
    writer->writeDouble(21, ent->secPoint.y);
    writer->writeDouble(31, ent->secPoint.z);
    writer->writeDouble(12, ent->thirdPoint.x);
    writer->writeDouble(22, ent->thirdPoint.y);
    writer->writeDouble(32, ent->thirdPoint.z);
    writer->writeDouble(13, ent->fourPoint.x);
    writer->writeDouble(23, ent->fourPoint.y);
    writer->writeDouble(33, ent->fourPoint.z);
    return true;
}

auto dxfRW::write3dface(DRW_3Dface *ent) -> bool
{
    writer->writeString(0, "3DFACE");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbFace");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    writer->writeDouble(30, ent->basePoint.z);
    writer->writeDouble(11, ent->secPoint.x);
    writer->writeDouble(21, ent->secPoint.y);
    writer->writeDouble(31, ent->secPoint.z);
    writer->writeDouble(12, ent->thirdPoint.x);
    writer->writeDouble(22, ent->thirdPoint.y);
    writer->writeDouble(32, ent->thirdPoint.z);
    writer->writeDouble(13, ent->fourPoint.x);
    writer->writeDouble(23, ent->fourPoint.y);
    writer->writeDouble(33, ent->fourPoint.z);
    writer->writeInt16(70, ent->invisibleflag);
    return true;
}

auto dxfRW::writeLWPolyline(DRW_LWPolyline *ent) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "LWPOLYLINE");
        writeEntity(ent);
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbPolyline");
        }
        ent->vertexnum = static_cast<int>(ent->vertlist.size());
        writer->writeInt32(90, ent->vertexnum);
        writer->writeInt16(70, ent->flags);
        writer->writeDouble(43, ent->width);
        if (not qFuzzyIsNull(ent->elevation))
            writer->writeDouble(38, ent->elevation);
        if (not qFuzzyIsNull(ent->thickness))
            writer->writeDouble(39, ent->thickness);
        for (int i = 0; i < ent->vertexnum; i++)
        {
            DRW_Vertex2D *v = ent->vertlist.at(static_cast<size_t>(i));
            writer->writeDouble(10, v->x);
            writer->writeDouble(20, v->y);
            if (not qFuzzyIsNull(v->stawidth))
                writer->writeDouble(40, v->stawidth);
            if (not qFuzzyIsNull(v->endwidth))
                writer->writeDouble(41, v->endwidth);
            if (not qFuzzyIsNull(v->bulge))
                writer->writeDouble(42, v->bulge);
        }
    }
    else
    {
        // RLZ: TODO convert lwpolyline in polyline (not exist in acad 12)
    }
    return true;
}

auto dxfRW::writePolyline(DRW_Polyline *ent) -> bool
{
    writer->writeString(0, "POLYLINE");
    writeEntity(ent);
    bool is3d = false;
    if (version > DRW::AC1009)
    {
        if (ent->flags & 8 || ent->flags & 16)
        {
            writer->writeString(100, "AcDb3dPolyline");
            is3d = true;
        }
        else
        {
            writer->writeString(100, "AcDb2dPolyline");
        }
    }
    else
    {
        writer->writeInt16(66, 1);
    }
    writer->writeDouble(10, 0.0);
    writer->writeDouble(20, 0.0);
    writer->writeDouble(30, ent->basePoint.z);
    if (not qFuzzyIsNull(ent->thickness))
    {
        writer->writeDouble(39, ent->thickness);
    }
    writer->writeInt16(70, ent->flags);
    if (not qFuzzyIsNull(ent->defstawidth))
    {
        writer->writeDouble(40, ent->defstawidth);
    }
    if (not qFuzzyIsNull(ent->defendwidth))
    {
        writer->writeDouble(41, ent->defendwidth);
    }
    if (ent->flags & 16 || ent->flags & 32)
    {
        writer->writeInt16(71, ent->vertexcount);
        writer->writeInt16(72, ent->facecount);
    }
    if (ent->smoothM != 0)
    {
        writer->writeInt16(73, ent->smoothM);
    }
    if (ent->smoothN != 0)
    {
        writer->writeInt16(74, ent->smoothN);
    }
    if (ent->curvetype != 0)
    {
        writer->writeInt16(75, ent->curvetype);
    }
    if (DRW_Coord const crd = ent->extPoint;
        not qFuzzyIsNull(crd.x) || not qFuzzyIsNull(crd.y) || not DRW_FuzzyComparePossibleNulls(crd.z, 1))
    {
        writer->writeDouble(210, crd.x);
        writer->writeDouble(220, crd.y);
        writer->writeDouble(230, crd.z);
    }

    size_t const vertexnum = ent->vertlist.size();
    for (size_t i = 0; i < vertexnum; i++)
    {
        DRW_Vertex *v = ent->vertlist.at(i);
        writer->writeString(0, "VERTEX");
        writeEntity(ent);
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbVertex");
            if (is3d)
            {
                writer->writeString(100, "AcDb3dPolylineVertex");
            }
            else
            {
                writer->writeString(100, "AcDb2dVertex");
            }
        }
        if ((v->flags & 128) && !(v->flags & 64))
        {
            writer->writeDouble(10, 0);
            writer->writeDouble(20, 0);
            writer->writeDouble(30, 0);
        }
        else
        {
            writer->writeDouble(10, v->basePoint.x);
            writer->writeDouble(20, v->basePoint.y);
            writer->writeDouble(30, v->basePoint.z);
        }
        if (not qFuzzyIsNull(v->stawidth))
            writer->writeDouble(40, v->stawidth);
        if (not qFuzzyIsNull(v->endwidth))
            writer->writeDouble(41, v->endwidth);
        if (not qFuzzyIsNull(v->bulge))
            writer->writeDouble(42, v->bulge);
        if (v->flags != 0)
        {
            writer->writeInt16(70, ent->flags);
        }
        if (v->flags & 2)
        {
            writer->writeDouble(50, v->tgdir);
        }
        if (v->flags & 128)
        {
            if (v->vindex1 != 0)
            {
                writer->writeInt16(71, v->vindex1);
            }
            if (v->vindex2 != 0)
            {
                writer->writeInt16(72, v->vindex2);
            }
            if (v->vindex3 != 0)
            {
                writer->writeInt16(73, v->vindex3);
            }
            if (v->vindex4 != 0)
            {
                writer->writeInt16(74, v->vindex4);
            }
            if (!(v->flags & 64))
            {
                writer->writeInt32(91, v->identifier);
            }
        }
    }
    writer->writeString(0, "SEQEND");
    writeEntity(ent);
    return true;
}

auto dxfRW::writeSpline(DRW_Spline *ent) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "SPLINE");
        writeEntity(ent);
        writer->writeString(100, "AcDbSpline");
        writer->writeDouble(210, ent->normalVec.x);
        writer->writeDouble(220, ent->normalVec.y);
        writer->writeDouble(230, ent->normalVec.z);
        writer->writeInt16(70, ent->flags);
        writer->writeInt16(71, ent->degree);
        writer->writeInt16(72, ent->nknots);
        writer->writeInt16(73, ent->ncontrol);
        writer->writeInt16(74, ent->nfit);
        writer->writeDouble(42, ent->tolknot);
        writer->writeDouble(43, ent->tolcontrol);
        writer->writeDouble(44, ent->tolfit);
        // RLZ: warning check if nknots are correct and ncontrol
        for (int i = 0; i < ent->nknots; i++)
        {
            writer->writeDouble(40, ent->knotslist.at(static_cast<size_t>(i)));
        }
        for (int i = 0; i < static_cast<int>(ent->weightlist.size()); i++)
        {
            writer->writeDouble(41, ent->weightlist.at(static_cast<size_t>(i)));
        }
        for (int i = 0; i < ent->ncontrol; i++)
        {
            DRW_Coord *crd = ent->controllist.at(static_cast<size_t>(i));
            writer->writeDouble(10, crd->x);
            writer->writeDouble(20, crd->y);
            writer->writeDouble(30, crd->z);
        }
        for (int i = 0; i < ent->nfit; i++)
        {
            auto crd = ent->fitlist.at(static_cast<std::size_t>(i));
            writer->writeDouble(11, crd->x);
            writer->writeDouble(21, crd->y);
            writer->writeDouble(31, crd->z);
        }
    }
    else
    {
        // RLZ: TODO convert spline in polyline (not exist in acad 12)
    }
    return true;
}

auto dxfRW::writeHatch(DRW_Hatch *ent) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "HATCH");
        writeEntity(ent);
        writer->writeString(100, "AcDbHatch");
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(210, ent->extPoint.x);
        writer->writeDouble(220, ent->extPoint.y);
        writer->writeDouble(230, ent->extPoint.z);
        writer->writeString(2, ent->name);
        writer->writeInt16(70, ent->solid);
        writer->writeInt16(71, ent->associative);
        ent->loopsnum = static_cast<int>(ent->looplist.size());
        writer->writeInt16(91, ent->loopsnum);
        // write paths data
        for (int i = 0; i < ent->loopsnum; i++)
        {
            DRW_HatchLoop *loop = ent->looplist.at(static_cast<size_t>(i));
            writer->writeInt16(92, loop->type);
            if ((loop->type & 2) == 2)
            {
                // RLZ: polyline boundary writeme
            }
            else
            {
                // boundary path
                loop->update();
                writer->writeInt16(93, loop->numedges);
                for (int j = 0; j < loop->numedges; ++j)
                {
                    switch ((loop->objlist.at(static_cast<size_t>(j)))->eType)
                    {
                        case DRW::LINE:
                        {
                            writer->writeInt16(72, 1);
                            auto *l = static_cast<DRW_Line *>(loop->objlist.at(static_cast<size_t>(j)));
                            writer->writeDouble(10, l->basePoint.x);
                            writer->writeDouble(20, l->basePoint.y);
                            writer->writeDouble(11, l->secPoint.x);
                            writer->writeDouble(21, l->secPoint.y);
                            break;
                        }
                        case DRW::ARC:
                        {
                            writer->writeInt16(72, 2);
                            auto *a = static_cast<DRW_Arc *>(loop->objlist.at(static_cast<size_t>(j)));
                            writer->writeDouble(10, a->basePoint.x);
                            writer->writeDouble(20, a->basePoint.y);
                            writer->writeDouble(40, a->radious);
                            writer->writeDouble(50, a->staangle * ARAD);
                            writer->writeDouble(51, a->endangle * ARAD);
                            writer->writeInt16(73, a->isccw);
                            break;
                        }
                        case DRW::ELLIPSE:
                        {
                            writer->writeInt16(72, 3);
                            auto *a = static_cast<DRW_Ellipse *>(loop->objlist.at(static_cast<size_t>(j)));
                            a->correctAxis();
                            writer->writeDouble(10, a->basePoint.x);
                            writer->writeDouble(20, a->basePoint.y);
                            writer->writeDouble(11, a->secPoint.x);
                            writer->writeDouble(21, a->secPoint.y);
                            writer->writeDouble(40, a->ratio);
                            writer->writeDouble(50, a->staparam * ARAD);
                            writer->writeDouble(51, a->endparam * ARAD);
                            writer->writeInt16(73, a->isccw);
                            break;
                        }
                        case DRW::SPLINE:
                            // RLZ: spline boundary writeme
                            //                        writer->writeInt16(72, 4);
                            break;
                        default:
                            break;
                    }
                }
                writer->writeInt16(97, 0);
            }
        }
        writer->writeInt16(75, ent->hstyle);
        writer->writeInt16(76, ent->hpattern);
        if (!ent->solid)
        {
            writer->writeDouble(52, ent->angle);
            writer->writeDouble(41, ent->scale);
            writer->writeInt16(77, ent->doubleflag);
            writer->writeInt16(78, ent->deflines);
        }
        /*        if (ent->deflines > 0){
                    writer->writeInt16(78, ent->deflines);
                }*/
        writer->writeInt32(98, 0);
    }
    else
    {
        // RLZ: TODO verify in acad12
    }
    return true;
}

auto dxfRW::writeLeader(DRW_Leader *ent) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "LEADER");
        writeEntity(ent);
        writer->writeString(100, "AcDbLeader");
        writer->writeUtf8String(3, ent->style);
        writer->writeInt16(71, ent->arrow);
        writer->writeInt16(72, ent->leadertype);
        writer->writeInt16(73, ent->flag);
        writer->writeInt16(74, ent->hookline);
        writer->writeInt16(75, ent->hookflag);
        writer->writeDouble(40, ent->textheight);
        writer->writeDouble(41, ent->textwidth);
        writer->writeDouble(76, ent->vertnum);
        writer->writeDouble(76, static_cast<double>(ent->vertexlist.size()));
        for (unsigned int i = 0; i < ent->vertexlist.size(); i++)
        {
            DRW_Coord *vert = ent->vertexlist.at(i);
            writer->writeDouble(10, vert->x);
            writer->writeDouble(20, vert->y);
            writer->writeDouble(30, vert->z);
        }
    }
    else
    {
        // RLZ: todo not supported by acad 12 saved as unnamed block
    }
    return true;
}
auto dxfRW::writeDimension(DRW_Dimension *ent) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "DIMENSION");
        writeEntity(ent);
        writer->writeString(100, "AcDbDimension");
        if (!ent->getName().empty())
        {
            writer->writeString(2, ent->getName());
        }
        writer->writeDouble(10, ent->getDefPoint().x);
        writer->writeDouble(20, ent->getDefPoint().y);
        writer->writeDouble(30, ent->getDefPoint().z);
        writer->writeDouble(11, ent->getTextPoint().x);
        writer->writeDouble(21, ent->getTextPoint().y);
        writer->writeDouble(31, ent->getTextPoint().z);
        if (!(ent->type & 32))
            ent->type = ent->type + 32;
        writer->writeInt16(70, ent->type);
        if (!(ent->getText().empty()))
            writer->writeUtf8String(1, ent->getText());
        writer->writeInt16(71, ent->getAlign());
        if (ent->getTextLineStyle() != 1)
            writer->writeInt16(72, ent->getTextLineStyle());
        if (not DRW_FuzzyComparePossibleNulls(ent->getTextLineFactor(), 1))
            writer->writeDouble(41, ent->getTextLineFactor());
        writer->writeUtf8String(3, ent->getStyle());
        if (not qFuzzyIsNull(ent->getTextLineFactor()))
            writer->writeDouble(53, ent->getDir());
        writer->writeDouble(210, ent->getExtrusion().x);
        writer->writeDouble(220, ent->getExtrusion().y);
        writer->writeDouble(230, ent->getExtrusion().z);
        if (ent->hasActualMeasurement())
            writer->writeDouble(42, ent->getActualMeasurement());

        switch (ent->eType)
        {
            case DRW::DIMALIGNED:
            case DRW::DIMLINEAR:
            {
                auto *dd = static_cast<DRW_DimAligned *>(ent);
                writer->writeString(100, "AcDbAlignedDimension");
                if (DRW_Coord const crd = dd->getClonepoint();
                    not qFuzzyIsNull(crd.x) || not qFuzzyIsNull(crd.y) || not qFuzzyIsNull(crd.z))
                {
                    writer->writeDouble(12, crd.x);
                    writer->writeDouble(22, crd.y);
                    writer->writeDouble(32, crd.z);
                }
                writer->writeDouble(13, dd->getDef1Point().x);
                writer->writeDouble(23, dd->getDef1Point().y);
                writer->writeDouble(33, dd->getDef1Point().z);
                writer->writeDouble(14, dd->getDef2Point().x);
                writer->writeDouble(24, dd->getDef2Point().y);
                writer->writeDouble(34, dd->getDef2Point().z);
                if (ent->eType == DRW::DIMLINEAR)
                {
                    auto *dl = static_cast<DRW_DimLinear *>(ent);
                    if (not qFuzzyIsNull(dl->getAngle()))
                        writer->writeDouble(50, dl->getAngle());
                    if (not qFuzzyIsNull(dl->getOblique()))
                        writer->writeDouble(52, dl->getOblique());
                    writer->writeString(100, "AcDbRotatedDimension");
                }
                break;
            }
            case DRW::DIMRADIAL:
            {
                auto *dd = static_cast<DRW_DimRadial *>(ent);
                writer->writeString(100, "AcDbRadialDimension");
                writer->writeDouble(15, dd->getDiameterPoint().x);
                writer->writeDouble(25, dd->getDiameterPoint().y);
                writer->writeDouble(35, dd->getDiameterPoint().z);
                writer->writeDouble(40, dd->getLeaderLength());
                break;
            }
            case DRW::DIMDIAMETRIC:
            {
                auto *dd = static_cast<DRW_DimDiametric *>(ent);
                writer->writeString(100, "AcDbDiametricDimension");
                writer->writeDouble(15, dd->getDiameter1Point().x);
                writer->writeDouble(25, dd->getDiameter1Point().y);
                writer->writeDouble(35, dd->getDiameter1Point().z);
                writer->writeDouble(40, dd->getLeaderLength());
                break;
            }
            case DRW::DIMANGULAR:
            {
                auto *dd = static_cast<DRW_DimAngular *>(ent);
                writer->writeString(100, "AcDb2LineAngularDimension");
                writer->writeDouble(13, dd->getFirstLine1().x);
                writer->writeDouble(23, dd->getFirstLine1().y);
                writer->writeDouble(33, dd->getFirstLine1().z);
                writer->writeDouble(14, dd->getFirstLine2().x);
                writer->writeDouble(24, dd->getFirstLine2().y);
                writer->writeDouble(34, dd->getFirstLine2().z);
                writer->writeDouble(15, dd->getSecondLine1().x);
                writer->writeDouble(25, dd->getSecondLine1().y);
                writer->writeDouble(35, dd->getSecondLine1().z);
                writer->writeDouble(16, dd->getDimPoint().x);
                writer->writeDouble(26, dd->getDimPoint().y);
                writer->writeDouble(36, dd->getDimPoint().z);
                break;
            }
            case DRW::DIMANGULAR3P:
            {
                auto *dd = static_cast<DRW_DimAngular3p *>(ent);
                writer->writeDouble(13, dd->getFirstLine().x);
                writer->writeDouble(23, dd->getFirstLine().y);
                writer->writeDouble(33, dd->getFirstLine().z);
                writer->writeDouble(14, dd->getSecondLine().x);
                writer->writeDouble(24, dd->getSecondLine().y);
                writer->writeDouble(34, dd->getSecondLine().z);
                writer->writeDouble(15, dd->getVertexPoint().x);
                writer->writeDouble(25, dd->getVertexPoint().y);
                writer->writeDouble(35, dd->getVertexPoint().z);
                break;
            }
            case DRW::DIMORDINATE:
            {
                auto *dd = static_cast<DRW_DimOrdinate *>(ent);
                writer->writeString(100, "AcDbOrdinateDimension");
                writer->writeDouble(13, dd->getFirstLine().x);
                writer->writeDouble(23, dd->getFirstLine().y);
                writer->writeDouble(33, dd->getFirstLine().z);
                writer->writeDouble(14, dd->getSecondLine().x);
                writer->writeDouble(24, dd->getSecondLine().y);
                writer->writeDouble(34, dd->getSecondLine().z);
                break;
            }
            default:
                break;
        }
    }
    else
    {
        // RLZ: todo not supported by acad 12 saved as unnamed block
    }
    return true;
}

auto dxfRW::ErrorString() const -> std::string
{
    return errorString;
}

auto dxfRW::writeInsert(DRW_Insert *ent) -> bool
{
    writer->writeString(0, "INSERT");
    writeEntity(ent);

    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbBlockReference");
        writer->writeUtf8String(2, ent->name);
    }
    else
    {
        writer->writeUtf8Caps(2, ent->name);
    }

    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);

    if (!qFuzzyIsNull(ent->basePoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
    }

    if (!DRW_FuzzyComparePossibleNulls(ent->xscale, 1))
    {
        writer->writeDouble(41, ent->xscale);
    }

    if (!DRW_FuzzyComparePossibleNulls(ent->yscale, 1))
    {
        writer->writeDouble(42, ent->yscale);
    }

    if (!DRW_FuzzyComparePossibleNulls(ent->zscale, 1))
    {
        writer->writeDouble(43, ent->zscale);
    }

    if (!qFuzzyIsNull(ent->angle))
    {
        writer->writeDouble(50, (ent->angle) * ARAD); // in dxf angle is writed in degrees
    }

    if (!DRW_FuzzyComparePossibleNulls(ent->colcount, 1))
    {
        writer->writeInt16(70, ent->colcount);
    }

    if (!DRW_FuzzyComparePossibleNulls(ent->rowcount, 1))
    {
        writer->writeInt16(71, ent->rowcount);
    }

    if (!qFuzzyIsNull(ent->colspace))
    {
        writer->writeDouble(44, ent->colspace);
    }

    if (!qFuzzyIsNull(ent->rowspace))
    {
        writer->writeDouble(45, ent->rowspace);
    }
    return true;
}

auto dxfRW::writeText(DRW_Text *ent) -> bool
{
    writer->writeString(0, "TEXT");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbText");
    }
    //    writer->writeDouble(39, ent->thickness);
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z))
    {
        writer->writeDouble(30, ent->basePoint.z);
    }
    writer->writeDouble(40, ent->height);
    writer->writeUtf8String(1, ent->text);
    writer->writeDouble(50, ent->angle);
    writer->writeDouble(41, ent->widthscale);
    writer->writeDouble(51, ent->oblique);
    if (version > DRW::AC1009)
        writer->writeUtf8String(7, ent->style);
    else
        writer->writeUtf8Caps(7, ent->style);
    writer->writeInt16(71, ent->textgen);
    if (ent->alignH != DRW_Text::HLeft)
    {
        writer->writeInt16(72, ent->alignH);
    }
    if (ent->alignH != DRW_Text::HLeft || ent->alignV != DRW_Text::VBaseLine)
    {
        writer->writeDouble(11, ent->secPoint.x);
        writer->writeDouble(21, ent->secPoint.y);
        writer->writeDouble(31, ent->secPoint.z);
    }
    writer->writeDouble(210, ent->extPoint.x);
    writer->writeDouble(220, ent->extPoint.y);
    writer->writeDouble(230, ent->extPoint.z);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbText");
    }
    if (ent->alignV != DRW_Text::VBaseLine)
    {
        writer->writeInt16(73, ent->alignV);
    }
    return true;
}

auto dxfRW::writeMText(DRW_MText *ent) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "MTEXT");
        writeEntity(ent);
        writer->writeString(100, "AcDbMText");
        writer->writeDouble(10, ent->basePoint.x);
        writer->writeDouble(20, ent->basePoint.y);
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(40, ent->height);
        writer->writeDouble(41, ent->widthscale);
        writer->writeInt16(71, ent->textgen);
        writer->writeInt16(72, ent->alignH);
        std::string const text = writer->fromUtf8String(ent->text);

        int i;
        for (i = 0; (text.size() - static_cast<size_t>(i)) > 250;)
        {
            writer->writeString(3, text.substr(static_cast<size_t>(i), 250));
            i += 250;
        }
        writer->writeString(1, text.substr(static_cast<size_t>(i)));
        writer->writeString(7, ent->style);
        writer->writeDouble(210, ent->extPoint.x);
        writer->writeDouble(220, ent->extPoint.y);
        writer->writeDouble(230, ent->extPoint.z);
        writer->writeDouble(50, ent->angle);
        writer->writeInt16(73, ent->alignV);
        writer->writeDouble(44, ent->interlin);
        // RLZ ... 11, 21, 31 needed?
    }
    else
    {
        // RLZ: TODO convert mtext in text lines (not exist in acad 12)
    }
    return true;
}

auto dxfRW::writeViewport(DRW_Viewport *ent) -> bool
{
    writer->writeString(0, "VIEWPORT");
    writeEntity(ent);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbViewport");
    }
    writer->writeDouble(10, ent->basePoint.x);
    writer->writeDouble(20, ent->basePoint.y);
    if (not qFuzzyIsNull(ent->basePoint.z))
        writer->writeDouble(30, ent->basePoint.z);
    writer->writeDouble(40, ent->pswidth);
    writer->writeDouble(41, ent->psheight);
    writer->writeInt16(68, ent->vpstatus);
    writer->writeInt16(69, ent->vpID);
    writer->writeDouble(12, ent->centerPX); // RLZ: verify if exist in V12
    writer->writeDouble(22, ent->centerPY); // RLZ: verify if exist in V12
    return true;
}

auto dxfRW::writeImage(DRW_Image *ent, const std::string &name) -> DRW_ImageDef *
{
    if (version > DRW::AC1009)
    {
        // search if exist imagedef with this mane (image inserted more than 1 time)
        // RLZ: imagedef_reactor seem needed to read in acad
        DRW_ImageDef *id = nullptr;
        for (unsigned int i = 0; i < imageDef.size(); i++)
        {
            if (imageDef.at(i)->name == name)
            {
                id = imageDef.at(i);
            }
        }
        if (id == nullptr)
        {
            id = new DRW_ImageDef();
            imageDef.push_back(id);
            id->handle = static_cast<duint32>(++entCount);
        }
        id->fileName = name;
        std::string const idReactor = toHexStr(++entCount);

        writer->writeString(0, "IMAGE");
        writeEntity(ent);
        writer->writeString(100, "AcDbRasterImage");
        writer->writeDouble(10, ent->basePoint.x);
        writer->writeDouble(20, ent->basePoint.y);
        writer->writeDouble(30, ent->basePoint.z);
        writer->writeDouble(11, ent->secPoint.x);
        writer->writeDouble(21, ent->secPoint.y);
        writer->writeDouble(31, ent->secPoint.z);
        writer->writeDouble(12, ent->vVector.x);
        writer->writeDouble(22, ent->vVector.y);
        writer->writeDouble(32, ent->vVector.z);
        writer->writeDouble(13, ent->sizeu);
        writer->writeDouble(23, ent->sizev);
        writer->writeString(340, toHexStr(static_cast<int>(id->handle)));
        writer->writeInt16(70, 1);
        writer->writeInt16(280, ent->clip);
        writer->writeInt16(281, ent->brightness);
        writer->writeInt16(282, ent->contrast);
        writer->writeInt16(283, ent->fade);
        writer->writeString(360, idReactor);
        id->reactors[idReactor] = toHexStr(static_cast<int>(ent->handle));
        return id;
    }
    return nullptr; // not exist in acad 12
}

auto dxfRW::writeBlockRecord(const std::string &name) -> bool
{
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "BLOCK_RECORD");
        writer->writeString(5, toHexStr(++entCount));

        blockMap[name] = entCount;
        entCount = 2 + entCount; // reserve 2 for BLOCK & ENDBLOCK
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "1");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbBlockTableRecord");
        writer->writeUtf8String(2, name);
        if (version > DRW::AC1018)
        {
            //    writer->writeInt16(340, 22);
            writer->writeInt16(70, 0);
            writer->writeInt16(280, 1);
            writer->writeInt16(281, 0);
        }
    }
    return true;
}

auto dxfRW::writeBlock(DRW_Block *bk) -> bool
{
    if (writingBlock)
    {
        writer->writeString(0, "ENDBLK");
        if (version > DRW::AC1009)
        {
            writer->writeString(5, toHexStr(currHandle + 2));
            if (version > DRW::AC1014)
            {
                writer->writeString(330, toHexStr(currHandle));
            }
            writer->writeString(100, "AcDbEntity");
        }
        writer->writeString(8, bk->layer);
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbBlockEnd");
        }
    }
    writingBlock = true;
    writer->writeString(0, "BLOCK");
    if (version > DRW::AC1009)
    {
        currHandle = (*(blockMap.find(bk->name))).second;
        writer->writeString(5, toHexStr(currHandle + 1));
        if (version > DRW::AC1014)
        {
            writer->writeString(330, toHexStr(currHandle));
        }
        writer->writeString(100, "AcDbEntity");
    }
    writer->writeString(8, bk->layer);
    if (version > DRW::AC1009)
    {
        writer->writeString(100, "AcDbBlockBegin");
        writer->writeUtf8String(2, bk->name);
    }
    else
    {
        writer->writeUtf8Caps(2, bk->name);
    }
    writer->writeInt16(70, bk->flags);
    writer->writeDouble(10, bk->basePoint.x);
    writer->writeDouble(20, bk->basePoint.y);
    if (not qFuzzyIsNull(bk->basePoint.z))
    {
        writer->writeDouble(30, bk->basePoint.z);
    }
    if (version > DRW::AC1009)
    {
        writer->writeUtf8String(3, bk->name);
    }
    else
    {
        writer->writeUtf8Caps(3, bk->name);
    }
    writer->writeString(1, "");

    return true;
}

auto dxfRW::writeTables() -> bool
{
    writer->writeString(0, "TABLE");
    writer->writeString(2, "VPORT");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "8");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 1); // end table def
    /*** VPORT ***/
    dimstyleStd = false;
    iface->writeVports();
    if (!dimstyleStd)
    {
        DRW_Vport portact;
        portact.name = "*ACTIVE";
        writeVport(&portact);
    }
    writer->writeString(0, "ENDTAB");
    /*** LTYPE ***/
    writer->writeString(0, "TABLE");
    writer->writeString(2, "LTYPE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "5");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 4); // end table def
    // Mandatory linetypes
    writer->writeString(0, "LTYPE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "14");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "5");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbLinetypeTableRecord");
        writer->writeString(2, "ByBlock");
    }
    else
        writer->writeString(2, "BYBLOCK");
    writer->writeInt16(70, 0);
    writer->writeString(3, "");
    writer->writeInt16(72, 65);
    writer->writeInt16(73, 0);
    writer->writeDouble(40, 0.0);

    writer->writeString(0, "LTYPE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "15");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "5");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbLinetypeTableRecord");
        writer->writeString(2, "ByLayer");
    }
    else
        writer->writeString(2, "BYLAYER");
    writer->writeInt16(70, 0);
    writer->writeString(3, "");
    writer->writeInt16(72, 65);
    writer->writeInt16(73, 0);
    writer->writeDouble(40, 0.0);

    writer->writeString(0, "LTYPE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "16");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "5");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbLinetypeTableRecord");
        writer->writeString(2, "Continuous");
    }
    else
    {
        writer->writeString(2, "CONTINUOUS");
    }
    writer->writeInt16(70, 0);
    writer->writeString(3, "Solid line");
    writer->writeInt16(72, 65);
    writer->writeInt16(73, 0);
    writer->writeDouble(40, 0.0);
    // Application linetypes
    writer->writeString(0, "ENDTAB");
    /*** LAYER ***/
    writer->writeString(0, "TABLE");
    writer->writeString(2, "LAYER");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "2");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 1); // end table def
    wlayer0 = false;
    iface->writeLayers();
    if (!wlayer0 && version > DRW::AC1009)
    {
        DRW_Layer lay0;
        lay0.name = '0';
        writeLayer(&lay0);
    }
    writer->writeString(0, "ENDTAB");
    /*** STYLE ***/
    writer->writeString(0, "TABLE");
    writer->writeString(2, "STYLE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "3");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 3); // end table def
    dimstyleStd = false;
    iface->writeTextstyles();
    if (!dimstyleStd)
    {
        DRW_Textstyle tsty;
        tsty.name = "Standard";
        writeTextstyle(&tsty);
    }
    writer->writeString(0, "ENDTAB");

    writer->writeString(0, "TABLE");
    writer->writeString(2, "VIEW");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "6");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 0); // end table def
    iface->writeViews();
    writer->writeString(0, "ENDTAB");

    writer->writeString(0, "TABLE");
    writer->writeString(2, "UCS");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "7");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 0); // end table def
    writer->writeString(0, "ENDTAB");

    writer->writeString(0, "TABLE");
    writer->writeString(2, "APPID");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "9");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 1); // end table def
    writer->writeString(0, "APPID");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "12");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "9");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbRegAppTableRecord");
    }
    writer->writeString(2, "ACAD");
    writer->writeInt16(70, 0);
    iface->writeAppId();
    writer->writeString(0, "ENDTAB");

    writer->writeString(0, "TABLE");
    writer->writeString(2, "DIMSTYLE");
    if (version > DRW::AC1009)
    {
        writer->writeString(5, "A");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
    }
    writer->writeInt16(70, 1); // end table def
    if (version > DRW::AC1014)
    {
        writer->writeString(100, "AcDbDimStyleTable");
        writer->writeInt16(71, 1); // end table def
    }
    dimstyleStd = false;
    iface->writeDimstyles();
    if (!dimstyleStd)
    {
        DRW_Dimstyle dsty;
        dsty.name = "Standard";
        writeDimstyle(&dsty);
    }
    writer->writeString(0, "ENDTAB");

    if (version > DRW::AC1009)
    {
        writer->writeString(0, "TABLE");
        writer->writeString(2, "BLOCK_RECORD");
        writer->writeString(5, "1");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "0");
        }
        writer->writeString(100, "AcDbSymbolTable");
        writer->writeInt16(70, 2); // end table def
        writer->writeString(0, "BLOCK_RECORD");
        writer->writeString(5, "1F");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "1");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbBlockTableRecord");
        writer->writeString(2, "*Model_Space");
        if (version > DRW::AC1018)
        {
            //    writer->writeInt16(340, 22);
            writer->writeInt16(70, 0);
            writer->writeInt16(280, 1);
            writer->writeInt16(281, 0);
        }
        writer->writeString(0, "BLOCK_RECORD");
        writer->writeString(5, "1E");
        if (version > DRW::AC1014)
        {
            writer->writeString(330, "1");
        }
        writer->writeString(100, "AcDbSymbolTableRecord");
        writer->writeString(100, "AcDbBlockTableRecord");
        writer->writeString(2, "*Paper_Space");
        if (version > DRW::AC1018)
        {
            //    writer->writeInt16(340, 22);
            writer->writeInt16(70, 0);
            writer->writeInt16(280, 1);
            writer->writeInt16(281, 0);
        }
    }
    /* always call writeBlockRecords to iface for prepare unnamed blocks */
    iface->writeBlockRecords();
    if (version > DRW::AC1009)
    {
        writer->writeString(0, "ENDTAB");
    }
    return true;
}

auto dxfRW::writeBlocks() -> bool
{
    if (version > DRW::AC1009 || m_xSpaceBlock)
    {
        writer->writeString(0, "BLOCK");
        if (version > DRW::AC1009)
        {
            writer->writeString(5, "20");
            if (version > DRW::AC1014)
            {
                writer->writeString(330, "1F");
            }
            writer->writeString(100, "AcDbEntity");
        }
        writer->writeString(8, "0");
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbBlockBegin");
            writer->writeString(2, "*Model_Space");
        }
        else
        {
            writer->writeString(2, "$MODEL_SPACE");
        }
        writer->writeInt16(70, 0);
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
        if (version > DRW::AC1009)
        {
            writer->writeString(3, "*Model_Space");
        }
        else
        {
            writer->writeString(3, "$MODEL_SPACE");
        }
        writer->writeString(1, "");
        writer->writeString(0, "ENDBLK");
        if (version > DRW::AC1009)
        {
            writer->writeString(5, "21");
            if (version > DRW::AC1014)
            {
                writer->writeString(330, "1F");
            }
            writer->writeString(100, "AcDbEntity");
        }
        writer->writeString(8, "0");
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbBlockEnd");
        }

        writer->writeString(0, "BLOCK");
        if (version > DRW::AC1009)
        {
            writer->writeString(5, "1C");
            if (version > DRW::AC1014)
            {
                writer->writeString(330, "1B");
            }
            writer->writeString(100, "AcDbEntity");
        }
        writer->writeString(8, "0");
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbBlockBegin");
            writer->writeString(2, "*Paper_Space");
        }
        else
        {
            writer->writeString(2, "$PAPER_SPACE");
        }
        writer->writeInt16(70, 0);
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
        if (version > DRW::AC1009)
        {
            writer->writeString(3, "*Paper_Space");
        }
        else
        {
            writer->writeString(3, "$PAPER_SPACE");
        }
        writer->writeString(1, "");
        writer->writeString(0, "ENDBLK");
        if (version > DRW::AC1009)
        {
            writer->writeString(5, "1D");
            if (version > DRW::AC1014)
            {
                writer->writeString(330, "1F");
            }
            writer->writeString(100, "AcDbEntity");
        }
        writer->writeString(8, "0");
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbBlockEnd");
        }
    }
    writingBlock = false;
    iface->writeBlocks();
    if (writingBlock)
    {
        writingBlock = false;
        writer->writeString(0, "ENDBLK");
        if (version > DRW::AC1009)
        {
            writer->writeString(5, toHexStr(currHandle + 2));
            //            writer->writeString(5, "1D");
            if (version > DRW::AC1014)
            {
                writer->writeString(330, toHexStr(currHandle));
            }
            writer->writeString(100, "AcDbEntity");
        }
        writer->writeString(8, "0");
        if (version > DRW::AC1009)
        {
            writer->writeString(100, "AcDbBlockEnd");
        }
    }
    return true;
}

auto dxfRW::writeObjects() -> bool
{
    writer->writeString(0, "DICTIONARY");
    std::string imgDictH;
    writer->writeString(5, "C");
    if (version > DRW::AC1014)
    {
        writer->writeString(330, "0");
    }
    writer->writeString(100, "AcDbDictionary");
    writer->writeInt16(281, 1);
    writer->writeString(3, "ACAD_GROUP");
    writer->writeString(350, "D");
    if (!imageDef.empty())
    {
        writer->writeString(3, "ACAD_IMAGE_DICT");
        imgDictH = toHexStr(++entCount);
        writer->writeString(350, imgDictH);
    }
    writer->writeString(0, "DICTIONARY");
    writer->writeString(5, "D");
    writer->writeString(330, "C");
    writer->writeString(100, "AcDbDictionary");
    writer->writeInt16(281, 1);
    // write IMAGEDEF_REACTOR
    for (unsigned int i = 0; i < imageDef.size(); i++)
    {
        DRW_ImageDef *id = imageDef.at(i);
        for (auto it = id->reactors.begin(); it != id->reactors.end(); ++it)
        {
            writer->writeString(0, "IMAGEDEF_REACTOR");
            writer->writeString(5, (*it).first);
            writer->writeString(330, (*it).second);
            writer->writeString(100, "AcDbRasterImageDefReactor");
            writer->writeInt16(90, 2); // version 2=R14 to v2010
            writer->writeString(330, (*it).second);
        }
    }
    if (!imageDef.empty())
    {
        writer->writeString(0, "DICTIONARY");
        writer->writeString(5, imgDictH);
        writer->writeString(330, "C");
        writer->writeString(100, "AcDbDictionary");
        writer->writeInt16(281, 1);
        for (unsigned int i = 0; i < imageDef.size(); i++)
        {
            size_t f1, f2;
            f1 = imageDef.at(i)->name.find_last_of("/\\");
            f2 = imageDef.at(i)->name.find_last_of('.');
            ++f1;
            writer->writeString(3, imageDef.at(i)->name.substr(f1, f2 - f1));
            writer->writeString(350, toHexStr(static_cast<int>(imageDef.at(i)->handle)));
        }
    }
    for (unsigned int i = 0; i < imageDef.size(); i++)
    {
        DRW_ImageDef *id = imageDef.at(i);
        writer->writeString(0, "IMAGEDEF");
        writer->writeString(5, toHexStr(static_cast<int>(id->handle)));
        if (version > DRW::AC1014)
        {
            //            writer->writeString(330, '0'); handle to DICTIONARY
        }
        writer->writeString(102, "{ACAD_REACTORS");
        for (auto it = id->reactors.begin(); it != id->reactors.end(); ++it)
        {
            writer->writeString(330, (*it).first);
        }
        writer->writeString(102, "}");
        writer->writeString(100, "AcDbRasterImageDef");
        writer->writeInt16(90, 0); // version 0=R14 to v2010
        writer->writeUtf8String(1, id->fileName);
        writer->writeDouble(10, id->u);
        writer->writeDouble(20, id->v);
        writer->writeDouble(11, id->up);
        writer->writeDouble(21, id->vp);
        writer->writeInt16(280, id->loaded);
        writer->writeInt16(281, id->resolution);
    }
    // no more needed imageDef, delete it
    while (!imageDef.empty())
    {
        imageDef.pop_back();
    }

    iface->writeObjects();

    return true;
}

auto dxfRW::writeExtData(const std::vector<DRW_Variant *> &ed) -> bool
{
    for (auto it = ed.begin(); it != ed.end(); ++it)
    {
        switch ((*it)->code)
        {
            case 1000:
            case 1001:
            case 1002:
            case 1003:
            case 1004:
            case 1005:
            {
                int const cc = (*it)->code;
                if ((*it)->type == DRW_Variant::STRING)
                    writer->writeUtf8String(cc, *(*it)->content.s);
                //            writer->writeUtf8String((*it)->code, (*it)->content.s);
                break;
            }
            case 1010:
            case 1011:
            case 1012:
            case 1013:
                if ((*it)->type == DRW_Variant::COORD)
                {
                    writer->writeDouble((*it)->code, (*it)->content.v->x);
                    writer->writeDouble((*it)->code + 10, (*it)->content.v->y);
                    writer->writeDouble((*it)->code + 20, (*it)->content.v->z);
                }
                break;
            case 1040:
            case 1041:
            case 1042:
                if ((*it)->type == DRW_Variant::DOUBLE)
                    writer->writeDouble((*it)->code, (*it)->content.d);
                break;
            case 1070:
                if ((*it)->type == DRW_Variant::INTEGER)
                    writer->writeInt16((*it)->code, (*it)->content.i);
                break;
            case 1071:
                if ((*it)->type == DRW_Variant::INTEGER)
                    writer->writeInt32((*it)->code, (*it)->content.i);
                break;
            default:
                break;
        }
    }
    return true;
}

/********* Reader Process *********/

auto dxfRW::processDxf() -> bool
{
    DRW_DBG("dxfRW::processDxf() start processing dxf\n");
    int code = -1;
    bool inSection = false;

    reader->setIgnoreComments(false);
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG(" code\n");
        /* at this level we should only get:
         999 - Comment
         0 - SECTION or EOF
         2 - section name
         everything else between "2 - section name" and "0 - ENDSEC" is handled in process() methods
        */
        switch (code)
        {
            case 999: // when DXF was created by libdxfrw, first record is a comment with dxfrw version info
                header.addComment(reader->getString());
                continue;

            case 0:
                // ignore further comments, as libdxfrw doesn't support comments in sections
                reader->setIgnoreComments(true);
                if (!inSection)
                {
                    std::string const sectionstr{reader->getString()};

                    if ("SECTION" == sectionstr)
                    {
                        DRW_DBG(sectionstr);
                        DRW_DBG(" new section\n");
                        inSection = true;
                        continue;
                    }
                    if ("EOF" == sectionstr)
                    {
                        return true; // found EOF terminate
                    }
                }
                else
                {
                    // in case SECTION was unknown or not supported
                    if ("ENDSEC" == reader->getString())
                    {
                        inSection = false;
                    }
                }
                break;

            case 2:
                if (inSection)
                {
                    bool processed{false};
                    std::string const sectionname{reader->getString()};

                    DRW_DBG(sectionname);
                    DRW_DBG(" process section\n");
                    if ("HEADER" == sectionname)
                    {
                        processed = processHeader();
                    }
                    else if ("TABLES" == sectionname)
                    {
                        processed = processTables();
                    }
                    else if ("BLOCKS" == sectionname)
                    {
                        processed = processBlocks();
                    }
                    else if ("ENTITIES" == sectionname)
                    {
                        processed = processEntities(false);
                    }
                    else if ("OBJECTS" == sectionname)
                    {
                        processed = processObjects();
                    }
                    else
                    {
                        // TODO handle CLASSES

                        DRW_DBG("section unknown or not supported\n");
                        continue;
                    }

                    if (!processed)
                    {
                        DRW_DBG("  failed\n");
                        return setError(DRW::BAD_READ_SECTION);
                    }

                    inSection = false;
                }
                continue;

            default:
                // landing here means an unknown or not supported SECTION
                inSection = false;
                break;
        }
        /*    if (!more)
                return true;*/
    }

    if (0 == code && "EOF" == reader->getString())
    {
        // in case the final EOF has no newline we end up here!
        // this is caused by filestr->good() which is false for missing newline on EOF
        return true;
    }

    return setError(DRW::BAD_UNKNOWN);
}

/********* Header Section *********/

auto dxfRW::processHeader() -> bool
{
    DRW_DBG("dxfRW::processHeader\n");
    int code;
    std::string sectionstr;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG(" processHeader\n");
        if (code == 0)
        {
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG(" processHeader\n\n");
            if (sectionstr == "ENDSEC")
            {
                iface->addHeader(&header);
                return true; // found ENDSEC terminate
            }
            DRW_DBG("unexpected 0 code in header!\n");
            return setError(DRW::BAD_READ_HEADER);
        }

        if (!header.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_HEADER);
}

/********* Tables Section *********/

auto dxfRW::processTables() -> bool
{
    DRW_DBG("dxfRW::processTables\n");
    int code;
    std::string sectionstr;
    bool more = true;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG(" processHeader\n\n");
            if (sectionstr == "TABLE")
            {
                more = reader->readRec(&code);
                DRW_DBG(code);
                DRW_DBG("\n");
                if (!more)
                {
                    return setError(DRW::BAD_READ_TABLES); // wrong dxf file
                }
                if (code == 2)
                {
                    sectionstr = reader->getString();
                    DRW_DBG(sectionstr);
                    DRW_DBG(" processHeader\n\n");
                    // found section, process it
                    if (sectionstr == "LTYPE")
                    {
                        processLType();
                    }
                    else if (sectionstr == "LAYER")
                    {
                        processLayer();
                    }
                    else if (sectionstr == "STYLE")
                    {
                        processTextStyle();
                    }
                    else if (sectionstr == "VPORT")
                    {
                        processVports();
                    }
                    else if (sectionstr == "VIEW")
                    {
                        processView();
                    }
                    else if (sectionstr == "UCS")
                    {
                        //                        processUCS();
                    }
                    else if (sectionstr == "APPID")
                    {
                        processAppId();
                    }
                    else if (sectionstr == "DIMSTYLE")
                    {
                        processDimStyle();
                    }
                    else if (sectionstr == "BLOCK_RECORD")
                    {
                        //                        processBlockRecord();
                    }
                }
            }
            else if (sectionstr == "ENDSEC")
            {
                return true; // found ENDSEC terminate
            }
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

bool dxfRW::processView()
{
    DRW_DBG("dxfRW::processView\n");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_View view;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
            {
                iface->addView(view);
            }
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "VIEW")
            {
                reading = true;
                view.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; //found ENDTAB terminate
            }
        }
        else if (reading && !view.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }
    return setError(DRW::BAD_READ_TABLES);
}

auto dxfRW::processLType() -> bool
{
    DRW_DBG("dxfRW::processLType\n");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_LType ltype;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
            {
                ltype.update();
                iface->addLType(ltype);
            }
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "LTYPE")
            {
                reading = true;
                ltype.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; // found ENDTAB terminate
            }
        }
        else if (reading && !ltype.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

auto dxfRW::processLayer() -> bool
{
    DRW_DBG("dxfRW::processLayer\n");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_Layer layer;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
                iface->addLayer(layer);
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "LAYER")
            {
                reading = true;
                layer.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; // found ENDTAB terminate
            }
        }
        else if (reading && !layer.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

auto dxfRW::processDimStyle() -> bool
{
    DRW_DBG("dxfRW::processDimStyle");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_Dimstyle dimSty;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
                iface->addDimStyle(dimSty);
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "DIMSTYLE")
            {
                reading = true;
                dimSty.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; // found ENDTAB terminate
            }
        }
        else if (reading && !dimSty.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

auto dxfRW::processTextStyle() -> bool
{
    DRW_DBG("dxfRW::processTextStyle");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_Textstyle TxtSty;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
                iface->addTextStyle(TxtSty);
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "STYLE")
            {
                reading = true;
                TxtSty.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; // found ENDTAB terminate
            }
        }
        else if (reading && !TxtSty.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

auto dxfRW::processVports() -> bool
{
    DRW_DBG("dxfRW::processVports");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_Vport vp;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
                iface->addVport(vp);
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "VPORT")
            {
                reading = true;
                vp.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; // found ENDTAB terminate
            }
        }
        else if (reading && !vp.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

auto dxfRW::processAppId() -> bool
{
    DRW_DBG("dxfRW::processAppId");
    int code;
    std::string sectionstr;
    bool reading = false;
    DRW_AppId vp;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            if (reading)
                iface->addAppId(vp);
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "APPID")
            {
                reading = true;
                vp.reset();
            }
            else if (sectionstr == "ENDTAB")
            {
                return true; // found ENDTAB terminate
            }
        }
        else if (reading && !vp.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_TABLES);
}

/********* Block Section *********/

auto dxfRW::processBlocks() -> bool
{
    DRW_DBG("dxfRW::processBlocks\n");
    int code;
    std::string sectionstr;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (code == 0)
        {
            sectionstr = reader->getString();
            DRW_DBG(sectionstr);
            DRW_DBG("\n");
            if (sectionstr == "BLOCK")
            {
                processBlock();
            }
            else if (sectionstr == "ENDSEC")
            {
                return true; // found ENDSEC terminate
            }
        }
    }

    return setError(DRW::BAD_READ_BLOCKS);
}

auto dxfRW::processBlock() -> bool
{
    DRW_DBG("dxfRW::processBlock");
    int code;
    DRW_Block block;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addBlock(block);
            if (nextentity == "ENDBLK")
            {
                iface->endBlock();
                return true; // found ENDBLK, terminate
            }
            else
            {
                processEntities(true);
                iface->endBlock();
                return true; // found ENDBLK, terminate
            }
        }

        if (!block.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_BLOCKS);
}

/********* Entities Section *********/

auto dxfRW::processEntities(bool isblock) -> bool
{
    DRW_DBG("dxfRW::processEntities\n");
    int code;
    if (!reader->readRec(&code))
    {
        return setError(DRW::BAD_READ_ENTITIES);
    }

    if (code == 0)
    {
        nextentity = reader->getString();
    }
    else if (!isblock)
    {
        return setError(DRW::BAD_READ_ENTITIES); // first record in entities is 0
    }

    bool processed{false};
    do
    {
        if (nextentity == "ENDSEC" || nextentity == "ENDBLK")
        {
            return true; // found ENDSEC or ENDBLK terminate
        }
        else if (nextentity == "POINT")
        {
            processed = processPoint();
        }
        else if (nextentity == "LINE")
        {
            processed = processLine();
        }
        else if (nextentity == "CIRCLE")
        {
            processed = processCircle();
        }
        else if (nextentity == "ARC")
        {
            processed = processArc();
        }
        else if (nextentity == "ELLIPSE")
        {
            processed = processEllipse();
        }
        else if (nextentity == "TRACE")
        {
            processed = processTrace();
        }
        else if (nextentity == "SOLID")
        {
            processed = processSolid();
        }
        else if (nextentity == "INSERT")
        {
            processed = processInsert();
        }
        else if (nextentity == "LWPOLYLINE")
        {
            processed = processLWPolyline();
        }
        else if (nextentity == "POLYLINE")
        {
            processed = processPolyline();
        }
        else if (nextentity == "TEXT")
        {
            processed = processText();
        }
        else if (nextentity == "MTEXT")
        {
            processed = processMText();
        }
        else if (nextentity == "HATCH")
        {
            processed = processHatch();
        }
        else if (nextentity == "SPLINE")
        {
            processed = processSpline();
        }
        else if (nextentity == "3DFACE")
        {
            processed = process3dface();
        }
        else if (nextentity == "VIEWPORT")
        {
            processed = processViewport();
        }
        else if (nextentity == "IMAGE")
        {
            processed = processImage();
        }
        else if (nextentity == "DIMENSION")
        {
            processed = processDimension();
        }
        else if (nextentity == "LEADER")
        {
            processed = processLeader();
        }
        else if (nextentity == "RAY")
        {
            processed = processRay();
        }
        else if (nextentity == "XLINE")
        {
            processed = processXline();
        }
        else
        {
            if (!reader->readRec(&code))
            {
                return setError(DRW::BAD_READ_ENTITIES); // end of file without ENDSEC
            }

            if (code == 0)
            {
                nextentity = reader->getString();
            }
            processed = true;
        }

    } while (processed);

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processEllipse() -> bool
{
    DRW_DBG("dxfRW::processEllipse");
    int code;
    DRW_Ellipse ellipse;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (applyExt)
                ellipse.applyExtrusion();
            iface->addEllipse(ellipse);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!ellipse.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processTrace() -> bool
{
    DRW_DBG("dxfRW::processTrace");
    int code;
    DRW_Trace trace;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (applyExt)
                trace.applyExtrusion();
            iface->addTrace(trace);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!trace.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processSolid() -> bool
{
    DRW_DBG("dxfRW::processSolid");
    int code;
    DRW_Solid solid;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (applyExt)
                solid.applyExtrusion();
            iface->addSolid(solid);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!solid.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::process3dface() -> bool
{
    DRW_DBG("dxfRW::process3dface");
    int code;
    DRW_3Dface face;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->add3dFace(face);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!face.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processViewport() -> bool
{
    DRW_DBG("dxfRW::processViewport");
    int code;
    DRW_Viewport vp;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addViewport(vp);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!vp.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processPoint() -> bool
{
    DRW_DBG("dxfRW::processPoint\n");
    int code;
    DRW_Point point;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addPoint(point);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!point.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processLine() -> bool
{
    DRW_DBG("dxfRW::processLine\n");
    int code;
    DRW_Line line;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addLine(line);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!line.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processRay() -> bool
{
    DRW_DBG("dxfRW::processRay\n");
    int code;
    DRW_Ray line;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addRay(line);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!line.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processXline() -> bool
{
    DRW_DBG("dxfRW::processXline\n");
    int code;
    DRW_Xline line;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addXline(line);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!line.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processCircle() -> bool
{
    DRW_DBG("dxfRW::processPoint\n");
    int code;
    DRW_Circle circle;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (applyExt)
                circle.applyExtrusion();
            iface->addCircle(circle);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!circle.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processArc() -> bool
{
    DRW_DBG("dxfRW::processPoint\n");
    int code;
    DRW_Arc arc;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (applyExt)
                arc.applyExtrusion();
            iface->addArc(arc);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!arc.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processInsert() -> bool
{
    DRW_DBG("dxfRW::processInsert");
    int code;
    DRW_Insert insert;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addInsert(insert);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!insert.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processLWPolyline() -> bool
{
    DRW_DBG("dxfRW::processLWPolyline");
    int code = 0;
    DRW_LWPolyline pl;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (applyExt)
                pl.applyExtrusion();
            iface->addLWPolyline(pl);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!pl.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processPolyline() -> bool
{
    DRW_DBG("dxfRW::processPolyline");
    int code;
    DRW_Polyline pl;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (nextentity != "VERTEX")
            {
                iface->addPolyline(pl);
                return true; // found new entity or ENDSEC, terminate
            }
            processVertex(&pl);
        }

        if (!pl.parseCode(code, reader))
        { // parseCode just initialize the members of pl
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processVertex(DRW_Polyline *pl) -> bool
{
    DRW_DBG("dxfRW::processVertex");
    int code;
    auto v = std::make_unique<DRW_Vertex>();
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            pl->appendVertex(v.release());
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            if (nextentity == "SEQEND")
            {
                return true; // found SEQEND no more vertex, terminate
            }
            if (nextentity == "VERTEX")
            {
                v.reset(new DRW_Vertex()); // another vertex
            }
        }

        if (!v->parseCode(code, reader))
        { // the members of v are reinitialized here
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processText() -> bool
{
    DRW_DBG("dxfRW::processText");
    int code;
    DRW_Text txt;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addText(txt);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!txt.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processMText() -> bool
{
    DRW_DBG("dxfRW::processMText");
    int code;
    DRW_MText txt;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            txt.updateAngle();
            iface->addMText(txt);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!txt.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processHatch() -> bool
{
    DRW_DBG("dxfRW::processHatch");
    int code;
    DRW_Hatch hatch;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addHatch(&hatch);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!hatch.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processSpline() -> bool
{
    DRW_DBG("dxfRW::processSpline");
    int code;
    DRW_Spline sp;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addSpline(&sp);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!sp.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processImage() -> bool
{
    DRW_DBG("dxfRW::processImage");
    int code;
    DRW_Image img;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addImage(&img);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!img.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processDimension() -> bool
{
    DRW_DBG("dxfRW::processDimension");
    int code;
    DRW_Dimension dim;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");

            QT_WARNING_PUSH
            QT_WARNING_DISABLE_GCC("-Wswitch-default")
            QT_WARNING_DISABLE_CLANG("-Wswitch-default")

            switch (int const type = dim.type & 0x0F; type)
            {
                case 0:
                {
                    DRW_DimLinear const d(dim);
                    iface->addDimLinear(&d);
                    break;
                }
                case 1:
                {
                    DRW_DimAligned const d(dim);
                    iface->addDimAlign(&d);
                    break;
                }
                case 2:
                {
                    DRW_DimAngular const d(dim);
                    iface->addDimAngular(&d);
                    break;
                }
                case 3:
                {
                    DRW_DimDiametric const d(dim);
                    iface->addDimDiametric(&d);
                    break;
                }
                case 4:
                {
                    DRW_DimRadial const d(dim);
                    iface->addDimRadial(&d);
                    break;
                }
                case 5:
                {
                    DRW_DimAngular3p const d(dim);
                    iface->addDimAngular3P(&d);
                    break;
                }
                case 6:
                {
                    DRW_DimOrdinate const d(dim);
                    iface->addDimOrdinate(&d);
                    break;
                }
            }

            // cppcheck-suppress unknownMacro
            QT_WARNING_POP

            return true; // found new entity or ENDSEC, terminate
        }

        if (!dim.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

auto dxfRW::processLeader() -> bool
{
    DRW_DBG("dxfRW::processLeader");
    int code;
    DRW_Leader leader;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addLeader(&leader);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!leader.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_ENTITIES);
}

/********* Objects Section *********/

auto dxfRW::processObjects() -> bool
{
    DRW_DBG("dxfRW::processObjects\n");
    int code;
    if (!reader->readRec(&code) || 0 != code)
    {
        return setError(DRW::BAD_READ_OBJECTS); // first record in objects must be 0
    }

    bool processed = false;
    nextentity = reader->getString();
    do
    {
        if ("ENDSEC" == nextentity)
        {
            return true; // found ENDSEC terminate
        }

        if ("IMAGEDEF" == nextentity)
        {
            processed = processImageDef();
        }
        else if ("PLOTSETTINGS" == nextentity)
        {
            processed = processPlotSettings();
        }
        else
        {
            if (!reader->readRec(&code))
            {
                return setError(DRW::BAD_READ_OBJECTS); // end of file without ENDSEC
            }

            if (code == 0)
            {
                nextentity = reader->getString();
            }
            processed = true;
        }
    } while (processed);

    return setError(DRW::BAD_READ_OBJECTS);
}

auto dxfRW::processImageDef() -> bool
{
    DRW_DBG("dxfRW::processImageDef");
    int code;
    DRW_ImageDef img;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->linkImage(&img);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!img.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_OBJECTS);
}

auto dxfRW::processPlotSettings() -> bool
{
    DRW_DBG("dxfRW::processPlotSettings");
    int code;
    DRW_PlotSettings ps;
    while (reader->readRec(&code))
    {
        DRW_DBG(code);
        DRW_DBG("\n");
        if (0 == code)
        {
            nextentity = reader->getString();
            DRW_DBG(nextentity);
            DRW_DBG("\n");
            iface->addPlotSettings(&ps);
            return true; // found new entity or ENDSEC, terminate
        }

        if (!ps.parseCode(code, reader))
        {
            return setError(DRW::BAD_CODE_PARSED);
        }
    }

    return setError(DRW::BAD_READ_OBJECTS);
}

auto dxfRW::writePlotSettings(DRW_PlotSettings *ent) -> bool
{
    writer->writeString(0, "PLOTSETTINGS");
    writer->writeString(5, toHexStr(++entCount));
    writer->writeString(100, "AcDbPlotSettings");
    writer->writeUtf8String(6, ent->plotViewName);
    writer->writeDouble(40, ent->marginLeft);
    writer->writeDouble(41, ent->marginBottom);
    writer->writeDouble(42, ent->marginRight);
    writer->writeDouble(43, ent->marginTop);
    return true;
}

/** utility function
 * convert a int to string in hex
 **/
auto dxfRW::toHexStr(int n) -> std::string
{
#if defined(__APPLE__)
    char buffer[9] = {'\0'};
    snprintf(buffer, 9, "%X", n);
    return std::string(buffer);
#else
    std::ostringstream Convert;
    Convert << std::uppercase << std::hex << n;
    return Convert.str();
#endif
}

auto dxfRW::getVersion() const -> DRW::Version
{
    return version;
}

auto dxfRW::getError() const -> DRW::error
{
    return error;
}

auto dxfRW::setError(DRW::error lastError) -> bool
{
    error = lastError;
    return (DRW::BAD_NONE == error);
}
