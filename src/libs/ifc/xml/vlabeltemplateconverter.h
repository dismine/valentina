/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 8, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VLABELTEMPLATECONVERTER_H
#define VLABELTEMPLATECONVERTER_H

#include "vabstractconverter.h"
#include "../vmisc/projectversion.h"

class VLabelTemplateConverter : public VAbstractConverter
{
public:
    explicit VLabelTemplateConverter(const QString &fileName);
    virtual ~VLabelTemplateConverter() = default;

    static const QString LabelTemplateMaxVerStr;
    static const QString CurrentSchema;
    static constexpr const unsigned LabelTemplateMinVer = FormatVersion(1, 0, 0);
    static constexpr const unsigned LabelTemplateMaxVer = FormatVersion(1, 0, 0);

    static auto XSDSchemas() -> QHash <unsigned, QString>;

protected:
    virtual auto MinVer() const -> unsigned override;
    virtual auto MaxVer() const -> unsigned override;

    virtual auto MinVerStr() const -> QString override;
    virtual auto MaxVerStr() const -> QString override;

    virtual void ApplyPatches() override;
    virtual void DowngradeToCurrentMaxVersion() override;

    virtual auto IsReadOnly() const -> bool override { return false; }

    auto Schemas() const -> QHash <unsigned, QString> override;

private:
    Q_DISABLE_COPY_MOVE(VLabelTemplateConverter) // NOLINT
    static const QString LabelTemplateMinVerStr;
};

#endif // VLABELTEMPLATECONVERTER_H
