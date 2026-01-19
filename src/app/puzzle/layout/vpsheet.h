/************************************************************************
 **
 **  @file   vpsheet.h
 **  @author Ronan Le Tiec
 **  @date   23 5, 2020
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
#ifndef VPSHEET_H
#define VPSHEET_H

#include <QComboBox>
#include <QFutureWatcher>
#include <QList>
#include <QMarginsF>
#include <QPageLayout>
#include <QSizeF>
#include <QUuid>

#include "../vlayout/vlayoutdef.h"
#include "../vmisc/def.h"
#include "layoutdef.h"

class VPLayout;
class VPPiece;
class VMainGraphicsScene;
class VPGraphicsPieceControls;
class VPGraphicsTransformationOrigin;
class VPGraphicsPiece;
class VPGraphicsTileGrid;
class VPGraphicsSheet;
class VLayoutPiece;
class QGraphicsItem;

struct VPiecePositionValidity
{
    bool outOfBound{false};
    bool superposition{false};
    bool gap{false};
};

class VPSheetSceneData
{
public:
    explicit VPSheetSceneData(const VPLayoutPtr &layout, const QUuid &sheetUuid);
    ~VPSheetSceneData();

    auto Scene() const -> QSharedPointer<VMainGraphicsScene>;

    /**
     * @brief RefreshLayout Refreshes the rectangles for the layout border and the margin
     */
    void RefreshLayout();

    void RefreshPieces(bool printMode = false);

    /**
     * @brief PrepareForExport prepares the graphic for an export (i.e hide margin etc)
     */
    void PrepareForExport(LayoutExportFormats format);

    /**
     * @brief CleanAfterExport cleans the graphic for an export (i.e show margin etc)
     */
    void CleanAfterExport();

    auto GraphicsPieces() const -> const QList<VPGraphicsPiece *> &;
    auto GraphicsPiecesAsItems() const -> QList<QGraphicsItem *>;

    auto RotationControls() const -> VPGraphicsPieceControls *;

    auto ScenePiece(const VPPiecePtr &piece) const -> VPGraphicsPiece *;

    void RemovePiece(VPGraphicsPiece *piece);
    void AddPiece(VPGraphicsPiece *piece);

    auto IsTextAsPaths() const -> bool;
    void SetTextAsPaths(bool textAsPaths);

    void PrepareTilesScheme();
    void ClearTilesScheme();

    void RefreshSheetSize();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPSheetSceneData) // NOLINT

    VPLayoutWeakPtr m_layout{};

    QSharedPointer<VMainGraphicsScene> m_scene{nullptr};

    VPGraphicsSheet *m_graphicsSheet{nullptr};

    VPGraphicsTileGrid *m_graphicsTileGrid{nullptr};

    VPGraphicsPieceControls *m_rotationControls{nullptr};
    VPGraphicsTransformationOrigin *m_rotationOrigin{nullptr};

    QList<VPGraphicsPiece *> m_graphicsPieces{};

    /**
     * variable to hold temporarly hte value of the show tiles
     */
    bool m_showTilesTmp{false};

    bool m_showTilesSchemeTmp{false};
    bool m_showTilesWatermarkSchemeTmp{false};

    /**
     * variable to hold temporarly hte value of the show grid
     */
    bool m_showGridTmp{false};

    QUuid m_sheetUuid;

    QList<VPPiecePtr> m_slectedPiecesTmp{};

    bool m_outOfBoundTmp{false};
    bool m_pieceSuperpositionTmp{false};
    bool m_pieceGapePositionTmp{false};
    bool m_textAsPaths{false};

    void ConnectPiece(VPGraphicsPiece *piece) const;
};

