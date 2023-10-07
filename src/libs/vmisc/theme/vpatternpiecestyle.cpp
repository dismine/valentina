/************************************************************************
 **
 **  @file   vpatternpiecestyle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 7, 2023
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
#include "vpatternpiecestyle.h"

#include <QJsonObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, PatternPieceStyleNodeVar, ("PatternPieceStyle"_L1))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PieceColorVar, ("PieceColor"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PointColorVar, ("PointColor"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, NodeLabelColorVar, ("NodeLabelColor"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, NodeLabelHoverColorVar, ("NodeLabelHoverColor"_L1))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, NodeLabelLineColorVar, ("NodeLabelLineColor"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, LabelBackgroundColorVar, ("LabelBackgroundColor"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, LabelTextColorVar, ("LabelTextColor"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, LabelModeColorVar, ("LabelModeColor"_L1))             // NOLINT

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPatternPieceStyle::VPatternPieceStyle()
{
    LoadJsonFile(VStylesheetStyle::GetResourceName());
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternPieceStyle::LoadJson(const QJsonObject &json)
{
    QJsonValue toolStyleValues = json[*PatternPieceStyleNodeVar];

    QJsonObject obj = toolStyleValues.toObject();

    ReadColor(obj, *PieceColorVar, m_pieceColor);
    ReadColor(obj, *PointColorVar, m_pointColor);
    ReadColor(obj, *NodeLabelColorVar, m_nodeLabelColor);
    ReadColor(obj, *NodeLabelHoverColorVar, m_nodeLabelHoverColor);
    ReadColor(obj, *NodeLabelLineColorVar, m_nodeLabelLineColor);
    ReadColor(obj, *LabelBackgroundColorVar, m_labelBackgroundColor);
    ReadColor(obj, *LabelTextColorVar, m_labelTextColor);
    ReadColor(obj, *LabelModeColorVar, m_labelModeColor);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternPieceStyle::ToJson() const -> QJsonObject
{
    QJsonObject obj;

    WriteColor(obj, *PieceColorVar, m_pieceColor);
    WriteColor(obj, *PointColorVar, m_pointColor);
    WriteColor(obj, *NodeLabelColorVar, m_nodeLabelColor);
    WriteColor(obj, *NodeLabelHoverColorVar, m_nodeLabelHoverColor);
    WriteColor(obj, *NodeLabelLineColorVar, m_nodeLabelLineColor);
    WriteColor(obj, *LabelBackgroundColorVar, m_labelBackgroundColor);
    WriteColor(obj, *LabelTextColorVar, m_labelTextColor);
    WriteColor(obj, *LabelModeColorVar, m_labelModeColor);

    QJsonObject root;
    root[*PatternPieceStyleNodeVar] = obj;

    return root;
}
