/************************************************************************
 **
 **  @file   vcirculargauge.h
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

#ifndef VCIRCULARGAUGE_H
#define VCIRCULARGAUGE_H

#include <QColor>
#include <QWidget>

/**
 * @brief A 270° dial showing a percentage, filled in a colour that goes from red at 0 % to green at 100 %.
 */
class VCircularGauge : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VCircularGauge(QWidget *parent = nullptr);

    void SetValue(int percent);
    auto Value() const -> int { return m_value; }

    /** @brief Colour The dial's fill for a percentage: hue 0 (red) to 120 (green). */
    static auto Colour(int percent) -> QColor;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_value{0};
};

#endif // VCIRCULARGAUGE_H
