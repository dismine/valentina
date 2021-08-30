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

//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(const VPLayoutPtr &layout) :
    m_layout(layout)
{
    SCASSERT(layout != nullptr)
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
auto VPSheet::GrainlineType() const -> enum GrainlineType
{
    VPLayoutPtr layout = GetLayout();
    if (not layout.isNull())
    {
        QSizeF size =  layout->LayoutSettings().GetSheetSize();
        if (size.height() < size.width())
        {
            return GrainlineType::Horizontal;
        }
    }

    return GrainlineType::Vertical;
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
    return GetSheetRect(GetLayout());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetMarginsRect() const -> QRectF
{
    return GetMarginsRect(GetLayout());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetSheetRect(const VPLayoutPtr &layout) -> QRectF
{
    if (layout.isNull())
    {
        return {};
    }

    QPoint topLeft = QPoint(0,0);
    QSizeF size = layout->LayoutSettings().GetSheetSize();
    QRectF rect = QRectF(topLeft, size);
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetMarginsRect(const VPLayoutPtr &layout) -> QRectF
{
    if (layout.isNull())
    {
        return {};
    }

    QSizeF size = layout->LayoutSettings().GetSheetSize();

    if (not layout->LayoutSettings().IgnoreMargins())
    {
        QMarginsF margins = layout->LayoutSettings().GetSheetMargins();
        QRectF rect = QRectF(QPointF(margins.left(), margins.top()),
                             QPointF(size.width()-margins.right(), size.height()-margins.bottom()));
        return rect;
    }

    return QRectF(0, 0, size.width(), size.height());
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
