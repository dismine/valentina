/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VISPIECESPECIALPOINTS_H
#define VISPIECESPECIALPOINTS_H

#include "vispath.h"

class VSimplePoint;

class VisPieceSpecialPoints : public VisPath
{
    Q_OBJECT // NOLINT
public:
    explicit VisPieceSpecialPoints(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisPieceSpecialPoints() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id = NULL_ID) override;

    void SetSpecialPoints(const QVector<quint32> &pins);
    void SetShowRect(bool show);
    void SetRect(const QRectF &rect);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::PieceSpecialPoints)};
private:
    Q_DISABLE_COPY_MOVE(VisPieceSpecialPoints) // NOLINT
    QVector<VSimplePoint *>  m_points{};
    QVector<quint32>         m_spoints{};
    bool                     m_showRect{false};
    QRectF                   m_placeLabelRect{};
    VCurvePathItem          *m_rectItem{nullptr};

    auto GetPoint(quint32 i, VColorRole role) -> VSimplePoint *;

    void HideAllItems();
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisPieceSpecialPoints::SetSpecialPoints(const QVector<quint32> &pins)
{
    m_spoints = pins;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisPieceSpecialPoints::SetShowRect(bool show)
{
    m_showRect = show;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisPieceSpecialPoints::SetRect(const QRectF &rect)
{
    m_placeLabelRect = rect;
}

#endif // VISPIECESPECIALPOINTS_H
