/************************************************************************
 **
 **  @file   vposter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 4, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VPOSTER_H
#define VPOSTER_H

#include <QCoreApplication>
#include <QRect>
#include <QtGlobal>
#include <QtCore/qcontainerfwd.h>

#include "../vmisc/def.h"

class QGraphicsItem;
class QPrinter;
struct VWatermarkData;

struct PosterData
{
    quint32 index{0}; // paper index
    quint32 row{0};   // positions in the grid
    quint32 column{0};
    quint32 rows{0};
    quint32 columns{0};
    QRect rect{}; // rect section
};

Q_DECLARE_TYPEINFO(PosterData, Q_MOVABLE_TYPE); // NOLINT

class VPoster
{
    Q_DECLARE_TR_FUNCTIONS(VPoster) // NOLINT
public:
    explicit VPoster(const QPrinter *printer);

    auto Calc(const QSize &imageRect, int page, PageOrientation orientation) const -> QVector<PosterData>;

    auto Tile(QGraphicsItem *parent, const PosterData &img, vsizetype sheets, const VWatermarkData &watermarkData,
              const QString &watermarkPath) const -> QVector<QGraphicsItem *>;

private:
    const QPrinter *printer;
    /**
     * @brief allowance is the width of the strip that holds the tiled
     * grid information and that is used for the gluing.
     */
    quint32 allowance;

    auto CountRows(int height, PageOrientation orientation) const -> int;
    auto CountColumns(int width, PageOrientation orientation) const -> int;

    auto Cut(int i, int j, const QSize &imageRect, PageOrientation orientation) const -> PosterData;

    auto PageRect() const -> QRect;

    void Ruler(QVector<QGraphicsItem *> &data, QGraphicsItem *parent, QRect rec) const;

    auto Borders(QGraphicsItem *parent, const PosterData &img, vsizetype sheets) const -> QVector<QGraphicsItem *>;

    auto TextWatermark(QGraphicsItem *parent, const PosterData &img, const VWatermarkData &watermarkData) const
        -> QVector<QGraphicsItem *>;
    auto ImageWatermark(QGraphicsItem *parent, const PosterData &img, const VWatermarkData &watermarkData,
                        const QString &watermarkPath) const -> QVector<QGraphicsItem *>;
};

#endif // VPOSTER_H
