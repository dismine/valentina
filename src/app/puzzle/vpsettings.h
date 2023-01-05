/************************************************************************
 **
 **  @file   vpsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
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
#ifndef VPSETTINGS_H
#define VPSETTINGS_H

#include <QByteArray>

#include "../vmisc/vcommonsettings.h"

class VPSettings : public VCommonSettings
{
    Q_OBJECT // NOLINT
public:
    VPSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
                    QObject *parent = nullptr);
    VPSettings(const QString &fileName, Format format, QObject *parent = nullptr);

    auto IsDockWidgetPropertiesActive() const -> bool;
    static auto GetDefDockWidgetPropertiesActive() -> bool;
    void SetDockWidgetPropertiesActive(bool value);

    auto IsDockWidgetPropertiesContentsActive() const -> bool;
    static auto GetDefDockWidgetPropertiesContentsActive() -> bool;
    void SetDockWidgetPropertiesContentsActive(bool value);

    void SetLayoutUnit(Unit unit);
    auto LayoutUnit() const -> Unit;

    void SetLayoutSheetPaperWidth(qreal width);
    auto GetLayoutSheetPaperWidth() const -> qreal;

    void SetLayoutSheetPaperHeight(qreal height);
    auto GetLayoutSheetPaperHeight() const -> qreal;

    void SetLayoutTilePaperWidth(qreal width);
    auto GetLayoutTilePaperWidth() const -> qreal;

    void SetLayoutTilePaperHeight(qreal height);
    auto GetLayoutTilePaperHeight() const -> qreal;

    void SetLayoutSheetMargins(const QMarginsF &margins);
    auto GetLayoutSheetMargins() const -> QMarginsF;

    void SetLayoutTileMargins(const QMarginsF &margins);
    auto GetLayoutTileMargins() const -> QMarginsF;

    void SetLayoutSheetIgnoreMargins(bool value);
    auto GetLayoutSheetIgnoreMargins() const -> bool;

    void SetLayoutTileIgnoreMargins(bool value);
    auto GetLayoutTileIgnoreMargins() const -> bool;

    void SetLayoutTileShowTiles(bool value);
    auto GetLayoutTileShowTiles() const -> bool;

    void SetLayoutTileShowWatermark(bool value);
    auto GetLayoutTileShowWatermark() const -> bool;

    void SetLayoutWarningPiecesSuperposition(bool value);
    auto GetLayoutWarningPiecesSuperposition() const -> bool;

    void SetLayoutStickyEdges(bool value);
    auto GetLayoutStickyEdges() const -> bool;

    void SetLayoutWarningPiecesOutOfBound(bool value);
    auto GetLayoutWarningPiecesOutOfBound() const -> bool;

    void SetLayoutFollowGrainline(bool value);
    auto GetLayoutFollowGrainline() const -> bool;

    static auto GetMaxLayoutPieceGap() -> qreal;
    void SetLayoutPieceGap(qreal value);
    auto GetLayoutPieceGap() const -> qreal;

    auto GetLayoutExportFormat() const -> qint8;
    void SetLayoutExportFormat(qint8 format);

    auto GetLayoutLineWidth() const -> int;
    void SetLayoutLineWidth(int width);

private:
    Q_DISABLE_COPY_MOVE(VPSettings) // NOLINT
};

#endif // VPSETTINGS_H
