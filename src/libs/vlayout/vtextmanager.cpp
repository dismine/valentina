/************************************************************************
 **
 **  @file   vpatternpiecedata.cpp
 **  @author Bojan Kverh
 **  @date   July 19, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QFlags> // QFlags<Qt::Alignment>
#include <QFontMetrics>
#include <QGlobalStatic>
#include <QLatin1String>
#include <QRegularExpression>
#include <QtMath>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/svgfont/vsvgfont.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
#include "../vmisc/vdatastreamenum.h"
#endif

#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vcontainer.h"
#include "vtextmanager.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QVector<TextLine>, m_patternLabelLinesCache) // NOLINT

QT_WARNING_POP

auto SplitTextByWidth(const QString &text, const QFont &font, int maxWidth) -> QStringList
{
    QFontMetrics fontMetrics(font);
    if (TextWidth(fontMetrics, text) <= maxWidth)
    {
        return {text};
    }

    QStringList substrings;
    substrings.reserve(2);

    const int textLength = static_cast<int>(text.length());
    int lineWidth = 0;

    for (int endIndex = 0; endIndex < textLength; ++endIndex)
    {
        QChar currentChar = text.at(endIndex);
        const int charWidth = TextWidth(fontMetrics, currentChar);

        if (lineWidth + charWidth > maxWidth)
        {
            if (endIndex > 0)
            {
                substrings.append(text.mid(0, endIndex));
            }

            if (endIndex < textLength)
            {
                substrings.append(text.mid(endIndex));
            }

            break;
        }

        lineWidth += charWidth;
    }

    return substrings;
}

//---------------------------------------------------------------------------------------------------------------------
auto SplitTextByWidth(const QString &text, const VSvgFont &font, int maxWidth, qreal penWidth) -> QStringList
{
    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine = db->FontEngine(font);

    if (engine.TextWidth(text, penWidth) <= maxWidth)
    {
        return {text};
    }

    QStringList substrings;
    substrings.reserve(2);

    const int textLength = static_cast<int>(text.length());
    qreal lineWidth = 0;

    for (int endIndex = 0; endIndex < textLength; ++endIndex)
    {
        QChar currentChar = text.at(endIndex);
        const qreal charWidth = engine.TextWidth(currentChar, penWidth);

        if (lineWidth + charWidth > maxWidth)
        {
            if (endIndex > 0)
            {
                substrings.append(text.mid(0, endIndex));
            }

            if (endIndex < textLength)
            {
                substrings.append(text.mid(endIndex));
            }

            break;
        }

        lineWidth += charWidth;
    }

    return substrings;
}
} // namespace

const quint32 TextLine::streamHeader = 0xA3881E49; // CRC-32Q string "TextLine"
const quint16 TextLine::classVersion = 1;

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const TextLine &data) -> QDataStream &
{
    dataStream << TextLine::streamHeader << TextLine::classVersion;

    // Added in classVersion = 1
    dataStream << data.m_qsText;
    dataStream << data.m_iFontSize;
    dataStream << data.m_bold;
    dataStream << data.m_italic;
    dataStream << data.m_eAlign;

    // Added in classVersion = 2

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, TextLine &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != TextLine::streamHeader)
    {
        QString message = QCoreApplication::tr("TextLine prefix mismatch error: actualStreamHeader = 0x%1 and "
                                               "streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                              .arg(TextLine::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > TextLine::classVersion)
    {
        QString message = QCoreApplication::tr("TextLine compatibility error: actualClassVersion = %1 and "
                                               "classVersion = %2")
                              .arg(actualClassVersion)
                              .arg(TextLine::classVersion);
        throw VException(message);
    }

    dataStream >> data.m_qsText;
    dataStream >> data.m_iFontSize;
    dataStream >> data.m_bold;
    dataStream >> data.m_italic;
    dataStream >> data.m_eAlign;

    //    if (actualClassVersion >= 2)
    //    {

    //    }

    return dataStream;
}

const quint32 VTextManager::streamHeader = 0x47E6A9EE; // CRC-32Q string "VTextManager"
const quint16 VTextManager::classVersion = 2;

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VTextManager &data) -> QDataStream &
{
    dataStream << VTextManager::streamHeader << VTextManager::classVersion;

    // Added in classVersion = 1
    dataStream << data.m_font;
    dataStream << data.m_liLines;

    // Added in classVersion = 2
    dataStream << data.m_svgFontFamily;
    dataStream << data.m_svgFontPointSize;

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VTextManager &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VTextManager::streamHeader)
    {
        QString message = QCoreApplication::tr("VTextManager prefix mismatch error: actualStreamHeader = 0x%1 and "
                                               "streamHeader = 0x%2")
                              .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                              .arg(VTextManager::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VTextManager::classVersion)
    {
        QString message = QCoreApplication::tr("VTextManager compatibility error: actualClassVersion = %1 and "
                                               "classVersion = %2")
                              .arg(actualClassVersion)
                              .arg(VTextManager::classVersion);
        throw VException(message);
    }

    dataStream >> data.m_font;
    dataStream >> data.m_liLines;

    if (actualClassVersion >= 2)
    {
        dataStream >> data.m_svgFontFamily;
        dataStream >> data.m_svgFontPointSize;
    }

    return dataStream;
}

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC(QDateTime, placeholdersExpirationTime) // NOLINT

using VPlaceholdersCache = QMap<QString, QString>;
Q_GLOBAL_STATIC(VPlaceholdersCache, placeholdersCache) // NOLINT

QT_WARNING_POP

const qint64 placeholdersExpirationTimeout = 15; // seconds

//---------------------------------------------------------------------------------------------------------------------
void PrepareMeasurementsPlaceholders(const VContainer *data, QMap<QString, QString> &placeholders)
{
    SCASSERT(data != nullptr)

    const QMap<QString, QSharedPointer<VMeasurement>> measurements = data->DataMeasurements();
    auto i = measurements.constBegin();
    while (i != measurements.constEnd())
    {
        placeholders.insert(pl_measurement + i.key(), QString::number(*i.value()->GetValue()));
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareCustomerPlaceholders(const VAbstractPattern *doc, QMap<QString, QString> &placeholders)
{
    QLocale locale(VAbstractApplication::VApp()->Settings()->GetLocale());

    if (VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Individual)
    {
        placeholders.insert(pl_customer, VAbstractValApplication::VApp()->GetCustomerName());

        const QString birthDate =
            locale.toString(VAbstractValApplication::VApp()->GetCustomerBirthDate(), doc->GetLabelDateFormat());
        placeholders.insert(pl_birthDate, birthDate);

        placeholders.insert(pl_email, VAbstractValApplication::VApp()->CustomerEmail());
    }
    else
    {
        placeholders.insert(pl_customer, doc->GetCustomerName());

        const QString birthDate = locale.toString(doc->GetCustomerBirthDate(), doc->GetLabelDateFormat());
        placeholders.insert(pl_birthDate, birthDate);

        placeholders.insert(pl_email, doc->GetCustomerEmail());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareDimensionPlaceholders(QMap<QString, QString> &placeholders)
{
    QString heightValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHeight());
    placeholders.insert(pl_height, heightValue);
    placeholders.insert(pl_dimensionX, heightValue);

    QString sizeValue = QString::number(VAbstractValApplication::VApp()->GetDimensionSize());
    placeholders.insert(pl_size, sizeValue);
    placeholders.insert(pl_dimensionY, sizeValue);

    QString hipValue = QString::number(VAbstractValApplication::VApp()->GetDimensionHip());
    placeholders.insert(pl_hip, hipValue);
    placeholders.insert(pl_dimensionZ, hipValue);

    QString waistValue = QString::number(VAbstractValApplication::VApp()->GetDimensionWaist());
    placeholders.insert(pl_waist, waistValue);
    placeholders.insert(pl_dimensionW, waistValue);

    {
        QString label = VAbstractValApplication::VApp()->GetDimensionHeightLabel();
        placeholders.insert(pl_heightLabel, not label.isEmpty() ? label : heightValue);

        label = VAbstractValApplication::VApp()->GetDimensionSizeLabel();
        placeholders.insert(pl_sizeLabel, not label.isEmpty() ? label : sizeValue);

        label = VAbstractValApplication::VApp()->GetDimensionHipLabel();
        placeholders.insert(pl_hipLabel, not label.isEmpty() ? label : hipValue);

        label = VAbstractValApplication::VApp()->GetDimensionWaistLabel();
        placeholders.insert(pl_waistLabel, not label.isEmpty() ? label : waistValue);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareUserMaterialsPlaceholders(const VAbstractPattern *doc, QMap<QString, QString> &placeholders)
{
    const QMap<int, QString> materials = doc->GetPatternMaterials();
    for (int i = 0; i < userMaterialPlaceholdersQuantity; ++i)
    {
        const QString number = QString::number(i + 1);

        QString value;
        if (materials.contains(i + 1))
        {
            value = materials.value(i + 1);
        }

        placeholders.insert(pl_userMaterial + number, value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareFinalMeasurementsPlaceholders(const VAbstractPattern *doc, bool pieceLabel,
                                          const QString &pieceAreaShortName, QMap<QString, QString> &placeholders)
{
    VContainer completeData = doc->GetCompleteData();
    completeData.FillPiecesAreas(VAbstractValApplication::VApp()->patternUnits());

    if (pieceLabel)
    {
        QScopedPointer<Calculator> cal(new Calculator());

        try
        {
            const QString formula = pieceArea_ + pieceAreaShortName;
            const qreal result = cal->EvalFormula(completeData.DataVariables(), formula);
            placeholders[pl_currentArea] = QString::number(result);
        }
        catch (qmu::QmuParserError &e)
        {
            const QString errorMsg = QObject::tr("Failed to prepare full piece area placeholder. %1.").arg(e.GetMsg());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }

        try
        {
            const QString formula = pieceSeamLineArea_ + pieceAreaShortName;
            const qreal result = cal->EvalFormula(completeData.DataVariables(), formula);
            placeholders[pl_currentSeamLineArea] = QString::number(result);
        }
        catch (qmu::QmuParserError &e)
        {
            const QString errorMsg =
                QObject::tr("Failed to prepare piece seam line area placeholder. %1.").arg(e.GetMsg());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    }
    else
    {
        placeholders.insert(pl_currentArea, QString());
        placeholders.insert(pl_currentSeamLineArea, QString());
    }

    const QVector<VFinalMeasurement> measurements = doc->GetFinalMeasurements();
    for (int i = 0; i < measurements.size(); ++i)
    {
        const VFinalMeasurement &m = measurements.at(i);

        try
        {
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(completeData.DataVariables(), m.formula);

            placeholders.insert(pl_finalMeasurement + m.name, QString::number(result));
        }
        catch (qmu::QmuParserError &e)
        {
            const QString errorMsg = QObject::tr("Failed to prepare final measurement placeholder. Parser error at "
                                                 "line %1: %2.")
                                         .arg(i + 1)
                                         .arg(e.GetMsg());
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto PreparePlaceholders(const VAbstractPattern *doc, const VContainer *data, bool pieceLabel = false,
                         const QString &pieceAreaShortName = QString()) -> QMap<QString, QString>
{
    SCASSERT(doc != nullptr)
    SCASSERT(data != nullptr)

    if (placeholdersExpirationTime->isValid() && QDateTime::currentDateTime() <= *placeholdersExpirationTime)
    {
        *placeholdersExpirationTime = QDateTime::currentDateTime().addSecs(placeholdersExpirationTimeout);
        return *placeholdersCache;
    }

    QMap<QString, QString> placeholders;

    // Pattern tags
    QLocale locale(VAbstractApplication::VApp()->Settings()->GetLocale());

    const QString date = locale.toString(QDate::currentDate(), doc->GetLabelDateFormat());
    placeholders.insert(pl_date, date);

    const QString time = locale.toString(QTime::currentTime(), doc->GetLabelTimeFormat());
    placeholders.insert(pl_time, time);

    placeholders.insert(pl_patternName, doc->GetPatternName());
    placeholders.insert(pl_patternNumber, doc->GetPatternNumber());
    placeholders.insert(pl_author, doc->GetCompanyName());

    placeholders.insert(pl_mUnits, UnitsToStr(VAbstractValApplication::VApp()->MeasurementsUnits(), true));
    const QString pUnits = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    placeholders.insert(pl_pUnits, pUnits);
    placeholders.insert(pl_mSizeUnits, UnitsToStr(VAbstractValApplication::VApp()->DimensionSizeUnits(), true));
    placeholders.insert(pl_areaUnits, pUnits + QStringLiteral("²"));

    PrepareCustomerPlaceholders(doc, placeholders);

    placeholders.insert(pl_pExt, QStringLiteral("val"));
    placeholders.insert(pl_pFileName, QFileInfo(VAbstractValApplication::VApp()->GetPatternPath()).baseName());
    placeholders.insert(pl_mFileName, QFileInfo(doc->MPath()).baseName());

    PrepareDimensionPlaceholders(placeholders);

    placeholders.insert(pl_mExt, VAbstractValApplication::VApp()->GetMeasurementsType() == MeasurementsType::Multisize
                                     ? QStringLiteral("vst")
                                     : QStringLiteral("vit"));

    PrepareUserMaterialsPlaceholders(doc, placeholders);
    PrepareMeasurementsPlaceholders(data, placeholders);
    PrepareFinalMeasurementsPlaceholders(doc, pieceLabel, pieceAreaShortName, placeholders);

    // Piece tags
    placeholders.insert(pl_pLetter, QString());
    placeholders.insert(pl_pAnnotation, QString());
    placeholders.insert(pl_pOrientation, QString());
    placeholders.insert(pl_pRotation, QString());
    placeholders.insert(pl_pTilt, QString());
    placeholders.insert(pl_pFoldPosition, QString());
    placeholders.insert(pl_pName, QString());
    placeholders.insert(pl_pQuantity, QString());
    placeholders.insert(pl_wOnFold, QString());

    QSharedPointer<QTranslator> phTr = VAbstractValApplication::VApp()->GetPlaceholderTranslator();

    placeholders.insert(pl_mFabric, phTr->translate("Placeholder", "Fabric"));
    placeholders.insert(pl_mLining, phTr->translate("Placeholder", "Lining"));
    placeholders.insert(pl_mInterfacing, phTr->translate("Placeholder", "Interfacing"));
    placeholders.insert(pl_mInterlining, phTr->translate("Placeholder", "Interlining"));
    placeholders.insert(pl_wCut, phTr->translate("Placeholder", "Cut"));

    *placeholdersCache = placeholders;
    *placeholdersExpirationTime = QDateTime::currentDateTime().addSecs(placeholdersExpirationTimeout);

    return placeholders;
}

//---------------------------------------------------------------------------------------------------------------------
void InitPiecePlaceholders(QMap<QString, QString> &placeholders, const QString &name, const VPieceLabelData &data)
{
    placeholders[pl_pLetter] = data.GetLetter();
    placeholders[pl_pAnnotation] = data.GetAnnotation();
    placeholders[pl_pOrientation] = data.GetOrientation();
    placeholders[pl_pRotation] = data.GetRotationWay();
    placeholders[pl_pTilt] = data.GetTilt();
    placeholders[pl_pFoldPosition] = data.GetFoldPosition();
    placeholders[pl_pName] = name;
    placeholders[pl_pQuantity] = QString::number(data.GetQuantity());

    if (data.IsOnFold())
    {
        QSharedPointer<QTranslator> phTr = VAbstractValApplication::VApp()->GetPlaceholderTranslator();
        placeholders[pl_wOnFold] = phTr->translate("Placeholder", "on fold");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplacePlaceholders(const QMap<QString, QString> &placeholders, QString line) -> QString
{
    QChar per('%');

    auto TestDimension = [per, placeholders, line](const QString &placeholder, const QString &errorMsg)
    {
        if (line.contains(per + placeholder + per) && placeholders.value(placeholder) == '0'_L1)
        {
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    };

    TestDimension(pl_height, QObject::tr("No data for the height dimension."));
    TestDimension(pl_size, QObject::tr("No data for the size dimension."));
    TestDimension(pl_hip, QObject::tr("No data for the hip dimension."));
    TestDimension(pl_waist, QObject::tr("No data for the waist dimension."));

    TestDimension(pl_dimensionX, QObject::tr("No data for the X dimension."));
    TestDimension(pl_dimensionY, QObject::tr("No data for the Y dimension."));
    TestDimension(pl_dimensionZ, QObject::tr("No data for the Z dimension."));
    TestDimension(pl_dimensionW, QObject::tr("No data for the W dimension."));

    auto i = placeholders.constBegin();
    while (i != placeholders.constEnd())
    {
        line.replace(per + i.key() + per, i.value());
        ++i;
    }
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareLines(const QVector<VLabelTemplateLine> &lines) -> QVector<TextLine>
{
    QVector<TextLine> textLines;

    for (const auto &line : lines)
    {
        if (not line.line.isEmpty())
        {
            TextLine tl;
            tl.m_qsText = line.line;
            tl.m_eAlign = static_cast<Qt::Alignment>(line.alignment);
            tl.m_iFontSize = line.fontSizeIncrement;
            tl.m_bold = line.bold;
            tl.m_italic = line.italic;

            textLines << tl;
        }
    }

    return textLines;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetSpacing returns the vertical spacing between the lines
 * @return spacing
 */
