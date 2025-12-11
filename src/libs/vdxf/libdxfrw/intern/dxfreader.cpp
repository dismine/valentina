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

#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include "dxfreader.h"
#include "drw_textcodec.h"
#include "drw_dbg.h"

auto dxfReader::readRec(int *codeData) -> bool
{
    //    std::string text;
    int code;

    if (!readCode(&code))
        return false;
    *codeData = code;

    if (code < 10)
        readString();
    else if (code < 60)
        readDouble();
    else if (code < 80)
        readInt16();
    else if (code > 89 && code < 100) //TODO this is an int 32b
        readInt32();
    else if (code == 100 || code == 102 || code == 105)
        readString();
    else if (code > 109 && code < 150) //skip not used at the v2012
        readDouble();
    else if (code > 159 && code < 170) //skip not used at the v2012
        readInt64();
    else if (code < 180)
        readInt16();
    else if (code > 209 && code < 240) //skip not used at the v2012
        readDouble();
    else if (code > 269 && code < 290) //skip not used at the v2012
        readInt16();
    else if (code < 300) //TODO this is a boolean indicator, int in Binary?
        readBool();
    else if (code < 310)
        readString();
    else if (code < 320)
        readBinary();
    else if (code < 370)
        readString();
    else if (code < 390)
        readInt16();
    else if (code < 400)
        readString();
    else if (code < 410)
        readInt16();
    else if (code < 420)
        readString();
    else if (code < 430) //TODO this is an int 32b
        readInt32();
    else if (code < 440)
        readString();
    else if (code < 450) //TODO this is an int 32b
        readInt32();
    else if (code < 460) //TODO this is long??
        readInt32();
    else if (code < 470) //TODO this is a floating point double precision??
        readDouble();
    else if (code < 481)
        readString();
    else if( 999 == code && m_bIgnoreComments) {
        readString();
        return readRec( codeData);
    }
    else if (code == 1004)
        readBinary();
    else if (code > 998 && code < 1009) //skip not used at the v2012
        readString();
    else if (code < 1060) //TODO this is a floating point double precision??
        readDouble();
    else if (code < 1071)
        readInt16();
    else if (code == 1071) //TODO this is an int 32b
        readInt32();
    else if (skip)
        //skip safely this dxf entry ( ok for ascii dxf)
        readString();
    else
        //break in binary files because the conduct is unpredictable
        return false;

    return (filestr->good());
}
auto dxfReader::getHandleString() -> int
{
    int res;
#if defined(__APPLE__)
    // cppcheck-suppress invalidScanfArgType_int
    int Succeeded = sscanf ( strData.c_str(), "%x", &res );
    if ( !Succeeded || Succeeded == EOF )
        res = 0;
#else
    if (std::istringstream Convert(strData); !(Convert >> std::hex >> res))
        res = 0;
#endif
    return res;
}

auto dxfReaderBinary::readCode(int *code) -> bool
{
    union {
        unsigned short val;
        char buf[2];
    } buffer;
    filestr->read(buffer.buf,2);
//exist a 32bits int (code 90) with 2 bytes???
    if ((*code == 90) && (buffer.val>2000)){
        DRW_DBG(*code); DRW_DBG(" de 16bits\n");
        filestr->seekg(-4, std::ios_base::cur);
        filestr->read(buffer.buf,2);
    }
    *code = buffer.val;
    DRW_DBG(*code); DRW_DBG("\n");

    return (filestr->good());
}

auto dxfReaderBinary::readString() -> bool
{
    type = STRING;
    std::getline(*filestr, strData, '\0');
    DRW_DBG(strData); DRW_DBG("\n");
    return (filestr->good());
}

auto dxfReaderBinary::readString(std::string *text) -> bool
{
    type = STRING;
    std::getline(*filestr, *text, '\0');
    DRW_DBG(*text); DRW_DBG("\n");
    return (filestr->good());
}

auto dxfReaderBinary::readBinary() -> bool
{
    unsigned char chunklen {0};

    filestr->read( reinterpret_cast<char *>(&chunklen), 1);
    filestr->seekg( chunklen, std::ios_base::cur);
    DRW_DBG( chunklen); DRW_DBG( " byte(s) binary data bypassed\n");

    return (filestr->good());
}

