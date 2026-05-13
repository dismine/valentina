/************************************************************************
 **
 **  @file   renametoken.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 5, 2026
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
#include "renametoken.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/def.h"

#include <QRegularExpression>
#include <QStringList>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CurveAliasPrefixes(CurveAliasType type) -> QStringList
{
    QStringList prefixes;

    if (type == CurveAliasType::All)
    {
        prefixes = QStringList{"Angle1Spl",   "Angle2Spl",       "Angle1SplPath",   "Angle2SplPath", "C1LengthSpl",
                               "C2LengthSpl", "C1LengthSplPath", "C2LengthSplPath", "Spl",           "SplPath",
                               "RadiusArc",   "Angle1Arc",       "Angle2Arc",       "Angle1ElArc",   "Angle2ElArc",
                               "Arc",         "ElArc",           "Radius1ElArc",    "Radius2ElArc",  "RotationElArc"};
    }
    else if (type == CurveAliasType::Arc)
    {
        prefixes = QStringList{"RadiusArc", "Angle1Arc", "Angle2Arc", "Arc"};
    }
    else if (type == CurveAliasType::ElArc)
    {
        prefixes = QStringList{"Angle1ElArc", "Angle2ElArc", "ElArc", "Radius1ElArc", "Radius2ElArc", "RotationElArc"};
    }
    else if (type == CurveAliasType::Spline)
    {
        prefixes = QStringList{"Angle1Spl", "Angle2Spl", "C1LengthSpl", "C2LengthSpl", "Spl"};
    }
    else if (type == CurveAliasType::SplinePath)
    {
        prefixes = QStringList{"Angle1SplPath", "Angle2SplPath", "C1LengthSplPath", "C2LengthSplPath", "SplPath"};
    }

    return prefixes;
}
} // namespace

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

            if (newLabel == label2)
            {
                return token; // Would produce duplicate labels (e.g. Spl_B_B)
            }

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
            if (newLabel == match.captured(2))
            {
                return token; // Would produce duplicate labels
            }
            return QStringLiteral("%1_%2_%3_%4").arg(match.captured(1), match.captured(2), newLabel, match.captured(4));
        }

        // Second try: prefix_<something>_oldLabel (no suffix)
        const QRegularExpression regex2b(QStringLiteral("^(%1)_(.+?)_(%2)$").arg(escapedPrefix, escapedOldLabel));
        if (const QRegularExpressionMatch match = regex2b.match(token); match.hasMatch())
        {
            if (newLabel == match.captured(2))
            {
                return token; // Would produce duplicate labels
            }
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
                      quint32 oldDuplicate,
                      quint32 newDuplicate) -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    if (newPair.first == newPair.second)
    {
        return token; // Would produce duplicate labels (e.g. Line_B_B)
    }

    QStringList prefixes;

    if (type == RenameObjectType::Line)
    {
        prefixes = QStringList{"Line", "AngleLine"};
    }
    else if (type == RenameObjectType::Spline)
    {
        prefixes = QStringList{"Angle1Spl", "Angle2Spl", "C1LengthSpl", "C2LengthSpl", "Spl"};
    }
    else // RenameObjectType::SplinePath
    {
        prefixes = QStringList{"Angle1SplPath", "Angle2SplPath", "C1LengthSplPath", "C2LengthSplPath", "SplPath"};
    }

    for (const auto &prefix : std::as_const(prefixes))
    {
        QString expectedToken;

        if (oldDuplicate > 0)
        {
            // With specific duplicate number
            expectedToken = QStringLiteral("%1_%2_%3_%4")
                                .arg(prefix, oldPair.first, oldPair.second, QString::number(oldDuplicate));
        }
        else
        {
            // Without duplicate number
            expectedToken = QStringLiteral("%1_%2_%3").arg(prefix, oldPair.first, oldPair.second);
        }

        // Check for exact match with label1_label2 order
        if (token == expectedToken)
        {
            if (newDuplicate > 0)
            {
                return QStringLiteral("%1_%2_%3_%4")
                    .arg(prefix, newPair.first, newPair.second, QString::number(newDuplicate));
            }
            return QStringLiteral("%1_%2_%3").arg(prefix, newPair.first, newPair.second);
        }
    }

    return token; // No replacement needed
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
            // Match: prefix_<someLabel>_pointName -> prefix_pointName<leftSub>
            const QRegularExpression regex(
                QStringLiteral("^(%1)_(%2)_(%3)$").arg(escapedPrefix, validName, escapedPointName));
            if (const QRegularExpressionMatch match = regex.match(token); match.hasMatch())
            {
                return QStringLiteral("%1_%2").arg(match.captured(1), leftSub);
            }
        }

        if (!rightSub.isEmpty())
        {
            // Match: prefix_pointName_<someLabel> -> prefix_pointName<rightSub>
            const QRegularExpression regex(
                QStringLiteral("^(%1)_(%2)_(%3)$").arg(escapedPrefix, escapedPointName, validName));
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
                     quint32 oldDuplicate,
                     quint32 newDuplicate) -> QString
{
    // Check if all variable types handled when we have new tool
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64);

    QStringList prefixes;

    if (type == RenameArcType::Arc)
    {
        prefixes = QStringList{"Angle1Arc", "Angle2Arc", "RadiusArc", "Arc"};
    }
    else // RenameArcType::ElArc
    {
        prefixes = QStringList{"Angle1ElArc", "Angle2ElArc", "Radius1ElArc", "Radius2ElArc", "RotationElArc", "ElArc"};
    }

    for (const auto &prefix : std::as_const(prefixes))
    {
        QString expectedToken;

        if (oldDuplicate > 0)
        {
            // With specific duplicate number
            expectedToken
                = QStringLiteral("%1_%2_%3_%4").arg(prefix, oldCenterLabel).arg(id).arg(QString::number(oldDuplicate));
        }
        else
        {
            // Without duplicate number
            expectedToken = QStringLiteral("%1_%2_%3").arg(prefix, oldCenterLabel).arg(id);
        }

        // Check for exact match with label1_label2 order
        if (token == expectedToken)
        {
            if (newDuplicate > 0)
            {
                return QStringLiteral("%1_%2_%3_%4")
                    .arg(prefix, newCenterLabel)
                    .arg(id)
                    .arg(QString::number(newDuplicate));
            }
            return QStringLiteral("%1_%2_%3").arg(prefix, newCenterLabel).arg(id);
        }
    }
    return token; // No replacement needed
}
