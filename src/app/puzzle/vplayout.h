/************************************************************************
 **
 **  @file   vplayout.h
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
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
#ifndef VPLAYOUT_H
#define VPLAYOUT_H

#include <QList>

#include "def.h"
#include "vplayoutsettings.h"

class VPPiece;
class VPSheet;

class VPLayout : public QObject
{
    Q_OBJECT
public:
    explicit VPLayout(QObject *parent=nullptr);
    virtual ~VPLayout();

    void AddPiece(VPPiece *piece);
    auto GetPieces() const -> QList<VPPiece *>;
    auto GetUnplacedPieces() const -> QList<VPPiece *>;
    auto GetTrashedPieces() const -> QList<VPPiece *>;

    auto AddSheet() -> VPSheet*;
    auto AddSheet(VPSheet *sheet) -> VPSheet*;
    auto GetSheets() -> QList<VPSheet *>;

    /**
     * @brief SetFocusedSheet Sets the focused sheet, to which pieces are added from the carrousel via drag
     * and drop
     * @param focusedSheet the new active sheet. If nullptr, then it sets automaticaly the first sheet from m_sheets
     */
    void SetFocusedSheet(VPSheet *focusedSheet = nullptr);

    /**
     * @brief GetFocusedSheet Returns the focused sheet, to which pieces are added from the carrousel via drag
     * and drop
     * @return the focused sheet
     */
    auto GetFocusedSheet() -> VPSheet*;

    auto GetTrashSheet() -> VPSheet*;

    auto LayoutSettings() -> VPLayoutSettings &;

    auto PiecesForSheet(const VPSheet* sheet) const -> QList<VPPiece *>;

private:
    Q_DISABLE_COPY(VPLayout)

    QList<VPPiece *> m_pieces{};

    VPSheet* m_trashSheet;

    QList<VPSheet*> m_sheets{};
    VPSheet *m_focusedSheet{nullptr};

    VPLayoutSettings m_layoutSettings{};
};

#endif // VPLAYOUT_H
