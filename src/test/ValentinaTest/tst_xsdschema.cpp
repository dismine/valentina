/************************************************************************
 **
 **  @file   tst_xsdschema.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 2, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "tst_xsdschema.h"

#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vlabeltemplateconverter.h"
#include "../ifc/xml/vlayoutconverter.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../ifc/xml/vwatermarkconverter.h"
#include "../ifc/xml/vparsererrorhandler.h"

#include <QTest>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <QMap>
#else
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void ValidateSchema(const QString &schema)
{
    QFile fileSchema(schema);
    if (not fileSchema.open(QIODevice::ReadOnly))
    {
        QFAIL(qUtf8Printable(QStringLiteral("Can't open file %1:\n%2.").arg(schema, fileSchema.errorString())));
    }

    VParserErrorHandler parserErrorHandler;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QScopedPointer<QTemporaryFile> tempSchema(QTemporaryFile::createNativeFile(fileSchema));
    if (tempSchema == nullptr)
    {
        QFAIL(qUtf8Printable(QStringLiteral("Can't create native file for schema file %1:\n%2.")
                                 .arg(schema, fileSchema.errorString())));
    }

    if (tempSchema->open())
    {
        XERCES_CPP_NAMESPACE::XercesDOMParser domParser;
        domParser.setErrorHandler(&parserErrorHandler);

        if (domParser.loadGrammar(
                tempSchema->fileName().toUtf8().constData(),
                XERCES_CPP_NAMESPACE::Grammar::SchemaGrammarType, true) == nullptr)
        {
            QFAIL(qUtf8Printable(QStringLiteral("%1 Could not load schema file '%2'.")
                                     .arg(parserErrorHandler.StatusMessage(), fileSchema.fileName())));
        }

        QVERIFY2(not parserErrorHandler.HasError(),
                 qUtf8Printable(QStringLiteral("%1 Schema file %2 invalid in line %3 column %4.")
                                    .arg(parserErrorHandler.StatusMessage(), fileSchema.fileName())
                                    .arg(parserErrorHandler.Line())
                                    .arg(parserErrorHandler.Column())));
    }
    else
    {
        QFAIL("Unable to open native file for schema");
    }
#else
    QXmlSchema sch;
    sch.setMessageHandler(&parserErrorHandler);
    if (not sch.load(&fileSchema, QUrl::fromLocalFile(fileSchema.fileName())))
    {
        QFAIL(qUtf8Printable(QStringLiteral("%1 Could not load schema file '%2'.")
                                 .arg(parserErrorHandler.StatusMessage(), fileSchema.fileName())));
    }

    QVERIFY2(sch.isValid(), qUtf8Printable(QStringLiteral("%1 Validation error file %2 in line %3 column %4.")
                                               .arg(parserErrorHandler.StatusMessage(), fileSchema.fileName())
                                               .arg(parserErrorHandler.Line())
                                               .arg(parserErrorHandler.Column())));
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto HashToMap(const QHash<unsigned int, QString> &hash) -> QMap<unsigned int, QString>
{
    QMap<unsigned int, QString> map;

    auto i = hash.constBegin();
    while (i != hash.constEnd())
    {
        map.insert(i.key(), i.value());
        ++i;
    }

    return map;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_XSDShema::TST_XSDShema(QObject *parent)
    :QObject(parent)
{}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestPatternSchema_data()
{
    QTest::addColumn<QString>("schema");

    const QMap<unsigned int, QString> schemas = HashToMap(VPatternConverter::XSDSchemas());

    auto i = schemas.constBegin();
    while (i != schemas.constEnd())
    {
        QFileInfo f(i.value());
        QTest::newRow(qUtf8Printable(f.fileName())) << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestPatternSchema()
{
    QFETCH(QString, schema);

    ValidateSchema(schema);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestPatternLabelSchema_data()
{
    QTest::addColumn<QString>("schema");

    const QMap<unsigned int, QString> schemas = HashToMap(VLabelTemplateConverter::XSDSchemas());

    auto i = schemas.constBegin();
    while (i != schemas.constEnd())
    {
        QFileInfo f(i.value());
        QTest::newRow(qUtf8Printable(f.fileName())) << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestPatternLabelSchema()
{
    QFETCH(QString, schema);

    ValidateSchema(schema);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestLayoutSchema_data()
{
    QTest::addColumn<QString>("schema");

    const QMap<unsigned int, QString> schemas = HashToMap(VLayoutConverter::XSDSchemas());

    auto i = schemas.constBegin();
    while (i != schemas.constEnd())
    {
        QFileInfo f(i.value());
        QTest::newRow(qUtf8Printable(f.fileName())) << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestLayoutSchema()
{
    QFETCH(QString, schema);

    ValidateSchema(schema);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestVITSchema_data()
{
    QTest::addColumn<QString>("schema");

    const QMap<unsigned int, QString> schemas = HashToMap(VVITConverter::XSDSchemas());

    auto i = schemas.constBegin();
    while (i != schemas.constEnd())
    {
        QFileInfo f(i.value());
        QTest::newRow(qUtf8Printable(f.fileName())) << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestVITSchema()
{
    QFETCH(QString, schema);

    ValidateSchema(schema);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestVSTSchema_data()
{
    QTest::addColumn<QString>("schema");

    const QMap<unsigned int, QString> schemas = HashToMap(VVSTConverter::XSDSchemas());

    auto i = schemas.constBegin();
    while (i != schemas.constEnd())
    {
        QFileInfo f(i.value());
        QTest::newRow(qUtf8Printable(f.fileName())) << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestVSTSchema()
{
    QFETCH(QString, schema);

    ValidateSchema(schema);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestWatermarkSchema_data()
{
    QTest::addColumn<QString>("schema");

    const QMap<unsigned int, QString> schemas = HashToMap(VWatermarkConverter::XSDSchemas());

    auto i = schemas.constBegin();
    while (i != schemas.constEnd())
    {
        QFileInfo f(i.value());
        QTest::newRow(qUtf8Printable(f.fileName())) << i.value();
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_XSDShema::TestWatermarkSchema()
{
    QFETCH(QString, schema);

    ValidateSchema(schema);
}
