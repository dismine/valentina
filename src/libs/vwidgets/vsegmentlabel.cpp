/************************************************************************
 **
 **  @file   vsegmentlabel.cpp
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
#include "vsegmentlabel.h"

#include "../ifc/ifcdef.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vabstractapplication.h"
#include "global.h"
#include "scalesceneitems.h"
#include "vgraphicssimpletextitem.h"

#include <utility>
#include <QPainter>
#include <QPainterStateGuard>

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, labelPrefix, ("⌒ ")) // NOLINT

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VSegmentLabel::VSegmentLabel(QGraphicsItem *parent)
  : QObject(nullptr),
    QGraphicsItem(parent)
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
VSegmentLabel::VSegmentLabel(VPointF pos, const QSharedPointer<VAbstractCurve> &curve, QGraphicsItem *parent)
  : QObject(nullptr),
    QGraphicsItem(parent),
    m_labelPos(std::move(pos)),
    m_segment(curve)
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSegmentLabel::boundingRect() const -> QRectF
{
    QRectF rect{-k_HotspotRadius, -k_HotspotRadius, k_HotspotRadius * 2.0, k_HotspotRadius * 2.0};

    if (m_hoverSegment && !m_segment.isNull())
    {
        const QVector<QPointF> segmentShape = m_segment->GetPoints();
        if (segmentShape.size() < 2)
        {
            return rect;
        }

        for (const QPointF &p : segmentShape)
        {
            rect |= QRectF(mapFromScene(p) - QPointF(1, 1), QSizeF(2, 2));
        }
    }

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_hoverSegment && !m_segment.isNull())
    {
        PaintSegment(painter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetLabelData(const VPointF &pos)
{
    m_labelPos = pos;
    setPos(m_labelPos.toQPointF());
    m_label->setText(*labelPrefix + m_labelPos.name());
    {
        const QSignalBlocker blocker(m_label);
        m_label->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        m_label->SetRealPos(QPointF(m_labelPos.mx(), m_labelPos.my()));
        m_label->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
    UpdateLabelLine(); // text width may shift the anchor
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetLabelPosition(const QPointF &pos)
{
    m_labelPos.setMx(pos.x());
    m_labelPos.setMy(pos.y());
    {
        const QSignalBlocker blocker(m_label);
        m_label->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        m_label->SetRealPos(QPointF(m_labelPos.mx(), m_labelPos.my()));
        m_label->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
    UpdateLabelLine(); // text width may shift the anchor
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetSegmentShape(const QSharedPointer<VAbstractCurve> &curve)
{
    m_segment = curve;
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::UpdateLabelLine()
{
    SCASSERT(m_label != nullptr)
    SCASSERT(m_labelLine != nullptr)

    // ── 0. If label is hidden, hide line too and bail early ──────────────
    if (!m_showLabel)
    {
        m_labelLine->setVisible(false);
        return;
    }

    // ── 1. Check overlap in local coordinates ────────────────────────────
    QRectF labelLocalRect = m_label->sceneBoundingRect();
    labelLocalRect.translate(-scenePos());

    if (boundingRect().intersects(labelLocalRect))
    {
        m_labelLine->setVisible(false);
        return;
    }

    // ── 2. Find where the line hits the label rect edge ──────────────────
    const QRectF labelSceneRect = m_label->sceneBoundingRect();
    const QPointF pRec = VGObject::LineIntersectRect(labelSceneRect, QLineF(scenePos(), labelSceneRect.center()));

    // Convert to local coordinates (line is drawn in item space)
    const QPointF pRecLocal = pRec - scenePos();

    // ── 3. Hide if the label is still too close (< 4mm scaled) ───────────
    const qreal minLength = ToPixel(4.0 / qMax(1.0, SceneScale(scene())), Unit::Mm);

    if (QLineF(QPointF(), pRecLocal).length() <= minLength)
    {
        m_labelLine->setVisible(false);
    }
    else
    {
        m_labelLine->setLine(QLineF(QPointF(), pRecLocal));
        m_labelLine->setVisible(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetLabelVisible(bool visible)
{
    m_showLabel = visible;
    m_label->setVisible(visible);
    UpdateLabelLine();

    if (!visible)
    {
        HoverSegment(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSegmentLabel::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == ItemSceneHasChanged)
    {
        UpdateLabelLine();
    }

    return QGraphicsItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::HoverSegment(bool hover)
{
    m_hoverSegment = hover;
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetEnabledState(bool enabled)
{
    m_label->SetEnabledState(enabled);
    m_labelLine->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetLabelMovable(bool value)
{
    m_label->setFlag(QGraphicsItem::ItemIsMovable, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::SetLabelSelectable(bool value)
{
    m_label->setFlag(QGraphicsItem::ItemIsSelectable, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::AllowLabelHover(bool enabled)
{
    m_label->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::LabelSelectionType(const SelectionType &type)
{
    m_label->LabelSelectionType(type);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::Init()
{
    setPos(m_labelPos.toQPointF());

    // ── Label ────────────────────────────────────────────────────────────
    m_label = new VGraphicsSimpleTextItem(VColorRole::DraftLabelColor, VColorRole::DraftLabelHoverColor, this);
    {
        const QSignalBlocker blocker(m_label);
        m_label->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        m_label->SetRealPos(QPointF(m_labelPos.mx(), m_labelPos.my()));
        m_label->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
    m_label->setText(*labelPrefix + m_labelPos.name());
    m_label->setVisible(m_showLabel);

    connect(m_label,
            &VGraphicsSimpleTextItem::PointChoosed,
            this,
            [this]() -> void
            {
                if (m_segment.isNull())
                {
                    return; // do nothing
                }

                emit SegmentChoosed(m_labelPos.id(), m_segment->SceneObjectType());
            });
    connect(m_label,
            &VGraphicsSimpleTextItem::PointSelected,
            this,
            [this](bool selected) -> void
            {
                emit SegmentSelected(selected);
            });
    connect(m_label,
            &VGraphicsSimpleTextItem::NameChangePosition,
            this,
            [this](const QPointF &pos) -> void
            {
                UpdateLabelLine();
                emit LabelPositionChanged(pos - this->pos());
            });
    connect(
        m_label,
        &VGraphicsSimpleTextItem::UpdateLine,
        this,
        [this]() -> void { UpdateLabelLine(); },
        Qt::QueuedConnection);
    connect(m_label, &VGraphicsSimpleTextItem::HoverCurve, this, &VSegmentLabel::HoverSegment);

    // ── Leader line ──────────────────────────────────────────────────────
    m_labelLine = new VScaledLine(VColorRole::DraftLabelLineColor, this);
    m_labelLine->SetBoldLine(false);
    m_labelLine->setLine(QLineF(0, 0, 1, 0));
    m_labelLine->setVisible(m_showLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void VSegmentLabel::PaintSegment(QPainter *painter) const
{
    if (m_segment.isNull())
    {
        return;
    }

    QVector<QPointF> segmentShape = m_segment->GetPoints();
    if (segmentShape.size() < 2)
    {
        return;
    }

    const qreal width = ScaleWidth(VAbstractApplication::VApp()->Settings()->WidthMainLine(), SceneScale(scene()));

    const QColor penColor = VSceneStylesheet::CorrectToolColor(this,
                                                               VSceneStylesheet::CorrectToolColorForDarkTheme(
                                                                   m_segment->GetColor()));

    QPainterStateGuard const guard(painter);
    QPen pen(penColor, width, LineStyleToPenStyle(m_segment->GetPenStyle()), Qt::RoundCap);
    painter->setPen(pen);

    // segmentShape is in scene coords — convert to local
    for (auto &i : segmentShape)
    {
        i = mapFromScene(i);
    }

    for (int i = 0; i < segmentShape.size() - 1; ++i)
    {
        painter->drawLine(segmentShape.at(i), segmentShape.at(i + 1));
    }

    if (const QPainterPath arrowsPath
        = VAbstractCurve::ShowDirection(m_segment->DirectionArrows(),
                                        ScaleWidth(VAbstractCurve::LengthCurveDirectionArrow(), SceneScale(scene())));
        arrowsPath != QPainterPath())
    {
        QPainterStateGuard const arrowsGuard(painter);

        pen.setStyle(Qt::SolidLine);

        painter->setPen(pen);
        painter->drawPath(mapFromScene(arrowsPath));
    }
}
