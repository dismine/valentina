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

#ifndef DXFWRITER_H
#define DXFWRITER_H

#include "drw_textcodec.h"

class dxfWriter {
public:
    explicit dxfWriter(std::ofstream *stream)
        : filestr(stream),
          encoder()
    {}

    virtual ~dxfWriter() = default;
    virtual auto writeString(int code, std::string text) -> bool = 0;
    auto writeUtf8String(int code, const std::string &text) -> bool;
    auto writeUtf8Caps(int code, const std::string &text) -> bool;
    auto fromUtf8String(const std::string &t) -> std::string { return encoder.fromUtf8(t); }
    virtual auto writeInt16(int code, int data) -> bool = 0;
    virtual auto writeInt32(int code, int data) -> bool = 0;
    virtual auto writeInt64(int code, unsigned long long int data) -> bool = 0;
    virtual auto writeDouble(int code, double data) -> bool = 0;
    virtual auto writeBool(int code, bool data) -> bool = 0;
    void setVersion(const std::string &v, bool dxfFormat){encoder.setVersion(v, dxfFormat);}
    void setCodePage(const std::string &c){encoder.setCodePage(c, true);}
    auto getCodePage() const -> std::string { return encoder.getCodePage(); }

protected:
    std::ofstream *filestr;
private:
    Q_DISABLE_COPY_MOVE(dxfWriter) // NOLINT
    DRW_TextCodec encoder;
};

class dxfWriterBinary : public dxfWriter {
public:
    explicit dxfWriterBinary(std::ofstream *stream)
        : dxfWriter(stream)
    {}
    virtual ~dxfWriterBinary() = default;
    virtual auto writeString(int code, std::string text) -> bool override;
    virtual auto writeInt16(int code, int data) -> bool override;
    virtual auto writeInt32(int code, int data) -> bool override;
    virtual auto writeInt64(int code, unsigned long long int data) -> bool override;
    virtual auto writeDouble(int code, double data) -> bool override;
    virtual auto writeBool(int code, bool data) -> bool override;
};

class dxfWriterAscii final : public dxfWriter {
public:
    explicit dxfWriterAscii(std::ofstream *stream);
    virtual ~dxfWriterAscii() = default;
    virtual auto writeString(int code, std::string text) -> bool override;
    virtual auto writeInt16(int code, int data) -> bool override;
    virtual auto writeInt32(int code, int data) -> bool override;
    virtual auto writeInt64(int code, unsigned long long int data) -> bool override;
    virtual auto writeDouble(int code, double data) -> bool override;
    virtual auto writeBool(int code, bool data) -> bool override;
};

#endif // DXFWRITER_H
