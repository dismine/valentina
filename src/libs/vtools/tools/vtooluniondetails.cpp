/************************************************************************
 **
 **  @file   vtooluniondetails.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 12, 2013
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

#include "vtooluniondetails.h"

#include <QByteArray>
#include <QDomNodeList>
#include <QHash>
#include <QLineF>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QUndoStack>
#include <QtDebug>
#include <memory>

#include "../dialogs/tools/dialogtool.h"
#include "../dialogs/tools/dialoguniondetails.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vplacelabelitem.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vsplinepoint.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "nodeDetails/vnodearc.h"
#include "nodeDetails/vnodeellipticalarc.h"
#include "nodeDetails/vnodepoint.h"
#include "nodeDetails/vnodespline.h"
#include "nodeDetails/vnodesplinepath.h"
#include "nodeDetails/vtoolpiecepath.h"
#include "nodeDetails/vtoolpin.h"
#include "nodeDetails/vtoolplacelabel.h"
#include "vdatatool.h"
#include "vtoolseamallowance.h"

const QString VToolUnionDetails::ToolType = QStringLiteral("unionDetails");     // NOLINT
const QString VToolUnionDetails::TagDetail = QStringLiteral("det");             // NOLINT
const QString VToolUnionDetails::TagNode = QStringLiteral("node");              // NOLINT
const QString VToolUnionDetails::TagChildren = QStringLiteral("children");      // NOLINT
const QString VToolUnionDetails::TagChild = QStringLiteral("child");            // NOLINT
const QString VToolUnionDetails::AttrIndexD1 = QStringLiteral("indexD1");       // NOLINT
const QString VToolUnionDetails::AttrIndexD2 = QStringLiteral("indexD2");       // NOLINT
const QString VToolUnionDetails::AttrIdObject = QStringLiteral("idObject");     // NOLINT
const QString VToolUnionDetails::AttrNodeType = QStringLiteral("nodeType");     // NOLINT
const QString VToolUnionDetails::NodeTypeContour = QStringLiteral("Contour");   // NOLINT
const QString VToolUnionDetails::NodeTypeModeling = QStringLiteral("Modeling"); // NOLINT

// Current version of union tag need for backward compatibility
const quint8 VToolUnionDetails::unionVersion = UNION_VERSSION;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vToolUnion, "v.toolUnion") // NOLINT

QT_WARNING_POP

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto GetPiecePath(int piece, VAbstractPattern *doc, quint32 id) -> VPiecePath
{
    const QDomElement tool = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        throw VException(QStringLiteral("Can't get tool by id='%1'.").arg(id));
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        if (const QDomElement element = nodesList.at(i).toElement();
            not element.isNull() && element.tagName() == VToolUnionDetails::TagDetail && i + 1 == piece)
        {
            const QDomNodeList detList = element.childNodes();
            QDOM_LOOP(detList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(detList, j).toElement();
                    not element.isNull() && element.tagName() == VAbstractPattern::TagNodes)
                {
                    return VAbstractPattern::ParsePieceNodes(element);
                }
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPiece1MainPath(VAbstractPattern *doc, quint32 id) -> VPiecePath
{
    return GetPiecePath(1, doc, id);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPiece2MainPath(VAbstractPattern *doc, quint32 id) -> VPiecePath
{
    return GetPiecePath(2, doc, id);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPiece2CSAPaths(VAbstractPattern *doc, quint32 id) -> QVector<CustomSARecord>
{
    const QDomElement tool = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        throw VException(QStringLiteral("Can't get tool by id='%1'.").arg(id));
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        if (const QDomElement element = nodesList.at(i).toElement();
            not element.isNull() && element.tagName() == VToolUnionDetails::TagDetail && i + 1 == 2)
        {
            const QDomNodeList detList = element.childNodes();
            QDOM_LOOP(detList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(detList, j).toElement();
                    not element.isNull() && element.tagName() == VToolSeamAllowance::TagCSA)
                {
                    return VAbstractPattern::ParsePieceCSARecords(element);
                }
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPiece2InternalPaths(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    const QDomElement tool = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        throw VException(QStringLiteral("Can't get tool by id='%1'.").arg(id));
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        if (const QDomElement element = nodesList.at(i).toElement();
            not element.isNull() && element.tagName() == VToolUnionDetails::TagDetail && i + 1 == 2)
        {
            const QDomNodeList detList = element.childNodes();
            QDOM_LOOP(detList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(detList, j).toElement();
                    not element.isNull() && element.tagName() == VToolSeamAllowance::TagIPaths)
                {
                    return VAbstractPattern::ParsePieceInternalPaths(element);
                }
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPiece2Pins(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    const QDomElement tool = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        throw VException(QStringLiteral("Can't get tool by id='%1'.").arg(id));
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        if (const QDomElement element = nodesList.at(i).toElement();
            not element.isNull() && element.tagName() == VToolUnionDetails::TagDetail && i + 1 == 2)
        {
            const QDomNodeList detList = element.childNodes();
            QDOM_LOOP(detList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(detList, j).toElement();
                    not element.isNull() && element.tagName() == VToolSeamAllowance::TagPins)
                {
                    return VAbstractPattern::ParsePiecePointRecords(element);
                }
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPiece2PlaceLabels(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    const QDomElement tool = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        throw VException(QStringLiteral("Can't get tool by id='%1'.").arg(id));
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        if (const QDomElement element = nodesList.at(i).toElement();
            not element.isNull() && element.tagName() == VToolUnionDetails::TagDetail && i + 1 == 2)
        {
            const QDomNodeList detList = element.childNodes();
            QDOM_LOOP(detList, j)
            {
                if (const QDomElement element = QDOM_ELEMENT(detList, j).toElement();
                    not element.isNull() && element.tagName() == VToolSeamAllowance::TagPlaceLabels)
                {
                    return VAbstractPattern::ParsePiecePointRecords(element);
                }
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto DrawName(VAbstractPattern *doc, quint32 d1id, quint32 d2id) -> QString
{
    const QDomElement detail1 = doc->FindElementById(d1id, VAbstractPattern::TagDetail);
    if (detail1.isNull())
    {
        return {};
    }

    const QDomElement detail2 = doc->FindElementById(d2id, VAbstractPattern::TagDetail);
    if (detail2.isNull())
    {
        return {};
    }

    const QDomElement draw1 = detail1.parentNode().parentNode().toElement();
    if (draw1.isNull() || not draw1.hasAttribute(VAbstractPattern::AttrName))
    {
        return {};
    }

    const QDomElement draw2 = detail2.parentNode().parentNode().toElement();
    if (draw2.isNull() || not draw2.hasAttribute(VAbstractPattern::AttrName))
    {
        return {};
    }

    QString draw1Name = draw1.attribute(VAbstractPattern::AttrName);
    QString draw2Name = draw2.attribute(VAbstractPattern::AttrName);

    if (draw1Name == draw2Name)
    {
        return draw1Name;
    }

    const QDomElement pattern = draw1.parentNode().toElement();
    if (pattern.isNull())
    {
        return {};
    }

    int indexD1 = 0;
    int indexD2 = 0;
    const QDomNodeList listDraws = pattern.elementsByTagName(VAbstractPattern::TagDraw);
    for (int i = 0; i < listDraws.size(); ++i)
    {
        const QDomElement draw = listDraws.at(i).toElement();
        if (draw == draw1)
        {
            indexD1 = i;
        }

        if (draw == draw2)
        {
            indexD2 = i;
        }
    }

    if (indexD1 >= indexD2)
    {
        return draw1Name;
    }

    return draw2Name;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief BiasRotatePoint bias and rotate point.
 * @param point point.
 * @param dx bias x axis.
 * @param dy bias y axis.
 * @param pRotate point rotation.
 * @param angle angle rotation.
 */