auto VTextManager::GetSpacing() const -> int
{
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFont set the text base font
 * @param font text base font
 */
void VTextManager::SetFont(const QFont &font)
{
    m_font = font;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFont returns the text base font
 * @return text base font
 */
auto VTextManager::GetFont() const -> const QFont &
{
    return m_font;
}

//---------------------------------------------------------------------------------------------------------------------
void VTextManager::SetSVGFontFamily(const QString &fontFamily)
{
    m_svgFontFamily = fontFamily;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::GetSVGFontFamily() const -> QString
{
    return m_svgFontFamily;
}

//---------------------------------------------------------------------------------------------------------------------
void VTextManager::SetSVGFontPointSize(int pointSize)
{
    m_svgFontPointSize = pointSize < VCommonSettings::MinPieceLabelFontPointSize()
                             ? VCommonSettings::MinPieceLabelFontPointSize()
                             : pointSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::GetSVGFontPointSize() const -> int
{
    return m_svgFontPointSize;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFontSize sets the font size
 * @param iFS font size in pixels
 */
void VTextManager::SetFontSize(int iFS)
{
    iFS < VCommonSettings::MinPieceLabelFontPointSize()
        ? m_font.setPointSize(qMax(VCommonSettings::MinPieceLabelFontPointSize(), 1))
        : m_font.setPointSize(qMax(iFS, 1));
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::GetAllSourceLines() const -> QVector<TextLine>
{
    return m_liLines;
}

//---------------------------------------------------------------------------------------------------------------------
void VTextManager::SetAllSourceLines(const QVector<TextLine> &lines)
{
    m_liLines = lines;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextManager::GetSourceLinesCount returns the number of input text lines
 * @return number of text lines that were added to the list by calling AddLine
 */
auto VTextManager::GetSourceLinesCount() const -> vsizetype
{
    return m_liLines.count();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextManager::GetSourceLine returns the reference to i-th text line
 * @param i index of the requested line
 * @return reference to the requested TextLine object
 */
auto VTextManager::GetSourceLine(vsizetype i) const -> const TextLine &
{
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < m_liLines.count());
    return m_liLines.at(i);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::GetLabelSourceLines(int width, const QFont &font) const -> QVector<TextLine>
{
    QVector<TextLine> lines;
    lines.reserve(m_liLines.size());
    QFont fnt = font;
    int fSize = m_font.pointSize();

    for (const auto &tl : m_liLines)
    {
        fnt.setPointSize(qMax(fSize + tl.m_iFontSize, 1));
        fnt.setBold(tl.m_bold);
        fnt.setItalic(tl.m_italic);

        QString qsText = tl.m_qsText;
        QFontMetrics fm(fnt);
        if (TextWidth(fm, qsText) > width)
        {
            const QStringList brokeLines = BreakTextIntoLines(qsText, fnt, width);
            for (const auto &lineText : brokeLines)
            {
                TextLine line = tl;
                line.m_qsText = lineText;
                lines.append(line);
            }
        }
        else
        {
            lines.append(tl);
        }
    }
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::GetLabelSourceLines(int width, const VSvgFont &font, qreal penWidth) const -> QVector<TextLine>
{
    if (!font.IsValid())
    {
        return m_liLines;
    }

    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    QVector<TextLine> lines;
    lines.reserve(m_liLines.size());
    int fSize = m_font.pointSize();

    for (const auto &tl : m_liLines)
    {
        VSvgFont lineFont = font;
        lineFont.SetPointSize(fSize + tl.m_iFontSize);
        lineFont.SetBold(tl.m_bold);
        lineFont.SetItalic(tl.m_italic);

        VSvgFontEngine engine = db->FontEngine(lineFont);

        VSvgFont svgFont = engine.Font();
        if (!svgFont.IsValid())
        {
            lines.append(tl);
            continue;
        }

        QString qsText = tl.m_qsText;
        if (engine.TextWidth(qsText, penWidth) > width)
        {
            const QStringList brokeLines = BreakTextIntoLines(qsText, svgFont, width, penWidth);
            for (const auto &lineText : brokeLines)
            {
                TextLine line = tl;
                line.m_qsText = lineText;
                lines.append(line);
            }
        }
        else
        {
            lines.append(tl);
        }
    }
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::MaxLineWidthOutlineFont(int width) const -> int
{
    int maxWidth = 0;
    for (int i = 0; i < m_liLines.count(); ++i)
    {
        const TextLine &tl = m_liLines.at(i);

        QFont fnt = m_font;
        fnt.setPointSize(qMax(fnt.pointSize() + tl.m_iFontSize, 1));
        fnt.setBold(tl.m_bold);
        fnt.setItalic(tl.m_italic);

        QFontMetrics fm(fnt);

        QString qsText = tl.m_qsText;

        if (TextWidth(fm, qsText) > width)
        {
            qsText = fm.elidedText(qsText, Qt::ElideMiddle, width);
        }

        maxWidth = qMax(TextWidth(fm, qsText), maxWidth);
    }

    return maxWidth;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::MaxLineWidthSVGFont(int width, qreal penWidth) const -> int
{
    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine =
        db->FontEngine(m_svgFontFamily, SVGFontStyle::Normal, SVGFontWeight::Normal, m_svgFontPointSize);
    VSvgFont svgFont = engine.Font();

    int maxWidth = 0;
    for (int i = 0; i < m_liLines.count(); ++i)
    {
        const TextLine &tl = m_liLines.at(i);

        VSvgFont fnt = svgFont;
        fnt.SetPointSize(fnt.PointSize() + tl.m_iFontSize);
        fnt.SetBold(tl.m_bold);
        fnt.SetItalic(tl.m_italic);

        engine = db->FontEngine(fnt);

        QString qsText = tl.m_qsText;

        if (engine.TextWidth(qsText, penWidth) > width)
        {
            qsText = engine.ElidedText(qsText, SVGTextElideMode::ElideMiddle, width);
        }

        maxWidth = qMax(qRound(engine.TextWidth(qsText, penWidth)), maxWidth);
    }

    return maxWidth;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextManager::Update updates the text lines with detail data
 * @param qsName detail name
 * @param data reference to the detail data
 */
void VTextManager::Update(const QString &qsName, const VPieceLabelData &data, const VContainer *pattern)
{
    m_liLines.clear();

    QMap<QString, QString> placeholders = PreparePlaceholders(VAbstractValApplication::VApp()->getCurrentDocument(),
                                                              pattern, true, data.GetAreaShortName());
    InitPiecePlaceholders(placeholders, qsName, data);

    QVector<VLabelTemplateLine> lines = data.GetLabelTemplate();

    for (auto &line : lines)
    {
        line.line = ReplacePlaceholders(placeholders, line.line);
    }

    m_liLines = PrepareLines(lines);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextManager::Update updates the text lines with pattern info
 * @param pDoc pointer to the abstract pattern object
 */
void VTextManager::Update(VAbstractPattern *pDoc, const VContainer *pattern)
{
    m_liLines.clear();

    if (m_patternLabelLinesCache->isEmpty() || pDoc->GetPatternWasChanged())
    {
        QVector<VLabelTemplateLine> lines = pDoc->GetPatternLabelTemplate();
        if (lines.isEmpty() && m_patternLabelLinesCache->isEmpty())
        {
            return; // Nothing to parse
        }

        const QMap<QString, QString> placeholders = PreparePlaceholders(pDoc, pattern);

        for (auto &line : lines)
        {
            line.line = ReplacePlaceholders(placeholders, line.line);
        }

        pDoc->SetPatternWasChanged(false);
        *m_patternLabelLinesCache = PrepareLines(lines);
    }

    m_liLines = *m_patternLabelLinesCache;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::BreakTextIntoLines(const QString &text, const QFont &font, int maxWidth) const -> QStringList
{
    QFontMetrics fontMetrics(font);
    QStringList words = text.split(' ');

    QString currentLine;
    int currentLineWidth = 0;
    const int spaceWidth = TextWidth(fontMetrics, QChar(' '));
    const float tolerance = 0.3F;

    QStringList lines;
    lines.reserve(words.size());
    QMutableListIterator<QString> iterator(words);

    auto AppendWord = [&currentLine, &currentLineWidth](const QString &word, int totalWidth)
    {
        if (!currentLine.isEmpty())
        {
            currentLine += ' '_L1;
        }
        currentLine += word;
        currentLineWidth = totalWidth;
    };

    while (iterator.hasNext())
    {
        const QString &word = iterator.next();
        int wordWidth = TextWidth(fontMetrics, word);
        int totalWidth = !currentLine.isEmpty() ? currentLineWidth + spaceWidth + wordWidth : wordWidth;

        if (totalWidth <= maxWidth)
        {
            // Append the word to the current line
            AppendWord(word, totalWidth);
        }
        else if ((maxWidth - currentLineWidth) <= qRound(static_cast<float>(maxWidth) * tolerance) &&
                 maxWidth >= wordWidth)
        {
            // Start a new line with the word if it doesn't exceed the tolerance
            lines.append(currentLine);
            currentLine = word;
            currentLineWidth = wordWidth;
        }
        else
        {
            // Word is too long, force line break
            if (currentLineWidth + spaceWidth + TextWidth(fontMetrics, word.at(0)) > maxWidth)
            {
                lines.append(currentLine);
                currentLine.clear();
                currentLineWidth = 0;
            }

            const int subWordWidth = !currentLine.isEmpty() ? maxWidth - (currentLineWidth + spaceWidth) : maxWidth;
            const QStringList subWords = SplitTextByWidth(word, font, subWordWidth);

            if (subWords.isEmpty() || subWords.size() > 2)
            {
                AppendWord(word, totalWidth);
            }
            else
            {
                const int width = TextWidth(fontMetrics, subWords.constFirst());
                const int tWidth = !currentLine.isEmpty() ? currentLineWidth + spaceWidth + width : width;
                AppendWord(subWords.constFirst(), tWidth);
                lines.append(currentLine);

                if (subWords.size() == 2)
                {
                    currentLine.clear();
                    currentLineWidth = 0;

                    // Insert the item after the current item
                    iterator.insert(subWords.constLast());
                    iterator.previous();
                }
            }
        }
    }

    // Add the last line
    if (!currentLine.isEmpty())
    {
        lines.append(currentLine);
    }

    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::BreakTextIntoLines(const QString &text, const VSvgFont &font, int maxWidth, qreal penWidth) const
    -> QStringList
{
    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();

    VSvgFontEngine engine = db->FontEngine(font);

    VSvgFont svgFont = engine.Font();
    if (!svgFont.IsValid())
    {
        return {text};
    }

    QStringList words = text.split(' ');

    QString currentLine;
    int currentLineWidth = 0;
    const int spaceWidth = qRound(engine.TextWidth(QChar(' ')));
    const float tolerance = 0.3F;

    QStringList lines;
    lines.reserve(words.size());
    QMutableListIterator<QString> iterator(words);

    auto AppendWord = [&currentLine, &currentLineWidth](const QString &word, int totalWidth)
    {
        if (!currentLine.isEmpty())
        {
            currentLine += ' '_L1;
        }
        currentLine += word;
        currentLineWidth = totalWidth;
    };

    while (iterator.hasNext())
    {
        const QString &word = iterator.next();
        int wordWidth = qRound(engine.TextWidth(word, penWidth));
        int totalWidth = !currentLine.isEmpty() ? currentLineWidth + spaceWidth + wordWidth : wordWidth;

        if (totalWidth <= maxWidth)
        {
            // Append the word to the current line
            AppendWord(word, totalWidth);
        }
        else if ((maxWidth - currentLineWidth) <= qFloor(static_cast<float>(maxWidth) * tolerance) &&
                 maxWidth >= wordWidth)
        {
            // Start a new line with the word if it doesn't exceed the tolerance
            lines.append(currentLine);
            currentLine = word;
            currentLineWidth = wordWidth;
        }
        else
        {
            // Word is too long, force line break
            if (currentLineWidth + spaceWidth + engine.TextWidth(word.at(0), penWidth) > maxWidth)
            {
                lines.append(currentLine);
                currentLine.clear();
                currentLineWidth = 0;
            }

            const int subWordWidth = !currentLine.isEmpty() ? maxWidth - (currentLineWidth + spaceWidth) : maxWidth;
            const QStringList subWords = SplitTextByWidth(word, svgFont, subWordWidth, penWidth);

            if (subWords.isEmpty() || subWords.size() > 2)
            {
                AppendWord(word, totalWidth);
            }
            else
            {
                const int width = qRound(engine.TextWidth(subWords.constFirst(), penWidth));
                const int tWidth = !currentLine.isEmpty() ? currentLineWidth + spaceWidth + width : width;
                AppendWord(subWords.constFirst(), tWidth);
                lines.append(currentLine);

                if (subWords.size() == 2)
                {
                    currentLine.clear();
                    currentLineWidth = 0;

                    // Insert the item after the current item
                    iterator.insert(subWords.constLast());
                    iterator.previous();
                }
            }
        }
    }

    // Add the last line
    if (!currentLine.isEmpty())
    {
        lines.append(currentLine);
    }

    return lines;
}
