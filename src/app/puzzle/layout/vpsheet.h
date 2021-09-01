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

#include <QSizeF>
#include <QMarginsF>
#include <QList>
#include <QComboBox>
#include <QUuid>

#include "def.h"
#include "layoutdef.h"

class VPLayout;
class VPPiece;

class VPSheet : public QObject
{
    Q_OBJECT
public:
    explicit VPSheet(const VPLayoutPtr &layout);

    virtual ~VPSheet() = default;

    /**
     * @brief GetLayout Returns the Layout of the sheet
     * @return Layout of the sheet
     */
    auto GetLayout() const -> VPLayoutPtr;

    auto GetPieces() const -> QList<VPPiecePtr>;

    auto GetSelectedPieces() const -> QList<VPPiecePtr>;

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

    bool IsVisible() const;
    void SetVisible(bool visible);

    auto GrainlineOrientation() const -> GrainlineType;
    auto GetGrainlineType() const -> GrainlineType;
    void SetGrainlineType(GrainlineType type);

    auto TransformationOrigin() const -> const VPTransformationOrigon &;
    void SetTransformationOrigin(const VPTransformationOrigon &newTransformationOrigin);

    void Clear();

    auto TrashSheet() const -> bool;
    void SetTrashSheet(bool newTrashSheet);

    void ValidateSuperpositionOfPieces() const;
    void ValidatePieceOutOfBound(const VPPiecePtr &piece) const;
    void ValidatePiecesOutOfBound() const;

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

public slots:
    void CheckPiecePositionValidity(const VPPiecePtr &piece) const;

private:
    Q_DISABLE_COPY(VPSheet)

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

    auto SheetUnits() const -> Unit;

};

Q_DECLARE_METATYPE(VPSheetPtr)

#endif // VPSHEET_H
