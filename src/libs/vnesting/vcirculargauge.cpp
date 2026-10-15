/************************************************************************
 **
 **  @file   vcirculargauge.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 10, 2026
 **
 **  @brief  A round percentage dial whose colour goes from red (0 %) to green (100 %).
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "vcirculargauge.h"

#include <QPainter>

namespace
{
constexpr int span = 270;      // degrees of dial
constexpr int start = 225;     // 12 o'clock minus span/2, Qt angles grow counter-clockwise
constexpr qreal stroke = 0.12; // pen width as a share of the dial size
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VCircularGauge::VCircularGauge(QWidget *parent)
  : QWidget(parent)
{
    setFixedSize(96, 96);
}

//---------------------------------------------------------------------------------------------------------------------
void VCircularGauge::SetValue(int percent)
{
    m_value = qBound(0, percent, 100);
    update();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCircularGauge::Colour(int percent) -> QColor
{
    return QColor::fromHsv(qBound(0, percent, 100) * 120 / 100, 200, 220);
}

//---------------------------------------------------------------------------------------------------------------------
void VCircularGauge::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const qreal side = qMin(width(), height());
    const qreal pen = side * stroke;
    const QRectF dial(pen / 2, pen / 2, side - pen, side - pen);

    QPen track(palette().color(QPalette::Mid), pen, Qt::SolidLine, Qt::FlatCap);
    painter.setPen(track);
    painter.drawArc(dial, start * 16, -span * 16);

    if (m_value > 0)
    {
        track.setColor(Colour(m_value));
        track.setCapStyle(Qt::RoundCap);
        painter.setPen(track);
        painter.drawArc(dial, start * 16, -span * m_value / 100 * 16);
    }

    painter.setPen(palette().color(QPalette::WindowText));
    QFont font = painter.font();
    font.setPointSizeF(font.pointSizeF() * 1.4);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(dial, Qt::AlignCenter, QStringLiteral("%1 %").arg(m_value));
}
