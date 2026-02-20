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
#include <QLatin1String>
#include <QRegularExpression>
#include <QtMath>

#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/svgfont/vsvgfont.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vtranslator.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/variables/vpiecearea.h"
#include "vtextmanager.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FileBaseName(const QString &filePath) -> QString
{
    // Known suffixes to check for
    QStringList const knownSuffixes = {".val", ".vst", ".vit"};

    QFileInfo const fileInfo(filePath);

    // Check if the file has one of the known suffixes
    for (const QString &suffix : knownSuffixes)
    {
        if (fileInfo.completeSuffix().endsWith(suffix, Qt::CaseInsensitive))
        {
            // Remove the known suffix and return the modified file name
            QString modifiedFileName = fileInfo.fileName();
            modifiedFileName.chop(suffix.length());
            return modifiedFileName;
        }
    }

    // Fallback to QFileInfo::baseName if no known suffix is found
    return fileInfo.baseName();
}

//---------------------------------------------------------------------------------------------------------------------
auto SplitTextByWidth(const QString &text, const QFont &font, int maxWidth) -> QStringList
{
    QFontMetrics const fontMetrics(font);
    if (fontMetrics.horizontalAdvance(text) <= maxWidth)
    {
        return {text};
    }

    QStringList substrings;
    substrings.reserve(2);

    const auto textLength = static_cast<int>(text.length());
    int lineWidth = 0;

    for (int endIndex = 0; endIndex < textLength; ++endIndex)
    {
        QChar const currentChar = text.at(endIndex);
        const int charWidth = fontMetrics.horizontalAdvance(currentChar);

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
    VSvgFontEngine const engine = db->FontEngine(font);

    if (engine.TextWidth(text, penWidth) <= maxWidth)
    {
        return {text};
    }

    QStringList substrings;
    substrings.reserve(2);

    const auto textLength = static_cast<int>(text.length());
    qreal lineWidth = 0;

    for (int endIndex = 0; endIndex < textLength; ++endIndex)
    {
        QChar const currentChar = text.at(endIndex);
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

//---------------------------------------------------------------------------------------------------------------------
auto LinePlaceholders(const QString &line) -> QSet<QString>
{
    QSet<QString> placeholders;

    static const QRegularExpression pattern(QStringLiteral("(%[^%]+%)"));

    QRegularExpressionMatchIterator matches = pattern.globalMatch(line);
    while (matches.hasNext())
    {
        QRegularExpressionMatch const match = matches.next();
        placeholders.insert(match.captured(1));
    }

    return placeholders;
}

//---------------------------------------------------------------------------------------------------------------------
auto UniquePlaceholders(const QVector<VLabelTemplateLine> &lines) -> QSet<QString>
{
    QSet<QString> placeholders;

    for (const auto &line : lines)
    {
        placeholders |= LinePlaceholders(line.line);
    }

    return placeholders;
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
    dataStream << data.qsText;
    dataStream << data.iFontSize;
    dataStream << data.bold;
    dataStream << data.italic;
    dataStream << data.eAlign;

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
        QString const message = QCoreApplication::tr("TextLine prefix mismatch error: actualStreamHeader = 0x%1 and "
                                                     "streamHeader = 0x%2")
                                    .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                                    .arg(TextLine::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > TextLine::classVersion)
    {
        QString const message = QCoreApplication::tr("TextLine compatibility error: actualClassVersion = %1 and "
                                                     "classVersion = %2")
                                    .arg(actualClassVersion)
                                    .arg(TextLine::classVersion);
        throw VException(message);
    }

    dataStream >> data.qsText;
    dataStream >> data.iFontSize;
    dataStream >> data.bold;
    dataStream >> data.italic;
    dataStream >> data.eAlign;

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
        QString const message =
            QCoreApplication::tr("VTextManager prefix mismatch error: actualStreamHeader = 0x%1 and "
                                 "streamHeader = 0x%2")
                .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                .arg(VTextManager::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VTextManager::classVersion)
    {
        QString const message = QCoreApplication::tr("VTextManager compatibility error: actualClassVersion = %1 and "
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
//---------------------------------------------------------------------------------------------------------------------
void PrepareMeasurementsPlaceholders(const VPieceLabelInfo &info, QMap<QString, QString> &placeholders,
                                     const QSet<QString> &uniquePlaceholders)
{
    auto AddPlaceholder = [&placeholders, uniquePlaceholders](const QString &name, const QString &value)
    {
        if (uniquePlaceholders.contains('%' + name + '%'))
        {
            placeholders.insert(name, value);
        }
    };

    const QMap<QString, QSharedPointer<VMeasurement>> measurements = info.measurements;
    auto i = measurements.constBegin();
    while (i != measurements.constEnd())
    {
        AddPlaceholder(pl_measurement + i.key(), QString::number(*i.value()->GetValue()));
        AddPlaceholder(pl_measurement + i.key() + pl_valueAlias, i.value()->GetValueAlias());
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareDimensionPlaceholders(const VPieceLabelInfo &info, QMap<QString, QString> &placeholders,
                                  const QSet<QString> &uniquePlaceholders)
{
    auto AddPlaceholder = [&placeholders, uniquePlaceholders](const QString &name, const QString &value)
    {
        if (uniquePlaceholders.contains('%' + name + '%'))
        {
            placeholders.insert(name, value);
        }
    };

    AddPlaceholder(pl_height, info.dimensionHeight);
    AddPlaceholder(pl_dimensionX, info.dimensionHeight);

    AddPlaceholder(pl_size, info.dimensionSize);
    AddPlaceholder(pl_dimensionY, info.dimensionSize);

    AddPlaceholder(pl_hip, info.dimensionHip);
    AddPlaceholder(pl_dimensionZ, info.dimensionHip);

    AddPlaceholder(pl_waist, info.dimensionWaist);
    AddPlaceholder(pl_dimensionW, info.dimensionWaist);

    AddPlaceholder(pl_heightLabel,
                   not info.dimensionHeightLabel.isEmpty() ? info.dimensionHeightLabel : info.dimensionHeight);
    AddPlaceholder(pl_dimensionXLabel,
                   not info.dimensionHeightLabel.isEmpty() ? info.dimensionHeightLabel : info.dimensionHeight);
    AddPlaceholder(pl_sizeLabel, not info.dimensionSizeLabel.isEmpty() ? info.dimensionSizeLabel : info.dimensionSize);
    AddPlaceholder(pl_dimensionYLabel,
                   not info.dimensionSizeLabel.isEmpty() ? info.dimensionSizeLabel : info.dimensionSize);
    AddPlaceholder(pl_hipLabel, not info.dimensionHipLabel.isEmpty() ? info.dimensionHipLabel : info.dimensionHip);
    AddPlaceholder(pl_dimensionZLabel,
                   not info.dimensionHipLabel.isEmpty() ? info.dimensionHipLabel : info.dimensionHip);
    AddPlaceholder(pl_waistLabel,
                   not info.dimensionWaistLabel.isEmpty() ? info.dimensionWaistLabel : info.dimensionWaist);
    AddPlaceholder(pl_dimensionWLabel,
                   not info.dimensionWaistLabel.isEmpty() ? info.dimensionWaistLabel : info.dimensionWaist);
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareUserMaterialsPlaceholders(const VPieceLabelInfo &info, QMap<QString, QString> &placeholders,
                                      const QSet<QString> &uniquePlaceholders)
{
    auto AddPlaceholder = [&placeholders, uniquePlaceholders](const QString &name, const QString &value)
    {
        if (uniquePlaceholders.contains('%' + name + '%'))
        {
            placeholders.insert(name, value);
        }
    };

    for (int i = 0; i < userMaterialPlaceholdersQuantity; ++i)
    {
        const QString number = QString::number(i + 1);

        QString value;
        if (info.patternMaterials.contains(i + 1))
        {
            value = info.patternMaterials.value(i + 1);
        }

        AddPlaceholder(pl_userMaterial + number, value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AddMeasurementTokensToUniqueNames(const VFinalMeasurement &m, QSet<QString> &uniquePieceNames)
{
    QMap<vsizetype, QString> tokens;

    try
    {
        QScopedPointer<qmu::QmuTokenParser> const cal(new qmu::QmuTokenParser(m.formula, false, false)); // Eval formula
        tokens = cal->GetTokens(); // Tokens (variables, measurements)
    }
    catch (qmu::QmuParserError &)
    {
        // Skip formula errors
    }

    QMapIterator i(tokens);
    while (i.hasNext())
    {
        i.next();
        if (i.value().startsWith(pieceArea_))
        {
            QString name = i.value();
            uniquePieceNames.insert(name.remove(0, pieceArea_.length()));
        }
        else if (i.value().startsWith(pieceSeamLineArea_))
        {
            QString name = i.value();
            uniquePieceNames.insert(name.remove(0, pieceSeamLineArea_.length()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ExtractUniquePieceNames(const VPieceLabelInfo &info, const QSet<QString> &uniquePlaceholders) -> QSet<QString>
{
    QSet<QString> uniquePieceNames;

    if (uniquePlaceholders.contains('%' + pieceArea_ + info.labelData.GetAreaShortName() + '%') ||
        uniquePlaceholders.contains('%' + pieceSeamLineArea_ + info.labelData.GetAreaShortName() + '%'))
    {
        uniquePieceNames.insert(info.labelData.GetAreaShortName());
    }

    const QVector<VFinalMeasurement> measurements = info.finalMeasurements;
    for (const auto &m : measurements)
    {
        if (uniquePlaceholders.contains('%' + pl_finalMeasurement + m.name + '%'))
        {
            AddMeasurementTokensToUniqueNames(m, uniquePieceNames);
        }
    }

    return uniquePieceNames;
}

//---------------------------------------------------------------------------------------------------------------------
void PopulatePieceData(VContainer &completeData, const QSet<QString> &uniquePieceNames)
{
    const QHash<quint32, VPiece> *pieces = completeData.DataPieces();
    Unit const patternUnits = VAbstractValApplication::VApp()->patternUnits();
    QSet<QString> processedNames;

    auto i = pieces->constBegin();
    while (i != pieces->constEnd())
    {
        if (uniquePieceNames.contains(i.value().GetShortName()) && !processedNames.contains(i.value().GetShortName()))
        {
            completeData.AddVariable(QSharedPointer<VPieceArea>::create(PieceAreaType::External, i.key(), i.value(),
                                                                        &completeData, patternUnits));
            completeData.AddVariable(QSharedPointer<VPieceArea>::create(PieceAreaType::SeamLine, i.key(), i.value(),
                                                                        &completeData, patternUnits));
            processedNames.insert(i.value().GetShortName());
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InsertPlaceholderValue(const VContainer &completeData, const QString &formula,
                            QMap<QString, QString> &placeholders, const QString &placeholderKey,
                            const QString &errorMsg)
{
    QScopedPointer<Calculator> const cal(new Calculator());

    try
    {
        const qreal result = cal->EvalFormula(completeData.DataVariables(), formula);
        placeholders.insert(placeholderKey, QString::number(result));
    }
    catch (qmu::QmuParserError &e)
    {
        const QString error = QStringLiteral("%1 %2.").arg(errorMsg).arg(e.GetMsg());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(error)
            : qWarning() << VAbstractValApplication::warningMessageSignature + error;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreparePieceLabelPlaceholders(const VContainer &completeData, const VPieceLabelInfo &info,
                                   QMap<QString, QString> &placeholders, const QSet<QString> &uniquePlaceholders)
{
    if (uniquePlaceholders.contains('%' + pl_currentArea + '%'))
    {
        InsertPlaceholderValue(completeData, pieceArea_ + info.labelData.GetAreaShortName(), placeholders,
                               pl_currentArea, "Failed to prepare full piece area placeholder.");
    }

    if (uniquePlaceholders.contains('%' + pl_currentSeamLineArea + '%'))
    {
        InsertPlaceholderValue(completeData, pieceSeamLineArea_ + info.labelData.GetAreaShortName(), placeholders,
                               pl_currentSeamLineArea, "Failed to prepare piece seam line area placeholder.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SetEmptyPieceLabelPlaceholders(QMap<QString, QString> &placeholders, const QSet<QString> &uniquePlaceholders)
{
    if (uniquePlaceholders.contains('%' + pl_currentArea + '%'))
    {
        placeholders.insert(pl_currentArea, QString());
    }

    if (uniquePlaceholders.contains('%' + pl_currentSeamLineArea + '%'))
    {
        placeholders.insert(pl_currentSeamLineArea, QString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InsertMeasurementPlaceholder(const VContainer &completeData, const VFinalMeasurement &m, int index,
                                  QMap<QString, QString> &placeholders)
{
    try
    {
        QScopedPointer<Calculator> const cal(new Calculator());
        const qreal result = cal->EvalFormula(completeData.DataVariables(), m.formula);

        placeholders.insert(pl_finalMeasurement + m.name, QString::number(result));
    }
    catch (qmu::QmuParserError &e)
    {
        const QString errorMsg =
            QObject::tr("Failed to prepare final measurement placeholder. Parser error at line %1: %2.")
                .arg(index + 1)
                .arg(e.GetMsg());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareFinalMeasurementPlaceholders(const VPieceLabelInfo &info, const VContainer &completeData,
                                         QMap<QString, QString> &placeholders, const QSet<QString> &uniquePlaceholders)
{
    for (int i = 0; i < info.finalMeasurements.size(); ++i)
    {
        const VFinalMeasurement &m = info.finalMeasurements.at(i);

        if (uniquePlaceholders.contains('%' + pl_finalMeasurement + m.name + '%'))
        {
            InsertMeasurementPlaceholder(completeData, m, i, placeholders);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PrepareFinalMeasurementsPlaceholders(bool pieceLabel, const VPieceLabelInfo &info,
                                          QMap<QString, QString> &placeholders, const QSet<QString> &uniquePlaceholders)
{
    QSet<QString> uniquePieceNames = ExtractUniquePieceNames(info, uniquePlaceholders);

    VContainer completeData = info.completeData;
    PopulatePieceData(completeData, uniquePieceNames);

    if (pieceLabel)
    {
        PreparePieceLabelPlaceholders(completeData, info, placeholders, uniquePlaceholders);
    }
    else
    {
        SetEmptyPieceLabelPlaceholders(placeholders, uniquePlaceholders);
    }

    PrepareFinalMeasurementPlaceholders(info, completeData, placeholders, uniquePlaceholders);
}

//---------------------------------------------------------------------------------------------------------------------
auto PreparePlaceholders(const VPieceLabelInfo &info, const QSet<QString> &uniquePlaceholders, bool pieceLabel = false)
    -> QMap<QString, QString>
{
    QMap<QString, QString> placeholders;

    auto AddPlaceholder = [&placeholders, uniquePlaceholders](const QString &name, const QString &value)
    {
        if (uniquePlaceholders.contains('%' + name + '%'))
        {
            placeholders.insert(name, value);
        }
    };

    // Pattern tags
    AddPlaceholder(pl_date, info.locale.toString(QDate::currentDate(), info.labelDateFormat));
    AddPlaceholder(pl_time, info.locale.toString(QTime::currentTime(), info.LabelTimeFormat));
    AddPlaceholder(pl_patternName, info.patternName);
    AddPlaceholder(pl_patternNumber, info.patternNumber);
    AddPlaceholder(pl_author, info.companyName);

    AddPlaceholder(pl_mUnits, UnitsToStr(info.measurementsUnits, true));
    const QString pUnits = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    AddPlaceholder(pl_pUnits, pUnits);
    AddPlaceholder(pl_mSizeUnits, UnitsToStr(info.dimensionSizeUnits, true));
    AddPlaceholder(pl_areaUnits, pUnits + QStringLiteral("²"));

    AddPlaceholder(pl_customer, info.customerName);
    AddPlaceholder(pl_birthDate, info.locale.toString(info.customerBirthDate, info.labelDateFormat));
    AddPlaceholder(pl_email, info.customerEmail);

    AddPlaceholder(pl_pExt, QStringLiteral("val"));
    AddPlaceholder(pl_pFileName, FileBaseName(VAbstractValApplication::VApp()->GetPatternPath()));
    AddPlaceholder(pl_mFileName, FileBaseName(info.measurementsPath));

    PrepareDimensionPlaceholders(info, placeholders, uniquePlaceholders);

    AddPlaceholder(pl_mExt, info.measurementsType == MeasurementsType::Multisize ? QStringLiteral("vst")
                                                                                 : QStringLiteral("vit"));

    PrepareUserMaterialsPlaceholders(info, placeholders, uniquePlaceholders);
    PrepareMeasurementsPlaceholders(info, placeholders, uniquePlaceholders);
    PrepareFinalMeasurementsPlaceholders(pieceLabel, info, placeholders, uniquePlaceholders);

    // Piece tags
    if (QSharedPointer<VTranslator> const phTr = info.placeholderTranslator; !phTr.isNull())
    {
        AddPlaceholder(pl_mFabric, phTr->translate("Placeholder", "Fabric"));
        AddPlaceholder(pl_mLining, phTr->translate("Placeholder", "Lining"));
        AddPlaceholder(pl_mInterfacing, phTr->translate("Placeholder", "Interfacing"));
        AddPlaceholder(pl_mInterlining, phTr->translate("Placeholder", "Interlining"));
        AddPlaceholder(pl_wCut, phTr->translate("Placeholder", "Cut"));
    }

    return placeholders;
}

//---------------------------------------------------------------------------------------------------------------------
void InitPiecePlaceholders(QMap<QString, QString> &placeholders, const VPieceLabelInfo &info,
                           const QSet<QString> &uniquePlaceholders)
{
    const VPieceLabelData data = info.labelData;

    auto AddPlaceholder = [&placeholders, uniquePlaceholders](const QString &name, const QString &value)
    {
        if (uniquePlaceholders.contains('%' + name + '%'))
        {
            placeholders.insert(name, value);
        }
    };

    AddPlaceholder(pl_pLetter, data.GetLetter());
    AddPlaceholder(pl_pAnnotation, data.GetAnnotation());
    AddPlaceholder(pl_pOrientation, data.GetOrientation());
    AddPlaceholder(pl_pRotation, data.GetRotationWay());
    AddPlaceholder(pl_pTilt, data.GetTilt());
    AddPlaceholder(pl_pFoldPosition, data.GetFoldPosition());
    AddPlaceholder(pl_pName, info.pieceName);
    AddPlaceholder(pl_pQuantity, QString::number(data.GetQuantity()));

    if (uniquePlaceholders.contains('%' + pl_wOnFold + '%'))
    {
        if (data.IsOnFold())
        {
            if (QSharedPointer<VTranslator> const phTr = info.placeholderTranslator; !phTr.isNull())
            {
                placeholders.insert(pl_wOnFold, phTr->translate("Placeholder", "on fold"));
            }
        }
        else
        {
            placeholders.insert(pl_wOnFold, QString());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplacePlaceholders(const QMap<QString, QString> &placeholders, QString line) -> QString
{
    QChar const per('%');

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
    textLines.reserve(lines.size());

    for (const auto &line : lines)
    {
        if (not line.line.isEmpty())
        {
            textLines.append({.qsText = line.line,
                              .iFontSize = line.fontSizeIncrement,
                              .bold = line.bold,
                              .italic = line.italic,
                              .eAlign = static_cast<Qt::Alignment>(line.alignment)});
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
    int const fSize = m_font.pointSize();

    for (const auto &tl : m_liLines)
    {
        fnt.setPointSize(qMax(fSize + tl.iFontSize, 1));
        if (!VAbstractApplication::VApp()->Settings()->GetSingleStrokeOutlineFont())
        {
            fnt.setBold(tl.bold);
        }
        fnt.setItalic(tl.italic);

        QString const qsText = tl.qsText;
        if (HorizontalAdvance(qsText, fnt) > width)
        {
            const QStringList brokeLines = BreakTextIntoLines(qsText, fnt, width);
            for (const auto &lineText : brokeLines)
            {
                TextLine line = tl;
                line.qsText = lineText;
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
    int const fSize = m_font.pointSize();

    for (const auto &tl : m_liLines)
    {
        VSvgFont lineFont = font;
        lineFont.SetPointSize(fSize + tl.iFontSize);
        lineFont.SetBold(tl.bold);
        lineFont.SetItalic(tl.italic);

        VSvgFontEngine const engine = db->FontEngine(lineFont);

        VSvgFont const svgFont = engine.Font();
        if (!svgFont.IsValid())
        {
            lines.append(tl);
            continue;
        }

        QString const qsText = tl.qsText;
        if (engine.TextWidth(qsText, penWidth) > width)
        {
            const QStringList brokeLines = BreakTextIntoLines(qsText, svgFont, width, penWidth);
            for (const auto &lineText : brokeLines)
            {
                TextLine line = tl;
                line.qsText = lineText;
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
/**
 * @brief VTextManager::Update updates the text lines with detail data
 */
void VTextManager::UpdatePieceLabelInfo(const VPieceLabelInfo &info)
{
    m_liLines.clear();

    QSet<QString> const uniquePlaceholders = UniquePlaceholders(info.labelData.GetLabelTemplate());

    QMap<QString, QString> placeholders = PreparePlaceholders(info, uniquePlaceholders, true);
    InitPiecePlaceholders(placeholders, info, uniquePlaceholders);

    QVector<VLabelTemplateLine> lines = info.labelData.GetLabelTemplate();

    for (auto &line : lines)
    {
        line.line = ReplacePlaceholders(placeholders, line.line);
    }

    m_liLines = PrepareLines(lines);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextManager::Update updates the text lines with pattern info
 */
void VTextManager::UpdatePatternLabelInfo(const VPieceLabelInfo &info)
{
    m_liLines.clear();

    QVector<VLabelTemplateLine> lines = info.patternLabelTemplate;
    if (lines.isEmpty())
    {
        return; // Nothing to parse
    }

    QSet<QString> const uniquePlaceholders = UniquePlaceholders(lines);

    QMap<QString, QString> placeholders = PreparePlaceholders(info, uniquePlaceholders);

    if (QSharedPointer<VTranslator> const phTr = info.placeholderTranslator; !phTr.isNull())
    {
        // These placeholders must be available only in piece label
        const QString errorValue = '<' + phTr->translate("Placeholder", "Error") + '>';
        auto AddPlaceholder = [&placeholders, uniquePlaceholders, errorValue](const QString &name, const QString &value)
        {
            const QString placeholder = '%' + name + '%';
            if (uniquePlaceholders.contains(placeholder))
            {
                const QString errorMsg =
                    QObject::tr("Incorrect use of placeholder %1. This placeholder is not available in pattern label.")
                        .arg(placeholder);
                VAbstractApplication::VApp()->IsPedantic()
                    ? throw VException(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;

                placeholders.insert(name, value);
            }
        };

        AddPlaceholder(pl_pLetter, errorValue);
        AddPlaceholder(pl_pAnnotation, errorValue);
        AddPlaceholder(pl_pOrientation, errorValue);
        AddPlaceholder(pl_pRotation, errorValue);
        AddPlaceholder(pl_pTilt, errorValue);
        AddPlaceholder(pl_pFoldPosition, errorValue);
        AddPlaceholder(pl_pName, errorValue);
        AddPlaceholder(pl_pQuantity, errorValue);
        AddPlaceholder(pl_wOnFold, errorValue);
    }

    for (auto &line : lines)
    {
        line.line = ReplacePlaceholders(placeholders, line.line);
    }

    m_liLines = PrepareLines(lines);
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::BreakTextIntoLines(const QString &text, const QFont &font, int maxWidth) -> QStringList
{
    QStringList words = text.split(' ');

    QString currentLine;
    int currentLineWidth = 0;
    const int spaceWidth = HorizontalAdvance(QChar(' '), font);
    const float tolerance = 0.3F;

    QStringList lines;
    lines.reserve(words.size());
    QMutableListIterator iterator(words);

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
        int const wordWidth = HorizontalAdvance(word, font);
        int const totalWidth = !currentLine.isEmpty() ? currentLineWidth + spaceWidth + wordWidth : wordWidth;

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
            if (currentLineWidth + spaceWidth + HorizontalAdvance(word.at(0), font) > maxWidth)
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
                const int width = HorizontalAdvance(subWords.constFirst(), font);
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

    VSvgFontEngine const engine = db->FontEngine(font);

    VSvgFont const svgFont = engine.Font();
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
    QMutableListIterator iterator(words);

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
        int const wordWidth = qRound(engine.TextWidth(word, penWidth));
        int const totalWidth = !currentLine.isEmpty() ? currentLineWidth + spaceWidth + wordWidth : wordWidth;

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

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::PrepareLabelInfo(const VAbstractPattern *doc, const VContainer *pattern, bool pieceLabel)
    -> VPieceLabelInfo
{
    VPieceLabelInfo info(doc->GetCompleteData());
    info.measurements = pattern->DataMeasurements();
    info.finalMeasurements = doc->GetFinalMeasurements();
    info.locale = QLocale(VAbstractApplication::VApp()->Settings()->GetLocale());
    info.labelDateFormat = doc->GetLabelDateFormat();
    info.LabelTimeFormat = doc->GetLabelTimeFormat();
    info.patternName = doc->GetPatternName();
    info.patternNumber = doc->GetPatternNumber();
    info.companyName = doc->GetCompanyName();
    info.measurementsUnits = VAbstractValApplication::VApp()->MeasurementsUnits();
    info.dimensionSizeUnits = VAbstractValApplication::VApp()->DimensionSizeUnits();
    info.measurementsPath = doc->MPath();
    info.placeholderTranslator = VAbstractApplication::VApp()->GetPlaceholderTranslator();
    info.measurementsType = VAbstractValApplication::VApp()->GetMeasurementsType();

    if (info.measurementsType == MeasurementsType::Individual)
    {
        info.customerName = VAbstractValApplication::VApp()->GetCustomerName();
        info.customerBirthDate = VAbstractValApplication::VApp()->GetCustomerBirthDate();
        info.customerEmail = VAbstractValApplication::VApp()->CustomerEmail();
    }
    else
    {
        info.customerName = doc->GetCustomerName();
        info.customerBirthDate = doc->GetCustomerBirthDate();
        info.customerEmail = doc->GetCustomerEmail();
    }

    info.dimensionHeight = QString::number(VAbstractValApplication::VApp()->GetDimensionHeight());
    info.dimensionSize = QString::number(VAbstractValApplication::VApp()->GetDimensionSize());
    info.dimensionHip = QString::number(VAbstractValApplication::VApp()->GetDimensionHip());
    info.dimensionWaist = QString::number(VAbstractValApplication::VApp()->GetDimensionWaist());
    info.dimensionHeightLabel = VAbstractValApplication::VApp()->GetDimensionHeightLabel();
    info.dimensionSizeLabel = VAbstractValApplication::VApp()->GetDimensionSizeLabel();
    info.dimensionHipLabel = VAbstractValApplication::VApp()->GetDimensionHipLabel();
    info.dimensionWaistLabel = VAbstractValApplication::VApp()->GetDimensionWaistLabel();
    info.patternMaterials = doc->GetPatternMaterials();
    if (!pieceLabel)
    {
        info.patternLabelTemplate = doc->GetPatternLabelTemplate();
    }

    return info;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextManager::HorizontalAdvance(const QString &text, const QFont &font) -> int
{
    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    qreal const penWidth = VAbstractApplication::VApp()->Settings()->WidthHairLine();

    QFontMetrics const fm(font);
    if (settings->GetSingleStrokeOutlineFont())
    {
        int w = 0;
        for (auto c : std::as_const(text))
        {
            w += fm.horizontalAdvance(c) + qRound(penWidth / 2.0);
        }

        return w;
    }
    return fm.horizontalAdvance(text);
}
