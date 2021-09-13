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
    Q_DECLARE_TR_FUNCTIONS(VLayoutConverter)
public:
    explicit VLayoutConverter(const QString &fileName);
    virtual ~VLayoutConverter() Q_DECL_EQ_DEFAULT;

    virtual QString GetFormatVersionStr() const override;

    static const QString LayoutMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const int LayoutMinVer = FormatVersion(0, 1, 0);
    static Q_DECL_CONSTEXPR const int LayoutMaxVer = FormatVersion(0, 1, 0);

protected:
    virtual int     MinVer() const override;
    virtual int     MaxVer() const override;

    virtual QString MinVerStr() const override;
    virtual QString MaxVerStr() const override;

    virtual QString XSDSchema(int ver) const override;
    virtual void    ApplyPatches() override;
    virtual void    DowngradeToCurrentMaxVersion() override;

    virtual bool IsReadOnly() const override;

private:
    Q_DISABLE_COPY(VLayoutConverter)
    static const QString LayoutMinVerStr;
};

//---------------------------------------------------------------------------------------------------------------------
inline int VLayoutConverter::MinVer() const
{
    return LayoutMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline int VLayoutConverter::MaxVer() const
{
    return LayoutMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VLayoutConverter::MinVerStr() const
{
    return LayoutMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VLayoutConverter::MaxVerStr() const
{
    return LayoutMaxVerStr;
}

#endif // VLAYOUTCONVERTER_H