void BiasRotatePoint(VPointF *point, qreal dx, qreal dy, const QPointF &pRotate, qreal angle)
{
    point->setX(point->x() + dx);
    point->setY(point->y() + dy);
    QLineF line(pRotate, static_cast<QPointF>(*point));
    line.setAngle(line.angle() + angle);
    point->setX(line.p2().x());
    point->setY(line.p2().y());
}

//---------------------------------------------------------------------------------------------------------------------
void PointsOnEdge(const VPiecePath &path, quint32 index, VPointF &p1, VPointF &p2, VContainer *data)
{
    VPieceNode det2p1;
    VPieceNode det2p2;
    path.NodeOnEdge(index, det2p1, det2p2);
    p1 = VPointF(*data->GeometricObject<VPointF>(det2p1.GetId()));
    p2 = VPointF(*data->GeometricObject<VPointF>(det2p2.GetId()));
}

//---------------------------------------------------------------------------------------------------------------------
void UnionInitParameters(const VToolUnionDetailsInitData &initData, const VPiecePath &d1Path, const VPiecePath &d2Path,
                         VPieceNode &det1p1, qreal &dx, qreal &dy, qreal &angle)
{
    VPieceNode det1p2;
    d1Path.NodeOnEdge(initData.indexD1, det1p1, det1p2);
    Q_UNUSED(det1p2)

    VPointF point1;
    VPointF point2;
    PointsOnEdge(d1Path, initData.indexD1, point1, point2, initData.data);

    VPointF point3;
    VPointF point4;
    PointsOnEdge(d2Path, initData.indexD2, point3, point4, initData.data);

    dx = point1.x() - point4.x();
    dy = point1.y() - point4.y();

    point3.setX(point3.x() + dx);
    point3.setY(point3.y() + dy);

    point4.setX(point4.x() + dx);
    point4.setY(point4.y() + dy);

    const auto p4p3 = QLineF(static_cast<QPointF>(point4), static_cast<QPointF>(point3));
    const auto p1p2 = QLineF(static_cast<QPointF>(point1), static_cast<QPointF>(point2));

    angle = p4p3.angleTo(p1p2);
}

//---------------------------------------------------------------------------------------------------------------------
auto AddNodePoint(const VPieceNode &node, const VToolUnionDetailsInitData &initData, QVector<quint32> &children,
                  const QString &drawName, qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    auto point = std::make_unique<VPointF>(*initData.data->GeometricObject<VPointF>(node.GetId()));
    point->setMode(Draw::Modeling);

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(point.get(), dx, dy, static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)),
                        angle);
    }

    auto point1 = std::make_unique<VPointF>(*point);

    const quint32 idObject = initData.data->AddGObject(point.release());
    children.append(idObject);
    point1->setIdObject(idObject);
    point1->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(point1.release());

    VAbstractNodeInitData initNodeData;
    initNodeData.id = id;
    initNodeData.idObject = idObject;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;
    initNodeData.scene = initData.scene;

    VNodePoint::Create(initNodeData);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto AddPin(quint32 id, const VToolUnionDetailsInitData &initData, QVector<quint32> &children, const QString &drawName,
            qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    auto point = std::make_unique<VPointF>(*initData.data->GeometricObject<VPointF>(id));
    point->setMode(Draw::Modeling);

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(point.get(), dx, dy, static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)),
                        angle);
    }

    auto point1 = std::make_unique<VPointF>(*point);

    const quint32 idObject = initData.data->AddGObject(point.release());
    children.append(idObject);
    point1->setMode(Draw::Modeling);
    const quint32 idPin = initData.data->AddGObject(point1.release());

    VToolPinInitData initNodeData;
    initNodeData.id = idPin;
    initNodeData.pointId = idObject;
    initNodeData.idObject = NULL_ID;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;

    VToolPin::Create(initNodeData);
    return idPin;
}

//---------------------------------------------------------------------------------------------------------------------
auto AddPlaceLabel(quint32 id, const VToolUnionDetailsInitData &initData, QVector<quint32> &children,
                   const QString &drawName, qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    auto label = std::make_unique<VPlaceLabelItem>(*initData.data->GeometricObject<VPlaceLabelItem>(id));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(label.get(), dx, dy, static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)),
                        angle);
    }

    label->SetCorrectionAngle(label->GetCorrectionAngle() + angle);

    VToolPlaceLabelInitData initNodeData;
    initNodeData.idObject = NULL_ID;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;
    initNodeData.width = label->GetWidthFormula();
    initNodeData.height = label->GetHeightFormula();
    initNodeData.angle = label->GetAngleFormula();
    initNodeData.visibilityTrigger = label->GetVisibilityTrigger();
    initNodeData.type = label->GetLabelType();

    auto label1 = std::make_unique<VPlaceLabelItem>(*label);

    initNodeData.centerPoint = initData.data->AddGObject(label.release());
    children.append(initNodeData.centerPoint);

    const quint32 idLabel = initData.data->AddGObject(label1.release());
    initNodeData.id = idLabel;

    VToolPlaceLabel::Create(initNodeData);
    return idLabel;
}

