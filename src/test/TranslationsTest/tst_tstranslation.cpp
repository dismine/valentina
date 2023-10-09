/************************************************************************
 **
 **  @file   tst_tstranslation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 12, 2015
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

#include "tst_tstranslation.h"

#include <QDomDocument>
#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

Q_DECLARE_METATYPE(QDomElement) // Need for testing

//---------------------------------------------------------------------------------------------------------------------
TST_TSTranslation::TST_TSTranslation(QObject *parent)
  : TST_AbstractTranslation(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckEnglishLocalization_data()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<QString>("translation");

    const QString fileName = QStringLiteral("valentina_en_US.ts");
    const QDomNodeList messages = LoadTSFile(fileName);
    if (messages.isEmpty())
    {
        QFAIL("Can't begin test.");
    }

    for (qint32 i = 0, num = messages.size(); i < num; ++i)
    {
        const QDomElement message = messages.at(i).toElement();
        if (not message.isNull())
        {
            const QString source = message.firstChildElement(TagSource).text();
            if (source.isEmpty())
            {
                continue;
            }

            const QDomElement translationTag = message.firstChildElement(TagTranslation);
            if (translationTag.hasAttribute(AttrType))
            {
                const QString attrVal = translationTag.attribute(AttrType);
                if (attrVal == AttrValVanished || attrVal == AttrValUnfinished || attrVal == AttrValObsolete)
                {
                    continue;
                }
            }

            auto PluralForm = [translationTag]()
            {
                QDomNodeList children = translationTag.childNodes();
                for (int i = 0; i < children.size(); ++i)
                {
                    QDomNode child = children.item(i);
                    if (child.isElement())
                    {
                        return true;
                    }
                }
                return false;
            };

            if (PluralForm())
            {
                continue; // do not check plural forms
            }

            const QString translation = translationTag.text();
            if (translation.isEmpty())
            {
                continue;
            }

            const QString caseName =
                QStringLiteral("File '%1'. Check modification source message '%2'.").arg(fileName, source);
            QTest::newRow(qUtf8Printable(caseName)) << source << translation;
        }
        else
        {
            const QString caseName = QStringLiteral("Message %1 is null.").arg(i);
            QFAIL(qUtf8Printable(caseName));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckEnglishLocalization()
{
    QFETCH(QString, source);
    QFETCH(QString, translation);

    QCOMPARE(source, translation);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckEmptyToolButton_data()
{
    PrepareOriginalStrings();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckEmptyToolButton()
{
    QFETCH(QString, source);
    QFETCH(QDomElement, message);

    if (source == "..."_L1)
    {
        const QDomElement translationTag = message.firstChildElement(TagTranslation);
        if (translationTag.hasAttribute(AttrType))
        {
            const QString attrVal = translationTag.attribute(AttrType);
            if (attrVal == AttrValVanished || attrVal == AttrValObsolete)
            {
                return;
            }
        }

        const QDomNode context = message.parentNode();
        if (context.isNull())
        {
            QFAIL("Can't get context.");
        }

        const QString contextName = context.firstChildElement(TagName).text();
        const QString error = u"Found '...' in context '%1'"_s.arg(contextName);
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckEllipsis_data()
{
    PrepareOriginalStrings();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckEllipsis()
{
    QFETCH(QString, source);
    QFETCH(QDomElement, message);

    if (source.endsWith("..."))
    {
        const QDomElement translationTag = message.firstChildElement(TagTranslation);
        if (translationTag.hasAttribute(AttrType))
        {
            const QString attrVal = translationTag.attribute(AttrType);
            if (attrVal == AttrValVanished || attrVal == AttrValObsolete)
            {
                return;
            }
        }

        const QDomNode context = message.parentNode();
        if (context.isNull())
        {
            QFAIL("Can't get context.");
        }

        const QString contextName = context.firstChildElement(TagName).text();
        const QString error =
            u"String '%1' ends with '...' in context '%2'. Repalce it with '…'."_s.arg(source, contextName);
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckInvalidCharacter_data()
{
    PrepareOriginalStrings();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::CheckInvalidCharacter()
{
    QFETCH(QString, source);
    QFETCH(QDomElement, message);

    if (source == '='_L1 or source == '%'_L1)
    {
        const QDomNode context = message.parentNode();
        if (context.isNull())
        {
            QFAIL("Can't get context.");
        }

        const QString contextName = context.firstChildElement(TagName).text();
        const QString error = u"String contains invalid character '%1' in context '%2'. It should not be "
                              u"marked for translation."_s.arg(source, contextName);
        QFAIL(qUtf8Printable(error));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_TSTranslation::PrepareOriginalStrings()
{
    QTest::addColumn<QString>("source");
    QTest::addColumn<QDomElement>("message");

    const QString fileName = QStringLiteral("valentina.ts");
    const QDomNodeList messages = LoadTSFile(fileName);
    if (messages.isEmpty())
    {
        QFAIL("Can't begin test.");
    }

    for (qint32 i = 0, num = messages.size(); i < num; ++i)
    {
        const QDomElement message = messages.at(i).toElement();
        if (not message.isNull())
        {
            const QString source = message.firstChildElement(TagSource).text();
            if (source.isEmpty())
            {
                continue;
            }

            const QString tag =
                QStringLiteral("File '%1'. Check modification source message '%2'.").arg(fileName, source);
            QTest::newRow(qUtf8Printable(tag)) << source << message;
        }
        else
        {
            const QString errorMessage = QStringLiteral("Message %1 is null.").arg(i);
            QFAIL(qUtf8Printable(errorMessage));
        }
    }
}
