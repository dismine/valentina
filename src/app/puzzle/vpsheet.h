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

#include <QObject>
#include <QSizeF>
#include <QMarginsF>
#include <QList>

#include "def.h"

// is this the right place for the definition?
enum class FollowGrainline : qint8 { No = 0, Follow90 = 1, Follow180 = 2};

class VPLayout;
class VPPieceList;

class VPSheet : public QObject
{
    Q_OBJECT
public:
    explicit VPSheet(VPLayout* layout);

    ~VPSheet();

    /**
     * @brief GetLayout Returns the Layout of the sheet
     * @return
     */
    VPLayout* GetLayout();

    /**
     * @brief GetPieceList returns the piece list of the sheet
     * @return piece list
     */
    VPPieceList* GetPieceList();

    /**
     * @brief GetName Returns the name of the sheet
     * @return the name
     */
    QString GetName() const;

    /**
     * @brief SetName Sets the name of the sheet to the given name
     * @param name the new sheet's name
     */
    void SetName(const QString &name);

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
    QSizeF GetSheetSize() const;

    /**
     * @brief GetSheetSizeConverted Returns the size in the layout's unit
     * @return the size in the layout's unit
     */
    QSizeF GetSheetSizeConverted() const;

    /**
     * @brief GetOrientation Returns the orientation of the sheet
     * @return orientation of the sheet
     */
    PageOrientation GetOrientation();

    /**
     * @brief SetOrientation Sets the orientation of the sheet to the given value
     * @param orientation the new page orientation
     */
    void SetOrientation(PageOrientation orientation);

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
    QMarginsF GetSheetMargins() const;

    /**
     * @brief GetSheetMarginsConverted Returns the margins in the layout's unit
     * @return the margins in the sheet's unit
     */
    QMarginsF GetSheetMarginsConverted() const;

    /**
     * @brief SetFollowGrainline Sets the type of grainline for the pieces to follow
     * @param state the type of grainline
     */
    void SetFollowGrainline(FollowGrainline state);

    /**
     * @brief GetFollowGrainline Returns if the sheet's pieces follow a grainline or not
     * @return wether the pieces follow a grainline and if so, which grainline
     */
    FollowGrainline GetFollowGrainline() const;

    /**
     * @brief SetPiecesGap sets the pieces gap to the given value, the unit has to be in Unit::Px
     * @param value pieces gap
     */
    void SetPiecesGap(qreal value);

    /**
     * @brief SetPiecesGapConverted sets the pieces gap to the given value, the unit has to be in the layout's unit
     * @param value pieces gap
     */
    void SetPiecesGapConverted(qreal value);

    /**
     * @brief GetPiecesGap returns the pieces gap in Unit::Px
     * @return the pieces gap in Unit::Px
     */
    qreal GetPiecesGap() const;

    /**
     * @brief GetPiecesGapConverted returns the pieces gap in the layout's unit
     * @return the pieces gap in the layout's unit
     */
    qreal GetPiecesGapConverted() const;

    /**
     * @brief ClearSelection goes through the piece list and pieces and calls
     * SetIsSelected(false) for the pieces that were selected.
     */
    void ClearSelection();

    void SetStickyEdges(bool state);
    bool GetStickyEdges() const;


private:
    Q_DISABLE_COPY(VPSheet)

    VPLayout *m_layout;

    VPPieceList *m_pieceList {nullptr};

    QString m_name{};

    /**
     * @brief m_size the Size in Unit::Px
     */
    QSizeF m_size{};

    /**
     * @brief holds the orientation of the sheet
     */
    PageOrientation m_orientation {PageOrientation::Portrait};

    // margins
    /**
     * @brief m_margins the margins in Unit::Px
     */
    QMarginsF m_margins{};

    // control
    FollowGrainline m_followGrainLine{FollowGrainline::No};

    /**
     * @brief m_piecesGap the pieces gap in Unit::Px
     */
    qreal m_piecesGap{0};

    bool m_stickyEdges{false};
};

#endif // VPSHEET_H
