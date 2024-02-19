/************************************************************************
 **
 **  @file   testpassmark.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 9, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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

#include "testpassmark.h"
#include "vpassmark.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>

#include "../vmisc/testpath.h"

#if !defined(V_NO_ASSERT)
namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto PassmarkShapeToJson(const QVector<QLineF> &shape) -> QJsonArray
{
    QJsonArray shapeArray;
    for (auto line : shape)
    {
        QJsonObject const lineObject{
            {"type", "QLineF"},
            {"p1", PointToJson(line.p1())},
            {"p2", PointToJson(line.p2())},
        };

        shapeArray.append(lineObject);
    }

    return shapeArray;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void DumpPassmarkData(const VPiecePassmarkData &data, const QString &templateName)
{
    QTemporaryFile temp;       // Go to tmp folder to find dump
    temp.setAutoRemove(false); // Remove dump manually

    if (not templateName.isEmpty())
    {
        temp.setFileTemplate(QDir::tempPath() + QDir::separator() + templateName);
    }

    if (temp.open())
    {
#if defined(Q_OS_LINUX)
        //        On Linux, QTemporaryFile will attempt to create unnamed temporary
        //        files. If that succeeds, open() will return true but exists() will be
        //        false. If you call fileName() or any function that calls it,
        //        QTemporaryFile will give the file a name, so most applications will
        //        not see a difference.
        temp.fileName(); // call to create a file on disk
#endif
        QJsonObject const dataObject{
            {"data", data.toJson()},
        };
        QJsonDocument const vector(dataObject);

        QTextStream out(&temp);
        out << vector.toJson();
        out.flush();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DumpPassmarkShape(const QVector<QLineF> &shape, const QString &templateName)
{
    QTemporaryFile temp;       // Go to tmp folder to find dump
    temp.setAutoRemove(false); // Remove dump manually

    if (not templateName.isEmpty())
    {
        temp.setFileTemplate(QDir::tempPath() + QDir::separator() + templateName);
    }

    if (temp.open())
    {
#if defined(Q_OS_LINUX)
        //        On Linux, QTemporaryFile will attempt to create unnamed temporary
        //        files. If that succeeds, open() will return true but exists() will be
        //        false. If you call fileName() or any function that calls it,
        //        QTemporaryFile will give the file a name, so most applications will
        //        not see a difference.
        temp.fileName(); // call to create a file on disk
#endif
        QJsonObject const shapeObject{
            {"shape", PassmarkShapeToJson(shape)},
        };
        QJsonDocument const vector(shapeObject);

        QTextStream out(&temp);
        out << vector.toJson();
        out.flush();
    }
}

#endif // !defined(V_NO_ASSERT)
