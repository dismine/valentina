#ifndef DRW_TEXTCODEC_H
#define DRW_TEXTCODEC_H

#include <string>
#include <QtGlobal>
#include "../drw_base.h"
#include <QtCore/qcontainerfwd.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "../vmisc/vtextcodec.h"
#else
#include "../vmisc/defglobal.h"
#include <QTextCodec>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringList;
#endif

class DRW_TextCodec
{
public:
    DRW_TextCodec();
    std::string fromUtf8(const std::string &s);
    std::string toUtf8(const std::string &s);
    int getVersion() const {return version;}
    void setVersion(const std::string &v, bool dxfFormat);
    void setVersion(DRW::Version v, bool dxfFormat);
    void setCodePage(const std::string &c, bool dxfFormat);
    std::string getCodePage() const {return cp;}

    static QMap<QString, QStringList> DXFCodePageMap();
    static VTextCodec* CodecForName(const QString &name);

private:
    static auto correctCodePage(const std::string& s) -> std::string;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DRW_TextCodec) // NOLINT
    DRW::Version version{DRW::UNKNOWNV};
    std::string cp{};
    VTextCodec *conv{nullptr};
};

#endif // DRW_TEXTCODEC_H
