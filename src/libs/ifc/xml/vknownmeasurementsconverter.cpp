/************************************************************************
 **
 **  @file   vknownmeasurementsconverter.cpp
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
#include "vknownmeasurementsconverter.h"

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VKnownMeasurementsConverter::KnownMeasurementsMinVerStr = QStringLiteral("1.0.0");
const QString VKnownMeasurementsConverter::KnownMeasurementsMaxVerStr = QStringLiteral("1.0.0");
const QString VKnownMeasurementsConverter::CurrentSchema = QStringLiteral("://schema/known_measurements/v1.0.0.xsd");

// VKnownMeasurementsConverter::KnownMeasurementsMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
// VKnownMeasurementsConverter::KnownMeasurementsMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

//---------------------------------------------------------------------------------------------------------------------
VKnownMeasurementsConverter::VKnownMeasurementsConverter(const QString &fileName)
  : VAbstractConverter(fileName)
{
    m_ver = GetFormatVersion(VKnownMeasurementsConverter::GetFormatVersionStr());
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::GetFormatVersionStr() const -> QString
{
    if (QDomNode const root = documentElement(); not root.isNull() && root.isElement())
    {
        const QDomElement layoutElement = root.toElement();
        if (not layoutElement.isNull())
        {
            return GetParametrString(layoutElement, AttrKMVersion, "0.0.0"_L1);
        }
    }
    return "0.0.0"_L1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::XSDSchemas() -> QHash<unsigned int, QString>
{
    static auto schemas = QHash<unsigned, QString>{std::make_pair(FormatVersion(1, 0, 0), CurrentSchema)};

    return schemas;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::MinVer() const -> unsigned int
{
    return KnownMeasurementsMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::MaxVer() const -> unsigned int
{
    return KnownMeasurementsMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::MinVerStr() const -> QString
{
    return KnownMeasurementsMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::MaxVerStr() const -> QString
{
    return KnownMeasurementsMaxVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (FormatVersion(1, 0, 0)):
            break;
        default:
            InvalidVersion(m_ver);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(KnownMeasurementsMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::IsReadOnly() const -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsConverter::Schemas() const -> QHash<unsigned int, QString>
{
    return XSDSchemas();
}
