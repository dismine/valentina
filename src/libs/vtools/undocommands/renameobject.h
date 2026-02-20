/************************************************************************
 **
 **  @file   renameobject.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 1, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef RENAMEOBJECT_H
#define RENAMEOBJECT_H

#include "../ifc/xml/vpatterngraphnode.h"
#include "../vgeometry/vgeometrydef.h"
#include "vundocommand.h"

enum class OperationType : quint8
{
    Redo,
    Undo,
    Unknown
};

class AbstractObjectRename : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    AbstractObjectRename(VAbstractPattern *doc, quint32 id, QUndoCommand *parent = nullptr);
    ~AbstractObjectRename() override = default;

    void undo() override;
    void redo() override;

protected:
    auto ProcessType() const -> OperationType;

    virtual auto ProcessToken(const QString &token) const -> QString = 0;

    void ProcessElementByType(QDomElement &element) const;

private:
    Q_DISABLE_COPY_MOVE(AbstractObjectRename) // NOLINT

    QVector<VNode> m_dependencies{};
    OperationType m_operationType{OperationType::Unknown};

    void RenameFormulas();

    void ProcessPointElement(QDomElement &element) const;
    void ProcessOperationElement(QDomElement &element) const;
    void ProcessArcElement(QDomElement &element) const;
    void ProcessElArcElement(QDomElement &element) const;
    void ProcessSplineElement(QDomElement &element) const;
    void ProcessNodeElement(QDomElement &node) const;
    void ProcessPathElement(QDomElement &element) const;
    void ProcessToolsElement(QDomElement &element) const;
    void ProcessDetailElement(QDomElement &element) const;

    void ProcessAttribute(QDomElement &element, const QString &attr) const;
    auto ProcessFormula(const QString &formula) const -> QString;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto AbstractObjectRename::ProcessType() const -> OperationType
{
    return m_operationType;
}

class RenameLabel : public AbstractObjectRename
{
    Q_OBJECT // NOLINT

public:
    RenameLabel(QString oldLabel, QString newLabel, VAbstractPattern *doc, quint32 id, QUndoCommand *parent = nullptr);
    ~RenameLabel() override = default;

protected:
    auto ProcessToken(const QString &token) const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(RenameLabel) // NOLINT
    QString m_oldLabel;
    QString m_newLabel;
};

enum class RenameObjectType : quint8
{
    Line,
    Spline,
    SplinePath
};

using ObjectPair_t = std::pair<QString, QString>;

class RenamePair : public AbstractObjectRename
{
    Q_OBJECT // NOLINT

public:
    Q_REQUIRED_RESULT static auto CreateForLine(const ObjectPair_t &oldPair,
                                                const ObjectPair_t &newPair,
                                                VAbstractPattern *doc,
                                                quint32 id,
                                                QUndoCommand *parent = nullptr) -> RenamePair *;

    Q_REQUIRED_RESULT static auto CreateForSpline(const ObjectPair_t &oldPair,
                                                  const ObjectPair_t &newPair,
                                                  quint32 duplicate,
                                                  VAbstractPattern *doc,
                                                  quint32 id,
                                                  QUndoCommand *parent = nullptr) -> RenamePair *;
    Q_REQUIRED_RESULT static auto CreateForSplinePath(const ObjectPair_t &oldPair,
                                                      const ObjectPair_t &newPair,
                                                      quint32 duplicate,
                                                      VAbstractPattern *doc,
                                                      quint32 id,
                                                      QUndoCommand *parent = nullptr) -> RenamePair *;

    ~RenamePair() override = default;

protected:
    auto ProcessToken(const QString &token) const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(RenamePair) // NOLINT
    RenameObjectType m_type;
    ObjectPair_t m_oldPair;
    ObjectPair_t m_newPair;
    quint32 m_duplicate;

    RenamePair(RenameObjectType type,
               ObjectPair_t oldPair,
               ObjectPair_t newPair,
               quint32 duplicate,
               VAbstractPattern *doc,
               quint32 id,
               QUndoCommand *parent = nullptr);
};

enum class CurveAliasType : quint8
{
    Arc,
    ElArc,
    Spline,
    SplinePath,
    All
};

class RenameAlias : public AbstractObjectRename
{
    Q_OBJECT // NOLINT

public:
    RenameAlias(CurveAliasType type,
                QString oldAlias,
                QString newAlias,
                VAbstractPattern *doc,
                quint32 id,
                QUndoCommand *parent = nullptr);
    ~RenameAlias() override = default;

    static auto CurveType(GOType objType) -> CurveAliasType;

protected:
    auto ProcessToken(const QString &token) const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(RenameAlias) // NOLINT
    CurveAliasType m_type;
    QString m_oldAlias;
    QString m_newAlias;
};

enum class RenameArcType : quint8
{
    Arc,
    ElArc
};

class RenameArc : public AbstractObjectRename
{
    Q_OBJECT // NOLINT

public:
    RenameArc(RenameArcType type,
              QString oldCenterLabel,
              QString newCenterLabel,
              int duplicate,
              VAbstractPattern *doc,
              quint32 id,
              QUndoCommand *parent = nullptr);
    ~RenameArc() override = default;

protected:
    auto ProcessToken(const QString &token) const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(RenameArc) // NOLINT
    RenameArcType m_type;
    QString m_oldCenterLabel;
    QString m_newCenterLabel;
    int m_duplicate;
};

class RenameSegmentCurves : public AbstractObjectRename
{
    Q_OBJECT // NOLINT

public:
    RenameSegmentCurves(CurveAliasType type,
                        QString pointName,
                        QString leftSub,
                        QString rightSub,
                        VAbstractPattern *doc,
                        QUndoCommand *parent = nullptr);
    ~RenameSegmentCurves() override = default;

    void undo() override;
    void redo() override;

protected:
    auto ProcessToken(const QString &token) const -> QString override;

private:
    Q_DISABLE_COPY_MOVE(RenameSegmentCurves) // NOLINT
    CurveAliasType m_type;
    QString m_pointName;
    QString m_leftSub;
    QString m_rightSub;
};

#endif // RENAMEOBJECT_H
