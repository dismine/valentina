// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#include <qtclasshelpermacros.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringconverter.h>

#ifndef WITH_TEXTCODEC
#error \
    "QTextCodec support is required but has not been enabled. Please enable the withTextCodec in your build configuration."
#endif

QT_BEGIN_NAMESPACE

class QTextDecoder;
class QTextEncoder;

class QTextCodec : public QStringConverterBase
{
    Q_DISABLE_COPY_MOVE(QTextCodec)

public:
    using ConversionFlags = QStringConverterBase::Flags;
    using ConverterState = QStringConverterBase::State;

    static constexpr Flag ConvertInvalidToNull = Flag::ConvertInvalidToNull;
    static constexpr Flag DefaultConversion = Flag::WriteBom;
    static constexpr Flag IgnoreHeader = Flag::ConvertInitialBom;

    static QTextCodec* codecForName(const QByteArray &name);
    static QTextCodec* codecForName(const char *name) { return codecForName(QByteArray(name)); }
    static QTextCodec* codecForMib(int mib);

    static QList<QByteArray> availableCodecs();
    static QList<int> availableMibs();

    static QTextCodec* codecForLocale();
    static void setCodecForLocale(QTextCodec *c);

    static QTextCodec *codecForTr() { return codecForMib(106); /* Utf8 */ }

    static QTextCodec *codecForHtml(const QByteArray &ba);
    static QTextCodec *codecForHtml(const QByteArray &ba, QTextCodec *defaultCodec);

    static QTextCodec *codecForUtfText(const QByteArray &ba);
    static QTextCodec *codecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec);

    bool canEncode(QChar) const;
    bool canEncode(const QString&) const;
    bool canEncode(QStringView) const;

    QString toUnicode(const QByteArray&) const;
    QString toUnicode(const char* chars) const;
    QByteArray fromUnicode(const QString& uc) const;
    QByteArray fromUnicode(QStringView uc) const;

    QString toUnicode(const char *in, int length, ConverterState *state = nullptr) const
    {
        if (in == nullptr)
            return {};
        if (length <= 0)
            return QLatin1String("");
        return convertToUnicode(in, length, state);
    }
    QByteArray fromUnicode(const QChar *in, int length, ConverterState *state = nullptr) const
    {
        if (in == nullptr)
            return {};
        if (length <= 0)
            return {""};
        return convertFromUnicode(in, length, state);
    }

    QTextDecoder* makeDecoder(ConversionFlags flags = DefaultConversion) const;
    QTextEncoder* makeEncoder(ConversionFlags flags = DefaultConversion) const;

    virtual QByteArray name() const = 0;
    virtual QList<QByteArray> aliases() const;
    virtual int mibEnum() const = 0;

protected:
    virtual ~QTextCodec();
    virtual QString convertToUnicode(const char *in, int length, ConverterState *state) const = 0;
    virtual QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const = 0;

    QTextCodec();
private:
    friend struct QTextCodecData;
};

class QTextEncoder
{
    Q_DISABLE_COPY_MOVE(QTextEncoder)

public:
    explicit QTextEncoder(const QTextCodec *codec)
        : c(codec), state(QTextCodec::DefaultConversion) {}
    explicit QTextEncoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);
    ~QTextEncoder();
    QByteArray fromUnicode(const QString& str);
    QByteArray fromUnicode(QStringView str);
    QByteArray fromUnicode(const QChar *uc, int len);
    bool hasFailure() const;
private:
    const QTextCodec *c = nullptr;
    QTextCodec::ConverterState state;
};

class QTextDecoder
{
    Q_DISABLE_COPY_MOVE(QTextDecoder)

public:
    explicit QTextDecoder(const QTextCodec *codec)
        : c(codec), state(QTextCodec::DefaultConversion) {}
    explicit QTextDecoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);
    ~QTextDecoder();
    QString toUnicode(const char* chars, int len);
    QString toUnicode(const QByteArray &ba);
    void toUnicode(QString *target, const char *chars, int len);
    bool hasFailure() const;
    bool needsMoreData() const;
private:
    const QTextCodec *c = nullptr;
    QTextCodec::ConverterState state;
};

namespace Qt
{
QTextCodec *codecForHtml(const QByteArray &ba);
}

QT_END_NAMESPACE

#endif // QTEXTCODEC_H
