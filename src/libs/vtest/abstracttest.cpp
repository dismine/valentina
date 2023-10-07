/************************************************************************
 **
 **  @file   abstracttest.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "abstracttest.h"

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFlags>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineF>
#include <QPointF>
#include <QProcess>
#include <QScopedPointer>
#include <QStringList>
#include <QVector>
#include <QtGlobal>
#include <qtestcase.h>

#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vlayout/vabstractpiece.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpassmark.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
auto FillPath(const QVector<QPointF> &path, qreal accuracy) -> QVector<QPointF>
{
    QVector<QPointF> pathFilled;
    pathFilled.reserve(path.size());

    for (int i = 0; i < path.size() - 1; ++i)
    {
        pathFilled.append(path.at(i));

        QLineF line(path.at(i), path.at(i + 1));
        if (line.length() > accuracy)
        {
            qreal len = accuracy;
            do
            {
                QLineF l = line;
                l.setLength(len);
                pathFilled.append(l.p2());
                len += accuracy;
            } while (line.length() > len);
        }
    }

    pathFilled.append(path.constLast());

    return pathFilled;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
AbstractTest::AbstractTest(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::PieceFromJson(const QString &json, VPiece &piece, QSharedPointer<VContainer> &data)
{
    QByteArray saveData;
    PrepareDocument(json, saveData);
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    const QString testCaseKey = QStringLiteral("testCase");
    const QString bdKey = QStringLiteral("bd");
    const QString pieceKey = QStringLiteral("piece");

    QJsonObject testCaseObject = loadDoc.object();
    TestRoot(testCaseObject, testCaseKey, json);

    QJsonObject testCase = testCaseObject[testCaseKey].toObject();

    if (testCase.contains(bdKey))
    {
        DBFromJson(testCase[bdKey].toObject(), data);
    }
    else
    {
        const QString error = QStringLiteral("Test case json object does not contain db data.");
        QFAIL(qUtf8Printable(error));
    }

    if (testCase.contains(pieceKey))
    {
        MainPathFromJson(testCase[pieceKey].toObject(), piece);
    }
    else
    {
        const QString error = QStringLiteral("Test case json object does not contain piece data.");
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::PassmarkDataFromJson(const QString &json, VPiecePassmarkData &data)
{
    QByteArray saveData;
    PrepareDocument(json, saveData);
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    const QString dataKey = QStringLiteral("data");

    QJsonObject dataObject = loadDoc.object();
    TestRoot(dataObject, dataKey, json);

    QJsonObject passmarkData = dataObject[dataKey].toObject();

    try
    {
        VSAPoint previousSAPoint;
        PointFromJson(passmarkData[QStringLiteral("previousSAPoint")].toObject(), previousSAPoint);
        data.previousSAPoint = previousSAPoint;

        VSAPoint passmarkSAPoint;
        PointFromJson(passmarkData[QStringLiteral("passmarkSAPoint")].toObject(), passmarkSAPoint);
        data.passmarkSAPoint = passmarkSAPoint;

        VSAPoint nextSAPoint;
        PointFromJson(passmarkData[QStringLiteral("nextSAPoint")].toObject(), nextSAPoint);
        data.nextSAPoint = nextSAPoint;
    }
    catch (const VException &e)
    {
        const QString error = QStringLiteral("Invalid json file '%1'. %2").arg(json, e.ErrorMessage());
        QFAIL(qUtf8Printable(error));
    }

    qreal saWidth = 0;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("saWidth"), saWidth);
    data.saWidth = saWidth;

    QString nodeName;
    AbstractTest::ReadStringValue(passmarkData, QStringLiteral("nodeName"), nodeName);
    data.nodeName = nodeName;

    QString pieceName;
    AbstractTest::ReadStringValue(passmarkData, QStringLiteral("pieceName"), pieceName);
    data.pieceName = pieceName;

    PassmarkLineType passmarkLineType = PassmarkLineType::OneLine;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("passmarkLineType"), passmarkLineType,
                                  QString::number(static_cast<int>(PassmarkLineType::OneLine)));
    data.passmarkLineType = passmarkLineType;

    PassmarkAngleType passmarkAngleType = PassmarkAngleType::Straightforward;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("passmarkAngleType"), passmarkAngleType,
                                  QString::number(static_cast<int>(PassmarkAngleType::Straightforward)));
    data.passmarkAngleType = passmarkAngleType;

    bool isMainPathNode = true;
    AbstractTest::ReadBooleanValue(passmarkData, QStringLiteral("isMainPathNode"), isMainPathNode);
    data.isMainPathNode = isMainPathNode;

    bool isShowSecondPassmark = true;
    AbstractTest::ReadBooleanValue(passmarkData, QStringLiteral("isShowSecondPassmark"), isShowSecondPassmark);
    data.isShowSecondPassmark = isShowSecondPassmark;

    int passmarkIndex = -1;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("passmarkIndex"), passmarkIndex, QStringLiteral("-1"));
    data.passmarkIndex = passmarkIndex;

    vidtype id = NULL_ID;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("id"), id, QString::number(NULL_ID));
    data.id = id;

    qreal globalPassmarkLength;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("globalPassmarkLength"), globalPassmarkLength,
                                  QString::number(NULL_ID));
    data.globalPassmarkLength = globalPassmarkLength;

    qreal globalPassmarkWidth;
    AbstractTest::ReadDoubleValue(passmarkData, QStringLiteral("globalPassmarkWidth"), globalPassmarkWidth,
                                  QString::number(NULL_ID));
    data.globalPassmarkWidth = globalPassmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::PassmarkShapeFromJson(const QString &json, QVector<QLineF> &shape)
{
    QByteArray saveData;
    PrepareDocument(json, saveData);
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    const QString shapeKey = QStringLiteral("shape");
    const QString typeKey = QStringLiteral("type");

    QJsonObject shapeObject = loadDoc.object();
    TestRoot(shapeObject, shapeKey, json);

    QJsonArray vectorArray = shapeObject[shapeKey].toArray();
    for (auto item : vectorArray)
    {
        QJsonObject lineObject = item.toObject();

        QString type;
        AbstractTest::ReadStringValue(lineObject, typeKey, type);

        if (type != "QLineF"_L1)
        {
            const QString error = QStringLiteral("Invalid json file '%1'. Unexpected class '%2'.")
                                      .arg(json, lineObject[typeKey].toString());
            QFAIL(qUtf8Printable(error));
        }

        shape.append(QLineFromJson(lineObject));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ComparePaths(const QVector<QPointF> &actual, const QVector<QPointF> &expected)
{
    QVERIFY2(actual.size() >= 2, "Not enough points");
    QVERIFY2(expected.size() >= 2, "Not enough points");

    const qreal accuracy = accuracyPointOnLine * 4.;
    QVector<QPointF> actualFilled = FillPath(actual, accuracy);

    bool onLine = false;
    QSet<int> usedEdges;
    for (auto p : actualFilled)
    {
        for (int i = 0; i < expected.size() - 1; ++i)
        {
            if (VGObject::IsPointOnLineSegment(p, expected.at(i), expected.at(i + 1), accuracyPointOnLine * 2.))
            {
                usedEdges.insert(i + 1);
                onLine = true;
            }
        }

        if (not onLine)
        {
            QFAIL("Paths are not the same. Point is not on edge.");
        }
        onLine = false;
    }

    QVERIFY2(expected.size() - 1 == usedEdges.size(), "Paths are not the same. Not all edges were used.");
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ComparePathsDistance(const QVector<QPointF> &ekv, const QVector<QPointF> &ekvOrig) const
{
    // Begin comparison
    QCOMPARE(ekv.size(), ekvOrig.size()); // First check if sizes are equal
    const qreal testAccuracy = MmToPixel(1.);

    for (int i = 0; i < ekv.size(); i++)
    {
        ComparePointsDistance(ekv.at(i), ekvOrig.at(i), testAccuracy);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ComparePointsDistance(const QPointF &result, const QPointF &expected, qreal testAccuracy) const
{
    const QString msg = QStringLiteral("Actual '%2;%3', Expected '%4;%5'. Distance between points %6 mm.")
                            .arg(result.x())
                            .arg(result.y())
                            .arg(expected.x())
                            .arg(expected.y())
                            .arg(UnitConvertor(QLineF(result, expected).length(), Unit::Px, Unit::Mm));
    // Check each point. Don't use comparison float values
    QVERIFY2(VFuzzyComparePoints(result, expected, testAccuracy), qUtf8Printable(msg));
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::CompareLinesDistance(const QVector<QLineF> &result, const QVector<QLineF> &expected) const
{
    // Begin comparison
    QCOMPARE(result.size(), expected.size()); // First check if sizes equal

    for (int i = 0; i < result.size(); i++)
    {
        const QLineF &line1 = result.at(i);
        const QLineF &line2 = expected.at(i);
        // Check each point. Don't use comparison float values
        QVERIFY2(
            VFuzzyComparePoints(line1.p1(), line2.p1()) && VFuzzyComparePoints(line1.p2(), line2.p2()),
            qUtf8Printable(QStringLiteral("Index: %1. Got line '(%2;%3):(%4;%5)', Expected line '(%6;%7):(%8;%9)'.")
                               .arg(i)
                               .arg(line1.p1().x())
                               .arg(line1.p1().y())
                               .arg(line1.p2().x())
                               .arg(line1.p2().y())
                               .arg(line2.p1().x())
                               .arg(line2.p1().y())
                               .arg(line2.p2().x())
                               .arg(line2.p2().y())));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::PuzzlePath() const -> QString
{
#ifdef QBS_BUILD
    return QStringLiteral(PUZZLE_BUILDDIR);
#else
    const QString path = QStringLiteral("/../../../app/puzzle/bin/puzzle");
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + path + ".exe"_L1;
#else
    return QCoreApplication::applicationDirPath() + path;
#endif
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::ValentinaPath() const -> QString
{
#ifdef QBS_BUILD
    return QStringLiteral(VALENTINA_BUILDDIR);
#else
    const QString path = QStringLiteral("/../../../app/valentina/bin/valentina");
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + path + ".exe"_L1;
#else
    return QCoreApplication::applicationDirPath() + path;
#endif
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::TapePath() const -> QString
{
#ifdef QBS_BUILD
    return QStringLiteral(TAPE_BUILDDIR);
#else
    const QString path = QStringLiteral("/../../../app/tape/bin/tape");
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + path + ".exe"_L1;
#else
    return QCoreApplication::applicationDirPath() + path;
#endif
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::TranslationsPath() -> QString
{
#ifdef QBS_BUILD
    return QStringLiteral(TRANSLATIONS_DIR);
#else
    return QCoreApplication::applicationDirPath() + QStringLiteral("/../../../app/valentina/bin/translations");
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::RunTimeout(int defMsecs) -> int
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    QString timeout = QString::fromLocal8Bit(qgetenv("VTEST_RUN_TIMEOUT"));
    if (timeout.isEmpty())
    {
        return defMsecs;
    }
#else
    QString timeout = qEnvironmentVariable("VTEST_RUN_TIMEOUT", QString::number(defMsecs));
#endif

    bool ok = false;
    int msecs = timeout.toInt(&ok);
    return ok ? msecs : defMsecs;
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::Run(int exit, const QString &program, const QStringList &arguments, QString &error, int msecs) -> int
{
    msecs = AbstractTest::RunTimeout(msecs);

    const QString parameters =
        QStringLiteral("Program: %1 \nArguments: %2.").arg(program, arguments.join(QStringLiteral(", ")));

    QFileInfo info(program);
    if (not info.exists())
    {
        error = QStringLiteral("Can't find binary.\n%1").arg(parameters);
        return TST_EX_BIN;
    }

    QScopedPointer<QProcess> process(new QProcess());
    process->setWorkingDirectory(info.absoluteDir().absolutePath());
    process->start(program, arguments);

    if (not process->waitForStarted(msecs))
    {
        error = QStringLiteral("The start operation timed out or an error occurred.\n%1\n%2")
                    .arg(parameters, QString(process->readAllStandardError()));
        process->kill();
        return TST_EX_START_TIME_OUT;
    }

    if (not process->waitForFinished(msecs))
    {
        error = QStringLiteral("The finish operation timed out or an error occurred.\n%1\n%2")
                    .arg(parameters, QString(process->readAllStandardError()));
        process->kill();
        return TST_EX_FINISH_TIME_OUT;
    }

    if (process->exitStatus() == QProcess::CrashExit)
    {
        error = QStringLiteral("Program crashed.\n%1\n%2").arg(parameters, QString(process->readAllStandardError()));
        return TST_EX_CRASH;
    }

    if (process->exitCode() != exit)
    {
        error = QStringLiteral("Unexpected finish. Exit code: %1\n%2")
                    .arg(process->exitCode())
                    .arg(QString(process->readAllStandardError()));
        return process->exitCode();
    }

    return process->exitCode();
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::CopyRecursively(const QString &srcFilePath, const QString &tgtFilePath) const -> bool
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir())
    {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        const QString dirName = QFileInfo(tgtFilePath).fileName();
        if (not targetDir.mkdir(dirName))
        {
            const QString msg = QStringLiteral("Can't create dir '%1'.").arg(dirName);
            QWARN(qUtf8Printable(msg));
            return false;
        }
        QDir sourceDir(srcFilePath);
        const QStringList fileNames =
            sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (auto &fileName : fileNames)
        {
            const QString newSrcFilePath = srcFilePath + QDir::separator() + fileName;
            const QString newTgtFilePath = tgtFilePath + QDir::separator() + fileName;
            if (not CopyRecursively(newSrcFilePath, newTgtFilePath))
            {
                return false;
            }
        }
    }
    else
    {
        if (QFileInfo::exists(tgtFilePath))
        {
            const QString msg = QStringLiteral("File '%1' exists.").arg(srcFilePath);
            QWARN(qUtf8Printable(msg));

            if (QFile::remove(tgtFilePath))
            {
                QWARN("File successfully removed.");
            }
            else
            {
                QWARN("Can't remove file.");
                return false;
            }
        }

        // Check error: Cannot open %file for input
        QFile srcFile(srcFilePath);
        if (not srcFile.open(QFile::ReadOnly))
        {
            const QString msg =
                QStringLiteral("Can't copy file '%1'. Error: %2").arg(srcFilePath, srcFile.errorString());
            QWARN(qUtf8Printable(msg));
            return false;
        }
        srcFile.close();

        if (not srcFile.copy(tgtFilePath))
        {
            const QString msg = QStringLiteral("Can't copy file '%1' to '%2'. Error: %3")
                                    .arg(srcFilePath, tgtFilePath, srcFile.errorString());
            QWARN(qUtf8Printable(msg));
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::PrepareDocument(const QString &json, QByteArray &data)
{
    QFile loadFile(json);
    if (not loadFile.open(QIODevice::ReadOnly))
    {
        const QString error = QStringLiteral("Couldn't open json file. %1").arg(loadFile.errorString());
        QFAIL(qUtf8Printable(error));
    }

    data = loadFile.readAll();
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::TestRoot(const QJsonObject &root, const QString &attribute, const QString &file)
{
    if (not root.contains(attribute))
    {
        const QString error = QStringLiteral("Invalid json file '%1'. File doesn't contain root object.").arg(file);
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ReadStringValue(const QJsonObject &itemObject, const QString &attribute, QString &value,
                                   const QString &defaultValue)
{
    if (itemObject.contains(attribute))
    {
        QJsonValue attributeValue = itemObject[attribute];
        if (attributeValue.isString())
        {
            value = attributeValue.toString();
        }
        else
        {
            const QString error = QStringLiteral("%1 is not string '%2'.").arg(attribute, attributeValue.toString());
            QFAIL(qUtf8Printable(error));
        }
    }
    else
    {
        if (not defaultValue.isEmpty())
        {
            value = defaultValue;
        }
        else
        {
            const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
            QFAIL(qUtf8Printable(error));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ReadBooleanValue(const QJsonObject &itemObject, const QString &attribute, bool &value,
                                    const QString &defaultValue)
{
    if (itemObject.contains(attribute))
    {
        QJsonValue attributeValue = itemObject[attribute];
        if (attributeValue.isBool())
        {
            value = attributeValue.toBool();
        }
        else
        {
            const QString error =
                QStringLiteral("%1 is not boolean value '%2'.").arg(attribute, attributeValue.toString());
            QFAIL(qUtf8Printable(error));
        }
    }
    else
    {
        if (not defaultValue.isEmpty())
        {
            bool ok = false;
            int defVal = defaultValue.toInt(&ok);

            if (not ok)
            {
                const QString error = QStringLiteral("Cannot convert default value '%1' to int.").arg(defaultValue);
                QFAIL(qUtf8Printable(error));
            }

            value = static_cast<bool>(defVal);
        }
        else
        {
            const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
            QFAIL(qUtf8Printable(error));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ReadPointValue(const QJsonObject &itemObject, const QString &attribute, VPointF &value)
{
    if (itemObject.contains(attribute))
    {
        PointFromJson(itemObject[attribute].toObject(), value);
    }
    else
    {
        const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ReadSplinePointValues(const QJsonObject &itemObject, const QString &attribute,
                                         QVector<VSplinePoint> &points)
{
    points.clear();
    if (itemObject.contains(attribute))
    {
        QJsonArray nodes = itemObject[attribute].toArray();
        for (int i = 0; i < nodes.size(); ++i)
        {
            QJsonObject item = nodes[i].toObject();
            VSplinePoint point;
            ReadSplinePointValue(item, point);
            points.append(point);
        }
    }
    else
    {
        const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ReadSplinePointValue(const QJsonObject &itemObject, VSplinePoint &point)
{
    qreal angle1 = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("angle1"), angle1);

    QString angle1Formula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("angle1Formula"), angle1Formula);

    qreal angle2 = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("angle2"), angle2);

    QString angle2Formula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("angle2Formula"), angle2Formula);

    qreal length1 = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("length1"), length1);

    QString length1Formula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("length1Formula"), length1Formula);

    qreal length2 = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("length2"), length2);

    QString length2Formula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("length2Formula"), length2Formula);

    VPointF pSpline;
    ReadPointValue(itemObject, QStringLiteral("point"), pSpline);

    point = VSplinePoint(pSpline, angle1, angle1Formula, angle2, angle2Formula, length1, length1Formula, length2,
                         length2Formula);
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::ReadPieceNodeValue(const QJsonObject &itemObject, VPieceNode &node)
{
    vidtype id = NULL_ID;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("id"), id);

    Tool typeTool = Tool::LAST_ONE_DO_NOT_USE;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("type"), typeTool);

    bool reverse = false;
    AbstractTest::ReadBooleanValue(itemObject, QStringLiteral("reverse"), reverse, QChar('0'));

    node = VPieceNode(id, typeTool, reverse);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename std::enable_if<std::is_floating_point<T>::value>::type *>
void AbstractTest::ReadDoubleValue(const QJsonObject &itemObject, const QString &attribute, T &value,
                                   const QString &defaultValue)
{
    if (itemObject.contains(attribute))
    {
        QJsonValue attributeValue = itemObject[attribute];
        if (attributeValue.isDouble())
        {
            value = static_cast<T>(attributeValue.toDouble());
        }
        else
        {
            const QString error = QStringLiteral("%1 is not double '%2'.").arg(attribute, attributeValue.toString());
            QFAIL(qUtf8Printable(error));
        }
    }
    else
    {
        if (not defaultValue.isEmpty())
        {
            bool ok = false;
            value = static_cast<T>(defaultValue.toDouble(&ok));

            if (not ok)
            {
                const QString error = QStringLiteral("Cannot convert default value '%1' to double.").arg(defaultValue);
                QFAIL(qUtf8Printable(error));
            }
        }
        else
        {
            const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
            QFAIL(qUtf8Printable(error));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename std::enable_if<std::is_enum<T>::value>::type *>
void AbstractTest::ReadDoubleValue(const QJsonObject &itemObject, const QString &attribute, T &value,
                                   const QString &defaultValue)
{
    if (itemObject.contains(attribute))
    {
        QJsonValue attributeValue = itemObject[attribute];
        if (attributeValue.isDouble())
        {
            value = static_cast<T>(static_cast<int>(attributeValue.toDouble()));
        }
        else
        {
            const QString error = QStringLiteral("%1 is not double '%2'.").arg(attribute, attributeValue.toString());
            QFAIL(qUtf8Printable(error));
        }
    }
    else
    {
        if (not defaultValue.isEmpty())
        {
            bool ok = false;
            value = static_cast<T>(defaultValue.toInt(&ok));

            if (not ok)
            {
                const QString error = QStringLiteral("Cannot convert default value '%1' to int.").arg(defaultValue);
                QFAIL(qUtf8Printable(error));
            }
        }
        else
        {
            const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
            QFAIL(qUtf8Printable(error));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename std::enable_if<std::is_integral<T>::value>::type *>
void AbstractTest::ReadDoubleValue(const QJsonObject &itemObject, const QString &attribute, T &value,
                                   const QString &defaultValue)
{
    if (itemObject.contains(attribute))
    {
        QJsonValue attributeValue = itemObject[attribute];
        if (attributeValue.isDouble())
        {
            value = static_cast<T>(attributeValue.toDouble());
        }
        else
        {
            const QString error = QStringLiteral("%1 is not double '%2'.").arg(attribute, attributeValue.toString());
            QFAIL(qUtf8Printable(error));
        }
    }
    else
    {
        if (not defaultValue.isEmpty())
        {
            bool ok = false;
            value = static_cast<T>(defaultValue.toInt(&ok));

            if (not ok)
            {
                const QString error = QStringLiteral("Cannot convert default value '%1' to int.").arg(defaultValue);
                QFAIL(qUtf8Printable(error));
            }
        }
        else
        {
            const QString error = QStringLiteral("Json object does not contain attribute '%1'.").arg(attribute);
            QFAIL(qUtf8Printable(error));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto AbstractTest::QLineFromJson(const QJsonObject &itemObject) -> QLineF
{
    QPointF p1;
    QPointF p2;
    PointFromJson(itemObject[QStringLiteral("p1")].toObject(), p1);
    PointFromJson(itemObject[QStringLiteral("p2")].toObject(), p2);
    return {p1, p2};
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::SplineFromJson(const QJsonObject &itemObject, QSharedPointer<VContainer> &data)
{
    vidtype id = NULL_ID;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("id"), id);

    qreal aScale = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("aScale"), aScale);

    qreal angle1 = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("angle1"), angle1);

    QString angle1Formula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("angle1Formula"), angle1Formula);

    qreal angle2 = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("angle2"), angle2);

    QString angle2Formula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("angle2Formula"), angle2Formula);

    qreal c1Length = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("c1Length"), c1Length);

    QString c1LengthFormula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("c1LengthFormula"), c1LengthFormula);

    qreal c2Length = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("c2Length"), c2Length);

    QString c2LengthFormula;
    AbstractTest::ReadStringValue(itemObject, QStringLiteral("c2LengthFormula"), c2LengthFormula);

    VPointF p1;
    ReadPointValue(itemObject, QStringLiteral("p1"), p1);
    data->UpdateGObject(p1.id(), new VPointF(p1));

    VPointF p4;
    ReadPointValue(itemObject, QStringLiteral("p4"), p4);
    data->UpdateGObject(p4.id(), new VPointF(p4));

    VSpline *spl = new VSpline(p1, p4, angle1, angle1Formula, angle2, angle2Formula, c1Length, c1LengthFormula,
                               c2Length, c2LengthFormula);
    spl->SetApproximationScale(aScale);
    data->UpdateGObject(id, spl);
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::SplinePathFromJson(const QJsonObject &itemObject, QSharedPointer<VContainer> &data)
{
    vidtype id = NULL_ID;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("id"), id);

    qreal aScale = 0;
    AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("aScale"), aScale);

    QVector<VSplinePoint> points;
    AbstractTest::ReadSplinePointValues(itemObject, QStringLiteral("nodes"), points);
    for (auto &point : points)
    {
        data->UpdateGObject(point.P().id(), new VPointF(point.P()));
    }

    VSplinePath *path = new VSplinePath(points);
    path->SetApproximationScale(aScale);
    data->UpdateGObject(id, path);
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::DBFromJson(const QJsonObject &dbObject, QSharedPointer<VContainer> &data)
{
    const QString itemsKey = QStringLiteral("items");

    if (dbObject.contains(itemsKey))
    {
        QJsonArray items = dbObject[itemsKey].toArray();
        for (auto item : items)
        {
            QJsonObject itemObject = item.toObject();
            GOType objectType;
            AbstractTest::ReadDoubleValue(itemObject, QStringLiteral("type"), objectType);

            switch (objectType)
            {
                case GOType::Point:
                {
                    VPointF point;
                    PointFromJson(itemObject, point);
                    data->UpdateGObject(point.id(), new VPointF(point));
                    break;
                }
                case GOType::Spline:
                    SplineFromJson(itemObject, data);
                    break;
                case GOType::SplinePath:
                    SplinePathFromJson(itemObject, data);
                    break;
                default:
                {
                    const QString error =
                        QStringLiteral("Not supported item type '%1'.").arg(static_cast<int>(objectType));
                    QFAIL(qUtf8Printable(error));
                }
            }
        }
    }
    else
    {
        const QString error = QStringLiteral("DB json object does not contain items.");
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AbstractTest::MainPathFromJson(const QJsonObject &pieceObject, VPiece &piece)
{
    qreal saWidth = 0;
    AbstractTest::ReadDoubleValue(pieceObject, QStringLiteral("saWidth"), saWidth);

    bool seamAllowance = false;
    AbstractTest::ReadBooleanValue(pieceObject, QStringLiteral("seamAllowance"), seamAllowance);

    piece.SetSeamAllowance(seamAllowance);
    piece.SetSAWidth(saWidth);

    piece.GetPath().Clear();

    const QString nodesKey = QStringLiteral("nodes");

    if (pieceObject.contains(nodesKey))
    {
        QJsonArray nodes = pieceObject[nodesKey].toArray();
        for (int i = 0; i < nodes.size(); ++i)
        {
            QJsonObject itemObject = nodes[i].toObject();

            VPieceNode node;
            ReadPieceNodeValue(itemObject, node);
            piece.GetPath().Append(node);
        }
    }
    else
    {
        const QString error = QStringLiteral("Piece json object does not contain nodes.");
        QFAIL(qUtf8Printable(error));
    }
}
