/************************************************************************
 **
 **  @file   vmeasurementconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VMEASUREMENTCONVERTER_H
#define VMEASUREMENTCONVERTER_H

#include <QCoreApplication>
#include <QString>
#include <QtGlobal>

#include "../vmisc/projectversion.h"
#include "vabstractmconverter.h"

class QDomElement;

class VVSTConverter final : public VAbstractMConverter
{
    Q_DECLARE_TR_FUNCTIONS(VVSTConverter) // NOLINT

public:
    explicit VVSTConverter(const QString &fileName);
    virtual ~VVSTConverter() = default;

    static const QString MeasurementMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const unsigned MeasurementMinVer = FormatVersion(0, 3, 0);
    static Q_DECL_CONSTEXPR const unsigned MeasurementMaxVer = FormatVersion(0, 6, 0);

    static auto XSDSchemas() -> QHash<unsigned, QString>;

protected:
    virtual auto MinVer() const -> unsigned override;
    virtual auto MaxVer() const -> unsigned override;

    virtual auto MinVerStr() const -> QString override;
    virtual auto MaxVerStr() const -> QString override;

    virtual void ApplyPatches() override;
    virtual void DowngradeToCurrentMaxVersion() override;
    virtual auto IsReadOnly() const -> bool override;

    auto Schemas() const -> QHash<unsigned, QString> override;

private:
    Q_DISABLE_COPY_MOVE(VVSTConverter) // NOLINT
    static const QString MeasurementMinVerStr;

    void AddNewTagsForV0_4_0();
    void RemoveTagsForV0_4_0();
    void ConvertMeasurementsToV0_4_0();
    auto AddMV0_4_0(const QString &name, qreal value, qreal sizeIncrease, qreal heightIncrease) -> QDomElement;
    void PM_SystemV0_4_1();
    void ConvertMeasurementsToV0_4_2();

    void AddNewTagsForV0_5_0();
    void RemoveTagsForV0_5_0();
    void ConvertMeasurementsToV0_5_0();

    void ConvertCircumferenceAttreibuteToV0_5_4();

    void ToV0_4_0();
    void ToV0_4_1();
    void ToV0_4_2();
    void ToV0_5_0();
    void ToV0_5_4();
    void ToV0_6_0();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VVSTConverter::MinVer() const -> unsigned
{
    return MeasurementMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VVSTConverter::MaxVer() const -> unsigned
{
    return MeasurementMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VVSTConverter::MinVerStr() const -> QString
{
    return MeasurementMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VVSTConverter::MaxVerStr() const -> QString
{
    return MeasurementMaxVerStr;
}

#endif // VMEASUREMENTCONVERTER_H
