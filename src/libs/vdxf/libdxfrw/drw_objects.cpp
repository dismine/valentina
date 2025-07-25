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

#include "drw_objects.h"
#include "drw_reserve.h"
#include "intern/drw_dbg.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"
#include <cmath>
#include <iostream>

//! Base class for tables entries
/*!
 *  Base class for tables entries
 *  @author Rallaz
 */
auto DRW_TableEntry::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 5:
            handle = static_cast<duint32>(reader->getHandleString());
            break;
        case 330:
            parentHandle = reader->getHandleString();
            break;
        case 2:
            name = reader->getUtf8String();
            break;
        case 70:
            flags = reader->getInt32();
            break;
        case 1000:
        case 1001:
        case 1002:
        case 1003:
        case 1004:
        case 1005:
            extData.push_back(new DRW_Variant(code, reader->getString()));
            break;
        case 1010:
        case 1011:
        case 1012:
        case 1013:
            // don't trust in X, Y, Z order!
            if (curr != nullptr)
            {
                curr->setCoordX(reader->getDouble());
            }
            else
            {
                curr = new DRW_Variant(code, DRW_Coord(reader->getDouble(), 0.0, 0.0));
                extData.push_back(curr);
            }
            break;
        case 1020:
        case 1021:
        case 1022:
        case 1023:
            // don't trust in X, Y, Z order!
            if (curr != nullptr)
            {
                curr->setCoordY(reader->getDouble());
            }
            else
            {
                curr = new DRW_Variant(code, DRW_Coord(0.0, reader->getDouble(), 0.0));
                extData.push_back(curr);
            }
            break;
        case 1030:
        case 1031:
        case 1032:
        case 1033:
            // don't trust in X, Y, Z order!
            if (curr != nullptr)
            {
                curr->setCoordZ(reader->getDouble());
            }
            else
            {
                curr = new DRW_Variant(code, DRW_Coord(0.0, 0.0, reader->getDouble()));
                extData.push_back(curr);
            }
            break;
        case 1040:
        case 1041:
        case 1042:
            extData.push_back(new DRW_Variant(code, reader->getDouble()));
            break;
        case 1070:
        case 1071:
            extData.push_back(new DRW_Variant(code, reader->getInt32()));
            break;
        default:
            break;
    }

    return true;
}

//! Class to handle dimstyle entries
/*!
 *  Class to handle ldim style symbol table entries
 *  @author Rallaz
 */
