/************************************************************************
 **
 **  @file   vabstractconverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 12, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vabstractconverter.h"
#include "vparsererrorhandler.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#else
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

#include <QDir>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QFileInfo>
#include <QLatin1String>
#include <QMap>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <QTextDocument>
#include <QUrl>

#include "../exception/vexception.h"
#include "vdomdocument.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VAbstractConverter::VAbstractConverter(const QString &fileName)
  : m_ver(0x0),
    m_originalFileName(fileName),
    m_convertedFileName(fileName)
{
    setXMLContent(m_convertedFileName); // Throw an exception on error
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractConverter::Convert() -> QString
{
    if (m_ver == MaxVer())
    {
        return m_convertedFileName;
    }

    if (not IsReadOnly())
    {
        ReserveFile();
    }

    if (m_tmpFile.open())
    {
        m_convertedFileName = m_tmpFile.fileName();
    }
    else
    {
        throw VException(tr("Error opening a temp file: %1.").arg(m_tmpFile.errorString()));
    }

    m_ver < MaxVer() ? ApplyPatches() : DowngradeToCurrentMaxVersion();

    return m_convertedFileName;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractConverter::GetCurrentFormatVersion() const -> unsigned
{
    return m_ver;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::ReserveFile() const
{
    // It's not possible in all cases make conversion without lose data.
    // For such cases we will store old version in a reserve file.
    QString error;
    QFileInfo info(m_convertedFileName);
    const QString reserveFileName = u"%1/%2(v%3).%4.bak"_s.arg(info.absoluteDir().absolutePath(), info.baseName(),
                                                               GetFormatVersionStr(), info.completeSuffix());
    if (not SafeCopy(m_convertedFileName, reserveFileName, error))
    {
        // #ifdef Q_OS_WIN32
        //         qt_ntfs_permission_lookup++; // turn checking on
        // #endif /*Q_OS_WIN32*/
        const bool isFileWritable = info.isWritable();
        // #ifdef Q_OS_WIN32
        //         qt_ntfs_permission_lookup--; // turn it off again
        // #endif /*Q_OS_WIN32*/
        if (not IsReadOnly() && isFileWritable)
        {
            const QString errorMsg(tr("Error creating a reserv copy: %1.").arg(error));
            throw VException(errorMsg);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::Replace(QString &formula, const QString &newName, vsizetype position, const QString &token,
                                 vsizetype &bias) const
{
    formula.replace(position, token.length(), newName);
    bias = token.length() - newName.length();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::CorrectionsPositions(vsizetype position, vsizetype bias,
                                              QMap<vsizetype, QString> &tokens) const
{
    if (bias == 0)
    {
        return; // Nothing to correct;
    }

    BiasTokens(position, bias, tokens);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::BiasTokens(vsizetype position, vsizetype bias, QMap<vsizetype, QString> &tokens)
{
    QMap<vsizetype, QString> newTokens;
    QMap<vsizetype, QString>::const_iterator i = tokens.constBegin();
    while (i != tokens.constEnd())
    {
        if (i.key() <= position)
        { // Tokens before position "position" did not change his positions.
            newTokens.insert(i.key(), i.value());
        }
        else
        {
            newTokens.insert(i.key() - bias, i.value());
        }
        ++i;
    }
    tokens = newTokens;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ValidateXML validate xml file by xsd schema.
 * @param schema path to schema file.
 */
void VAbstractConverter::ValidateXML(const QString &schema) const
{
    qCDebug(vXML, "Validation xml file %s.", qUtf8Printable(m_convertedFileName));

    QFile fileSchema(schema);
    if (not fileSchema.open(QIODevice::ReadOnly))
    {
        const QString errorMsg(tr("Can't open schema file %1:\n%2.").arg(schema, fileSchema.errorString()));
        throw VException(errorMsg);
    }

    VParserErrorHandler parserErrorHandler;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    XERCES_CPP_NAMESPACE::XercesDOMParser domParser;
    domParser.setErrorHandler(&parserErrorHandler);

    QByteArray data = fileSchema.readAll();
    const char *schemaData = data.constData();

    QScopedPointer<XERCES_CPP_NAMESPACE::InputSource> grammarSource(new XERCES_CPP_NAMESPACE::MemBufInputSource(
        reinterpret_cast<const XMLByte *>(schemaData), strlen(schemaData), "schema"));

    if (domParser.loadGrammar(*grammarSource, XERCES_CPP_NAMESPACE::Grammar::SchemaGrammarType, true) == nullptr)
    {
        VException e(parserErrorHandler.StatusMessage());
        e.AddMoreInformation(tr("Could not load schema file '%1'.").arg(fileSchema.fileName()));
        throw e;
    }

    qCDebug(vXML, "Schema loaded.");

    if (parserErrorHandler.HasError())
    {
        VException e(parserErrorHandler.StatusMessage());
        e.AddMoreInformation(tr("Schema file %3 invalid in line %1 column %2")
                                 .arg(parserErrorHandler.Line())
                                 .arg(parserErrorHandler.Column())
                                 .arg(fileSchema.fileName()));
        throw e;
    }

    domParser.setValidationScheme(XERCES_CPP_NAMESPACE::XercesDOMParser::Val_Always);
    domParser.setDoNamespaces(true);
    domParser.setDoSchema(true);
    domParser.setValidationConstraintFatal(true);
    domParser.setValidationSchemaFullChecking(true);
    domParser.useCachedGrammarInParse(true);

    QFile pattern(m_convertedFileName);
    if (not pattern.open(QIODevice::ReadOnly))
    {
        const QString errorMsg(tr("Can't open file %1:\n%2.").arg(m_convertedFileName, pattern.errorString()));
        throw VException(errorMsg);
    }

    QByteArray patternFileData = pattern.readAll();
    const char *patternData = patternFileData.constData();

    QScopedPointer<XERCES_CPP_NAMESPACE::InputSource> patternSource(new XERCES_CPP_NAMESPACE::MemBufInputSource(
        reinterpret_cast<const XMLByte *>(patternData), strlen(patternData), "pattern"));

    domParser.parse(*patternSource);

    if (domParser.getErrorCount() > 0)
    {
        VException e(parserErrorHandler.StatusMessage());
        e.AddMoreInformation(tr("Validation error file %3 in line %1 column %2")
                                 .arg(parserErrorHandler.Line())
                                 .arg(parserErrorHandler.Column())
                                 .arg(m_originalFileName));
        throw e;
    }
#else
    QFile pattern(m_convertedFileName);
    if (not pattern.open(QIODevice::ReadOnly))
    {
        const QString errorMsg(tr("Can't open file %1:\n%2.").arg(m_convertedFileName, pattern.errorString()));
        throw VException(errorMsg);
    }

    QXmlSchema sch;
    sch.setMessageHandler(&parserErrorHandler);
    if (sch.load(&fileSchema, QUrl::fromLocalFile(fileSchema.fileName())) == false)
    {
        VException e(parserErrorHandler.StatusMessage());
        e.AddMoreInformation(tr("Could not load schema file '%1'.").arg(fileSchema.fileName()));
        throw e;
    }
    qCDebug(vXML, "Schema loaded.");

    bool errorOccurred = false;
    if (sch.isValid() == false)
    {
        errorOccurred = true;
    }
    else
    {
        QXmlSchemaValidator validator(sch);
        if (validator.validate(&pattern, QUrl::fromLocalFile(pattern.fileName())) == false)
        {
            errorOccurred = true;
        }
    }

    if (errorOccurred)
    {
        VException e(parserErrorHandler.StatusMessage());
        e.AddMoreInformation(tr("Validation error file %3 in line %1 column %2")
                                 .arg(parserErrorHandler.Line())
                                 .arg(parserErrorHandler.Column())
                                 .arg(m_originalFileName));
        throw e;
    }
#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
}

//---------------------------------------------------------------------------------------------------------------------
Q_NORETURN void VAbstractConverter::InvalidVersion(unsigned ver) const
{
    if (ver < MinVer())
    {
        const QString errorMsg(tr("Invalid version. Minimum supported format version is %1").arg(MinVerStr()));
        throw VException(errorMsg);
    }

    if (ver > MaxVer())
    {
        const QString errorMsg(tr("Invalid version. Maximum supported format version is %1").arg(MaxVerStr()));
        throw VException(errorMsg);
    }

    const QString errorMsg(tr("Unexpected version \"%1\".").arg(ver, 0, 16));
    throw VException(errorMsg);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::ValidateInputFile(const QString &currentSchema) const
{
    QString schema;
    try
    {
        schema = XSDSchema(m_ver);
    }
    catch (const VException &e)
    {
        if (m_ver < MinVer())
        { // Version less than minimally supported version. Can't do anything.
            throw;
        }

        if (m_ver > MaxVer())
        { // Version bigger than maximum supported version. We still have a chance to open the file.
            try
            { // Try to open like the current version.
                ValidateXML(currentSchema);
            }
            catch (const VException &exp)
            { // Nope, we can't.
                Q_UNUSED(exp)
                throw e;
            }
        }
        else
        { // Unexpected version. Most time mean that we do not catch all versions between min and max.
            throw;
        }

        return; // All is fine and we can try to convert to current max version.
    }

    ValidateXML(schema);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::Save()
{
    m_tmpFile.resize(0); // clear previous content
    const int indent = 4;
    QTextStream out(&m_tmpFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    save(out, indent);

    if (not m_tmpFile.flush())
    {
        throw VException(m_tmpFile.errorString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::SetVersion(const QString &version)
{
    ValidateVersion(version);

    if (setTagText(TagVersion, version) == false)
    {
        VException e(tr("Could not change version."));
        throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractConverter::XSDSchema(unsigned int ver) const -> QString
{
    const QHash<unsigned, QString> schemas = Schemas();
    if (schemas.contains(ver))
    {
        return schemas.value(ver);
    }

    InvalidVersion(ver);
}
