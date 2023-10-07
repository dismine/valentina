/************************************************************************
 **
 **  @file   vtoolstyle.cpp
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
#include "vtoolstyle.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <algorithm>
#include <vector>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, ToolStyleNodeVar, ("ToolStyle"_L1))                 // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PatternColorVar, ("PatternColor"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, DisabledColorVar, ("DisabledColor"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, ColorAdjustmentsVar, ("ColorAdjustments"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PointColorVar, ("PointColor"_L1))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, LabelColorVar, ("LabelColor"_L1))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, LabelHoverColorVar, ("LabelHoverColor"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, LabelLineColorVar, ("LabelLineColor"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, AccuracyRadiusColorVar, ("AccuracyRadiusColor"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, ControlLineColorVar, ("ControlLineColor"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, ControlPointColorVar, ("ControlPointColor"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, VisMainColorVar, ("VisMainColor"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, VisSupportColorVar, ("VisSupportColor"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, VisSupportColor2Var, ("VisSupportColor2"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, VisSupportColor3Var, ("VisSupportColor3"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, VisSupportColor4Var, ("VisSupportColor4"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, BasePointColorVar, ("BasePointColor"_L1))           // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
void ReadColorAdjustments(const QJsonObject &values, const QString &variable, QMap<QString, QString> &adjustments)
{
    auto valueRef = values[variable];

    if (valueRef.type() == QJsonValue::Undefined || valueRef.type() == QJsonValue::Null || !valueRef.isObject())
    {
        return;
    }

    QJsonObject colorAdjustmentsObject = valueRef.toObject();

    for (const QString &key : colorAdjustmentsObject.keys())
    {
        QColor keyColor(key);
        QColor valueColor = VStylesheetStyle::ReadColorValue(colorAdjustmentsObject.value(key));
        QString colorName = valueColor.name(QColor::HexArgb);
        adjustments.insert(keyColor.name(QColor::HexArgb), colorName);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void WriteColorAdjustments(const QJsonObject &values, const QString &variable,
                           const QMap<QString, QString> &adjustments)
{
    QJsonObject colorAdjustmentsObject;

    for (auto it = adjustments.begin(); it != adjustments.end(); ++it)
    {
        colorAdjustmentsObject.insert(it.key(), it.value());
    }

    values[variable] = colorAdjustmentsObject;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VToolStyle::VToolStyle()
{
    LoadJsonFile(VStylesheetStyle::GetResourceName());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolStyle::LoadJson(const QJsonObject &json)
{
    QJsonValue toolStyleValues = json[*ToolStyleNodeVar];

    QJsonObject obj = toolStyleValues.toObject();

    ReadColor(obj, *PatternColorVar, m_patternColor);
    ReadColor(obj, *DisabledColorVar, m_disabledColor);
    ReadColorAdjustments(obj, *ColorAdjustmentsVar, m_colorAdjustments);
    ReadColor(obj, *PointColorVar, m_pointColor);
    ReadColor(obj, *LabelColorVar, m_labelColor);
    ReadColor(obj, *LabelHoverColorVar, m_labelHoverColor);
    ReadColor(obj, *LabelLineColorVar, m_labelLineColor);
    ReadColor(obj, *AccuracyRadiusColorVar, m_accuracyRadiusColor);
    ReadColor(obj, *ControlLineColorVar, m_controlLineColor);
    ReadColor(obj, *ControlPointColorVar, m_controlPointColor);
    ReadColor(obj, *VisMainColorVar, m_visMainColor);
    ReadColor(obj, *VisSupportColorVar, m_visSupportColor);
    ReadColor(obj, *VisSupportColor2Var, m_visSupportColor2);
    ReadColor(obj, *VisSupportColor3Var, m_visSupportColor3);
    ReadColor(obj, *VisSupportColor4Var, m_visSupportColor4);
    ReadColor(obj, *BasePointColorVar, m_basePointColor);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolStyle::ToJson() const -> QJsonObject
{
    QJsonObject obj;

    WriteColor(obj, *PatternColorVar, m_patternColor);
    WriteColor(obj, *DisabledColorVar, m_disabledColor);
    WriteColorAdjustments(obj, *ColorAdjustmentsVar, m_colorAdjustments);
    WriteColor(obj, *PointColorVar, m_pointColor);
    WriteColor(obj, *LabelColorVar, m_labelColor);
    WriteColor(obj, *LabelHoverColorVar, m_labelHoverColor);
    WriteColor(obj, *LabelLineColorVar, m_labelLineColor);
    WriteColor(obj, *AccuracyRadiusColorVar, m_accuracyRadiusColor);
    WriteColor(obj, *ControlLineColorVar, m_controlLineColor);
    WriteColor(obj, *ControlPointColorVar, m_controlPointColor);
    WriteColor(obj, *VisMainColorVar, m_visMainColor);
    WriteColor(obj, *VisSupportColorVar, m_visSupportColor);
    WriteColor(obj, *VisSupportColor2Var, m_visSupportColor2);
    WriteColor(obj, *VisSupportColor3Var, m_visSupportColor3);
    WriteColor(obj, *VisSupportColor4Var, m_visSupportColor4);
    WriteColor(obj, *BasePointColorVar, m_basePointColor);

    QJsonObject root;
    root[*ToolStyleNodeVar] = obj;

    return root;
}
