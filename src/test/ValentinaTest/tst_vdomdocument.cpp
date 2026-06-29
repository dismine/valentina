/************************************************************************
 **
 **  @file   tst_vdomdocument.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 6, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "tst_vdomdocument.h"

#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternconverter.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VDomDocument::TST_VDomDocument(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// Saving an empty document must fail and must NOT overwrite an existing good file on disk. QSaveFile is atomic but
// does not protect against committing empty content - this guard does. See empty .val corruption reports.
void TST_VDomDocument::RefuseEmptyDocumentSave() const
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temporary directory.");

    const QString path = dir.filePath(QStringLiteral("pattern.val"));
    const QByteArray sentinel = QByteArrayLiteral("<pattern>good</pattern>");

    {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::WriteOnly), "Failed to write sentinel file.");
        file.write(sentinel);
    }

    VDomDocument doc; // empty: no document element
    QString error;
    QVERIFY2(not doc.SaveDocument(path, error), "Saving an empty document unexpectedly succeeded.");
    QVERIFY2(not error.isEmpty(), "Failed save did not report an error.");

    QFile file(path);
    QVERIFY2(file.open(QIODevice::ReadOnly), "Failed to reopen sentinel file.");
    QCOMPARE(file.readAll(), sentinel); // existing good file must be untouched
}

//---------------------------------------------------------------------------------------------------------------------
// ValidateXMLData must reject content that does not conform to the schema, so a damaged document can never be
// committed over a good file.
void TST_VDomDocument::RejectInvalidDataAgainstSchema() const
{
    const QByteArray invalid = QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<notapattern/>\n");
    QString error;
    QVERIFY2(not VDomDocument::ValidateXMLData(VPatternConverter::CurrentSchema, invalid, QStringLiteral("test"), error),
             "Invalid XML unexpectedly passed schema validation.");
    QVERIFY2(not error.isEmpty(), "Failed validation did not report an error.");
}
