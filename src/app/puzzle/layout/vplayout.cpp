/************************************************************************
 **
 **  @file   vplayout.cpp
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
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

#include "vplayout.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vwatermarkconverter.h"
#include "../vformat/vwatermark.h"
#include "../vpapplication.h"
#include "../vptilefactory.h"
#include "vppiece.h"
#include "vpsheet.h"

#include <QLoggingCategory>
#include <QPixmapCache>
#include <QUndoStack>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pLayout, "p.layout") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
VPLayout::VPLayout(QUndoStack *undoStack)
  : m_undoStack(undoStack)
{
    SCASSERT(m_undoStack != nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
VPLayout::~VPLayout() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::CreateLayout(QUndoStack *undoStack) -> VPLayoutPtr
{
    SCASSERT(undoStack != nullptr)
    undoStack->clear();
    VPLayoutPtr layout(new VPLayout(undoStack));
    layout->AddTrashSheet(VPSheetPtr(new VPSheet(layout)));

    // create a standard sheet
    VPSheetPtr const sheet(new VPSheet(layout));
    sheet->SetName(tr("Sheet %1").arg(layout->GetAllSheets().size() + 1));
    layout->AddSheet(sheet);
    layout->SetFocusedSheet(sheet);

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

    layout->LayoutSettings().SetUnit(settings->LayoutUnit());

    layout->LayoutSettings().SetShowTiles(settings->GetLayoutTileShowTiles());
    layout->LayoutSettings().SetShowWatermark(settings->GetLayoutTileShowWatermark());
    layout->LayoutSettings().SetTilesSize(
        QSizeF(settings->GetLayoutTilePaperWidth(), settings->GetLayoutTilePaperHeight()));
    layout->LayoutSettings().SetIgnoreTilesMargins(settings->GetLayoutTileIgnoreMargins());
    layout->LayoutSettings().SetTilesMargins(settings->GetLayoutTileMargins());

    layout->LayoutSettings().SetWarningSuperpositionOfPieces(settings->GetLayoutWarningPiecesSuperposition());
    layout->LayoutSettings().SetWarningPiecesOutOfBound(settings->GetLayoutWarningPiecesOutOfBound());
    layout->LayoutSettings().SetWarningPieceGapePosition(settings->GetLayoutWarningPieceGapePosition());
    layout->LayoutSettings().SetFollowGrainline(settings->GetLayoutFollowGrainline());
    layout->LayoutSettings().SetStickyEdges(settings->GetLayoutStickyEdges());
    layout->LayoutSettings().SetPiecesGap(settings->GetLayoutPieceGap());

    // --------------------------------------------------------

    // init the tile factory
    auto tileFactory = QSharedPointer<VPTileFactory>(new VPTileFactory(layout, settings));
    tileFactory->RefreshTileInfos();
    tileFactory->RefreshWatermarkData();
    layout->SetTileFactory(tileFactory);

    return layout;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::AddPiece(const VPLayoutPtr &layout, const VPPiecePtr &piece)
{
    piece->SetLayout(layout);
    layout->AddPiece(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::AddPiece(const VPPiecePtr &piece)
{
    if (piece.isNull())
    {
        return;
    }

    VPPiece::CleanPosition(piece);

    const QString uniqueId = piece->GetUniqueID();
    if (not m_pieces.contains(uniqueId))
    {
        m_pieces.insert(uniqueId, piece);
    }
    else
    {
        VPPiecePtr const oldPiece = m_pieces.value(uniqueId);
        if (not oldPiece.isNull())
        {
            oldPiece->Update(piece);
            emit PieceTransformationChanged(oldPiece);
        }
        else
        {
            m_pieces.insert(uniqueId, piece);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::Uuid() const -> const QUuid &
{
    return m_uuid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::TileFactory() const -> QSharedPointer<VPTileFactory>
{
    return m_tileFactory;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTileFactory(const QSharedPointer<VPTileFactory> &newTileFactory)
{
    m_tileFactory.clear();
    m_tileFactory = newTileFactory;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::RefreshScenePieces() const
{
    const QList<VPSheetPtr> sheets = GetSheets();
    for (const auto &sheet : sheets)
    {
        if (not sheet.isNull())
        {
            sheet->SceneData()->RefreshPieces();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::WatermarkData() const -> VWatermarkData
{
    VWatermarkData data;
    if (not m_layoutSettings.WatermarkPath().isEmpty())
    {
        try
        {
            VWatermarkConverter converter(m_layoutSettings.WatermarkPath());
            VWatermark watermark;
            watermark.setXMLContent(converter.Convert());
            data = watermark.GetWatermark();
        }
        catch (VException &)
        {
            data.invalidFile = true;
            data.opacity = 20;
            data.showImage = true;
            data.path = "fake.png"_L1;
            data.showText = false;
            return data;
        }
    }

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::IsSheetsUniform() const -> bool
{
    QList<VPSheetPtr> sheets = GetSheets();
    if (sheets.size() < 2)
    {
        return true;
    }

    const VPSheetPtr &sheet = sheets.constFirst();
    if (sheet.isNull())
    {
        return false;
    }

    QSizeF const sheetSize = sheet->GetSheetSize().toSize();

    return std::all_of(sheets.begin(), sheets.end(),
                       [sheetSize](const VPSheetPtr &sheet)
                       {
                           if (sheet.isNull())
                           {
                               return false;
                           }
                           QSize const size = sheet->GetSheetSize().toSize();
                           return size == sheetSize || size.transposed() == sheetSize;
                       });
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetPieces() const -> QList<VPPiecePtr>
{
    return m_pieces.values();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetPlacedPieces() const -> QList<VPPiecePtr>
{
    QList<VPPiecePtr> pieces;
    pieces.reserve(m_pieces.size());

    for (const auto &piece : m_pieces)
    {
        if (not piece->isNull() && piece->Sheet() != VPSheetPtr() && piece->Sheet() != m_trashSheet)
        {
            pieces.append(piece);
        }
    }

    return pieces;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetUnplacedPieces() const -> QList<VPPiecePtr>
{
    return PiecesForSheet(VPSheetPtr());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetTrashedPieces() const -> QList<VPPiecePtr>
{
    if (m_trashSheet.isNull())
    {
        return {};
    }
    return PiecesForSheet(m_trashSheet->Uuid());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::AddSheet(const VPSheetPtr &sheet) -> VPSheetPtr
{
    if (not sheet.isNull() && GetSheet(sheet->Uuid()).isNull())
    {
        m_sheets.append(sheet);
        connect(this, &VPLayout::PieceTransformationChanged, sheet.data(), &VPSheet::CheckPiecePositionValidity);
    }
    return sheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetAllSheets() const -> QList<VPSheetPtr>
{
    return m_sheets;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetSheets() const -> QList<VPSheetPtr>
{
    QList<VPSheetPtr> sheets;
    sheets.reserve(m_sheets.size());

    for (const auto &sheet : m_sheets)
    {
        if (not sheet.isNull() && sheet->IsVisible())
        {
            sheets.append(sheet);
        }
    }
    return sheets;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetSheet(const QUuid &uuid) -> VPSheetPtr
{
    if (auto sheet = std::find_if(m_sheets.begin(), m_sheets.end(),
                                  [uuid](const VPSheetPtr &sheet) { return sheet->Uuid() == uuid; });
        sheet != m_sheets.end())
    {
        return *sheet;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetFocusedSheet(const VPSheetPtr &focusedSheet)
{
    if (m_sheets.isEmpty())
    {
        m_focusedSheet = {};
    }
    else
    {
        m_focusedSheet = focusedSheet.isNull() ? m_sheets.constFirst() : focusedSheet;
    }

    CheckPiecesPositionValidity(m_focusedSheet);

    emit ActiveSheetChanged(m_focusedSheet);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetFocusedSheet() -> VPSheetPtr
{
    return m_focusedSheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetTrashSheet() -> VPSheetPtr
{
    return m_trashSheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::LayoutSettings() const -> const VPLayoutSettings &
{
    return m_layoutSettings;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::LayoutSettings() -> VPLayoutSettings &
{
    return m_layoutSettings;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::PiecesForSheet(const VPSheetPtr &sheet) const -> QList<VPPiecePtr>
{
    QList<VPPiecePtr> list;
    list.reserve(m_pieces.size());

    for (const auto &piece : m_pieces)
    {
        if (not piece.isNull() && piece->Sheet() == sheet)
        {
            list.append(piece);
        }
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::PiecesForSheet(const QUuid &uuid) const -> QList<VPPiecePtr>
{
    QList<VPPiecePtr> list;
    list.reserve(m_pieces.size());

    for (const auto &piece : m_pieces)
    {
        if (not piece.isNull())
        {
            VPSheetPtr const sheet = piece->Sheet();
            if (not sheet.isNull() && sheet->Uuid() == uuid)
            {
                list.append(piece);
            }
        }
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::UndoStack() const -> QUndoStack *
{
    return m_undoStack;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetUndoStack(QUndoStack *newUndoStack)
{
    m_undoStack = newUndoStack;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::Clear()
{
    if (m_undoStack != nullptr)
    {
        m_undoStack->clear();
    }

    m_pieces.clear();
    m_trashSheet->Clear();
    m_sheets.clear();
    m_focusedSheet.clear();
    m_layoutSettings = VPLayoutSettings();
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::CheckPiecesPositionValidity() const
{
    for (const auto &sheet : m_sheets)
    {
        CheckPiecesPositionValidity(sheet);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::CheckPiecesPositionValidity(const VPSheetPtr &sheet) const
{
    if (not sheet.isNull())
    {
        const VPLayoutSettings &settings = LayoutSettings();

        if (settings.GetWarningPiecesOutOfBound())
        {
            sheet->ValidatePiecesOutOfBound();
        }

        if (settings.GetWarningSuperpositionOfPieces())
        {
            sheet->ValidateSuperpositionOfPieces();
        }

        if (settings.GetWarningPieceGapePosition())
        {
            sheet->ValidatePieceGapePosition();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::AddTrashSheet(const VPSheetPtr &sheet)
{
    m_trashSheet = sheet;
    m_trashSheet->SetTrashSheet(true);
}
