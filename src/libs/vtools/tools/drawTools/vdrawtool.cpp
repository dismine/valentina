/************************************************************************
 **
 **  @file   vdrawtool.cpp
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

#include "vdrawtool.h"

#include <QDialog>
#include <QDomNode>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QUndoStack>
#include <QtDebug>

#include "../../undocommands/addtocalc.h"
#include "../../undocommands/savetooloptions.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vabstracttool.h"
#include "../vpatterndb/vcontainer.h"

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VDrawTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 */
VDrawTool::VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QString notes, QObject *parent)
  : VInteractiveTool(doc, data, id, parent),
    nameActivDraw(doc->GetNameActivPP()),
    m_lineType(TypeLineLine),
    m_notes(std::move(notes))
{
    connect(this->doc, &VAbstractPattern::ChangedActivPP, this, &VDrawTool::ChangedActivDraw);
    connect(this->doc, &VAbstractPattern::ChangedNameDraw, this, &VDrawTool::ChangedNameDraw);
    connect(this->doc, &VAbstractPattern::ShowTool, this, &VDrawTool::ShowTool);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.
 * @param id object id in container.
 * @param enable enable or disable highlight.
 */
void VDrawTool::ShowTool(quint32 id, bool enable)
{
    Q_UNUSED(id)
    Q_UNUSED(enable)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedActivDraw disable or enable context menu after change active pattern peace.
 * @param newName new name active pattern peace.
 */
void VDrawTool::ChangedActivDraw(const QString &newName)
{
    Disable(!(nameActivDraw == newName), newName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedNameDraw save new name active pattern peace.
 * @param oldName old name.
 * @param newName new name active pattern peace. new name.
 */
void VDrawTool::ChangedNameDraw(const QString &oldName, const QString &newName)
{
    if (nameActivDraw == oldName)
    {
        nameActivDraw = newName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveDialogChange(const QString &undoText)
{
    Q_UNUSED(undoText)
    qCDebug(vTool, "Saving tool options after using dialog");
    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();
        QList<quint32> oldDependencies;
        QList<quint32> newDependencies;
        SaveDialog(newDomElement, oldDependencies, newDependencies);
        ApplyToolOptions(oldDependencies, newDependencies, oldDomElement, newDomElement);
    }
    else
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ApplyToolOptions(const QList<quint32> &oldDependencies, const QList<quint32> &newDependencies,
                                 const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    if (newDependencies != oldDependencies || not VDomDocument::Compare(newDomElement, oldDomElement))
    {
        auto *saveOptions =
            new SaveToolOptions(oldDomElement, newDomElement, oldDependencies, newDependencies, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        VAbstractApplication::VApp()->getUndoStack()->push(saveOptions);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VDrawTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOption(QSharedPointer<VGObject> &obj)
{
    qCDebug(vTool, "Saving tool options");
    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();

        SaveOptions(newDomElement, obj);

        auto *saveOptions =
            new SaveToolOptions(oldDomElement, newDomElement, QList<quint32>(), QList<quint32>(), doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        VAbstractApplication::VApp()->getUndoStack()->push(saveOptions);
    }
    else
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    Q_UNUSED(obj)

    doc->SetAttribute(tag, VDomDocument::AttrId, m_id);
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrNotes, m_notes,
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::MakeToolTip() const -> QString
{
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::UpdateNamePosition(quint32 id, const QPointF &pos)
{
    Q_UNUSED(id)
    Q_UNUSED(pos)
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::CorrectDisable(bool disable, const QString &namePP) const -> bool
{
    if (disable)
    {
        return disable;
    }

    return !(nameActivDraw == namePP);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadAttributes()
{
    const QDomElement domElement = doc->FindElementById(m_id, getTagName());
    if (domElement.isElement())
    {
        ReadToolAttributes(domElement);
    }
    else
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadToolAttributes(const QDomElement &domElement)
{
    m_notes = VDomDocument::GetParametrEmptyString(domElement, AttrNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::EnableToolMove(bool move)
{
    Q_UNUSED(move)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SetDetailsMode(bool mode)
{
    Q_UNUSED(mode)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ChangeLabelVisibility(quint32 id, bool visible)
{
    Q_UNUSED(id)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToCalculation add tool to calculation tag in pattern file.
 * @param domElement tag in xml tree.
 */
void VDrawTool::AddToCalculation(const QDomElement &domElement)
{
    const QDomElement duplicate = doc->FindElementById(m_id);
    if (not duplicate.isNull())
    {
        throw VExceptionWrongId(tr("This id (%1) is not unique.").arg(m_id), duplicate);
    }

    auto *addToCal = new AddToCalc(domElement, doc);
    connect(addToCal, &AddToCalc::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(addToCal);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::AddDependence(QList<quint32> &list, quint32 objectId) const
{
    if (objectId != NULL_ID)
    {
        auto originPoint = VAbstractTool::data.GetGObject(objectId);
        list.append(originPoint->getIdTool());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::getLineType() const -> QString
{
    return m_lineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SetLineType(const QString &value)
{
    m_lineType = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::IsLabelVisible(quint32 id) const -> bool
{
    Q_UNUSED(id)
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDrawTool::GetNotes() const -> QString
{
    return m_notes;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SetNotes(const QString &notes)
{
    m_notes = notes;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}
