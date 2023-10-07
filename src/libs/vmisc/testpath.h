/************************************************************************
 **
 **  @file   testpath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2019
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
#ifndef TESTPATH_H
#define TESTPATH_H

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>
#include <QtCore/qcontainerfwd.h>

class QPointF;
class VSAPoint;
class VRawSAPoint;

#if !defined(V_NO_ASSERT)
auto PointToJson(const QPointF &point) -> QJsonObject;
void VectorToJson(const QVector<QPointF> &points, QJsonObject &json);

template <class T> void VectorToJson(const QVector<T> &points, QJsonObject &json);

//---------------------------------------------------------------------------------------------------------------------
template <class T> void DumpVector(const QVector<T> &points, const QString &templateName = QString())
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        //        On Linux, QTemporaryFile will attempt to create unnamed temporary
        //        files. If that succeeds, open() will return true but exists() will be
        //        false. If you call fileName() or any function that calls it,
        //        QTemporaryFile will give the file a name, so most applications will
        //        not see a difference.
        temp.fileName(); // call to create a file on disk
#endif
#endif
        QJsonObject vectorObject;
        VectorToJson(points, vectorObject);
        QJsonDocument vector(vectorObject);

        QTextStream out(&temp);
        out << vector.toJson();
        out.flush();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> void VectorToJson(const QVector<T> &points, QJsonObject &json)
{
    QJsonArray pointsArray;
    for (auto point : points)
    {
        pointsArray.append(point.toJson());
    }
    json[QLatin1String("vector")] = pointsArray;
}
#endif // !defined(V_NO_ASSERT)
#endif // TESTPATH_H
