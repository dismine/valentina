/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 11, 2016
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

#include "vtoolpiecepath.h"
#include "../../dialogs/tools/piece/dialogpiecepath.h"
#include "../../undocommands/savepieceoptions.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vlayout/vabstractpiece.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vtoolseamallowance.h"
#include "../vwidgets/global.h"
#include "vtools/tools/vabstracttool.h"

//---------------------------------------------------------------------------------------------------------------------
auto VToolPiecePath::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                            VContainer *data) -> VToolPiecePath *
{
    SCASSERT(not dialog.isNull());
    const QPointer<DialogPiecePath> dialogTool = qobject_cast<DialogPiecePath *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolPiecePathInitData initData;
    initData.path = dialogTool->GetPiecePath();
    initData.idObject = dialogTool->GetPieceId();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;

    initData.path.SetNodes(PrepareNodes(initData.path, scene, doc, data));

    return Create(initData);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPiecePath::Create(VToolPiecePathInitData initData) -> VToolPiecePath *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddPiecePath(initData.path);
    }
    else
    {
        initData.data->UpdatePiecePath(initData.id, initData.path);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::MODELING_OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());

    if (initData.path.GetType() == PiecePathType::InternalPath)
    {
        const auto varData = initData.data->DataDependencyVariables();
        initData.doc->FindFormulaDependencies(initData.path.GetVisibilityTrigger(), initData.id, varData);
    }

    bool ignorePath = false;
    for (int i = 0; i < initData.path.CountNodes(); ++i)
    {
        quint32 const id = initData.path.at(i).GetId();
        try
        {
            initData.data->GetGObject(id);
        }
        catch (const VExceptionBadId &)
        { // Possible case. Parent was deleted, but the node object is still here.
            qDebug() << "Broken relation. Parent was deleted, but the piece path object is still here. Piece "
                        "path id ="
                     << initData.id << ".";
            ignorePath = true; // Just ignore
        }
        patternGraph->AddEdge(id, initData.id);
    }

    if (ignorePath)
    {
        return nullptr;
    }

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PiecePath, initData.doc);
        // TODO Need create garbage collector and remove all nodes, that we don't use.
        // Better check garbage before each saving file. Check only modeling tags.
        auto *pathTool = new VToolPiecePath(initData);

        VAbstractPattern::AddTool(initData.id, pathTool);
        if (initData.idTool != NULL_ID)
        {
            // Some nodes we don't show on scene. Tool that create this nodes must free memory.
            VDataTool *tool = VAbstractPattern::getTool(initData.idTool);
            SCASSERT(tool != nullptr);
            pathTool->setParent(tool); // Adopted by a tool
        }
        else
        {
            if (initData.typeCreation == Source::FromGui && initData.path.GetType() == PiecePathType::InternalPath)
            { // Seam allowance tool already initializated and can't init the path
                SCASSERT(initData.idObject > NULL_ID);
                auto *saTool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(initData.idObject));
                SCASSERT(saTool != nullptr);
                pathTool->setParentItem(saTool);
                pathTool->SetParentType(ParentType::Item);
            }
            else
            {
                // Try to prevent memory leak
                initData.scene->addItem(pathTool); // First adopted by scene
                pathTool->hide();                  // If no one will use node, it will stay hidden
                pathTool->SetParentType(ParentType::Scene);
            }
        }
        return pathTool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPiecePath::getTagName() const -> QString
{
    return VAbstractPattern::TagPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    qreal width = VAbstractApplication::VApp()->Settings()->WidthHairLine();

    const qreal scale = SceneScale(scene());
    if (scale > 1)
    {
        width = qMax(1., width / scale);
    }

    QPen toolPen = pen();
    toolPen.setWidthF(width);
    toolPen.setColor(VSceneStylesheet::PatternPieceStyle().PieceColor());

    setPen(toolPen);

    PaintWithFixItemHighlightSelected<QGraphicsPathItem>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiecePath &path)
{
    doc->SetAttribute(domElement, VDomDocument::AttrId, id);
    doc->SetAttribute(domElement, AttrName, path.GetName());
    doc->SetAttribute(domElement, AttrType, static_cast<int>(path.GetType()));
    doc->SetAttribute(domElement, AttrTypeLine, PenStyleToLineStyle(path.GetPenType()));

    if (path.GetType() == PiecePathType::InternalPath)
    {
        doc->SetAttribute(domElement, VAbstractPattern::AttrVisible, path.GetVisibilityTrigger());
        doc->SetAttribute(domElement, AttrCut, path.IsCutPath());
        doc->SetAttribute(domElement, AttrFirstToContour, path.IsFirstToCuttingContour());
        doc->SetAttribute(domElement, AttrLastToContour, path.IsLastToCuttingContour());
        doc->SetAttributeOrRemoveIf<bool>(domElement,
                                          AttrNotMirrored,
                                          path.IsNotMirrored(),
                                          [](bool value) noexcept -> bool { return not value; });
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::FullUpdateFromFile()
{
    RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::AllowHover(bool enabled)
{
    Q_UNUSED(enabled)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::AllowSelecting(bool enabled)
{
    Q_UNUSED(enabled)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::AddToFile()
{
    QDomElement domElement = doc->createElement(VToolPiecePath::getTagName());
    const VPiecePath path = VAbstractTool::data.GetPiecePath(m_id);

    AddAttributes(doc, domElement, m_id, path);

    if (idTool != NULL_ID)
    {
        doc->SetAttribute(domElement, AttrIdTool, idTool);
    }

    AddNodes(doc, domElement, path);

    AddToModeling(domElement);

    if (m_pieceId > NULL_ID)
    {
        const VPiece oldDet = VAbstractTool::data.GetPiece(m_pieceId);
        VPiece newDet = oldDet;

        if (path.GetType() == PiecePathType::InternalPath)
        {
            newDet.GetInternalPaths().append(m_id);
        }
        else if (path.GetType() == PiecePathType::CustomSeamAllowance)
        {
            newDet.GetCustomSARecords().append({.path = m_id});
        }

        VAbstractApplication::VApp()->getUndoStack()->push(new SavePieceOptions(oldDet, newDet, doc, m_pieceId));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::ToolCreation(const Source &typeCreation)
{
    if (typeCreation == Source::FromGui || typeCreation == Source::FromTool)
    {
        VToolPiecePath::AddToFile();
    }
    else
    {
        VToolPiecePath::RefreshDataInFile();
    }
}

//---------------------------------------------------------------------------------------------------------------------
VToolPiecePath::VToolPiecePath(const VToolPiecePathInitData &initData, QObject *qoParent, QGraphicsItem *parent)
  : VAbstractNode(initData.doc, initData.data, initData.id, NULL_ID, initData.drawName, initData.idTool, qoParent),
    QGraphicsPathItem(parent),
    m_pieceId(initData.idObject)
{
    RefreshGeometry();
    VToolPiecePath::ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPiecePath::RefreshGeometry()
{
    const VPiecePath path = VAbstractTool::data.GetPiecePath(m_id);
    if (path.GetType() != PiecePathType::InternalPath)
    {
        setVisible(false);
        return;
    }

    QVector<QPointF> cuttingPath;
    const quint32 pieceId = VAbstractTool::data.GetPieceForPiecePath(m_id);
    bool showFullPiece = true;
    QLineF mirrorLine;
    if (pieceId > NULL_ID)
    {
        VPiece const piece = VAbstractTool::data.GetPiece(pieceId);
        // We cannot use current VContainer because it doesn't have current seam allowance value
        const VContainer pData = VAbstractPattern::getTool(pieceId)->getData();
        cuttingPath = piece.CuttingPathPoints(&pData);

        showFullPiece = piece.IsShowFullPiece();
        mirrorLine = piece.SeamAllowanceMirrorLine(&pData);
    }
    QPainterPath p = path.PainterPath(this->getData(), cuttingPath);

    if (!path.IsNotMirrored() && showFullPiece && !mirrorLine.isNull())
    {
        QVector<VLayoutPoint> points = path.PathPoints(this->getData(), cuttingPath);
        const QTransform matrix = VGObject::FlippingMatrix(mirrorLine);
        std::transform(points.begin(),
                       points.end(),
                       points.begin(),
                       [&matrix](const VLayoutPoint &point) -> VLayoutPoint
                       { return VAbstractPiece::MapPoint(point, matrix); });
        QVector<QPointF> casted;
        CastTo(points, casted);
        p.addPath(VPiecePath::MakePainterPath(casted));
    }

    p.setFillRule(Qt::OddEvenFill);

    this->setPath(p);
    QPen pen = this->pen();
    pen.setStyle(path.GetPenType());
    this->setPen(pen);
    if (IsRemovable() != RemoveStatus::Removable && GetParentType() == ParentType::Item)
    {
        setVisible(path.IsVisible(this->getData()->DataVariables()));
    }
}
