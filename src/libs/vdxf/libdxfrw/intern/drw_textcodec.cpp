#include "drw_textcodec.h"
#include "../drw_base.h"
#include "drw_cptable932.h"
#include "drw_cptable936.h"
#include "drw_cptable949.h"
#include "drw_cptable950.h"
#include "drw_cptables.h"
#include <QDebug>
#include <QSet>
#include <QString>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iomanip>
#include <memory>
#include <sstream>

DRW_TextCodec::DRW_TextCodec()
  : version(DRW::AC1021),
    conv(std::make_unique<DRW_Converter>())
{
}

void DRW_TextCodec::setVersion(DRW::Version v, bool dxfFormat)
{
    switch (v)
    {
        case DRW::UNKNOWNV:
        case DRW::MC00:
        case DRW::AC12:
        case DRW::AC14:
        case DRW::AC150:
        case DRW::AC210:
        case DRW::AC1002:
        case DRW::AC1003:
        case DRW::AC1004:
            // unhandled?
            break;
        case DRW::AC1006:
        case DRW::AC1009:
        {
            version = DRW::AC1009;
            cp = "ANSI_1252";
            setCodePage(cp, dxfFormat);
            break;
        }
        case DRW::AC1012:
        case DRW::AC1014:
        case DRW::AC1015:
        case DRW::AC1018:
        {
            version = DRW::AC1015;
            //            if (cp.empty()) { //codepage not set, initialize
            cp = "ANSI_1252";
            setCodePage(cp, dxfFormat);
            //            }
            break;
        }
        case DRW::AC1021:
        case DRW::AC1024:
        case DRW::AC1027:
        case DRW::AC1032:
        {
            version = DRW::AC1021;
            if (dxfFormat)
            {
                cp = "UTF-8"; // RLZ: can be UCS2 or UTF-16 16bits per char
            }
            else
            {
                cp = "UTF-16"; // RLZ: can be UCS2 or UTF-16 16bits per char
            }
            setCodePage(cp, dxfFormat);
            break;
        }
        default:
            break;
    }
}

void DRW_TextCodec::setVersion(const std::string &v, bool dxfFormat)
{
    version = DRW::UNKNOWNV;
    for (auto [first, second] : DRW::dwgVersionStrings)
    {
        if (std::strcmp(v.c_str(), first) == 0)
        {
            version = second;
            setVersion(second, dxfFormat);
            break;
        }
    }
}

void DRW_TextCodec::setCodePage(const std::string &c, bool dxfFormat)
{
    cp = correctCodePage(c);
    conv.reset();
    if (version == DRW::AC1009 || version == DRW::AC1015)
    {
        if (cp == "ANSI_874")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table874);
        }
        else if (cp == "ANSI_932")
        {
            conv = std::make_unique<DRW_Conv932Table>();
        }
        else if (cp == "ANSI_936")
        {
            conv = std::make_unique<DRW_ConvDBCSTable<CPLENGTH936>>(DRW_LeadTable936, DRW_DoubleTable936);
        }
        else if (cp == "ANSI_949")
        {
            conv = std::make_unique<DRW_ConvDBCSTable<CPLENGTH949>>(DRW_LeadTable949, DRW_DoubleTable949);
        }
        else if (cp == "ANSI_950")
        {
            conv = std::make_unique<DRW_ConvDBCSTable<CPLENGTH950>>(DRW_LeadTable950, DRW_DoubleTable950);
        }
        else if (cp == "ANSI_1250")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1250);
        }
        else if (cp == "ANSI_1251")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1251);
        }
        else if (cp == "ANSI_1253")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1253);
        }
        else if (cp == "ANSI_1254")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1254);
        }
        else if (cp == "ANSI_1255")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1255);
        }
        else if (cp == "ANSI_1256")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1256);
        }
        else if (cp == "ANSI_1257")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1257);
        }
        else if (cp == "ANSI_1258")
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1258);
        }
        else if (cp == "UTF-8")
        { // DXF older than 2007 are write in win codepages
            cp = "ANSI_1252";
            conv = std::make_unique<DRW_Converter>();
        }
        else
        {
            conv = std::make_unique<DRW_ConvTable>(DRW_Table1252);
        }
    }
    else
    {
        if (dxfFormat)
        {
            conv = std::make_unique<DRW_Converter>(); // utf16 to utf8
        }
        else
        {
            conv = std::make_unique<DRW_ConvUTF16>(); // utf16 to utf8
        }
    }
}

