/************************************************************************
 **
 **  @file   renameobject.cpp
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
#include "renameobject.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../qmuparser/qmutokenparser.h"

#include <utility>
#include <QRegularExpression>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FormulaTokens(const QString &formula) -> QMap<vsizetype, QString>
{
    try
    {
        QScopedPointer<qmu::QmuTokenParser> const cal(new qmu::QmuTokenParser(formula, false, false));
        return cal->GetTokens();
    }
    catch (qmu::QmuParserError &e)
    {
        qDebug() << "\nMath parser error:\n"
                 << "--------------------------------------\n"
                 << "Message:     " << e.GetMsg() << "\n"
                 << "Expression:  " << e.GetExpr() << "\n"
                 << "--------------------------------------";
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplaceTokenLabel(const QString &token, const QString &oldLabel, const QString &newLabel) -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Token patterns with possible duplicate suffix or ID at the end
    const QStringList twoLabelPrefixes = {"Line",
                                          "AngleLine",
                                          "Angle1Spl",
                                          "Angle2Spl",
                                          "Angle1SplPath",
                                          "Angle2SplPath",
                                          "C1LengthSpl",
                                          "C2LengthSpl",
                                          "C1LengthSplPath",
                                          "C2LengthSplPath",
                                          "Spl",
                                          "SplPath"};

    const QStringList oneLabelPrefixes = {"RadiusArc",
                                          "Angle1Arc",
                                          "Angle2Arc",
                                          "Angle1ElArc",
                                          "Angle2ElArc",
                                          "Arc",
                                          "ElArc",
                                          "Radius1ElArc",
                                          "Radius2ElArc"};

    // Escape special regex characters in labels
    const QString escapedOldLabel = QRegularExpression::escape(oldLabel);

    // Check two-label patterns
    for (const auto &prefix : twoLabelPrefixes)
    {
        const QString escapedPrefix = QRegularExpression::escape(prefix);

        // Pattern with optional duplicate suffix: prefix_label1_label2 or prefix_label1_label2_<number>
        // Match label1 position: prefix_oldLabel_<anything> or prefix_oldLabel_<anything>_<number>
        const QRegularExpression regex1(
            QStringLiteral("^(%1)_(%2)_(.+?)(?:_(\\d+))?$").arg(escapedPrefix, escapedOldLabel));
        if (const QRegularExpressionMatch match = regex1.match(token); match.hasMatch())
        {
            const QString label2 = match.captured(3);
            const QString suffix = match.captured(4); // May be empty

            if (suffix.isEmpty())
            {
                return QStringLiteral("%1_%2_%3").arg(match.captured(1), newLabel, label2);
            }

            return QStringLiteral("%1_%2_%3_%4").arg(match.captured(1), newLabel, label2, suffix);
        }

        // Match label2 position: prefix_<anything>_oldLabel or prefix_<anything>_oldLabel_<number>
        // This is tricky because we need to check if oldLabel ends the token or is followed by _<number>

        // First try: prefix_<something>_oldLabel_<number>
        const QRegularExpression regex2a(QStringLiteral("^(%1)_(.+?)_(%2)_(\\d+)$").arg(escapedPrefix, escapedOldLabel));
        if (const QRegularExpressionMatch match = regex2a.match(token); match.hasMatch())
        {
            return QStringLiteral("%1_%2_%3_%4").arg(match.captured(1), match.captured(2), newLabel, match.captured(4));
        }

        // Second try: prefix_<something>_oldLabel (no suffix)
        const QRegularExpression regex2b(QStringLiteral("^(%1)_(.+?)_(%2)$").arg(escapedPrefix, escapedOldLabel));
        if (const QRegularExpressionMatch match = regex2b.match(token); match.hasMatch())
        {
            return QStringLiteral("%1_%2_%3").arg(match.captured(1), match.captured(2), newLabel);
        }
    }

    // Check one-label patterns (with ID or ID_duplicate)
    for (const auto &prefix : oneLabelPrefixes)
    {
        const QString escapedPrefix = QRegularExpression::escape(prefix);

        // Pattern: prefix_label_<id> or prefix_label_<id>_<duplicate>
        // Match: prefix_oldLabel_<number> or prefix_oldLabel_<number>_<number>
        const QRegularExpression regex1(
            QStringLiteral("^(%1)_(%2)_(\\d+)(?:_(\\d+))?$").arg(escapedPrefix, escapedOldLabel));
        if (const QRegularExpressionMatch match = regex1.match(token); match.hasMatch())
        {
            const QString id = match.captured(3);
            const QString duplicate = match.captured(4); // May be empty

            if (duplicate.isEmpty())
            {
                return QStringLiteral("%1_%2_%3").arg(match.captured(1), newLabel, id);
            }
            return QStringLiteral("%1_%2_%3_%4").arg(match.captured(1), newLabel, id, duplicate);
        }
    }

    return token; // No replacement needed
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplaceTokenPair(const QString &token,
                      RenameObjectType type,
                      const ObjectPair_t &oldPair,
                      const ObjectPair_t &newPair,
                      quint32 duplicate) -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    QStringList prefixes;

    if (type == RenameObjectType::Line)
    {
        prefixes = {"Line", "AngleLine"};
    }
    else if (type == RenameObjectType::Spline)
    {
        prefixes = {"Angle1Spl", "Angle2Spl", "C1LengthSpl", "C2LengthSpl", "Spl"};
    }
    else // RenameObjectType::SplinePath
    {
        prefixes = {"Angle1SplPath", "Angle2SplPath", "C1LengthSplPath", "C2LengthSplPath", "SplPath"};
    }

    for (const auto &prefix : std::as_const(prefixes))
    {
        QString expectedToken;

        if (duplicate > 0)
        {
            // With specific duplicate number
            expectedToken = QStringLiteral("%1_%2_%3_%4")
                                .arg(prefix, oldPair.first, oldPair.second, QString::number(duplicate));
        }
        else
        {
            // Without duplicate number
            expectedToken = QStringLiteral("%1_%2_%3").arg(prefix, oldPair.first, oldPair.second);
        }

        // Check for exact match with label1_label2 order
        if (token == expectedToken)
        {
            if (duplicate > 0)
            {
                return QStringLiteral("%1_%2_%3_%4")
                    .arg(prefix, newPair.first, newPair.second, QString::number(duplicate));
            }
            return QStringLiteral("%1_%2_%3").arg(prefix, newPair.first, newPair.second);
        }
    }

    return token; // No replacement needed
}

//---------------------------------------------------------------------------------------------------------------------
auto CurveAliasPrefixes(CurveAliasType type) -> QStringList
{
    QStringList prefixes;

    if (type == CurveAliasType::All)
    {
        prefixes = {"Angle1Spl",   "Angle2Spl",       "Angle1SplPath",   "Angle2SplPath", "C1LengthSpl",
                    "C2LengthSpl", "C1LengthSplPath", "C2LengthSplPath", "Spl",           "SplPath",
                    "RadiusArc",   "Angle1Arc",       "Angle2Arc",       "Angle1ElArc",   "Angle2ElArc",
                    "Arc",         "ElArc",           "Radius1ElArc",    "Radius2ElArc",  "RotationElArc"};
    }
    else if (type == CurveAliasType::Arc)
    {
        prefixes = {"RadiusArc", "Angle1Arc", "Angle2Arc", "Arc"};
    }
    else if (type == CurveAliasType::ElArc)
    {
        prefixes = {"Angle1ElArc", "Angle2ElArc", "ElArc", "Radius1ElArc", "Radius2ElArc", "RotationElArc"};
    }
    else if (type == CurveAliasType::Spline)
    {
        prefixes = {"Angle1Spl", "Angle2Spl", "C1LengthSpl", "C2LengthSpl", "Spl"};
    }
    else if (type == CurveAliasType::SplinePath)
    {
        prefixes = {"Angle1SplPath", "Angle2SplPath", "C1LengthSplPath", "C2LengthSplPath", "SplPath"};
    }

    return prefixes;
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplaceTokenAlias(const QString &token, CurveAliasType type, const QString &oldAlias, const QString &newAlias)
    -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    QStringList const prefixes = CurveAliasPrefixes(type);

    if (prefixes.isEmpty())
    {
        return token; // Unexpected type
    }

    for (const auto &prefix : std::as_const(prefixes))
    {
        if (token == QStringLiteral("%1_%2").arg(prefix, oldAlias))
        {
            return QStringLiteral("%1_%2").arg(prefix, newAlias);
        }
    }

    return token; // No replacement needed
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplaceTokenSegmentCurve(const QString &token,
                              CurveAliasType type,
                              const QString &pointName,
                              const QString &leftSub,
                              const QString &rightSub) -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    QStringList const prefixes = CurveAliasPrefixes(type);

    if (prefixes.isEmpty() || pointName.isEmpty())
    {
        return token; // Unexpected type
    }

    static QString validName;
    if (validName.isEmpty())
    {
        // Get valid name pattern and strip \A and \z anchors for embedding
        validName = NameRegExp(VariableRegex::Variable);
        validName.remove(QStringLiteral("\\A"));
        validName.remove(QStringLiteral("\\z"));
    }

    const QString escapedPointName = QRegularExpression::escape(pointName);

    for (const auto &prefix : std::as_const(prefixes))
    {
        const QString escapedPrefix = QRegularExpression::escape(prefix);

        if (!leftSub.isEmpty())
        {
            // Match: prefix_pointName_<someLabel> -> prefix_pointName<leftSub>
            const QRegularExpression regex(
                QStringLiteral("^(%1)_(%2)_(%3)$").arg(escapedPrefix, escapedPointName, validName));
            if (const QRegularExpressionMatch match = regex.match(token); match.hasMatch())
            {
                return QStringLiteral("%1_%2").arg(match.captured(1), leftSub);
            }
        }

        if (!rightSub.isEmpty())
        {
            // Match: prefix_<someLabel>_pointName -> prefix_pointName<rightSub>
            const QRegularExpression regex(
                QStringLiteral("^(%1)_(%2)_(%3)$").arg(escapedPrefix, validName, escapedPointName));
            if (const QRegularExpressionMatch match = regex.match(token); match.hasMatch())
            {
                return QStringLiteral("%1_%2").arg(match.captured(1), rightSub);
            }
        }
    }

    return token; // No replacement needed
}

//---------------------------------------------------------------------------------------------------------------------
auto ReplaceTokenArc(const QString &token,
                     RenameArcType type,
                     const QString &oldCenterLabel,
                     const QString &newCenterLabel,
                     quint32 id,
                     int duplicate) -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    QStringList prefixes;

    if (type == RenameArcType::Arc)
    {
        prefixes = {"Angle1Arc", "Angle2Arc", "RadiusArc", "Arc"};
    }
    else // RenameArcType::ElArc
    {
        prefixes = {"Angle1ElArc", "Angle2ElArc", "Radius1ElArc", "Radius2ElArc", "RotationElArc", "ElArc"};
    }

    for (const auto &prefix : std::as_const(prefixes))
    {
        QString expectedToken;

        if (duplicate > 0)
        {
            // With specific duplicate number
            expectedToken
                = QStringLiteral("%1_%2_%3_%4").arg(prefix, oldCenterLabel).arg(id).arg(QString::number(duplicate));
        }
        else
        {
            // Without duplicate number
            expectedToken = QStringLiteral("%1_%2_%3").arg(prefix, oldCenterLabel).arg(id);
        }

        // Check for exact match with label1_label2 order
        if (token == expectedToken)
        {
            if (duplicate > 0)
            {
                return QStringLiteral("%1_%2_%3_%4").arg(prefix, newCenterLabel).arg(id).arg(QString::number(duplicate));
            }
            return QStringLiteral("%1_%2_%3").arg(prefix, newCenterLabel).arg(id);
        }
    }
    return token; // No replacement needed
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
AbstractObjectRename::AbstractObjectRename(VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent)
{
    if (id != NULL_ID)
    {
        // Do it here in case graph will not be completed when we next time call undo/redo
        const VPatternGraph *graph = doc->PatternGraph();
        auto Filter = [](const auto &node) -> bool { return node.type != VNodeType::OBJECT; };
        m_dependencies = graph->GetDependentNodes(id, Filter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::undo()
{
    m_operationType = OperationType::Undo;
    RenameFormulas();
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::redo()
{
    m_operationType = OperationType::Redo;
    RenameFormulas();
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::RenameFormulas()
{
    if (m_operationType == OperationType::Unknown)
    {
        return;
    }

    // Check if all tags handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    const QStringList validTags = {VAbstractPattern::TagPoint,
                                   VAbstractPattern::TagOperation,
                                   VAbstractPattern::TagArc,
                                   VAbstractPattern::TagElArc,
                                   VAbstractPattern::TagSpline,
                                   VAbstractPattern::TagPath,
                                   VAbstractPattern::TagTools,
                                   VAbstractPattern::TagDetail};

    for (const auto &node : std::as_const(m_dependencies))
    {
        QDomElement domElement = Doc()->FindElementById(node.id);
        if (!domElement.isElement())
        {
            continue;
        }

        if (!validTags.contains(domElement.tagName()))
        {
            continue;
        }

        ProcessElementByType(domElement);
    }

    if (m_operationType != OperationType::Undo)
    {
        emit NeedLiteParsing(Document::LiteParse);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessElementByType(QDomElement &element) const
{
    if (!element.isElement())
    {
        return;
    }

    // Check if all tags handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    if (const QString tagName = element.tagName(); tagName == VAbstractPattern::TagPoint)
    {
        ProcessPointElement(element);
    }
    else if (tagName == VAbstractPattern::TagOperation)
    {
        ProcessOperationElement(element);
    }
    else if (tagName == VAbstractPattern::TagArc)
    {
        ProcessArcElement(element);
    }
    else if (tagName == VAbstractPattern::TagElArc)
    {
        ProcessElArcElement(element);
    }
    else if (tagName == VAbstractPattern::TagSpline)
    {
        ProcessSplineElement(element);
    }
    else if (tagName == VAbstractPattern::TagPath)
    {
        ProcessPathElement(element);
    }
    else if (tagName == VAbstractPattern::TagTools)
    {
        ProcessToolsElement(element);
    }
    else if (tagName == VAbstractPattern::TagDetail)
    {
        ProcessDetailElement(element);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessPointElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Point elements have no children with formulas, only direct attributes
    const QStringList attrs = {AttrLength,
                               AttrAngle,
                               AttrRadius,
                               AttrC1Radius,
                               AttrC2Radius,
                               AttrCRadius,
                               AttrWidth,
                               AttrHeight,
                               VAbstractPattern::AttrVisible};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessOperationElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Formulas saved only in direct attributes
    const QStringList attrs = {AttrAngle, AttrLength, AttrRotationAngle};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessArcElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Arc elements have no children with formulas, only direct attributes
    const QStringList attrs = {AttrLength, AttrAngle1, AttrAngle2, AttrRadius};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessElArcElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Elliptical arc elements have no children with formulas, only direct attributes
    const QStringList attrs = {AttrLength, AttrAngle1, AttrAngle2, AttrRadius1, AttrRadius2, AttrRotationAngle};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessSplineElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process spline's own attributes
    const QStringList attrs = {AttrAngle1, AttrAngle2, AttrLength1, AttrLength2, AttrWidth};
    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr);
    }

    // Process pathPoint children
    const QDomNodeList pathPoints = element.elementsByTagName(TagPathPoint);
    for (int i = 0; i < pathPoints.count(); ++i)
    {
        QDomElement pathPoint = pathPoints.at(i).toElement();
        const QStringList pathPointAttrs
            = {AttrKAsm2, AttrAngle, AttrAngle1, AttrAngle2, AttrLength1, AttrLength2, AttrKAsm1};
        for (const auto &attr : pathPointAttrs)
        {
            ProcessAttribute(pathPoint, attr);
        }
    }

    // Process offset children
    const QDomNodeList offsets = element.elementsByTagName(VAbstractPattern::TagOffset);
    for (int i = 0; i < offsets.count(); ++i)
    {
        QDomElement offset = offsets.at(i).toElement();
        ProcessAttribute(offset, AttrWidth);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessNodeElement(QDomElement &node) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    const QStringList nodeAttrs = {VAbstractPattern::AttrSABefore,
                                   VAbstractPattern::AttrSAAfter,
                                   VAbstractPattern::AttrPassmarkLength,
                                   VAbstractPattern::AttrPassmarkWidth,
                                   VAbstractPattern::AttrPassmarkAngle};
    for (const auto &attr : nodeAttrs)
    {
        ProcessAttribute(node, attr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessPathElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process path's own attributes
    ProcessAttribute(element, VAbstractPattern::AttrVisible);

    // Process nodes
    const QDomNodeList nodeList = element.elementsByTagName(VAbstractPattern::TagNode);
    for (int i = 0; i < nodeList.count(); ++i)
    {
        QDomElement node = nodeList.at(i).toElement();
        ProcessNodeElement(node);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessToolsElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process tools' own attributes
    ProcessAttribute(element, VAbstractPattern::AttrVisible);

    // Process nodes
    const QDomNodeList nodeList = element.elementsByTagName(VAbstractPattern::TagNode);
    for (int i = 0; i < nodeList.count(); ++i)
    {
        QDomElement node = nodeList.at(i).toElement();
        ProcessNodeElement(node);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessDetailElement(QDomElement &element) const
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process detail's own attributes
    ProcessAttribute(element, AttrWidth);

    // Process data child - can use firstChildElement for single elements
    QDomElement data = element.firstChildElement(VAbstractPattern::TagData);
    if (!data.isNull())
    {
        const QStringList dataAttrs = {AttrWidth, AttrHeight, VAbstractPattern::AttrRotation};
        for (const auto &attr : dataAttrs)
        {
            ProcessAttribute(data, attr);
        }
    }

    // Process patternInfo child
    QDomElement patternInfo = element.firstChildElement(VAbstractPattern::TagPatternInfo);
    if (!patternInfo.isNull())
    {
        const QStringList patternInfoAttrs = {AttrWidth, AttrHeight, VAbstractPattern::AttrRotation};
        for (const auto &attr : patternInfoAttrs)
        {
            ProcessAttribute(patternInfo, attr);
        }
    }

    // Process grainline child
    QDomElement grainline = element.firstChildElement(VAbstractPattern::TagGrainline);
    if (!grainline.isNull())
    {
        const QStringList grainlineAttrs = {AttrLength, VAbstractPattern::AttrRotation};
        for (const auto &attr : grainlineAttrs)
        {
            ProcessAttribute(grainline, attr);
        }
    }

    // Process mirrorLine child
    QDomElement mirrorLine = element.firstChildElement(VAbstractPattern::TagMirrorLine);
    if (!mirrorLine.isNull())
    {
        const QStringList mirrorLineAttrs = {VAbstractPattern::AttrFoldLineHeightFormula,
                                             VAbstractPattern::AttrFoldLineWidthFormula,
                                             VAbstractPattern::AttrFoldLineCenterFormula};
        for (const auto &attr : mirrorLineAttrs)
        {
            ProcessAttribute(mirrorLine, attr);
        }
    }

    // Process nodes
    const QDomNodeList nodeList = element.elementsByTagName(VAbstractPattern::TagNode);
    for (int i = 0; i < nodeList.count(); ++i)
    {
        QDomElement node = nodeList.at(i).toElement();
        ProcessNodeElement(node);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractObjectRename::ProcessAttribute(QDomElement &element, const QString &attr) const
{
    if (!element.hasAttribute(attr))
    {
        return;
    }

    const QString formula = element.attribute(attr);
    const QString newFormula = ProcessFormula(formula);
    if (newFormula != formula)
    {
        element.setAttribute(attr, newFormula);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractObjectRename::ProcessFormula(const QString &formula) const -> QString
{
    if (formula.isEmpty())
    {
        return formula;
    }

    QMap<vsizetype, QString> tokens = FormulaTokens(formula);
    if (tokens.isEmpty())
    {
        return formula;
    }

    QString result = formula;

    // Process tokens in reverse order to maintain correct positions during replacement
    QList<vsizetype> positions = tokens.keys();
    std::sort(positions.begin(), positions.end(), std::greater<>());

    for (const vsizetype pos : std::as_const(positions))
    {
        const QString &token = tokens[pos];
        const QString newToken = ProcessToken(token);

        if (newToken != token)
        {
            result.replace(pos, token.length(), newToken);
        }
    }

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
RenameLabel::RenameLabel(QString oldLabel, QString newLabel, VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : AbstractObjectRename(doc, id, parent),
    m_oldLabel(std::move(oldLabel)),
    m_newLabel(std::move(newLabel))
{
    SCASSERT(!m_oldLabel.isEmpty())
    SCASSERT(!m_newLabel.isEmpty())
}

//---------------------------------------------------------------------------------------------------------------------
auto RenameLabel::ProcessToken(const QString &token) const -> QString
{
    if (ProcessType() == OperationType::Unknown)
    {
        return token;
    }

    if (ProcessType() == OperationType::Redo)
    {
        return ReplaceTokenLabel(token, m_oldLabel, m_newLabel);
    }

    return ReplaceTokenLabel(token, m_newLabel, m_oldLabel);
}

//---------------------------------------------------------------------------------------------------------------------
RenamePair::RenamePair(RenameObjectType type,
                       ObjectPair_t oldPair,
                       ObjectPair_t newPair,
                       quint32 duplicate,
                       VAbstractPattern *doc,
                       quint32 id,
                       QUndoCommand *parent)
  : AbstractObjectRename(doc, id, parent),
    m_type(type),
    m_oldPair(std::move(oldPair)),
    m_newPair(std::move(newPair)),
    m_duplicate(duplicate)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePair::CreateForLine(const ObjectPair_t &oldPair,
                               const ObjectPair_t &newPair,
                               VAbstractPattern *doc,
                               quint32 id,
                               QUndoCommand *parent) -> RenamePair *
{
    return new RenamePair(RenameObjectType::Line, oldPair, newPair, 0, doc, id, parent);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePair::CreateForSpline(const ObjectPair_t &oldPair,
                                 const ObjectPair_t &newPair,
                                 quint32 duplicate,
                                 VAbstractPattern *doc,
                                 quint32 id,
                                 QUndoCommand *parent) -> RenamePair *
{
    return new RenamePair(RenameObjectType::Spline, oldPair, newPair, duplicate, doc, id, parent);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePair::CreateForSplinePath(const ObjectPair_t &oldPair,
                                     const ObjectPair_t &newPair,
                                     quint32 duplicate,
                                     VAbstractPattern *doc,
                                     quint32 id,
                                     QUndoCommand *parent) -> RenamePair *
{
    return new RenamePair(RenameObjectType::SplinePath, oldPair, newPair, duplicate, doc, id, parent);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePair::ProcessToken(const QString &token) const -> QString
{
    if (ProcessType() == OperationType::Unknown)
    {
        return token;
    }

    if (ProcessType() == OperationType::Redo)
    {
        return ReplaceTokenPair(token, m_type, m_oldPair, m_newPair, m_duplicate);
    }

    return ReplaceTokenPair(token, m_type, m_newPair, m_oldPair, m_duplicate);
}

//---------------------------------------------------------------------------------------------------------------------
RenameAlias::RenameAlias(
    CurveAliasType type, QString oldAlias, QString newAlias, VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : AbstractObjectRename(doc, id, parent),
    m_type(type),
    m_oldAlias(std::move(oldAlias)),
    m_newAlias(std::move(newAlias))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto RenameAlias::CurveType(GOType objType) -> CurveAliasType
{
    switch (objType)
    {
        case GOType::Arc:
            return CurveAliasType::Arc;
        case GOType::EllipticalArc:
            return CurveAliasType::ElArc;
        case GOType::CubicBezier:
        case GOType::Spline:
            return CurveAliasType::Spline;
        case GOType::SplinePath:
        case GOType::CubicBezierPath:
            return CurveAliasType::SplinePath;
        case GOType::PlaceLabel:
        case GOType::Unknown:
        case GOType::Point:
        default:
            return CurveAliasType::All;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto RenameAlias::ProcessToken(const QString &token) const -> QString
{
    if (ProcessType() == OperationType::Unknown)
    {
        return token;
    }

    if (ProcessType() == OperationType::Redo)
    {
        return ReplaceTokenAlias(token, m_type, m_oldAlias, m_newAlias);
    }

    return ReplaceTokenAlias(token, m_type, m_newAlias, m_oldAlias);
}

//---------------------------------------------------------------------------------------------------------------------
RenameArc::RenameArc(RenameArcType type,
                     QString oldCenterLabel,
                     QString newCenterLabel,
                     int duplicate,
                     VAbstractPattern *doc,
                     quint32 id,
                     QUndoCommand *parent)
  : AbstractObjectRename(doc, id, parent),
    m_type(type),
    m_oldCenterLabel(std::move(oldCenterLabel)),
    m_newCenterLabel(std::move(newCenterLabel)),
    m_duplicate(duplicate)
{
    SCASSERT(!m_oldCenterLabel.isEmpty());
    SCASSERT(!m_newCenterLabel.isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
auto RenameArc::ProcessToken(const QString &token) const -> QString
{
    if (ProcessType() == OperationType::Unknown)
    {
        return token;
    }

    if (ProcessType() == OperationType::Redo)
    {
        return ReplaceTokenArc(token, m_type, m_oldCenterLabel, m_newCenterLabel, ElementId(), m_duplicate);
    }

    return ReplaceTokenArc(token, m_type, m_newCenterLabel, m_oldCenterLabel, ElementId(), m_duplicate);
}

//---------------------------------------------------------------------------------------------------------------------
RenameSegmentCurves::RenameSegmentCurves(CurveAliasType type,
                                         QString pointName,
                                         QString leftSub,
                                         QString rightSub,
                                         VAbstractPattern *doc,
                                         QUndoCommand *parent)
  : AbstractObjectRename(doc, NULL_ID, parent),
    m_type(type),
    m_pointName(std::move(pointName)),
    m_leftSub(std::move(leftSub)),
    m_rightSub(std::move(rightSub))
{
}

//---------------------------------------------------------------------------------------------------------------------
void RenameSegmentCurves::undo()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void RenameSegmentCurves::redo()
{
    const QVector<VToolRecord> *history = Doc()->getHistory();
    QSet<quint32> processedTools;
    processedTools.reserve(history->size());
    for (const auto &record : *history)
    {
        processedTools.insert(record.GetId());
    }

    // Check if all tags handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    const QStringList validTags = {VAbstractPattern::TagPoint,
                                   VAbstractPattern::TagOperation,
                                   VAbstractPattern::TagArc,
                                   VAbstractPattern::TagElArc,
                                   VAbstractPattern::TagSpline,
                                   VAbstractPattern::TagPath,
                                   VAbstractPattern::TagTools,
                                   VAbstractPattern::TagDetail};

    for (const QString &tagName : validTags)
    {
        const QDomNodeList elements = Doc()->elementsByTagName(tagName);
        for (int i = 0; i < elements.size(); ++i)
        {
            QDomElement domElement = elements.at(i).toElement();
            if (domElement.isNull())
            {
                continue;
            }

            const quint32 id = domElement.attribute(VAbstractPattern::AttrId, NULL_ID_STR).toUInt();
            if (!processedTools.contains(id))
            {
                ProcessElementByType(domElement);
            }
        }
    }

    setObsolete(true); // Replace works only in one way
}

//---------------------------------------------------------------------------------------------------------------------
auto RenameSegmentCurves::ProcessToken(const QString &token) const -> QString
{
    return ReplaceTokenSegmentCurve(token, m_type, m_pointName, m_leftSub, m_rightSub);
}