//---------------------------------------------------------------------------------------------------------------------
auto AddNodeArc(const VPieceNode &node, const VToolUnionDetailsInitData &initData, QVector<quint32> &children,
                const QString &drawName, qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    const QSharedPointer<VArc> arc = initData.data->GeometricObject<VArc>(node.GetId());
    auto p1 = VPointF(arc->GetP1(), QChar('A'), 0, 0);
    auto p2 = VPointF(arc->GetP2(), QChar('A'), 0, 0);
    auto center = std::make_unique<VPointF>(arc->GetCenter());

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const auto p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.get(), dx, dy, p, angle);
    }

    QLineF const l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF const l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));
    center->setMode(Draw::Modeling);
    VPointF *tmpCenter = center.release();
    const quint32 idCenter = initData.data->AddGObject(tmpCenter);
    Q_UNUSED(idCenter)
    auto arc1 = std::make_unique<VArc>(*tmpCenter, arc->GetRadius(), arc->GetFormulaRadius(), l1.angle(),
                                       QString().setNum(l1.angle()), l2.angle(), QString().setNum(l2.angle()));
    arc1->setMode(Draw::Modeling);

    auto arc2 = std::make_unique<VArc>(*arc1);

    const quint32 idObject = initData.data->AddGObject(arc1.release());
    children.append(idObject);

    arc2->setIdObject(idObject);
    arc2->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(arc2.release());

    VAbstractNodeInitData initNodeData;
    initNodeData.id = id;
    initNodeData.idObject = idObject;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;

    VNodeArc::Create(initNodeData);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto AddNodeElArc(const VPieceNode &node, const VToolUnionDetailsInitData &initData, QVector<quint32> &children,
                  const QString &drawName, qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    const QSharedPointer<VEllipticalArc> arc = initData.data->GeometricObject<VEllipticalArc>(node.GetId());
    auto p1 = VPointF(arc->GetP1(), QChar('A'), 0, 0);
    auto p2 = VPointF(arc->GetP2(), QChar('A'), 0, 0);
    auto center = std::make_unique<VPointF>(arc->GetCenter());

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const auto p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.get(), dx, dy, p, angle);
    }

    QLineF const l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF const l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));
    center->setMode(Draw::Modeling);
    VPointF *tmpCenter = center.release();
    quint32 const idCenter = initData.data->AddGObject(tmpCenter);
    Q_UNUSED(idCenter)
    auto arc1 = std::make_unique<VEllipticalArc>(
        *tmpCenter, arc->GetRadius1(), arc->GetRadius2(), arc->GetFormulaRadius1(), arc->GetFormulaRadius2(),
        l1.angle(), QString().setNum(l1.angle()), l2.angle(), QString().setNum(l2.angle()), 0, QChar('0'));
    arc1->setMode(Draw::Modeling);

    auto arc2 = std::make_unique<VEllipticalArc>(*arc1);

    const quint32 idObject = initData.data->AddGObject(arc1.release());
    children.append(idObject);

    arc2->setIdObject(idObject);
    arc2->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(arc2.release());

    VAbstractNodeInitData initNodeData;
    initNodeData.id = id;
    initNodeData.idObject = idObject;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;

    VNodeEllipticalArc::Create(initNodeData);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto AddNodeSpline(const VPieceNode &node, const VToolUnionDetailsInitData &initData, QVector<quint32> &children,
                   const QString &drawName, qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    const QSharedPointer<VAbstractCubicBezier> spline =
        initData.data->GeometricObject<VAbstractCubicBezier>(node.GetId());

    QScopedPointer<VPointF> const p1(new VPointF(spline->GetP1()));
    VPointF p2 = spline->GetP2();
    VPointF p3 = spline->GetP3();
    QScopedPointer<VPointF> const p4(new VPointF(spline->GetP4()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const auto p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(p1.data(), dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(&p3, dx, dy, p, angle);
        BiasRotatePoint(p4.data(), dx, dy, p, angle);
    }

    auto *spl = new VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4, 0, Draw::Modeling);
    const quint32 idObject = initData.data->AddGObject(spl);
    children.append(idObject);

    auto *spl1 = new VSpline(*spl);
    spl1->setIdObject(idObject);
    spl1->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(spl1);

    VAbstractNodeInitData initNodeData;
    initNodeData.id = id;
    initNodeData.idObject = idObject;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;

    VNodeSpline::Create(initNodeData);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto AddNodeSplinePath(const VPieceNode &node, const VToolUnionDetailsInitData &initData, QVector<quint32> &children,
                       const QString &drawName, qreal dx, qreal dy, quint32 pRotate, qreal angle) -> quint32
{
    auto path = std::make_unique<VSplinePath>();
    path->setMode(Draw::Modeling);
    const QSharedPointer<VAbstractCubicBezierPath> splinePath =
        initData.data->GeometricObject<VAbstractCubicBezierPath>(node.GetId());
    for (qint32 i = 1; i <= splinePath->CountSubSpl(); ++i)
    {
        const VSpline spline = splinePath->GetSpline(i);

        QScopedPointer<VPointF> const p1(new VPointF(spline.GetP1()));
        VPointF p2 = spline.GetP2();
        VPointF p3 = spline.GetP3();
        QScopedPointer<VPointF> const p4(new VPointF(spline.GetP4()));
        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            const auto p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

            BiasRotatePoint(p1.data(), dx, dy, p, angle);
            BiasRotatePoint(&p2, dx, dy, p, angle);
            BiasRotatePoint(&p3, dx, dy, p, angle);
            BiasRotatePoint(p4.data(), dx, dy, p, angle);
        }

        auto const spl = VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4);
        if (i == 1)
        {
            const qreal angle1 = spl.GetStartAngle() + 180;
            const QString angle1F = QString::number(angle1);

            path->append(VSplinePoint(*p1, angle1, angle1F, spl.GetStartAngle(), spl.GetStartAngleFormula(), 0,
                                      QChar('0'), spline.GetC1Length(), spline.GetC1LengthFormula()));
        }

        const qreal angle2 = spl.GetEndAngle() + 180;
        const QString angle2F = QString::number(angle2);
        qreal pL2 = 0;
        QString pL2F('0');
        if (i + 1 <= splinePath->CountSubSpl())
        {
            const VSpline nextSpline = splinePath->GetSpline(i + 1);
            pL2 = nextSpline.GetC1Length();
            pL2F = nextSpline.GetC1LengthFormula();
        }

        path->append(VSplinePoint(*p4, spl.GetEndAngle(), spl.GetEndAngleFormula(), angle2, angle2F,
                                  spline.GetC2Length(), spline.GetC2LengthFormula(), pL2, pL2F));
    }
    auto path1 = std::make_unique<VSplinePath>(*path);

    const quint32 idObject = initData.data->AddGObject(path.release());
    children.append(idObject);

    path1->setIdObject(idObject);
    path1->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(path1.release());

    VAbstractNodeInitData initNodeData;
    initNodeData.id = id;
    initNodeData.idObject = idObject;
    initNodeData.doc = initData.doc;
    initNodeData.data = initData.data;
    initNodeData.parse = Document::FullParse;
    initNodeData.typeCreation = Source::FromTool;
    initNodeData.idTool = initData.id;
    initNodeData.drawName = drawName;

    VNodeSplinePath::Create(initNodeData);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToNewDetail create united detail adding one node per time.
 */
void AddNodeToNewPath(const VToolUnionDetailsInitData &initData, VPiecePath &newPath, VPieceNode node,
                      QVector<quint32> &children, const QString &drawName, qreal dx = 0, qreal dy = 0,
                      quint32 pRotate = NULL_ID, qreal angle = 0);

void AddNodeToNewPath(const VToolUnionDetailsInitData &initData, VPiecePath &newPath, VPieceNode node,
                      QVector<quint32> &children, const QString &drawName, qreal dx, qreal dy, quint32 pRotate,
                      qreal angle)
{
    quint32 id = 0;
    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
            id = AddNodePoint(node, initData, children, drawName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeArc):
            id = AddNodeArc(node, initData, children, drawName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeElArc):
            id = AddNodeElArc(node, initData, children, drawName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSpline):
            id = AddNodeSpline(node, initData, children, drawName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSplinePath):
            id = AddNodeSplinePath(node, initData, children, drawName, dx, dy, pRotate, angle);
            break;
        default:
            qDebug() << "May be wrong tool type!!! Ignoring." << Q_FUNC_INFO;
            break;
    }

    node.SetId(id);
    newPath.Append(node);
}

