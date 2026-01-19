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

#ifndef VTEXTMANAGER_H
#define VTEXTMANAGER_H

#include <QCoreApplication>
#include <QDate>
#include <QFont>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>
#include <utility>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/vcontainer.h"

class VAbstractPattern;
class VSvgFont;
class VMeasurement;
class VTranslator;

/**
 * @brief The TextLine struct holds the information about one text line
 */
struct TextLine
{
    QString qsText{};
    int iFontSize{0}; // 0 means default
    bool bold{false};
    bool italic{false};
    Qt::Alignment eAlign{Qt::AlignCenter};

    friend auto operator<<(QDataStream &dataStream, const TextLine &data) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, TextLine &data) -> QDataStream &;

private:
    static const quint32 streamHeader;
    static const quint16 classVersion;
};

struct VPieceLabelInfo
{
    explicit VPieceLabelInfo(VContainer data)
      : completeData(std::move(data))
    {
    }

    ~VPieceLabelInfo() = default;

    VPieceLabelInfo(const VPieceLabelInfo &) = default;
    VPieceLabelInfo(VPieceLabelInfo &&) noexcept = default;

    auto operator=(const VPieceLabelInfo &) -> VPieceLabelInfo & = default;
    auto operator=(VPieceLabelInfo &&) noexcept -> VPieceLabelInfo & = default;

    QString pieceName{};                                        // NOLINT(misc-non-private-member-variables-in-classes)
    VPieceLabelData labelData{};                                // NOLINT(misc-non-private-member-variables-in-classes)
    QMap<QString, QSharedPointer<VMeasurement>> measurements{}; // NOLINT(misc-non-private-member-variables-in-classes)
    VContainer completeData;                                    // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<VFinalMeasurement> finalMeasurements{};             // NOLINT(misc-non-private-member-variables-in-classes)
    QLocale locale{};                                           // NOLINT(misc-non-private-member-variables-in-classes)
    QString labelDateFormat{};                                  // NOLINT(misc-non-private-member-variables-in-classes)
    QString LabelTimeFormat{};                                  // NOLINT(misc-non-private-member-variables-in-classes)
    QString patternName{};                                      // NOLINT(misc-non-private-member-variables-in-classes)
    QString patternNumber{};                                    // NOLINT(misc-non-private-member-variables-in-classes)
    QString companyName{};                                      // NOLINT(misc-non-private-member-variables-in-classes)
    QString customerName{};                                     // NOLINT(misc-non-private-member-variables-in-classes)
    QString customerEmail{};                                    // NOLINT(misc-non-private-member-variables-in-classes)
    Unit measurementsUnits{Unit::Cm};                           // NOLINT(misc-non-private-member-variables-in-classes)
    Unit dimensionSizeUnits{Unit::Cm};                          // NOLINT(misc-non-private-member-variables-in-classes)
    QString measurementsPath{};                                 // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MeasurementsType measurementsType{MeasurementsType::Individual};
    QSharedPointer<VTranslator> placeholderTranslator{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QDate customerBirthDate{};                           // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionHeight{};                           // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionSize{};                             // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionHip{};                              // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionWaist{};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionHeightLabel{};                      // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionSizeLabel{};                        // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionHipLabel{};                         // NOLINT(misc-non-private-member-variables-in-classes)
    QString dimensionWaistLabel{};                       // NOLINT(misc-non-private-member-variables-in-classes)
    QMap<int, QString> patternMaterials{};               // NOLINT(misc-non-private-member-variables-in-classes)
    QVector<VLabelTemplateLine> patternLabelTemplate{};  // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * @brief The VTextManager class this class is used to determine whether a collection of
 * text lines can fit into specified bounding box and with what font size
 */
class VTextManager
{
    Q_DECLARE_TR_FUNCTIONS(VTextManager) // NOLINT

public:
    DEF_CONSTRUCTOR(VTextManager)
    virtual ~VTextManager() = default;

    VTextManager(const VTextManager &text) = default;
    auto operator=(const VTextManager &text) -> VTextManager & = default;

    VTextManager(VTextManager &&text) noexcept = default;
    auto operator=(VTextManager &&text) noexcept -> VTextManager & = default;

    virtual auto GetSpacing() const -> int;

    void SetFont(const QFont &font);
    auto GetFont() const -> const QFont &;

    void SetSVGFontFamily(const QString &fontFamily);
    auto GetSVGFontFamily() const -> QString;

    void SetSVGFontPointSize(int pointSize);
    auto GetSVGFontPointSize() const -> int;

    void SetFontSize(int iFS);

    auto GetAllSourceLines() const -> QVector<TextLine>;
    void SetAllSourceLines(const QVector<TextLine> &lines);
    auto GetSourceLinesCount() const -> vsizetype;
    auto GetSourceLine(vsizetype i) const -> const TextLine &;

    auto GetLabelSourceLines(int width, const QFont &font) const -> QVector<TextLine>;
    auto GetLabelSourceLines(int width, const VSvgFont &font, qreal penWidth) const -> QVector<TextLine>;

    void UpdatePieceLabelInfo(const VPieceLabelInfo &info);
    void UpdatePatternLabelInfo(const VPieceLabelInfo &info);

    friend auto operator<<(QDataStream &dataStream, const VTextManager &data) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VTextManager &data) -> QDataStream &;

    static auto PrepareLabelInfo(const VAbstractPattern *doc, const VContainer *pattern, bool pieceLabel)
        -> VPieceLabelInfo;

    static auto HorizontalAdvance(const QString &text, const QFont &font) -> int;

private:
    QFont m_font{};
    QString m_svgFontFamily{};
    int m_svgFontPointSize{-1};
    QVector<TextLine> m_liLines{};

    static const quint32 streamHeader;
    static const quint16 classVersion;

    static auto BreakTextIntoLines(const QString &text, const QFont &font, int maxWidth) -> QStringList;
    auto BreakTextIntoLines(const QString &text, const VSvgFont &font, int maxWidth, qreal penWidth) const
        -> QStringList;
};

#endif // VTEXTMANAGER_H
