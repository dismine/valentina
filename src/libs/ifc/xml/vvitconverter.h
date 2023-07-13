/************************************************************************
 **
 **  @file   vvitconverter.h
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

#ifndef VVITCONVERTER_H
#define VVITCONVERTER_H


#include <QCoreApplication>
#include <QString>
#include <QtGlobal>

#include "vabstractmconverter.h"
#include "../vmisc/projectversion.h"

class QDomElement;

class VVITConverter final : public VAbstractMConverter
{
    Q_DECLARE_TR_FUNCTIONS(VVITConverter) // NOLINT
public:
    explicit VVITConverter(const QString &fileName);
    virtual ~VVITConverter() = default;

    static const QString MeasurementMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const unsigned MeasurementMinVer = FormatVersion(0, 2, 0);
    static Q_DECL_CONSTEXPR const unsigned MeasurementMaxVer = FormatVersion(0, 5, 2);

    static auto XSDSchemas() -> QHash <unsigned, QString>;

protected:
    virtual auto MinVer() const -> unsigned override;
    virtual auto MaxVer() const -> unsigned override;

    virtual auto MinVerStr() const -> QString override;
    virtual auto MaxVerStr() const -> QString override;

    virtual void ApplyPatches() override;
    virtual void DowngradeToCurrentMaxVersion() override;
    virtual auto IsReadOnly() const -> bool override;

    auto Schemas() const -> QHash <unsigned, QString> override;

private:
    Q_DISABLE_COPY_MOVE(VVITConverter) // NOLINT
    static const QString MeasurementMinVerStr;

    void AddNewTagsForV0_3_0();
    auto MUnitV0_2_0() -> QString;
    void ConvertMeasurementsToV0_3_0();
    auto AddMV0_3_0(const QString &name, qreal value) -> QDomElement;
    void GenderV0_3_1();
    void PM_SystemV0_3_2();
    void ConvertMeasurementsToV0_3_3();
    void ConverCustomerNameToV0_4_0();

    void ToV0_3_0();
    void ToV0_3_1();
    void ToV0_3_2();
    void ToV0_3_3();
    void ToV0_4_0();
    void ToV0_5_2();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VVITConverter::MinVer() const -> unsigned
{
    return MeasurementMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VVITConverter::MaxVer() const -> unsigned
{
    return MeasurementMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VVITConverter::MinVerStr() const -> QString
{
    return MeasurementMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VVITConverter::MaxVerStr() const -> QString
{
    return MeasurementMaxVerStr;
}

#endif // VVITCONVERTER_H
