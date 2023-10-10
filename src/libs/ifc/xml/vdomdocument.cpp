/************************************************************************
 **
 **  @file   vdomdocument.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vdomdocument.h"

#include <QSaveFile>
#include <qdom.h>

#include "../exception/vexception.h"
#include "../exception/vexceptionbadid.h"
#include "../exception/vexceptionconversionerror.h"
#include "../exception/vexceptionemptyparameter.h"
#include "../exception/vexceptionwrongid.h"
#include "../ifcdef.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractMessageHandler>
#include <QSourceLocation>
#endif

#include <QByteArray>
#include <QDir>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QIODevice>
#include <QMessageLogger>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVector>
#include <QXmlStreamWriter>
#include <QtConcurrentRun>
#include <QtDebug>

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <unistd.h>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void SaveNodeCanonically(QXmlStreamWriter &stream, const QDomNode &domNode)
{
    if (stream.hasError())
    {
        return;
    }

    if (domNode.isElement())
    {
        const QDomElement domElement = domNode.toElement();
        if (not domElement.isNull())
        {
            stream.writeStartElement(domElement.tagName());

            if (domElement.hasAttributes())
            {
                QMap<QString, QString> attributes;
                const QDomNamedNodeMap attributeMap = domElement.attributes();
                for (int i = 0; i < attributeMap.count(); ++i)
                {
                    const QDomNode attribute = attributeMap.item(i);
                    attributes.insert(attribute.nodeName(), attribute.nodeValue());
                }

                QMap<QString, QString>::const_iterator i = attributes.constBegin();
                while (i != attributes.constEnd())
                {
                    stream.writeAttribute(i.key(), i.value());
                    ++i;
                }
            }

            if (domElement.hasChildNodes())
            {
                QDomNode elementChild = domElement.firstChild();
                while (not elementChild.isNull())
                {
                    SaveNodeCanonically(stream, elementChild);
                    elementChild = elementChild.nextSibling();
                }
            }

            stream.writeEndElement();
        }
    }
    else if (domNode.isComment())
    {
        stream.writeComment(domNode.nodeValue());
    }
    else if (domNode.isText())
    {
        stream.writeCharacters(domNode.nodeValue());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GetChildElements(const QDomNode &e) -> QList<QDomNode>
{
    QDomNodeList children = e.childNodes();
    QList<QDomNode> r;
    r.reserve(children.size());
    for (int k = 0; k < children.size(); ++k)
    {
        r << children.at(k);
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto GetElementAttributes(const QDomNode &e) -> QList<QDomNode>
{
    QDomNamedNodeMap attributes = e.attributes();
    QList<QDomNode> r;
    r.reserve(attributes.size());
    for (int k = 0; k < attributes.size(); ++k)
    {
        r << attributes.item(k);
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto LessThen(const QDomNode &element1, const QDomNode &element2) -> bool
{
    if (element1.nodeType() != element2.nodeType())
    {
        return element1.nodeType() < element2.nodeType();
    }

    QString tag1 = element1.nodeName();
    QString tag2 = element2.nodeName();

    // qDebug() << tag1 <<tag2;
    if (tag1 != tag2)
    {
        return tag1 < tag2;
    }

    // Compare attributes
    QList<QDomNode> attributes1 = GetElementAttributes(element1);
    QList<QDomNode> attributes2 = GetElementAttributes(element2);

    if (attributes1.size() != attributes2.size())
    {
        return attributes1.size() < attributes2.size();
    }

    auto CompareDomNodeLists = [](QList<QDomNode> list1, QList<QDomNode> list2, bool *stop)
    {
        *stop = false;
        std::sort(list1.begin(), list1.end(), LessThen);
        std::sort(list2.begin(), list2.end(), LessThen);
        // qDebug() << "comparing sorted lists";
        for (int k = 0; k < list1.size(); ++k)
        {
            if (!LessThen(list1[k], list2[k]))
            {
                if (LessThen(list2[k], list1[k]))
                {
                    *stop = true;
                    // qDebug() << "false!";
                    return false;
                }
            }
            else
            {
                *stop = true;
                // qDebug() << "true!";
                return true;
            }
        }
        return false;
    };

    bool stop = false;
    bool result = CompareDomNodeLists(attributes1, attributes2, &stop);
    if (stop)
    {
        return result;
    }

    // Compare children
    QList<QDomNode> elts1 = GetChildElements(element1);
    QList<QDomNode> elts2 = GetChildElements(element2);

    QString value1, value2;

    if (elts1.size() != elts2.size())
    {
        return elts1.size() < elts2.size();
    }

    if (elts1.isEmpty())
    {
        value1 = element1.nodeValue();
        value2 = element2.nodeValue();

        // qDebug() <<value1 << value2 << (value1 < value2);
        return value1 < value2;
    }

    result = CompareDomNodeLists(elts1, elts2, &stop);
    if (stop)
    {
        return result;
    }
    return false;
}
} // namespace

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vXML, "v.xml") // NOLINT

QT_WARNING_POP

const QString VDomDocument::AttrId = QStringLiteral("id");
const QString VDomDocument::AttrText = QStringLiteral("text");
const QString VDomDocument::AttrBold = QStringLiteral("bold");
const QString VDomDocument::AttrItalic = QStringLiteral("italic");
const QString VDomDocument::AttrAlignment = QStringLiteral("alignment");
const QString VDomDocument::AttrFSIncrement = QStringLiteral("sfIncrement");

const QString VDomDocument::TagVersion = QStringLiteral("version");
const QString VDomDocument::TagUnit = QStringLiteral("unit");
const QString VDomDocument::TagLine = QStringLiteral("line");

//---------------------------------------------------------------------------------------------------------------------
VDomDocument::VDomDocument(QObject *parent)
  : QObject(parent),
    QDomDocument(),
    m_elementIdCache(),
    m_watcher(new QFutureWatcher<QHash<quint32, QDomElement>>(this))
{
    connect(m_watcher, &QFutureWatcher<QHash<quint32, QDomElement>>::finished, this, &VDomDocument::CacheRefreshed);
}

//---------------------------------------------------------------------------------------------------------------------
VDomDocument::~VDomDocument()
{
    m_watcher->cancel();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::elementById(quint32 id, const QString &tagName, bool updateCache) -> QDomElement
{
    if (id == 0)
    {
        return QDomElement();
    }

    if (m_elementIdCache.contains(id))
    {
        const QDomElement e = m_elementIdCache.value(id);
        if (e.parentNode().nodeType() != QDomNode::BaseNode)
        {
            if (not tagName.isEmpty())
            {
                if (e.tagName() == tagName)
                {
                    return e;
                }
            }
            else
            {
                return e;
            }
        }
    }

    if (updateCache)
    { // Cached missed
        RefreshElementIdCache();
    }

    if (tagName.isEmpty())
    {
        // Because VDomDocument::find checks for unique id we must use temp cache
        QHash<quint32, QDomElement> tmpCache;
        if (VDomDocument::find(tmpCache, this->documentElement(), id))
        {
            return tmpCache.value(id);
        }
    }
    else
    {
        const QDomNodeList list = elementsByTagName(tagName);
        for (int i = 0; i < list.size(); ++i)
        {
            const QDomElement domElement = list.at(i).toElement();
            if (not domElement.isNull() && domElement.hasAttribute(AttrId))
            {
                const quint32 elementId = GetParametrUInt(domElement, AttrId, NULL_ID_STR);

                if (elementId == id)
                {
                    return domElement;
                }
            }
        }
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Find element by id.
 * @param cache cache with element ids
 * @param node node
 * @param id id value
 * @return true if found
 */
