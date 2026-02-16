/************************************************************************
 **
 **  @file   toolsdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef TOOLSDEF_H
#define TOOLSDEF_H

#include <QMetaType>
#include <QString>

#include "../ifc/ifcdef.h"

class VGObject;
class VContainer;

struct VRawGraduatedCurveOffset
{
    QString name{};
    QString formula{};
    QString description{};
};

struct SourceItem
{
    quint32 id{NULL_ID};
    QString name{};
    QString penStyle{TypeLineDefault};
    QString color{ColorDefault};
};

Q_DECLARE_METATYPE(SourceItem)
Q_DECLARE_TYPEINFO(SourceItem, Q_MOVABLE_TYPE); // NOLINT

// Default label position
const int labelMX = 10;
const int labelMY = 15;

auto SourceToObjects(const QVector<SourceItem> &source) -> QVector<quint32>;

void FillDefSourceNames(QVector<SourceItem> &source, const VContainer *data, const QString &suffix);

auto GetSourceItemName(const QString &name, quint32 id, const VContainer *data) -> QString;
auto IsValidSourceName(const QString &newName, quint32 id, const QVector<SourceItem> &source, const VContainer *data)
    -> bool;

auto OperationLineStylesPics(QColor backgroundColor, QColor textColor) -> QMap<QString, QIcon>;

auto ConfirmDeletion() -> int;

#endif // TOOLSDEF_H
