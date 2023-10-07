/************************************************************************
 **
 **  @file   vmanuallayoutstyle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 7, 2023
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
#include "vmanuallayoutstyle.h"

#include <QJsonObject>
#include <QJsonValue>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, ManualLayoutStyleNodeVar, ("ManualLayoutStyle"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SheetBorderColorVar, ("SheetBorderColor"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SheetMarginColorVar, ("SheetMarginColor"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SheetGridColorVar, ("SheetGridColor"_L1))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SheetTileGridColorVar, ("SheetTileGridColor"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, SheetTileNumberColorVar, ("SheetTileNumberColor"_L1))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PieceOkColorVar, ("PieceOkColor"_L1))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PieceErrorColorVar, ("PieceErrorColor"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PieceHoverColorVar, ("PieceHoverColor"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PieceHandleColorVar, ("PieceHandleColor"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, PieceHandleHoverColorVar, ("PieceHandleHoverColor"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, CarrouselPieceColorVar, ("CarrouselPieceColor"_L1))     // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, CarrouselPieceSelectedColorVar, ("CarrouselPieceSelectedColor"_L1))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, CarrouselPieceBackgroundColorVar, ("CarrouselPieceBackgroundColor"_L1))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, CarrouselPieceForegroundColorVar, ("CarrouselPieceForegroundColor"_L1))

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VManualLayoutStyle::VManualLayoutStyle()
{
    LoadJsonFile(VStylesheetStyle::GetResourceName());
}

//---------------------------------------------------------------------------------------------------------------------
void VManualLayoutStyle::LoadJson(const QJsonObject &json)
{
    QJsonValue toolStyleValues = json[*ManualLayoutStyleNodeVar];

    QJsonObject obj = toolStyleValues.toObject();

    ReadColor(obj, *SheetBorderColorVar, m_sheetBorderColor);
    ReadColor(obj, *SheetMarginColorVar, m_sheetMarginColor);
    ReadColor(obj, *SheetGridColorVar, m_sheetGridColor);
    ReadColor(obj, *SheetTileGridColorVar, m_sheetTileGridColor);
    ReadColor(obj, *SheetTileNumberColorVar, m_sheetTileNumberColor);
    ReadColor(obj, *PieceOkColorVar, m_pieceOkColor);
    ReadColor(obj, *PieceErrorColorVar, m_pieceErrorColor);
    ReadColor(obj, *PieceHoverColorVar, m_pieceHoverColor);
    ReadColor(obj, *PieceHandleColorVar, m_pieceHandleColor);
    ReadColor(obj, *PieceHandleHoverColorVar, m_pieceHandleHoverColor);
    ReadColor(obj, *CarrouselPieceColorVar, m_carrouselPieceColor);
    ReadColor(obj, *CarrouselPieceSelectedColorVar, m_carrouselPieceSelectedColor);
    ReadColor(obj, *CarrouselPieceBackgroundColorVar, m_carrouselPieceBackgroundColor);
    ReadColor(obj, *CarrouselPieceForegroundColorVar, m_carrouselPieceForegroundColor);
}

//---------------------------------------------------------------------------------------------------------------------
auto VManualLayoutStyle::ToJson() const -> QJsonObject
{
    QJsonObject obj;

    WriteColor(obj, *SheetBorderColorVar, m_sheetBorderColor);
    WriteColor(obj, *SheetMarginColorVar, m_sheetMarginColor);
    WriteColor(obj, *SheetGridColorVar, m_sheetGridColor);
    WriteColor(obj, *SheetTileGridColorVar, m_sheetTileGridColor);
    WriteColor(obj, *SheetTileNumberColorVar, m_sheetTileNumberColor);
    WriteColor(obj, *PieceOkColorVar, m_pieceOkColor);
    WriteColor(obj, *PieceErrorColorVar, m_pieceErrorColor);
    WriteColor(obj, *PieceHoverColorVar, m_pieceHoverColor);
    WriteColor(obj, *PieceHandleColorVar, m_pieceHandleColor);
    WriteColor(obj, *PieceHandleHoverColorVar, m_pieceHandleHoverColor);
    WriteColor(obj, *CarrouselPieceColorVar, m_carrouselPieceColor);
    WriteColor(obj, *CarrouselPieceSelectedColorVar, m_carrouselPieceSelectedColor);
    WriteColor(obj, *CarrouselPieceBackgroundColorVar, m_carrouselPieceBackgroundColor);
    WriteColor(obj, *CarrouselPieceForegroundColorVar, m_carrouselPieceForegroundColor);

    QJsonObject root;
    root[*ManualLayoutStyleNodeVar] = obj;

    return root;
}
