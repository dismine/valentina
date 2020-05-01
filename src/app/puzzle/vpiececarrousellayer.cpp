/************************************************************************
 **
 **  @file   vpiececarrousellayer.cpp
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

#include "vpiececarrousellayer.h"

#include <QVBoxLayout>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselLayer, "p.carrouselLayer")

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrouselLayer::VPieceCarrouselLayer(VPuzzleLayer *layer, QWidget *parent) : QWidget(parent), m_layer(layer)
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrouselLayer::~VPieceCarrouselLayer()
{
    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrouselLayer::Init()
{
    // initiales the structure
    QVBoxLayout *layoutPiecesLayout = new QVBoxLayout();
    layoutPiecesLayout->setMargin(0);
    setLayout(layoutPiecesLayout);

    // then refresh the content
    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrouselLayer::Refresh()
{
    // remove the existing carrousel pieces
    // TODO

    // Updates the carrousel pieces from the pieces list
    QList<VPuzzlePiece*> pieces = m_layer->GetPieces();

    // sort the pieces in alphabetical order
    std::sort(pieces.begin(), pieces.end(),
          [](const VPuzzlePiece* a, const VPuzzlePiece* b) -> bool { return a->GetName() < b->GetName();});

    // create the corresponding carrousel pieces

    for (auto piece : pieces)
    {
//        qCDebug(pCarrouselLayer, "piece name : %s", piece->GetName().toStdString().c_str());

        VPieceCarrouselPiece *carrouselPiece = new VPieceCarrouselPiece(piece);
        m_carrouselPieces.append(carrouselPiece);
        layout()->addWidget(carrouselPiece);

        // FIXME? the fitInView inside the refresh of the piece doesn't workd properly.
        // only by doing the following I did get it to work:
        setVisible(true);
        carrouselPiece->CleanPreview();
        setVisible(false);

        connect(carrouselPiece, QOverload<VPieceCarrouselPiece*>::of(&VPieceCarrouselPiece::clicked), this,
                        &VPieceCarrouselLayer::on_PieceClicked);

    }
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPieceCarrouselPiece*> VPieceCarrouselLayer::GetCarrouselPieces()
{
    return m_carrouselPieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrouselLayer::on_PieceClicked(VPieceCarrouselPiece* carrouselPiece)
{
    emit pieceClicked(carrouselPiece);
}
