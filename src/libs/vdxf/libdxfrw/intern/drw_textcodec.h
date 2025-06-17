#ifndef DRW_TEXTCODEC_H
#define DRW_TEXTCODEC_H

#include "../drw_base.h"
#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>
#include <array>
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
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_TextCodec) // NOLINT
    DRW::Version version{DRW::UNKNOWNV};
    std::string cp{};
    std::unique_ptr<DRW_Converter> conv;

    static auto correctCodePage(const std::string &s) -> std::string;
};

class DRW_Converter
{
public:
    DRW_Converter() = default;
    virtual ~DRW_Converter() = default;

    virtual auto fromUtf8(const std::string &s) -> std::string { return s; }
    virtual auto toUtf8(const std::string &s) -> std::string;

    static auto encodeText(const std::string &stmp) -> std::string;
    static auto decodeText(int c) -> std::string;
    static auto encodeNum(int c) -> std::string;
    static auto decodeNum(const std::string &s, unsigned int *b) -> int;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_Converter) // NOLINT
};

class DRW_ConvUTF16 : public DRW_Converter
{
public:
    DRW_ConvUTF16() = default;
    ~DRW_ConvUTF16() override = default;

    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_ConvUTF16) // NOLINT
};

class DRW_ConvTable : public DRW_Converter
{
public:
    explicit DRW_ConvTable(const std::array<int, 128> &t)
      : table{t}
    {
    }
    ~DRW_ConvTable() override = default;

    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_ConvTable) // NOLINT
    const std::array<int, 128> &table;
};

template <size_t DoubleTableSize> class DRW_ConvDBCSTable : public DRW_Converter
{
public:
    DRW_ConvDBCSTable(const std::array<int, 127> &lt, const std::array<std::array<int, 2>, DoubleTableSize> &dt)
      : leadTable{lt},
        doubleTable{dt}
    {
    }
    ~DRW_ConvDBCSTable() override = default;

    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_ConvDBCSTable) // NOLINT
    const std::array<int, 127> &leadTable;
    const std::array<std::array<int, 2>, DoubleTableSize> &doubleTable;
};

class DRW_Conv932Table : public DRW_Converter
{
public:
    DRW_Conv932Table() = default;
    ~DRW_Conv932Table() override = default;

    auto fromUtf8(const std::string &s) -> std::string override;
    auto toUtf8(const std::string &s) -> std::string override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_Conv932Table) // NOLINT
};

#endif // DRW_TEXTCODEC_H
