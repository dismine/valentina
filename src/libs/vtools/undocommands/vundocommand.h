/************************************************************************
 **
 **  @file   vundocommand.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 7, 2014
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

#ifndef VUNDOCOMMAND_H
#define VUNDOCOMMAND_H

#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QUndoCommand>
#include <QVector>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"

Q_DECLARE_LOGGING_CATEGORY(vUndo) //NOLINT

enum class UndoCommand : qint8
{
    AddPatternPiece,
    AddToCalc,
    MoveSpline,
    MoveSplinePath,
    MoveSPoint,
    SaveToolOptions,
    SaveDetailOptions,
    SavePieceOptions,
    SavePiecePathOptions,
    SavePlaceLabelOptions,
    MovePiece,
    DeleteTool,
    DeletePatternPiece,
    RenamePP,
    MoveLabel,
    MoveDoubleLabel,
    RotationMoveLabel,
    MoveBackGroundImage,
    ScaleBackGroundImage,
    RotateBackGroundImage,
    RenamePiece,
    MoveToolUp,
    MoveToolDown
};

class VPattern;

class VUndoCommand : public QObject, public QUndoCommand
{
    Q_OBJECT // NOLINT

public:
    explicit VUndoCommand(VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    VUndoCommand(VAbstractPattern *doc, quint32 id, QUndoCommand *parent = nullptr);
    VUndoCommand(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    ~VUndoCommand() override = default;

signals:
    void ClearScene();
    void NeedFullParsing();
    void NeedLiteParsing(const Document &parse);

protected:
    virtual void RedoFullParsing();
    void UndoDeleteAfterSibling(QDomNode &parentNode, quint32 siblingId, const QString &tagName = QString()) const;

    auto GetDestinationObject(quint32 idTool, quint32 idPoint) const -> QDomElement;

    static void DisablePieceNodes(const VPiecePath &path);
    static void EnablePieceNodes(const VPiecePath &path);

    static void DisableInternalPaths(const QVector<quint32> &paths);

    auto GetElement() const -> QDomElement;
    void SetElement(const QDomElement &newElement);

    auto Doc() const -> VAbstractPattern *;

    auto ElementId() const -> quint32;
    void SetElementId(quint32 newId);

    auto RedoFlag() const -> bool;
    void SetRedoFlag(bool newRedoFlag);

private:
    Q_DISABLE_COPY_MOVE(VUndoCommand) // NOLINT

    QDomElement m_element{};
    VAbstractPattern *m_doc;
    quint32 m_elementId{NULL_ID};
    bool m_redoFlag{false};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VUndoCommand::Doc() const -> VAbstractPattern *
{
    return m_doc;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VUndoCommand::ElementId() const -> quint32
{
    return m_elementId;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VUndoCommand::SetElementId(quint32 newId)
{
    m_elementId = newId;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VUndoCommand::RedoFlag() const -> bool
{
    return m_redoFlag;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VUndoCommand::SetRedoFlag(bool newRedoFlag)
{
    m_redoFlag = newRedoFlag;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VUndoCommand::GetElement() const -> QDomElement
{
    return m_element;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VUndoCommand::SetElement(const QDomElement &newElement)
{
    m_element = newElement;
}

#endif // VUNDOCOMMAND_H
