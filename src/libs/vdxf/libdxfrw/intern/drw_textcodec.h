#ifndef DRW_TEXTCODEC_H
#define DRW_TEXTCODEC_H

#include "../drw_base.h"
#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>
#include <string>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringList;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
class QTextCodec;
#else
class VTextCodec;
using QTextCodec = VTextCodec;
#endif // WITH_TEXTCODEC
#else
class QTextCodec;
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

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

    static auto DXFCodePageMap() -> QMap<QString, QStringList>;
    static auto CodecForName(const QString &name) -> QTextCodec *;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_TextCodec) // NOLINT
    DRW::Version version{DRW::UNKNOWNV};
    std::string cp{};
    QTextCodec *conv{nullptr};

    static auto correctCodePage(const std::string &s) -> std::string;
};

#endif // DRW_TEXTCODEC_H