auto DRW_TextCodec::toUtf8(const std::string &s) -> std::string
{
    return conv->toUtf8(s);
}

auto DRW_TextCodec::fromUtf8(const std::string &s) -> std::string
{
    return conv->fromUtf8(s);
}

auto DRW_TextCodec::correctCodePage(const std::string &s) -> std::string
{
    // stringstream cause crash in OS/X, bug#3597944
    std::string cp = s;
    transform(cp.begin(), cp.end(), cp.begin(), toupper);

    static auto map = QMap<std::string, QSet<QString>>{
        // Latin/Thai
        {"ANSI_874", {"ANSI_874", "CP874", "ISO8859-11", "TIS-620"}}, // Central Europe and Eastern Europe
        {"ANSI_1250", {"ANSI_1250", "CP1250", "ISO8859-2"}},          // Cyrillic script
        {"ANSI_1251", {"ANSI_1251", "CP1251", "ISO8859-5", "KOI8-R", "KOI8-U", "IBM 866"}}, // Western Europe
        {"ANSI_1252",
         {"ANSI_1252", "CP1252", "LATIN1", "ISO-8859-1", "CP819", "CSISO", "IBM819", "ISO_8859-1", "APPLE ROMAN",
          "ISO8859-1", "ISO8859-15", "ISO-IR-100", "L1", "IBM 850"}},                    // Greek
        {"ANSI_1253", {"ANSI_1253", "CP1253", "ISO8859-7"}},                             // Turkish
        {"ANSI_1254", {"ANSI_1254", "CP1254", "ISO8859-9", "ISO8859-3"}},                // Hebrew
        {"ANSI_1255", {"ANSI_1255", "CP1255", "ISO8859-8"}},                             // Arabic
        {"ANSI_1256", {"ANSI_1256", "CP1256", "ISO8859-6"}},                             // Baltic
        {"ANSI_1257", {"ANSI_1257", "CP1257", "ISO8859-4", "ISO8859-10", "ISO8859-13"}}, // Vietnamese
        {"ANSI_1258", {"ANSI_1258", "CP1258"}},                                          // Japanese
        {"ANSI_932",
         {"ANSI_932", "SHIFT-JIS", "SHIFT_JIS", "CSSHIFTJIS", "CSWINDOWS31J", "MS_KANJI", "X-MS-CP932", "X-SJIS",
          "EUCJP", "EUC-JP", "CSEUCPKDFMTJAPANESE", "X-EUC", "X-EUC-JP", "JIS7"}}, // Chinese PRC GBK (XGB) simplified
        {"ANSI_936",
         {"ANSI_936", "GBK", "GB2312", "CHINESE", "CN-GB", "CSGB2312", "CSGB231280", "CSISO58BG231280", "GB_2312-80",
          "GB231280", "GB2312-80", "ISO-IR-58", "GB18030"}}, // Korean
        {"ANSI_949", {"ANSI_949", "EUCKR"}},                 // Chinese Big5 (Taiwan, Hong Kong SAR)
        {"ANSI_950", {"ANSI_950", "BIG5", "CN-BIG5", "CSBIG5", "X-X-BIG5", "BIG5-HKSCS"}}, // celtic
        //        {"ISO8859-14", {"ISO8859-14"}},
        //        {"TSCII", {"TSCII"}},//tamil
        {"UTF-8", {"UTF-8", "UTF8", "UTF8-BIT"}},
        {"UTF-16", {"UTF-16", "UTF16", "UTF16-BIT"}}};

    auto i = map.constBegin();
    while (i != map.constEnd())
    {
        // TODO: Since Qt 6.1 possible to use std::string instead of QString
        if (i.value().contains(QString::fromStdString(cp)))
        {
            return i.key();
        }
        ++i;
    }

    return "ANSI_1252";
}

