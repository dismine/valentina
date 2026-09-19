/************************************************************************
 **
 **  @file   tst_misc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 10, 2015
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

#include "tst_misc.h"
#include "../ifc/xml/vbackgroundpatternimage.h"
#include "../vmisc/def.h"
#include "../vgeometry/vgobject.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_Misc::TST_Misc(QObject *parent)
    :QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestRelativeFilePath_data()
{
    QTest::addColumn<QString>("patternPath");
    QTest::addColumn<QString>("absoluteMPath");
    QTest::addColumn<QString>("output");

    QTest::newRow("Measurements one level above")
            << "/home/user/patterns/pattern.val" << "/home/user/measurements/m.vit" << "../measurements/m.vit";

    QTest::newRow("Measurements one level under")
            << "/home/user/patterns/pattern.val" << "/home/user/patterns/measurements/m.vit" << "measurements/m.vit";

    QTest::newRow("Measurements in the same folder")
            << "/home/user/patterns/pattern.val" << "/home/user/patterns/m.vit" << "m.vit";

    QTest::newRow("Path to measurements is empty")
            << "/home/user/patterns/pattern.val" << QString() << QString();

    QTest::newRow("Path to a pattern file is empty. Ablosute measurements path.")
            << QString() << "/home/user/patterns/m.vit" << "/home/user/patterns/m.vit";

    QTest::newRow("Path to a pattern file is empty. Relative measurements path.")
            << QString() << "measurements/m.vit" << "measurements/m.vit";

    QTest::newRow("Relative measurements path.")
            << "/home/user/patterns/pattern.val" << "../measurements/m.vit" << "../measurements/m.vit";

    QTest::newRow("Both paths are empty") << QString() << QString() << QString();

    QTest::newRow("Path to measurements is relative")
            << "/home/user/patterns/pattern.val" << "m.vit" << "m.vit";

    QTest::newRow("Absolute pattern path.") << "/home/user/patterns" << "m.vit" << "m.vit";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestRelativeFilePath()
{
    QFETCH(QString, patternPath);
    QFETCH(QString, absoluteMPath);
    QFETCH(QString, output);

    const QString result = RelativeMPath(patternPath, absoluteMPath);
    QCOMPARE(output, result);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestAbsoluteFilePath_data()
{
    QTest::addColumn<QString>("patternPath");
    QTest::addColumn<QString>("relativeMPath");
    QTest::addColumn<QString>("output");

    #ifdef Q_OS_WIN
    QTest::newRow("Measurements one level above")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << "../measurements/m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/measurements/m.vit");
    QTest::newRow("Measurements one level above")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << "../measurements/m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/measurements/m.vit");

    QTest::newRow("Measurements one level under")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << "measurements/m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/measurements/m.vit");

    QTest::newRow("Measurements in the same folder")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << "m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/m.vit");

    QTest::newRow("Path to measurements is empty")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << QString() << QString();

    QTest::newRow("Path to a pattern file is empty. Ablosute measurements path.")
            << QString()
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/m.vit")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/m.vit");

    QTest::newRow("Path to a pattern file is empty. Relative measurements path.")
            << QString()
            << "measurements/m.vit"
            << "measurements/m.vit";

    QTest::newRow("Relative measurements path.")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << "../measurements/m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/measurements/m.vit");

    QTest::newRow("Both paths are empty") << QString() << QString() << QString();

    QTest::newRow("Path to measurements is relative")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/pattern.val")
            << "m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns/m.vit");

    QTest::newRow("Absolute pattern path.")
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/patterns")
            << "m.vit"
            << QCoreApplication::applicationDirPath() + QStringLiteral("/home/user/m.vit");
#else
    QTest::newRow("Measurements one level above")
            << "/home/user/patterns/pattern.val" << "../measurements/m.vit" << "/home/user/measurements/m.vit";

    QTest::newRow("Measurements one level under")
            << "/home/user/patterns/pattern.val" << "measurements/m.vit" << "/home/user/patterns/measurements/m.vit";

    QTest::newRow("Measurements in the same folder")
            << "/home/user/patterns/pattern.val" << "m.vit" << "/home/user/patterns/m.vit";

    QTest::newRow("Path to measurements is empty")
            << "/home/user/patterns/pattern.val" << QString() << QString();

    QTest::newRow("Path to a pattern file is empty. Ablosute measurements path.")
            << QString() << "/home/user/patterns/m.vit" << "/home/user/patterns/m.vit";

    QTest::newRow("Path to a pattern file is empty. Relative measurements path.")
            << QString() << "measurements/m.vit" << "measurements/m.vit";

    QTest::newRow("Relative measurements path.")
            << "/home/user/patterns/pattern.val" << "../measurements/m.vit" << "/home/user/measurements/m.vit";

    QTest::newRow("Both paths are empty") << QString() << QString() << QString();

    QTest::newRow("Path to measurements is relative")
            << "/home/user/patterns/pattern.val" << "m.vit" << "/home/user/patterns/m.vit";

    QTest::newRow("Absolute pattern path.") << "/home/user/patterns" << "m.vit" << "/home/user/m.vit";
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestAbsoluteFilePath()
{
    QFETCH(QString, patternPath);
    QFETCH(QString, relativeMPath);
    QFETCH(QString, output);

    const QString result = AbsoluteMPath(patternPath, relativeMPath);
    QCOMPARE(output, result);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestCLocale_data()
{
    QTest::addColumn<qreal>("number");
    QTest::addColumn<QString>("expected");

    QTest::newRow("10000") << 10000.0 << "10000";
    QTest::newRow("10000.5") << 10000.5 << "10000.5";
}

//---------------------------------------------------------------------------------------------------------------------
// Need for testing thousand separator in the C locale.
// Better be sure that the C locale have not thousand separator
void TST_Misc::TestCLocale()
{
    QFETCH(qreal, number);
    QFETCH(QString, expected);

    const QString localized = QString::number(number);

    QCOMPARE(localized, expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestIssue485()
{
    const qreal radius = 5.6692913385826778;
    const QPointF cPoint(407.9527559055118, 39.999874015748034);
    const QPointF sPoint(407.9527559055118, 39.999874015748034);

    QPointF p1, p2;
    const int res = VGObject::LineIntersectCircle(QPointF(), radius, QLineF(QPointF(), sPoint-cPoint), p1, p2);
    QCOMPARE(res, 0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Misc::TestOversizedSvgBackgroundImage()
{
    const QByteArray normalSvg = QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\"><rect width=\"100\" "
        "height=\"100\"/></svg>");

    // TEMP DIAGNOSTIC: investigating CI-only failure on macOS arm Qt 6.7.3, see run 35438615714.
    // qWarning() is swallowed by the "-silent" flag the CI test runner uses, so route the findings
    // through a QVERIFY2 failure message instead, which survives -silent.
    // Remove once root cause of QMimeDatabase::mimeTypeForData() behavior is confirmed.
    {
        const QMimeType svgMime = QMimeDatabase().mimeTypeForData(normalSvg);
        const QByteArray pngMagic = QByteArray::fromHex("89504e470d0a1a0a0000000d49484452");
        const QMimeType pngMime = QMimeDatabase().mimeTypeForData(pngMagic);
        const QMimeType octetMime = QMimeDatabase().mimeTypeForName(QStringLiteral("application/octet-stream"));

        const QString diag = QStringLiteral("svg name=%1 aliases=%2 valid=%3 | png-magic name=%4 | "
                                             "octet aliases=%5 | allMimeTypesCount=%6")
                                 .arg(svgMime.name(), svgMime.aliases().join(QLatin1Char(',')))
                                 .arg(svgMime.isValid())
                                 .arg(pngMime.name(), octetMime.aliases().join(QLatin1Char(',')))
                                 .arg(QMimeDatabase().allMimeTypes().size());
        QVERIFY2(svgMime.name() == QStringLiteral("image/svg+xml"), qUtf8Printable(diag));
    }

    VBackgroundPatternImage normalImage;
    normalImage.SetContentData(normalSvg.toBase64(), QStringLiteral("image/svg+xml"));
    QVERIFY(normalImage.IsValid());

    // Same document, but with a canvas far beyond anything a real pattern background needs. Regression test for
    // a crash where such a declared size sailed through validation and later crashed Qt's paint engine when the
    // item's device-coordinate pixmap cache tried to allocate a buffer that size.
    const QByteArray oversizedSvg = QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100000\" height=\"100000\"><rect width=\"100000\" "
        "height=\"100000\"/></svg>");

    VBackgroundPatternImage oversizedImage;
    oversizedImage.SetContentData(oversizedSvg.toBase64(), QStringLiteral("image/svg+xml"));

    QVERIFY(not oversizedImage.IsValid());
    QCOMPARE(oversizedImage.ErrorString(),
             QStringLiteral("The image declares a canvas size that is too large to render safely."));
}
