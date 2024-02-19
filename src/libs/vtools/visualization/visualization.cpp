/************************************************************************
 **
 **  @file   visualization.cpp
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

#include "visualization.h"

#include <QBrush>
#include <QColor>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QLineF>
#include <QMessageLogger>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QScopedPointer>
#include <QString>
#include <QtCore/qcontainerfwd.h>
#include <QtDebug>
#include <QtMath>
#include <qnumeric.h>

#include "../qmuparser/qmuparsererror.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/global.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/vcurvepathitem.h"
#include "../vwidgets/vmaingraphicsscene.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vVis, "v.visualization") // NOLINT

QT_WARNING_POP

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto InitPointItem(VColorRole role, QGraphicsItem *parent, qreal z = 0) -> VScaledEllipse *
{
    auto *point = new VScaledEllipse(role, parent);
    point->setZValue(1);
    point->setBrush(QBrush(Qt::NoBrush));
    point->setRect(PointRect(ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))));
    point->setPos(QPointF());
    point->setFlags(QGraphicsItem::ItemStacksBehindParent);
    point->setZValue(z);
    point->setVisible(false);
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto InitCurveItem(VColorRole role, QGraphicsItem *parent, qreal z = 0) -> VCurvePathItem *
{
    auto *curve = new VCurvePathItem(role, parent);
    curve->setBrush(QBrush(Qt::NoBrush));
    curve->setFlags(QGraphicsItem::ItemStacksBehindParent);
    curve->setZValue(z);
    curve->setVisible(false);
    return curve;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
Visualization::Visualization(const VContainer *data)
  : m_data(data)
{
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::SetLineStyle(const Qt::PenStyle &value)
{
    m_lineStyle = value;
    InitPen();
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::StartVisualMode()
{
    auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
    SCASSERT(scene != nullptr)

    this->m_scenePos = scene->getScenePos();
    RefreshGeometry();

    AddOnScene();
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::MousePos(const QPointF &scenePos)
{
    this->m_scenePos = scenePos;
    RefreshGeometry();
    RefreshToolTip();
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::InitPoint(VColorRole role, QGraphicsItem *parent, qreal z) -> VScaledEllipse *
{
    return InitPointItem(role, parent, z);
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::FindLengthFromUser(const QString &expression,
                                       const QHash<QString, QSharedPointer<VInternalVariable>> *vars, bool fromUser)
    -> qreal
{
    return VAbstractValApplication::VApp()->toPixel(FindValFromUser(expression, vars, fromUser));
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::FindValFromUser(const QString &expression,
                                    const QHash<QString, QSharedPointer<VInternalVariable>> *vars, bool fromUser)
    -> qreal
{
    qreal val = 0;
    if (expression.isEmpty())
    {
        val = 0;
    }
    else
    {
        try
        {
            // Replace line return with spaces for calc if exist
            QString formula = expression;
            if (fromUser)
            {
                formula = VAbstractApplication::VApp()->TrVars()->FormulaFromUser(
                    formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            }

            QScopedPointer<Calculator> const cal(new Calculator());
            val = cal->EvalFormula(vars, formula);

            if (qIsInf(val) || qIsNaN(val))
            {
                val = 0;
            }
        }
        catch (qmu::QmuParserError &e)
        {
            val = 0;
            qDebug() << "\nMath parser error:\n"
                     << "--------------------------------------\n"
                     << "Message:     " << e.GetMsg() << "\n"
                     << "Expression:  " << e.GetExpr() << "\n"
                     << "--------------------------------------";
        }
    }
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::CorrectAngle(qreal angle) -> qreal
{
    qreal ang = angle;
    if (angle > 360)
    {
        ang = angle - 360.0 * qFloor(angle / 360);
    }

    return (qFloor(qAbs(ang) / 5.)) * 5;
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::RefreshToolTip() const
{
    if (!m_toolTip.isEmpty())
    {
        emit ToolTip(m_toolTip);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::DrawPoint(QGraphicsEllipseItem *point, const QPointF &pos, Qt::PenStyle style)
{
    SCASSERT(point != nullptr)

    point->setPos(pos);

    QPen visPen = point->pen();
    visPen.setStyle(style);
    point->setPen(visPen);

    point->setVisible(true);
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::DrawLine(VScaledLine *lineItem, const QLineF &line, Qt::PenStyle style)
{
    SCASSERT(lineItem != nullptr)

    QPen visPen = lineItem->pen();
    visPen.setStyle(not line.isNull() ? style : Qt::NoPen);

    lineItem->setPen(visPen);
    if (not line.isNull())
    {
        lineItem->setLine(line);
    }

    lineItem->setVisible(not line.isNull());
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::DrawPath(VCurvePathItem *pathItem, const QPainterPath &path, Qt::PenStyle style,
                             Qt::PenCapStyle cap)
{
    DrawPath(pathItem, path, QVector<DirectionArrow>(), style, cap);
}

//---------------------------------------------------------------------------------------------------------------------
void Visualization::DrawPath(VCurvePathItem *pathItem, const QPainterPath &path,
                             const QVector<DirectionArrow> &directionArrows, Qt::PenStyle style, Qt::PenCapStyle cap)
{
    SCASSERT(pathItem != nullptr)

    QPen visPen = pathItem->pen();
    visPen.setStyle(style);
    visPen.setCapStyle(cap);
    pathItem->setPen(visPen);

    pathItem->setPath(path);
    pathItem->SetDirectionArrows(directionArrows);
    pathItem->setVisible(true);
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::GetPointItem(QVector<VScaledEllipse *> &points, quint32 i, VColorRole role, QGraphicsItem *parent)
    -> VScaledEllipse *
{
    if (not points.isEmpty() && static_cast<quint32>(points.size() - 1) >= i)
    {
        return points.at(static_cast<int>(i));
    }

    auto *point = InitPointItem(role, parent);
    points.append(point);
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::GetCurveItem(QVector<VCurvePathItem *> &curves, quint32 i, VColorRole role, QGraphicsItem *parent)
    -> VCurvePathItem *
{
    if (not curves.isEmpty() && static_cast<quint32>(curves.size() - 1) >= i)
    {
        return curves.at(static_cast<int>(i));
    }

    auto *point = InitCurveItem(role, parent);
    curves.append(point);
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::LengthToUser(qreal value) -> QString
{
    auto *app = VAbstractValApplication::VApp();
    return app->TrVars()->FormulaToUser(QString::number(app->fromPixel(value)), app->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
auto Visualization::AngleToUser(qreal value) -> QString
{
    auto *app = VAbstractApplication::VApp();
    return app->TrVars()->FormulaToUser(QString::number(value), app->Settings()->GetOsSeparator());
}
