/************************************************************************
 **
 **  @file   vmaingraphicsscene.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VMAINGRAPHICSSCENE_H
#define VMAINGRAPHICSSCENE_H


#include <QGraphicsScene>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QTransform>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"

/**
 * @brief The VMainGraphicsScene class main scene.
 */
class VMainGraphicsScene : public QGraphicsScene
{
    Q_OBJECT // NOLINT
public:
    explicit VMainGraphicsScene(QObject *parent = nullptr);
    explicit VMainGraphicsScene(const QRectF & sceneRect, QObject * parent = nullptr);
    auto getHorScrollBar() const -> qint32;
    void          setHorScrollBar(const qint32 &value);
    auto getVerScrollBar() const -> qint32;
    void          setVerScrollBar(const qint32 &value);
    auto transform() const -> QTransform;
    void          setTransform(const QTransform &transform);
    void EnableTools();
    auto getScenePos() const -> QPointF;

    auto VisibleItemsBoundingRect() const -> QRectF;
    void          InitOrigins();
    void          SetOriginsVisible(bool visible);

    auto IsNonInteractive() const -> bool;
    void          SetNonInteractive(bool nonInteractive);

    void SetAcceptDrop(bool newAcceptDrop);
    auto AcceptDrop() const -> bool;

public slots:
    void          ChoosedItem(quint32 id, const SceneObject &type);
    void          SelectedItem(bool selected, quint32 object, quint32 tool);
    void          EnableItemMove(bool move);
    void          EnableDetailsMode(bool mode);
    void          ItemsSelection(const SelectionType &type);
    void          HighlightItem(quint32 id);
    void          UpdatePiecePassmarks();

    void          ToggleLabelSelection(bool enabled);
    void          TogglePointSelection(bool enabled);
    void          ToggleLineSelection(bool enabled);
    void          ToggleArcSelection(bool enabled);
    void          ToggleElArcSelection(bool enabled);
    void          ToggleSplineSelection(bool enabled);
    void          ToggleSplinePathSelection(bool enabled);
    void          ToggleNodeLabelSelection(bool enabled);
    void          ToggleNodePointSelection(bool enabled);
    void          ToggleDetailSelection(bool enabled);

    void          ToggleLabelHover(bool enabled);
    void          TogglePointHover(bool enabled);
    void          ToggleLineHover(bool enabled);
    void          ToggleArcHover(bool enabled);
    void          ToggleElArcHover(bool enabled);
    void          ToggleSplineHover(bool enabled);
    void          ToggleSplinePathHover(bool enabled);
    void          ToggleNodeLabelHover(bool enabled);
    void          ToggleNodePointHover(bool enabled);
    void          ToggleDetailHover(bool enabled);
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    /**
     * @brief mouseMove send new mouse position.
     * @param scenePos new mouse position.
     */
    void          mouseMove(const QPointF &scenePos);

    void          MouseLeftPressed();
    void          MouseLeftReleased();
    void          ItemByMousePress(QGraphicsItem* pItem);
    void          ItemByMouseRelease(QGraphicsItem* pItem);
    void          AddBackgroundImage(const QPointF &pos, const QString &fileName);

    /**
     * @brief ChoosedObject send option choosed object.
     * @param id object id.
     * @param type object scene type.
     */
    void          ChoosedObject(quint32 id, SceneObject type);
    void          SelectedObject(bool selected, quint32 object, quint32 tool);
    void EnableItem();
    void          EnableToolMove(bool move);
    void          CurveDetailsMode(bool mode);
    void          ItemSelection(const SelectionType &type);
    void          HighlightDetail(quint32 id);
    void          UpdatePassmarks();

    void          EnableLabelItemSelection(bool enable);
    void          EnablePointItemSelection(bool enable);
    void          EnableLineItemSelection(bool enable);
    void          EnableArcItemSelection(bool enable);
    void          EnableElArcItemSelection(bool enable);
    void          EnableSplineItemSelection(bool enable);
    void          EnableSplinePathItemSelection(bool enable);
    void          EnableNodeLabelItemSelection(bool enabled);
    void          EnableNodePointItemSelection(bool enabled);
    void          EnableDetailItemSelection(bool enabled);

    void          EnableLabelItemHover(bool enable);
    void          EnablePointItemHover(bool enable);
    void          EnableArcItemHover(bool enable);
    void          EnableElArcItemHover(bool enable);
    void          EnableSplineItemHover(bool enable);
    void          EnableSplinePathItemHover(bool enable);
    void          EnableNodeLabelItemHover(bool enabled);
    void          EnableNodePointItemHover(bool enabled);
    void          EnableDetailItemHover(bool enabled);
    void          EnableLineItemHover(bool enabled);
    void          DimensionsChanged();
    void          LanguageChanged();

private:
    Q_DISABLE_COPY_MOVE(VMainGraphicsScene) // NOLINT
    /** @brief horScrollBar value horizontal scroll bar. */
    qint32        horScrollBar;

    /** @brief verScrollBar value vertical scroll bar. */
    qint32        verScrollBar;

    /** @brief _transform view transform value. */
    QTransform    _transform;
    QPointF       scenePos;
    QVector<QGraphicsItem *> origins;

    /** @brief m_nonInteractive all item on scene in non interactive. */
    bool          m_nonInteractive{false};

    bool m_acceptDrop{false};
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getHorScrollBar return scene horizontal scrollbar.
 * @return horizontal scrollbar.
 */
inline auto VMainGraphicsScene::getHorScrollBar() const -> qint32
{
    return horScrollBar;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setHorScrollBar set scene horizontal scrollbar.
 * @param value horizontal scrollbar.
 */
inline void VMainGraphicsScene::setHorScrollBar(const qint32 &value)
{
    horScrollBar = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getVerScrollBar return scene vertical scrollbar.
 * @return vertical scrollbar.
 */
inline auto VMainGraphicsScene::getVerScrollBar() const -> qint32
{
    return verScrollBar;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setVerScrollBar set scene vertical scrollbar.
 * @param value vertical scrollbar.
 */
inline void VMainGraphicsScene::setVerScrollBar(const qint32 &value)
{
    verScrollBar = value;
}

#endif // VMAINGRAPHICSSCENE_H
