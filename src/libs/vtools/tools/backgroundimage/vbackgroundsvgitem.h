/************************************************************************
 **
 **  @file   vbackgroundsvgitem.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef VBACKGROUNDSVGITEM_H
#define VBACKGROUNDSVGITEM_H

#include "vbackgroundimageitem.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

class QSvgRenderer;

class VBackgroundSVGItem : public VBackgroundImageItem
{
    Q_OBJECT // NOLINT
public:
    VBackgroundSVGItem(const VBackgroundPatternImage &image, VAbstractPattern *doc, QGraphicsItem *parent = nullptr);
    ~VBackgroundSVGItem() override;

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Tool::BackgroundSVGImage)};

    auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private slots:
    void RepaintItem();

private:
    Q_DISABLE_COPY_MOVE(VBackgroundSVGItem) // NOLINT

    QSvgRenderer *m_renderer{nullptr};

    auto Renderer() const -> QSvgRenderer *;
};

#endif // VBACKGROUNDSVGITEM_H
