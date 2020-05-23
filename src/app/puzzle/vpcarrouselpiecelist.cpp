/************************************************************************
 **
 **  @file   vpcarrouselpiecelist.cpp
 **  @author Ronan Le Tiec
 **  @date   25 4, 2020
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

#include "vpcarrouselpiecelist.h"
#include "vpcarrousel.h"
#include "../vmisc/backport/qoverload.h"

#include <QVBoxLayout>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselLayer, "p.carrouselLayer")

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::VPCarrouselPieceList(VPuzzleLayer *layer, VPCarrousel *carrousel) :
    m_layer(layer),
    m_carrousel(carrousel),
    m_carrouselPieces(QList<VPieceCarrouselPiece*>())
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::~VPCarrouselPieceList()
{
    Clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::Init()
{
    // initiales the structure
    QVBoxLayout *layoutPiecesLayout = new QVBoxLayout();
    layoutPiecesLayout->setMargin(0);
    setLayout(layoutPiecesLayout);

    // then refresh the content
    Refresh();

    // add the connections
    connect(m_layer, &VPuzzleLayer::PieceAdded, this, &VPCarrouselPieceList::on_PieceAdded);
    connect(m_layer, &VPuzzleLayer::PieceRemoved, this, &VPCarrouselPieceList::on_PieceRemoved);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::Refresh()
{
    Clear();

    // Updates the carrousel pieces from the pieces list
    QList<VPuzzlePiece*> pieces = m_layer->GetPieces();

    // sort the pieces in alphabetical order
    std::sort(pieces.begin(), pieces.end(),
          [](const VPuzzlePiece* a, const VPuzzlePiece* b) -> bool { return a->GetName() < b->GetName();});

    // create the corresponding carrousel pieces

    bool _isVisible = isVisible();
    setVisible(true);
    for (auto piece : pieces)
    {
        VPieceCarrouselPiece *carrouselPiece = new VPieceCarrouselPiece(piece, this);
        m_carrouselPieces.append(carrouselPiece);
        layout()->addWidget(carrouselPiece);
        carrouselPiece->CleanPreview(); // fitInView only works if the widget is displayed.
    }
    setVisible(_isVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::Clear()
{
    // Removes and deletes the carrousel pieces from the layer
    while (!m_carrouselPieces.isEmpty())
    {
        VPieceCarrouselPiece *carrouselPiece = m_carrouselPieces.takeLast();

        if(carrouselPiece != nullptr)
        {
            layout()->removeWidget(carrouselPiece);
            delete carrouselPiece;
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
QList<VPieceCarrouselPiece*> VPCarrouselPieceList::GetCarrouselPieces()
{
    return m_carrouselPieces;
}

//---------------------------------------------------------------------------------------------------------------------
VPCarrousel* VPCarrouselPieceList::GetCarrousel()
{
    return m_carrousel;
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer* VPCarrouselPieceList::GetLayer()
{
    return m_layer;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_PieceAdded(VPuzzlePiece* piece)
{
    Q_UNUSED(piece)

    // TODO/ FIXME: see if we find a solution more efficient refreshing the complete layout everytime.

    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_PieceRemoved(VPuzzlePiece* piece)
{
    for (auto carrouselPiece : m_carrouselPieces)
    {
        if(carrouselPiece->GetPiece() == piece)
        {
            m_carrouselPieces.removeAll(carrouselPiece);
            layout()->removeWidget(carrouselPiece);
            delete carrouselPiece;

            return;
        }
    }
}
