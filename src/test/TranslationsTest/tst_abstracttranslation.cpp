/************************************************************************
 **
 **  @file   tst_abstracttranslation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 2, 2018
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2018 Valentina project
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
#include "tst_abstracttranslation.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString TST_AbstractTranslation::TagName = QStringLiteral("name");
const QString TST_AbstractTranslation::TagMessage = QStringLiteral("message");
const QString TST_AbstractTranslation::TagSource = QStringLiteral("source");
const QString TST_AbstractTranslation::TagTranslation = QStringLiteral("translation");

const QString TST_AbstractTranslation::AttrType = QStringLiteral("type");
const QString TST_AbstractTranslation::AttrValVanished = QStringLiteral("vanished");
const QString TST_AbstractTranslation::AttrValUnfinished = QStringLiteral("unfinished");
const QString TST_AbstractTranslation::AttrValObsolete = QStringLiteral("obsolete");

//---------------------------------------------------------------------------------------------------------------------
TST_AbstractTranslation::TST_AbstractTranslation(QObject *parent)
  : QObject(parent),
    tsFile(),
    tsXML()
{
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_AbstractTranslation::LoadTSFile(const QString &filename) -> QDomNodeList
{
    tsFile.reset();
    tsFile = QSharedPointer<QFile>(new QFile(u"%1/%2"_s.arg(TS_DIR, filename)));
    if (not tsFile->exists())
    {
        qWarning("Can't find '%s'.\n%s.", qUtf8Printable(filename), qUtf8Printable(tsFile->errorString()));
        return {};
    }

    if (tsFile->open(QIODevice::ReadOnly) == false)
    {
        qWarning("Can't open file '%s'.\n%s.", qUtf8Printable(filename), qUtf8Printable(tsFile->errorString()));
        return {};
    }

    tsXML.reset();
    tsXML = QSharedPointer<QDomDocument>(new QDomDocument());

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (const QDomDocument::ParseResult result = tsXML->setContent(tsFile.data()); !result)
    {
        qWarning("Parsing error file %s in line %d column %d.", qUtf8Printable(filename),
                 static_cast<int>(result.errorLine), static_cast<int>(result.errorColumn));
        return {};
    }
#else
    QString errorMsg;
    int errorLine = -1;
    int errorColumn = -1;
    if (!tsXML->setContent(tsFile.data(), &errorMsg, &errorLine, &errorColumn))
    {
        qWarning("Parsing error file %s in line %d column %d.", qUtf8Printable(filename), errorLine, errorColumn);
        return {};
    }
#endif

    const QDomNodeList messages = tsXML->elementsByTagName(TagMessage);
    if (messages.isEmpty())
    {
        qWarning("File doesn't contain any messages.");
        return {};
    }

    return messages;
}