auto VDomDocument::find(QHash<quint32, QDomElement> &cache, const QDomElement &node, quint32 id) -> bool
{
    if (node.hasAttribute(AttrId))
    {
        try
        {
            const quint32 elementId = GetParametrUInt(node, AttrId, NULL_ID_STR);

            if (cache.contains(elementId))
            {
                qWarning() << tr("Not unique id (%1)").arg(elementId);
            }

            cache.insert(elementId, node);
            if (elementId == id)
            {
                return true;
            }
        }
        catch (const VExceptionConversionError &)
        {
            // ignore
        }
    }

    for (qint32 i = 0; i < node.childNodes().length(); ++i)
    {
        const QDomNode n = node.childNodes().at(i);
        if (n.isElement())
        {
            if (VDomDocument::find(cache, n.toElement(), id))
            {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::RefreshCache(const QDomElement &root) const -> QHash<quint32, QDomElement>
{
    QHash<quint32, QDomElement> cache;
    VDomDocument::find(cache, root, NULL_ID);
    return cache;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::SaveCanonicalXML(QIODevice *file, int indent, QString &error) const -> bool
{
    SCASSERT(file != nullptr)

    QXmlStreamWriter stream(file);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(indent);
    stream.writeStartDocument();

    QDomNode root = documentElement();
    while (not root.isNull())
    {
        SaveNodeCanonically(stream, root);
        if (stream.hasError())
        {
            break;
        }
        root = root.nextSibling();
    }

    stream.writeEndDocument();

    if (stream.hasError())
    {
        error = tr("Fail to write Canonical XML.");
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the long long value of the given attribute. RENAME: GetParameterLongLong?
 * @param domElement tag in xml tree
 * @param name attribute name
 * @return long long value
 */
auto VDomDocument::GetParametrUInt(const QDomElement &domElement, const QString &name, const QString &defValue)
    -> quint32
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name of parametr is empty");
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null"); //-V591

    bool ok = false;
    QString parametr;
    quint32 id = 0;

    try
    {
        parametr = GetParametrString(domElement, name, defValue);
        id = parametr.toUInt(&ok);
        if (!ok)
        {
            throw VExceptionConversionError(QObject::tr("Can't convert toUInt parameter"), name);
        }
    }
    catch (const VExceptionEmptyParameter &e)
    {
        VExceptionConversionError excep(QObject::tr("Can't convert toUInt parameter"), name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }

    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetParametrInt(const QDomElement &domElement, const QString &name, const QString &defValue) -> int
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name of parametr is empty");
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null"); //-V591

    bool ok = false;
    QString parametr;
    int value = 0;

    try
    {
        parametr = GetParametrString(domElement, name, defValue);
        value = parametr.toInt(&ok);
        if (ok == false)
        {
            throw VExceptionConversionError(QObject::tr("Can't convert toInt parameter"), name);
        }
    }
    catch (const VExceptionEmptyParameter &e)
    {
        VExceptionConversionError excep(QObject::tr("Can't convert toInt parameter"), name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }

    return value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetParametrBool(const QDomElement &domElement, const QString &name, const QString &defValue) -> bool
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name of parametr is empty");
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    QString parametr;
    bool val = true;

    const QString message = QObject::tr("Can't convert toBool parameter");
    try
    {
        parametr = GetParametrString(domElement, name, defValue);

        const QStringList bools{trueStr, falseStr, QChar('1'), QChar('0')};
        switch (bools.indexOf(parametr))
        {
            case 0: // true
            case 2: // 1
                val = true;
                break;
            case 1: // false
            case 3: // 0
                val = false;
                break;
            default: // others
                throw VExceptionConversionError(message, name);
        }
    }
    catch (const VExceptionEmptyParameter &e)
    {
        VExceptionConversionError excep(message, name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }

    return val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetParametrUsage(const QDomElement &domElement, const QString &name) -> NodeUsage
{
    const bool value = GetParametrBool(domElement, name, trueStr);
    if (value)
    {
        return NodeUsage::InUse;
    }
    return NodeUsage::NotInUse;
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::SetParametrUsage(QDomElement &domElement, const QString &name, const NodeUsage &value)
{
    if (value == NodeUsage::InUse)
    {
        domElement.setAttribute(name, trueStr);
    }
    else
    {
        domElement.setAttribute(name, falseStr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the string value of the given attribute. RENAME: see above
 *
 * if attribute empty return default value. If default value empty too throw exception.
 * @return attribute value
 * @throw VExceptionEmptyParameter when attribute is empty
 */
auto VDomDocument::GetParametrString(const QDomElement &domElement, const QString &name, const QString &defValue)
    -> QString
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name of parametr is empty");
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    const QString parameter = domElement.attribute(name, defValue);
    if (parameter.isEmpty())
    {
        if (defValue.isEmpty())
        {
            throw VExceptionEmptyParameter(QObject::tr("Got empty parameter"), name, domElement);
        }
        return defValue;
    }
    return parameter;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetParametrEmptyString(const QDomElement &domElement, const QString &name) -> QString
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name of parametr is empty");
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    return domElement.attribute(name);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the double value of the given attribute.
 * @param domElement tag in xml tree
 * @param name attribute name
 * @return double value
 */
auto VDomDocument::GetParametrDouble(const QDomElement &domElement, const QString &name, const QString &defValue)
    -> qreal
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name of parametr is empty");
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    bool ok = false;
    qreal param = 0;

    try
    {
        QString parametr = GetParametrString(domElement, name, defValue);
        param = parametr.replace(','_L1, '.'_L1).toDouble(&ok);
        if (ok == false)
        {
            throw VExceptionConversionError(QObject::tr("Can't convert toDouble parameter"), name);
        }
    }
    catch (const VExceptionEmptyParameter &e)
    {
        VExceptionConversionError excep(QObject::tr("Can't convert toDouble parameter"), name);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    return param;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetParametrId return value id attribute.
 * @param domElement tag in xml tree.
 * @return id value.
 */
auto VDomDocument::GetParametrId(const QDomElement &domElement) -> quint32
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    quint32 id = NULL_ID;

    const QString message = QObject::tr("Got wrong parameter id. Need only id > 0.");
    try
    {
        id = GetParametrUInt(domElement, VDomDocument::AttrId, NULL_ID_STR);
        if (id == NULL_ID)
        {
            throw VExceptionWrongId(message, domElement);
        }
    }
    catch (const VExceptionConversionError &e)
    {
        VExceptionWrongId excep(message, domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::UniqueTagText(const QString &tagName, const QString &defVal) const -> QString
{
    QDomElement domElement = UniqueTag(tagName);
    if (not domElement.isNull())
    {
        const QString text = domElement.text();
        if (text.isEmpty())
        {
            return defVal;
        }

        return text;
    }

    return defVal;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TestUniqueId test exist unique id in pattern file. Each id must be unique.
 */
void VDomDocument::TestUniqueId() const
{
    QVector<quint32> vector;
    CollectId(documentElement(), vector);
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::CollectId(const QDomElement &node, QVector<quint32> &vector) const
{
    if (node.hasAttribute(VDomDocument::AttrId))
    {
        const quint32 id = GetParametrId(node);
        if (vector.contains(id))
        {
            throw VExceptionWrongId(tr("This id (%1) is not unique.").arg(id), node);
        }
        vector.append(id);
    }

    for (qint32 i = 0; i < node.childNodes().length(); ++i)
    {
        const QDomNode n = node.childNodes().at(i);
        if (n.isElement())
        {
            CollectId(n.toElement(), vector);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::RefreshElementIdCache()
{
    if (m_watcher->isFinished())
    {
        m_watcher->setFuture(QtConcurrent::run([this]() { return RefreshCache(documentElement()); }));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::Compare(const QDomElement &element1, const QDomElement &element2) -> bool
{
    QFuture<bool> lessThen2 = QtConcurrent::run(LessThen, element2, element1);
    return !LessThen(element1, element2) && !lessThen2.result();
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::CacheRefreshed()
{
    m_elementIdCache = m_watcher->future().result();
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::setXMLContent(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        const QString errorMsg(tr("Can't open file %1:\n%2.").arg(fileName, file.errorString()));
        throw VException(errorMsg);
    }

    QString errorMsg;
    int errorLine = -1;
    int errorColumn = -1;
    if (QDomDocument::setContent(&file, &errorMsg, &errorLine, &errorColumn) == false)
    {
        file.close();
        VException e(errorMsg);
        e.AddMoreInformation(
            tr("Parsing error file %3 in line %1 column %2").arg(errorLine).arg(errorColumn).arg(fileName));
        throw e;
    }

    RefreshElementIdCache();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::UnitsHelpString() -> QString
{
    QString r;
    for (auto i = static_cast<int>(Unit::Mm), last = static_cast<int>(Unit::LAST_UNIT_DO_NOT_USE); i < last; ++i)
    {
        r += UnitsToStr(static_cast<Unit>(i));
        if (i < last - 1)
        {
            r += ", ";
        }
    }
    return r;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::CreateElementWithText(const QString &tagName, const QString &text) -> QDomElement
{
    QDomElement tag = createElement(tagName);
    tag.appendChild(createTextNode(text));
    return tag;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::SaveDocument(const QString &fileName, QString &error) -> bool
{
    if (fileName.isEmpty())
    {
        qDebug() << "Got empty file name.";
        return false;
    }
    bool success = false;
    QSaveFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        // See issue #666. QDomDocument produces random attribute order.
        const int indent = 4;
        if (not SaveCanonicalXML(&file, indent, error))
        {
            return false;
        }
        // Left these strings in case we will need them for testing purposes
        // QTextStream out(&file);
        // #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        //  out.setCodec("UTF-8");
        // #endif
        //  save(out, indent);

        success = file.commit();

#if defined(_POSIX_SYNCHRONIZED_IO) && _POSIX_SYNCHRONIZED_IO > 0
        if (success)
        {
            // https://stackoverflow.com/questions/74051505/does-qsavefilecommit-fsync-the-file-to-the-filesystem
            QString directoryPath = QFileInfo(file.fileName()).absoluteDir().path();
            int dirFd = ::open(directoryPath.toLocal8Bit().data(), O_RDONLY | O_DIRECTORY);
            if (dirFd != -1)
            {
                ::fsync(dirFd);
                ::close(dirFd);
            }
        }
#endif
    }

    if (not success)
    {
        error = file.errorString();
    }

    return success;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VDomDocument::Major() const -> QString
{
    QString version = UniqueTagText(TagVersion, "0.0.0");
    QStringList v = version.split('.'_L1);
    return v.at(0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VDomDocument::Minor() const -> QString
{
    QString version = UniqueTagText(TagVersion, "0.0.0");
    QStringList v = version.split('.'_L1);
    return v.at(1);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VDomDocument::Patch() const -> QString
{
    QString version = UniqueTagText(TagVersion, "0.0.0");
    QStringList v = version.split('.'_L1);
    return v.at(2);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetFormatVersionStr() const -> QString
{
    const QDomNodeList nodeList = this->elementsByTagName(TagVersion);
    if (nodeList.isEmpty())
    {
        const QString errorMsg(tr("Couldn't get version information."));
        throw VException(errorMsg);
    }

    if (nodeList.count() > 1)
    {
        const QString errorMsg(tr("Too many tags <%1> in file.").arg(TagVersion));
        throw VException(errorMsg);
    }

    const QDomNode domNode = nodeList.at(0);
    if (domNode.isNull() == false && domNode.isElement())
    {
        const QDomElement domElement = domNode.toElement();
        if (domElement.isNull() == false)
        {
            return domElement.text();
        }
    }
    return QString(QStringLiteral("0.0.0"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetFormatVersion(const QString &version) -> unsigned
{
    ValidateVersion(version);

    const QStringList ver = version.split('.'_L1);

    bool ok = false;
    const unsigned major = ver.at(0).toUInt(&ok);
    if (not ok)
    {
        return 0x0;
    }

    ok = false;
    const unsigned minor = ver.at(1).toUInt(&ok);
    if (not ok)
    {
        return 0x0;
    }

    ok = false;
    const unsigned patch = ver.at(2).toUInt(&ok);
    if (not ok)
    {
        return 0x0;
    }

    return (major << 16u) | (minor << 8u) | (patch);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::setTagText(const QString &tag, const QString &text) -> bool
{
    const QDomNodeList nodeList = this->elementsByTagName(tag);
    if (nodeList.isEmpty())
    {
        qDebug() << "Can't save tag " << tag << Q_FUNC_INFO;
    }
    else
    {
        QDomNode domNode = nodeList.at(0);
        if (not domNode.isNull() && domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            return setTagText(domElement, text);
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::setTagText(QDomElement &domElement, const QString &text) -> bool
{
    if (not domElement.isNull())
    {
        QDomNode oldText = domElement.firstChild();
        const QDomText newText = createTextNode(text);

        if (oldText.isNull())
        {
            domElement.appendChild(newText);
        }
        else
        {
            if (oldText.nodeType() == QDomNode::TextNode)
            {
                domElement.replaceChild(newText, oldText);
            }
            else
            {
                RemoveAllChildren(domElement);
                domElement.appendChild(newText);
            }
        }

        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::UniqueTag(const QString &tagName) const -> QDomElement
{
    const QDomNodeList nodeList = this->elementsByTagName(tagName);
    if (nodeList.isEmpty())
    {
        return {};
    }

    const QDomNode domNode = nodeList.at(0);
    if (not domNode.isNull() && domNode.isElement())
    {
        const QDomElement domElement = domNode.toElement();
        if (not domElement.isNull())
        {
            return domElement;
        }
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveAllChildren remove all children from file.
 * @param domElement tag in xml tree.
 */
void VDomDocument::RemoveAllChildren(QDomElement &domElement)
{
    if (domElement.hasChildNodes())
    {
        while (domElement.childNodes().length() >= 1)
        {
            domElement.removeChild(domElement.firstChild());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::ParentNodeById(const quint32 &nodeId) -> QDomNode
{
    QDomElement domElement = NodeById(nodeId);
    return domElement.parentNode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::CloneNodeById(const quint32 &nodeId) -> QDomElement
{
    QDomElement domElement = NodeById(nodeId);
    return domElement.cloneNode().toElement();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::NodeById(const quint32 &nodeId, const QString &tagName) -> QDomElement
{
    QDomElement domElement = elementById(nodeId, tagName);
    if (domElement.isNull() || domElement.isElement() == false)
    {
        throw VExceptionBadId(tr("Couldn't get node"), nodeId);
    }
    return domElement;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::SafeCopy(const QString &source, const QString &destination, QString &error) -> bool
{
    bool result = false;

    // #ifdef Q_OS_WIN32
    //     qt_ntfs_permission_lookup++; // turn checking on
    // #endif /*Q_OS_WIN32*/

    QTemporaryFile destFile(destination + ".XXXXXX"_L1);
    destFile.setAutoRemove(false); // Will be renamed to be destination file
    if (not destFile.open())
    {
        error = destFile.errorString();
    }
    else
    {
        QFile sourceFile(source);
        if (sourceFile.open(QIODevice::ReadOnly))
        {
            result = true;
            char block[4096];
            qint64 bytes;
            while ((bytes = sourceFile.read(block, sizeof(block))) > 0)
            {
                if (bytes != destFile.write(block, bytes))
                {
                    error = destFile.errorString();
                    result = false;
                    break;
                }
            }

            if (bytes == -1)
            {
                error = sourceFile.errorString();
                result = false;
            }

            if (result)
            {
                QFile::remove(destination);
                if (not destFile.rename(destination))
                {
                    error = destFile.errorString();
                    result = false;
                }
                else
                {
                    result = true;
                }
            }
        }
        else
        {
            error = sourceFile.errorString();
        }
    }

    // #ifdef Q_OS_WIN32
    //     qt_ntfs_permission_lookup--; // turn off check permission again
    // #endif /*Q_OS_WIN32*/

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDomDocument::GetLabelTemplate(const QDomElement &element) const -> QVector<VLabelTemplateLine>
{
    // We use implicit conversion. That's why check if values are still the same as excpected.
    Q_STATIC_ASSERT(Qt::AlignLeft == 1);
    Q_STATIC_ASSERT(Qt::AlignRight == 2);
    Q_STATIC_ASSERT(Qt::AlignHCenter == 4);

    QVector<VLabelTemplateLine> lines;

    if (not element.isNull())
    {
        QDomElement tagLine = element.firstChildElement();
        while (tagLine.isNull() == false)
        {
            if (tagLine.tagName() == TagLine)
            {
                VLabelTemplateLine line;
                line.line =
                    GetParametrString(tagLine, AttrText, QCoreApplication::translate("VDomDocument", "<empty>"));
                line.bold = GetParametrBool(tagLine, AttrBold, falseStr);
                line.italic = GetParametrBool(tagLine, AttrItalic, falseStr);
                line.alignment = static_cast<int>(GetParametrUInt(tagLine, AttrAlignment, QChar('0')));
                line.fontSizeIncrement = static_cast<int>(GetParametrUInt(tagLine, AttrFSIncrement, QChar('0')));
                lines.append(line);
            }
            tagLine = tagLine.nextSiblingElement(TagLine);
        }
    }

    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::SetLabelTemplate(QDomElement &element, const QVector<VLabelTemplateLine> &lines)
{
    if (not element.isNull())
    {
        for (const auto &line : lines)
        {
            QDomElement tagLine = createElement(TagLine);

            SetAttribute(tagLine, AttrText, line.line);
            SetAttribute(tagLine, AttrBold, line.bold);
            SetAttribute(tagLine, AttrItalic, line.italic);
            SetAttribute(tagLine, AttrAlignment, line.alignment);
            SetAttribute(tagLine, AttrFSIncrement, line.fontSizeIncrement);

            element.appendChild(tagLine);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDomDocument::ValidateVersion(const QString &version)
{
    const QRegularExpression rx(QStringLiteral("^([0-9]|[1-9][0-9]|[1-2][0-5][0-5]).([0-9]|[1-9][0-9]|[1-2][0-5][0-5])"
                                               ".([0-9]|[1-9][0-9]|[1-2][0-5][0-5])$"));

    if (rx.match(version).hasMatch() == false)
    {
        const QString errorMsg(tr("Version \"%1\" invalid.").arg(version));
        throw VException(errorMsg);
    }

    if (version == "0.0.0"_L1)
    {
        const QString errorMsg(tr("Version \"0.0.0\" invalid."));
        throw VException(errorMsg);
    }
}
