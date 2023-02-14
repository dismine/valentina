/************************************************************************
 **
 **  @file   vparsererrorhandler.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 2, 2023
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
#include "vparsererrorhandler.h"

#include <QTextDocument>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::StatusMessage() const -> QString
{
    QTextDocument doc;
    doc.setHtml(m_description);
    return doc.toPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::Line() const -> qint64
{
    return m_sourceLocation.line();
}

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::Column() const -> qint64
{
    return m_sourceLocation.column();
}

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::HasError() const -> bool
{
    return m_hasError;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VParserErrorHandler::handleMessage(QtMsgType type, const QString &description, const QUrl &identifier,
                                        const QSourceLocation &sourceLocation)
{
    Q_UNUSED(type)
    Q_UNUSED(identifier)

    m_messageType = type;
    m_description = description;
    m_sourceLocation = sourceLocation;
    m_hasError = true;
}

#else

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::StatusMessage() const -> QString
{
    return m_description;
}

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::Line() const -> XMLFileLoc
{
    return m_line;
}

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::Column() const -> XMLFileLoc
{
    return m_column;
}

//---------------------------------------------------------------------------------------------------------------------
void VParserErrorHandler::handleMessage(const SAXParseException &ex)
{
    char* msg = XMLString::transcode(ex.getMessage());
    m_description = QString(msg);
    m_line = ex.getLineNumber();
    m_column = ex.getColumnNumber();
    m_hasError = true;
    XMLString::release(&msg);
}

//---------------------------------------------------------------------------------------------------------------------
void VParserErrorHandler::warning(const SAXParseException &ex)
{
    handleMessage(ex);
}

//---------------------------------------------------------------------------------------------------------------------
void VParserErrorHandler::error(const SAXParseException &ex)
{
    handleMessage(ex);
}

//---------------------------------------------------------------------------------------------------------------------
void VParserErrorHandler::fatalError(const SAXParseException &ex)
{
    handleMessage(ex);
}

//---------------------------------------------------------------------------------------------------------------------
void VParserErrorHandler::resetErrors()
{
    m_description.clear();
    m_line = 0;
    m_column = 0;
    m_hasError = false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VParserErrorHandler::HasError() const -> bool
{
    return m_hasError;
}

#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
