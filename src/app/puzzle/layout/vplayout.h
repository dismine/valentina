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
#include <QMap>

#include "def.h"
#include "vplayoutsettings.h"
#include "layoutdef.h"

class VPPiece;
class VPSheet;
class QUndoStack;

class VPLayout : public QObject
{
    Q_OBJECT
public:
    virtual ~VPLayout() = default;

    static auto CreateLayout(QUndoStack *undoStack) -> VPLayoutPtr;
    static void AddPiece(const VPLayoutPtr &layout, const VPPiecePtr &piece);

    auto GetPieces() const -> QList<VPPiecePtr>;
    auto GetUnplacedPieces() const -> QList<VPPiecePtr>;
    auto GetTrashedPieces() const -> QList<VPPiecePtr>;

    auto AddSheet(const VPSheetPtr &sheet) -> VPSheetPtr;
    auto GetSheets() -> QList<VPSheetPtr>;
    auto GetSheet(const QUuid &uuid) -> VPSheetPtr;

    /**
     * @brief SetFocusedSheet Sets the focused sheet, to which pieces are added from the carrousel via drag
     * and drop
     * @param focusedSheet the new active sheet. If nullptr, then it sets automaticaly the first sheet from m_sheets
     */
    void SetFocusedSheet(const VPSheetPtr &focusedSheet = VPSheetPtr());

    /**
     * @brief GetFocusedSheet Returns the focused sheet, to which pieces are added from the carrousel via drag
     * and drop
     * @return the focused sheet
     */
    auto GetFocusedSheet() -> VPSheetPtr;

    void AddTrashSheet(const VPSheetPtr &sheet);
    auto GetTrashSheet() -> VPSheetPtr;

    auto LayoutSettings() -> VPLayoutSettings &;

    auto PiecesForSheet(const VPSheetPtr &sheet) const -> QList<VPPiecePtr>;
    auto PiecesForSheet(const QUuid &uuid) const -> QList<VPPiecePtr>;

    QUndoStack *UndoStack() const;

    void SetUndoStack(QUndoStack *newUndoStack);

    void Clear();

signals:
    void PieceSheetChanged(const VPPiecePtr &piece);
    void ActiveSheetChanged(const VPSheetPtr &focusedSheet);
    void PieceTransformationChanged(const VPPiecePtr &piece);
    void TransformationOriginChanged();
    void SheetListChanged();
    void PieceSelectionChanged(const VPPiecePtr &piece);
    void LayoutChanged();

protected:
    explicit VPLayout(QUndoStack *undoStack);

    void AddPiece(const VPPiecePtr &piece);

private:
    Q_DISABLE_COPY(VPLayout)

    QMap<QString, VPPiecePtr> m_pieces{};

    VPSheetPtr m_trashSheet{};

    QList<VPSheetPtr> m_sheets{};
    VPSheetPtr m_focusedSheet{};

    VPLayoutSettings m_layoutSettings{};

    QUndoStack *m_undoStack;
};

Q_DECLARE_METATYPE(VPLayoutPtr)

#endif // VPLAYOUT_H
