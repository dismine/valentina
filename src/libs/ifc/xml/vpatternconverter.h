/************************************************************************
 **
 **  @file   vpatternconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 12, 2014
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

#ifndef VPATTERNCONVERTER_H
#define VPATTERNCONVERTER_H

#include <QCoreApplication>
#include <QDomElement>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/projectversion.h"
#include "vabstractconverter.h"

class QDomElement;

class VPatternConverter final : public VAbstractConverter
{
    Q_DECLARE_TR_FUNCTIONS(VPatternConverter) // NOLINT

public:
    explicit VPatternConverter(const QString &fileName);
    virtual ~VPatternConverter() = default;

    static const QString PatternMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const unsigned PatternMinVer = FormatVersion(0, 1, 4);
    static Q_DECL_CONSTEXPR const unsigned PatternMaxVer = FormatVersion(0, 9, 3);

    static auto XSDSchemas() -> QHash<unsigned, QString>;

protected:
    void Save() override;

    virtual auto MinVer() const -> unsigned override;
    virtual auto MaxVer() const -> unsigned override;

    virtual auto MinVerStr() const -> QString override;
    virtual auto MaxVerStr() const -> QString override;

    virtual void ApplyPatches() override;
    virtual void DowngradeToCurrentMaxVersion() override;

    virtual auto IsReadOnly() const -> bool override;

    auto Schemas() const -> QHash<unsigned, QString> override;

private:
    Q_DISABLE_COPY_MOVE(VPatternConverter) // NOLINT
    static const QString PatternMinVerStr;

    void ToV0_2_0();
    void ToV0_2_1();
    void ToV0_2_4();
    void ToV0_3_0();
    void ToV0_3_1();
    void ToV0_4_0();
    void ToV0_4_4();
    void ToV0_6_0();
    void ToV0_6_2();
    void ToV0_8_8();
    void ToV0_9_0();
    void ToV0_9_1();
    void ToV0_9_2();
    void ToV0_9_3();

    void TagUnitToV0_2_0();
    void TagIncrementToV0_2_0();
    void ConvertMeasurementsToV0_2_0();
    void TagMeasurementsToV0_2_0();

    void ConvertMeasurementsToV0_2_1();

    void RemoveColorToolCutV0_3_1();

    auto FixIncrementsToV0_2_0() -> QSet<QString>;
    auto FixIncrementInFormulaToV0_2_0(const QString &formula, const QSet<QString> &names) -> QString;
    void FixPointExpressionsToV0_2_0(const QSet<QString> &names);
    void FixArcExpressionsToV0_2_0(const QSet<QString> &names);
    void FixPathPointExpressionsToV0_2_0(const QSet<QString> &names);

    void ConvertPointExpressionsToV0_2_0(const QMap<QString, QString> &names);
    void ConvertArcExpressionsToV0_2_0(const QMap<QString, QString> &names);
    void ConvertPathPointExpressionsToV0_2_0(const QMap<QString, QString> &names);
    auto FixMeasurementInFormulaToV0_2_0(const QString &formula, const QMap<QString, QString> &names) -> QString;

    auto MUnitV0_1_4() const -> QString;
    auto TagMeasurementsV0_1_4() const -> QDomElement;
    auto TagIncrementsV0_1_4() const -> QDomElement;

    void FixToolUnionToV0_2_4();
    void ParseModelingToV0_2_4(const QDomElement &modeling);
    void SaveChildrenToolUnionToV0_2_4(quint32 id, const QVector<quint32> &children);

    static auto OldNamesToNewNames_InV0_2_0() -> QMap<QString, QString>;
    static auto OldNamesToNewNames_InV0_2_1() -> QMap<QString, QString>;

    void FixCutPoint();
    void FixSubPaths(int i, quint32 id, quint32 baseCurve);

    void TagRemoveAttributeTypeObjectInV0_4_0();
    void TagDetailToV0_4_0();
    void TagUnionDetailsToV0_4_0();
    auto GetUnionDetailNodesV0_4_0(const QDomElement &detail) -> QDomElement;
    auto GetUnionChildrenNodesV0_4_0(const QDomElement &detail) -> QDomElement;

    void LabelTagToV0_4_4(const QString &tagName);

    auto AddTagPatternLabelV0_5_1() -> QDomElement;
    void PortPatternLabeltoV0_6_0(QDomElement &label);
    void AddLabelTemplateLineV0_6_0(QDomElement &label, const QString &text, bool bold, bool italic, int alignment,
                                    int fontSizeIncrement);
    void PortPieceLabelstoV0_6_0();
    void RemoveUnusedTagsV0_6_0();
    void RemoveUniqueTagV0_6_0(const QString &tag);

    void AddTagPreviewCalculationsV0_6_2();

    void RemoveGradationV0_8_8();
    void AddPieceUUIDV0_8_8();

    void ConvertImageToV0_9_0();

    void ConvertMeasurementsPathToV0_9_1();

    void ConvertPathAttributesToV0_9_2();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternConverter::MinVer() const -> unsigned
{
    return PatternMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternConverter::MaxVer() const -> unsigned
{
    return PatternMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternConverter::MinVerStr() const -> QString
{
    return PatternMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternConverter::MaxVerStr() const -> QString
{
    return PatternMaxVerStr;
}

#endif // VPATTERNCONVERTER_H
