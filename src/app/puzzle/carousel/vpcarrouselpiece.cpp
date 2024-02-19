/************************************************************************
 **
 **  @file   vpcarrouselpiece.cpp
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

#include "vpcarrouselpiece.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>

#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../vmisc/theme/vscenestylesheet.h"

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pCarrouselPiece, "p.carrouselPiece") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPiece::VPCarrouselPiece(const VPPiecePtr &piece, QListWidget *parent)
  : QListWidgetItem(parent, Type),
    m_piece(piece)
{
    SCASSERT(m_piece != nullptr)
    const int width = 120 - 8;
    QString const clippedText = QFontMetrics(font()).elidedText(piece->GetName(), Qt::ElideRight, width);
    RefreshPieceIcon();
    setText(clippedText);
    setToolTip(piece->GetName());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPCarrouselPiece::GetPiece() const -> VPPiecePtr
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::RefreshSelection()
{
    VPPiecePtr const piece = GetPiece();
    if (not piece.isNull())
    {
        setSelected(piece->IsSelected());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::RefreshPieceIcon()
{
    setIcon(CreatePieceIcon(QSize(120, 120)));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPCarrouselPiece::CreatePieceIcon(const QSize &size, bool isDragIcon) const -> QIcon
{
    VPPiecePtr const piece = GetPiece();
    if (piece.isNull())
    {
        return {};
    }

    QRectF const boundingRect = piece->DetailBoundingRect();
    qreal const canvasSize = qMax(boundingRect.height(), boundingRect.width());
    QRectF const canvas = QRectF(0, 0, canvasSize, canvasSize);

    qreal const dx = canvas.center().x() - boundingRect.center().x();
    qreal const dy = canvas.center().y() - boundingRect.center().y();

    QVector<QIcon::Mode> iconModes;
    iconModes.append(QIcon::Normal);

    if (not isDragIcon)
    {
        iconModes.append(QIcon::Selected);
    }

    QIcon icon;

    const VManualLayoutStyle &style = VSceneStylesheet::ManualLayoutStyle();

    for (auto iconMode : iconModes)
    {
        QPixmap pixmap(size);
        pixmap.fill(not isDragIcon ? style.CarrouselPieceBackgroundColor() : QColor(Qt::transparent));

        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        int const spacing = 2;

        painter.translate(spacing, spacing);

        qreal const scaleFactorX = canvasSize * 100 / (size.width() - spacing * 2) / 100;
        qreal const scaleFactorY = canvasSize * 100 / (size.height() - spacing * 2) / 100;
        painter.scale(1. / scaleFactorX, 1. / scaleFactorY);
        painter.setPen(QPen(style.CarrouselPieceColor(), 0.8 * qMax(scaleFactorX, scaleFactorY)));

        if (not isDragIcon)
        {
            painter.translate(dx, dy);
        }
        else
        {
            painter.translate(-boundingRect.topLeft().x() + spacing, -boundingRect.topLeft().y() + spacing);
        }

        painter.setBrush(QBrush(iconMode == QIcon::Selected ? style.CarrouselPieceSelectedColor()
                                                            : style.CarrouselPieceForegroundColor()));

        bool togetherWithNotches = false;
        VPLayoutPtr const pieceLayout = piece->Layout();
        if (not pieceLayout.isNull())
        {
            togetherWithNotches = pieceLayout->LayoutSettings().IsBoundaryTogetherWithNotches();
        }

        piece->DrawMiniature(painter, togetherWithNotches);

        painter.end();

        icon.addPixmap(pixmap, iconMode);
    }

    return icon;
}
