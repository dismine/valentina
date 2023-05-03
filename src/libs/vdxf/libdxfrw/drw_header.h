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

#ifndef DRW_HEADER_H
#define DRW_HEADER_H


#include <map>
#include "drw_base.h"

class dxfReader;
class dxfWriter;

#define SETHDRFRIENDS  friend class dxfRW;

//! Class to handle header entries
/*!
*  Class to handle header vars, to read iterate over "std::map vars"
*  to write add a DRW_Variant* into "std::map vars" (do not delete it, are cleared in dtor)
*  or use add* helper functions.
*  @author Rallaz
*/
class DRW_Header {
    SETHDRFRIENDS
public:
    DRW_Header();

    ~DRW_Header() {
        clearVars();
    }

    enum Units {
        /** $ISUNITS header variable, since ACAD2000/AC1015 */
        None = 0,               ///< No unit (unit from parent)
        Inch = 1,               ///< 25.4 mm
        Foot = 2,               ///< 12 Inches = 0.3048 m
        Mile = 3,               ///< 1760 Yards = 1609 m
        Millimeter = 4,         ///< 0.001 m
        Centimeter = 5,         ///< 0.01 m
        Meter = 6,
        Kilometer = 7,          ///< 1000 m
        Microinch = 8,          ///< 0.000001 Inch = 0.0000254 mm = 25.4 Nanometer
        Mil = 9,                ///< 0.001 Inch = 0.0254 mm = 25.4 Micron
        Yard = 10,              ///< 3 Feet = 0.9144 m
        Angstrom = 11,          ///< 10^-10 m
        Nanometer = 12,         ///< 10^-9 m
        Micron = 13,            ///< 10^-6 m
        Decimeter = 14,         ///< 0.1 m
        Decameter = 15,         ///< 10 m
        Hectometer = 16,        ///< 100 m
        Gigameter = 17,         ///< 10^9 m
        Astro = 18,             ///< ~149.6 x 10^9 m
        Lightyear = 19,         ///< ~9.46 x 10^15 m
        Parsec = 20,            ///< ~3.0857 x 10^16 m
        UnitCount = 21,         ///< Used to iterate through units

        /** $MEASUREMENT header variable, since R14/AC1014 */
        English = 0,            ///< English/Imperial drawing */
        Metric = 1,             ///< Metric drawing */
    };

    DRW_Header(const DRW_Header& h)
        : vars(),
          comments(h.comments),
          name(),
          curr(nullptr),
          version(h.version),
          linetypeCtrl(),
          layerCtrl(),
          styleCtrl(),
          dimstyleCtrl(),
          appidCtrl(),
          blockCtrl(),
          viewCtrl(),
          ucsCtrl(),
          vportCtrl(),
          vpEntHeaderCtrl()
    {
        for (auto it=h.vars.begin(); it!=h.vars.end(); ++it)
        {
            this->vars[it->first] = new DRW_Variant( *(it->second) );
        }
    }
    auto operator=(const DRW_Header &h) -> DRW_Header &
    {
       if(this != &h)
       {
           clearVars();
           this->version = h.version;
           this->comments = h.comments;
           for (auto it=h.vars.begin(); it!=h.vars.end(); ++it)
           {
               this->vars[it->first] = new DRW_Variant( *(it->second) );
           }
       }
       return *this;
    }

    void addDouble(std::string key, double value, int code);
    void addInt(std::string key, int value, int code);
    void addStr(std::string key, const std::string &value, int code);
    void addCoord(std::string key, const DRW_Coord &value, int code);
    auto getComments() const -> std::string { return comments; }
    void write(dxfWriter *writer, DRW::Version ver);
    void addComment(const std::string &c);

protected:
    auto parseCode(int code, dxfReader *reader) -> bool;

private:
    auto getDouble(const std::string &key, double *varDouble) const -> bool;
    auto getInt(const std::string &key, int *varInt) const -> bool;
    auto getStr(const std::string &key, std::string *varStr) const -> bool;
    auto getCoord(const std::string &key, DRW_Coord *varCoord) const -> bool;
    void clearVars()
    {
        for (auto it=vars.begin(); it!=vars.end(); ++it)
        {
            delete it->second;
        }

        vars.clear();
    }

public:
    std::unordered_map<std::string,DRW_Variant*> vars;
private:
    std::string comments;
    std::string name;
    DRW_Variant* curr {nullptr};
    int version; //to use on read

    duint32 linetypeCtrl;
    duint32 layerCtrl;
    duint32 styleCtrl;
    duint32 dimstyleCtrl;
    duint32 appidCtrl;
    duint32 blockCtrl;
    duint32 viewCtrl;
    duint32 ucsCtrl;
    duint32 vportCtrl;
    duint32 vpEntHeaderCtrl;

    auto measurement(const int unit) -> int;
};

#endif

// EOF
