/************************************************************************
 **
 **  @file   vdecorationaligningdelegate.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 10, 2020
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
#ifndef VDECORATIONALIGNINGDELEGATE_H
#define VDECORATIONALIGNINGDELEGATE_H

#include <QStyledItemDelegate>

#include "../vmisc/defglobal.h"

class VDecorationAligningDelegate : public QStyledItemDelegate
{
    Q_OBJECT // NOLINT

public:
    explicit VDecorationAligningDelegate(Qt::Alignment alignment, QObject *parent = nullptr);

    auto Alignment() const -> Qt::Alignment;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VDecorationAligningDelegate) // NOLINT
    Qt::Alignment const m_alignment;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VDecorationAligningDelegate::Alignment() const -> Qt::Alignment
{
    return m_alignment;
}

#endif // VDECORATIONALIGNINGDELEGATE_H