auto DRW_Converter::toUtf8(const std::string &s) -> std::string
{
    std::string result;
    unsigned int j = 0;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        auto c = static_cast<unsigned char>(s.at(i));
        if (c < 0x80)
        { // ascii check for /U+????
            if (c == '\\' && i + 6 < s.length() && s.at(i + 1) == 'U' && s.at(i + 2) == '+')
            {
                result += s.substr(j, i - j);
                result += encodeText(s.substr(i, 7));
                i += 6;
                j = i + 1;
            }
        }
        else if (c < 0xE0)
        { // 2 bits
            i++;
        }
        else if (c < 0xF0)
        { // 3 bits
            i += 2;
        }
        else if (c < 0xF8)
        { // 4 bits
            i += 3;
        }
    }
    result += s.substr(j);

    return result;
}

auto DRW_ConvTable::fromUtf8(const std::string &s) -> std::string
{
    std::string result;
    bool notFound;
    int code;

    unsigned int j = 0;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        auto c = static_cast<unsigned char>(s.at(i));
        if (c > 0x7F)
        { // need to decode
            result += s.substr(j, i - j);
            std::string const part1 = s.substr(i, 4);
            unsigned int l;
            code = decodeNum(part1, &l);
            j = i + l;
            i = j - 1;
            notFound = true;
            for (size_t k = 0; k < table.size(); k++)
            {
                if (table[k] == code)
                {
                    result += static_cast<char>(CPOFFSET + k); // translate from table
                    notFound = false;
                    break;
                }
            }
            if (notFound)
            {
                result += decodeText(code);
            }
        }
    }
    result += s.substr(j);

    return result;
}

auto DRW_ConvTable::toUtf8(const std::string &s) -> std::string
{
    std::string res;
    for (auto it = s.begin(); it < s.end(); ++it)
    {
        auto c = static_cast<unsigned char>(*it);
        if (c < 0x80)
        {
            // check for \U+ encoded text
            if (c == '\\')
            {
                if (s.end() - it > 6 && *(it + 1) == 'U' && *(it + 2) == '+')
                {
                    res += encodeText(std::string(it, it + 7));
                    it += 6;
                }
                else
                {
                    res += static_cast<char>(c); // no \U+ encoded text write
                }
            }
            else
            {
                res += static_cast<char>(c); // c!='\' ascii char write
            }
        }
        else
        {                                                           // end c < 0x80
            res += encodeNum(table[static_cast<size_t>(c - 0x80)]); // translate from table
        }
    } // end for

    return res;
}

auto DRW_Converter::encodeText(const std::string &stmp) -> std::string
{
    int code;
#if defined(__APPLE__)
    int Succeeded = sscanf(&(stmp.substr(3, 4)[0]), "%x", &code);
    if (!Succeeded || Succeeded == EOF)
        code = 0;
#else
    std::istringstream sd(stmp.substr(3, 4));
    sd >> std::hex >> code;
#endif
    return encodeNum(code);
}

auto DRW_Converter::decodeText(int c) -> std::string
{
    std::string res = "\\U+";
    std::string num;
#if defined(__APPLE__)
    std::string str(16, '\0');
    snprintf(&(str[0]), 16, "%04X", c);
    num = str;
#else
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << c;
    ss >> num;
#endif
    res += num;
    return res;
}

auto DRW_Converter::encodeNum(int c) -> std::string
{
    std::string ret;

    if (c < 128)
    { // 0-7F US-ASCII 7 bits
        ret += static_cast<char>(c);
    }
    else if (c < 0x800)
    { // 80-07FF 2 bytes
        ret += static_cast<char>(0xC0 | (c >> 6));
        ret += static_cast<char>(0x80 | (c & 0x3F));
    }
    else if (c < 0x10000)
    { // 800-FFFF 3 bytes
        ret += static_cast<char>(0xE0 | (c >> 12));
        ret += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        ret += static_cast<char>(0x80 | (c & 0x3F));
    }
    else
    { // 10000-10FFFF 4 bytes
        ret += static_cast<char>(0xF0 | (c >> 18));
        ret += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
        ret += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        ret += static_cast<char>(0x80 | (c & 0x3F));
    }

    return ret;
}

