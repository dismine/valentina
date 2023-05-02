/************************************************************************
 **
 **  @file   vtoolcurveintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#ifndef VTOOLCURVEINTERSECTAXIS_H
#define VTOOLCURVEINTERSECTAXIS_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vpatterndb/vformula.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

struct VToolCurveIntersectAxisInitData : VToolLinePointInitData
{
    QString formulaAngle{'0'};
    quint32 basePointId{NULL_ID};
    quint32 curveId{NULL_ID};
    QPair<QString, QString> segments{};
    QString aliasSuffix1{};
    QString aliasSuffix2{};
};

class VToolCurveIntersectAxis : public VToolLinePoint
{
    Q_OBJECT // NOLINT
public:
    virtual ~VToolCurveIntersectAxis() = default;
    virtual void SetDialog() override;

    static VToolCurveIntersectAxis *Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                           VAbstractPattern *doc, VContainer *data);
    static VToolCurveIntersectAxis *Create(VToolCurveIntersectAxisInitData &initData);

    static bool FindPoint(const QPointF &point, qreal angle, const QVector<QPointF> &curvePoints,
                          QPointF *intersectionPoint);

    static const QString ToolType;
    virtual int       type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CurveIntersectAxis)};

    VFormula     GetFormulaAngle() const;
    void         SetFormulaAngle(const VFormula &value);

    QString CurveName() const;

    virtual void ShowVisualization(bool show) override;
protected slots:
    virtual void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;
protected:
    virtual void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                            QList<quint32> &newDependencies) override;
    virtual void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void ReadToolAttributes(const QDomElement &domElement) override;
    virtual void SetVisualization() override;
    virtual auto MakeToolTip() const -> QString override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    void SetSegments(const QPair<QString, QString> &segments);
private:
    Q_DISABLE_COPY_MOVE(VToolCurveIntersectAxis) // NOLINT
    QString formulaAngle;
    quint32 curveId;
    QPair<QString, QString> m_segments{};
    QString m_aliasSuffix1{};
    QString m_aliasSuffix2{};

    explicit VToolCurveIntersectAxis(const VToolCurveIntersectAxisInitData &initData, QGraphicsItem *parent = nullptr);

    template <class Item>
    static void InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId);
    static void InitSegments(GOType curveType, qreal segLength, const VPointF *p, quint32 curveId, VContainer *data);
};

#endif // VTOOLCURVEINTERSECTAXIS_H
