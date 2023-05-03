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

#ifndef DXFREADER_H
#define DXFREADER_H

#include "drw_textcodec.h"
#include "../vmisc/defglobal.h"

class dxfReader {
public:
    enum TYPE {
        STRING,
        INT32,
        INT64,
        DOUBLE,
        BOOL,
        INVALID
    };
    TYPE type;
public:
    explicit dxfReader(std::istream *stream)
        : type(INVALID),
          filestr(stream),
          strData(),
          doubleData(),
          intData(),
          int64(),
          skip(),
          decoder()
    {}

    virtual ~dxfReader() = default;
    auto readRec(int *codeData) -> bool;

    auto getString() const -> std::string { return strData; }
    auto getHandleString() -> int; // Convert hex string to int
    auto toUtf8String(const std::string &t) -> std::string { return decoder.toUtf8(t); }
    auto getUtf8String() -> std::string { return decoder.toUtf8(strData); }
    auto getDouble() const -> double { return doubleData; }
    auto getInt32() const -> int { return intData; }
    auto getInt64() const -> unsigned long long int { return int64; }
    auto getBool() const -> bool { return (intData == 0) ? false : true; }
    auto getVersion() const -> int { return decoder.getVersion(); }
    void setVersion(const std::string &v, bool dxfFormat){decoder.setVersion(v, dxfFormat);}
    void setCodePage(const std::string &c){decoder.setCodePage(c, true);}
    auto getCodePage() const -> std::string { return decoder.getCodePage(); }
    void setIgnoreComments(const bool bValue) {m_bIgnoreComments = bValue;}

protected:
    virtual auto readCode(int *code) -> bool = 0; // return true if successful (not EOF)
    virtual auto readString(std::string *text) -> bool = 0;
    virtual auto readString() -> bool = 0;
    virtual auto readBinary() -> bool = 0;
    virtual auto readInt16() -> bool = 0;
    virtual auto readInt32() -> bool = 0;
    virtual auto readInt64() -> bool = 0;
    virtual auto readDouble() -> bool = 0;
    virtual auto readBool() -> bool = 0;

protected:
    std::istream *filestr;
    std::string strData;
    double doubleData;
    signed int intData; //32 bits integer
    unsigned long long int int64; //64 bits integer
    bool skip; //set to true for ascii dxf, false for binary
private:
    Q_DISABLE_COPY_MOVE(dxfReader) // NOLINT
    DRW_TextCodec decoder;
    bool m_bIgnoreComments {false};
};

class dxfReaderBinary : public dxfReader {
public:
    explicit dxfReaderBinary(std::istream *stream)
        : dxfReader(stream)
    {skip = false; }

    virtual ~dxfReaderBinary() = default;
    virtual auto readCode(int *code) -> bool override;
    virtual auto readString(std::string *text) -> bool override;
    virtual auto readString() -> bool override;
    virtual auto readBinary() -> bool override;
    virtual auto readInt16() -> bool override;
    virtual auto readInt32() -> bool override;
    virtual auto readInt64() -> bool override;
    virtual auto readDouble() -> bool override;
    virtual auto readBool() -> bool override;
};

class dxfReaderAscii final : public dxfReader {
public:
    explicit dxfReaderAscii(std::istream *stream)
        : dxfReader(stream)
    {skip = true; }

    virtual ~dxfReaderAscii() = default;
    virtual auto readCode(int *code) -> bool override;
    virtual auto readString(std::string *text) -> bool override;
    virtual auto readString() -> bool override;
    virtual auto readBinary() -> bool override;
    virtual auto readInt16() -> bool override;
    virtual auto readDouble() -> bool override;
    virtual auto readInt32() -> bool override;
    virtual auto readInt64() -> bool override;
    virtual auto readBool() -> bool override;
};

#endif // DXFREADER_H
