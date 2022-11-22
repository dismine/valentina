/************************************************************************
 **
 **  @file   vbackgroundimageitem.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef VBACKGROUNDIMAGEITEM_H
#define VBACKGROUNDIMAGEITEM_H

#include <QGraphicsObject>

#include "../ifc/xml/vbackgroundpatternimage.h"
#include "../vmisc/def.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

class VAbstractPattern;

class VBackgroundImageItem : public QGraphicsObject
{
    Q_OBJECT // NOLINT
public:
    VBackgroundImageItem(const VBackgroundPatternImage &image, VAbstractPattern *doc, QGraphicsItem *parent = nullptr);
    ~VBackgroundImageItem() override = default;

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Tool::BackgroundImage)};

    auto Image() const -> const VBackgroundPatternImage &;
    void SetImage(const VBackgroundPatternImage &newImage);

    static auto pen() -> QPen;

    auto name() const -> QString;
    void setName(const QString &newName);

    auto IsHold() const -> bool;
    void SetHold(bool hold);

    auto IsVisible() const -> bool;
    void SetVisible(bool visible);

    auto GetOpacity() const -> qreal;
    void SetOpacity(qreal opacity);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void Selected(const QUuid &id);
    void UpdateControls();
    void ActivateControls(const QUuid &id);
    void DeleteImage(const QUuid &id);
    void ShowImageInExplorer(const QUuid &id);
    void SaveImage(const QUuid &id);

public slots:
    void PositionChanged(QUuid id);
    void ImageTransformationChanged(QUuid id);
    void HoldChanged(QUuid id);
    void OpacityChanged(const QUuid &id);
    void VisibilityChanged(QUuid id);
    void NameChanged(QUuid id);
    void EnableSelection(bool enable);
    void DeleteFromMenu();

protected:
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void mousePressEvent( QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) override;
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) override;
    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) override;
    void hoverMoveEvent ( QGraphicsSceneHoverEvent * event ) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent ( QGraphicsSceneContextMenuEvent * event ) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent * event) override;

    auto Stale() const -> bool;
    void MakeFresh() const;
    void DeleteToolWithConfirm(bool ask = true);

    auto Invalid() const -> bool;
    void SetInvalid(bool newInvalid);

private slots:
    void UpdateHoldState();
    void UpdateVisibilityState();
    void ZValueChanged();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VBackgroundImageItem) // NOLINT

    VBackgroundPatternImage m_image;
    VAbstractPattern *m_doc;
    mutable bool m_stale{true};
    bool m_allowChangeMerge{false};
    QPointF m_lastMoveDistance{};
    bool m_wasMoved{false};
    bool m_showHover{false};
    bool m_selectable{true};

    void InitImage();

    void TranslateImageOn(qreal dx, qreal dy);
    void RotateImageByAngle(qreal angle);
    void ScaleImageByAdjustSize(qreal value);
    void ScaleImageByFactor(qreal factor);
    void MoveImageZValue(int move);

    void UpdateSceneRect();

    void SetupZValue();
};

#endif // VBACKGROUNDIMAGEITEM_H
