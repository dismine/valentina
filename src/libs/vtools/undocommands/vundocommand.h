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

Q_DECLARE_LOGGING_CATEGORY(vUndo)

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
    RenamePiece
};

class VPattern;

class VUndoCommand : public QObject, public QUndoCommand
{
    Q_OBJECT // NOLINT

public:
    VUndoCommand(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    virtual ~VUndoCommand() = default;
signals:
    void ClearScene();
    void NeedFullParsing();
    void NeedLiteParsing(const Document &parse);

protected:
    QDomElement xml;
    VAbstractPattern *doc;
    quint32 nodeId;
    bool redoFlag;
    virtual void RedoFullParsing();
    void UndoDeleteAfterSibling(QDomNode &parentNode, quint32 siblingId, const QString &tagName = QString()) const;

    void IncrementReferences(const QVector<quint32> &nodes) const;
    void DecrementReferences(const QVector<quint32> &nodes) const;

    void IncrementReferences(const QVector<CustomSARecord> &nodes) const;
    void DecrementReferences(const QVector<CustomSARecord> &nodes) const;

    void IncrementReferences(const QVector<VPieceNode> &nodes) const;
    void DecrementReferences(const QVector<VPieceNode> &nodes) const;

    auto GetDestinationObject(quint32 idTool, quint32 idPoint) const -> QDomElement;

private:
    Q_DISABLE_COPY_MOVE(VUndoCommand) // NOLINT
};

#endif // VUNDOCOMMAND_H
