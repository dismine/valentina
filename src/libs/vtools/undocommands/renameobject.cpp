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
auto ProcessFormula(const QString &formula, const QString &oldLabel, const QString &newLabel) -> QString
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
        const QString newToken = ReplaceTokenLabel(token, oldLabel, newLabel);

        if (newToken != token)
        {
            result.replace(pos, token.length(), newToken);
        }
    }

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessAttribute(QDomElement &element, const QString &attr, const QString &oldLabel, const QString &newLabel)
{
    if (!element.hasAttribute(attr))
    {
        return;
    }

    const QString formula = element.attribute(attr);
    const QString newFormula = ProcessFormula(formula, oldLabel, newLabel);
    if (newFormula != formula)
    {
        element.setAttribute(attr, newFormula);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessPointElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
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
        ProcessAttribute(element, attr, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessOperationElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Formulas saved only in direct attributes
    const QStringList attrs = {AttrAngle, AttrLength, AttrRotationAngle};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessArcElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Arc elements have no children with formulas, only direct attributes
    const QStringList attrs = {AttrLength, AttrAngle1, AttrAngle2, AttrRadius};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessElArcElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Elliptical arc elements have no children with formulas, only direct attributes
    const QStringList attrs = {AttrLength, AttrAngle1, AttrAngle2, AttrRadius1, AttrRadius2, AttrRotationAngle};

    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessSplineElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process spline's own attributes
    const QStringList attrs = {AttrAngle1, AttrAngle2, AttrLength1, AttrLength2, AttrWidth};
    for (const auto &attr : attrs)
    {
        ProcessAttribute(element, attr, oldLabel, newLabel);
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
            ProcessAttribute(pathPoint, attr, oldLabel, newLabel);
        }
    }

    // Process offset children
    const QDomNodeList offsets = element.elementsByTagName(VAbstractPattern::TagOffset);
    for (int i = 0; i < offsets.count(); ++i)
    {
        QDomElement offset = offsets.at(i).toElement();
        ProcessAttribute(offset, AttrWidth, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessNodeElement(QDomElement &node, const QString &oldLabel, const QString &newLabel)
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
        ProcessAttribute(node, attr, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessPathElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process path's own attributes
    ProcessAttribute(element, VAbstractPattern::AttrVisible, oldLabel, newLabel);

    // Process nodes
    const QDomNodeList nodeList = element.elementsByTagName(VAbstractPattern::TagNode);
    for (int i = 0; i < nodeList.count(); ++i)
    {
        QDomElement node = nodeList.at(i).toElement();
        ProcessNodeElement(node, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessToolsElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process tools' own attributes
    ProcessAttribute(element, VAbstractPattern::AttrVisible, oldLabel, newLabel);

    // Process nodes
    const QDomNodeList nodeList = element.elementsByTagName(VAbstractPattern::TagNode);
    for (int i = 0; i < nodeList.count(); ++i)
    {
        QDomElement node = nodeList.at(i).toElement();
        ProcessNodeElement(node, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessDetailElement(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    // Check if all attributes handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    // Process detail's own attributes
    ProcessAttribute(element, AttrWidth, oldLabel, newLabel);

    // Process data child - can use firstChildElement for single elements
    QDomElement data = element.firstChildElement(VAbstractPattern::TagData);
    if (!data.isNull())
    {
        const QStringList dataAttrs = {AttrWidth, AttrHeight, VAbstractPattern::AttrRotation};
        for (const auto &attr : dataAttrs)
        {
            ProcessAttribute(data, attr, oldLabel, newLabel);
        }
    }

    // Process patternInfo child
    QDomElement patternInfo = element.firstChildElement(VAbstractPattern::TagPatternInfo);
    if (!patternInfo.isNull())
    {
        const QStringList patternInfoAttrs = {AttrWidth, AttrHeight, VAbstractPattern::AttrRotation};
        for (const auto &attr : patternInfoAttrs)
        {
            ProcessAttribute(patternInfo, attr, oldLabel, newLabel);
        }
    }

    // Process grainline child
    QDomElement grainline = element.firstChildElement(VAbstractPattern::TagGrainline);
    if (!grainline.isNull())
    {
        const QStringList grainlineAttrs = {AttrLength, VAbstractPattern::AttrRotation};
        for (const auto &attr : grainlineAttrs)
        {
            ProcessAttribute(grainline, attr, oldLabel, newLabel);
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
            ProcessAttribute(mirrorLine, attr, oldLabel, newLabel);
        }
    }

    // Process nodes
    const QDomNodeList nodeList = element.elementsByTagName(VAbstractPattern::TagNode);
    for (int i = 0; i < nodeList.count(); ++i)
    {
        QDomElement node = nodeList.at(i).toElement();
        ProcessNodeElement(node, oldLabel, newLabel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ProcessElementByType(QDomElement &element, const QString &oldLabel, const QString &newLabel)
{
    if (!element.isElement())
    {
        return;
    }

    // Check if all tags handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    if (const QString tagName = element.tagName(); tagName == VAbstractPattern::TagPoint)
    {
        ProcessPointElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagOperation)
    {
        ProcessOperationElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagArc)
    {
        ProcessArcElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagElArc)
    {
        ProcessElArcElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagSpline)
    {
        ProcessSplineElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagPath)
    {
        ProcessPathElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagTools)
    {
        ProcessToolsElement(element, oldLabel, newLabel);
    }
    else if (tagName == VAbstractPattern::TagDetail)
    {
        ProcessDetailElement(element, oldLabel, newLabel);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
RenameLabel::RenameLabel(QString oldLabel, QString newLabel, VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_oldLabel(std::move(oldLabel)),
    m_newLabel(std::move(newLabel))
{
    SCASSERT(!m_oldLabel.isEmpty())
    SCASSERT(!m_newLabel.isEmpty())

    nodeId = id;

    // Do it here in case graph will not be completed when we next time call undo/redo
    const VPatternGraph *graph = doc->PatternGraph();
    auto Filter = [](const auto &node) -> bool { return node.type != VNodeType::OBJECT; };
    m_dependencies = graph->GetDependentNodes(id, Filter);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameLabel::undo()
{
    RenameLabelInFormulas(m_newLabel, m_oldLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameLabel::redo()
{
    RenameLabelInFormulas(m_oldLabel, m_newLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameLabel::RenameLabelInFormulas(const QString &oldLabel, const QString &newLabel)
{
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
        QDomElement domElement = doc->FindElementById(node.id);
        if (!domElement.isElement())
        {
            continue;
        }

        if (!validTags.contains(domElement.tagName()))
        {
            continue;
        }

        ProcessElementByType(domElement, oldLabel, newLabel);
    }

    if (!m_dependencies.isEmpty())
    {
        emit NeedLiteParsing(Document::LiteParse);
    }
}
