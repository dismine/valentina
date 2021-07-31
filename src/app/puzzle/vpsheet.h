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
#include <QComboBox>
#include <QUuid>

#include "def.h"

class VPLayout;
class VPPiece;

class VPSheet : public QObject
{
    Q_OBJECT
public:
    explicit VPSheet(VPLayout* layout);

    virtual ~VPSheet() = default;

    /**
     * @brief GetLayout Returns the Layout of the sheet
     * @return Layout of the sheet
     */
    auto GetLayout() -> VPLayout*;

    auto GetPieces() const -> QList<VPPiece *>;

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

private:
    Q_DISABLE_COPY(VPSheet)

    VPLayout *m_layout;

    QString m_name{};

    QUuid m_uuid{QUuid::createUuid()};

    bool m_visible{true};
};

#endif // VPSHEET_H
