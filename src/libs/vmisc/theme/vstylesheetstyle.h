/************************************************************************
 **
 **  @file   vstylesheetstyle.h
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
#ifndef VSTYLESHEETSTYLE_H
#define VSTYLESHEETSTYLE_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QtCompilerDetection>
#endif

class QJsonObject;
class QJsonValue;
class QByteArray;
class QString;
class QColor;

enum class ExportColorScheme
{
    Default,
    BackAndWhite
};

class VStylesheetStyle
{
public:
    VStylesheetStyle() = default;
    virtual ~VStylesheetStyle() = default;

    VStylesheetStyle(const VStylesheetStyle &) = default;
    auto operator=(const VStylesheetStyle &) -> VStylesheetStyle & = default;

#ifdef Q_COMPILER_RVALUE_REFS
    VStylesheetStyle(VStylesheetStyle &&) = default;
    auto operator=(VStylesheetStyle &&) -> VStylesheetStyle & = default;
#endif

    virtual void LoadJson(QJsonObject const &json) = 0;
    virtual auto ToJson() const -> QJsonObject = 0;

    /// Loads from utf-8 byte array.
    virtual void LoadJsonFromByteArray(const QByteArray &byteArray);
    virtual void LoadJsonText(const QString &jsonText);
    virtual void LoadJsonFile(const QString &fileName);

    static void SetExportColorScheme(ExportColorScheme colorScheme);

    static auto GetResourceName() -> QString;

    static void CheckUndefinedValue(const QJsonValue &v, const QString &variable);

    static auto ReadColorValue(const QJsonValue &value) -> QColor;
    static void ReadColor(const QJsonObject &values, const QString &colorName, QColor &variable);
    static void WriteColor(const QJsonObject &values, const QString &colorName, const QColor &variable);
};

#endif // VSTYLESHEETSTYLE_H
