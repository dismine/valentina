/************************************************************************
 **
 **  @file   vpsheet.cpp
 **  @author Ronan Le Tiec
 **  @date   23 5, 2020
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
#include "vpsheet.h"

#include "vplayout.h"
#include "vppiece.h"
#include "../vpapplication.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../scene/vpgraphicssheet.h"
#include "../scene/vpgraphicspiece.h"
#include "../scene/vpgraphicstilegrid.h"
#include "../scene/vpgraphicspiececontrols.h"
#include "../scene/vpgraphicstilegrid.h"

// VPSheetSceneData
//---------------------------------------------------------------------------------------------------------------------
VPSheetSceneData::VPSheetSceneData(const VPLayoutPtr &layout, const QUuid &sheetUuid)
    : m_layout(layout),
      m_scene(new VMainGraphicsScene()),
      m_sheetUuid(sheetUuid)
{
    SCASSERT(not layout.isNull())

    m_graphicsSheet = new VPGraphicsSheet(layout);
    m_graphicsSheet->setPos(0, 0);
    m_scene->addItem(m_graphicsSheet);

    m_graphicsTileGrid = new VPGraphicsTileGrid(layout, m_sheetUuid);
    m_scene->addItem(m_graphicsTileGrid);

    m_rotationControls = new VPGraphicsPieceControls(layout);
    m_scene->addItem(m_rotationControls);

    m_rotationOrigin = new VPGraphicsTransformationOrigin(layout);
    m_rotationOrigin->setVisible(false);
    m_scene->addItem(m_rotationOrigin);

    QObject::connect(m_rotationControls, &VPGraphicsPieceControls::ShowOrigin,
                     m_rotationOrigin, &VPGraphicsTransformationOrigin::on_ShowOrigin);
    QObject::connect(m_rotationControls, &VPGraphicsPieceControls::TransformationOriginChanged,
                     m_rotationOrigin, &VPGraphicsTransformationOrigin::SetTransformationOrigin);
}

//---------------------------------------------------------------------------------------------------------------------
VPSheetSceneData::~VPSheetSceneData()
{
    delete m_scene;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheetSceneData::Scene() const -> VMainGraphicsScene *
{
    return m_scene;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::RefreshLayout()
{
    m_graphicsSheet->update();

    m_graphicsTileGrid->update();

    m_scene->update();
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::RefreshPieces()
{
    qDeleteAll(m_graphicsPieces);
    m_graphicsPieces.clear();

    VPLayoutPtr layout = m_layout.toStrongRef();
    if(layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = layout->GetSheet(m_sheetUuid);
    if (not sheet.isNull())
    {
        QList<VPPiecePtr> pieces = sheet->GetPieces();
        m_graphicsPieces.reserve(pieces.size());

        for (const auto &piece : pieces)
        {
            if (not piece.isNull())
            {
                auto *graphicsPiece = new VPGraphicsPiece(piece);
                m_graphicsPieces.append(graphicsPiece);
                m_scene->addItem(graphicsPiece);
                ConnectPiece(graphicsPiece);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::PrepareForExport()
{
    m_graphicsSheet->SetShowBorder(false);
    m_graphicsSheet->SetShowMargin(false);

    m_rotationControls->setVisible(false);
    m_rotationOrigin->setVisible(false);

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        m_showGridTmp = layout->LayoutSettings().GetShowGrid();
        layout->LayoutSettings().SetShowGrid(false);

        m_showTilesTmp = layout->LayoutSettings().GetShowTiles();
        layout->LayoutSettings().SetShowTiles(false);

        VPSheetPtr sheet = layout->GetSheet(m_sheetUuid);
        m_slectedPiecesTmp = sheet->GetSelectedPieces();

        for (const auto& piece : qAsConst(m_slectedPiecesTmp))
        {
            if (not piece.isNull())
            {
                piece->SetSelected(false);
            }
        }

        m_outOfBoundTmp = layout->LayoutSettings().GetWarningPiecesOutOfBound();
        layout->LayoutSettings().SetWarningPiecesOutOfBound(false);

        m_pieceSuperpositionTmp = layout->LayoutSettings().GetWarningSuperpositionOfPieces();
        layout->LayoutSettings().SetWarningSuperpositionOfPieces(false);
    }

    RefreshLayout();
    RefreshPieces();
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::CleanAfterExport()
{
    m_graphicsSheet->SetShowBorder(true);
    m_graphicsSheet->SetShowMargin(true);

    m_rotationControls->setVisible(true);

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        layout->LayoutSettings().SetShowGrid(m_showGridTmp);
        layout->LayoutSettings().SetShowTiles(m_showTilesTmp);

        for (const auto& piece : qAsConst(m_slectedPiecesTmp))
        {
            if (not piece.isNull())
            {
                piece->SetSelected(true);
                emit layout->PieceSelectionChanged(piece);
            }
        }

        layout->LayoutSettings().SetWarningPiecesOutOfBound(m_outOfBoundTmp);
        layout->LayoutSettings().SetWarningSuperpositionOfPieces(m_pieceSuperpositionTmp);
    }

    RefreshLayout();
    RefreshPieces();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheetSceneData::GraphicsPieces() const -> const QList<VPGraphicsPiece *> &
{
    return m_graphicsPieces;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheetSceneData::GraphicsPiecesAsItems() const -> QList<QGraphicsItem *>
{
    QList<QGraphicsItem *> items;
    items.reserve(m_graphicsPieces.size());

    for(auto *item : m_graphicsPieces)
    {
        items.append(item);
    }

    return items;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheetSceneData::RotationControls() const -> VPGraphicsPieceControls *
{
    return m_rotationControls;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheetSceneData::ScenePiece(const VPPiecePtr &piece) const -> VPGraphicsPiece *
{
    auto _graphicsPiece =
            std::find_if(m_graphicsPieces.begin(), m_graphicsPieces.end(),
                         [piece](VPGraphicsPiece *graphicPiece) { return graphicPiece->GetPiece() == piece; });

    if (_graphicsPiece != m_graphicsPieces.end())
    {
        return *_graphicsPiece;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::RemovePiece(VPGraphicsPiece *piece)
{
    m_graphicsPieces.removeAll(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::AddPiece(VPGraphicsPiece *piece)
{
    m_graphicsPieces.append(piece);
    ConnectPiece(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::SetTextAsPaths(bool textAsPaths) const
{
    for (auto *piece : m_graphicsPieces)
    {
        if (piece != nullptr)
        {
            piece->SetTextAsPaths(textAsPaths);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::PrepareTilesScheme()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        m_showTilesSchemeTmp = layout->LayoutSettings().GetShowTiles();
        layout->LayoutSettings().SetShowTiles(true);

        m_showTilesWatermarkSchemeTmp = layout->LayoutSettings().GetShowWatermark();
        layout->LayoutSettings().SetShowWatermark(false);
    }

    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::ClearTilesScheme()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        layout->LayoutSettings().SetShowTiles(m_showTilesSchemeTmp);
        layout->LayoutSettings().SetShowWatermark(m_showTilesWatermarkSchemeTmp);
    }

    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::RefreshSheetSize()
{
    if (m_graphicsSheet != nullptr)
    {
        m_graphicsSheet->RefreshBoundingRect();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::ConnectPiece(VPGraphicsPiece *piece)
{
    SCASSERT(piece != nullptr)

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QObject::connect(layout.data(), &VPLayout::PieceTransformationChanged, piece,
                     &VPGraphicsPiece::on_RefreshPiece);
    QObject::connect(layout.data(), &VPLayout::PieceZValueChanged, piece,
                     &VPGraphicsPiece::PieceZValueChanged);
    QObject::connect(layout.data(), &VPLayout::PieceSelectionChanged,
                     m_rotationControls, &VPGraphicsPieceControls::on_UpdateControls);
    QObject::connect(layout.data(), &VPLayout::PiecePositionValidityChanged,
                     piece, &VPGraphicsPiece::on_RefreshPiece);
    QObject::connect(piece, &VPGraphicsPiece::PieceTransformationChanged,
                     m_rotationControls, &VPGraphicsPieceControls::on_UpdateControls);
    QObject::connect(piece, &VPGraphicsPiece::HideTransformationHandles,
                     m_rotationControls, &VPGraphicsPieceControls::on_HideHandles);
    QObject::connect(piece, &VPGraphicsPiece::HideTransformationHandles,
                     m_rotationOrigin, &VPGraphicsTransformationOrigin::on_HideHandles);
}

// VPSheet
//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(const VPLayoutPtr &layout, QObject *parent)
    : QObject(parent),
      m_layout(layout),
      m_sceneData(new VPSheetSceneData(layout, Uuid()))
{
    SCASSERT(not layout.isNull())

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    SetIgnoreMargins(settings->GetLayoutSheetIgnoreMargins());
    SetSheetMargins(settings->GetLayoutSheetMargins());
    SetSheetSize(QSizeF(settings->GetLayoutSheetPaperWidth(), settings->GetLayoutSheetPaperHeight()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetLayout() const -> VPLayoutPtr
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetPieces() const -> QList<VPPiecePtr>
{
    VPLayoutPtr layout = GetLayout();
    if (not layout.isNull())
    {
        return layout->PiecesForSheet(m_uuid);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSelectedPieces() const -> QList<VPPiecePtr>
{
    VPLayoutPtr layout = GetLayout();
    if (not layout.isNull())
    {
        QList<VPPiecePtr> list = layout->PiecesForSheet(m_uuid);

        QList<VPPiecePtr> selected;
        selected.reserve(list.size());

        for (const auto& piece : list)
        {
            if (not piece.isNull() && piece->IsSelected())
            {
                selected.append(piece);
            }
        }

        return selected;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetAsLayoutPieces() const -> QVector<VLayoutPiece>
{
    QList<VPPiecePtr> pieces = GetPieces();

    QVector<VLayoutPiece> details;
    details.reserve(pieces.size());

    for (const auto& piece : pieces)
    {
        if (not piece.isNull())
        {
            details.append(*piece);
        }
    }

    return details;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetName() const -> QString
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetName(const QString &name)
{
    m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::Uuid() const -> const QUuid &
{
    return m_uuid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::IsVisible() const -> bool
{
    return m_visible;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetVisible(bool visible)
{
    m_visible = visible;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GrainlineOrientation() const -> GrainlineType
{
    if (m_grainlineType == GrainlineType::NotFixed)
    {
        if (m_size.height() < m_size.width())
        {
            return GrainlineType::Horizontal;
        }

        return GrainlineType::Vertical;
    }

    return m_grainlineType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetGrainlineType() const -> GrainlineType
{
    return m_grainlineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetGrainlineType(GrainlineType type)
{
    m_grainlineType = type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::TransformationOrigin() const -> const VPTransformationOrigon &
{
    return m_transformationOrigin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetTransformationOrigin(const VPTransformationOrigon &newTransformationOrigin)
{
    m_transformationOrigin = newTransformationOrigin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::Clear()
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    m_name.clear();
    m_visible = true;
    m_transformationOrigin = VPTransformationOrigon();
    m_trashSheet = false;

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::TrashSheet() const -> bool
{
    return m_trashSheet;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetTrashSheet(bool newTrashSheet)
{
    m_trashSheet = newTrashSheet;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ValidateSuperpositionOfPieces() const
{
    QList<VPPiecePtr> pieces = GetPieces();

    for (const auto &piece : pieces)
    {
        if (piece.isNull())
        {
            continue;
        }

        const bool oldSuperpositionOfPieces = piece->HasSuperpositionWithPieces();
        QVector<QPointF> path1;
        CastTo(piece->GetMappedExternalContourPoints(), path1);
        bool hasSuperposition = false;

        for (const auto &p : pieces)
        {
            if (p.isNull() || piece == p)
            {
                continue;
            }

            QVector<QPointF> path2;
            CastTo(p->GetMappedExternalContourPoints(), path2);

            bool superposition = VPPiece::PathsSuperposition(path1, path2);
            if (superposition)
            {
                hasSuperposition = superposition;
                break;
            }
        }

        piece->SetHasSuperpositionWithPieces(hasSuperposition);

        if (oldSuperpositionOfPieces != piece->HasSuperpositionWithPieces())
        {
            VPLayoutPtr layout = GetLayout();
            if (not layout.isNull())
            {
                emit layout->PiecePositionValidityChanged(piece);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ValidatePieceOutOfBound(const VPPiecePtr &piece) const
{
    if (piece.isNull())
    {
        return;
    }

    const bool oldOutOfBound = piece->OutOfBound();

    QRectF pieceRect = piece->MappedDetailBoundingRect();
    QRectF sheetRect = GetMarginsRect();

    piece->SetOutOfBound(not sheetRect.contains(pieceRect));

    if (oldOutOfBound != piece->OutOfBound())
    {
        VPLayoutPtr layout = GetLayout();
        if (not layout.isNull())
        {
            emit layout->PiecePositionValidityChanged(piece);
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ValidatePiecesOutOfBound() const
{
    QList<VPPiecePtr> pieces = GetPieces();
    for (const auto &piece : pieces)
    {
        ValidatePieceOutOfBound(piece);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetRect() const -> QRectF
{
    return {QPoint(0, 0), m_size};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetMarginsRect() const -> QRectF
{
    if (not m_ignoreMargins)
    {
        QRectF rect = QRectF(QPointF(m_margins.left(), m_margins.top()),
                             QPointF(m_size.width() - m_margins.right(), m_size.height() - m_margins.bottom()));
        return rect;
    }

    return {0, 0, m_size.width(), m_size.height()};
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::RemoveUnusedLength()
{
    VPLayoutPtr layout = GetLayout();
    if (layout.isNull())
    {
        return;
    }

    QList<VPPiecePtr> pieces = GetPieces();
    if (pieces.isEmpty())
    {
        return;
    }

    QRectF piecesBoundingRect;

    for (const auto& piece : pieces)
    {
        if (not piece.isNull())
        {
            piece->SetSelected(false);
            emit layout->PieceSelectionChanged(piece);
            piecesBoundingRect = piecesBoundingRect.united(piece->MappedDetailBoundingRect());
        }
    }

    const qreal extra = 2;
    QRectF sheetRect = GetSheetRect();
    GrainlineType type = GrainlineOrientation();

    if (type == GrainlineType::Vertical)
    {
        qreal margin = 0;
        if (not m_ignoreMargins)
        {
            margin = m_margins.bottom();
        }

        if (sheetRect.bottomRight().y() - margin > piecesBoundingRect.bottomRight().y())
        {
            m_size = QSizeF(m_size.width(), piecesBoundingRect.bottomRight().y() + margin + extra);
        }
    }
    else if (type == GrainlineType::Horizontal)
    {
        qreal margin = 0;
        if (not m_ignoreMargins)
        {
            margin = m_margins.right();
        }

        if (sheetRect.topRight().x() - margin > piecesBoundingRect.topRight().x())
        {
            m_size = QSizeF(piecesBoundingRect.topRight().x() + margin + extra, m_size.height());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::CheckPiecePositionValidity(const VPPiecePtr &piece) const
{
    VPLayoutPtr layout = GetLayout();
    if (layout.isNull())
    {
        return;
    }

    QList<VPPiecePtr> pieces = GetPieces();
    if (piece.isNull() || not pieces.contains(piece))
    {
        return;
    }

    if (layout->LayoutSettings().GetWarningPiecesOutOfBound())
    {
        ValidatePieceOutOfBound(piece);
    }

    if (layout->LayoutSettings().GetWarningSuperpositionOfPieces())
    {
        ValidateSuperpositionOfPieces();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::SceneData() const -> VPSheetSceneData *
{
    return m_sceneData;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ClearSelection() const
{
    QList<VPPiecePtr> selectedPieces = GetSelectedPieces();
    for (const auto& piece : selectedPieces)
    {
        if (piece->IsSelected())
        {
            piece->SetSelected(false);
        }
    }

    if (not selectedPieces.isEmpty())
    {
        VPLayoutPtr layout = GetLayout();
        if (not layout.isNull())
        {
            emit layout->PieceSelectionChanged(VPPiecePtr());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetOrientation() const -> QPageLayout::Orientation
{
    return m_size.height() >= m_size.width() ? QPageLayout::Portrait : QPageLayout::Landscape;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::SheetUnits() const -> Unit
{
    VPLayoutPtr layout = GetLayout();
    if (not layout.isNull())
    {
        return layout->LayoutSettings().GetUnit();
    }

    return Unit::Cm;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(qreal width, qreal height)
{
    m_size.setWidth(width);
    m_size.setHeight(height);

    if (m_sceneData != nullptr)
    {
        m_sceneData->RefreshSheetSize();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(qreal width, qreal height)
{
    Unit unit = SheetUnits();
    m_size.setWidth(UnitConvertor(width, unit, Unit::Px));
    m_size.setHeight(UnitConvertor(height, unit, Unit::Px));

    if (m_sceneData != nullptr)
    {
        m_sceneData->RefreshSheetSize();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(const QSizeF &size)
{
    m_size = size;

    if (m_sceneData != nullptr)
    {
        m_sceneData->RefreshSheetSize();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(const QSizeF &size)
{
    Unit unit = SheetUnits();
    m_size = QSizeF(UnitConvertor(size.width(), unit, Unit::Px),
                    UnitConvertor(size.height(), unit, Unit::Px));

    if (m_sceneData != nullptr)
    {
        m_sceneData->RefreshSheetSize();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetSize() const -> QSizeF
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetSizeConverted() const -> QSizeF
{
    Unit unit = SheetUnits();
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_size.width(), Unit::Px, unit),
                UnitConvertor(m_size.height(), Unit::Px, unit)
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    Unit unit = SheetUnits();
    m_margins.setLeft(UnitConvertor(left, unit, Unit::Px));
    m_margins.setTop(UnitConvertor(top, unit, Unit::Px));
    m_margins.setRight(UnitConvertor(right, unit, Unit::Px));
    m_margins.setBottom(UnitConvertor(bottom, unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMargins(const QMarginsF &margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMarginsConverted(const QMarginsF &margins)
{
    Unit unit = SheetUnits();
    m_margins = UnitConvertor(margins, unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetMargins() const -> QMarginsF
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetMarginsConverted() const -> QMarginsF
{
    Unit unit = SheetUnits();
    return UnitConvertor(m_margins, Unit::Px, unit);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::IgnoreMargins() const -> bool
{
    return m_ignoreMargins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetIgnoreMargins(bool newIgnoreMargins)
{
    m_ignoreMargins = newIgnoreMargins;
}
