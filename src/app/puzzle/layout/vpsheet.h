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

enum class GrainlineType : qint8
{
    Vertical,
    Horizontal
};

struct VPTransformationOrigon
{
    QPointF origin{};
    bool    custom{false};
};

class VPSheet
{
public:
    explicit VPSheet(const VPLayoutPtr &layout);

    virtual ~VPSheet() = default;

    /**
     * @brief GetLayout Returns the Layout of the sheet
     * @return Layout of the sheet
     */
    auto GetLayout() const -> VPLayoutPtr;

    auto GetPieces() const -> QList<VPPiecePtr>;

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

    auto GrainlineType() const -> GrainlineType;

    auto TransformationOrigin() const -> const VPTransformationOrigon &;
    void SetTransformationOrigin(const VPTransformationOrigon &newTransformationOrigin);

    void Clear();

private:
    Q_DISABLE_COPY(VPSheet)

    VPLayoutWeakPtr m_layout{};

    QString m_name{};

    QUuid m_uuid{QUuid::createUuid()};

    bool m_visible{true};

    VPTransformationOrigon m_transformationOrigin{};
};

Q_DECLARE_METATYPE(VPSheetPtr)

#endif // VPSHEET_H