/** 's' is a string with at least 4 bytes length
** returned 'b' is byte length of encoded char: 2,3 or 4
**/
auto DRW_Converter::decodeNum(const std::string &s, unsigned int *b) -> int
{
    int code = 0;
    auto c = static_cast<unsigned char>(s.at(0));
    if ((c & 0xE0) == 0xC0)
    { // 2 bytes
        code = (c & 0x1F) << 6;
        code = (s.at(1) & 0x3F) | code;
        *b = 2;
    }
    else if ((c & 0xF0) == 0xE0)
    { // 3 bytes
        code = (c & 0x0F) << 12;
        code = ((s.at(1) & 0x3F) << 6) | code;
        code = (s.at(2) & 0x3F) | code;
        *b = 3;
    }
    else if ((c & 0xF8) == 0xF0)
    { // 4 bytes
        code = (c & 0x07) << 18;
        code = ((s.at(1) & 0x3F) << 12) | code;
        code = ((s.at(2) & 0x3F) << 6) | code;
        code = (s.at(3) & 0x3F) | code;
        *b = 4;
    }

    return code;
}

template <size_t DoubleTableSize> auto DRW_ConvDBCSTable<DoubleTableSize>::fromUtf8(const std::string &s) -> std::string
{
    std::string result;
    bool notFound;
    int code;

    unsigned int j = 0;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        auto c = static_cast<unsigned char>(s.at(i));
        if (c > 0x7F)
        { // need to decode
            result += s.substr(j, i - j);
            std::string const part1 = s.substr(i, 4);
            unsigned int l;
            code = decodeNum(part1, &l);
            j = i + l;
            i = j - 1;
            notFound = true;
            for (const auto &row : doubleTable)
            {
                if (row[1] == code)
                {
                    int const data = row[0];
                    char d[3];
                    d[0] = static_cast<char>(data >> 8);
                    d[1] = static_cast<char>(data & 0xFF);
                    d[2] = '\0';
                    result += d; // translate from table
                    notFound = false;
                    break;
                }
            }
            if (notFound)
            {
                result += decodeText(code);
            }
        } // direct conversion
    }
    result += s.substr(j);

    return result;
}

template <size_t DoubleTableSize> auto DRW_ConvDBCSTable<DoubleTableSize>::toUtf8(const std::string &s) -> std::string
{
    std::string res;
    for (auto it = s.begin(); it < s.end(); ++it)
    {
        bool notFound = true;
        auto c = static_cast<unsigned char>(*it);
        if (c < 0x80)
        {
            notFound = false;
            // check for \U+ encoded text
            if (c == '\\')
            {
                if (s.end() - it > 6 && *(it + 1) == 'U' && *(it + 2) == '+')
                {
                    res += encodeText(std::string(it, it + 7));
                    it += 6;
                }
                else
                {
                    res += static_cast<char>(c); // no \U+ encoded text write
                }
            }
            else
            {
                res += static_cast<char>(c); // c!='\' ascii char write
            }
        }
        else if (c == 0x80)
        { // 1 byte table
            notFound = false;
            res += encodeNum(0x20AC); // euro sign
        }
        else
        { // 2 bytes
            ++it;
            int const code = (c << 8) | static_cast<unsigned char>(*it);
            int const sta = leadTable[static_cast<size_t>(c - 0x81)];
            int const end = leadTable[static_cast<size_t>(c - 0x80)];
            for (auto k = static_cast<size_t>(sta); k < static_cast<size_t>(end); k++)
            {
                if (doubleTable[k][0] == code)
                {
                    res += encodeNum(doubleTable[k][1]); // translate from table
                    notFound = false;
                    break;
                }
            }
        }
        // not found
        if (notFound)
        {
            res += encodeNum(NOTFOUND936);
        }
    } // end for

    return res;
}

