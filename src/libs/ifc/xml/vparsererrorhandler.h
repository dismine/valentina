/************************************************************************
 **
 **  @file   vparsererrorhandler.h
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
#ifndef VPARSERERRORHANDLER_H
#define VPARSERERRORHANDLER_H

#include <QString>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractMessageHandler>
#include <QSourceLocation>

//This class need for validation pattern file using XSD shema
class VParserErrorHandler : public QAbstractMessageHandler
{
public:
    VParserErrorHandler() =default;

    auto StatusMessage() const -> QString;
    auto Line() const -> qint64;
    auto Column() const -> qint64;

    auto HasError() const -> bool;
protected:
    // cppcheck-suppress unusedFunction
    void handleMessage(QtMsgType type, const QString &description,
                       const QUrl &identifier, const QSourceLocation &sourceLocation) override;
private:
    QtMsgType       m_messageType{};
    QString         m_description{};
    QSourceLocation m_sourceLocation{};
    bool            m_hasError{false};
};

#else

#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

class VParserErrorHandler : public XERCES_CPP_NAMESPACE::ErrorHandler
{
public:
    auto StatusMessage() const -> QString;
    auto Line() const -> XMLFileLoc;
    auto Column() const -> XMLFileLoc;

    void warning(const XERCES_CPP_NAMESPACE::SAXParseException& ex) override;
    void error(const XERCES_CPP_NAMESPACE::SAXParseException& ex) override;
    void fatalError(const XERCES_CPP_NAMESPACE::SAXParseException& ex) override;
    void resetErrors() override;

    auto HasError() const -> bool;

private:
    XMLFileLoc m_line{0};
    XMLFileLoc m_column{0};
    QString    m_description{};
    bool       m_hasError{false};

    void handleMessage(const XERCES_CPP_NAMESPACE::SAXParseException& ex);
};

#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

#endif // VPARSERERRORHANDLER_H
