/************************************************************************
 **
 **  @file   vstylesheetstyle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 7, 2023
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
#include "vstylesheetstyle.h"
#include "vtheme.h"

#include <QByteArray>
#include <QColor>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <algorithm>
#include <vector>

namespace
{
ExportColorScheme exportColorScheme = ExportColorScheme::Default;
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::LoadJsonFromByteArray(const QByteArray &byteArray)
{
    auto json = QJsonDocument::fromJson(byteArray).object();

    LoadJson(json);
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::LoadJsonText(const QString &jsonText)
{
    LoadJsonFromByteArray(jsonText.toUtf8());
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::LoadJsonFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Couldn't open file " << fileName;

        return;
    }

    LoadJsonFromByteArray(file.readAll());
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::SetExportColorScheme(ExportColorScheme colorScheme)
{
    exportColorScheme = colorScheme;
}

//---------------------------------------------------------------------------------------------------------------------
auto VStylesheetStyle::GetResourceName() -> QString
{
    const QString light = QStringLiteral("light");
    QString colorSheme;

    if (exportColorScheme == ExportColorScheme::BackAndWhite)
    {
        colorSheme = light;
    }
    else
    {
        colorSheme = (VTheme::ColorSheme() == VColorSheme::Dark ? QStringLiteral("dark") : light);
    }

    return QStringLiteral(":/scenestyle/%1/style.json").arg(colorSheme);
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::CheckUndefinedValue(const QJsonValue &v, const QString &variable)
{
    if (v.type() == QJsonValue::Undefined || v.type() == QJsonValue::Null)
    {
        qWarning() << "Undefined value for parameter:" << variable;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VStylesheetStyle::ReadColorValue(const QJsonValue &value) -> QColor
{
    if (value.isArray())
    {
        auto colorArray = value.toArray();
        std::vector<int> rgba;
        rgba.reserve(4);

        std::transform(colorArray.begin(), colorArray.end(), std::back_inserter(rgba),
                       [](const QJsonValue &value) { return value.toInt(); });

        if (rgba.size() == 3)
        {
            rgba.push_back(255);
        }

        if (rgba.size() == 4)
        {
            return {rgba[0], rgba[1], rgba[2], rgba[3]};
        }
    }
    else
    {
        return {value.toString()};
    }
    return Qt::black;
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::ReadColor(const QJsonObject &values, const QString &colorName, QColor &variable)
{
    auto valueRef = values[colorName];

    CheckUndefinedValue(valueRef, colorName);

    variable = ReadColorValue(valueRef);
}

//---------------------------------------------------------------------------------------------------------------------
void VStylesheetStyle::WriteColor(const QJsonObject &values, const QString &colorName, const QColor &variable)
{
    values[colorName] = variable.name(QColor::HexArgb);
}
