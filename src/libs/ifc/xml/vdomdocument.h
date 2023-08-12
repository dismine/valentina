/************************************************************************
 **
 **  @file   vdomdocument.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program that allows creating and modelling patterns of clothing.
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

#ifndef VDOMDOCUMENT_H
#define VDOMDOCUMENT_H

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QHash>
#include <QLatin1String>
#include <QLocale>
#include <QLoggingCategory>
#include <QString>
#include <QtCore/qcontainerfwd.h>
#include <QtGlobal>
#include <functional>

#include "../ifcdef.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/def.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/literals.h"

class QDomElement;
class QDomNode;
template <typename T> class QFutureWatcher;

Q_DECLARE_LOGGING_CATEGORY(vXML)

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

/**
 * @brief The VDomDocument class represents a Valentina document (.val file).
 *
 * A Valentina document describes the construction of a sewing pattern. The
 * information is stored in XML format. By parsing a VDomDocument, the contained
 * pattern is rendered to a Valentina graphics scene (VMainGraphicsScene).
 *
 * A sewing pattern consists of zero or more increments and one
 * or more pattern pieces.
 *
 * An increment is an auxiliary variable that is calculated from regular measurement
 * variables (that belong to the multisize measurements table). Increments are used to
 * create a graduation schema for the sewing pattern.
 *
 * A pattern piece contains
 * 1) auxiliary pattern construction elements (calculation),
 * 2) pattern construction elements (modeling), and
 * 3) special markers, e.g. seam allowances (details).
 * Of these, 2) and 3) are visible in the final pattern (draw mode 'Modeling'),
 * 1) is only displayed when editing (draw mode 'Calculation') the pattern.
 */
class VDomDocument : public QObject, public QDomDocument
{
    Q_OBJECT // NOLINT

public:
    static const QString AttrId;
    static const QString AttrText;
    static const QString AttrBold;
    static const QString AttrItalic;
    static const QString AttrAlignment;
    static const QString AttrFSIncrement;

    static const QString TagVersion;
    static const QString TagUnit;
    static const QString TagLine;

    explicit VDomDocument(QObject *parent = nullptr);
    virtual ~VDomDocument();
    auto elementById(quint32 id, const QString &tagName = QString(), bool updateCache = true) -> QDomElement;

    template <typename T> void SetAttribute(QDomElement &domElement, const QString &name, const T &value) const;

    template <typename T>
    void SetAttributeOrRemoveIf(QDomElement &domElement, const QString &name, const T &value,
                                const std::function<bool(const T &)> &removeCondition) const;

    static auto GetParametrUInt(const QDomElement &domElement, const QString &name, const QString &defValue) -> quint32;
    static auto GetParametrInt(const QDomElement &domElement, const QString &name, const QString &defValue) -> int;
    static auto GetParametrBool(const QDomElement &domElement, const QString &name, const QString &defValue) -> bool;

    static auto GetParametrUsage(const QDomElement &domElement, const QString &name) -> NodeUsage;
    static void SetParametrUsage(QDomElement &domElement, const QString &name, const NodeUsage &value);

    static auto GetParametrString(const QDomElement &domElement, const QString &name,
                                  const QString &defValue = QString()) -> QString;
    static auto GetParametrEmptyString(const QDomElement &domElement, const QString &name) -> QString;
    static auto GetParametrDouble(const QDomElement &domElement, const QString &name, const QString &defValue) -> qreal;
    static auto GetParametrId(const QDomElement &domElement) -> quint32;

    virtual void setXMLContent(const QString &fileName);
    static auto UnitsHelpString() -> QString;

    auto CreateElementWithText(const QString &tagName, const QString &text) -> QDomElement;

    virtual auto SaveDocument(const QString &fileName, QString &error) -> bool;
    auto Major() const -> QString;
    auto Minor() const -> QString;
    auto Patch() const -> QString;
    virtual auto GetFormatVersionStr() const -> QString;
    static auto GetFormatVersion(const QString &version) -> unsigned;
    static void RemoveAllChildren(QDomElement &domElement);

    auto ParentNodeById(const quint32 &nodeId) -> QDomNode;
    auto CloneNodeById(const quint32 &nodeId) -> QDomElement;
    auto NodeById(const quint32 &nodeId, const QString &tagName = QString()) -> QDomElement;

    static auto SafeCopy(const QString &source, const QString &destination, QString &error) -> bool;

    auto GetLabelTemplate(const QDomElement &element) const -> QVector<VLabelTemplateLine>;
    void SetLabelTemplate(QDomElement &element, const QVector<VLabelTemplateLine> &lines);

    void TestUniqueId() const;

    void RefreshElementIdCache();

    static auto Compare(const QDomElement &element1, const QDomElement &element2) -> bool;

protected:
    auto setTagText(const QString &tag, const QString &text) -> bool;
    auto setTagText(QDomElement &domElement, const QString &text) -> bool;
    auto UniqueTag(const QString &tagName) const -> QDomElement;
    auto UniqueTagText(const QString &tagName, const QString &defVal = QString()) const -> QString;
    void CollectId(const QDomElement &node, QVector<quint32> &vector) const;

    static void ValidateVersion(const QString &version);

private slots:
    void CacheRefreshed();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VDomDocument) // NOLINT
    /** @brief Map used for finding element by id. */
    QHash<quint32, QDomElement> m_elementIdCache;
    QFutureWatcher<QHash<quint32, QDomElement>> *m_watcher;

    static auto find(QHash<quint32, QDomElement> &cache, const QDomElement &node, quint32 id) -> bool;
    auto RefreshCache(const QDomElement &root) const -> QHash<quint32, QDomElement>;

    auto SaveCanonicalXML(QIODevice *file, int indent, QString &error) const -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
/**
 * @brief SetAttribute set attribute in pattern file. Replace "," by ".".
 * @param domElement element in xml tree.
 * @param name name of attribute.
 * @param value value of attribute.
 */
inline void VDomDocument::SetAttribute(QDomElement &domElement, const QString &name, const T &value) const
{
    // See specification for xs:decimal
    const QLocale locale = QLocale::c();
    domElement.setAttribute(name, locale.toString(value).remove(LocaleGroupSeparator(locale)));
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VDomDocument::SetAttribute<QString>(QDomElement &domElement, const QString &name,
                                                const QString &value) const
{
    domElement.setAttribute(name, value);
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VDomDocument::SetAttribute<QLatin1String>(QDomElement &domElement, const QString &name,
                                                      const QLatin1String &value) const
{
    domElement.setAttribute(name, value);
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VDomDocument::SetAttribute<QChar>(QDomElement &domElement, const QString &name, const QChar &value) const
{
    domElement.setAttribute(name, value);
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VDomDocument::SetAttribute<bool>(QDomElement &domElement, const QString &name, const bool &value) const
{
    domElement.setAttribute(name, value ? trueStr : falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline void VDomDocument::SetAttribute<MeasurementsType>(QDomElement &domElement, const QString &name,
                                                         const MeasurementsType &value) const
{
    domElement.setAttribute(name, value == MeasurementsType::Multisize ? QStringLiteral("multisize")
                                                                       : QStringLiteral("individual"));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void VDomDocument::SetAttributeOrRemoveIf(QDomElement &domElement, const QString &name, const T &value,
                                                 const std::function<bool(const T &)> &removeCondition) const
{
    not removeCondition(value) ? SetAttribute(domElement, name, value) : domElement.removeAttribute(name);
}

QT_WARNING_POP

#endif // VDOMDOCUMENT_H