auto DRW_Conv932Table::fromUtf8(const std::string &s) -> std::string
{
    std::string result;
    bool notFound;
    int code;

    unsigned int j = 0;
    for (unsigned int i = 0; i < s.length(); i++)
    {
        auto c = static_cast<unsigned char>(s.at(i));
        if (c > 0x7F)
        { // need to decode
            result += s.substr(j, i - j);
            std::string const part1 = s.substr(i, 4);
            unsigned int l;
            code = decodeNum(part1, &l);
            j = i + l;
            i = j - 1;
            notFound = true;
            // 1 byte table
            if (code > 0xff60 && code < 0xFFA0)
            {
                result += static_cast<char>(code - CPOFFSET932); // translate from table
                notFound = false;
            }
            if (notFound &&
                (code < 0xF8 || (code > 0x390 && code < 0x542) || (code > 0x200F && code < 0x9FA1) || code > 0xF928))
            {
                for (const auto &row : DRW_DoubleTable932)
                {
                    if (row[1] == code)
                    {
                        int const data = row[0];
                        char d[3];
                        d[0] = static_cast<char>(data >> 8);
                        d[1] = static_cast<char>(data & 0xFF);
                        d[2] = '\0';
                        result += d; // translate from table
                        notFound = false;
                        break;
                    }
                }
            }
            if (notFound)
                result += decodeText(code);
        } // direct conversion
    }
    result += s.substr(j);

    return result;
}

auto DRW_Conv932Table::toUtf8(const std::string &s) -> std::string
{
    std::string res;
    for (auto it = s.begin(); it < s.end(); ++it)
    {
        bool notFound = true;
        auto c = static_cast<unsigned char>(*it);
        if (c < 0x80)
        {
            notFound = false;
            // check for \U+ encoded text
            if (c == '\\')
            {
                if (s.end() - it > 6 && *(it + 1) == 'U' && *(it + 2) == '+')
                {
                    res += encodeText(std::string(it, it + 7));
                    it += 6;
                }
                else
                {
                    res += static_cast<char>(c); // no \U+ encoded text write
                }
            }
            else
            {
                res += static_cast<char>(c); // c!='\' ascii char write
            }
        }
        else if (c > 0xA0 && c < 0xE0)
        { // 1 byte table
            notFound = false;
            res += encodeNum(c + CPOFFSET932); // translate from table
        }
        else
        { // 2 bytes
            ++it;
            int const code = (c << 8) | static_cast<unsigned char>(*it);
            int sta = 0;
            int end = 0;
            if (c > 0x80 && c < 0xA0)
            {
                sta = DRW_LeadTable932[static_cast<size_t>(c - 0x81)];
                end = DRW_LeadTable932[static_cast<size_t>(c - 0x80)];
            }
            else if (c > 0xDF && c < 0xFD)
            {
                sta = DRW_LeadTable932[static_cast<size_t>(c - 0xC1)];
                end = DRW_LeadTable932[static_cast<size_t>(c - 0xC0)];
            }
            if (end > 0)
            {
                for (auto k = static_cast<size_t>(sta); k < static_cast<size_t>(end); k++)
                {
                    if (DRW_DoubleTable932[k][0] == code)
                    {
                        res += encodeNum(DRW_DoubleTable932[k][1]); // translate from table
                        notFound = false;
                        break;
                    }
                }
            }
        }
        // not found
        if (notFound)
        {
            res += encodeNum(NOTFOUND932);
        }
    } // end for

    return res;
}

auto DRW_ConvUTF16::fromUtf8(const std::string &s) -> std::string
{
    DRW_UNUSED(s);
    // RLZ: to be written (only needed for write dwg 2007+)
    return {};
}

auto DRW_ConvUTF16::toUtf8(const std::string &s) -> std::string
{ // RLZ: pending to write
    std::string res;
    for (auto it = s.begin(); it < s.end(); ++it)
    {
        auto c1 = static_cast<unsigned char>(*it);
        auto c2 = static_cast<unsigned char>(*(++it));
        auto ch = static_cast<duint16>((c2 << 8) | c1);
        res += encodeNum(ch);
    } // end for

    return res;
}
