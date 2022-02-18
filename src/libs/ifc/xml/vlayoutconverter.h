/************************************************************************
 **
 **  @file   vlayoutconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef VLAYOUTCONVERTER_H
#define VLAYOUTCONVERTER_H

#include "vabstractconverter.h"

class QDomElement;
class QString;

class VLayoutConverter : public VAbstractConverter
{
public:
    explicit VLayoutConverter(const QString &fileName);
    ~VLayoutConverter() override =default;

    auto GetFormatVersionStr() const -> QString override;

    static const QString LayoutMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const unsigned LayoutMinVer = FormatVersion(0, 1, 0);
    static Q_DECL_CONSTEXPR const unsigned LayoutMaxVer = FormatVersion(0, 1, 1);

protected:
    void SetVersion(const QString &version) override;

    auto MinVer() const -> unsigned override;
    auto MaxVer() const -> unsigned override;

    auto MinVerStr() const -> QString override;
    auto MaxVerStr() const -> QString override;

    auto XSDSchema(unsigned ver) const -> QString override;
    void ApplyPatches() override;
    void DowngradeToCurrentMaxVersion() override;

    auto IsReadOnly() const -> bool override;

    void ToV0_1_1();

private:
    Q_DISABLE_COPY(VLayoutConverter)
    static const QString LayoutMinVerStr;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutConverter::MinVer() const -> unsigned
{
    return LayoutMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutConverter::MaxVer() const -> unsigned
{
    return LayoutMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutConverter::MinVerStr() const -> QString
{
    return LayoutMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLayoutConverter::MaxVerStr() const -> QString
{
    return LayoutMaxVerStr;
}

#endif // VLAYOUTCONVERTER_H
