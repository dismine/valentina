/************************************************************************
 **
 **  @file   vaspectratiopixmaplabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2023
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
#include "vaspectratiopixmaplabel.h"

//---------------------------------------------------------------------------------------------------------------------
VAspectRatioPixmapLabel::VAspectRatioPixmapLabel(QWidget *parent, Qt::WindowFlags f)
  : QLabel(parent, f)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
}

//---------------------------------------------------------------------------------------------------------------------
VAspectRatioPixmapLabel::VAspectRatioPixmapLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
  : QLabel(text, parent, f)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAspectRatioPixmapLabel::setPixmap(const QPixmap &p)
{
    m_pix = p;
    QLabel::setPixmap(ScaledPixmap());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAspectRatioPixmapLabel::heightForWidth(int width) const -> int
{
    if (m_pix.isNull())
    {
        return QLabel::heightForWidth(width);
    }

    return static_cast<int>((static_cast<qreal>(m_pix.height()) * width) / m_pix.width());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAspectRatioPixmapLabel::sizeHint() const -> QSize
{
    if (m_pix.isNull())
    {
        return QLabel::sizeHint();
    }

    int const w = m_pix.width();
    return {w, heightForWidth(w)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAspectRatioPixmapLabel::ScaledPixmap() const -> QPixmap
{
    if (m_pix.isNull())
    {
        return m_pix;
    }

    auto scaled = m_pix.scaled(size() * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(devicePixelRatioF());
    return scaled;
}

//---------------------------------------------------------------------------------------------------------------------
void VAspectRatioPixmapLabel::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)

    if (!m_pix.isNull())
    {
        QLabel::setPixmap(ScaledPixmap());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAspectRatioPixmapLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit clicked();
}
