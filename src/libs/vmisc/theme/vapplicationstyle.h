/************************************************************************
 **
 **  @file   vapplicationstyle.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 7, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VAPPLICATIONSTYLE_H
#define VAPPLICATIONSTYLE_H

#include <QStyle>
#include <QtGlobal>

class VApplicationStyle : public QStyle
{
    Q_OBJECT // NOLINT

public:
    explicit VApplicationStyle(QStyle *style);
    ~VApplicationStyle() override = default;

#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    auto name() const -> QString;
#endif

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const override;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const override;
    auto subElementRect(SubElement subElement, const QStyleOption *option, const QWidget *widget) const
        -> QRect override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *widget) const override;
    auto hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt,
                               const QWidget *widget) const -> SubControl override;
    auto subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const
        -> QRect override;
    auto pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const -> int override;
    auto sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *w) const
        -> QSize override;
    auto styleHint(StyleHint stylehint, const QStyleOption *opt, const QWidget *widget,
                   QStyleHintReturn *returnData) const -> int override;
    auto standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget) const
        -> QPixmap override;
    auto standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
        -> QIcon override;
    auto generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
        -> QPixmap override;
    auto layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                       Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
        -> int override;

private:
    Q_DISABLE_COPY_MOVE(VApplicationStyle) // NOLINT

    QStyle *m_style;

    auto StyleIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const -> QIcon;
    auto StylesheetIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const -> QIcon;
};

#endif // VAPPLICATIONSTYLE_H
