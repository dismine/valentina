#ifndef DRW_TEXTCODEC_H
#define DRW_TEXTCODEC_H

#include "../drw_base.h"
#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>
#include <memory>
#include <string>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringList;
#endif

class DRW_Converter;

class DRW_TextCodec
{
public:
    DRW_TextCodec();
    ~DRW_TextCodec() = default;
    auto fromUtf8(const std::string &s) -> std::string;
    auto toUtf8(const std::string &s) -> std::string;
    auto getVersion() const -> int { return version; }
    void setVersion(const std::string &v, bool dxfFormat);
    void setVersion(DRW::Version v, bool dxfFormat);
    void setCodePage(const std::string &c, bool dxfFormat);
    auto getCodePage() const -> std::string { return cp; }

private:
    static auto correctCodePage(const std::string &s) -> std::string;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_TextCodec) // NOLINT
    DRW::Version version{DRW::UNKNOWNV};
    std::string cp{};
    std::unique_ptr<DRW_Converter> conv;
};

class DRW_Converter
{
public:
    DRW_Converter(const int *t, int l)
      : table{t},
        cpLength{l}
    {
    }
    virtual ~DRW_Converter() = default;
    virtual auto fromUtf8(const std::string &s) -> std::string { return s; }
    virtual auto toUtf8(const std::string &s) -> std::string;

    static auto encodeText(const std::string &stmp) -> std::string;
    static auto decodeText(int c) -> std::string;
    static auto encodeNum(int c) -> std::string;
    static auto decodeNum(const std::string &s, unsigned int *b) -> int;

    const int *table{nullptr};
    int cpLength;
};

class DRW_ConvUTF16 : public DRW_Converter
{
public:
    DRW_ConvUTF16()
      : DRW_Converter(nullptr, 0)
    {
    }
    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;
};

class DRW_ConvTable : public DRW_Converter
{
public:
    DRW_ConvTable(const int *t, int l)
      : DRW_Converter(t, l)
    {
    }
    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;
};

class DRW_ConvDBCSTable : public DRW_Converter
{
public:
    DRW_ConvDBCSTable(const int *t, const int *lt, const int dt[][2], int l)
      : DRW_Converter(t, l),
        leadTable{lt},
        doubleTable{dt}
    {
    }

    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;

private:
    const int *leadTable{nullptr};
    const int (*doubleTable)[2];
};

class DRW_Conv932Table : public DRW_Converter
{
public:
    DRW_Conv932Table();
    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;
};

#endif // DRW_TEXTCODEC_H
