/************************************************************************
 **
 **  @file   vknownmeasurementsconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 10, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VKNOWNMEASUREMENTSCONVERTER_H
#define VKNOWNMEASUREMENTSCONVERTER_H

#include "../vmisc/projectversion.h"
#include "vabstractconverter.h"

class VKnownMeasurementsConverter : public VAbstractConverter
{
    Q_OBJECT // NOLINT

public:
    explicit VKnownMeasurementsConverter(const QString &fileName);
    ~VKnownMeasurementsConverter() override = default;

    auto GetFormatVersionStr() const -> QString override;

    static const QString KnownMeasurementsMaxVerStr;
    static const QString CurrentSchema;
    static constexpr const unsigned KnownMeasurementsMinVer = FormatVersion(1, 0, 0);
    static constexpr const unsigned KnownMeasurementsMaxVer = FormatVersion(1, 0, 0);

    static auto XSDSchemas() -> QHash<unsigned, QString>;

protected:
    auto MinVer() const -> unsigned override;
    auto MaxVer() const -> unsigned override;

    auto MinVerStr() const -> QString override;
    auto MaxVerStr() const -> QString override;

    void ApplyPatches() override;
    void DowngradeToCurrentMaxVersion() override;

    auto IsReadOnly() const -> bool override;

    auto Schemas() const -> QHash<unsigned, QString> override;

private:
    Q_DISABLE_COPY_MOVE(VKnownMeasurementsConverter) // NOLINT
    static const QString KnownMeasurementsMinVerStr;
};

#endif // VKNOWNMEASUREMENTSCONVERTER_H
