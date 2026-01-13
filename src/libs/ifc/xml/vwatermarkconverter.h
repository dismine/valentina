/************************************************************************
 **
 **  @file   vwatermarkconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 12, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VWATERMARKCONVERTER_H
#define VWATERMARKCONVERTER_H

#include "vabstractconverter.h"
#include "../vmisc/projectversion.h"

class VWatermarkConverter final : public VAbstractConverter
{
public:
    explicit VWatermarkConverter(const QString &fileName);
    ~VWatermarkConverter() override = default;

    static const QString WatermarkMaxVerStr;
    static const QString CurrentSchema;
    static constexpr const unsigned WatermarkMinVer = FormatVersion(1, 0, 0);
    static constexpr const unsigned WatermarkMaxVer = FormatVersion(1, 1, 0);

    static auto XSDSchemas() -> QHash <unsigned, QString>;

protected:
    auto MinVer() const -> unsigned override;
    auto MaxVer() const -> unsigned override;

    auto MinVerStr() const -> QString override;
    auto MaxVerStr() const -> QString override;

    void ApplyPatches() override;
    void DowngradeToCurrentMaxVersion() override;

    auto IsReadOnly() const -> bool override { return false; }

    auto Schemas() const -> QHash <unsigned, QString> override;

private:
    Q_DISABLE_COPY_MOVE(VWatermarkConverter) // NOLINT
    static const QString WatermarkMinVerStr;

    void ToV1_1_0();
};

#endif // VWATERMARKCONVERTER_H