auto DRW_Dimstyle::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 105:
            handle = static_cast<duint32>(reader->getHandleString());
            break;
        case 3:
            dimpost = reader->getUtf8String();
            break;
        case 4:
            dimapost = reader->getUtf8String();
            break;
        case 5:
            dimblk = reader->getUtf8String();
            break;
        case 6:
            dimblk1 = reader->getUtf8String();
            break;
        case 7:
            dimblk2 = reader->getUtf8String();
            break;
        case 40:
            dimscale = reader->getDouble();
            break;
        case 41:
            dimasz = reader->getDouble();
            break;
        case 42:
            dimexo = reader->getDouble();
            break;
        case 43:
            dimdli = reader->getDouble();
            break;
        case 44:
            dimexe = reader->getDouble();
            break;
        case 45:
            dimrnd = reader->getDouble();
            break;
        case 46:
            dimdle = reader->getDouble();
            break;
        case 47:
            dimtp = reader->getDouble();
            break;
        case 48:
            dimtm = reader->getDouble();
            break;
        case 49:
            dimfxl = reader->getDouble();
            break;
        case 140:
            dimtxt = reader->getDouble();
            break;
        case 141:
            dimcen = reader->getDouble();
            break;
        case 142:
            dimtsz = reader->getDouble();
            break;
        case 143:
            dimaltf = reader->getDouble();
            break;
        case 144:
            dimlfac = reader->getDouble();
            break;
        case 145:
            dimtvp = reader->getDouble();
            break;
        case 146:
            dimtfac = reader->getDouble();
            break;
        case 147:
            dimgap = reader->getDouble();
            break;
        case 148:
            dimaltrnd = reader->getDouble();
            break;
        case 71:
            dimtol = reader->getInt32();
            break;
        case 72:
            dimlim = reader->getInt32();
            break;
        case 73:
            dimtih = reader->getInt32();
            break;
        case 74:
            dimtoh = reader->getInt32();
            break;
        case 75:
            dimse1 = reader->getInt32();
            break;
        case 76:
            dimse2 = reader->getInt32();
            break;
        case 77:
            dimtad = reader->getInt32();
            break;
        case 78:
            dimzin = reader->getInt32();
            break;
        case 79:
            dimazin = reader->getInt32();
            break;
        case 170:
            dimalt = reader->getInt32();
            break;
        case 171:
            dimaltd = reader->getInt32();
            break;
        case 172:
            dimtofl = reader->getInt32();
            break;
        case 173:
            dimsah = reader->getInt32();
            break;
        case 174:
            dimtix = reader->getInt32();
            break;
        case 175:
            dimsoxd = reader->getInt32();
            break;
        case 176:
            dimclrd = reader->getInt32();
            break;
        case 177:
            dimclre = reader->getInt32();
            break;
        case 178:
            dimclrt = reader->getInt32();
            break;
        case 179:
            dimadec = reader->getInt32();
            break;
        case 270:
            dimunit = reader->getInt32();
            break;
        case 271:
            dimdec = reader->getInt32();
            break;
        case 272:
            dimtdec = reader->getInt32();
            break;
        case 273:
            dimaltu = reader->getInt32();
            break;
        case 274:
            dimalttd = reader->getInt32();
            break;
        case 275:
            dimaunit = reader->getInt32();
            break;
        case 276:
            dimfrac = reader->getInt32();
            break;
        case 277:
            dimlunit = reader->getInt32();
            break;
        case 278:
            dimdsep = reader->getInt32();
            break;
        case 279:
            dimtmove = reader->getInt32();
            break;
        case 280:
            dimjust = reader->getInt32();
            break;
        case 281:
            dimsd1 = reader->getInt32();
            break;
        case 282:
            dimsd2 = reader->getInt32();
            break;
        case 283:
            dimtolj = reader->getInt32();
            break;
        case 284:
            dimtzin = reader->getInt32();
            break;
        case 285:
            dimaltz = reader->getInt32();
            break;
        case 286:
            dimaltttz = reader->getInt32();
            break;
        case 287:
            dimfit = reader->getInt32();
            break;
        case 288:
            dimupt = reader->getInt32();
            break;
        case 289:
            dimatfit = reader->getInt32();
            break;
        case 290:
            dimfxlon = reader->getInt32();
            break;
        case 340:
            dimtxsty = reader->getUtf8String();
            break;
        case 341:
            dimldrblk = reader->getUtf8String();
            break;
        case 342:
            dimblk = reader->getUtf8String();
            break;
        case 343:
            dimblk1 = reader->getUtf8String();
            break;
        case 344:
            dimblk2 = reader->getUtf8String();
            break;
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

//! Class to handle line type entries
/*!
 *  Class to handle line type symbol table entries
 *  @author Rallaz
 */
auto DRW_LType::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 3:
            desc = reader->getUtf8String();
            break;
        case 73:
            size = reader->getInt32();
            path.clear();
            if (!DRW::reserve(path, size))
            {
                return false;
            }
            break;
        case 40:
            length = reader->getDouble();
            break;
        case 49:
            path.push_back(reader->getDouble());
            pathIdx++;
            break;
            /*    case 74:
                    haveShape = reader->getInt32();
                    break;*/
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

//! Update line type
/*!
 *  Update the size and length of line type according to the path
 *  @author Rallaz
 */
/*TODO: control max length permited */
void DRW_LType::update()
{
    double d = 0;
    size = static_cast<int>(path.size());
    for (int i = 0; i < size; i++)
    {
        d += fabs(path.at(static_cast<size_t>(i)));
    }
    length = d;
}

//! Class to handle layer entries
/*!
 *  Class to handle layer symbol table entries
 *  @author Rallaz
 */
