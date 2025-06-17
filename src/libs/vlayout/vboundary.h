/************************************************************************
 **
 **  @file   vboundary.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 11, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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

#ifndef VBOUNDARY_H
#define VBOUNDARY_H

#include "../vgeometry/vgeometrydef.h"
#include "vlayoutpoint.h"

#include <QCoreApplication>
#include <QList>
#include <QVariant>
#include <QVector>

enum class VBoundarySequenceItem : char
{
    Boundary,
    Passmark,
    PassmarkShape,
    Unknown
};

struct VBoundarySequenceItemData
{
    int number{0};
    VBoundarySequenceItem type{VBoundarySequenceItem::Unknown};
    QVariant item{};
    bool mirror{false};
};

class VBoundary
{
    Q_DECLARE_TR_FUNCTIONS(VBoundary) // NOLINT

public:
    VBoundary(const QVector<VLayoutPoint> &boundary, bool seamAllowance, bool builtInSeamAllowance = false);

    auto Combine(const QVector<VLayoutPassmark> &passmarks, bool drawMode, bool layoutAllowance = false) const
        -> QList<VBoundarySequenceItemData>;

    void SetPieceName(const QString &newPieceName);
    void SetMirrorLine(const QLineF &newMirrorLine);

private:
    QVector<VLayoutPoint> m_boundary;
    bool m_seamAllowance;
    bool m_builtInSeamAllowance;
    QString m_pieceName{};
    QLineF m_mirrorLine{};

    auto SkipPassmark(const VLayoutPassmark &passmark, bool layoutAllowance) const -> bool;

    void InsertPassmark(const VBoundarySequenceItemData &item, QList<VBoundarySequenceItemData> &sequence,
                        bool drawMode) const;

    auto InsertDisconnect(QList<VBoundarySequenceItemData> &sequence, int i, const VBoundarySequenceItemData &item,
                          bool drawMode) const -> bool;
    auto InsertCutOut(QList<VBoundarySequenceItemData> &sequence, int i, const VBoundarySequenceItemData &item,
                      bool drawMode) const -> bool;

    auto PreparePassmarkShape(const VLayoutPassmark &passmark, bool drawMode, bool mirrorNotch) const
        -> QVector<QVector<VLayoutPoint>>;
    auto PrepareNoneBreakingPassmarkShape(const VLayoutPassmark &passmark, bool mirrorNotch) const
        -> QVector<QVector<VLayoutPoint>>;
    auto PrepareExternalVPassmarkShape(const VLayoutPassmark &passmark, bool drawMode, bool mirrorNotch) const
        -> QVector<QVector<VLayoutPoint>>;
    auto PrepareTPassmarkShape(const VLayoutPassmark &passmark, bool drawMode, bool mirrorNotch) const
        -> QVector<QVector<VLayoutPoint>>;
    auto PrepareUPassmarkShape(const VLayoutPassmark &passmark, bool mirrorNotch) const
        -> QVector<QVector<VLayoutPoint>>;
};

//---------------------------------------------------------------------------------------------------------------------
inline void VBoundary::SetPieceName(const QString &newPieceName)
{
    m_pieceName = newPieceName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VBoundary::SetMirrorLine(const QLineF &newMirrorLine)
{
    m_mirrorLine = newMirrorLine;
}

#endif // VBOUNDARY_H
