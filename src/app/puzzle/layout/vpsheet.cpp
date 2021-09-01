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

//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(const VPLayoutPtr &layout) :
    m_layout(layout)
{
    SCASSERT(layout != nullptr)

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
GrainlineType VPSheet::GrainlineOrientation() const
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
    m_name.clear();
    m_visible = true;
    m_transformationOrigin = VPTransformationOrigon();
    m_trashSheet = false;
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
        QVector<QPointF> path1 = piece->GetMappedExternalContourPoints();
        bool hasSuperposition = false;

        for (const auto &p : pieces)
        {
            if (p.isNull() || piece == p)
            {
                continue;
            }

            QVector<QPointF> path2 = p->GetMappedExternalContourPoints();

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
    return QRectF(QPoint(0, 0), m_size);
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

    return QRectF(0, 0, m_size.width(), m_size.height());
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
Unit VPSheet::SheetUnits() const
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
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(qreal width, qreal height)
{
    Unit unit = SheetUnits();
    m_size.setWidth(UnitConvertor(width, unit, Unit::Px));
    m_size.setHeight(UnitConvertor(height, unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(const QSizeF &size)
{
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(const QSizeF &size)
{
    Unit unit = SheetUnits();
    m_size = QSizeF(UnitConvertor(size.width(), unit, Unit::Px),
                    UnitConvertor(size.height(), unit, Unit::Px));
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
