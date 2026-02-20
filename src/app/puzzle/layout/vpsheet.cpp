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

#include "../scene/vpgraphicspiece.h"
#include "../scene/vpgraphicspiececontrols.h"
#include "../scene/vpgraphicssheet.h"
#include "../scene/vpgraphicstilegrid.h"
#include "../vpapplication.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "theme/vscenestylesheet.h"
#include "theme/vstylesheetstyle.h"
#include "vplayout.h"
#include "vppiece.h"

#include <QtConcurrent>

namespace
{
struct VSheetPiece
{
    QString id{};
    bool showFullPiece{false};
    QLineF seamMirrorLine{};
    QLineF seamAllowanceMirrorLine{};
    QVector<QPointF> externalContourPoints{};
};

struct VPiecesValidationData
{
    bool warnPiecesOutOfBound{false};
    bool warnSuperpositionOfPieces{false};
    bool warnPieceGapePosition{false};
    bool cutOnFold{false};
    QRectF sheetRect{};
    qreal pieceGap{0};
    QVector<VSheetPiece> pieces{};
};

//---------------------------------------------------------------------------------------------------------------------
void ValidatePiecesOutOfBound(const VPiecesValidationData &data, QHash<QString, VPiecePositionValidity> &validations)
{
    for (const auto &piece : data.pieces)
    {
        VPiecePositionValidity validation = validations.value(piece.id);

        if (data.cutOnFold && not piece.showFullPiece && !piece.seamMirrorLine.isNull())
        {
            QLineF const foldLine = data.sheetRect.width() >= data.sheetRect.height()
                                        ? QLineF(data.sheetRect.topLeft(), data.sheetRect.topRight())
                                        : QLineF(data.sheetRect.topRight(), data.sheetRect.bottomRight());

            validation.outOfBound = not IsLineSegmentOnLineSegment(foldLine,
                                                                   piece.seamAllowanceMirrorLine,
                                                                   MmToPixel(0.5));
        }
        else
        {
            QRectF const pieceRect = VLayoutPiece::BoundingRect(piece.externalContourPoints);
            validation.outOfBound = not data.sheetRect.contains(pieceRect);
        }

        validations.insert(piece.id, validation);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ValidateSuperpositionOfPieces(const VPiecesValidationData &data,
                                   QHash<QString, VPiecePositionValidity> &validations)
{
    QSet<QString> invalidPieces;
    invalidPieces.reserve(data.pieces.size());

    for (const auto &piece : data.pieces)
    {
        if (invalidPieces.contains(piece.id))
        {
            continue;
        }

        bool hasSuperposition = false;
        VSheetPiece invalidPiece;

        for (const auto &p : data.pieces)
        {
            if (piece.id == p.id)
            {
                continue;
            }

            if (VPPiece::PathsSuperposition(piece.externalContourPoints, p.externalContourPoints))
            {
                hasSuperposition = true;
                invalidPiece = p;
                break;
            }
        }

        auto UpdateValidity = [&validations, hasSuperposition, &invalidPieces](const QString &id) -> void
        {
            VPiecePositionValidity validation = validations.value(id);
            validation.superposition = hasSuperposition;
            validations.insert(id, validation);
            if (hasSuperposition)
            {
                invalidPieces.insert(id);
            }
        };

        UpdateValidity(piece.id);

        if (!invalidPiece.id.isEmpty())
        {
            UpdateValidity(invalidPiece.id);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ValidatePiecesGapePosition(const VPiecesValidationData &data, QHash<QString, VPiecePositionValidity> &validations)
{
    if (data.pieceGap <= 0)
    {
        return;
    }

    QSet<QString> invalidPieces;
    invalidPieces.reserve(data.pieces.size());

    for (const auto &piece : data.pieces)
    {
        if (invalidPieces.contains(piece.id))
        {
            continue;
        }

        QRectF const path1Rect = VLayoutPiece::BoundingRect(piece.externalContourPoints)
                                     .adjusted(-data.pieceGap, -data.pieceGap, data.pieceGap, data.pieceGap);
        QVector<QPointF> const path1 = VPPiece::PrepareStickyPath(piece.externalContourPoints);
        bool hasInvalidPieceGapPosition = false;
        VSheetPiece invalidPiece;

        for (const auto &p : data.pieces)
        {
            if (piece.id == p.id)
            {
                continue;
            }

            if (QRectF const path2Rect = VLayoutPiece::BoundingRect(p.externalContourPoints)
                                             .adjusted(-data.pieceGap, -data.pieceGap, data.pieceGap, data.pieceGap);
                !path1Rect.intersects(path2Rect) && !path1Rect.contains(path2Rect) && !path2Rect.contains(path1Rect))
            {
                continue;
            }

            QVector<QPointF> const path2 = VPPiece::PrepareStickyPath(p.externalContourPoints);

            QLineF const distance = VPPiece::ClosestDistance(path1, path2);

            if (distance.length() < data.pieceGap - accuracyPointOnLine)
            {
                hasInvalidPieceGapPosition = true;
                invalidPiece = p;
                break;
            }
        }

        auto UpdateValidity = [&validations, hasInvalidPieceGapPosition, &invalidPieces](const QString &id) -> void
        {
            VPiecePositionValidity validation = validations.value(id);
            validation.gap = hasInvalidPieceGapPosition;
            validations.insert(id, validation);
            if (hasInvalidPieceGapPosition)
            {
                invalidPieces.insert(id);
            }
        };

        UpdateValidity(piece.id);

        if (!invalidPiece.id.isEmpty())
        {
            UpdateValidity(invalidPiece.id);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ValidatePiecesPositions(const VPiecesValidationData &data) -> QHash<QString, VPiecePositionValidity>
{
    QHash<QString, VPiecePositionValidity> validations;

    if (data.warnPiecesOutOfBound)
    {
        ValidatePiecesOutOfBound(data, validations);
    }

    if (data.warnSuperpositionOfPieces)
    {
        ValidateSuperpositionOfPieces(data, validations);
    }

    if (data.warnPieceGapePosition)
    {
        ValidatePiecesGapePosition(data, validations);
    }

    return validations;
}
} // namespace

// VPSheetSceneData
//---------------------------------------------------------------------------------------------------------------------
VPSheetSceneData::VPSheetSceneData(const VPLayoutPtr &layout, const QUuid &sheetUuid)
  : m_layout(layout),
    m_scene(QSharedPointer<VMainGraphicsScene>::create()),
    m_sheetUuid(sheetUuid)
{
    SCASSERT(not layout.isNull())

    m_graphicsSheet = new VPGraphicsSheet(layout);
    m_graphicsSheet->setPos(0, 0);
    m_scene->addItem(m_graphicsSheet);

    m_graphicsTileGrid = new VPGraphicsTileGrid(layout, m_sheetUuid);
    m_graphicsTileGrid->setZValue(10);
    m_scene->addItem(m_graphicsTileGrid);

    m_rotationControls = new VPGraphicsPieceControls(layout);
    m_scene->addItem(m_rotationControls);

    m_rotationOrigin = new VPGraphicsTransformationOrigin(layout);
    m_rotationOrigin->setVisible(false);
    m_scene->addItem(m_rotationOrigin);

    QObject::connect(m_rotationControls, &VPGraphicsPieceControls::ShowOrigin, m_rotationOrigin,
                     &VPGraphicsTransformationOrigin::on_ShowOrigin);
    QObject::connect(m_rotationControls, &VPGraphicsPieceControls::TransformationOriginChanged, m_rotationOrigin,
                     &VPGraphicsTransformationOrigin::SetTransformationOrigin);
}

//---------------------------------------------------------------------------------------------------------------------
VPSheetSceneData::~VPSheetSceneData() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VPSheetSceneData::Scene() const -> QSharedPointer<VMainGraphicsScene>
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
void VPSheetSceneData::RefreshPieces(bool printMode)
{
    qDeleteAll(m_graphicsPieces);
    m_graphicsPieces.clear();

    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr const sheet = layout->GetSheet(m_sheetUuid);
    if (sheet.isNull())
    {
        return;
    }

    QList<VPPiecePtr> const pieces = sheet->GetPieces();
    m_graphicsPieces.reserve(pieces.size());

    for (const auto &piece : pieces)
    {
        if (piece.isNull())
        {
            continue;
        }

        auto *graphicsPiece = new VPGraphicsPiece(piece);
        graphicsPiece->SetPrintMode(printMode);
        m_graphicsPieces.append(graphicsPiece);
        m_scene->addItem(graphicsPiece);

        // Restore selection state
        if (piece->IsSelected())
        {
            graphicsPiece->setSelected(piece->IsSelected());
        }

        ConnectPiece(graphicsPiece);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::PrepareForExport(LayoutExportFormats format)
{
    VStylesheetStyle::SetExportColorScheme(ExportColorScheme::BlackAndWhite);
    VSceneStylesheet::ResetStyles();

    const bool printMode = true;

    m_graphicsSheet->SetShowBorder(false);
    m_graphicsSheet->SetShowMargin(false);
    m_graphicsSheet->SetPrintMode(printMode);
    m_graphicsSheet->SetPDFTiledExport(format == LayoutExportFormats::PDFTiled);

    m_rotationControls->setVisible(false);
    m_rotationOrigin->setVisible(false);

    if (VPLayoutPtr const layout = m_layout.toStrongRef(); not layout.isNull())
    {
        m_showGridTmp = layout->LayoutSettings().GetShowGrid();
        layout->LayoutSettings().SetShowGrid(false);

        m_showTilesTmp = layout->LayoutSettings().GetShowTiles();
        layout->LayoutSettings().SetShowTiles(false);

        VPSheetPtr const sheet = layout->GetSheet(m_sheetUuid);
        m_slectedPiecesTmp = sheet->GetSelectedPieces();

        for (const auto &piece : std::as_const(m_slectedPiecesTmp))
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

        m_pieceGapePositionTmp = layout->LayoutSettings().GetWarningPieceGapePosition();
        layout->LayoutSettings().SetWarningPieceGapePosition(false);
    }

    RefreshPieces(printMode);
    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::CleanAfterExport()
{
    VStylesheetStyle::SetExportColorScheme(ExportColorScheme::Default);
    VSceneStylesheet::ResetStyles();

    const bool printMode = false;

    m_graphicsSheet->SetShowBorder(true);
    m_graphicsSheet->SetShowMargin(true);
    m_graphicsSheet->SetPrintMode(printMode);
    m_graphicsSheet->SetPDFTiledExport(false);

    m_rotationControls->setVisible(true);

    if (VPLayoutPtr const layout = m_layout.toStrongRef(); not layout.isNull())
    {
        layout->LayoutSettings().SetShowGrid(m_showGridTmp);
        layout->LayoutSettings().SetShowTiles(m_showTilesTmp);

        for (const auto &piece : std::as_const(m_slectedPiecesTmp))
        {
            if (not piece.isNull())
            {
                piece->SetSelected(true);
                emit layout->PieceSelectionChanged(piece);
            }
        }

        layout->LayoutSettings().SetWarningPiecesOutOfBound(m_outOfBoundTmp);
        layout->LayoutSettings().SetWarningSuperpositionOfPieces(m_pieceSuperpositionTmp);
        layout->LayoutSettings().SetWarningPieceGapePosition(m_pieceGapePositionTmp);
    }

    RefreshPieces(printMode);
    RefreshLayout();
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

    for (auto *item : m_graphicsPieces)
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
    if (auto _graphicsPiece =
            std::find_if(m_graphicsPieces.begin(), m_graphicsPieces.end(),
                         [piece](VPGraphicsPiece *graphicPiece) { return graphicPiece->GetPiece() == piece; });
        _graphicsPiece != m_graphicsPieces.end())
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
void VPSheetSceneData::SetTextAsPaths(bool textAsPaths)
{
    for (auto *piece : std::as_const(m_graphicsPieces))
    {
        if (piece != nullptr)
        {
            piece->SetTextAsPaths(textAsPaths);
        }
    }

    m_textAsPaths = textAsPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::PrepareTilesScheme()
{
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); not layout.isNull())
    {
        m_showTilesSchemeTmp = layout->LayoutSettings().GetShowTiles();
        layout->LayoutSettings().SetShowTiles(true);

        m_showTilesWatermarkSchemeTmp = layout->LayoutSettings().GetShowWatermark();
        layout->LayoutSettings().SetShowWatermark(false);
    }

    m_graphicsTileGrid->SetPrintMode(true);
    m_graphicsTileGrid->SetTextAsPaths(m_textAsPaths);

    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheetSceneData::ClearTilesScheme()
{
    if (VPLayoutPtr const layout = m_layout.toStrongRef(); not layout.isNull())
    {
        layout->LayoutSettings().SetShowTiles(m_showTilesSchemeTmp);
        layout->LayoutSettings().SetShowWatermark(m_showTilesWatermarkSchemeTmp);
    }

    m_graphicsTileGrid->SetPrintMode(false);
    m_graphicsTileGrid->SetTextAsPaths(false);

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
void VPSheetSceneData::ConnectPiece(VPGraphicsPiece *piece) const
{
    SCASSERT(piece != nullptr)

    VPLayoutPtr const layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    QObject::connect(layout.data(), &VPLayout::PieceTransformationChanged, piece, &VPGraphicsPiece::on_RefreshPiece);
    QObject::connect(layout.data(), &VPLayout::PieceZValueChanged, piece, &VPGraphicsPiece::PieceZValueChanged);
    QObject::connect(layout.data(), &VPLayout::PieceSelectionChanged, m_rotationControls,
                     &VPGraphicsPieceControls::on_UpdateControls, Qt::UniqueConnection);
    QObject::connect(layout.data(), &VPLayout::PiecePositionValidityChanged, piece, &VPGraphicsPiece::on_RefreshPiece);
    QObject::connect(piece, &VPGraphicsPiece::PieceTransformationChanged, m_rotationControls,
                     &VPGraphicsPieceControls::on_UpdateControls);
    QObject::connect(piece, &VPGraphicsPiece::HideTransformationHandles, m_rotationControls,
                     &VPGraphicsPieceControls::on_HideHandles);
    QObject::connect(piece, &VPGraphicsPiece::HideTransformationHandles, m_rotationOrigin,
                     &VPGraphicsTransformationOrigin::on_HideHandles);
    QObject::connect(layout.data(), &VPLayout::BoundaryTogetherWithNotchesChanged, piece,
                     &VPGraphicsPiece::on_RefreshPiece);
}

// VPSheet
//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(const VPLayoutPtr &layout, QObject *parent)
  : QObject(parent),
    m_layout(layout),
    m_sceneData(QSharedPointer<VPSheetSceneData>::create(layout, Uuid())),
    m_validityWatcher(new QFutureWatcher<QHash<QString, VPiecePositionValidity>>(this))
{
    SCASSERT(not layout.isNull())

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    SetIgnoreMargins(settings->GetLayoutSheetIgnoreMargins());
    SetSheetMargins(settings->GetLayoutSheetMargins());
    SetSheetSize(QSizeF(settings->GetLayoutSheetPaperWidth(), settings->GetLayoutSheetPaperHeight()));

    connect(qApp, &QCoreApplication::aboutToQuit, m_validityWatcher,
            [this]()
            {
                m_validityWatcher->cancel();
                m_validityWatcher->waitForFinished();
            });
    connect(m_validityWatcher, &QFutureWatcher<QHash<QString, VPiecePositionValidity>>::finished, this,
            &VPSheet::UpdatePiecesValidity);
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet::~VPSheet()
{
    m_validityWatcher->cancel();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetLayout() const -> VPLayoutPtr
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetPieces() const -> QList<VPPiecePtr>
{
    if (VPLayoutPtr const layout = GetLayout(); not layout.isNull())
    {
        return layout->PiecesForSheet(m_uuid);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSelectedPieces() const -> QList<VPPiecePtr>
{
    if (VPLayoutPtr const layout = GetLayout(); not layout.isNull())
    {
        QList<VPPiecePtr> const list = layout->PiecesForSheet(m_uuid);

        QList<VPPiecePtr> selected;
        selected.reserve(list.size());

        for (const auto &piece : list)
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
    QList<VPPiecePtr> const pieces = GetPieces();

    QVector<VLayoutPiece> details;
    details.reserve(pieces.size());

    for (const auto &piece : pieces)
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
auto VPSheet::GetSheetRect() const -> QRectF
{
    return {QPoint(0, 0), m_size};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetMarginsRect() const -> QRectF
{
    if (not m_ignoreMargins)
    {
        auto rect = QRectF(QPointF(m_margins.left(), m_margins.top()),
                           QPointF(m_size.width() - m_margins.right(), m_size.height() - m_margins.bottom()));
        return rect;
    }

    return {0, 0, m_size.width(), m_size.height()};
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::RemoveUnusedLength()
{
    VPLayoutPtr const layout = GetLayout();
    if (layout.isNull())
    {
        return;
    }

    QList<VPPiecePtr> const pieces = GetPieces();
    if (pieces.isEmpty())
    {
        return;
    }

    QRectF piecesBoundingRect;

    for (const auto &piece : pieces)
    {
        if (not piece.isNull())
        {
            piece->SetSelected(false);
            emit layout->PieceSelectionChanged(piece);
            piecesBoundingRect = piecesBoundingRect.united(piece->MappedDetailBoundingRect());
        }
    }

    const qreal extra = 2;
    QRectF const sheetRect = GetSheetRect();
    GrainlineType const type = GrainlineOrientation();

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
void VPSheet::CheckPiecesPositionValidity() const
{
    if (m_validityWatcher->isFinished())
    {
        VPLayoutPtr const layout = GetLayout();
        if (layout.isNull())
        {
            return;
        }

        QList<VPPiecePtr> const pieces = GetPieces();
        QVector<VSheetPiece> sheetPieces;
        sheetPieces.reserve(pieces.size());

        for (const auto &piece : pieces)
        {
            QVector<QPointF> points;
            CastTo(piece->GetMappedExternalContourPoints(), points);
            sheetPieces.append({.id = piece->GetUniqueID(),
                                .showFullPiece = piece->IsShowFullPiece(),
                                .seamMirrorLine = piece->GetMappedSeamMirrorLine(),
                                .seamAllowanceMirrorLine = piece->GetMappedSeamAllowanceMirrorLine(),
                                .externalContourPoints = points});
        }

        const VPiecesValidationData data
            = {.warnPiecesOutOfBound = layout->LayoutSettings().GetWarningPiecesOutOfBound(),
               .warnSuperpositionOfPieces = layout->LayoutSettings().GetWarningSuperpositionOfPieces(),
               .warnPieceGapePosition = layout->LayoutSettings().GetWarningPieceGapePosition(),
               .cutOnFold = layout->LayoutSettings().IsCutOnFold(),
               .sheetRect = GetMarginsRect(),
               .pieceGap = layout->LayoutSettings().GetPiecesGap(),
               .pieces = sheetPieces};

        m_validationStale = false;
        m_validityWatcher->setFuture(QtConcurrent::run([data]() -> QHash<QString, VPiecePositionValidity>
                                                       { return ValidatePiecesPositions(data); }));
    }
    else
    {
        m_validationStale = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::UpdatePiecesValidity()
{
    if (m_validityWatcher->isCanceled())
    {
        return;
    }

    QHash<QString, VPiecePositionValidity> const newValidations = m_validityWatcher->future().result();

    QList<VPPiecePtr> const pieces = GetPieces();
    QHash<QString, VPPiecePtr> sortedPieces;
    sortedPieces.reserve(pieces.size());

    for (const auto &piece : pieces)
    {
        sortedPieces.insert(piece->GetUniqueID(), piece);
    }

    VPLayoutPtr const layout = GetLayout();
    if (layout.isNull())
    {
        return;
    }

    for (auto i = newValidations.cbegin(), end = newValidations.cend(); i != end; ++i)
    {
        if (sortedPieces.contains(i.key()))
        {
            VPPiecePtr const piece = sortedPieces.value(i.key());
            piece->SetOutOfBound(i.value().outOfBound);
            piece->SetHasSuperpositionWithPieces(i.value().superposition);
            piece->SetHasInvalidPieceGapPosition(i.value().gap);

            emit layout->PiecePositionValidityChanged(piece);
        }
    }

    if (m_validationStale)
    {
        m_validationStale = false;
        CheckPiecesPositionValidity();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::SceneData() const -> QSharedPointer<VPSheetSceneData>
{
    return m_sceneData;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ClearSelection() const
{
    QList<VPPiecePtr> const selectedPieces = GetSelectedPieces();
    for (const auto &piece : selectedPieces)
    {
        if (piece->IsSelected())
        {
            piece->SetSelected(false);
        }
    }

    if (not selectedPieces.isEmpty())
    {
        VPLayoutPtr const layout = GetLayout();
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
    if (VPLayoutPtr const layout = GetLayout(); not layout.isNull())
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
    Unit const unit = SheetUnits();
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
    Unit const unit = SheetUnits();
    m_size = QSizeF(UnitConvertor(size.width(), unit, Unit::Px), UnitConvertor(size.height(), unit, Unit::Px));

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
    Unit const unit = SheetUnits();
    auto convertedSize =
        QSizeF(UnitConvertor(m_size.width(), Unit::Px, unit), UnitConvertor(m_size.height(), Unit::Px, unit));

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
    Unit const unit = SheetUnits();
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
    Unit const unit = SheetUnits();
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
    Unit const unit = SheetUnits();
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