auto dxfReaderBinary::readInt16() -> bool
{
    type = INT32;
    char buffer[2];
    filestr->read(buffer,2);
    intData = static_cast<int>((buffer[1] << 8) | buffer[0]);
    DRW_DBG(intData); DRW_DBG("\n");
    return (filestr->good());
}

auto dxfReaderBinary::readInt32() -> bool
{
    type = INT32;
    union {
        signed int val;
        char buf[4];
    } buffer;
    filestr->read(buffer.buf,4);
    intData = buffer.val;
    // cppcheck-suppress danglingLifetime
    DRW_DBG(intData); DRW_DBG("\n");
    return (filestr->good());
}

auto dxfReaderBinary::readInt64() -> bool
{
    type = INT64;
    union {
        unsigned long long int val;
        char buf[8];
    } buffer;
    filestr->read(buffer.buf,8);
    int64 = buffer.val;
    // cppcheck-suppress danglingLifetime
    DRW_DBG(int64); DRW_DBG(" int64\n");
    return (filestr->good());
}

auto dxfReaderBinary::readDouble() -> bool
{
    type = DOUBLE;
    union {
        double val;
        char buf[8];
    } buffer;
    filestr->read(buffer.buf,8);
    doubleData = buffer.val;
    // cppcheck-suppress danglingLifetime
    DRW_DBG(doubleData); DRW_DBG("\n");
    return (filestr->good());
}

//saved as int or add a bool member??
auto dxfReaderBinary::readBool() -> bool
{
    char buffer[1];
    filestr->read(buffer,1);
    intData = static_cast<int>(buffer[0]);
    DRW_DBG(intData); DRW_DBG("\n");
    return (filestr->good());
}

auto dxfReaderAscii::readCode(int *code) -> bool
{
    std::string text;
    std::getline(*filestr, text);
    *code = atoi(text.c_str());
    DRW_DBG(*code); DRW_DBG("\n");
    return (filestr->good());
}
auto dxfReaderAscii::readString(std::string *text) -> bool
{
    type = STRING;
    std::getline(*filestr, *text);
    if (!text->empty() && text->at(text->size()-1) == '\r')
        text->erase(text->size()-1);
    return (filestr->good());
}

auto dxfReaderAscii::readString() -> bool
{
    type = STRING;
    std::getline(*filestr, strData);
    if (!strData.empty() && strData.at(strData.size()-1) == '\r')
        strData.erase(strData.size()-1);
    DRW_DBG(strData); DRW_DBG("\n");
    return (filestr->good());
}

auto dxfReaderAscii::readBinary() -> bool
{
    return readString();
}

auto dxfReaderAscii::readInt16() -> bool
{
    type = INT32;
    std::string text;
    if (readString(&text)){
        intData = atoi(text.c_str());
        DRW_DBG(intData); DRW_DBG("\n");
        return true;
    } else
        return false;
}

auto dxfReaderAscii::readInt32() -> bool
{
    type = INT32;
    return readInt16();
}

auto dxfReaderAscii::readInt64() -> bool
{
    type = INT64;
    return readInt16();
}

auto dxfReaderAscii::readDouble() -> bool
{
    type = DOUBLE;
    std::string text;
    if (readString(&text)){
#if defined(__APPLE__)
        int succeeded=sscanf( & (text[0]), "%lg", &doubleData);
        if(succeeded != 1) {
            DRW_DBG("dxfReaderAscii::readDouble(): reading double error: ");
            DRW_DBG(text);
            DRW_DBG('\n');
        }
#else
        std::istringstream sd(text);
        sd >> doubleData;
        DRW_DBG(doubleData); DRW_DBG('\n');
#endif
        return true;
    } else
        return false;
}

//saved as int or add a bool member??
auto dxfReaderAscii::readBool() -> bool
{
    type = BOOL;
    std::string text;
    if (readString(&text)){
        intData = atoi(text.c_str());
        DRW_DBG(intData); DRW_DBG("\n");
        return true;
    } else
        return false;
}
