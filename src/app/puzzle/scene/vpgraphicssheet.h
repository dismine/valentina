/************************************************************************
 **
 **  @file   vpgraphicssheet.h
 **  @author Ronan Le Tiec
 **  @date   3 5, 2020
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

#ifndef VPGRAPHICSSHEET_H
#define VPGRAPHICSSHEET_H

#include <QCoreApplication>
#include <QGraphicsItem>
#include <QPainter>

#include "../layout/layoutdef.h"

class VPLayout;

class VPGraphicsSheet : public QGraphicsItem
{
    Q_DECLARE_TR_FUNCTIONS(VPGraphicsSheet) // NOLINT

public:
    explicit VPGraphicsSheet(const VPLayoutPtr &layout, QGraphicsItem *parent = nullptr);
    ~VPGraphicsSheet() override = default;

    auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    auto GetSheetRect() const -> QRectF;
    auto GetMarginsRect() const -> QRectF;

    /**
     * @brief SetShowMargin Sets Wether we see the margin
     * @param value true to show the margin
     */
    void SetShowMargin(bool value);

    /**
     * @brief SetShowBorder Sets whether we see the border of the sheet
     * @param value true to show the border
     */
    void SetShowBorder(bool value);

    void RefreshBoundingRect();

    static auto FoldText() -> QString;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPGraphicsSheet) // NOLINT

    VPLayoutWeakPtr m_layout{};

    bool m_showMargin{true};
    bool m_showBorder{true};

    void PaintVerticalFold(QPainter *painter, const QRectF &sheetRect) const;
    void PaintHorizontalFold(QPainter *painter, const QRectF &sheetRect) const;

    void PaintMargins(QPainter *painter) const;
    void PaintBorder(QPainter *painter) const;
    void PaintFold(QPainter *painter) const;
    void PaintGrid(QPainter *painter) const;

    auto FoldField(const QRectF &sheetRect) const -> QRectF;
};

#endif // VPGRAPHICSSHEET_H
