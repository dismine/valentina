/************************************************************************
 **
 **  @file   vsegmentlabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 3, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef VSEGMENTLABEL_H
#define VSEGMENTLABEL_H

#include <QGraphicsItem>
#include <QObject>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"

class VGraphicsSimpleTextItem;
class VScaledLine;

class VSegmentLabel : public QObject, public QGraphicsItem
{
    Q_OBJECT                    // NOLINT
    Q_INTERFACES(QGraphicsItem) // NOLINT

public:
    explicit VSegmentLabel(QGraphicsItem *parent = nullptr);
    VSegmentLabel(VPointF pos, const QSharedPointer<VAbstractCurve> &curve, QGraphicsItem *parent = nullptr);
    ~VSegmentLabel() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::SegmentLabel)
    };

    auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void SetLabelData(const VPointF &pos);
    void SetLabelPosition(const QPointF &pos);

    void SetSegmentShape(const QSharedPointer<VAbstractCurve> &curve);

    void UpdateLabelLine();

    void SetEnabledState(bool enabled);

    void SetLabelMovable(bool value);

    void SetLabelSelectable(bool value);

    void AllowLabelHover(bool enabled);

    void LabelSelectionType(const SelectionType &type);

signals:
    void SegmentChoosed(quint32 id, SceneObject type);
    void SegmentSelected(bool selected);
    void LabelPositionChanged(const QPointF &pos);

public slots:
    void SetLabelVisible(bool visible);
    void ShowExplicitly(bool show);

protected:
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;

private slots:
    void HoverSegment(bool hover);

private:
    Q_DISABLE_COPY_MOVE(VSegmentLabel) // NOLINT

    VPointF m_labelPos{};

    bool m_showLabel{false};
    bool m_forcedVisible{false};

    bool m_hoverSegment{false};

    VGraphicsSimpleTextItem *m_label{nullptr};
    VScaledLine *m_labelLine{nullptr};

    QSharedPointer<VAbstractCurve> m_segment{};

    // The invisible "hotspot" half-size for hit-testing
    static constexpr qreal k_HotspotRadius = 4.0;

    void Init();

    void PaintSegment(QPainter *painter) const;
};

#endif // VSEGMENTLABEL_H
