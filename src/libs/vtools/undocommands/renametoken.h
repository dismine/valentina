/************************************************************************
 **
 **  @file   renametoken.h
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
#ifndef RENAMETOKEN_H
#define RENAMETOKEN_H

#include <QString>
#include <QtGlobal>
#include <utility>

enum class RenameObjectType : quint8
{
    Line,
    Spline,
    SplinePath
};

using ObjectPair_t = std::pair<QString, QString>;

enum class CurveAliasType : quint8
{
    Arc,
    ElArc,
    Spline,
    SplinePath,
    All
};

enum class RenameArcType : quint8
{
    Arc,
    ElArc
};

auto ReplaceTokenLabel(const QString &token, const QString &oldLabel, const QString &newLabel) -> QString;

auto ReplaceTokenPair(const QString &token,
                      RenameObjectType type,
                      const ObjectPair_t &oldPair,
                      const ObjectPair_t &newPair,
                      quint32 oldDuplicate,
                      quint32 newDuplicate) -> QString;

auto ReplaceTokenAlias(const QString &token, CurveAliasType type, const QString &oldAlias, const QString &newAlias)
    -> QString;

auto ReplaceTokenSegmentCurve(const QString &token,
                              CurveAliasType type,
                              const QString &pointName,
                              const QString &leftSub,
                              const QString &rightSub) -> QString;

auto ReplaceTokenArc(const QString &token,
                     RenameArcType type,
                     const QString &oldCenterLabel,
                     const QString &newCenterLabel,
                     quint32 id,
                     quint32 oldDuplicate,
                     quint32 newDuplicate) -> QString;

#endif // RENAMETOKEN_H
