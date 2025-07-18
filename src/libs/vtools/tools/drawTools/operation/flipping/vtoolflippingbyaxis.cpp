/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vtoolflippingbyaxis.h"

#include <QColor>
#include <QDomNode>
#include <QDomNodeList>
#include <QMapIterator>
#include <QPoint>
#include <QSharedPointer>
#include <QUndoStack>
#include <climits>
#include <new>
#include <qiterator.h>

#include "../../../../dialogs/tools/dialogflippingbyaxis.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/line/operation/vistoolflippingbyaxis.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../../../vdatatool.h"
#include "../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractsimple.h"
#include "../vwidgets/vmaingraphicsscene.h"

template <class T> class QSharedPointer;

const QString VToolFlippingByAxis::ToolType = QStringLiteral("flippingByAxis");

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogFlippingByAxis> dialogTool = qobject_cast<DialogFlippingByAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->SetOriginPointId(m_originPointId);
    dialogTool->SetAxisType(m_axisType);
    dialogTool->SetSuffix(suffix);
    dialogTool->SetNotes(m_notes);
    dialogTool->SetSourceObjects(source);

    SetDialogVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByAxis::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data) -> VToolFlippingByAxis *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogFlippingByAxis> dialogTool = qobject_cast<DialogFlippingByAxis *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolFlippingByAxisInitData initData;
    initData.originPointId = dialogTool->GetOriginPointId();
    initData.axisType = dialogTool->GetAxisType();
    initData.suffix = dialogTool->GetSuffix();
    initData.source = dialogTool->GetSourceObjects();
    initData.hasLinkedVisibilityGroup = dialogTool->HasLinkedVisibilityGroup();
    initData.visibilityGroupName = dialogTool->GetVisibilityGroupName();
    initData.visibilityGroupTags = dialogTool->GetVisibilityGroupTags();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolFlippingByAxis *operation = Create(initData);
    if (operation != nullptr)
    {
        operation->m_dialog = dialog;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByAxis::Create(VToolFlippingByAxisInitData initData) -> VToolFlippingByAxis *
{
    const auto originPoint = *initData.data->GeometricObject<VPointF>(initData.originPointId);
    const auto fPoint = static_cast<QPointF>(originPoint);

    QPointF sPoint;
    if (initData.axisType == AxisType::VerticalAxis)
    {
        sPoint = QPointF(fPoint.x(), fPoint.y() + 100);
    }
    else
    {
        sPoint = QPointF(fPoint.x() + 100, fPoint.y());
    }

    CreateDestination(initData, fPoint, sPoint);

    if (initData.parse == Document::FullParse)
    {
        if (initData.typeCreation == Source::FromGui && initData.hasLinkedVisibilityGroup)
        {
            VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("Flipping by axis"));
        }

        VAbstractTool::AddRecord(initData.id, Tool::FlippingByAxis, initData.doc);
        auto *tool = new VToolFlippingByAxis(initData);
        initData.scene->addItem(tool);
        InitOperationToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);
        initData.doc->IncrementReferens(originPoint.getIdTool());
        for (auto object : qAsConst(initData.source))
        {
            initData.doc->IncrementReferens(initData.data->GetGObject(object.id)->getIdTool());
        }

        if (initData.typeCreation == Source::FromGui && initData.hasLinkedVisibilityGroup)
        {
            VAbstractOperation::CreateVisibilityGroup(initData);
            VAbstractApplication::VApp()->getUndoStack()->endMacro();
        }

        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByAxis::GetAxisType() const -> AxisType
{
    return m_axisType;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::SetAxisType(AxisType value)
{
    m_axisType = value;

    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByAxis::OriginPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(m_originPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolFlippingByAxis>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogFlippingByAxis>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolFlippingByAxis *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetObjects(SourceToObjects(source));
        visual->SetOriginPointId(m_originPointId);
        visual->SetAxisType(m_axisType);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                     QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogFlippingByAxis> dialogTool = qobject_cast<DialogFlippingByAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, m_originPointId);
    AddDependence(newDependencies, dialogTool->GetOriginPointId());

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetOriginPointId()));
    doc->SetAttribute(domElement, AttrAxisType, QString().setNum(static_cast<int>(dialogTool->GetAxisType())));
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->GetSuffix());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });

    source = dialogTool->GetSourceObjects();
    SaveSourceDestination(domElement);

    // Save visibility data for later use
    SaveVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::ReadToolAttributes(const QDomElement &domElement)
{
    VAbstractFlipping::ReadToolAttributes(domElement);

    m_originPointId = VAbstractPattern::GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    m_axisType = static_cast<AxisType>(VAbstractPattern::GetParametrUInt(domElement, AttrAxisType, QChar('1')));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByAxis::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractFlipping::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, QString().setNum(m_originPointId));
    doc->SetAttribute(tag, AttrAxisType, QString().setNum(static_cast<int>(m_axisType)));
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByAxis::MakeToolTip() const -> QString
{
    return QStringLiteral("<tr> <td><b>%1:</b> %2</td> </tr>"
                          "%3")
        .arg(tr("Origin point"), OriginPointName()) // 1, 2
        .arg(VisibilityGroupToolTip());             // 3
}

//---------------------------------------------------------------------------------------------------------------------
VToolFlippingByAxis::VToolFlippingByAxis(const VToolFlippingByAxisInitData &initData, QGraphicsItem *parent)
  : VAbstractFlipping(initData, parent),
    m_originPointId(initData.originPointId),
    m_axisType(initData.axisType)
{
    InitOperatedObjects();
    ToolCreation(initData.typeCreation);
}
