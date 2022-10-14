/************************************************************************
 **
 **  @file   vlayoutconverter.cpp
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
#include "vlayoutconverter.h"
#include "../exception/vexception.h"
#include "ifcdef.h"

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VLayoutConverter::LayoutMinVerStr = QStringLiteral("0.1.0");
const QString VLayoutConverter::LayoutMaxVerStr = QStringLiteral("0.1.2");
const QString VLayoutConverter::CurrentSchema   = QStringLiteral("://schema/layout/v0.1.2.xsd");

//VLayoutConverter::LayoutMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
//VLayoutConverter::LayoutMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

//---------------------------------------------------------------------------------------------------------------------
VLayoutConverter::VLayoutConverter(const QString &fileName)
    : VAbstractConverter(fileName)
{
    m_ver = GetFormatVersion(VLayoutConverter::GetFormatVersionStr());
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::GetFormatVersionStr() const -> QString
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        const QDomElement layoutElement = root.toElement();
        if (not layoutElement.isNull())
        {
            return GetParametrString(layoutElement, AttrLayoutVersion, QStringLiteral("0.0.0"));
        }
    }
    return QStringLiteral("0.0.0");
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::SetVersion(const QString &version)
{
    ValidateVersion(version);

    QDomElement root = documentElement().toElement();
    if (root.isElement() && root.hasAttribute(AttrLayoutVersion))
    {
        root.setAttribute(AttrLayoutVersion, version);
    }
    else
    {
        throw VException(tr("Could not change version."));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::XSDSchema(unsigned ver) const -> QString
{
    QHash <unsigned, QString> schemas =
    {
        std::make_pair(FormatVersion(0, 1, 0), QStringLiteral("://schema/layout/v0.1.0.xsd")),
        std::make_pair(FormatVersion(0, 1, 1), QStringLiteral("://schema/layout/v0.1.1.xsd")),
        std::make_pair(FormatVersion(0, 1, 2), CurrentSchema),
    };

    if (schemas.contains(ver))
    {
        return schemas.value(ver);
    }

    InvalidVersion(ver);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (FormatVersion(0, 1, 0)):
        case (FormatVersion(0, 1, 1)):
            ToV0_1_2();
            ValidateXML(XSDSchema(FormatVersion(0, 1, 2)));
            Q_FALLTHROUGH();
        case (FormatVersion(0, 1, 2)):
            break;
        default:
            InvalidVersion(m_ver);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(LayoutMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::IsReadOnly() const -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ToV0_1_2()
{
    // TODO. Delete if minimal supported version is 0.1.2
    Q_STATIC_ASSERT_X(VLayoutConverter::LayoutMinVer < FormatVersion(0, 1, 2),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.1.2"));
    Save();
}
