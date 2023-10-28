/************************************************************************
 **
 **  @file   vknownmeasurementsdocument.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2023
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
#include "vknownmeasurementsdocument.h"
#include "../ifc/xml/vknownmeasurementsconverter.h"
#include "../ifc/xml/vpatternimage.h"
#include "../vmisc/projectversion.h"
#include "ifcdef.h"
#include "literals.h"
#include "quuid.h"
#include "vformat/knownmeasurements/vknownmeasurement.h"
#include "vknownmeasurements.h"

#include <QUuid>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// The list of all string we use for conversion
// Better to use global variables because repeating QStringLiteral blows up code size
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagKnownMeasurements, ("known-measurements"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagMeasurements, ("measurements"_L1))            // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagMeasurement, ("m"_L1))                        // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagDiagrams, ("diagrams"_L1))                    // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagImage, ("tagImage"_L1))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagName, ("name"_L1))                            // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagDescription, ("description"_L1))              // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tagInfo, ("info"_L1))                            // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrFormula, ("formula"_L1))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrSpecialUnits, ("specialUnits"_L1))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrName, ("name"_L1))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrContentType, ("contentType"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrUId, ("uid"_L1))                             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrTitle, ("title"_L1))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrReadOnly, ("read-only"_L1))                  // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrDescription, ("description"_L1))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrFullName, ("full_name"_L1))                  // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, attrDiagram, ("diagram"_L1))                     // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto FileComment() -> QString
{
    return u"Known measurements created with Valentina v%1 (https://smart-pattern.com.ua/)."_s.arg(AppVersionStr());
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VKnownMeasurementsDocument::VKnownMeasurementsDocument(QObject *parent)
  : VDomDocument(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsDocument::SaveDocument(const QString &fileName, QString &error) -> bool
{
    // Update comment with Valentina version
    QDomNode commentNode = documentElement().firstChild();
    if (commentNode.isComment())
    {
        QDomComment comment = commentNode.toComment();
        comment.setData(FileComment());
    }

    return VDomDocument::SaveDocument(fileName, error);
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::CreateEmptyFile()
{
    clear();
    QDomElement kmElement = this->createElement(*tagKnownMeasurements);

    kmElement.appendChild(createComment(FileComment()));
    kmElement.setAttribute(AttrKMVersion, VKnownMeasurementsConverter::KnownMeasurementsMaxVerStr);
    kmElement.setAttribute(*attrUId, QUuid::createUuid().toString());

    QDomElement info = createElement(*tagInfo);
    info.appendChild(createElement(*tagName));
    info.appendChild(createElement(*tagDescription));
    kmElement.appendChild(info);

    kmElement.appendChild(createElement(*tagDiagrams));
    kmElement.appendChild(createElement(*tagMeasurements));

    appendChild(kmElement);
    insertBefore(
        createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")),
        firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::AddEmptyMeasurement(const QString &name)
{
    const QDomElement element = MakeEmptyMeasurement(name);

    const QDomNodeList list = elementsByTagName(*tagMeasurements);
    list.at(0).appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::AddEmptyMeasurementAfter(const QString &after, const QString &name)
{
    const QDomElement element = MakeEmptyMeasurement(name);
    const QDomElement sibling = FindM(after);

    const QDomNodeList list = elementsByTagName(*tagMeasurements);

    if (sibling.isNull())
    {
        list.at(0).appendChild(element);
    }
    else
    {
        list.at(0).insertAfter(element, sibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::AddImage(const VPatternImage &image)
{
    const QDomElement element = MakeEmptyImage(image);

    const QDomNodeList list = elementsByTagName(*tagDiagrams);
    list.at(0).appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::RemoveMeasurement(const QString &name)
{
    const QDomNodeList list = elementsByTagName(*tagMeasurements);
    list.at(0).removeChild(FindM(name));
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::RemoveImage(const QUuid &id)
{
    const QDomNodeList list = elementsByTagName(*tagMeasurements);
    list.at(0).removeChild(FindImage(id));
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::MoveTop(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomNodeList mList = elementsByTagName(*tagMeasurement);
    if (mList.size() >= 2)
    {
        const QDomNode top = mList.at(0);
        if (not top.isNull())
        {
            const QDomNodeList list = elementsByTagName(*tagMeasurements);
            list.at(0).insertBefore(node, top);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::MoveUp(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomElement prSibling = node.previousSiblingElement(*tagMeasurement);
    if (not prSibling.isNull())
    {
        const QDomNodeList list = elementsByTagName(*tagMeasurements);
        list.at(0).insertBefore(node, prSibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::MoveDown(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomElement nextSibling = node.nextSiblingElement(*tagMeasurement);
    if (not nextSibling.isNull())
    {
        const QDomNodeList list = elementsByTagName(*tagMeasurements);
        list.at(0).insertAfter(node, nextSibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::MoveBottom(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomNodeList mList = elementsByTagName(*tagMeasurement);
    if (mList.size() >= 2)
    {
        const QDomNode bottom = mList.at(mList.size() - 1);
        if (not bottom.isNull())
        {
            const QDomNodeList list = elementsByTagName(*tagMeasurements);
            list.at(0).insertAfter(node, bottom);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QUuid VKnownMeasurementsDocument::GetUId() const
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        const QDomElement rootElement = root.toElement();
        if (not rootElement.isNull())
        {
            return QUuid(GetParametrEmptyString(rootElement, AttrKMVersion));
        }
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetUId(const QUuid &id)
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        QDomElement rootElement = root.toElement();
        if (not rootElement.isNull())
        {
            SetAttribute(rootElement, *attrUId, id.toString());
        }
    }
    else
    {
        qDebug() << "Can't save known measurements uid " << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VKnownMeasurementsDocument::Name() const
{
    return UniqueTagText(*tagName, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetName(const QString &name)
{
    setTagText(*tagName, name);
}

//---------------------------------------------------------------------------------------------------------------------
QString VKnownMeasurementsDocument::Description() const
{
    return UniqueTagText(*tagDescription, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetDescription(const QString &desc)
{
    setTagText(*tagDescription, desc);
}

//---------------------------------------------------------------------------------------------------------------------
bool VKnownMeasurementsDocument::IsReadOnly() const
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        const QDomElement rootElement = root.toElement();
        if (not rootElement.isNull())
        {
            return GetParametrBool(rootElement, *attrReadOnly, falseStr);
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetReadOnly(bool ro)
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        QDomElement rootElement = root.toElement();
        if (not rootElement.isNull())
        {
            SetAttributeOrRemoveIf<bool>(rootElement, *attrReadOnly, ro, [](bool ro) noexcept { return not ro; });
        }
    }
    else
    {
        qDebug() << "Can't save known measurements read only status " << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsDocument::KnownMeasurements() const -> VKnownMeasurements
{
    VKnownMeasurements known;

    known.SetUId(GetUId());
    known.SetName(Name());
    known.SetDescription(Description());
    known.SetReadOnly(IsReadOnly());

    ReadImages(known);
    ReadMeasurements(known);

    return known;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetMName(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, *attrName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetMFormula(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, *attrFormula, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetMSpecialUnits(const QString &name, bool special)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttributeOrRemoveIf<bool>(node, *attrSpecialUnits, special,
                                     [](bool special) noexcept { return not special; });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetMDescription(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttributeOrRemoveIf<QString>(node, *attrDescription, text,
                                        [](const QString &text) noexcept { return text.isEmpty(); });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetMFullName(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, *attrFullName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetMImage(const QString &name, const QUuid &imageId)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttributeOrRemoveIf<QUuid>(node, *attrDiagram, imageId,
                                      [](const QUuid &imageId) noexcept { return imageId.isNull(); });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetImageContent(const QUuid &id, const VPatternImage &image)
{
    QDomElement node = FindImage(id);
    if (not node.isNull())
    {
        setTagText(node, image.ContentData());
        node.setAttribute(*attrContentType, image.ContentType());
    }
    else
    {
        qWarning() << tr("Can't find image by id '%1'").arg(id.toString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::SetImageTitle(const QUuid &id, const QString &text)
{
    QDomElement node = FindImage(id);
    if (not node.isNull())
    {
        SetAttributeOrRemoveIf<QString>(node, *attrTitle, text,
                                        [](const QString &text) noexcept { return text.isEmpty(); });
    }
    else
    {
        qWarning() << tr("Can't find image by id '%1'").arg(id.toString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VKnownMeasurementsDocument::MakeEmptyMeasurement(const QString &name)
{
    QDomElement element = createElement(*tagMeasurement);
    SetAttribute(element, *attrName, name);
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurementsDocument::FindM(const QString &name) const -> QDomElement
{
    if (name.isEmpty())
    {
        qWarning() << tr("The measurement name is empty!");
        return {};
    }

    QDomNodeList list = elementsByTagName(*tagMeasurement);

    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull())
        {
            continue;
        }

        const QString parameter = domElement.attribute(*attrName);
        if (parameter == name)
        {
            return domElement;
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VKnownMeasurementsDocument::MakeEmptyImage(const VPatternImage &image)
{
    QDomElement element = createElement(*tagImage);

    SetAttribute(element, *attrContentType, image.ContentType());
    SetAttribute(element, *attrUId, QUuid::createUuid().toString());
    SetAttributeOrRemoveIf<QString>(element, *attrTitle, image.Title(),
                                    [](const QString &text) noexcept { return text.isEmpty(); });
    setTagText(element, image.ContentData());

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VKnownMeasurementsDocument::FindImage(const QUuid &id) const
{
    if (id.isNull())
    {
        qWarning() << tr("The image id is empty!");
        return {};
    }

    QDomNodeList list = elementsByTagName(*tagImage);

    QString idString = id.toString();

    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull())
        {
            continue;
        }

        const QString parameter = domElement.attribute(*attrUId);
        if (parameter == idString)
        {
            return domElement;
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::ReadImages(VKnownMeasurements &known) const
{
    QDomNodeList list = elementsByTagName(*tagImage);

    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull())
        {
            continue;
        }

        VPatternImage image;
        image.SetContentData(domElement.text().toLatin1(), domElement.attribute(*attrContentType));
        image.SetTitle(domElement.attribute(*attrTitle));

        known.AddImage(QUuid(domElement.attribute(*attrUId)), image);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurementsDocument::ReadMeasurements(VKnownMeasurements &known) const
{
    QDomNodeList list = elementsByTagName(*tagMeasurement);

    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull())
        {
            continue;
        }

        VKnownMeasurement m;
        m.name = domElement.attribute(*attrName);
        m.fullName = domElement.attribute(*attrFullName);
        m.description = domElement.attribute(*attrDescription);
        m.formula = domElement.attribute(*attrFormula);
        m.specialUnits = GetParametrBool(domElement, *attrSpecialUnits, falseStr);
        m.diagram = QUuid(GetParametrEmptyString(domElement, *attrDiagram));
        m.index = i;

        known.AddMeasurement(m);
    }
}
