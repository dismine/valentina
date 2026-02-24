/************************************************************************
 **
 **  @file   abstracttest.h
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

#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtCore/qcontainerfwd.h>
#include <qtestcase.h>
#include <typeinfo>

#include "../vgeometry/vpointf.h"
#include "../vlayout/vrawsapoint.h"
#include "../vlayout/vsapoint.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/typedef.h"

#ifdef __GNUC__
#define V_UNUSED __attribute__((unused))
#else
#define V_UNUSED
#endif

// Return codes for testing run application
static const auto V_UNUSED TST_EX_BIN = -1;             // Can't find binary.
static const auto V_UNUSED TST_EX_FINISH_TIME_OUT = -2; // The operation timed out or an error occurred.
static const auto V_UNUSED TST_EX_START_TIME_OUT = -3;  // The operation timed out or an error occurred.
static const auto V_UNUSED TST_EX_CRASH = -4;           // Program crashed.

#undef V_UNUSED

enum ErrorState
{
    ErrorLoad = 0,
    ErrorInstall,
    ErrorSize,
    ErrorMissing,
    NoError
};

class VPiece;
class VContainer;
class VPointF;
class VSplinePoint;
class VPieceNode;
enum class GOType : qint8;
struct VPiecePassmarkData;
class VRawSAPoint;

class AbstractTest : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit AbstractTest(QObject *parent = nullptr);

    template <class T> static auto VectorFromJson(const QString &json) -> QVector<T>;

    void PieceFromJson(const QString &json, VPiece &piece, QSharedPointer<VContainer> &data);

    void PassmarkDataFromJson(const QString &json, VPiecePassmarkData &data);
    void PassmarkShapeFromJson(const QString &json, QVector<QLineF> &shape);

protected:
    static void ComparePaths(const QVector<QPointF> &actual, const QVector<QPointF> &expected);
    void ComparePathsDistance(const QVector<QPointF> &ekv, const QVector<QPointF> &ekvOrig) const;
    void ComparePointsDistance(const QPointF &result, const QPointF &expected, qreal testAccuracy) const;
    void CompareLinesDistance(const QVector<QLineF> &result, const QVector<QLineF> &expected) const;

    static auto PuzzlePath() -> QString;
    static auto ValentinaPath() -> QString;
    static auto TapePath() -> QString;
    static auto TranslationsPath() -> QString;

    static auto RunTimeout(int defMsecs) -> int;

    auto Run(int exit, const QString &program, const QStringList &arguments, QString &error, int msecs = 120000) -> int;
    auto CopyRecursively(const QString &srcFilePath, const QString &tgtFilePath) const -> bool;

    static void PrepareDocument(const QString &json, QByteArray &data);
    static void TestRoot(const QJsonObject &root, const QString &attribute, const QString &file);

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>> * = nullptr>
    static void ReadDoubleValue(const QJsonObject &itemObject, const QString &attribute, T &value,
                                const QString &defaultValue = QString());
    template <typename T, std::enable_if_t<std::is_enum_v<T>> * = nullptr>
    static void ReadDoubleValue(const QJsonObject &itemObject, const QString &attribute, T &value,
                                const QString &defaultValue = QString());
    template <typename T, std::enable_if_t<std::is_integral_v<T>> * = nullptr>
    static void ReadDoubleValue(const QJsonObject &itemObject, const QString &attribute, T &value,
                                const QString &defaultValue = QString());
    static void ReadStringValue(const QJsonObject &itemObject, const QString &attribute, QString &value,
                                const QString &defaultValue = QString());
    static void ReadBooleanValue(const QJsonObject &itemObject, const QString &attribute, bool &value,
                                 const QString &defaultValue = QString());
    void ReadPointValue(const QJsonObject &itemObject, const QString &attribute, VPointF &value);
    void ReadSplinePointValues(const QJsonObject &itemObject, const QString &attribute, QVector<VSplinePoint> &points);
    void ReadSplinePointValue(const QJsonObject &itemObject, VSplinePoint &point);
    void ReadPieceNodeValue(const QJsonObject &itemObject, VPieceNode &node);

    template <class T> static void CheckClassType(const QJsonObject &itemObject);

    template <class T> static auto ReadPointData(const QJsonObject &pointObject) -> T;

    template<class T>
    static auto ItemFromJson(const QJsonObject &pointObject, T &point) -> void;

    auto QLineFromJson(const QJsonObject &itemObject) -> QLineF;
    void SplineFromJson(const QJsonObject &itemObject, QSharedPointer<VContainer> &data);
    void SplinePathFromJson(const QJsonObject &itemObject, QSharedPointer<VContainer> &data);

    void DBFromJson(const QJsonObject &dbObject, QSharedPointer<VContainer> &data);
    void MainPathFromJson(const QJsonObject &pieceObject, VPiece &piece);
};

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto AbstractTest::VectorFromJson(const QString &json) -> QVector<T>
{
    QByteArray saveData;
    PrepareDocument(json, saveData);
    QJsonDocument const loadDoc(QJsonDocument::fromJson(saveData));

    const auto vectorKey = QStringLiteral("vector");

    QJsonObject vectorObject = loadDoc.object();
    TestRoot(vectorObject, vectorKey, json);

    QJsonArray const vectorArray = vectorObject[vectorKey].toArray();
    QVector<T> vector;
    vector.reserve(vectorArray.size());

    for (auto item : vectorArray)
    {
        try
        {
            T point;
            ItemFromJson(item.toObject(), point);
            vector.append(point);
        }
        catch (const VException &e)
        {
            throw VException(QStringLiteral("Invalid json file '%1'. %2").arg(json, e.ErrorMessage()));
        }
    }

    return vector;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline void AbstractTest::CheckClassType(const QJsonObject &itemObject)
{
    const auto typeKey = QStringLiteral("type");

    QString type;
    AbstractTest::ReadStringValue(itemObject, typeKey, type);

    const QStringList types{
        QStringLiteral("QPointF"),         // 0
        QStringLiteral("VLayoutPoint"),    // 1
        QStringLiteral("VRawSAPoint"),     // 2
        QStringLiteral("VSAPoint"),        // 3
        QStringLiteral("VLayoutPassmark"), // 4
    };

    bool res = false;
    switch (types.indexOf(type))
    {
        case 0:
            res = (typeid(T) == typeid(QPointF));
            break;
        case 1:
            res = (typeid(T) == typeid(VLayoutPoint));
            break;
        case 2:
            res = (typeid(T) == typeid(VRawSAPoint));
            break;
        case 3:
            res = (typeid(T) == typeid(VSAPoint));
            break;
        case 4:
            res = (typeid(T) == typeid(VLayoutPassmark));
            break;
        default:
            break;
    }

    if (not res)
    {
        throw VException(QStringLiteral("Unexpected class '%2'.").arg(itemObject[typeKey].toString()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto AbstractTest::ReadPointData(const QJsonObject &pointObject) -> T
{
    T point;
    qreal x = 0;
    AbstractTest::ReadDoubleValue(pointObject, QChar('x'), x);
    point.setX(x);

    qreal y = 0;
    AbstractTest::ReadDoubleValue(pointObject, QChar('y'), y);
    point.setY(y);

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
inline auto AbstractTest::ItemFromJson(const QJsonObject &pointObject, T &point) -> void
{
    CheckClassType<T>(pointObject);
    point = ReadPointData<T>(pointObject);
}

//---------------------------------------------------------------------------------------------------------------------
template<>
inline auto AbstractTest::ItemFromJson(const QJsonObject &pointObject, VPointF &point) -> void
{
    vidtype id = NULL_ID;
    AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("id"), id);

    qreal mx = 0;
    AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("mx"), mx);

    qreal my = 0;
    AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("my"), my);

    QString name;
    AbstractTest::ReadStringValue(pointObject, QStringLiteral("name"), name);

    qreal x = 0;
    AbstractTest::ReadDoubleValue(pointObject, QChar('x'), x);

    qreal y = 0;
    AbstractTest::ReadDoubleValue(pointObject, QChar('y'), y);

    point = VPointF(x, y, name, mx, my);
    point.setId(id);
}

//---------------------------------------------------------------------------------------------------------------------
template<>
inline auto AbstractTest::ItemFromJson(const QJsonObject &object, VLayoutPassmark &passmark) -> void
{
    // Read "lines" array
    if (object.contains(QStringLiteral("lines")) && object[QStringLiteral("lines")].isArray())
    {
        QJsonArray linesArray = object[QStringLiteral("lines")].toArray();
        QVector<QLineF> lines;

        for (const auto lineValue : linesArray)
        {
            if (lineValue.isObject())
            {
                QJsonObject lineObject = lineValue.toObject();

                qreal x1 = 0;
                AbstractTest::ReadDoubleValue(lineObject, QStringLiteral("x1"), x1);

                qreal y1 = 0;
                AbstractTest::ReadDoubleValue(lineObject, QStringLiteral("y1"), y1);

                qreal x2 = 0;
                AbstractTest::ReadDoubleValue(lineObject, QStringLiteral("x2"), x2);

                qreal y2 = 0;
                AbstractTest::ReadDoubleValue(lineObject, QStringLiteral("y2"), y2);

                lines.append(QLineF(x1, y1, x2, y2));
            }
        }
        passmark.lines = lines;
    }

    // Read "type" if present
    if (object.contains(QStringLiteral("passmarkType")))
    {
        passmark.type = static_cast<PassmarkLineType>(object[QStringLiteral("passmarkType")].toInt());
    }

    // Read "baseLine" if present
    if (object.contains(QStringLiteral("baseLine")) && object[QStringLiteral("baseLine")].isObject())
    {
        QJsonObject baseLineObject = object[QStringLiteral("baseLine")].toObject();

        qreal x1 = 0;
        AbstractTest::ReadDoubleValue(baseLineObject, QStringLiteral("x1"), x1);

        qreal y1 = 0;
        AbstractTest::ReadDoubleValue(baseLineObject, QStringLiteral("y1"), y1);

        qreal x2 = 0;
        AbstractTest::ReadDoubleValue(baseLineObject, QStringLiteral("x2"), x2);

        qreal y2 = 0;
        AbstractTest::ReadDoubleValue(baseLineObject, QStringLiteral("y2"), y2);

        passmark.baseLine = QLineF(x1, y1, x2, y2);
    }

    // Read "isBuiltIn" if present
    if (object.contains(QStringLiteral("isBuiltIn")))
    {
        ReadBooleanValue(object, QStringLiteral("isBuiltIn"), passmark.isBuiltIn);
    }

    // Read "isClockwiseOpening" if present
    if (object.contains(QStringLiteral("isClockwiseOpening")))
    {
        ReadBooleanValue(object, QStringLiteral("isClockwiseOpening"), passmark.isClockwiseOpening);
    }

    // Read "label" if present
    if (object.contains(QStringLiteral("label")))
    {
        AbstractTest::ReadStringValue(object, QStringLiteral("label"), passmark.label);
    }

    // Read "notMirrored" if present
    if (object.contains(QStringLiteral("notMirrored")))
    {
        ReadBooleanValue(object, QStringLiteral("notMirrored"), passmark.notMirrored);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <> inline auto AbstractTest::ReadPointData(const QJsonObject &pointObject) -> VLayoutPoint
{
    VLayoutPoint point(ReadPointData<QPointF>(pointObject));

    bool turnPoint;
    AbstractTest::ReadBooleanValue(pointObject, QStringLiteral("turnPoint"), turnPoint, QStringLiteral("0"));
    point.SetTurnPoint(turnPoint);

    bool curvePoint;
    AbstractTest::ReadBooleanValue(pointObject, QStringLiteral("curvePoint"), curvePoint, QStringLiteral("0"));
    point.SetCurvePoint(curvePoint);

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
template<>
inline auto AbstractTest::ItemFromJson(const QJsonObject &pointObject, VLayoutPoint &point) -> void
{
    CheckClassType<VLayoutPoint>(pointObject);
    point = ReadPointData<VLayoutPoint>(pointObject);
}

//---------------------------------------------------------------------------------------------------------------------
template<>
inline auto AbstractTest::ItemFromJson(const QJsonObject &pointObject, VSAPoint &point) -> void
{
    CheckClassType<VSAPoint>(pointObject);

    point = VSAPoint(ReadPointData<VLayoutPoint>(pointObject));

    qreal saBefore;
    AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("saBefore"), saBefore, QStringLiteral("-1"));
    point.SetSABefore(saBefore);

    qreal saAfter;
    AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("saAfter"), saAfter, QStringLiteral("-1"));
    point.SetSAAfter(saAfter);

    PieceNodeAngle angleType = PieceNodeAngle::ByLength;
    AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("angle"), angleType,
                                  QString::number(static_cast<int>(PieceNodeAngle::ByLength)));
    point.SetAngleType(angleType);

    bool manualPassmarkLength = false;
    AbstractTest::ReadBooleanValue(pointObject, QLatin1String("manualPassmarkLength"), manualPassmarkLength,
                                   QStringLiteral("0"));
    point.SetManualPasskmarkLength(manualPassmarkLength);

    if (manualPassmarkLength)
    {
        qreal passmarkLength = 0;
        AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("passmarkLength"), passmarkLength,
                                      QStringLiteral("0"));
        point.SetPasskmarkLength(passmarkLength);
    }

    bool manualPassmarkWidth = false;
    AbstractTest::ReadBooleanValue(pointObject, QLatin1String("manualPassmarkWidth"), manualPassmarkWidth,
                                   QStringLiteral("0"));
    point.SetManualPasskmarkWidth(manualPassmarkWidth);

    if (manualPassmarkWidth)
    {
        qreal passmarkWidth = 0;
        AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("passmarkWidth"), passmarkWidth, QStringLiteral("0"));
        point.SetPasskmarkWidth(passmarkWidth);
    }
    else
    {
        bool passmarkClockwiseOpening = false;
        AbstractTest::ReadBooleanValue(pointObject, QLatin1String("passmarkClockwiseOpening"), passmarkClockwiseOpening,
                                       QStringLiteral("0"));
        point.SetPassmarkClockwiseOpening(passmarkClockwiseOpening);
    }

    bool manualPassmarkAngle = false;
    AbstractTest::ReadBooleanValue(pointObject, QLatin1String("manualPassmarkAngle"), manualPassmarkAngle,
                                   QStringLiteral("0"));
    point.SetManualPasskmarkAngle(manualPassmarkAngle);

    bool customSA = false;
    AbstractTest::ReadBooleanValue(pointObject, QLatin1String("customSA"), customSA, QStringLiteral("0"));
    point.SetCustomSA(customSA);

    if (manualPassmarkAngle)
    {
        qreal passmarkAngle = 0;
        AbstractTest::ReadDoubleValue(pointObject, QStringLiteral("passmarkAngle"), passmarkAngle, QStringLiteral("0"));
        point.SetPasskmarkAngle(passmarkAngle);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<>
inline auto AbstractTest::ItemFromJson(const QJsonObject &pointObject, VRawSAPoint &point) -> void
{
    CheckClassType<VRawSAPoint>(pointObject);

    point = VRawSAPoint(ReadPointData<VLayoutPoint>(pointObject));

    bool loopPoint;
    AbstractTest::ReadBooleanValue(pointObject, QStringLiteral("loopPoint"), loopPoint, QStringLiteral("0"));
    point.SetLoopPoint(loopPoint);
}

#endif // ABSTRACTTEST_H