auto DRW_Layer::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 6:
            lineType = reader->getUtf8String();
            break;
        case 62:
            color = reader->getInt32();
            break;
        case 290:
            plotF = reader->getBool();
            break;
        case 370:
            lWeight = DRW_LW_Conv::dxfInt2lineWidth(reader->getInt32());
            break;
        case 390:
            handlePlotS = reader->getString();
            break;
        case 347:
            handleMaterialS = reader->getString();
            break;
        case 420:
            color24 = reader->getInt32();
            break;
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

//! Class to handle text style entries
/*!
 *  Class to handle text style symbol table entries
 *  @author Rallaz
 */
auto DRW_Textstyle::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 3:
            font = reader->getUtf8String();
            break;
        case 4:
            bigFont = reader->getUtf8String();
            break;
        case 40:
            height = reader->getDouble();
            break;
        case 41:
            width = reader->getDouble();
            break;
        case 50:
            oblique = reader->getDouble();
            break;
        case 42:
            lastHeight = reader->getDouble();
            break;
        case 71:
            genFlag = reader->getInt32();
            break;
        case 1071:
            fontFamily = reader->getInt32();
            break;
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

//! Class to handle vport entries
/*!
 *  Class to handle vport symbol table entries
 *  @author Rallaz
 */
auto DRW_Vport::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 10:
            lowerLeft.x = reader->getDouble();
            break;
        case 20:
            lowerLeft.y = reader->getDouble();
            break;
        case 11:
            UpperRight.x = reader->getDouble();
            break;
        case 21:
            UpperRight.y = reader->getDouble();
            break;
        case 12:
            center.x = reader->getDouble();
            break;
        case 22:
            center.y = reader->getDouble();
            break;
        case 13:
            snapBase.x = reader->getDouble();
            break;
        case 23:
            snapBase.y = reader->getDouble();
            break;
        case 14:
            snapSpacing.x = reader->getDouble();
            break;
        case 24:
            snapSpacing.y = reader->getDouble();
            break;
        case 15:
            gridSpacing.x = reader->getDouble();
            break;
        case 25:
            gridSpacing.y = reader->getDouble();
            break;
        case 16:
            viewDir.x = reader->getDouble();
            break;
        case 26:
            viewDir.y = reader->getDouble();
            break;
        case 36:
            viewDir.z = reader->getDouble();
            break;
        case 17:
            viewTarget.x = reader->getDouble();
            break;
        case 27:
            viewTarget.y = reader->getDouble();
            break;
        case 37:
            viewTarget.z = reader->getDouble();
            break;
        case 40:
            height = reader->getDouble();
            break;
        case 41:
            width = reader->getDouble();
            break;
        case 42:
            lensHeight = reader->getDouble();
            break;
        case 43:
            frontClip = reader->getDouble();
            break;
        case 44:
            backClip = reader->getDouble();
            break;
        case 50:
            snapAngle = reader->getDouble();
            break;
        case 51:
            twistAngle = reader->getDouble();
            break;
        case 71:
            viewMode = reader->getInt32();
            break;
        case 72:
            circleZoom = reader->getInt32();
            break;
        case 73:
            fastZoom = reader->getInt32();
            break;
        case 74:
            ucsIcon = reader->getInt32();
            break;
        case 75:
            snap = reader->getInt32();
            break;
        case 76:
            grid = reader->getInt32();
            break;
        case 77:
            snapStyle = reader->getInt32();
            break;
        case 78:
            snapIsopair = reader->getInt32();
            break;
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

auto DRW_ImageDef::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 1:
            fileName = reader->getUtf8String();
            break;
        case 5:
            handle = static_cast<duint32>(reader->getHandleString());
            break;
        case 10:
            u = reader->getDouble();
            break;
        case 20:
            v = reader->getDouble();
            break;
        case 11:
            up = reader->getDouble();
            break;
        case 12:
            vp = reader->getDouble();
            break;
        case 21:
            vp = reader->getDouble();
            break;
        case 280:
            loaded = reader->getInt32();
            break;
        case 281:
            resolution = reader->getInt32();
            break;
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