//---------------------------------------------------------------------------------------------------------------------
void FindIndexJ(qint32 pointsD2, const VPiecePath &d2Path, quint32 indexD2, qint32 &j)
{
    if (pointsD2 == 0)
    {
        VPieceNode node1;
        VPieceNode node2;
        d2Path.NodeOnEdge(indexD2, node1, node2);
        const VPiecePath removedD2 = d2Path.RemoveEdge(indexD2);
        const int k = removedD2.indexOfNode(node2.GetId());
        SCASSERT(k != -1)
        if (k == removedD2.CountNodes() - 1)
        { // We have last node in detail, we wil begin from 0
            j = 0;
        }
        else
        { // Continue from next node
            j = k + 1;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GetTagChildren(VAbstractPattern *doc, quint32 id) -> QDomElement
{
    QDomElement toolUnion = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (toolUnion.isNull())
    {
        throw VException(QStringLiteral("Can't get tool by id='%1'.").arg(id));
    }

    QDomElement tagChildren = toolUnion.firstChildElement(VToolUnionDetails::TagChildren);

    if (tagChildren.isNull())
    {
        tagChildren = doc->createElement(VToolUnionDetails::TagChildren);
        toolUnion.appendChild(tagChildren);
    }

    return tagChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void SaveChildren(VAbstractPattern *doc, quint32 id, QDomElement section, const QVector<quint32> &children)
{
    if (!children.empty())
    {
        for (auto child : children)
        {
            section.appendChild(doc->CreateElementWithText(VToolUnionDetails::TagChild, QString().setNum(child)));
        }

        GetTagChildren(doc, id).appendChild(section);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SaveNodesChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(VAbstractPattern::TagNodes), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SaveCSAChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(VToolSeamAllowance::TagCSA), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SaveInternalPathsChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(VToolSeamAllowance::TagIPaths), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SavePinsChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(VToolSeamAllowance::TagPins), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SavePlaceLabelsChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(VToolSeamAllowance::TagPlaceLabels), children);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetChildren(VAbstractPattern *doc, quint32 id, const QString &tagName) -> QVector<quint32>
{
    const QDomElement toolUnion = doc->FindElementById(id, VAbstractPattern::TagTools);
    if (toolUnion.isNull())
    {
        return {};
    }

    const QDomElement tagChildren = toolUnion.firstChildElement(VToolUnionDetails::TagChildren);
    if (tagChildren.isNull())
    {
        return {};
    }

    const QDomElement tagNodes = tagChildren.firstChildElement(tagName);
    if (tagNodes.isNull())
    {
        return {};
    }

    const QDomNodeList listChildren = tagNodes.elementsByTagName(VToolUnionDetails::TagChild);
    QVector<quint32> childrenId;
    childrenId.reserve(listChildren.size());

    for (int i = 0; i < listChildren.size(); ++i)
    {
        if (const QDomElement domElement = listChildren.at(i).toElement(); not domElement.isNull())
        {
            childrenId.append(domElement.text().toUInt());
        }
    }
    return childrenId;
}

//---------------------------------------------------------------------------------------------------------------------
auto GetNodesChildren(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    return GetChildren(doc, id, VAbstractPattern::TagNodes);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetCSAChildren(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    return GetChildren(doc, id, VToolSeamAllowance::TagCSA);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetInternalPathsChildren(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    return GetChildren(doc, id, VToolSeamAllowance::TagIPaths);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPinChildren(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    return GetChildren(doc, id, VToolSeamAllowance::TagPins);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPlaceLabelChildren(VAbstractPattern *doc, quint32 id) -> QVector<quint32>
{
    return GetChildren(doc, id, VToolSeamAllowance::TagPlaceLabels);
}

//---------------------------------------------------------------------------------------------------------------------
auto TakeNextId(QVector<quint32> &children) -> quint32
{
    quint32 idChild = NULL_ID;
    if (not children.isEmpty())
    {
        idChild = children.takeFirst();
    }
    else
    {
        idChild = NULL_ID;
    }
    return idChild;
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodePoint(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                     quint32 pRotate, qreal angle)
{
    auto point = std::make_unique<VPointF>(*data->GeometricObject<VPointF>(node.GetId()));
    point->setMode(Draw::Modeling);
    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(point.get(), dx, dy, static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate)), angle);
    }
    data->UpdateGObject(TakeNextId(children), point.release());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeArc(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                   quint32 pRotate, qreal angle)
{
    const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(node.GetId());
    auto p1 = VPointF(arc->GetP1());
    auto p2 = VPointF(arc->GetP2());
    QScopedPointer<VPointF> const center(new VPointF(arc->GetCenter()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const auto p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.data(), dx, dy, p, angle);
    }

    QLineF const l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF const l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));

    auto arc1 = std::make_unique<VArc>(*center, arc->GetRadius(), arc->GetFormulaRadius(), l1.angle(),
                                       QString().setNum(l1.angle()), l2.angle(), QString().setNum(l2.angle()));
    arc1->setMode(Draw::Modeling);
    data->UpdateGObject(TakeNextId(children), arc1.release());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeElArc(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                     quint32 pRotate, qreal angle)
{
    const QSharedPointer<VEllipticalArc> arc = data->GeometricObject<VEllipticalArc>(node.GetId());
    auto p1 = VPointF(arc->GetP1());
    auto p2 = VPointF(arc->GetP2());
    QScopedPointer<VPointF> const center(new VPointF(arc->GetCenter()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const auto p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.data(), dx, dy, p, angle);
    }

    QLineF const l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF const l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));

    auto arc1 = std::make_unique<VEllipticalArc>(
        *center, arc->GetRadius1(), arc->GetRadius2(), arc->GetFormulaRadius1(), arc->GetFormulaRadius2(), l1.angle(),
        QString().setNum(l1.angle()), l2.angle(), QString().setNum(l2.angle()), 0, QChar('0'));
    arc1->setMode(Draw::Modeling);
    data->UpdateGObject(TakeNextId(children), arc1.release());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeSpline(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                      quint32 pRotate, qreal angle)
{
    const QSharedPointer<VAbstractCubicBezier> spline = data->GeometricObject<VAbstractCubicBezier>(node.GetId());

    QScopedPointer<VPointF> const p1(new VPointF(spline->GetP1()));
    VPointF p2 = spline->GetP2();
    VPointF p3 = spline->GetP3();
    QScopedPointer<VPointF> const p4(new VPointF(spline->GetP4()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const auto p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(p1.data(), dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(&p3, dx, dy, p, angle);
        BiasRotatePoint(p4.data(), dx, dy, p, angle);
    }

    auto spl =
        std::make_unique<VSpline>(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4, 0U, Draw::Modeling);
    data->UpdateGObject(TakeNextId(children), spl.release());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeSplinePath(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                          quint32 pRotate, qreal angle)
{
    auto path = std::make_unique<VSplinePath>();
    path->setMode(Draw::Modeling);
    const QSharedPointer<VAbstractCubicBezierPath> splinePath =
        data->GeometricObject<VAbstractCubicBezierPath>(node.GetId());
    SCASSERT(splinePath != nullptr)
    for (qint32 i = 1; i <= splinePath->CountSubSpl(); ++i)
    {
        const VSpline spline = splinePath->GetSpline(i);

        QScopedPointer<VPointF> const p1(new VPointF(spline.GetP1()));
        VPointF p2 = spline.GetP2();
        VPointF p3 = spline.GetP3();
        QScopedPointer<VPointF> const p4(new VPointF(spline.GetP4()));

        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            const auto p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

            BiasRotatePoint(p1.data(), dx, dy, p, angle);
            BiasRotatePoint(&p2, dx, dy, p, angle);
            BiasRotatePoint(&p3, dx, dy, p, angle);
            BiasRotatePoint(p4.data(), dx, dy, p, angle);
        }

        auto const spl = VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4);
        if (i == 1)
        {
            const qreal angle1 = spl.GetStartAngle() + 180;
            const QString angle1F = QString::number(angle1);

            path->append(VSplinePoint(*p1, angle1, angle1F, spl.GetStartAngle(), spl.GetStartAngleFormula(), 0,
                                      QChar('0'), spline.GetC1Length(), spline.GetC1LengthFormula()));
        }

        const qreal angle2 = spl.GetEndAngle() + 180;
        const QString angle2F = QString::number(angle2);

        qreal pL2 = 0;
        QString pL2F('0');
        if (i + 1 <= splinePath->CountSubSpl())
        {
            const VSpline nextSpline = splinePath->GetSpline(i + 1);
            pL2 = nextSpline.GetC1Length();
            pL2F = nextSpline.GetC1LengthFormula();
        }

        path->append(VSplinePoint(*p4, spl.GetEndAngle(), spl.GetEndAngleFormula(), angle2, angle2F,
                                  spline.GetC2Length(), spline.GetC2LengthFormula(), pL2, pL2F));
    }
    data->UpdateGObject(TakeNextId(children), path.release());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateNodes update nodes of united detail.
 * @param data container with variables.
 * @param node detail's node.
 * @param children list ids of all children.
 * @param dx bias node x axis.
 * @param dy bias node y axis.
 * @param pRotate point rotation.
 * @param angle angle rotation.
 */
void UpdatePathNode(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx = 0, qreal dy = 0,
                    quint32 pRotate = NULL_ID, qreal angle = 0);
void UpdatePathNode(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                    quint32 pRotate, qreal angle)
{
    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
            UpdateNodePoint(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeArc):
            UpdateNodeArc(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeElArc):
            UpdateNodeElArc(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSpline):
            UpdateNodeSpline(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSplinePath):
            UpdateNodeSplinePath(data, node, children, dx, dy, pRotate, angle);
            break;
        default:
            qDebug() << "May be wrong tool type!!! Ignoring." << Q_FUNC_INFO;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedNodes(VPiece &newDetail, const VPiece &d1, const VPiece &d2, const QString &drawName,
                       const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    const VPiecePath d1Path = d1.GetPath().RemoveEdge(initData.indexD1);
    const VPiecePath d2Path = d2.GetPath().RemoveEdge(initData.indexD2);

    for (qint32 i = 0; i < d1Path.CountNodes(); ++i)
    {
        patternGraph->AddEdge(d1Path.at(i).GetId(), initData.id);
    }

    for (qint32 i = 0; i < d2Path.CountNodes(); ++i)
    {
        patternGraph->AddEdge(d2Path.at(i).GetId(), initData.id);
    }

    const auto unitedPath = VToolUnionDetails::CalcUnitedPath(d1Path, d2Path, initData.indexD2, pRotate);

    QVector<quint32> children;
    VPiecePath newPath;

    for (const auto &[first, second] : unitedPath)
    {
        patternGraph->AddEdge(second.GetId(), initData.id);
        if (first)
        { // first piece
            AddNodeToNewPath(initData, newPath, second, children, drawName);
        }
        else
        { // second piece
            AddNodeToNewPath(initData, newPath, second, children, drawName, dx, dy, pRotate, angle);
        }
    }

    newDetail.SetPath(newPath);

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }

    SCASSERT(not children.isEmpty())
    SaveNodesChildren(initData.doc, initData.id, children);
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedDetailCSA(VPiece &newDetail, const VPiece &d, QVector<quint32> &children, const QString &drawName,
                           const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    QVector<quint32> nodeChildren;
    const QVector<CustomSARecord> records = d.GetCustomSARecords();
    for (auto record : records)
    {
        const VPiecePath path = initData.data->GetPiecePath(record.path);
        VPiecePath newPath = path;
        newPath.Clear(); // Clear nodes
        for (int i = 0; i < path.CountNodes(); ++i)
        {
            patternGraph->AddEdge(path.at(i).GetId(), initData.id);
            AddNodeToNewPath(initData, newPath, path.at(i), nodeChildren, drawName, dx, dy, pRotate, angle);
        }
        const quint32 idPath = initData.data->AddPiecePath(newPath);

        VToolPiecePathInitData initNodeData;
        initNodeData.id = idPath;
        initNodeData.idObject = NULL_ID;
        initNodeData.scene = initData.scene;
        initNodeData.doc = initData.doc;
        initNodeData.data = initData.data;
        initNodeData.parse = Document::FullParse;
        initNodeData.typeCreation = Source::FromTool;
        initNodeData.idTool = initData.id;
        initNodeData.drawName = drawName;
        initNodeData.path = newPath;

        VToolPiecePath::Create(initNodeData);
        record.path = idPath;
        newDetail.GetCustomSARecords().append(record);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedCSA(VPiece &newDetail, const VPiece &d1, const VPiece &d2, const QString &drawName,
                     const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    const QVector<CustomSARecord> records = d1.GetCustomSARecords();
    for (const auto &record : records)
    {
        newDetail.GetCustomSARecords().append(record);
    }

    QVector<quint32> children;
    CreateUnitedDetailCSA(newDetail, d2, children, drawName, initData, dx, dy, pRotate, angle);
    SaveCSAChildren(initData.doc, initData.id, children);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedDetailInternalPaths(VPiece &newDetail, const VPiece &d, QVector<quint32> &children,
                                     const QString &drawName, const VToolUnionDetailsInitData &initData, qreal dx,
                                     qreal dy, quint32 pRotate, qreal angle)
{
    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    QVector<quint32> nodeChildren;
    const QVector<quint32> internalPaths = d.GetInternalPaths();
    for (auto iPath : internalPaths)
    {
        const VPiecePath path = initData.data->GetPiecePath(iPath);
        VPiecePath newPath = path;
        newPath.Clear(); // Clear nodes

        for (int i = 0; i < path.CountNodes(); ++i)
        {
            patternGraph->AddEdge(path.at(i).GetId(), initData.id);
            AddNodeToNewPath(initData, newPath, path.at(i), nodeChildren, drawName, dx, dy, pRotate, angle);
        }
        const quint32 idPath = initData.data->AddPiecePath(newPath);

        VToolPiecePathInitData initNodeData;
        initNodeData.id = idPath;
        initNodeData.idObject = NULL_ID;
        initNodeData.scene = initData.scene;
        initNodeData.doc = initData.doc;
        initNodeData.data = initData.data;
        initNodeData.parse = Document::FullParse;
        initNodeData.typeCreation = Source::FromTool;
        initNodeData.idTool = initData.id;
        initNodeData.drawName = drawName;
        initNodeData.path = newPath;

        VToolPiecePath::Create(initNodeData);
        newDetail.GetInternalPaths().append(idPath);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedInternalPaths(VPiece &newDetail, const VPiece &d1, const VPiece &d2, const QString &drawName,
                               const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                               qreal angle)
{
    const QVector<quint32> paths = d1.GetInternalPaths();
    for (auto path : paths)
    {
        newDetail.GetInternalPaths().append(path);
    }

    QVector<quint32> children;
    CreateUnitedDetailInternalPaths(newDetail, d2, children, drawName, initData, dx, dy, pRotate, angle);
    SaveInternalPathsChildren(initData.doc, initData.id, children);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedDetailPins(VPiece &newDetail, const VPiece &d, QVector<quint32> &children, const QString &drawName,
                            const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    QVector<quint32> const nodeChildren;
    const QVector<quint32> pins = d.GetPins();
    for (auto pin : pins)
    {
        patternGraph->AddEdge(pin, initData.id);
        const quint32 id = AddPin(pin, initData, children, drawName, dx, dy, pRotate, angle);
        newDetail.GetPins().append(id);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedDetailPlaceLabels(VPiece &newDetail, const VPiece &d, QVector<quint32> &children,
                                   const QString &drawName, const VToolUnionDetailsInitData &initData, qreal dx,
                                   qreal dy, quint32 pRotate, qreal angle)
{
    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    QVector<quint32> const nodeChildren;
    const QVector<quint32> placeLabels = d.GetPlaceLabels();
    for (auto placeLabel : placeLabels)
    {
        patternGraph->AddEdge(placeLabel, initData.id);

        const quint32 id = AddPlaceLabel(placeLabel, initData, children, drawName, dx, dy, pRotate, angle);
        newDetail.GetPlaceLabels().append(id);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedPins(VPiece &newDetail, const VPiece &d1, const VPiece &d2, const QString &drawName,
                      const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    const auto pins = d1.GetPins();
    for (auto pin : pins)
    {
        newDetail.GetPins().append(pin);
    }

    QVector<quint32> children;
    CreateUnitedDetailPins(newDetail, d2, children, drawName, initData, dx, dy, pRotate, angle);
    SavePinsChildren(initData.doc, initData.id, children);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedPlaceLabels(VPiece &newDetail, const VPiece &d1, const VPiece &d2, const QString &drawName,
                             const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                             qreal angle)
{
    const auto labels = d1.GetPlaceLabels();
    for (auto label : labels)
    {
        newDetail.GetPlaceLabels().append(label);
    }

    QVector<quint32> children;
    CreateUnitedDetailPlaceLabels(newDetail, d2, children, drawName, initData, dx, dy, pRotate, angle);
    SavePlaceLabelsChildren(initData.doc, initData.id, children);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedNodes(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    QVector<quint32> children = GetNodesChildren(initData.doc, initData.id);
    if (children.isEmpty())
    {
        return;
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }

    const VPiecePath d1REPath = GetPiece1MainPath(initData.doc, initData.id).RemoveEdge(initData.indexD1);
    const VPiecePath d2REPath = GetPiece2MainPath(initData.doc, initData.id).RemoveEdge(initData.indexD2);

    const vsizetype countNodeD1 = d1REPath.CountNodes();
    const vsizetype countNodeD2 = d2REPath.CountNodes();

    for (qint32 i = 0; i < countNodeD1; ++i)
    {
        patternGraph->AddEdge(d1REPath.at(i).GetId(), initData.id);
    }

    for (qint32 i = 0; i < countNodeD2; ++i)
    {
        patternGraph->AddEdge(d2REPath.at(i).GetId(), initData.id);
    }

    // This check needed for backward compatibility
    // Remove check and "else" part if min version is 0.3.2
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 3, 2), "Time to refactor the code.");
    if (children.size() == countNodeD1 + countNodeD2 - 1)
    {
        const auto unitedPath = VToolUnionDetails::CalcUnitedPath(d1REPath, d2REPath, initData.indexD2, pRotate);

        for (const auto &[first, second] : unitedPath)
        {
            patternGraph->AddEdge(second.GetId(), initData.id);
            if (first)
            { // first piece
                UpdatePathNode(initData.data, second, children);
            }
            else
            { // second piece
                UpdatePathNode(initData.data, second, children, dx, dy, pRotate, angle);
            }
        }
    }
    else // remove if min version is 0.3.2
    {
        qint32 pointsD2 = 0; // Keeps number points the second detail, what we have already added.
        qint32 i = 0;
        const int indexOfNode = d1REPath.indexOfNode(pRotate);
        do
        {
            ++i;
            if (i > indexOfNode)
            {
                const vsizetype childrenCount = children.size();
                qint32 j = 0;
                FindIndexJ(pointsD2, d2REPath, initData.indexD2, j);
                do
                {
                    if (j >= countNodeD2)
                    {
                        j = 0;
                    }
                    patternGraph->AddEdge(d2REPath.at(j).GetId(), initData.id);
                    UpdatePathNode(initData.data, d2REPath.at(j), children, dx, dy, pRotate, angle);
                    ++pointsD2;
                    ++j;
                } while (pointsD2 < childrenCount);
                break;
            }
        } while (i < countNodeD1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FixChildren fix bug in first version of Union Details Tool.
 *
 * Bugged code produces incorrect order which breaks convention. This function fix the excpected order.
 */
auto FixChildren(QVector<quint32> records, QVector<quint32> children, VContainer *data) -> QVector<quint32>
{
    // TODO. Delete if minimal supported version is 0.7.0
    Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 7, 0), "Time to refactor the code.");
    SCASSERT(data != nullptr)

    if (records.isEmpty())
    {
        return children;
    }

    QVector<quint32> fixedChildren;
    while (not records.isEmpty())
    {
        const vsizetype childrenIndex = records.size() - 1;
        if (children.size() > childrenIndex)
        {
            fixedChildren.append(children.takeAt(childrenIndex));

            const VPiecePath path = data->GetPiecePath(records.takeFirst());
            for (int i = 0; i < path.CountNodes(); ++i)
            {
                if (children.size() > childrenIndex)
                {
                    fixedChildren.append(children.takeAt(childrenIndex));
                }
            }
        }
    }

    return fixedChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedDetailPaths(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                             qreal angle, const QVector<quint32> &records, QVector<quint32> children)
{
    if (initData.version == 1)
    {
        // TODO. Delete if minimal supported version is 0.7.0
        Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 7, 0), "Time to refactor the code.");
        // Fixing bug in first version of the tool. Mostly for backward compatibility.
        children = FixChildren(records, children, initData.data);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }

    for (auto record : records)
    {
        patternGraph->AddEdge(record, initData.id);
        const VPiecePath path = initData.data->GetPiecePath(record);

        if (initData.version == 1)
        {
            // TODO. Delete if minimal supported version is 0.7.0
            Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < FormatVersion(0, 7, 0), "Time to refactor the code.");
            const quint32 updatedId = TakeNextId(children);

            VPiecePath updatedPath(path);
            updatedPath.Clear();

            for (int j = 0; j < path.CountNodes(); ++j)
            {
                const VPieceNode &node = path.at(j);
                const quint32 id = TakeNextId(children);
                updatedPath.Append(VPieceNode(id, node.GetTypeTool(), node.GetReverse()));
                QVector<quint32> nodeChildren{id};
                patternGraph->AddEdge(path.at(j).GetId(), id);
                UpdatePathNode(initData.data, path.at(j), nodeChildren, dx, dy, pRotate, angle);
            }
            initData.data->UpdatePiecePath(updatedId, updatedPath);
        }
        else
        {
            for (int j = 0; j < path.CountNodes(); ++j)
            {
                const quint32 id = TakeNextId(children);
                QVector<quint32> nodeChildren = {id};
                patternGraph->AddEdge(path.at(j).GetId(), id);
                UpdatePathNode(initData.data, path.at(j), nodeChildren, dx, dy, pRotate, angle);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedDetailCSA(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle,
                           const QVector<CustomSARecord> &records)
{
    QVector<quint32> idRecords;
    idRecords.reserve(records.size());

    for (const auto &record : records)
    {
        idRecords.append(record.path);
    }

    UpdateUnitedDetailPaths(initData, dx, dy, pRotate, angle, idRecords, GetCSAChildren(initData.doc, initData.id));
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedDetailInternalPaths(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                                     qreal angle, const QVector<quint32> &records)
{
    UpdateUnitedDetailPaths(initData, dx, dy, pRotate, angle, records,
                            GetInternalPathsChildren(initData.doc, initData.id));
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedDetailPins(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle,
                            const QVector<quint32> &records)
{
    QVector<quint32> children = GetPinChildren(initData.doc, initData.id);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }

    for (auto record : records)
    {
        patternGraph->AddEdge(record, initData.id);
        auto point = std::make_unique<VPointF>(*initData.data->GeometricObject<VPointF>(record));
        point->setMode(Draw::Modeling);
        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            BiasRotatePoint(point.get(), dx, dy,
                            static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)), angle);
        }
        initData.data->UpdateGObject(TakeNextId(children), point.release());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedDetailPlaceLabels(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                                   qreal angle, const QVector<quint32> &records)
{
    QVector<quint32> children = GetPlaceLabelChildren(initData.doc, initData.id);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto child : std::as_const(children))
    {
        patternGraph->AddVertex(child, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, child);
    }

    for (auto record : records)
    {
        patternGraph->AddEdge(record, initData.id);
        QSharedPointer<VPlaceLabelItem> const parentLabel = initData.data->GeometricObject<VPlaceLabelItem>(record);
        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            BiasRotatePoint(parentLabel.data(), dx, dy,
                            static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)), angle);
        }
        auto label = std::make_unique<VPlaceLabelItem>();
        label->setName(parentLabel->name());
        label->setX(parentLabel->x());
        label->setY(parentLabel->y());
        label->setMx(parentLabel->mx());
        label->setMy(parentLabel->my());

        label->SetCorrectionAngle(parentLabel->GetCorrectionAngle() + angle);
        initData.data->UpdateGObject(TakeNextId(children), label.release());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedDetail(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    const QString drawName = DrawName(initData.doc, initData.d1id, initData.d2id);
    SCASSERT(not drawName.isEmpty())

    const VPiece d1 = initData.data->GetPiece(initData.d1id);
    const VPiece d2 = initData.data->GetPiece(initData.d2id);

    VPiece newDetail;

    CreateUnitedNodes(newDetail, d1, d2, drawName, initData, dx, dy, pRotate, angle);
    CreateUnitedCSA(newDetail, d1, d2, drawName, initData, dx, dy, pRotate, angle);
    CreateUnitedInternalPaths(newDetail, d1, d2, drawName, initData, dx, dy, pRotate, angle);
    CreateUnitedPins(newDetail, d1, d2, drawName, initData, dx, dy, pRotate, angle);
    CreateUnitedPlaceLabels(newDetail, d1, d2, drawName, initData, dx, dy, pRotate, angle);

    newDetail.SetName(QObject::tr("United detail"));
    QString const formulaSAWidth = d1.GetFormulaSAWidth();
    newDetail.SetFormulaSAWidth(formulaSAWidth, d1.GetSAWidth());
    newDetail.SetMx(d1.GetMx());
    newDetail.SetMy(d1.GetMy());
    newDetail.SetUnited(true);

    VToolSeamAllowanceInitData pieceInitData;
    pieceInitData.detail = newDetail;
    pieceInitData.width = formulaSAWidth;
    pieceInitData.scene = initData.scene;
    pieceInitData.doc = initData.doc;
    pieceInitData.data = initData.data;
    pieceInitData.parse = initData.parse;
    pieceInitData.typeCreation = Source::FromTool;
    pieceInitData.drawName = drawName;

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    if (VToolSeamAllowance *piece = VToolSeamAllowance::Create(pieceInitData))
    {
        piece->RefreshGeometry(true); // Refresh internal paths
        patternGraph->AddEdge(initData.id, pieceInitData.id);
    }

    auto DuplicateDetail = [&initData](quint32 id) -> void
    {
        VToolSeamAllowanceInitData initPieceData;
        initPieceData.scene = initData.scene;
        initPieceData.doc = initData.doc;
        initPieceData.parse = Document::FullParse;
        initPieceData.typeCreation = Source::FromGui;
        initPieceData.drawName = initData.doc->PieceDrawName(id);

        VContainer toolData = VAbstractPattern::getTool(id)->getData();
        initPieceData.data = &toolData;

        initPieceData.detail = initData.data->GetPiece(id);
        initPieceData.width = initPieceData.detail.GetFormulaSAWidth();

        if (VToolSeamAllowance *duplicate = VToolSeamAllowance::Duplicate(initPieceData))
        {
            duplicate->RefreshGeometry(true); // Refresh internal paths
        }
    };

    if (initData.retainPieces)
    {
        DuplicateDetail(initData.d1id);
        DuplicateDetail(initData.d2id);
    }

    auto RemoveDetail = [](quint32 id) -> void
    {
        auto *toolDet = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(id));
        SCASSERT(toolDet != nullptr);
        toolDet->RemoveWithConfirm(false);
    };

    RemoveDetail(initData.d1id);
    RemoveDetail(initData.d2id);
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedDetail(const VToolUnionDetailsInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    UpdateUnitedNodes(initData, dx, dy, pRotate, angle);
    UpdateUnitedDetailCSA(initData, dx, dy, pRotate, angle, GetPiece2CSAPaths(initData.doc, initData.id));
    UpdateUnitedDetailInternalPaths(initData, dx, dy, pRotate, angle,
                                    GetPiece2InternalPaths(initData.doc, initData.id));
    UpdateUnitedDetailPins(initData, dx, dy, pRotate, angle, GetPiece2Pins(initData.doc, initData.id));
    UpdateUnitedDetailPlaceLabels(initData, dx, dy, pRotate, angle, GetPiece2PlaceLabels(initData.doc, initData.id));
}

//---------------------------------------------------------------------------------------------------------------------
void UniteDetails(const VToolUnionDetailsInitData &initData)
{
    VPieceNode det1p1;
    qreal dx = 0;
    qreal dy = 0;
    qreal angle = 0;

    if (initData.typeCreation == Source::FromGui)
    {
        const VPiece d1 = initData.data->GetPiece(initData.d1id);
        const VPiece d2 = initData.data->GetPiece(initData.d2id);
        UnionInitParameters(initData, d1.GetPath(), d2.GetPath(), det1p1, dx, dy, angle);
        CreateUnitedDetail(initData, dx, dy, det1p1.GetId(), angle);
    }
    else
    {
        try
        {
            const VPiecePath d1Path = GetPiece1MainPath(initData.doc, initData.id);
            const VPiecePath d2Path = GetPiece2MainPath(initData.doc, initData.id);
            UnionInitParameters(initData, d1Path, d2Path, det1p1, dx, dy, angle);
            UpdateUnitedDetail(initData, dx, dy, det1p1.GetId(), angle);
        }
        catch (const VExceptionBadId &)
        {
            // There is a chance that the tool depends on an already deleted object. We can skip updating in this case.
            // We still can get error in child in case we should not get an error here. Debug will be a little bit
            // obscured. But if this happened something really bad happened.
        }
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolUnionDetails costructor.
 * @param initData global init data.
 * @param parent parent object.
 */
VToolUnionDetails::VToolUnionDetails(const VToolUnionDetailsInitData &initData, QObject *parent)
  : VAbstractTool(initData.doc, initData.data, initData.id, parent),
    d1id(initData.d1id),
    d2id(initData.d2id),
    indexD1(initData.indexD1),
    indexD2(initData.indexD2),
    version(initData.version)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolUnionDetails::getTagName() const -> QString
{
    return VAbstractPattern::TagTools;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolUnionDetails::ShowVisualization(bool show)
{
    Q_UNUSED(show)
}

//---------------------------------------------------------------------------------------------------------------------
void VToolUnionDetails::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolUnionDetails::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                               VContainer *data) -> VToolUnionDetails *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogUnionDetails> dialogTool = qobject_cast<DialogUnionDetails *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolUnionDetailsInitData initData;
    initData.d1id = dialogTool->getD1();
    initData.d2id = dialogTool->getD2();
    initData.indexD1 = static_cast<quint32>(dialogTool->getIndexD1());
    initData.indexD2 = static_cast<quint32>(dialogTool->getIndexD2());
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.retainPieces = dialogTool->RetainPieces();

    VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("union details"));
    VToolUnionDetails *tool = Create(initData);
    VAbstractApplication::VApp()->getUndoStack()->endMacro();
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param initData contains all init data.
 */
auto VToolUnionDetails::Create(VToolUnionDetailsInitData initData) -> VToolUnionDetails *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->getNextId();
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::MODELING_TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    VToolUnionDetails *tool = nullptr;
    // First add tool to file
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::UnionDetails, initData.doc);
        // Scene doesn't show this tool, so doc will destroy this object.
        tool = new VToolUnionDetails(initData);
        VAbstractPattern::AddTool(initData.id, tool);
        // Unfortunatelly doc will destroy all objects only in the end, but we should delete them before each FullParse
        initData.doc->AddToolOnRemove(tool);
    }

    UniteDetails(initData);

    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VToolUnionDetails::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());

    doc->SetAttribute(domElement, VDomDocument::AttrId, m_id);
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrIndexD1, indexD1);
    doc->SetAttribute(domElement, AttrIndexD2, indexD2);
    doc->SetAttribute(domElement, AttrVersion, unionVersion);

    AddDetail(domElement, data.GetPiece(d1id));
    AddDetail(domElement, data.GetPiece(d2id));

    AddToModeling(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddDetail add detail to xml file.
 * @param domElement tag in xml tree.
 * @param d detail.
 */
void VToolUnionDetails::AddDetail(QDomElement &domElement, const VPiece &d) const
{
    QDomElement det = doc->createElement(TagDetail);

    // nodes
    VToolSeamAllowance::AddNodes(doc, det, d);
    // custom seam allowance
    VToolSeamAllowance::AddCSARecords(doc, det, d.GetCustomSARecords());
    VToolSeamAllowance::AddInternalPaths(doc, det, d.GetInternalPaths());
    VToolSeamAllowance::AddPins(doc, det, d.GetPins());
    VToolSeamAllowance::AddPlaceLabels(doc, det, d.GetPlaceLabels());

    domElement.appendChild(det);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToModeling add tool to xml tree.
 * @param domElement tag in xml tree.
 */
void VToolUnionDetails::AddToModeling(const QDomElement &domElement)
{
    const QDomElement duplicate = doc->FindElementById(m_id);
    if (not duplicate.isNull())
    {
        throw VExceptionWrongId(tr("This id (%1) is not unique.").arg(m_id), duplicate);
    }

    const QString drawName = DrawName(doc, d1id, d2id);
    SCASSERT(not drawName.isEmpty())

    const VPatternBlockMapper *blocks = doc->PatternBlockMapper();
    QDomElement modeling = blocks->GetElement(drawName).firstChildElement(VAbstractPattern::TagModeling);
    if (not modeling.isNull())
    {
        modeling.appendChild(domElement);
    }
    else
    {
        qCCritical(vToolUnion, "Can't find tag %s.", qUtf8Printable(VAbstractPattern::TagModeling));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolUnionDetails::GetReferenceObjects() const -> QVector<quint32>
{
    QVector<quint32> list;
    const QDomElement tool = doc->FindElementById(m_id, getTagName());
    if (tool.isNull())
    {
        return list;
    }

    const auto parts = QStringList() << VAbstractPattern::TagNodes          /*0*/
                                     << VToolSeamAllowance::TagCSA          /*1*/
                                     << VToolSeamAllowance::TagIPaths       /*2*/
                                     << VToolSeamAllowance::TagPins         /*3*/
                                     << VToolSeamAllowance::TagPlaceLabels; /*4*/

    const QDomNodeList nodeList = tool.childNodes();
    QDOM_LOOP(nodeList, i)
    {
        const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
        if (element.isNull() || element.tagName() != VToolUnionDetails::TagDetail)
        {
            continue;
        }

        const QDomNodeList detList = element.childNodes();
        QDOM_LOOP(detList, j)
        {
            const QDomElement element = QDOM_ELEMENT(detList, j).toElement();
            if (element.isNull())
            {
                continue;
            }

            switch (parts.indexOf(element.tagName()))
            {
                case 0: // VAbstractPattern::TagNodes
                    list += ReferenceObjects(element, TagNode, AttrIdObject);
                    break;
                case 1: // VToolSeamAllowance::TagCSA
                case 2: // VToolSeamAllowance::TagIPaths
                    list += ReferenceObjects(element, VToolSeamAllowance::TagRecord, VAbstractPattern::AttrPath);
                    break;
                case 3: // VToolSeamAllowance::TagPins
                case 4: // VToolSeamAllowance::TagPlaceLabels
                {
                    const QDomNodeList children = element.childNodes();
                    QDOM_LOOP(children, index)
                    {
                        if (const QDomElement record = QDOM_ELEMENT(children, index).toElement();
                            not record.isNull() && record.tagName() == VToolSeamAllowance::TagRecord)
                        {
                            list.append(record.text().toUInt());
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolUnionDetails::ReferenceObjects(const QDomElement &root, const QString &tag, const QString &attribute)
    -> QVector<quint32>
{
    const QDomNodeList list = root.childNodes();
    QVector<quint32> objects;
    objects.reserve(list.size());

    QDOM_LOOP(list, i)
    {
        if (const QDomElement element = QDOM_ELEMENT(list, i).toElement();
            not element.isNull() && element.tagName() == tag)
        {
            if (const quint32 id = VAbstractPattern::GetParametrUInt(element, attribute, NULL_ID_STR); id > NULL_ID)
            {
                objects.append(id);
            }
        }
    }

    return objects;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolUnionDetails::CalcUnitedPath(const VPiecePath &d1Path, const VPiecePath &d2Path, quint32 indexD2,
                                       quint32 pRotate) -> QVector<QPair<bool, VPieceNode>>
{
    QVector<QPair<bool, VPieceNode>> path;

    const vsizetype countNodeD1 = d1Path.CountNodes();
    const vsizetype countNodeD2 = d2Path.CountNodes();

    qint32 pointsD2 = 0; // Keeps number points the second detail, that we have already added.
    qint32 i = 0;
    const int det1P1Index = d1Path.indexOfNode(pRotate);
    bool checkUniqueness = false;
    do
    {
        VPieceNode node = d1Path.at(i);
        if (checkUniqueness)
        {
            // See issue #835. Union Tool - changes in workpiece tool can't be saved because of double points
            node.SetCheckUniqueness(false);
            checkUniqueness = false;
        }
        path.append(qMakePair(true, node));
        ++i;
        if (i > det1P1Index && pointsD2 < countNodeD2 - 1)
        {
            qint32 j = 0;
            FindIndexJ(pointsD2, d2Path, indexD2, j);
            do
            {
                if (j >= countNodeD2)
                {
                    j = 0;
                }
                path.append(qMakePair(false, d2Path.at(j)));
                ++pointsD2;
                ++j;
            } while (pointsD2 < countNodeD2 - 1);
            checkUniqueness = true;
        }
    } while (i < countNodeD1);

    return path;
}
