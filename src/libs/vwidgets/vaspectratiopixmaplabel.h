/************************************************************************
 **
 **  @file   vaspectratiopixmaplabel.h
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
#ifndef VASPECTRATIOPIXMAPLABEL_H
#define VASPECTRATIOPIXMAPLABEL_H

#include <QLabel>

class VAspectRatioPixmapLabel : public QLabel
{
    Q_OBJECT // NOLINT

public:
    explicit VAspectRatioPixmapLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit VAspectRatioPixmapLabel(const QString &text, QWidget *parent = nullptr,
                                     Qt::WindowFlags f = Qt::WindowFlags());
    ~VAspectRatioPixmapLabel() override = default;

    auto heightForWidth(int width) const -> int override;
    auto sizeHint() const -> QSize override;

    auto ScaledPixmap() const -> QPixmap;

public slots:
    void setPixmap(const QPixmap &p);
    void resizeEvent(QResizeEvent *e) override;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(VAspectRatioPixmapLabel) // NOLINT
    QPixmap m_pix{};
};

#endif // VASPECTRATIOPIXMAPLABEL_H