auto DRW_PlotSettings::parseCode(int code, const std::unique_ptr<dxfReader> &reader) -> bool
{
    switch (code)
    {
        case 5:
            handle = static_cast<duint32>(reader->getHandleString());
            break;
        case 6:
            plotViewName = reader->getUtf8String();
            break;
        case 40:
            marginLeft = reader->getDouble();
            break;
        case 41:
            marginBottom = reader->getDouble();
            break;
        case 42:
            marginRight = reader->getDouble();
            break;
        case 43:
            marginTop = reader->getDouble();
            break;
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }

    return true;
}

bool DRW_View::parseCode(int code, const std::unique_ptr<dxfReader> &reader)
{
    switch (code)
    {
        case 40:
        {
            size.y = reader->getDouble();
            break;
        }
        case 41:
        {
            size.x = reader->getDouble();
            break;
        }
        case 10:
        {
            center.x = reader->getDouble();
            break;
        }
        case 20:
        {
            center.y = reader->getDouble();
            break;
        }
        case 11:
        {
            viewDirectionFromTarget.x = reader->getDouble();
            break;
        }
        case 21:
        {
            viewDirectionFromTarget.y = reader->getDouble();
            break;
        }
        case 31:
        {
            viewDirectionFromTarget.z = reader->getDouble();
            break;
        }
        case 12:
        {
            targetPoint.x = reader->getDouble();
            break;
        }
        case 22:
        {
            targetPoint.y = reader->getDouble();
            break;
        }
        case 32:
        {
            targetPoint.z = reader->getDouble();
            break;
        }
        case 42:
        {
            lensLen = reader->getDouble();
            break;
        }
        case 43:
        {
            frontClippingPlaneOffset = reader->getDouble();
            break;
        }
        case 44:
        {
            backClippingPlaneOffset = reader->getDouble();
            break;
        }
        case 50:
        {
            twistAngle = reader->getDouble();
            break;
        }
        case 71:
        {
            viewMode = reader->getInt32(); // tmp - check size to read
            break;
        }
        case 281:
        {
            renderMode = static_cast<unsigned>(reader->getInt32()); // tmp - check size to read
            break;
        }
        case 72:
        {
            hasUCS = reader->getBool();
            break;
        }
        case 73:
        {
            cameraPlottable = reader->getBool();
            break;
        }
        case 110:
        {
            ucsOrigin.x = reader->getDouble();
            break;
        }
        case 120:
        {
            ucsOrigin.y = reader->getDouble();
            break;
        }
        case 130:
        {
            ucsOrigin.z = reader->getDouble();
            break;
        }
        case 111:
        {
            ucsXAxis.x = reader->getDouble();
            break;
        }
        case 121:
        {
            ucsXAxis.y = reader->getDouble();
            break;
        }
        case 131:
        {
            ucsXAxis.z = reader->getDouble();
            break;
        }
        case 112:
        {
            ucsYAxis.x = reader->getDouble();
            break;
        }
        case 122:
        {
            ucsYAxis.y = reader->getDouble();
            break;
        }
        case 132:
        {
            ucsYAxis.z = reader->getDouble();
            break;
        }
        case 79:
        {
            ucsOrthoType = reader->getInt32();
            break;
        }
        case 146:
        {
            ucsElevation = reader->getDouble();
            break;
        }
        case 345:
        {
            // ID/handle of AcDbUCSTableRecord if UCS is a named UCS. If not present, then UCS is unnamed, code 345
            namedUCS_ID = static_cast<duint32>(reader->getHandleString());
            break;
        }
        case 346:
        {
            // ID/handle of AcDbUCSTableRecord of base UCS if UCS is orthographic, If not present and 79 code is
            // non-zero, then base UCS is taken to be WORLD, code 346
            baseUCS_ID = static_cast<duint32>(reader->getHandleString());
            break;
        }
        default:
            return DRW_TableEntry::parseCode(code, reader);
    }
    return true;
}
