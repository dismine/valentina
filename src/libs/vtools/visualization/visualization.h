/************************************************************************
 **
 **  @file   visualization.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QGraphicsItem>
#include <QLoggingCategory>
#include <QObject>
#include <QtGlobal>

#include "../vgeometry/vabstractcurve.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vwidgets/vcurvepathitem.h"
#include "../vwidgets/vmaingraphicsscene.h"

Q_DECLARE_LOGGING_CATEGORY(vVis) // NOLINT

class VScaledEllipse;
class VScaledLine;
class VContainer;
class VInternalVariable;

enum class Mode : qint8
{
    Creation,
    Show
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class Visualization : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit Visualization(const VContainer *data);
    ~Visualization() override = default;

    virtual void RefreshGeometry() = 0;
    virtual void VisualMode(quint32 id) = 0;

    void SetLineStyle(const Qt::PenStyle &value);
    auto LineStyle() const -> Qt::PenStyle;

    auto GetData() const -> const VContainer *;
    void SetData(const VContainer *data);

    auto GetMode() const -> Mode;
    void SetMode(const Mode &value);

    static auto FindLengthFromUser(const QString &expression,
                                   const QHash<QString, QSharedPointer<VInternalVariable>> *vars, bool fromUser = true)
        -> qreal;
    static auto FindValFromUser(const QString &expression,
                                const QHash<QString, QSharedPointer<VInternalVariable>> *vars, bool fromUser = true)
        -> qreal;
    static auto CorrectAngle(qreal angle) -> qreal;

    auto CurrentToolTip() const -> QString;

    void RefreshToolTip() const;

signals:
    void ToolTip(const QString &toolTip) const; // clazy:exclude=const-signal-or-slot

public slots:
    void MousePos(const QPointF &scenePos);

protected:
    virtual void InitPen() = 0;
    virtual void AddOnScene() = 0;

    static auto InitPoint(VColorRole role, QGraphicsItem *parent, qreal z = 0) -> VScaledEllipse *;
    static void DrawPoint(QGraphicsEllipseItem *point, const QPointF &pos, Qt::PenStyle style = Qt::SolidLine);
    virtual void DrawLine(VScaledLine *lineItem, const QLineF &line, Qt::PenStyle style = Qt::SolidLine);
    static void DrawPath(VCurvePathItem *pathItem, const QPainterPath &path, Qt::PenStyle style = Qt::SolidLine,
                         Qt::PenCapStyle cap = Qt::SquareCap);
    static void DrawPath(VCurvePathItem *pathItem, const QPainterPath &path,
                         const QVector<DirectionArrow> &directionArrows, Qt::PenStyle style = Qt::SolidLine,
                         Qt::PenCapStyle cap = Qt::SquareCap);

    template <typename Item> void AddItem(Item *item);

    template <class Item> auto InitItem(VColorRole role, QGraphicsItem *parent) -> Item *;

    static auto GetPointItem(QVector<VScaledEllipse *> &points, quint32 i, VColorRole role, QGraphicsItem *parent)
        -> VScaledEllipse *;
    static auto GetCurveItem(QVector<VCurvePathItem *> &curves, quint32 i, VColorRole role, QGraphicsItem *parent)
        -> VCurvePathItem *;

    static auto LengthToUser(qreal value) -> QString;
    static auto AngleToUser(qreal value) -> QString;

    void SetScenePos(QPointF pos);
    auto ScenePos() const -> QPointF;

    void SetToolTip(const QString &tooltip);

    void StartVisualMode();

private:
    Q_DISABLE_COPY_MOVE(Visualization) // NOLINT

    const VContainer *m_data;
    QPointF m_scenePos{};
    Qt::PenStyle m_lineStyle{Qt::SolidLine};
    QString m_toolTip{};
    Mode m_mode{Mode::Creation};
};

// cppcheck-suppress unknownMacro
QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
template <typename Item> inline void Visualization::AddItem(Item *item)
{
    SCASSERT(item != nullptr)
    auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
    SCASSERT(scene != nullptr)

    scene->addItem(item);
    connect(scene, &VMainGraphicsScene::mouseMove, item, &Visualization::MousePos);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item> inline auto Visualization::InitItem(VColorRole role, QGraphicsItem *parent) -> Item *
{
    auto *item = new Item(role, parent);
    item->setZValue(1);
    item->setFlags(QGraphicsItem::ItemStacksBehindParent);
    item->setVisible(false);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto Visualization::GetMode() const -> Mode
{
    return m_mode;
}

//---------------------------------------------------------------------------------------------------------------------
inline void Visualization::SetMode(const Mode &value)
{
    m_mode = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto Visualization::CurrentToolTip() const -> QString
{
    return m_toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
inline void Visualization::SetScenePos(QPointF pos)
{
    m_scenePos = pos;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto Visualization::ScenePos() const -> QPointF
{
    return m_scenePos;
}

//---------------------------------------------------------------------------------------------------------------------
inline void Visualization::SetToolTip(const QString &tooltip)
{
    this->m_toolTip = tooltip;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto Visualization::LineStyle() const -> Qt::PenStyle
{
    return GetMode() == Mode::Creation && m_lineStyle == Qt::NoPen ? Qt::SolidLine : m_lineStyle;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto Visualization::GetData() const -> const VContainer *
{
    return m_data;
}

//---------------------------------------------------------------------------------------------------------------------
inline void Visualization::SetData(const VContainer *data)
{
    this->m_data = data;
}

#endif // VISUALIZATION_H
