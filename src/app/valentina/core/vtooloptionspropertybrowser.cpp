/************************************************************************
 **
 **  @file   vtooloptionspropertybrowser.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 8, 2014
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

#include "vtooloptionspropertybrowser.h"
#include "../qmuparser/qmudef.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vformula.h"
#include "../vpropertyexplorer/plugins/vboolproperty.h"
#include "../vpropertyexplorer/plugins/venumproperty.h"
#include "../vpropertyexplorer/plugins/vlabelproperty.h"
#include "../vpropertyexplorer/plugins/vlinecolorproperty.h"
#include "../vpropertyexplorer/plugins/vlinetypeproperty.h"
#include "../vpropertyexplorer/plugins/vnumberproperty.h"
#include "../vpropertyexplorer/plugins/vpointfproperty.h"
#include "../vpropertyexplorer/plugins/vstringproperty.h"
#include "../vpropertyexplorer/plugins/vtextproperty.h"
#include "../vtools/tools/backgroundimage/vbackgroundpixmapitem.h"
#include "../vtools/tools/backgroundimage/vbackgroundsvgitem.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vsimplecurve.h"
#include "../vwidgets/vsimplepoint.h"
#include "qobject.h"
#include "vformulaproperty.h"

#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyaxis.h"
#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyline.h"
#include "../vtools/tools/drawTools/operation/vtoolmove.h"
#include "../vtools/tools/drawTools/operation/vtoolrotation.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarcwithlength.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezier.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezierpath.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolellipticalarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolspline.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/tooldoublepoint/vtooltruedarts.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolalongline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolbisector.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolcurveintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolheight.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollineintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolshoulderpoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolbasepoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoollineintersect.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromarcandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromcircleandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofcontact.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersection.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectionarcs.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncircles.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../vtools/tools/drawTools/vtoolline.h"

#include <QComboBox>
#include <QDebug>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QPalette>
#include <QRegularExpression>
#include <QScrollArea>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, AttrHold, ("hold"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, AttrVisible, ("visible"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, AttrOpacity, ("opacity"_L1)) // NOLINT

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VToolOptionsPropertyBrowser::VToolOptionsPropertyBrowser(QDockWidget *parent)
  : QObject(parent)
{
    m_PropertyModel = new VPE::VPropertyModel(this);
    m_formView = new VPE::VPropertyFormView(m_PropertyModel, parent);
    m_formView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_formView);

    parent->setWidget(scroll);

    connect(m_PropertyModel, &VPE::VPropertyModel::onDataChangedByEditor, this,
            &VToolOptionsPropertyBrowser::userChangedData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ClearPropertyBrowser()
{
    m_PropertyModel->clear();
    m_propertyToId.clear();
    m_idToProperty.clear();
    m_currentItem = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowItemOptions(QGraphicsItem *item)
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were used in switch.");

    switch (item->type())
    {
        case VToolBasePoint::Type:
            ShowOptionsToolSinglePoint(item);
            break;
        case VToolEndLine::Type:
            ShowOptionsToolEndLine(item);
            break;
        case VToolAlongLine::Type:
            ShowOptionsToolAlongLine(item);
            break;
        case VToolArc::Type:
            ShowOptionsToolArc(item);
            break;
        case VToolArcWithLength::Type:
            ShowOptionsToolArcWithLength(item);
            break;
        case VToolBisector::Type:
            ShowOptionsToolBisector(item);
            break;
        case VToolCutArc::Type:
            ShowOptionsToolCutArc(item);
            break;
        case VToolCutSpline::Type:
            ShowOptionsToolCutSpline(item);
            break;
        case VToolCutSplinePath::Type:
            ShowOptionsToolCutSplinePath(item);
            break;
        case VToolHeight::Type:
            ShowOptionsToolHeight(item);
            break;
        case VToolLine::Type:
            ShowOptionsToolLine(item);
            break;
        case VToolLineIntersect::Type:
            ShowOptionsToolLineIntersect(item);
            break;
        case VToolNormal::Type:
            ShowOptionsToolNormal(item);
            break;
        case VToolPointOfContact::Type:
            ShowOptionsToolPointOfContact(item);
            break;
        case VToolPointOfIntersection::Type:
            ShowOptionsToolPointOfIntersection(item);
            break;
        case VToolPointOfIntersectionArcs::Type:
            ShowOptionsToolPointOfIntersectionArcs(item);
            break;
        case VToolPointOfIntersectionCircles::Type:
            ShowOptionsToolPointOfIntersectionCircles(item);
            break;
        case VToolPointOfIntersectionCurves::Type:
            ShowOptionsToolPointOfIntersectionCurves(item);
            break;
        case VToolShoulderPoint::Type:
            ShowOptionsToolShoulderPoint(item);
            break;
        case VToolSpline::Type:
            ShowOptionsToolSpline(item);
            break;
        case VToolCubicBezier::Type:
            ShowOptionsToolCubicBezier(item);
            break;
        case VToolSplinePath::Type:
            ShowOptionsToolSplinePath(item);
            break;
        case VToolCubicBezierPath::Type:
            ShowOptionsToolCubicBezierPath(item);
            break;
        case VToolTriangle::Type:
            ShowOptionsToolTriangle(item);
            break;
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
        case VSimplePoint::Type:
        case VSimpleCurve::Type:
            m_currentItem = item->parentItem();
            ShowItemOptions(m_currentItem);
            break;
        case VToolLineIntersectAxis::Type:
            ShowOptionsToolLineIntersectAxis(item);
            break;
        case VToolCurveIntersectAxis::Type:
            ShowOptionsToolCurveIntersectAxis(item);
            break;
        case VToolPointFromCircleAndTangent::Type:
            ShowOptionsToolPointFromCircleAndTangent(item);
            break;
        case VToolPointFromArcAndTangent::Type:
            ShowOptionsToolPointFromArcAndTangent(item);
            break;
        case VToolTrueDarts::Type:
            ShowOptionsToolTrueDarts(item);
            break;
        case VToolRotation::Type:
            ShowOptionsToolRotation(item);
            break;
        case VToolFlippingByLine::Type:
            ShowOptionsToolFlippingByLine(item);
            break;
        case VToolFlippingByAxis::Type:
            ShowOptionsToolFlippingByAxis(item);
            break;
        case VToolMove::Type:
            ShowOptionsToolMove(item);
            break;
        case VToolEllipticalArc::Type:
            ShowOptionsToolEllipticalArc(item);
            break;
        case VBackgroundPixmapItem::Type:
            ShowOptionsBackgroundPixmapItem(item);
            break;
        case VBackgroundSVGItem::Type:
            ShowOptionsBackgroundSVGItem(item);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptions()
{
    if (m_currentItem == nullptr)
    {
        return;
    }

    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were used in switch.");

    switch (m_currentItem->type())
    {
        case VToolBasePoint::Type:
            UpdateOptionsToolSinglePoint();
            break;
        case VToolEndLine::Type:
            UpdateOptionsToolEndLine();
            break;
        case VToolAlongLine::Type:
            UpdateOptionsToolAlongLine();
            break;
        case VToolArc::Type:
            UpdateOptionsToolArc();
            break;
        case VToolArcWithLength::Type:
            UpdateOptionsToolArcWithLength();
            break;
        case VToolBisector::Type:
            UpdateOptionsToolBisector();
            break;
        case VToolCutArc::Type:
            UpdateOptionsToolCutArc();
            break;
        case VToolCutSpline::Type:
            UpdateOptionsToolCutSpline();
            break;
        case VToolCutSplinePath::Type:
            UpdateOptionsToolCutSplinePath();
            break;
        case VToolHeight::Type:
            UpdateOptionsToolHeight();
            break;
        case VToolLine::Type:
            UpdateOptionsToolLine();
            break;
        case VToolLineIntersect::Type:
            UpdateOptionsToolLineIntersect();
            break;
        case VToolNormal::Type:
            UpdateOptionsToolNormal();
            break;
        case VToolPointOfContact::Type:
            UpdateOptionsToolPointOfContact();
            break;
        case VToolPointOfIntersection::Type:
            UpdateOptionsToolPointOfIntersection();
            break;
        case VToolPointOfIntersectionArcs::Type:
            UpdateOptionsToolPointOfIntersectionArcs();
            break;
        case VToolPointOfIntersectionCircles::Type:
            UpdateOptionsToolPointOfIntersectionCircles();
            break;
        case VToolPointOfIntersectionCurves::Type:
            UpdateOptionsToolPointOfIntersectionCurves();
            break;
        case VToolShoulderPoint::Type:
            UpdateOptionsToolShoulderPoint();
            break;
        case VToolSpline::Type:
            UpdateOptionsToolSpline();
            break;
        case VToolCubicBezier::Type:
            UpdateOptionsToolCubicBezier();
            break;
        case VToolSplinePath::Type:
            UpdateOptionsToolSplinePath();
            break;
        case VToolCubicBezierPath::Type:
            UpdateOptionsToolCubicBezierPath();
            break;
        case VToolTriangle::Type:
            UpdateOptionsToolTriangle();
            break;
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
            ShowItemOptions(m_currentItem->parentItem());
            break;
        case VToolLineIntersectAxis::Type:
            UpdateOptionsToolLineIntersectAxis();
            break;
        case VToolCurveIntersectAxis::Type:
            UpdateOptionsToolCurveIntersectAxis();
            break;
        case VToolPointFromCircleAndTangent::Type:
            UpdateOptionsToolPointFromCircleAndTangent();
            break;
        case VToolPointFromArcAndTangent::Type:
            UpdateOptionsToolPointFromArcAndTangent();
            break;
        case VToolTrueDarts::Type:
            UpdateOptionsToolTrueDarts();
            break;
        case VToolRotation::Type:
            UpdateOptionsToolRotation();
            break;
        case VToolFlippingByLine::Type:
            UpdateOptionsToolFlippingByLine();
            break;
        case VToolFlippingByAxis::Type:
            UpdateOptionsToolFlippingByAxis();
            break;
        case VToolMove::Type:
            UpdateOptionsToolMove();
            break;
        case VToolEllipticalArc::Type:
            UpdateOptionsToolEllipticalArc();
            break;
        case VBackgroundPixmapItem::Type:
            UpdateOptionsBackgroundPixmapItem();
            break;
        case VBackgroundSVGItem::Type:
            UpdateOptionsBackgroundSVGItem();
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::RefreshOptions()
{
    QGraphicsItem *item = m_currentItem;
    itemClicked(nullptr); // close options
    itemClicked(item);    // reopen options
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::userChangedData(VPE::VProperty *property)
{
    VPE::VProperty *prop = property;
    if (!m_propertyToId.contains(prop))
    {
        if (!m_propertyToId.contains(prop->getParent())) // Maybe we know parent
        {
            return;
        }

        prop = prop->getParent();
    }

    if (!m_currentItem)
    {
        return;
    }

    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were used in switch.");

    switch (m_currentItem->type())
    {
        case VToolBasePoint::Type:
            ChangeDataToolSinglePoint(prop);
            break;
        case VToolEndLine::Type:
            ChangeDataToolEndLine(prop);
            break;
        case VToolAlongLine::Type:
            ChangeDataToolAlongLine(prop);
            break;
        case VToolArc::Type:
            ChangeDataToolArc(prop);
            break;
        case VToolArcWithLength::Type:
            ChangeDataToolArcWithLength(prop);
            break;
        case VToolBisector::Type:
            ChangeDataToolBisector(prop);
            break;
        case VToolCutArc::Type:
            ChangeDataToolCutArc(prop);
            break;
        case VToolCutSpline::Type:
            ChangeDataToolCutSpline(prop);
            break;
        case VToolCutSplinePath::Type:
            ChangeDataToolCutSplinePath(prop);
            break;
        case VToolHeight::Type:
            ChangeDataToolHeight(prop);
            break;
        case VToolLine::Type:
            ChangeDataToolLine(prop);
            break;
        case VToolLineIntersect::Type:
            ChangeDataToolLineIntersect(prop);
            break;
        case VToolNormal::Type:
            ChangeDataToolNormal(prop);
            break;
        case VToolPointOfContact::Type:
            ChangeDataToolPointOfContact(prop);
            break;
        case VToolPointOfIntersection::Type:
            ChangeDataToolPointOfIntersection(prop);
            break;
        case VToolPointOfIntersectionArcs::Type:
            ChangeDataToolPointOfIntersectionArcs(prop);
            break;
        case VToolPointOfIntersectionCircles::Type:
            ChangeDataToolPointOfIntersectionCircles(prop);
            break;
        case VToolPointOfIntersectionCurves::Type:
            ChangeDataToolPointOfIntersectionCurves(prop);
            break;
        case VToolShoulderPoint::Type:
            ChangeDataToolShoulderPoint(prop);
            break;
        case VToolSpline::Type:
            ChangeDataToolSpline(prop);
            break;
        case VToolCubicBezier::Type:
            ChangeDataToolCubicBezier(prop);
            break;
        case VToolSplinePath::Type:
            ChangeDataToolSplinePath(prop);
            break;
        case VToolCubicBezierPath::Type:
            ChangeDataToolCubicBezierPath(prop);
            break;
        case VToolTriangle::Type:
            ChangeDataToolTriangle(prop);
            break;
        case VToolLineIntersectAxis::Type:
            ChangeDataToolLineIntersectAxis(prop);
            break;
        case VToolCurveIntersectAxis::Type:
            ChangeDataToolCurveIntersectAxis(prop);
            break;
        case VToolPointFromCircleAndTangent::Type:
            ChangeDataToolPointFromCircleAndTangent(prop);
            break;
        case VToolPointFromArcAndTangent::Type:
            ChangeDataToolPointFromArcAndTangent(prop);
            break;
        case VToolTrueDarts::Type:
            ChangeDataToolTrueDarts(prop);
            break;
        case VToolRotation::Type:
            ChangeDataToolRotation(prop);
            break;
        case VToolFlippingByLine::Type:
            ChangeDataToolFlippingByLine(prop);
            break;
        case VToolFlippingByAxis::Type:
            ChangeDataToolFlippingByAxis(prop);
            break;
        case VToolMove::Type:
            ChangeDataToolMove(prop);
            break;
        case VToolEllipticalArc::Type:
            ChangeDataToolEllipticalArc(prop);
            break;
        case VBackgroundPixmapItem::Type:
            ChangeDataBackgroundPixmapItem(prop);
            break;
        case VBackgroundSVGItem::Type:
            ChangeDataBackgroundSVGItem(prop);
            break;
        default:
            break;
    }
    VAbstractValApplication::VApp()->getSceneView()->update();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolOptionsPropertyBrowser::ComboBoxPalette() const -> QPalette
{
    QComboBox comboBox;
    comboBox.setPalette(m_formView->palette());
    return comboBox.palette();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::itemClicked(QGraphicsItem *item)
{
    if (item != nullptr)
    {
        if (not item->isEnabled())
        {
            return;
        }
    }

    if (m_currentItem == item && item != nullptr)
    {
        UpdateOptions();
        return;
    }

    m_PropertyModel->clear();
    m_propertyToId.clear();
    m_idToProperty.clear();

    if (m_currentItem != nullptr)
    {
        auto *previousTool = dynamic_cast<VAbstractTool *>(m_currentItem);
        if (previousTool != nullptr)
        {
            previousTool->ShowVisualization(false); // hide for previous tool
        }
    }

    m_currentItem = item;
    if (m_currentItem == nullptr)
    {
        m_formView->setTitle(QString());
        return;
    }

    ShowItemOptions(m_currentItem);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyFormula(const QString &propertyName, const VFormula &formula,
                                                     const QString &attrName)
{
    auto *itemLength = new VFormulaProperty(propertyName);
    itemLength->SetFormula(formula);
    AddProperty(itemLength, attrName);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool>
void VToolOptionsPropertyBrowser::AddPropertyObjectName(Tool *i, const QString &propertyName, bool readOnly)
{
    if (readOnly)
    {
        auto *itemName = new VPE::VLabelProperty(propertyName);
        itemName->setValue(i->name());
        AddProperty(itemName, AttrName);
    }
    else
    {
        auto *itemName = new VPE::VStringProperty(propertyName);
        itemName->setClearButtonEnable(true);
        itemName->setValue(i->name());
        itemName->setReadOnly(readOnly);
        AddProperty(itemName, AttrName);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyAlias(Tool *i, const QString &propertyName)
{
    auto *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(VAbstractApplication::VApp()->TrVars()->VarToUser(i->GetAliasSuffix()));
    AddProperty(itemName, AttrAlias);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyAlias1(Tool *i, const QString &propertyName)
{
    auto *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(VAbstractApplication::VApp()->TrVars()->VarToUser(i->GetAliasSuffix1()));
    AddProperty(itemName, AttrAlias1);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyAlias2(Tool *i, const QString &propertyName)
{
    auto *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(VAbstractApplication::VApp()->TrVars()->VarToUser(i->GetAliasSuffix2()));
    AddProperty(itemName, AttrAlias2);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyPointName1(Tool *i, const QString &propertyName)
{
    auto *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(i->nameP1());
    AddProperty(itemName, AttrName1);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyPointName2(Tool *i, const QString &propertyName)
{
    auto *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(i->nameP2());
    AddProperty(itemName, AttrName2);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool>
void VToolOptionsPropertyBrowser::AddPropertyOperationSuffix(Tool *i, const QString &propertyName, bool readOnly)
{
    auto *itemSuffix = new VPE::VStringProperty(propertyName);
    itemSuffix->setClearButtonEnable(true);
    itemSuffix->setValue(i->Suffix());
    itemSuffix->setReadOnly(readOnly);
    AddProperty(itemSuffix, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyParentPointName(const QString &pointName, const QString &propertyName,
                                                             const QString &propertyAttribure)
{
    auto *itemParentPoint = new VPE::VLabelProperty(propertyName);
    itemParentPoint->setValue(pointName);
    AddProperty(itemParentPoint, propertyAttribure);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyText(const QString &propertyName, const QString &text,
                                                  const QString &attrName)
{
    auto *itemText = new VPE::VTextProperty(propertyName);
    itemText->setValue(text);
    AddProperty(itemText, attrName);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyBool(const QString &propertyName, bool value, const QString &attrName)
{
    auto *itemBool = new VPE::VBoolProperty(propertyName);
    itemBool->setValue(value);
    AddProperty(itemBool, attrName);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyCrossPoint(Tool *i, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(
        QStringList({VToolOptionsPropertyBrowser::tr("First point"), VToolOptionsPropertyBrowser::tr("Second point")}));
    itemProperty->setValue(static_cast<int>(i->GetCrossCirclesPoint()) - 1);
    AddProperty(itemProperty, AttrCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyVCrossPoint(Tool *i, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList(
        {VToolOptionsPropertyBrowser::tr("Highest point"), VToolOptionsPropertyBrowser::tr("Lowest point")}));
    itemProperty->setValue(static_cast<int>(i->GetVCrossPoint()) - 1);
    AddProperty(itemProperty, AttrVCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyHCrossPoint(Tool *i, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList(
        {VToolOptionsPropertyBrowser::tr("Leftmost point"), VToolOptionsPropertyBrowser::tr("Rightmost point")}));
    itemProperty->setValue(static_cast<int>(i->GetHCrossPoint()) - 1);
    AddProperty(itemProperty, AttrHCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::AddPropertyAxisType(Tool *i, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList(
        {VToolOptionsPropertyBrowser::tr("Vertical axis"), VToolOptionsPropertyBrowser::tr("Horizontal axis")}));
    itemProperty->setValue(static_cast<int>(i->GetAxisType()) - 1);
    AddProperty(itemProperty, AttrAxisType);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool>
void VToolOptionsPropertyBrowser::AddPropertyLineType(Tool *i, const QString &propertyName,
                                                      const QMap<QString, QIcon> &styles)
{
    auto *lineTypeProperty = new VPE::VLineTypeProperty(propertyName);
    lineTypeProperty->setStyles(styles);
    const auto index = VPE::VLineTypeProperty::IndexOfStyle(styles, i->getLineType());
    if (index == -1)
    {
        qWarning() << "Can't find line style" << i->getLineType() << "in list";
    }
    lineTypeProperty->setValue(index);
    AddProperty(lineTypeProperty, AttrTypeLine);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool>
void VToolOptionsPropertyBrowser::AddPropertyCurvePenStyle(Tool *i, const QString &propertyName,
                                                           const QMap<QString, QIcon> &styles)
{
    auto *penStyleProperty = new VPE::VLineTypeProperty(propertyName);
    penStyleProperty->setStyles(styles);
    const auto index = VPE::VLineTypeProperty::IndexOfStyle(styles, i->GetPenStyle());
    if (index == -1)
    {
        qWarning() << "Can't find pen style" << i->getLineType() << "in list";
    }
    penStyleProperty->setValue(index);
    AddProperty(penStyleProperty, AttrPenStyle);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool>
void VToolOptionsPropertyBrowser::AddPropertyLineColor(Tool *i, const QString &propertyName,
                                                       const QMap<QString, QString> &colors, const QString &id)
{
    auto *lineColorProperty = new VPE::VLineColorProperty(propertyName);
    lineColorProperty->setColors(colors);
    const auto index = VPE::VLineColorProperty::IndexOfColor(colors, i->GetLineColor());
    if (index == -1)
    {
        qWarning() << "Can't find line style" << i->GetLineColor() << "in list";
    }
    lineColorProperty->setValue(index);
    AddProperty(lineColorProperty, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyApproximationScale(const QString &propertyName, qreal aScale)
{
    QMap<QString, QVariant> settings;
    settings.insert(QStringLiteral("Min"), 0);
    settings.insert(QStringLiteral("Max"), maxCurveApproximationScale);
    settings.insert(QStringLiteral("Step"), 0.1);
    settings.insert(QStringLiteral("Precision"), 1);

    auto *aScaleProperty = new VPE::VDoubleProperty(propertyName, settings);
    aScaleProperty->setValue(aScale);
    AddProperty(aScaleProperty, AttrAScale);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyOpacity(const QString &propertyName, int opacity)
{
    QMap<QString, QVariant> settings{{QStringLiteral("Min"), 0},
                                     {QStringLiteral("Max"), 100},
                                     {QStringLiteral("Step"), 1},
                                     {QStringLiteral("Suffix"), QChar('%')}};

    auto *opacityProperty = new VPE::VIntegerProperty(propertyName, settings);
    opacityProperty->setValue(opacity);
    AddProperty(opacityProperty, *AttrOpacity);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetName(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString name = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (name == i->name())
        {
            return;
        }

        i->setName(name);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetHold(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        bool hold = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toBool();
        if (hold == i->IsHold())
        {
            return;
        }

        i->SetHold(hold);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetVisible(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        bool visible = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toBool();
        if (visible == i->IsVisible())
        {
            return;
        }

        i->SetVisible(visible);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetOpacity(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        const int opacity = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toInt();
        if (VFuzzyComparePossibleNulls(opacity, i->GetOpacity()))
        {
            return;
        }

        i->SetOpacity(opacity / 100.);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetPointName(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString name = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (name == i->name())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || not VContainer::IsUnique(name, valentinaNamespace) || not rx.match(name).hasMatch())
        {
            m_idToProperty[AttrName]->setValue(i->name());
        }
        else
        {
            i->setName(name);
        }
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetPointName1(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString name = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (name == i->nameP1())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || not VContainer::IsUnique(name, valentinaNamespace) || not rx.match(name).hasMatch())
        {
            m_idToProperty[AttrName1]->setValue(i->nameP1());
        }
        else
        {
            i->setNameP1(name);
        }
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetPointName2(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString name = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (name == i->nameP2())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || not VContainer::IsUnique(name, valentinaNamespace) || not rx.match(name).hasMatch())
        {
            m_idToProperty[AttrName2]->setValue(i->nameP2());
        }
        else
        {
            i->setNameP2(name);
        }
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetOperationSuffix(VPE::VProperty *property)
{
    if (auto *item = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString suffix = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();

        if (suffix == item->Suffix())
        {
            return;
        }

        if (suffix.isEmpty())
        {
            m_idToProperty[AttrSuffix]->setValue(item->Suffix());
            return;
        }

        QRegularExpression rx(NameRegExp());
        const QStringList uniqueNames = VContainer::AllUniqueNames(valentinaNamespace);
        for (const auto &uniqueName : uniqueNames)
        {
            const QString name = uniqueName + suffix;
            if (not rx.match(name).hasMatch() || not VContainer::IsUnique(name, valentinaNamespace))
            {
                m_idToProperty[AttrSuffix]->setValue(item->Suffix());
                return;
            }
        }

        item->SetSuffix(suffix);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Type> auto VToolOptionsPropertyBrowser::GetCrossPoint(const QVariant &value) -> Type
{
    bool ok = false;
    const int val = value.toInt(&ok);

    auto cross = static_cast<Type>(1);
    if (ok)
    {
        switch (val)
        {
            case 0:
            case 1:
                cross = static_cast<Type>(val + 1);
                break;
            default:
                break;
        }
    }

    return cross;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetCrossCirclesPoint(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
        i->SetCrossCirclesPoint(GetCrossPoint<CrossCirclesPoint>(value));
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetVCrossCurvesPoint(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
        auto type = GetCrossPoint<VCrossCurvesPoint>(value);
        if (type == i->GetVCrossPoint())
        {
            return;
        }

        i->SetVCrossPoint(type);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetHCrossCurvesPoint(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
        auto type = GetCrossPoint<HCrossCurvesPoint>(value);
        if (type == i->GetHCrossPoint())
        {
            return;
        }

        i->SetHCrossPoint(GetCrossPoint<HCrossCurvesPoint>(value));
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetAxisType(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
        auto type = GetCrossPoint<AxisType>(value);
        if (type == i->GetAxisType())
        {
            return;
        }

        i->SetAxisType(type);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetNotes(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString notes = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (notes == i->GetNotes())
        {
            return;
        }

        i->SetNotes(notes);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetAlias(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString notes = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (notes == i->GetAliasSuffix())
        {
            return;
        }

        i->SetAliasSuffix(notes);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetAlias1(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString notes = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (notes == i->GetAliasSuffix1())
        {
            return;
        }

        i->SetAliasSuffix1(notes);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetAlias2(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString notes = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (notes == i->GetAliasSuffix2())
        {
            return;
        }

        i->SetAliasSuffix2(notes);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetLineType(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString type = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (type == i->getLineType())
        {
            return;
        }

        i->SetLineType(type);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetLineColor(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString color = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (color == i->GetLineColor())
        {
            return;
        }

        i->SetLineColor(color);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetFormulaLength(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
        if (formula == i->GetFormulaLength())
        {
            return;
        }

        i->SetFormulaLength(formula);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetFormulaAngle(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
        if (formula == i->GetFormulaAngle())
        {
            return;
        }

        i->SetFormulaAngle(formula);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetFormulaRadius(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
        if (formula == i->GetFormulaRadius())
        {
            return;
        }

        i->SetFormulaRadius(formula);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetFormulaF1(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
        if (formula == i->GetFormulaF1())
        {
            return;
        }

        i->SetFormulaF1(formula);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetFormulaF2(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
        if (formula == i->GetFormulaF2())
        {
            return;
        }

        i->SetFormulaF2(formula);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetPenStyle(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        QString pen = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toString();
        if (pen == i->GetPenStyle())
        {
            return;
        }

        i->SetPenStyle(pen);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetFormulaRotationAngle(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
        if (formula == i->GetFormulaRotationAngle())
        {
            return;
        }

        i->SetFormulaRotationAngle(formula);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Tool> void VToolOptionsPropertyBrowser::SetApproximationScale(VPE::VProperty *property)
{
    if (auto *i = qgraphicsitem_cast<Tool *>(m_currentItem))
    {
        double scale = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toDouble();
        if (VFuzzyComparePossibleNulls(scale, i->GetApproximationScale()))
        {
            return;
        }

        i->SetApproximationScale(scale);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddProperty(VPE::VProperty *property, const QString &id)
{
    m_propertyToId[property] = id;
    m_idToProperty[id] = property;
    m_PropertyModel->addProperty(property, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolSinglePoint(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    auto SetPosition = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolBasePoint *>(m_currentItem))
        {
            QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
            i->SetBasePointPos(value.toPointF());
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolBasePoint>(property);
            break;
        case 1: // QLatin1String("position")
            SetPosition(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolBasePoint>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolEndLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolEndLine>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolEndLine>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolEndLine>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolEndLine>(property);
            break;
        case 5: // AttrAngle
            SetFormulaAngle<VToolEndLine>(property);
            break;
        case 2: // AttrBasePoint (read only)
            break;
        case 61: // AttrNotes
            SetNotes<VToolEndLine>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolAlongLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolAlongLine>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolAlongLine>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolAlongLine>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolAlongLine>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolAlongLine>(property);
            break;
        case 2: // AttrBasePoint (read only)
        case 7: // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 8: // AttrRadius
            SetFormulaRadius<VToolArc>(property);
            break;
        case 9: // AttrAngle1
            SetFormulaF1<VToolArc>(property);
            break;
        case 10: // AttrAngle2
            SetFormulaF2<VToolArc>(property);
            break;
        case 27: // AttrTypeColor
            SetLineColor<VToolArc>(property);
            break;
        case 11: // AttrCenter (read only)
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolArc>(property);
            break;
        case 60: // AttrAScale
            SetApproximationScale<VToolArc>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolArc>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolArc>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolArcWithLength(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 8: // AttrRadius
            SetFormulaRadius<VToolArcWithLength>(property);
            break;
        case 9: // AttrAngle1
            SetFormulaF1<VToolArcWithLength>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolArcWithLength>(property);
            break;
        case 27: // AttrTypeColor
            SetLineColor<VToolArcWithLength>(property);
            break;
        case 11: // AttrCenter (read only)
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolArcWithLength>(property);
            break;
        case 60: // AttrAScale
            SetApproximationScale<VToolArcWithLength>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolArcWithLength>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolArcWithLength>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolBisector(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolBisector>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolBisector>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolBisector>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolBisector>(property);
            break;
        case 6:  // AttrFirstPoint (read only)
        case 2:  // AttrBasePoint (read only)
        case 12: // AttrThirdPoint (read only)
            break;
        case 61: // AttrNotes
            SetNotes<VToolBisector>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolTrueDarts(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 32: // AttrName1
            SetPointName1<VToolTrueDarts>(property);
            break;
        case 33: // AttrName2
            SetPointName2<VToolTrueDarts>(property);
            break;
        case 6:  // AttrFirstPoint (read only)
        case 7:  // AttrSecondPoint (read only)
        case 43: // AttrDartP1 (read only)
        case 44: // AttrDartP2 (read only)
        case 45: // AttrDartP3 (read only)
            break;
        case 61: // AttrNotes
            SetNotes<VToolTrueDarts>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCutArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCutArc>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolCutArc>(property);
            break;
        case 13: // AttrArc (read only)
            break;
        case 61: // AttrNotes
            SetNotes<VToolCutArc>(property);
            break;
        case 63: // AttrAlias1
            SetAlias1<VToolCutArc>(property);
            break;
        case 64: // AttrAlias2
            SetAlias2<VToolCutArc>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCutSpline(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCutSpline>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolCutSpline>(property);
            break;
        case 46: // AttrCurve (read only)
            break;
        case 61: // AttrNotes
            SetNotes<VToolCutSpline>(property);
            break;
        case 63: // AttrAlias1
            SetAlias1<VToolCutSpline>(property);
            break;
        case 64: // AttrAlias2
            SetAlias2<VToolCutSpline>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCutSplinePath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCutSplinePath>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolCutSplinePath>(property);
            break;
        case 46: // AttrCurve (read only)
            break;
        case 61: // AttrNotes
            SetNotes<VToolCutSplinePath>(property);
            break;
        case 63: // AttrAlias1
            SetAlias1<VToolCutSplinePath>(property);
            break;
        case 64: // AttrAlias2
            SetAlias2<VToolCutSplinePath>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolHeight(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolHeight>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolHeight>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolHeight>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolHeight>(property);
            break;
        case 2:  // AttrBasePoint (read only)
        case 16: // AttrP1Line (read only)
        case 17: // AttrP2Line (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 3: // AttrTypeLine
            SetLineType<VToolLine>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolLine>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolLine>(property);
            break;
        case 6: // AttrFirstPoint (read only)
        case 7: // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolLineIntersect(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolLineIntersect>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolLineIntersect>(property);
            break;
        case 18: // AttrP1Line1 (read only)
        case 19: // AttrP2Line1 (read only)
        case 20: // AttrP1Line2 (read only)
        case 21: // AttrP2Line2 (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolNormal(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    auto SetAngle = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolNormal *>(m_currentItem))
        {
            double value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toDouble();
            if (VFuzzyComparePossibleNulls(value, i->GetAngle()))
            {
                return;
            }

            i->SetAngle(value);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    switch (PropertiesList().indexOf(id))
    {
        case 4: // AttrLength
            SetFormulaLength<VToolNormal>(property);
            break;
        case 0: // AttrName
            SetPointName<VToolNormal>(property);
            break;
        case 5: // AttrAngle
            SetAngle(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolNormal>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolNormal>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolNormal>(property);
            break;
        case 2: // AttrBasePoint (read only)
        case 7: // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfContact(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    auto SetArcRadius = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolPointOfContact *>(m_currentItem))
        {
            auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
            if (formula == i->getArcRadius())
            {
                return;
            }

            i->setArcRadius(formula);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    switch (PropertiesList().indexOf(id))
    {
        case 8: // AttrRadius
            SetArcRadius(property);
            break;
        case 0: // AttrName
            SetPointName<VToolPointOfContact>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointOfContact>(property);
            break;
        case 11: // AttrCenter (read only)
        case 6:  // AttrFirstPoint (read only)
        case 7:  // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersection(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointOfIntersection>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointOfIntersection>(property);
            break;
        case 6: // AttrFirstPoint (read only)
        case 7: // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersectionArcs(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointOfIntersectionArcs>(property);
            break;
        case 28: // AttrCrossPoint
            SetCrossCirclesPoint<VToolPointOfIntersectionArcs>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointOfIntersectionArcs>(property);
            break;
        case 47: // AttrFirstArc (read only)
        case 48: // AttrSecondArc (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersectionCircles(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    auto SetFirstCircleRadius = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolPointOfIntersectionCircles *>(m_currentItem))
        {
            auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
            if (formula == i->GetFirstCircleRadius())
            {
                return;
            }

            i->SetFirstCircleRadius(formula);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    auto SetSecondCircleRadius = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolPointOfIntersectionCircles *>(m_currentItem))
        {
            auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
            if (formula == i->GetSecondCircleRadius())
            {
                return;
            }

            i->SetSecondCircleRadius(formula);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointOfIntersectionCircles>(property);
            break;
        case 28: // AttrCrossPoint
            SetCrossCirclesPoint<VToolPointOfIntersectionCircles>(property);
            break;
        case 29: // AttrC1Radius
            SetFirstCircleRadius(property);
            break;
        case 30: // AttrC2Radius
            SetSecondCircleRadius(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointOfIntersectionCircles>(property);
            break;
        case 49: // AttrC1Center (read only)
        case 50: // AttrC2Center (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersectionCurves(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointOfIntersectionCurves>(property);
            break;
        case 34: // AttrVCrossPoint
            SetVCrossCurvesPoint<VToolPointOfIntersectionCurves>(property);
            break;
        case 35: // AttrHCrossPoint
            SetHCrossCurvesPoint<VToolPointOfIntersectionCurves>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointOfIntersectionCurves>(property);
            break;
        case 51: // AttrCurve1 (read only)
        case 52: // AttrCurve2 (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointFromCircleAndTangent(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    auto SetCircleRadius = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolPointFromCircleAndTangent *>(m_currentItem))
        {
            auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
            if (formula == i->GetCircleRadius())
            {
                return;
            }

            i->SetCircleRadius(formula);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointFromCircleAndTangent>(property);
            break;
        case 31: // AttrCRadius
            SetCircleRadius(property);
            break;
        case 28: // AttrCrossPoint
            SetCrossCirclesPoint<VToolPointFromCircleAndTangent>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointFromCircleAndTangent>(property);
            break;
        case 53: // AttrCCenter (read only)
        case 54: // AttrTangent (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointFromArcAndTangent(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointFromArcAndTangent>(property);
            break;
        case 28: // AttrCrossPoint
            SetCrossCirclesPoint<VToolPointFromArcAndTangent>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolPointFromArcAndTangent>(property);
            break;
        case 54: // AttrTangent (read only)
        case 13: // AttrArc (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolShoulderPoint(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 4: // AttrLength
            SetFormulaLength<VToolShoulderPoint>(property);
            break;
        case 0: // AttrName
            SetPointName<VToolShoulderPoint>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolShoulderPoint>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolShoulderPoint>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolShoulderPoint>(property);
            break;
        case 6:  // AttrFirstPoint (read only)
        case 2:  // AttrBasePoint (read only)
        case 12: // AttrThirdPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolSpline(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = m_propertyToId[property];

    auto *i = qgraphicsitem_cast<VToolSpline *>(m_currentItem);
    SCASSERT(i != nullptr)

    VSpline spl = i->getSpline();
    const auto f = value.value<VFormula>();

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 9: // AttrAngle1
            if (not f.error())
            {
                spl.SetStartAngle(f.getDoubleValue(), f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 10: // AttrAngle2
            if (not f.error())
            {
                spl.SetEndAngle(f.getDoubleValue(), f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 36: // AttrLength1
            if (not f.error() && f.getDoubleValue() >= 0)
            {
                spl.SetC1Length(VAbstractValApplication::VApp()->toPixel(f.getDoubleValue()),
                                f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 37: // AttrLength2
            if (not f.error() && f.getDoubleValue() >= 0)
            {
                spl.SetC2Length(VAbstractValApplication::VApp()->toPixel(f.getDoubleValue()),
                                f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 27: // AttrTypeColor
            SetLineColor<VToolSpline>(property);
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolSpline>(property);
            break;
        case 60: // AttrAScale
            spl.SetApproximationScale(value.toDouble());
            i->setSpline(spl);
            break;
        case 61: // AttrNotes
            SetNotes<VToolSpline>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolSpline>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCubicBezier(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 27: // AttrTypeColor
            SetLineColor<VToolCubicBezier>(property);
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolCubicBezier>(property);
            break;
        case 60: // AttrAScale
        {
            auto *i = qgraphicsitem_cast<VToolCubicBezier *>(m_currentItem);
            SCASSERT(i != nullptr)

            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
            VCubicBezier spl = i->getSpline();
            spl.SetApproximationScale(value.toDouble());
            i->setSpline(spl);
            break;
        }
        case 61: // AttrNotes
            SetNotes<VToolCubicBezier>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolCubicBezier>(property);
            break;
        case 55: // AttrPoint1 (read only)
        case 56: // AttrPoint2 (read only)
        case 57: // AttrPoint3 (read only)
        case 58: // AttrPoint4 (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolSplinePath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 27: // AttrTypeColor
            SetLineColor<VToolSplinePath>(property);
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolSplinePath>(property);
            break;
        case 60: // AttrAScale
        {
            auto *i = qgraphicsitem_cast<VToolSplinePath *>(m_currentItem);
            SCASSERT(i != nullptr)

            QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
            VSplinePath spl = i->getSplinePath();
            spl.SetApproximationScale(value.toDouble());
            i->setSplinePath(spl);
            break;
        }
        case 61: // AttrNotes
            SetNotes<VToolSplinePath>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolSplinePath>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCubicBezierPath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 27: // AttrTypeColor
            SetLineColor<VToolCubicBezierPath>(property);
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolCubicBezierPath>(property);
            break;
        case 60: // AttrAScale
        {
            auto *i = qgraphicsitem_cast<VToolCubicBezierPath *>(m_currentItem);
            SCASSERT(i != nullptr)

            QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
            VCubicBezierPath spl = i->getSplinePath();
            spl.SetApproximationScale(value.toDouble());
            i->setSplinePath(spl);
            break;
        }
        case 61: // AttrNotes
            SetNotes<VToolCubicBezierPath>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolCubicBezierPath>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolTriangle(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolTriangle>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolTriangle>(property);
            break;
        case 23: // AttrAxisP1 (read only)
        case 24: // AttrAxisP2 (read only)
        case 6:  // AttrFirstPoint (read only)
        case 7:  // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolLineIntersectAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolLineIntersectAxis>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolLineIntersectAxis>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolLineIntersectAxis>(property);
            break;
        case 5: // AttrAngle
            SetFormulaAngle<VToolLineIntersectAxis>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolLineIntersectAxis>(property);
            break;
        case 2: // AttrBasePoint (read only)
        case 6: // AttrFirstPoint (read only)
        case 7: // AttrSecondPoint (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCurveIntersectAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCurveIntersectAxis>(property);
            break;
        case 3: // AttrTypeLine
            SetLineType<VToolCurveIntersectAxis>(property);
            break;
        case 26: // AttrTypeLineColor
            SetLineColor<VToolCurveIntersectAxis>(property);
            break;
        case 5: // AttrAngle
            SetFormulaAngle<VToolCurveIntersectAxis>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolCurveIntersectAxis>(property);
            break;
        case 2:  // AttrBasePoint (read only)
        case 46: // AttrCurve (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolRotation(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            SetOperationSuffix<VToolRotation>(property);
            break;
        case 5: // AttrAngle
            SetFormulaAngle<VToolRotation>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolRotation>(property);
            break;
        case 11: // AttrCenter (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolMove(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            SetOperationSuffix<VToolMove>(property);
            break;
        case 5: // AttrAngle
            SetFormulaAngle<VToolMove>(property);
            break;
        case 4: // AttrLength
            SetFormulaLength<VToolMove>(property);
            break;
        case 42: // AttrRotationAngle
            SetFormulaRotationAngle<VToolMove>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolMove>(property);
            break;
        case 11: // AttrCenter (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolFlippingByLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            SetOperationSuffix<VToolFlippingByLine>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolFlippingByLine>(property);
            break;
        case 6: // AttrFirstPoint
        case 7: // AttrSecondPoint
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolFlippingByAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    switch (PropertiesList().indexOf(id))
    {
        case 39: // AttrAxisType
            SetAxisType<VToolFlippingByAxis>(property);
            break;
        case 38: // AttrSuffix
            SetOperationSuffix<VToolFlippingByAxis>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolFlippingByAxis>(property);
            break;
        case 11: // AttrCenter (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolEllipticalArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];

    auto SetFormulaRadius1 = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolEllipticalArc *>(m_currentItem))
        {
            auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
            if (formula == i->GetFormulaRadius1())
            {
                return;
            }

            i->SetFormulaRadius1(formula);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    auto SetFormulaRadius2 = [this](VPE::VProperty *property)
    {
        if (auto *i = qgraphicsitem_cast<VToolEllipticalArc *>(m_currentItem))
        {
            auto formula = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).value<VFormula>();
            if (formula == i->GetFormulaRadius2())
            {
                return;
            }

            i->SetFormulaRadius2(formula);
        }
        else
        {
            qWarning() << "Can't cast item";
        }
    };

    switch (PropertiesList().indexOf(id))
    {
        case 0:              // AttrName
            Q_UNREACHABLE(); // The attribute is read only
            break;
        case 40: // AttrRadius1
            SetFormulaRadius1(property);
            break;
        case 41: // AttrRadius2
            SetFormulaRadius2(property);
            break;
        case 9: // AttrAngle1
            SetFormulaF1<VToolEllipticalArc>(property);
            break;
        case 10: // AttrAngle2
            SetFormulaF2<VToolEllipticalArc>(property);
            break;
        case 42: // AttrRotationAngle
            SetFormulaRotationAngle<VToolEllipticalArc>(property);
            break;
        case 27: // AttrColor
            SetLineColor<VToolEllipticalArc>(property);
            break;
        case 62: // AttrAlias
            SetAlias<VToolEllipticalArc>(property);
            break;
        case 59: // AttrPenStyle
            SetPenStyle<VToolEllipticalArc>(property);
            break;
        case 60: // AttrAScale
            SetApproximationScale<VToolEllipticalArc>(property);
            break;
        case 61: // AttrNotes
            SetNotes<VToolEllipticalArc>(property);
            break;
        case 11: // AttrCenter (read only)
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataBackgroundPixmapItem(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId.value(property);

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetName<VBackgroundPixmapItem>(property);
            break;
        case 65: // AttrHold
            SetHold<VBackgroundPixmapItem>(property);
            break;
        case 66: // AttrVisible
            SetVisible<VBackgroundPixmapItem>(property);
            break;
        case 67: // AttrOpacity
            SetOpacity<VBackgroundPixmapItem>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataBackgroundSVGItem(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId.value(property);

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetName<VBackgroundSVGItem>(property);
            break;
        case 65: // AttrHold
            SetHold<VBackgroundSVGItem>(property);
            break;
        case 66: // AttrVisible
            SetVisible<VBackgroundSVGItem>(property);
            break;
        case 67: // AttrOpacity
            SetOpacity<VBackgroundPixmapItem>(property);
            break;
        default:
            qWarning() << "Unknown property type. id = " << id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolSinglePoint(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolBasePoint *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Base point"));

    AddPropertyObjectName(i, tr("Point label:"));

    auto *itemPosition = new VPE::VPointFProperty(tr("Position:"));
    itemPosition->setValue(i->GetBasePointPos());
    AddProperty(itemPosition, QStringLiteral("position"));

    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolEndLine(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolEndLine *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point at distance and angle"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("Base point:"), AttrBasePoint);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolAlongLine(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolAlongLine *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point at distance along line"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("First point:"), AttrBasePoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("Second point:"), AttrSecondPoint);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolArc(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolArc *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Arc"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Name:"), true);
    AddPropertyParentPointName(i->CenterPointName(), tr("Center point:"), AttrCenter);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius(), AttrRadius);
    AddPropertyFormula(tr("First angle:"), i->GetFormulaF1(), AttrAngle1);
    AddPropertyFormula(tr("Second angle:"), i->GetFormulaF2(), AttrAngle2);
    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyCurvePenStyle(
        i, tr("Pen style:"),
        CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), i->GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolArcWithLength(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolArcWithLength *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Arc with given length"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Name:"), true);
    AddPropertyParentPointName(i->CenterPointName(), tr("Center point:"), AttrCenter);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius(), AttrRadius);
    AddPropertyFormula(tr("First angle:"), i->GetFormulaF1(), AttrAngle1);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyCurvePenStyle(
        i, tr("Pen style:"),
        CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), i->GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolBisector(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolBisector *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point along bisector"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->FirstPointName(), tr("First point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->BasePointName(), tr("Second point:"), AttrBasePoint);
    AddPropertyParentPointName(i->ThirdPointName(), tr("Third point:"), AttrThirdPoint);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolTrueDarts(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolTrueDarts *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("True darts"));

    AddPropertyPointName1(i, tr("Point 1 label:"));
    AddPropertyPointName2(i, tr("Point 2 label:"));
    AddPropertyParentPointName(i->BaseLineP1Name(), tr("First base point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->BaseLineP2Name(), tr("Second base point:"), AttrSecondPoint);
    AddPropertyParentPointName(i->DartP1Name(), tr("First dart point:"), AttrDartP1);
    AddPropertyParentPointName(i->DartP2Name(), tr("First dart point:"), AttrDartP2);
    AddPropertyParentPointName(i->DartP3Name(), tr("First dart point:"), AttrDartP3);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCutArc(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolCutArc *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Cut arc tool"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->CurveName(), tr("Arc:"), AttrArc);
    AddPropertyAlias1(i, tr("Alias1:"));
    AddPropertyAlias2(i, tr("Alias2:"));
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCutSpline(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolCutSpline *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool for segmenting a curve"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->CurveName(), tr("Curve:"), AttrCurve);
    AddPropertyAlias1(i, tr("Alias1:"));
    AddPropertyAlias2(i, tr("Alias2:"));
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCutSplinePath(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolCutSplinePath *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool segment a pathed curve"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->CurveName(), tr("Curve:"), AttrCurve);
    AddPropertyAlias1(i, tr("Alias1:"));
    AddPropertyAlias2(i, tr("Alias2:"));
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolHeight(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolHeight *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Perpendicular point along line"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("Base point:"), AttrBasePoint);
    AddPropertyParentPointName(i->FirstLinePointName(), tr("First line point:"), AttrP1Line);
    AddPropertyParentPointName(i->SecondLinePointName(), tr("Second line point:"), AttrP2Line);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolLine(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolLine *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Line between points"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyParentPointName(i->FirstPointName(), tr("First point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("Second point:"), AttrSecondPoint);
    QMap<QString, QIcon> styles =
        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text));
    styles.remove(TypeLineNone);
    AddPropertyLineType(i, tr("Line type:"), styles);
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolLineIntersect(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolLineIntersect *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point at line intersection"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->Line1P1Name(), tr("First line (first point):"), AttrP1Line1);
    AddPropertyParentPointName(i->Line1P2Name(), tr("First line (second point):"), AttrP2Line1);
    AddPropertyParentPointName(i->Line2P1Name(), tr("Second line (first point):"), AttrP1Line2);
    AddPropertyParentPointName(i->Line2P2Name(), tr("Second line (second point):"), AttrP2Line2);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolNormal(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolNormal *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point along perpendicular"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("First point:"), AttrBasePoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("Second point:"), AttrSecondPoint);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);

    auto *itemAngle = new VPE::VDoubleProperty(tr("Additional angle degrees:"));
    itemAngle->setValue(i->GetAngle());
    itemAngle->setSetting(QStringLiteral("Min"), -360);
    itemAngle->setSetting(QStringLiteral("Max"), 360);
    itemAngle->setSetting(QStringLiteral("Precision"), 3);
    AddProperty(itemAngle, AttrAngle);

    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfContact(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointOfContact *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point of intersection circle and segment"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->ArcCenterPointName(), tr("Center of arc:"), AttrCenter);
    AddPropertyParentPointName(i->FirstPointName(), tr("Top of the line:"), AttrFirstPoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("End of the line:"), AttrSecondPoint);
    AddPropertyFormula(tr("Radius:"), i->getArcRadius(), AttrRadius);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersection(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersection *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool to make point from x & y of two other points"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->FirstPointName(), tr("X: vertical point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("Y: horizontal point:"), AttrSecondPoint);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersectionArcs(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool to make point from intersection two arcs"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->FirstArcName(), tr("First arc:"), AttrFirstArc);
    AddPropertyParentPointName(i->SecondArcName(), tr("Second arc:"), AttrSecondArc);
    AddPropertyCrossPoint(i, tr("Take:"));
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersectionCircles(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersectionCircles *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool to make point from intersection two circles"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->FirstCircleCenterPointName(), tr("Center of the first circle:"), AttrC1Center);
    AddPropertyParentPointName(i->SecondCircleCenterPointName(), tr("Center of the second circle:"), AttrC2Center);
    AddPropertyFormula(tr("First circle radius:"), i->GetFirstCircleRadius(), AttrC1Radius);
    AddPropertyFormula(tr("Second circle radius:"), i->GetSecondCircleRadius(), AttrC2Radius);
    AddPropertyCrossPoint(i, tr("Take:"));
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersectionCurves(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool to make point from intersection two curves"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->FirstCurveName(), tr("First curve:"), AttrCurve1);
    AddPropertyParentPointName(i->SecondCurveName(), tr("Second curve:"), AttrCurve2);
    AddPropertyVCrossPoint(i, tr("Vertical correction:"));
    AddPropertyHCrossPoint(i, tr("Horizontal correction:"));
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointFromCircleAndTangent(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointFromCircleAndTangent *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool to make point from circle and tangent"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->CircleCenterPointName(), tr("Center of the circle:"), AttrCCenter);
    AddPropertyParentPointName(i->TangentPointName(), tr("Tangent point:"), AttrTangent);
    AddPropertyFormula(tr("Circle radius:"), i->GetCircleRadius(), AttrCRadius);
    AddPropertyCrossPoint(i, tr("Take:"));
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointFromArcAndTangent(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool to make point from arc and tangent"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->TangentPointName(), tr("Tangent point:"), AttrTangent);
    AddPropertyParentPointName(i->ArcName(), tr("Arc:"), AttrArc);
    AddPropertyCrossPoint(i, tr("Take:"));
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolShoulderPoint(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolShoulderPoint *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Special point on shoulder"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("First point:"), AttrBasePoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("Second point:"), AttrSecondPoint);
    AddPropertyParentPointName(i->ShoulderPointName(), tr("Third point:"), AttrThirdPoint);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolSpline(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolSpline *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Curve tool"));

    const auto spl = i->getSpline();

    AddPropertyObjectName(i, tr("Name:"), true);

    VFormula angle1(spl.GetStartAngleFormula(), i->getData());
    angle1.setCheckZero(false);
    angle1.setToolId(i->getId());
    angle1.setPostfix(degreeSymbol);
    angle1.Eval();
    AddPropertyFormula(tr("C1: angle:"), angle1, AttrAngle1);

    VFormula length1(spl.GetC1LengthFormula(), i->getData());
    length1.setCheckZero(false);
    length1.setToolId(i->getId());
    length1.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    length1.Eval();
    AddPropertyFormula(tr("C1: length:"), length1, AttrLength1);

    VFormula angle2(spl.GetEndAngleFormula(), i->getData());
    angle2.setCheckZero(false);
    angle2.setToolId(i->getId());
    angle2.setPostfix(degreeSymbol);
    angle2.Eval();
    AddPropertyFormula(tr("C2: angle:"), angle2, AttrAngle2);

    VFormula length2(spl.GetC2LengthFormula(), i->getData());
    length2.setCheckZero(false);
    length2.setToolId(i->getId());
    length2.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    length2.Eval();
    AddPropertyFormula(tr("C2: length:"), length2, AttrLength2);

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyCurvePenStyle(
        i, tr("Pen style:"),
        CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), spl.GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCubicBezier(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolCubicBezier *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Cubic bezier curve"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Name:"), true);
    AddPropertyParentPointName(i->FirstPointName(), tr("First point:"), AttrPoint1);
    AddPropertyParentPointName(i->SecondPointName(), tr("Second point:"), AttrPoint2);
    AddPropertyParentPointName(i->ThirdPointName(), tr("Third point:"), AttrPoint3);
    AddPropertyParentPointName(i->ForthPointName(), tr("Fourth point:"), AttrPoint4);
    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyCurvePenStyle(
        i, tr("Pen style:"),
        CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), i->getSpline().GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolSplinePath(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolSplinePath *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool for path curve"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Name:"), true);
    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyCurvePenStyle(
        i, tr("Pen style:"),
        CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), i->getSplinePath().GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCubicBezierPath(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolCubicBezierPath *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool cubic bezier curve"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Name:"), true);
    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyCurvePenStyle(
        i, tr("Pen style:"),
        CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), i->getSplinePath().GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolTriangle(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolTriangle *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool triangle"));

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->AxisP1Name(), tr("First point of axis:"), AttrAxisP1);
    AddPropertyParentPointName(i->AxisP2Name(), tr("Second point of axis:"), AttrAxisP2);
    AddPropertyParentPointName(i->FirstPointName(), tr("First point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->SecondPointName(), tr("Second point:"), AttrSecondPoint);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolLineIntersectAxis(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolLineIntersectAxis *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point intersection line and axis"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("Axis point:"), AttrBasePoint);
    AddPropertyParentPointName(i->FirstLinePoint(), tr("First line point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->SecondLinePoint(), tr("Second line point:"), AttrSecondPoint);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCurveIntersectAxis(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolCurveIntersectAxis *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Point intersection curve and axis"));

    QPalette comboBoxPalette = ComboBoxPalette();

    AddPropertyObjectName(i, tr("Point label:"));
    AddPropertyParentPointName(i->BasePointName(), tr("Axis point:"), AttrBasePoint);
    AddPropertyParentPointName(i->CurveName(), tr("Curve:"), AttrCurve);
    AddPropertyLineType(i, tr("Line type:"),
                        LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)));
    AddPropertyLineColor(i, tr("Line color:"), VAbstractTool::ColorsList(), AttrLineColor);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolRotation(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolRotation *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool rotation"));

    AddPropertyOperationSuffix(i, tr("Suffix:"));
    AddPropertyParentPointName(i->OriginPointName(), tr("Origin point:"), AttrCenter);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolMove(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolMove *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool move"));

    AddPropertyOperationSuffix(i, tr("Suffix:"));
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyFormula(tr("Rotation angle:"), i->GetFormulaRotationAngle(), AttrRotationAngle);
    AddPropertyParentPointName(i->OriginPointName(), tr("Rotation origin point:"), AttrCenter);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolFlippingByLine(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolFlippingByLine *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool flipping by line"));

    AddPropertyOperationSuffix(i, tr("Suffix:"));
    AddPropertyParentPointName(i->FirstLinePointName(), tr("First line point:"), AttrFirstPoint);
    AddPropertyParentPointName(i->SecondLinePointName(), tr("Second line point:"), AttrSecondPoint);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolFlippingByAxis(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolFlippingByAxis *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Tool flipping by axis"));

    AddPropertyAxisType(i, tr("Axis type:"));
    AddPropertyOperationSuffix(i, tr("Suffix:"));
    AddPropertyParentPointName(i->OriginPointName(), tr("Origin point:"), AttrCenter);
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolEllipticalArc(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VToolEllipticalArc *>(item);
    i->ShowVisualization(true);
    m_formView->setTitle(tr("Elliptical arc"));

    AddPropertyObjectName(i, tr("Name:"), true);
    AddPropertyParentPointName(i->CenterPointName(), tr("Center point:"), AttrCenter);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius1(), AttrRadius1);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius2(), AttrRadius2);
    AddPropertyFormula(tr("First angle:"), i->GetFormulaF1(), AttrAngle1);
    AddPropertyFormula(tr("Second angle:"), i->GetFormulaF2(), AttrAngle2);
    AddPropertyFormula(tr("Rotation angle:"), i->GetFormulaRotationAngle(), AttrRotationAngle);
    AddPropertyAlias(i, tr("Alias:"));
    AddPropertyLineColor(i, tr("Color:"), VAbstractTool::ColorsList(), AttrColor);
    AddPropertyApproximationScale(tr("Approximation scale:"), i->GetApproximationScale());
    AddPropertyText(tr("Notes:"), i->GetNotes(), AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsBackgroundPixmapItem(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VBackgroundPixmapItem *>(item);
    m_formView->setTitle(tr("Background image"));

    AddPropertyObjectName(i, tr("Name:"), false);
    AddPropertyBool(tr("Hold:"), i->IsHold(), *AttrHold);
    AddPropertyBool(tr("Visible:"), i->IsVisible(), *AttrVisible);
    AddPropertyOpacity(tr("Opacity:"), static_cast<int>(i->GetOpacity() * 100));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsBackgroundSVGItem(QGraphicsItem *item)
{
    auto *i = qgraphicsitem_cast<VBackgroundSVGItem *>(item);
    m_formView->setTitle(tr("Background image"));

    AddPropertyObjectName(i, tr("Name:"), false);
    AddPropertyBool(tr("Hold:"), i->IsHold(), *AttrHold);
    AddPropertyBool(tr("Visible:"), i->IsVisible(), *AttrVisible);
    AddPropertyOpacity(tr("Opacity:"), static_cast<int>(i->GetOpacity() * 100));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolSinglePoint()
{
    auto *i = qgraphicsitem_cast<VToolBasePoint *>(m_currentItem);
    m_idToProperty[AttrName]->setValue(i->name());
    m_idToProperty[QStringLiteral("position")]->setValue(i->GetBasePointPos());
    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolEndLine()
{
    auto *i = qgraphicsitem_cast<VToolEndLine *>(m_currentItem);
    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    m_idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolAlongLine()
{
    auto *i = qgraphicsitem_cast<VToolAlongLine *>(m_currentItem);
    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolArc()
{
    auto *i = qgraphicsitem_cast<VToolArc *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueRadius;
    valueRadius.setValue(i->GetFormulaRadius());
    m_idToProperty[AttrRadius]->setValue(valueRadius);

    QVariant valueFirstAngle;
    valueFirstAngle.setValue(i->GetFormulaF1());
    m_idToProperty[AttrAngle1]->setValue(valueFirstAngle);

    QVariant valueSecondAngle;
    valueSecondAngle.setValue(i->GetFormulaF2());
    m_idToProperty[AttrAngle2]->setValue(valueSecondAngle);

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->GetPenStyle());
        m_idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrColor]->setValue(index);
    }

    QVariant valueCenterPoint;
    valueCenterPoint.setValue(i->CenterPointName());
    m_idToProperty[AttrCenter]->setValue(valueCenterPoint);

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(i->GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolArcWithLength()
{
    auto *i = qgraphicsitem_cast<VToolArcWithLength *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueRadius;
    valueRadius.setValue(i->GetFormulaRadius());
    m_idToProperty[AttrRadius]->setValue(valueRadius);

    QVariant valueFirstAngle;
    valueFirstAngle.setValue(i->GetFormulaF1());
    m_idToProperty[AttrAngle1]->setValue(valueFirstAngle);

    QVariant valueLength;
    valueLength.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueLength);

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->GetPenStyle());
        m_idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrColor]->setValue(index);
    }

    QVariant valueCenterPoint;
    valueCenterPoint.setValue(i->CenterPointName());
    m_idToProperty[AttrCenter]->setValue(valueCenterPoint);

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(i->GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolBisector()
{
    auto *i = qgraphicsitem_cast<VToolBisector *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstPointName());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueThirdPoint;
    valueThirdPoint.setValue(i->ThirdPointName());
    m_idToProperty[AttrThirdPoint]->setValue(valueThirdPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolTrueDarts()
{
    auto *i = qgraphicsitem_cast<VToolTrueDarts *>(m_currentItem);

    m_idToProperty[AttrName1]->setValue(i->nameP1());
    m_idToProperty[AttrName2]->setValue(i->nameP2());

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->BaseLineP1Name());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->BaseLineP2Name());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    QVariant valueDrartFirstPoint;
    valueDrartFirstPoint.setValue(i->DartP1Name());
    m_idToProperty[AttrDartP1]->setValue(valueDrartFirstPoint);

    QVariant valueDartSecondPoint;
    valueDartSecondPoint.setValue(i->DartP2Name());
    m_idToProperty[AttrDartP2]->setValue(valueDartSecondPoint);

    QVariant valueDartThirdPoint;
    valueDartThirdPoint.setValue(i->DartP3Name());
    m_idToProperty[AttrDartP3]->setValue(valueDartThirdPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCutArc()
{
    auto *i = qgraphicsitem_cast<VToolCutArc *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueArc;
    valueArc.setValue(i->CurveName());
    m_idToProperty[AttrArc]->setValue(valueArc);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias1]->setValue(i->GetAliasSuffix1());
    m_idToProperty[AttrAlias2]->setValue(i->GetAliasSuffix2());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCutSpline()
{
    auto *i = qgraphicsitem_cast<VToolCutSpline *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueCurve;
    valueCurve.setValue(i->CurveName());
    m_idToProperty[AttrCurve]->setValue(valueCurve);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias1]->setValue(i->GetAliasSuffix1());
    m_idToProperty[AttrAlias2]->setValue(i->GetAliasSuffix2());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCutSplinePath()
{
    auto *i = qgraphicsitem_cast<VToolCutSplinePath *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueCurve;
    valueCurve.setValue(i->CurveName());
    m_idToProperty[AttrCurve]->setValue(valueCurve);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias1]->setValue(i->GetAliasSuffix1());
    m_idToProperty[AttrAlias2]->setValue(i->GetAliasSuffix2());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolHeight()
{
    auto *i = qgraphicsitem_cast<VToolHeight *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueFirstLinePoint;
    valueFirstLinePoint.setValue(i->FirstLinePointName());
    m_idToProperty[AttrP1Line]->setValue(valueFirstLinePoint);

    QVariant valueSecondLinePoint;
    valueSecondLinePoint.setValue(i->SecondLinePointName());
    m_idToProperty[AttrP2Line]->setValue(valueSecondLinePoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolLine()
{
    auto *i = qgraphicsitem_cast<VToolLine *>(m_currentItem);

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstPointName());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolLineIntersect()
{
    auto *i = qgraphicsitem_cast<VToolLineIntersect *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueLine1P1;
    valueLine1P1.setValue(i->Line1P1Name());
    m_idToProperty[AttrP1Line1]->setValue(valueLine1P1);

    QVariant valueLine1P2;
    valueLine1P2.setValue(i->Line1P2Name());
    m_idToProperty[AttrP2Line1]->setValue(valueLine1P2);

    QVariant valueLine2P1;
    valueLine2P1.setValue(i->Line2P1Name());
    m_idToProperty[AttrP1Line2]->setValue(valueLine2P1);

    QVariant valueLine2P2;
    valueLine2P2.setValue(i->Line2P2Name());
    m_idToProperty[AttrP2Line2]->setValue(valueLine2P2);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolNormal()
{
    auto *i = qgraphicsitem_cast<VToolNormal *>(m_currentItem);

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    m_idToProperty[AttrName]->setValue(i->name());

    m_idToProperty[AttrAngle]->setValue(i->GetAngle());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfContact()
{
    auto *i = qgraphicsitem_cast<VToolPointOfContact *>(m_currentItem);

    QVariant valueFormula;
    valueFormula.setValue(i->getArcRadius());
    m_idToProperty[AttrRadius]->setValue(valueFormula);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueCenterPoint;
    valueCenterPoint.setValue(i->ArcCenterPointName());
    m_idToProperty[AttrCenter]->setValue(valueCenterPoint);

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstPointName());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersection()
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersection *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstPointName());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersectionArcs()
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());
    m_idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint()) - 1);

    QVariant valueFirstArc;
    valueFirstArc.setValue(i->FirstArcName());
    m_idToProperty[AttrFirstArc]->setValue(valueFirstArc);

    QVariant valueSecondArc;
    valueSecondArc.setValue(i->SecondArcName());
    m_idToProperty[AttrSecondArc]->setValue(valueSecondArc);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersectionCircles()
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersectionCircles *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());
    m_idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint()) - 1);

    QVariant c1Radius;
    c1Radius.setValue(i->GetFirstCircleRadius());
    m_idToProperty[AttrC1Radius]->setValue(c1Radius);

    QVariant c2Radius;
    c2Radius.setValue(i->GetSecondCircleRadius());
    m_idToProperty[AttrC2Radius]->setValue(c2Radius);

    QVariant valueC1Center;
    valueC1Center.setValue(i->FirstCircleCenterPointName());
    m_idToProperty[AttrC1Center]->setValue(valueC1Center);

    QVariant valueC2Center;
    valueC2Center.setValue(i->SecondCircleCenterPointName());
    m_idToProperty[AttrC2Center]->setValue(valueC2Center);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersectionCurves()
{
    auto *i = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());
    m_idToProperty[AttrVCrossPoint]->setValue(static_cast<int>(i->GetVCrossPoint()) - 1);
    m_idToProperty[AttrHCrossPoint]->setValue(static_cast<int>(i->GetHCrossPoint()) - 1);

    QVariant valueCurve1;
    valueCurve1.setValue(i->FirstCurveName());
    m_idToProperty[AttrCurve1]->setValue(valueCurve1);

    QVariant valueCurve2;
    valueCurve2.setValue(i->SecondCurveName());
    m_idToProperty[AttrCurve2]->setValue(valueCurve2);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointFromCircleAndTangent()
{
    auto *i = qgraphicsitem_cast<VToolPointFromCircleAndTangent *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());
    m_idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint()) - 1);

    QVariant cRadius;
    cRadius.setValue(i->GetCircleRadius());
    m_idToProperty[AttrCRadius]->setValue(cRadius);

    QVariant valueCenterPoint;
    valueCenterPoint.setValue(i->CircleCenterPointName());
    m_idToProperty[AttrCCenter]->setValue(valueCenterPoint);

    QVariant valueTangentPoint;
    valueTangentPoint.setValue(i->TangentPointName());
    m_idToProperty[AttrTangent]->setValue(valueTangentPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointFromArcAndTangent()
{
    auto *i = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());
    m_idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint()) - 1);

    QVariant valueTangentPoint;
    valueTangentPoint.setValue(i->TangentPointName());
    m_idToProperty[AttrTangent]->setValue(valueTangentPoint);

    QVariant valueArc;
    valueArc.setValue(i->ArcName());
    m_idToProperty[AttrArc]->setValue(valueArc);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolShoulderPoint()
{
    auto *i = qgraphicsitem_cast<VToolShoulderPoint *>(m_currentItem);

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueFormula);

    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    QVariant valueThirdPoint;
    valueThirdPoint.setValue(i->ShoulderPointName());
    m_idToProperty[AttrThirdPoint]->setValue(valueThirdPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolSpline()
{
    auto *i = qgraphicsitem_cast<VToolSpline *>(m_currentItem);
    const VSpline spl = i->getSpline();

    m_idToProperty[AttrName]->setValue(i->name());

    VFormula angle1F(spl.GetStartAngleFormula(), i->getData());
    angle1F.setCheckZero(false);
    angle1F.setToolId(i->getId());
    angle1F.setPostfix(degreeSymbol);
    angle1F.Eval();
    QVariant angle1;
    angle1.setValue(angle1F);
    m_idToProperty[AttrAngle1]->setValue(angle1);

    VFormula length1F(spl.GetC1LengthFormula(), i->getData());
    length1F.setCheckZero(false);
    length1F.setToolId(i->getId());
    length1F.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    length1F.Eval();
    QVariant length1;
    length1.setValue(length1F);
    m_idToProperty[AttrLength1]->setValue(length1);

    VFormula angle2F(spl.GetEndAngleFormula(), i->getData());
    angle2F.setCheckZero(false);
    angle2F.setToolId(i->getId());
    angle2F.setPostfix(degreeSymbol);
    angle2F.Eval();
    QVariant angle2;
    angle2.setValue(angle2F);
    m_idToProperty[AttrAngle2]->setValue(angle2);

    VFormula length2F(spl.GetC2LengthFormula(), i->getData());
    length2F.setCheckZero(false);
    length2F.setToolId(i->getId());
    length2F.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    length2F.Eval();
    QVariant length2;
    length2.setValue(length2F);
    m_idToProperty[AttrLength2]->setValue(length2);

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->GetPenStyle());
        m_idToProperty[AttrPenStyle]->setValue(index);
    }

    m_idToProperty[AttrColor]->setValue(
        VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor()));

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(spl.GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCubicBezier()
{
    auto *i = qgraphicsitem_cast<VToolCubicBezier *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->GetPenStyle());
        m_idToProperty[AttrPenStyle]->setValue(index);
    }

    m_idToProperty[AttrColor]->setValue(
        VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor()));

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstPointName());
    m_idToProperty[AttrPoint1]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrPoint2]->setValue(valueSecondPoint);

    QVariant valueThirdPoint;
    valueThirdPoint.setValue(i->ThirdPointName());
    m_idToProperty[AttrPoint3]->setValue(valueThirdPoint);

    QVariant valueForthPoint;
    valueForthPoint.setValue(i->ForthPointName());
    m_idToProperty[AttrPoint4]->setValue(valueForthPoint);

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(i->getSpline().GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolSplinePath()
{
    auto *i = qgraphicsitem_cast<VToolSplinePath *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->GetPenStyle());
        m_idToProperty[AttrPenStyle]->setValue(index);
    }

    m_idToProperty[AttrColor]->setValue(
        VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor()));

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(i->getSplinePath().GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCubicBezierPath()
{
    auto *i = qgraphicsitem_cast<VToolCubicBezierPath *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            CurvePenStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->GetPenStyle());
        m_idToProperty[AttrPenStyle]->setValue(index);
    }

    m_idToProperty[AttrColor]->setValue(
        VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor()));

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(i->getSplinePath().GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolTriangle()
{
    auto *i = qgraphicsitem_cast<VToolTriangle *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueAxisP1;
    valueAxisP1.setValue(i->AxisP1Name());
    m_idToProperty[AttrAxisP1]->setValue(valueAxisP1);

    QVariant valueAxisP2;
    valueAxisP2.setValue(i->AxisP2Name());
    m_idToProperty[AttrAxisP2]->setValue(valueAxisP2);

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstPointName());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondPointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolLineIntersectAxis()
{
    auto *i = qgraphicsitem_cast<VToolLineIntersectAxis *>(m_currentItem);
    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    m_idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstLinePoint());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondLinePoint());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCurveIntersectAxis()
{
    auto *i = qgraphicsitem_cast<VToolCurveIntersectAxis *>(m_currentItem);
    m_idToProperty[AttrName]->setValue(i->name());

    {
        QPalette comboBoxPalette = ComboBoxPalette();
        const auto index = VPE::VLineTypeProperty::IndexOfStyle(
            LineStylesPics(comboBoxPalette.color(QPalette::Base), comboBoxPalette.color(QPalette::Text)),
            i->getLineType());
        m_idToProperty[AttrTypeLine]->setValue(index);
    }

    {
        const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
        m_idToProperty[AttrLineColor]->setValue(index);
    }

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    m_idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueBasePoint;
    valueBasePoint.setValue(i->BasePointName());
    m_idToProperty[AttrBasePoint]->setValue(valueBasePoint);

    QVariant valueCurve;
    valueCurve.setValue(i->CurveName());
    m_idToProperty[AttrCurve]->setValue(valueCurve);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolRotation()
{
    auto *i = qgraphicsitem_cast<VToolRotation *>(m_currentItem);
    m_idToProperty[AttrSuffix]->setValue(i->Suffix());

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    m_idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueOriginPoint;
    valueOriginPoint.setValue(i->OriginPointName());
    m_idToProperty[AttrCenter]->setValue(valueOriginPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolMove()
{
    auto *i = qgraphicsitem_cast<VToolMove *>(m_currentItem);
    m_idToProperty[AttrSuffix]->setValue(i->Suffix());

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    m_idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueLength;
    valueLength.setValue(i->GetFormulaLength());
    m_idToProperty[AttrLength]->setValue(valueLength);

    QVariant valueRotationAngle;
    valueRotationAngle.setValue(i->GetFormulaRotationAngle());
    m_idToProperty[AttrRotationAngle]->setValue(valueRotationAngle);

    QVariant valueRotationOriginPoint;
    valueRotationOriginPoint.setValue(i->OriginPointName());
    m_idToProperty[AttrCenter]->setValue(valueRotationOriginPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolFlippingByLine()
{
    auto *i = qgraphicsitem_cast<VToolFlippingByLine *>(m_currentItem);
    m_idToProperty[AttrSuffix]->setValue(i->Suffix());

    QVariant valueFirstPoint;
    valueFirstPoint.setValue(i->FirstLinePointName());
    m_idToProperty[AttrFirstPoint]->setValue(valueFirstPoint);

    QVariant valueSecondPoint;
    valueSecondPoint.setValue(i->SecondLinePointName());
    m_idToProperty[AttrSecondPoint]->setValue(valueSecondPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolFlippingByAxis()
{
    auto *i = qgraphicsitem_cast<VToolFlippingByAxis *>(m_currentItem);
    m_idToProperty[AttrAxisType]->setValue(static_cast<int>(i->GetAxisType()) - 1);
    m_idToProperty[AttrSuffix]->setValue(i->Suffix());

    QVariant valueOriginPoint;
    valueOriginPoint.setValue(i->OriginPointName());
    m_idToProperty[AttrCenter]->setValue(valueOriginPoint);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolEllipticalArc()
{
    auto *i = qgraphicsitem_cast<VToolEllipticalArc *>(m_currentItem);

    m_idToProperty[AttrName]->setValue(i->name());

    QVariant valueFormulaRadius1;
    valueFormulaRadius1.setValue(i->GetFormulaRadius1());
    m_idToProperty[AttrRadius1]->setValue(valueFormulaRadius1);

    QVariant valueFormulaRadius2;
    valueFormulaRadius2.setValue(i->GetFormulaRadius2());
    m_idToProperty[AttrRadius2]->setValue(valueFormulaRadius2);

    QVariant valueFormulaF1;
    valueFormulaF1.setValue(i->GetFormulaF1());
    m_idToProperty[AttrAngle1]->setValue(valueFormulaF1);

    QVariant valueFormulaF2;
    valueFormulaF2.setValue(i->GetFormulaF2());
    m_idToProperty[AttrAngle2]->setValue(valueFormulaF2);

    QVariant valueFormulaRotationAngle;
    valueFormulaRotationAngle.setValue(i->GetFormulaRotationAngle());
    m_idToProperty[AttrRotationAngle]->setValue(valueFormulaRotationAngle);

    const auto index = VPE::VLineColorProperty::IndexOfColor(VAbstractTool::ColorsList(), i->GetLineColor());
    m_idToProperty[AttrColor]->setValue(index);

    QVariant valueCenterPoint;
    valueCenterPoint.setValue(i->CenterPointName());
    m_idToProperty[AttrCenter]->setValue(valueCenterPoint);

    QVariant valueApproximationScale;
    valueApproximationScale.setValue(i->GetApproximationScale());
    m_idToProperty[AttrAScale]->setValue(valueApproximationScale);

    m_idToProperty[AttrNotes]->setValue(i->GetNotes());

    m_idToProperty[AttrAlias]->setValue(i->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsBackgroundPixmapItem()
{
    auto *i = qgraphicsitem_cast<VBackgroundPixmapItem *>(m_currentItem);

    m_idToProperty.value(AttrName)->setValue(i->name());
    m_idToProperty.value(*AttrHold)->setValue(i->IsHold());
    m_idToProperty.value(*AttrVisible)->setValue(i->IsVisible());
    m_idToProperty.value(*AttrOpacity)->setValue(static_cast<int>(i->GetOpacity() * 100));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsBackgroundSVGItem()
{
    auto *i = qgraphicsitem_cast<VBackgroundSVGItem *>(m_currentItem);

    m_idToProperty.value(AttrName)->setValue(i->name());
    m_idToProperty.value(*AttrHold)->setValue(i->IsHold());
    m_idToProperty.value(*AttrVisible)->setValue(i->IsVisible());
    m_idToProperty.value(*AttrOpacity)->setValue(static_cast<int>(i->GetOpacity() * 100));
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolOptionsPropertyBrowser::PropertiesList() -> QStringList
{
    static QStringList attr{
        AttrName,                           /* 0 */
        "position"_L1,                      /* 1 */
        AttrBasePoint,                      /* 2 */
        AttrTypeLine,                       /* 3 */
        AttrLength,                         /* 4 */
        AttrAngle,                          /* 5 */
        AttrFirstPoint,                     /* 6 */
        AttrSecondPoint,                    /* 7 */
        AttrRadius,                         /* 8 */
        AttrAngle1,                         /* 9 */
        AttrAngle2,                         /* 10 */
        AttrCenter,                         /* 11 */
        AttrThirdPoint,                     /* 12 */
        AttrArc,                            /* 13 */
        VToolCutSpline::AttrSpline,         /* 14 */
        VToolCutSplinePath::AttrSplinePath, /* 15 */
        AttrP1Line,                         /* 16 */
        AttrP2Line,                         /* 17 */
        AttrP1Line1,                        /* 18 */
        AttrP2Line1,                        /* 19 */
        AttrP1Line2,                        /* 20 */
        AttrP2Line2,                        /* 21 */
        AttrPShoulder,                      /* 22 */
        AttrAxisP1,                         /* 23 */
        AttrAxisP2,                         /* 24 */
        AttrKCurve,
        /*Not used*/       /* 25 */
        AttrLineColor,     /* 26 */
        AttrColor,         /* 27 */
        AttrCrossPoint,    /* 28 */
        AttrC1Radius,      /* 29 */
        AttrC2Radius,      /* 30 */
        AttrCRadius,       /* 31 */
        AttrName1,         /* 32 */
        AttrName2,         /* 33 */
        AttrVCrossPoint,   /* 34 */
        AttrHCrossPoint,   /* 35 */
        AttrLength1,       /* 36 */
        AttrLength2,       /* 37 */
        AttrSuffix,        /* 38 */
        AttrAxisType,      /* 39 */
        AttrRadius1,       /* 40 */
        AttrRadius2,       /* 41 */
        AttrRotationAngle, /* 42 */
        AttrDartP1,        /* 43 */
        AttrDartP2,        /* 44 */
        AttrDartP3,        /* 45 */
        AttrCurve,         /* 46 */
        AttrFirstArc,      /* 47 */
        AttrSecondArc,     /* 48 */
        AttrC1Center,      /* 49 */
        AttrC2Center,      /* 50 */
        AttrCurve1,        /* 51 */
        AttrCurve2,        /* 52 */
        AttrCCenter,       /* 53 */
        AttrTangent,       /* 54 */
        AttrPoint1,        /* 55 */
        AttrPoint2,        /* 56 */
        AttrPoint3,        /* 57 */
        AttrPoint4,        /* 58 */
        AttrPenStyle,      /* 59 */
        AttrAScale,        /* 60 */
        AttrNotes,         /* 61 */
        AttrAlias,         /* 62 */
        AttrAlias1,        /* 63 */
        AttrAlias2,        /* 64 */
        *AttrHold,         /* 65 */
        *AttrVisible,      /* 66 */
        *AttrOpacity       /* 67 */
    };
    return attr;
}