class VPSheet : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit VPSheet(const VPLayoutPtr &layout, QObject *parent = nullptr);

    ~VPSheet() override;

    /**
     * @brief GetLayout Returns the Layout of the sheet
     * @return Layout of the sheet
     */
    auto GetLayout() const -> VPLayoutPtr;

    auto GetPieces() const -> QList<VPPiecePtr>;

    auto GetSelectedPieces() const -> QList<VPPiecePtr>;

    auto GetAsLayoutPieces() const -> QVector<VLayoutPiece>;

    /**
     * @brief GetName Returns the name of the sheet
     * @return the name
     */
    auto GetName() const -> QString;

    /**
     * @brief SetName Sets the name of the sheet to the given name
     * @param name the new sheet's name
     */
    void SetName(const QString &name);

    auto Uuid() const -> const QUuid &;

    auto IsVisible() const -> bool;
    void SetVisible(bool visible);

    auto GrainlineOrientation() const -> GrainlineType;
    auto GetGrainlineType() const -> GrainlineType;
    void SetGrainlineType(GrainlineType type);

    auto TransformationOrigin() const -> const VPTransformationOrigon &;
    void SetTransformationOrigin(const VPTransformationOrigon &newTransformationOrigin);

    void Clear();

    auto TrashSheet() const -> bool;
    void SetTrashSheet(bool newTrashSheet);

    auto GetSheetRect() const -> QRectF;
    auto GetMarginsRect() const -> QRectF;

    void RemoveUnusedLength();

    /**
     * @brief SetSheetSize sets the size of the sheet, the values have to be in Unit::Px
     * @param width sheet width
     * @param height sheet height
     */
    void SetSheetSize(qreal width, qreal height);

    /**
     * @brief SetSheetSize sets the size of the sheet, the values have to be in the layout's unit
     * @param width sheet width
     * @param height sheet height
     */
    void SetSheetSizeConverted(qreal width, qreal height);

    /**
     * @brief SetSheetSize sets the size of the sheet, the values have to be in Unit::Px
     * @param size sheet size
     */
    void SetSheetSize(const QSizeF &size);
    /**
     * @brief SetSheetSizeConverted sets the size of the sheet, the values have to be in the layout's unit
     * @param size sheet size
     */
    void SetSheetSizeConverted(const QSizeF &size);

    /**
     * @brief GetSheetSize Returns the size in Unit::Px
     * @return sheet size in Unit::Px
     */
    auto GetSheetSize() const -> QSizeF;

    /**
     * @brief GetSheetSizeConverted Returns the size in the layout's unit
     * @return the size in the layout's unit
     */
    auto GetSheetSizeConverted() const -> QSizeF;

    /**
     * @brief SetSheetMargins, set the margins of the sheet, the values have to be in Unit::Px
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetSheetMargins(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetSheetMargins, set the margins of the sheet, the values have to be in the unit of the layout
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetSheetMarginsConverted(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetSheetMargins set the margins of the sheet, the values have to be in Unit::Px
     * @param margins sheet margins
     */
    void SetSheetMargins(const QMarginsF &margins);

    /**
     * @brief SetSheetMargins set the margins of the sheet, the values have to be in the unit of the layout
     * @param margins sheet margins
     */
    void SetSheetMarginsConverted(const QMarginsF &margins);

    /**
     * @brief GetSheetMargins Returns the size in Unit::Px
     * @return the size in Unit::Px
     */
    auto GetSheetMargins() const -> QMarginsF;

    /**
     * @brief GetSheetMarginsConverted Returns the margins in the layout's unit
     * @return the margins in the sheet's unit
     */
    auto GetSheetMarginsConverted() const -> QMarginsF;

    auto IgnoreMargins() const -> bool;
    void SetIgnoreMargins(bool newIgnoreMargins);

    auto SceneData() const -> QSharedPointer<VPSheetSceneData>;

    void ClearSelection() const;

    auto GetSheetOrientation() const -> QPageLayout::Orientation;

public slots:
    void CheckPiecesPositionValidity() const;

private slots:
    void UpdatePiecesValidity();

private:
    Q_DISABLE_COPY_MOVE(VPSheet) // NOLINT

    VPLayoutWeakPtr m_layout{};

    QString m_name{};

    QUuid m_uuid{QUuid::createUuid()};

    bool m_visible{true};
    bool m_trashSheet{false};

    VPTransformationOrigon m_transformationOrigin{};

    /**
     * @brief m_size the Size in Unit::Px
     */
    QSizeF m_size{};

    // margins
    /**
     * @brief m_margins the margins in Unit::Px
     */
    QMarginsF m_margins{};

    bool m_ignoreMargins{false};

    GrainlineType m_grainlineType{GrainlineType::NotFixed};

    QSharedPointer<VPSheetSceneData> m_sceneData{nullptr};

    QFutureWatcher<QHash<QString, VPiecePositionValidity>> *m_validityWatcher;
    mutable bool m_validationStale{false};

    auto SheetUnits() const -> Unit;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPSheetSceneData::IsTextAsPaths() const -> bool
{
    return m_textAsPaths;
}

Q_DECLARE_METATYPE(VPSheetPtr) // NOLINT

#endif // VPSHEET_H
