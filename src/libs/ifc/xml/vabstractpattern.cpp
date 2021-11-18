/************************************************************************
 **
 **  @file   vabstractpattern.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 6, 2015
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

#include "vabstractpattern.h"

#include <QDomNode>
#include <QDomNodeList>
#include <QLatin1String>
#include <QList>
#include <QMessageLogger>
#include <QSet>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QtDebug>
#include <QtConcurrentMap>
#include <QFuture>
#include <QtConcurrentRun>

#include "../exception/vexceptionemptyparameter.h"
#include "../exception/vexceptionobjecterror.h"
#include "../exception/vexceptionconversionerror.h"
#include "../qmuparser/qmutokenparser.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/ifcdef.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vtools/tools/vdatatool.h"
#include "vpatternconverter.h"
#include "vdomdocument.h"
#include "vtoolrecord.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/compatibility.h"
#include "../vlayout/vtextmanager.h"

class QDomElement;

const QString VAbstractPattern::TagPattern          = QStringLiteral("pattern");
const QString VAbstractPattern::TagCalculation      = QStringLiteral("calculation");
const QString VAbstractPattern::TagModeling         = QStringLiteral("modeling");
const QString VAbstractPattern::TagDetails          = QStringLiteral("details");
const QString VAbstractPattern::TagDetail           = QStringLiteral("detail");
const QString VAbstractPattern::TagDescription      = QStringLiteral("description");
const QString VAbstractPattern::TagNotes            = QStringLiteral("notes");
const QString VAbstractPattern::TagImage            = QStringLiteral("image");
const QString VAbstractPattern::TagMeasurements     = QStringLiteral("measurements");
const QString VAbstractPattern::TagIncrements       = QStringLiteral("increments");
const QString VAbstractPattern::TagPreviewCalculations = QStringLiteral("previewCalculations");
const QString VAbstractPattern::TagIncrement        = QStringLiteral("increment");
const QString VAbstractPattern::TagDraw             = QStringLiteral("draw");
const QString VAbstractPattern::TagGroups           = QStringLiteral("groups");
const QString VAbstractPattern::TagGroup            = QStringLiteral("group");
const QString VAbstractPattern::TagGroupItem        = QStringLiteral("item");
const QString VAbstractPattern::TagPoint            = QStringLiteral("point");
const QString VAbstractPattern::TagSpline           = QStringLiteral("spline");
const QString VAbstractPattern::TagArc              = QStringLiteral("arc");
const QString VAbstractPattern::TagElArc            = QStringLiteral("elArc");
const QString VAbstractPattern::TagTools            = QStringLiteral("tools");
const QString VAbstractPattern::TagOperation        = QStringLiteral("operation");
const QString VAbstractPattern::TagData             = QStringLiteral("data");
const QString VAbstractPattern::TagPatternInfo      = QStringLiteral("patternInfo");
const QString VAbstractPattern::TagPatternName      = QStringLiteral("patternName");
const QString VAbstractPattern::TagPatternNum       = QStringLiteral("patternNumber");
const QString VAbstractPattern::TagCustomerName     = QStringLiteral("customer");
const QString VAbstractPattern::TagCustomerBirthDate = QStringLiteral("birthDate");
const QString VAbstractPattern::TagCustomerEmail     = QStringLiteral("email");
const QString VAbstractPattern::TagCompanyName      = QStringLiteral("company");
const QString VAbstractPattern::TagPatternLabel     = QStringLiteral("patternLabel");
const QString VAbstractPattern::TagWatermark        = QStringLiteral("watermark");
const QString VAbstractPattern::TagPatternMaterials = QStringLiteral("patternMaterials");
const QString VAbstractPattern::TagFinalMeasurements= QStringLiteral("finalMeasurements");
const QString VAbstractPattern::TagMaterial         = QStringLiteral("material");
const QString VAbstractPattern::TagFMeasurement     = QStringLiteral("finalMeasurment");
const QString VAbstractPattern::TagGrainline        = QStringLiteral("grainline");
const QString VAbstractPattern::TagPath             = QStringLiteral("path");
const QString VAbstractPattern::TagNodes            = QStringLiteral("nodes");
const QString VAbstractPattern::TagNode             = QStringLiteral("node");

const QString VAbstractPattern::AttrName              = QStringLiteral("name");
const QString VAbstractPattern::AttrVisible           = QStringLiteral("visible");
const QString VAbstractPattern::AttrObject            = QStringLiteral("object");
const QString VAbstractPattern::AttrTool              = QStringLiteral("tool");
const QString VAbstractPattern::AttrType              = QStringLiteral("type");
const QString VAbstractPattern::AttrLetter            = QStringLiteral("letter");
const QString VAbstractPattern::AttrAnnotation        = QStringLiteral("annotation");
const QString VAbstractPattern::AttrOrientation       = QStringLiteral("orientation");
const QString VAbstractPattern::AttrRotationWay       = QStringLiteral("rotationWay");
const QString VAbstractPattern::AttrTilt              = QStringLiteral("tilt");
const QString VAbstractPattern::AttrFoldPosition      = QStringLiteral("foldPosition");
const QString VAbstractPattern::AttrQuantity          = QStringLiteral("quantity");
const QString VAbstractPattern::AttrOnFold            = QStringLiteral("onFold");
const QString VAbstractPattern::AttrDateFormat        = QStringLiteral("dateFormat");
const QString VAbstractPattern::AttrTimeFormat        = QStringLiteral("timeFormat");
const QString VAbstractPattern::AttrArrows            = QStringLiteral("arrows");
const QString VAbstractPattern::AttrNodeReverse       = QStringLiteral("reverse");
const QString VAbstractPattern::AttrNodeExcluded      = QStringLiteral("excluded");
const QString VAbstractPattern::AttrNodePassmark      = QStringLiteral("passmark");
const QString VAbstractPattern::AttrNodePassmarkLine  = QStringLiteral("passmarkLine");
const QString VAbstractPattern::AttrNodePassmarkAngle = QStringLiteral("passmarkAngle");
const QString VAbstractPattern::AttrNodeShowSecondPassmark = QStringLiteral("showSecondPassmark");
const QString VAbstractPattern::AttrSABefore          = QStringLiteral("before");
const QString VAbstractPattern::AttrSAAfter           = QStringLiteral("after");
const QString VAbstractPattern::AttrStart             = QStringLiteral("start");
const QString VAbstractPattern::AttrPath              = QStringLiteral("path");
const QString VAbstractPattern::AttrEnd               = QStringLiteral("end");
const QString VAbstractPattern::AttrIncludeAs         = QStringLiteral("includeAs");
const QString VAbstractPattern::AttrRotation          = QStringLiteral("rotation");
const QString VAbstractPattern::AttrNumber            = QStringLiteral("number");
const QString VAbstractPattern::AttrCheckUniqueness   = QStringLiteral("checkUniqueness");
const QString VAbstractPattern::AttrManualPassmarkLength = QStringLiteral("manualPassmarkLength");
const QString VAbstractPattern::AttrPassmarkLength    = QStringLiteral("passmarkLength");
const QString VAbstractPattern::AttrOpacity           = QStringLiteral("opacity");
const QString VAbstractPattern::AttrTags              = QStringLiteral("tags");

const QString VAbstractPattern::AttrExtension       = QStringLiteral("extension");

const QString VAbstractPattern::AttrFormula     = QStringLiteral("formula");
const QString VAbstractPattern::AttrDescription = QStringLiteral("description");

const QString VAbstractPattern::NodeArc        = QStringLiteral("NodeArc");
const QString VAbstractPattern::NodeElArc      = QStringLiteral("NodeElArc");
const QString VAbstractPattern::NodePoint      = QStringLiteral("NodePoint");
const QString VAbstractPattern::NodeSpline     = QStringLiteral("NodeSpline");
const QString VAbstractPattern::NodeSplinePath = QStringLiteral("NodeSplinePath");

QHash<quint32, VDataTool*> VAbstractPattern::tools = QHash<quint32, VDataTool*>();
QVector<VLabelTemplateLine> VAbstractPattern::patternLabelLines = QVector<VLabelTemplateLine>();
QMap<int, QString> VAbstractPattern::patternMaterials = QMap<int, QString>();
bool VAbstractPattern::patternLabelWasChanged = false;

namespace
{
void ReadExpressionAttribute(QVector<VFormulaField> &expressions, const QDomElement &element, const QString &attribute)
{
    VFormulaField formula;
    formula.expression = VDomDocument::GetParametrEmptyString(element, attribute);

    if (formula.expression.isEmpty())
    {
        return;
    }

    formula.element = element;
    formula.attribute = attribute;

    expressions.append(formula);
}

//---------------------------------------------------------------------------------------------------------------------
QList<QString> GetTokens(const VFormulaField &formula)
{
    try
    {
        QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula.expression, false, false));
        return cal->GetTokens().values();
    }
    catch (const qmu::QmuParserError &e)
    {
        qWarning() << QObject::tr("Cannot get tokens from formula '%1'. Parser error: %2.")
                      .arg(formula.expression, e.GetMsg());
        return QList<QString>();
    }
    catch (const qmu::QmuParserWarning &e)
    {
        qWarning() << QObject::tr("Cannot get tokens from formula '%1'. Formula error: %2.")
                          .arg(formula.expression, e.GetMsg());
        return QList<QString>();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void GatherTokens(QSet<QString> &tokens, const QList<QString> &tokenList)
{
    tokens = tokens.unite(ConvertToSet(tokenList));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AdjustMaterials help function that combine user materials from pattern and cli.
 * @param materials materials from pattern
 * @return combined list
 */
QMap<int, QString> AdjustMaterials(QMap<int, QString> materials)
{
    const QMap<int, QString> cliMaterials = VAbstractValApplication::VApp()->GetUserMaterials();
    QMap<int, QString>::const_iterator i = cliMaterials.constBegin();
    while (i != cliMaterials.constEnd())
    {
        if (not materials.contains(i.key()))
        {
            qWarning() << QObject::tr("User material number %1 was not defined in this pattern.").arg(i.key());
        }

        materials.insert(i.key(), i.value());
        ++i;
    }

    return materials;
}

//---------------------------------------------------------------------------------------------------------------------
QString PrepareGroupTags(QStringList tags)
{
    for (auto &tag : tags)
    {
        tag = tag.simplified();
    }

    return ConvertToList(ConvertToSet<QString>(tags)).join(',');
}
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractPattern::VAbstractPattern(QObject *parent)
    : VDomDocument(parent),
      nameActivPP(),
      cursor(0),
      toolsOnRemove(QVector<VDataTool*>()),
      history(QVector<VToolRecord>()),
      patternPieces(),
      modified(false)
{}

//---------------------------------------------------------------------------------------------------------------------
VAbstractPattern::~VAbstractPattern()
{
    qDeleteAll(toolsOnRemove);
    toolsOnRemove.clear();
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::RequiresMeasurements() const
{
    return not ListMeasurements().isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::ListMeasurements() const
{
    const QFuture<QStringList> futureIncrements = QtConcurrent::run(this, &VAbstractPattern::ListIncrements);
    const QList<QString> tokens = ConvertToList(QtConcurrent::blockingMappedReduced(ListExpressions(), GetTokens,
                                                                                    GatherTokens));

    QSet<QString> measurements;
    QSet<QString> others = ConvertToSet<QString>(futureIncrements.result());

    for (const auto &token : tokens)
    {
        if (token == QChar('-') || measurements.contains(token) || others.contains(token))
        {
            continue;
        }

        IsVariable(token) || IsFunction(token) ? others.insert(token) : measurements.insert(token);
    }

    return QStringList(measurements.values());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangeActivPP set new active pattern piece name.
 * @param name new name.
 * @param parse parser file mode.
 */
void VAbstractPattern::ChangeActivPP(const QString &name, const Document &parse)
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name pattern piece is empty");
    if (CheckExistNamePP(name))
    {
        this->nameActivPP = name;
        if (parse == Document::FullParse)
        {
            emit ChangedActivPP(name);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetActivDrawElement return draw tag for current pattern peace.
 * @param element draw tag.
 * @return true if found.
 */
bool VAbstractPattern::GetActivDrawElement(QDomElement &element) const
{
    if (nameActivPP.isEmpty() == false)
    {
        const QDomNodeList elements = this->documentElement().elementsByTagName( TagDraw );
        if (elements.size() == 0)
        {
            return false;
        }
        for ( qint32 i = 0; i < elements.count(); i++ )
        {
            element = elements.at( i ).toElement();
            if (element.isNull() == false)
            {
                const QString fieldName = element.attribute( AttrName );
                if ( fieldName == nameActivPP )
                {
                    return true;
                }
            }
        }
        element = QDomElement();
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VToolRecord> VAbstractPattern::getLocalHistory(const QString &draw) const
{
    QVector<VToolRecord> historyPP;
    for (qint32 i = 0; i< history.size(); ++i)
    {
        const VToolRecord &tool = history.at(i);
        if (tool.getNameDraw() == draw)
        {
            historyPP.append(tool);
        }
    }
    return historyPP;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckNameDraw check if exist pattern peace with this name.
 * @param name pattern peace name.
 * @return true if exist.
 */
bool VAbstractPattern::CheckExistNamePP(const QString &name) const
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name draw is empty");
    const QDomNodeList elements = this->documentElement().elementsByTagName( TagDraw );
    if (elements.size() == 0)
    {
        return false;
    }
    for ( qint32 i = 0; i < elements.count(); i++ )
    {
        const QDomElement elem = elements.at( i ).toElement();
        if (elem.isNull() == false)
        {
            if ( GetParametrString(elem, AttrName) == name )
            {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetActivNodeElement find element in current pattern piece by name.
 * @param name name tag.
 * @param element element.
 * @return true if found.
 */
bool VAbstractPattern::GetActivNodeElement(const QString &name, QDomElement &element) const
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name draw is empty");
    QDomElement drawElement;
    if (GetActivDrawElement(drawElement))
    {
        const QDomNodeList listElement = drawElement.elementsByTagName(name);
        if (listElement.size() != 1)
        {
            return false;
        }
        element = listElement.at( 0 ).toElement();
        if (element.isNull() == false)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::ParseGroups(const QDomElement &domElement)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    QMap<quint32, quint32> itemTool;
    QMap<quint32, bool> itemVisibility;

    QDomNode domNode = domElement.firstChild();
    while (not domNode.isNull())
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (not domElement.isNull())
            {
                if (domElement.tagName() == TagGroup)
                {
                    VContainer::UpdateId(GetParametrUInt(domElement, AttrId, NULL_ID_STR), valentinaNamespace);

                    const QPair<bool, QMap<quint32, quint32> > groupData = ParseItemElement(domElement);
                    const QMap<quint32, quint32> group = groupData.second;
                    auto i = group.constBegin();
                    while (i != group.constEnd())
                    {
                        if (not itemTool.contains(i.key()))
                        {
                            itemTool.insert(i.key(), i.value());
                        }

                        const bool previous = itemVisibility.value(i.key(), false);
                        itemVisibility.insert(i.key(), previous || groupData.first);
                        ++i;
                    }
                }
            }
        }
        domNode = domNode.nextSibling();
    }

    auto i = itemTool.constBegin();
    while (i != itemTool.constEnd())
    {
        if (tools.contains(i.value()))
        {
            VDataTool* tool = tools.value(i.value());
            tool->GroupVisibility(i.key(), itemVisibility.value(i.key(), true));
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VAbstractPattern::CountPP() const
{
    const QDomElement rootElement = this->documentElement();
    if (rootElement.isNull())
    {
        return 0;
    }

    return rootElement.elementsByTagName( TagDraw ).count();
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::GetPPElement(const QString &name)
{
    if (not name.isEmpty())
    {
        const QDomNodeList elements = this->documentElement().elementsByTagName( TagDraw );
        if (elements.size() == 0)
        {
            return QDomElement();
        }

        for ( qint32 i = 0; i < elements.count(); i++ )
        {
            QDomElement element = elements.at(i).toElement();
            if (not element.isNull())
            {
                if (element.attribute(AttrName) == name)
                {
                    return element;
                }
            }
        }
    }
    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangeNamePP change pattern piece name.
 * @param oldName old pattern piece name.
 * @param newName new pattern piece name.
 * @return true if success.
 */
bool VAbstractPattern::ChangeNamePP(const QString &oldName, const QString &newName)
{
    Q_ASSERT_X(not newName.isEmpty(), Q_FUNC_INFO, "new name pattern piece is empty");
    Q_ASSERT_X(not oldName.isEmpty(), Q_FUNC_INFO, "old name pattern piece is empty");

    if (CheckExistNamePP(oldName) == false)
    {
        qDebug()<<"Do not exist pattern piece with name"<<oldName;
        return false;
    }

    if (CheckExistNamePP(newName))
    {
        qDebug()<<"Already exist pattern piece with name"<<newName;
        return false;
    }

    QDomElement ppElement = GetPPElement(oldName);
    if (ppElement.isElement())
    {
        if (nameActivPP == oldName)
        {
            nameActivPP = newName;
        }
        ppElement.setAttribute(AttrName, newName);
        emit patternChanged(false);//For situation when we change name directly, without undocommands.
        emit ChangedNameDraw(oldName, newName);
        return true;
    }
    else
    {
        qDebug()<<"Can't find pattern piece node with name"<<oldName<<Q_FUNC_INFO;
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief appendPP add new pattern piece.
 *
 * Method check if not exist pattern piece with the same name and change name active pattern piece name, send signal
 * about change pattern piece. Doen't add pattern piece to file structure. This task make SPoint tool.
 * @param name pattern peace name.
 * @return true if success.
 */
bool VAbstractPattern::appendPP(const QString &name)
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name pattern piece is empty");
    if (name.isEmpty())
    {
        return false;
    }
    if (CheckExistNamePP(name) == false)
    {
        SetActivPP(name);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VAbstractPattern::getCursor() const
{
    return cursor;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setCursor(const quint32 &value)
{
    if (cursor != value)
    {
        cursor = value;
        emit ChangedCursor(cursor);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setXMLContent(const QString &fileName)
{
    Clear();
    VDomDocument::setXMLContent(fileName);
    m_patternNumber = ReadPatternNumber();
    m_labelDateFormat = ReadLabelDateFormat();
    m_patternName = ReadPatternName();
    m_MPath = ReadMPath();
    m_watermarkPath = ReadWatermarkPath();
    m_companyName = ReadCompanyName();
    m_units = ReadUnits();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::Clear()
{
    clear();
    m_patternNumber.clear();
    m_labelDateFormat.clear();
    m_patternName.clear();
    m_MPath.clear();
    m_watermarkPath.clear();
    m_companyName.clear();
    m_units = Unit::LAST_UNIT_DO_NOT_USE;
    modified = false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getTool return tool from tool list.
 * @param id tool id.
 * @return tool.
 */
VDataTool *VAbstractPattern::getTool(quint32 id)
{
    ToolExists(id);
    return tools.value(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddTool add tool to list tools.
 * @param id tool id.
 * @param tool tool.
 */
void VAbstractPattern::AddTool(quint32 id, VDataTool *tool)
{
    Q_ASSERT_X(id != 0, Q_FUNC_INFO, "id == 0");
    SCASSERT(tool != nullptr)
            tools.insert(id, tool);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::RemoveTool(quint32 id)
{
    tools.remove(id);
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath VAbstractPattern::ParsePieceNodes(const QDomElement &domElement)
{
    VPiecePath path;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            path.Append(ParseSANode(element));
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> VAbstractPattern::ParsePieceCSARecords(const QDomElement &domElement)
{
    QVector<CustomSARecord> records;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            CustomSARecord record;
            record.startPoint = GetParametrUInt(element, VAbstractPattern::AttrStart, NULL_ID_STR);
            record.path = GetParametrUInt(element, VAbstractPattern::AttrPath, NULL_ID_STR);
            record.endPoint = GetParametrUInt(element, VAbstractPattern::AttrEnd, NULL_ID_STR);
            record.reverse = GetParametrBool(element, VAbstractPattern::AttrNodeReverse, falseStr);
            record.includeType = static_cast<PiecePathIncludeType>(GetParametrUInt(element,
                                                                                   VAbstractPattern::AttrIncludeAs,
                                                                                   QChar('1')));
            records.append(record);
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VAbstractPattern::ParsePieceInternalPaths(const QDomElement &domElement)
{
    QVector<quint32> records;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            const quint32 path = GetParametrUInt(element, VAbstractPattern::AttrPath, NULL_ID_STR);
            if (path > NULL_ID)
            {
                records.append(path);
            }
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VAbstractPattern::ParsePiecePointRecords(const QDomElement &domElement)
{
    QVector<quint32> records;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            const quint32 path = element.text().toUInt();
            if (path > NULL_ID)
            {
                records.append(path);
            }
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode VAbstractPattern::ParseSANode(const QDomElement &domElement)
{
    const quint32 id = VDomDocument::GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
    const bool reverse = VDomDocument::GetParametrUInt(domElement, VAbstractPattern::AttrNodeReverse, QChar('0'));
    const bool excluded = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrNodeExcluded, falseStr);
    const bool uniqeness = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrCheckUniqueness, trueStr);
    const QString saBefore = VDomDocument::GetParametrString(domElement, VAbstractPattern::AttrSABefore,
                                                             currentSeamAllowance);
    const QString saAfter = VDomDocument::GetParametrString(domElement, VAbstractPattern::AttrSAAfter,
                                                            currentSeamAllowance);
    const PieceNodeAngle angle = static_cast<PieceNodeAngle>(VDomDocument::GetParametrUInt(domElement, AttrAngle,
                                                                                           QChar('0')));

    const bool passmark = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrNodePassmark, falseStr);
    const PassmarkLineType passmarkLine = StringToPassmarkLineType(VDomDocument::GetParametrString(domElement,
                                                                                 VAbstractPattern::AttrNodePassmarkLine,
                                                                                                   strOne));
    const PassmarkAngleType passmarkAngle = StringToPassmarkAngleType(VDomDocument::GetParametrString(domElement,
                                                                                VAbstractPattern::AttrNodePassmarkAngle,
                                                                                                   strStraightforward));

    const bool showSecond = VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrNodeShowSecondPassmark,
                                                          trueStr);
    const bool manualPassmarkLength =
            VDomDocument::GetParametrBool(domElement, VAbstractPattern::AttrManualPassmarkLength, falseStr);
    const QString passmarkLength =
            VDomDocument::GetParametrEmptyString(domElement, VAbstractPattern::AttrPassmarkLength);

    const QString t = VDomDocument::GetParametrString(domElement, AttrType, VAbstractPattern::NodePoint);
    Tool tool;

    const QStringList types
    {
        VAbstractPattern::NodePoint,
        VAbstractPattern::NodeArc,
        VAbstractPattern::NodeSpline,
        VAbstractPattern::NodeSplinePath,
        VAbstractPattern::NodeElArc
    };

    switch (types.indexOf(t))
    {
        case 0: // VAbstractPattern::NodePoint
            tool = Tool::NodePoint;
            break;
        case 1: // VAbstractPattern::NodeArc
            tool = Tool::NodeArc;
            break;
        case 2: // VAbstractPattern::NodeSpline
            tool = Tool::NodeSpline;
            break;
        case 3: // VAbstractPattern::NodeSplinePath
            tool = Tool::NodeSplinePath;
            break;
        case 4: // NodeElArc
            tool = Tool::NodeElArc;
            break;
        default:
            VException e(QObject::tr("Wrong tag name '%1'.").arg(t));
            throw e;
    }
    VPieceNode node(id, tool, reverse);
    node.SetFormulaSABefore(saBefore);
    node.SetFormulaSAAfter(saAfter);
    node.SetAngleType(angle);
    node.SetExcluded(excluded);
    node.SetCheckUniqueness(uniqeness);
    node.SetShowSecondPassmark(showSecond);
    node.SetPassmark(passmark);
    node.SetPassmarkLineType(passmarkLine);
    node.SetPassmarkAngleType(passmarkAngle);
    node.SetManualPassmarkLength(manualPassmarkLength);
    node.SetFormulaPassmarkLength(passmarkLength);

    return node;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::AddToolOnRemove(VDataTool *tool)
{
    SCASSERT(tool != nullptr)
    toolsOnRemove.append(tool);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getHistory return list with list of history records.
 * @return list of history records.
 */
QVector<VToolRecord> *VAbstractPattern::getHistory()
{
    return &history;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VToolRecord> VAbstractPattern::getLocalHistory() const
{
    return getLocalHistory(GetNameActivPP());
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::MPath() const
{
    return m_MPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetMPath(const QString &path)
{
    if (setTagText(TagMeasurements, path))
    {
        m_MPath = path;
        patternLabelWasChanged = true;
        modified = true;
        emit patternChanged(false);
    }
    else
    {
        qDebug()<<"Can't save path to measurements"<<Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VAbstractPattern::SiblingNodeId(const quint32 &nodeId) const
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55, "Check if need to ignore modeling tools.");

    quint32 siblingId = NULL_ID;

    const QVector<VToolRecord> history = getLocalHistory();
    for (qint32 i = 0; i < history.size(); ++i)
    {
        const VToolRecord tool = history.at(i);
        if (nodeId == tool.getId())
        {
            if (i == 0)
            {
                siblingId = NULL_ID;
            }
            else
            {
                for (qint32 j = i; j > 0; --j)
                {
                    const VToolRecord tool = history.at(j-1);
                    switch ( tool.getTypeTool() )
                    {
                        case Tool::Arrow:
                        case Tool::Piece:
                        case Tool::UnionDetails:
                        case Tool::NodeArc:
                        case Tool::NodeElArc:
                        case Tool::NodePoint:
                        case Tool::NodeSpline:
                        case Tool::NodeSplinePath:
                        case Tool::PlaceLabel:
                        case Tool::Pin:
                        case Tool::PiecePath:
                        case Tool::InsertNode:
                        case Tool::DuplicateDetail:
                            continue;
                        default:
                            siblingId = tool.getId();
                            j = 0;// break loop
                            break;
                    }
                }
            }
        }
    }
    return siblingId;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::getPatternPieces() const
{
    return patternPieces;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetDescription() const
{
    return UniqueTagText(TagDescription);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetDescription(const QString &text)
{
    CheckTagExists(TagDescription);
    setTagText(TagDescription, text);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetNotes() const
{
    return UniqueTagText(TagNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetNotes(const QString &text)
{
    CheckTagExists(TagNotes);
    setTagText(TagNotes, text);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetPatternName() const
{
    return m_patternName;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternName(const QString &qsName)
{
    m_patternName = qsName;
    CheckTagExists(TagPatternName);
    setTagText(TagPatternName, m_patternName);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetCompanyName() const
{
    return m_companyName;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetCompanyName(const QString& qsName)
{
    m_companyName = qsName;
    CheckTagExists(TagCompanyName);
    setTagText(TagCompanyName, m_companyName);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetPatternNumber() const
{
    return m_patternNumber;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternNumber(const QString& qsNum)
{
    m_patternNumber = qsNum;
    CheckTagExists(TagPatternNum);
    setTagText(TagPatternNum, m_patternNumber);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetCustomerName() const
{
    return UniqueTagText(TagCustomerName);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetCustomerName(const QString& qsName)
{
    CheckTagExists(TagCustomerName);
    setTagText(TagCustomerName, qsName);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QDate VAbstractPattern::GetCustomerBirthDate() const
{
    return QDate::fromString(UniqueTagText(TagCustomerBirthDate), "yyyy-MM-dd");
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetCustomerBirthDate(const QDate &date)
{
    CheckTagExists(TagCustomerBirthDate);
    setTagText(TagCustomerBirthDate, date.toString("yyyy-MM-dd"));
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetCustomerEmail() const
{
    return UniqueTagText(TagCustomerEmail);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetCustomerEmail(const QString &email)
{
    CheckTagExists(TagCustomerEmail);
    setTagText(TagCustomerEmail, email);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetLabelDateFormat() const
{
    return m_labelDateFormat;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetLabelDateFormat(const QString &format)
{
    m_labelDateFormat = format;
    QDomElement tag = CheckTagExists(TagPatternLabel);
    SetAttribute(tag, AttrDateFormat, m_labelDateFormat);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetLabelTimeFormat() const
{
    QString globalLabelTimeFormat = VAbstractApplication::VApp()->Settings()->GetLabelTimeFormat();

    const QDomNodeList list = elementsByTagName(TagPatternLabel);
    if (list.isEmpty())
    {
        return globalLabelTimeFormat;
    }

    QDomElement tag = list.at(0).toElement();
    return GetParametrString(tag, AttrTimeFormat, globalLabelTimeFormat);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetLabelTimeFormat(const QString &format)
{
    QDomElement tag = CheckTagExists(TagPatternLabel);
    SetAttribute(tag, AttrTimeFormat, format);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternLabelTemplate(const QVector<VLabelTemplateLine> &lines)
{
    QDomElement tag = CheckTagExists(TagPatternLabel);
    RemoveAllChildren(tag);
    SetLabelTemplate(tag, lines);
    patternLabelLines = lines;
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VLabelTemplateLine> VAbstractPattern::GetPatternLabelTemplate() const
{
    if (patternLabelLines.isEmpty())
    {
        const QDomNodeList list = elementsByTagName(TagPatternLabel);
        if (list.isEmpty() || list.at(0).childNodes().count() == 0)
        {
            return QVector<VLabelTemplateLine>();
        }

        patternLabelLines = GetLabelTemplate(list.at(0).toElement());
    }

    return patternLabelLines;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::SetWatermarkPath(const QString &path)
{
    QDomElement tag = CheckTagExists(TagWatermark);

    if (path.isEmpty())
    {
        QDomNode parent = tag.parentNode();
        parent.removeChild(tag);

        emit patternChanged(false);
        patternLabelWasChanged = true;
        m_watermarkPath = path;
        modified = true;
        return true;
    }
    else
    {
        if (setTagText(tag, path))
        {
            emit patternChanged(false);
            patternLabelWasChanged = true;
            m_watermarkPath = path;
            modified = true;
            return true;
        }
        else
        {
            qDebug() << "Can't save path to watermark" << Q_FUNC_INFO;
            return false;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetWatermarkPath() const
{
    return m_watermarkPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternMaterials(const QMap<int, QString> &materials)
{
    QDomElement tag = CheckTagExists(TagPatternMaterials);
    RemoveAllChildren(tag);
    SetMaterials(tag, materials);
    patternMaterials = materials;
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> VAbstractPattern::GetPatternMaterials() const
{
    if (patternMaterials.isEmpty())
    {
        const QDomNodeList list = elementsByTagName(TagPatternMaterials);
        if (list.isEmpty() || list.at(0).childNodes().count() == 0)
        {
            return QMap<int, QString>();
        }

        patternMaterials = GetMaterials(list.at(0).toElement());
    }

    return AdjustMaterials(patternMaterials);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFinalMeasurement> VAbstractPattern::GetFinalMeasurements() const
{
    const QDomNodeList list = elementsByTagName(TagFinalMeasurements);
    if (list.isEmpty() || list.at(0).childNodes().count() == 0)
    {
        return QVector<VFinalMeasurement>();
    }

    return GetFMeasurements(list.at(0).toElement());
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetFinalMeasurements(const QVector<VFinalMeasurement> &measurements)
{
    QDomElement tag = CheckTagExists(TagFinalMeasurements);
    RemoveAllChildren(tag);
    SetFMeasurements(tag, measurements);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternWasChanged(bool changed)
{
    patternLabelWasChanged = changed;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::GetPatternWasChanged() const
{
    return patternLabelWasChanged;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetPassmarkLengthVariable() const
{
    const QDomElement pattern = documentElement();

    if (pattern.isNull())
    {
        return {};
    }

    return GetParametrEmptyString(pattern, AttrPassmarkLength);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPassmarkLengthVariable(const QString &name)
{
    QDomElement pattern = documentElement();

    if (not pattern.isNull())
    {
        SetAttribute(pattern, AttrPassmarkLength, name);
        modified = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetImage() const
{
    return UniqueTagText(TagImage);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetImageExtension() const
{
    const QString defExt =  QStringLiteral("PNG");
    const QDomNodeList nodeList = this->elementsByTagName(TagImage);
    if (nodeList.isEmpty())
    {
        return defExt;
    }
    else
    {
        const QDomNode domNode = nodeList.at(0);
        if (domNode.isNull() == false && domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                const QString ext = domElement.attribute(AttrExtension, defExt);
                return ext;
            }
        }
    }
    return defExt;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetImage(const QString &text, const QString &extension)
{
    QDomElement imageElement = CheckTagExists(TagImage);
    setTagText(imageElement, text);
    CheckTagExists(TagImage).setAttribute(AttrExtension, extension);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::DeleteImage()
{
    QDomElement pattern = documentElement();
    pattern.removeChild(CheckTagExists(TagImage));
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetVersion() const
{
    return UniqueTagText(TagVersion, VPatternConverter::PatternMaxVerStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetVersion()
{
    setTagText(TagVersion, VPatternConverter::PatternMaxVerStr);
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief haveLiteChange we have unsaved change.
 */
void VAbstractPattern::haveLiteChange()
{
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::NeedFullParsing()
{
    emit UndoCommand();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::ClearScene()
{
    emit ClearMainWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::CheckInLayoutList()
{
    emit UpdateInLayoutList();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SelectedDetail(quint32 id)
{
    emit ShowDetail(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::UpdateVisiblityGroups()
{
    emit UpdateGroups();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::ToolExists(const quint32 &id)
{
    if (tools.contains(id) == false)
    {
        throw VExceptionBadId(tr("Can't find tool in table."), id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath VAbstractPattern::ParsePathNodes(const QDomElement &domElement)
{
    VPiecePath path;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull() && element.tagName() == VAbstractPattern::TagNode)
        {
            path.Append(ParseSANode(element));
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetActivPP set current pattern piece.
 * @param name pattern peace name.
 */
void VAbstractPattern::SetActivPP(const QString &name)
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name pattern piece is empty");
    this->nameActivPP = name;
    emit ChangedActivPP(name);
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::CheckTagExists(const QString &tag)
{
    const QDomNodeList list = elementsByTagName(tag);
    QDomElement element;
    if (list.isEmpty())
    {
        const QStringList tags
        {
            TagUnit, // 0
            TagImage, // 1
            TagDescription, // 2
            TagNotes, // 3
            TagPatternName, // 4
            TagPatternNum, // 5
            TagCompanyName, // 6
            TagCustomerName, // 7
            TagCustomerBirthDate, // 8
            TagCustomerEmail, // 9
            TagPatternLabel, // 10
            TagWatermark, // 11
            TagPatternMaterials, // 12
            TagFinalMeasurements // 13
        };

        switch (tags.indexOf(tag))
        {
            case 1: //TagImage
                element = createElement(TagImage);
                break;
            case 2: //TagDescription
                element = createElement(TagDescription);
                break;
            case 3: //TagNotes
                element = createElement(TagNotes);
                break;
            case 4: // TagPatternName
                element = createElement(TagPatternName);
                break;
            case 5: // TagPatternNum
                element = createElement(TagPatternNum);
                break;
            case 6: // TagCompanyName
                element = createElement(TagCompanyName);
                break;
            case 7: // TagCustomerName
                element = createElement(TagCustomerName);
                break;
            case 8: // TagCustomerBirthDate
                element = createElement(TagCustomerBirthDate);
                break;
            case 9: // TagCustomerEmail
                element = createElement(TagCustomerEmail);
                break;
            case 10: // TagPatternLabel
                element = createElement(TagPatternLabel);
                break;
            case 11: // TagWatermark
                element = createElement(TagWatermark);
                break;
            case 12: // TagPatternMaterials
                element = createElement(TagPatternMaterials);
                break;
            case 13: // TagFinalMeasurements
                element = createElement(TagFinalMeasurements);
                break;
            case 0: //TagUnit (Mandatory tag)
            default:
                return QDomElement();
        }
        InsertTag(tags, element);
        return element;
    }
    return list.at(0).toElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::InsertTag(const QStringList &tags, const QDomElement &element)
{
    QDomElement pattern = documentElement();
    for (int i = tags.indexOf(element.tagName())-1; i >= 0; --i)
    {
        const QDomNodeList list = elementsByTagName(tags.at(i));
        if (not list.isEmpty())
        {
            pattern.insertAfter(element, list.at(0));
            break;
        }
    }
    SetVersion();
}

//---------------------------------------------------------------------------------------------------------------------
int VAbstractPattern::GetIndexActivPP() const
{
    const QDomNodeList drawList = elementsByTagName(TagDraw);

    int index = 0;
    if (not drawList.isEmpty())
    {
        for (int i = 0; i < drawList.size(); ++i)
        {
            QDomElement node = drawList.at(i).toElement();
            if (node.attribute(AttrName) == nameActivPP)
            {
                index = i;
                break;
            }
        }
    }

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::ListIncrements() const
{
    QStringList increments;

    auto GetExpressions = [&increments, this](const QString &type)
    {
        const QDomNodeList list = elementsByTagName(type);
        for (int i=0; i < list.size(); ++i)
        {
            const QString name = GetParametrEmptyString(list.at(i).toElement(), AttrName);
            if (not name.isEmpty())
            {
                increments.append(name);
            }
        }
    };

    GetExpressions(TagIncrement);
    GetExpressions(TagPreviewCalculations);

    return increments;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListExpressions() const
{
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    // Note. Tool Union Details also contains formulas, but we don't use them for union and keep only to simplifying
    // working with nodes. Same code for saving reading.
    auto futurePointExpressions = QtConcurrent::run(this, &VAbstractPattern::ListPointExpressions);
    auto futureArcExpressions = QtConcurrent::run(this, &VAbstractPattern::ListArcExpressions);
    auto futureElArcExpressions = QtConcurrent::run(this, &VAbstractPattern::ListElArcExpressions);
    auto futureSplineExpressions = QtConcurrent::run(this, &VAbstractPattern::ListSplineExpressions);
    auto futureIncrementExpressions = QtConcurrent::run(this, &VAbstractPattern::ListIncrementExpressions);
    auto futureOperationExpressions = QtConcurrent::run(this, &VAbstractPattern::ListOperationExpressions);
    auto futurePathExpressions = QtConcurrent::run(this, &VAbstractPattern::ListPathExpressions);
    auto futurePieceExpressions = QtConcurrent::run(this, &VAbstractPattern::ListPieceExpressions);
    auto futureFinalMeasurementsExpressions = QtConcurrent::run(this,
                                                                &VAbstractPattern::ListFinalMeasurementsExpressions);

    QVector<VFormulaField> list;
    list << futurePointExpressions.result();
    list << futureArcExpressions.result();
    list << futureElArcExpressions.result();
    list << futureSplineExpressions.result();
    list << futureIncrementExpressions.result();
    list << futureOperationExpressions.result();
    list << futurePathExpressions.result();
    list << futurePieceExpressions.result();
    list << futureFinalMeasurementsExpressions.result();

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPointExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment a number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagPoint);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrLength);
        ReadExpressionAttribute(expressions, dom, AttrAngle);
        ReadExpressionAttribute(expressions, dom, AttrC1Radius);
        ReadExpressionAttribute(expressions, dom, AttrC2Radius);
        ReadExpressionAttribute(expressions, dom, AttrCRadius);
        ReadExpressionAttribute(expressions, dom, AttrRadius);
        ReadExpressionAttribute(expressions, dom, AttrWidth);
        ReadExpressionAttribute(expressions, dom, AttrHeight);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListArcExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagArc);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrAngle1);
        ReadExpressionAttribute(expressions, dom, AttrAngle2);
        ReadExpressionAttribute(expressions, dom, AttrRadius);
        ReadExpressionAttribute(expressions, dom, AttrLength);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListElArcExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagElArc);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrRadius1);
        ReadExpressionAttribute(expressions, dom, AttrRadius2);
        ReadExpressionAttribute(expressions, dom, AttrAngle1);
        ReadExpressionAttribute(expressions, dom, AttrAngle2);
        ReadExpressionAttribute(expressions, dom, AttrRotationAngle);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListSplineExpressions() const
{
    QVector<VFormulaField> expressions;
    expressions << ListPathPointExpressions();
    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPathPointExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(AttrPathPoint);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrKAsm1);
        ReadExpressionAttribute(expressions, dom, AttrKAsm2);
        ReadExpressionAttribute(expressions, dom, AttrAngle);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListIncrementExpressions() const
{
    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagIncrement);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        ReadExpressionAttribute(expressions, dom, AttrFormula);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListOperationExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagOperation);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrAngle);
        ReadExpressionAttribute(expressions, dom, AttrLength);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListNodesExpressions(const QDomElement &nodes) const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;

    const QDomNodeList nodeList = nodes.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull() && element.tagName() == VAbstractPattern::TagNode)
        {
            ReadExpressionAttribute(expressions, element, VAbstractPattern::AttrSABefore);
            ReadExpressionAttribute(expressions, element, VAbstractPattern::AttrSAAfter);
            ReadExpressionAttribute(expressions, element, VAbstractPattern::AttrPassmarkLength);
        }
    }
    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPathExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagPath);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();
        if (dom.isNull())
        {
            continue;
        }

        expressions << ListNodesExpressions(dom.firstChildElement(TagNodes));
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListGrainlineExpressions(const QDomElement &element) const
{
    QVector<VFormulaField> expressions;
    if (not element.isNull())
    {
        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, element, AttrRotation);
        ReadExpressionAttribute(expressions, element, AttrLength);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPieceExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 55);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagDetail);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();
        if (dom.isNull())
        {
            continue;
        }

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrWidth);

        expressions << ListNodesExpressions(dom.firstChildElement(TagNodes));
        expressions << ListGrainlineExpressions(dom.firstChildElement(TagGrainline));
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListFinalMeasurementsExpressions() const
{
    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagFMeasurement);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();
        if (dom.isNull())
        {
            continue;
        }

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrFormula);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::IsVariable(const QString &token) const
{
    for (auto &var : builInVariables)
    {
        if (token.indexOf( var ) == 0)
        {
            if (var == currentLength || var == currentSeamAllowance)
            {
                return token == var;
            }
            else
            {
                return true;
            }
        }
    }

    if (token.startsWith('#'))
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::IsFunction(const QString &token) const
{
    for (auto &fn : builInFunctions)
    {
        if (token.indexOf(fn) == 0)
        {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
QPair<bool, QMap<quint32, quint32> > VAbstractPattern::ParseItemElement(const QDomElement &domElement)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        const bool visible = GetParametrBool(domElement, AttrVisible, trueStr);

        QMap<quint32, quint32> items;

        const QDomNodeList nodeList = domElement.childNodes();
        const qint32 num = nodeList.size();
        for (qint32 i = 0; i < num; ++i)
        {
            const QDomElement element = nodeList.at(i).toElement();
            if (not element.isNull() && element.tagName() == TagGroupItem)
            {
                const quint32 tool = GetParametrUInt(element, AttrTool, NULL_ID_STR);
                const quint32 object = GetParametrUInt(element, AttrObject, QString::number(tool));

                items.insert(object, tool);
            }
        }

        QPair<bool, QMap<quint32, quint32> > group;
        group.first = visible;
        group.second = items;

        return group;
    }
    catch (const VExceptionBadId &e)
    {
        VExceptionObjectError excep(tr("Error creating or updating group"), domElement);
        excep.AddMoreInformation(e.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> VAbstractPattern::GetMaterials(const QDomElement &element) const
{
    QMap<int, QString> materials;

    if (not element.isNull())
    {
        QDomElement tagMaterial = element.firstChildElement();
        while (tagMaterial.isNull() == false)
        {
            if (tagMaterial.tagName() == TagMaterial)
            {
                const int number = static_cast<int>(GetParametrUInt(tagMaterial, AttrNumber, QChar('0')));
                const QString name = GetParametrEmptyString(tagMaterial, AttrName);

                if (number > 0 && number <= userMaterialPlaceholdersQuantity)
                {
                    materials.insert(number, name);
                }
            }
            tagMaterial = tagMaterial.nextSiblingElement(TagMaterial);
        }
    }

    return materials;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetMaterials(QDomElement &element, const QMap<int, QString> &materials)
{
    if (not element.isNull())
    {
        QMap<int, QString>::const_iterator i = materials.constBegin();
        while (i != materials.constEnd())
        {
            QDomElement tagMaterial = createElement(TagMaterial);

            SetAttribute(tagMaterial, AttrNumber, i.key());
            SetAttribute(tagMaterial, AttrName, i.value());

            element.appendChild(tagMaterial);
            ++i;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFinalMeasurement> VAbstractPattern::GetFMeasurements(const QDomElement &element) const
{
    QVector<VFinalMeasurement> measurements;

    if (not element.isNull())
    {
        QDomElement tagFMeasurement = element.firstChildElement();
        while (not tagFMeasurement.isNull())
        {
            if (tagFMeasurement.tagName() == TagFMeasurement)
            {
                VFinalMeasurement m;

                m.name = GetParametrString(tagFMeasurement, AttrName, tr("measurement"));
                m.formula = GetParametrString(tagFMeasurement, AttrFormula, QChar('0'));
                m.description = GetParametrEmptyString(tagFMeasurement, AttrDescription);

                measurements.append(m);
            }
            tagFMeasurement = tagFMeasurement.nextSiblingElement(TagFMeasurement);
        }
    }

    return measurements;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetFMeasurements(QDomElement &element, const QVector<VFinalMeasurement> &measurements)
{
    if (not element.isNull())
    {
        for (auto &m : measurements)
        {
            QDomElement tagFMeasurement = createElement(TagFMeasurement);

            SetAttribute(tagFMeasurement, AttrName, m.name);
            SetAttribute(tagFMeasurement, AttrFormula, m.formula);
            SetAttributeOrRemoveIf<QString>(tagFMeasurement, AttrDescription, m.description,
                                            [](const QString &description) noexcept {return description.isEmpty();});

            element.appendChild(tagFMeasurement);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsModified state of the document for cases that do not cover QUndoStack.
 * @return true if the document was modified without using QUndoStack.
 */
bool VAbstractPattern::IsModified() const
{
    return modified;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetModified(bool modified)
{
    this->modified = modified;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::GetDraw(const QString &name) const
{
    const QDomNodeList draws = documentElement().elementsByTagName(TagDraw);
    for (int i=0; i < draws.size(); ++i)
    {
        QDomElement draw = draws.at(i).toElement();
        if (draw.isNull())
        {
            continue;
        }

        if (draw.attribute(AttrName) == name)
        {
            return draw;
        }
    }
    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPattern::CreateGroups(const QString &patternPieceName) -> QDomElement
{
    QDomElement draw;

    if (patternPieceName.isEmpty())
    {
        if (not GetActivDrawElement(draw))
        {
            return QDomElement();
        }
    }
    else
    {
        draw = GetPPElement(patternPieceName);
        if (not draw.isElement())
        {
            return QDomElement();
        }
    }

    QDomElement groups = draw.firstChildElement(TagGroups);

    if (groups.isNull())
    {
        groups = createElement(TagGroups);
        draw.appendChild(groups);
    }

    return groups;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPattern::CreateGroup(quint32 id, const QString &name, const QStringList &tags,
                                   const QMap<quint32, quint32> &groupData, vidtype tool) -> QDomElement
{
    if (id == NULL_ID)
    {
        return QDomElement();
    }

    const QString preparedTags = PrepareGroupTags(tags);

    QDomElement group = createElement(TagGroup);
    SetAttribute(group, AttrId, id);
    SetAttribute(group, AttrName, name);
    SetAttribute(group, AttrVisible, true);
    SetAttributeOrRemoveIf<vidtype>(group, AttrTool, tool, [](vidtype tool) noexcept { return tool == null_id;});
    SetAttributeOrRemoveIf<QString>(group, AttrTags, preparedTags,
                                    [](const QString &preparedTags) noexcept {return preparedTags.isEmpty();});

    auto i = groupData.constBegin();
    while (i != groupData.constEnd())
    {
        QDomElement item = createElement(TagGroupItem);
        item.setAttribute(AttrTool, i.value());
        SetAttributeOrRemoveIf<vidtype>(item, AttrObject, i.key(), [i](vidtype object) noexcept
        {return object == i.value();});
        group.appendChild(item);
        ++i;
    }

    return group;
}
//---------------------------------------------------------------------------------------------------------------------
vidtype VAbstractPattern::GroupLinkedToTool(vidtype toolId) const
{
    const QDomNodeList groups = elementsByTagName(TagGroup);
    for (int i=0; i < groups.size(); ++i)
    {
        const QDomElement group = groups.at(i).toElement();
        if (not group.isNull() && group.hasAttribute(AttrTool))
        {
            const quint32 id = GetParametrUInt(group, AttrTool, NULL_ID_STR);

            if (toolId == id)
            {
                return GetParametrUInt(group, AttrId, NULL_ID_STR);
            }
        }
    }

    return null_id;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetGroupName(quint32 id)
{
    QString name = tr("New group");
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        name = GetParametrString(group, AttrName, name);

    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetGroupName(quint32 id, const QString &name)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        group.setAttribute(AttrName, name);
        modified = true;
        emit patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::GetGroupTags(vidtype id)
{
    QStringList tags;
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        tags = FilterGroupTags(GetParametrEmptyString(group, AttrTags));
    }

    return tags;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetGroupTags(quint32 id, const QStringList &tags)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        SetAttributeOrRemoveIf<QString>(group, AttrTags, tags.join(','),
                                        [](const QString &rawTags) noexcept {return rawTags.isEmpty();});
        modified = true;
        emit patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::GetGroupCategories() const
{
    QSet<QString> categories;

    const QDomNodeList groups = elementsByTagName(TagGroup);
    for (int i=0; i < groups.size(); ++i)
    {
        const QDomElement group = groups.at(i).toElement();
        if (not group.isNull() && group.hasAttribute(AttrTags))
        {
            QStringList groupTags = VAbstractPattern::FilterGroupTags(GetParametrEmptyString(group, AttrTags));
            categories.unite(ConvertToSet<QString>(groupTags));
        }
    }

    return ConvertToList(categories);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPattern::GetGroups(const QString &patternPieceName) -> QMap<quint32, VGroupData>
{
    QMap<quint32, VGroupData> data;

    try
    {
        QDomElement groups = CreateGroups(patternPieceName);
        if (not groups.isNull())
        {
            QDomNode domNode = groups.firstChild();
            while (not domNode.isNull())
            {
                if (domNode.isElement())
                {
                    const QDomElement group = domNode.toElement();
                    if (not group.isNull())
                    {
                        if (group.tagName() == TagGroup)
                        {
                            VGroupData groupData;
                            const quint32 id = GetParametrUInt(group, AttrId, QChar('0'));
                            groupData.visible = GetParametrBool(group, AttrVisible, trueStr);
                            groupData.name = GetParametrString(group, AttrName, tr("New group"));
                            groupData.tags = FilterGroupTags(GetParametrEmptyString(group, AttrTags));
                            groupData.tool = GetParametrUInt(group, AttrTool, NULL_ID_STR);

                            QVector<QPair<quint32, quint32>> items;

                            const QDomNodeList nodeList = group.childNodes();
                            const qint32 num = nodeList.size();
                            for (qint32 i = 0; i < num; ++i)
                            {
                                const QDomElement element = nodeList.at(i).toElement();
                                if (not element.isNull() && element.tagName() == TagGroupItem)
                                {
                                    const quint32 tool = GetParametrUInt(element, AttrTool, NULL_ID_STR);
                                    const quint32 object = GetParametrUInt(element, AttrObject, QString::number(tool));

                                    items.append(QPair<quint32, quint32>(object, tool));
                                }
                            }

                            groupData.items = items;

                            data.insert(id, groupData);
                        }
                    }
                }
                domNode = domNode.nextSibling();
            }
        }
        else
        {
            qDebug("Can't get tag Groups.");
        }
    }
    catch (const VExceptionConversionError &)
    {
        return QMap<quint32, VGroupData>();
    }

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the groups that contain or do not contain the item identified by the toolid and the objectid
 * @param toolId tool id
 * @param objectId object id
 * @param containItem |true if the groups have to contain the given item, false if they musn't contain the item
 * @return groups that contain or do not contain the item identified by the toolid and the objectid
 */
QMap<quint32, QString> VAbstractPattern::GetGroupsContainingItem(quint32 toolId, quint32 objectId, bool containItem)
{
    QMap<quint32, QString> data;

    if(objectId == 0)
    {
        objectId = toolId;
    }

    // TODO : order in alphabetical order

    QDomElement groups = CreateGroups();
    if (not groups.isNull())
    {
        QDomNode domNode = groups.firstChild();
        while (domNode.isNull() == false) // iterate through the groups
        {
            if (domNode.isElement())
            {
                const QDomElement group = domNode.toElement();
                if (group.isNull() == false)
                {
                    if (group.tagName() == TagGroup)
                    {
                        bool groupHasItem = GroupHasItem(group, toolId, objectId);
                        if((containItem && groupHasItem) || (not containItem && not groupHasItem))
                        {
                            const quint32 groupId = GetParametrUInt(group, AttrId, QChar('0'));
                            const QString name = GetParametrString(group, AttrName, tr("New group"));
                            data.insert(groupId, name);
                        }
                    }
                }
            }
            domNode = domNode.nextSibling();
        }
    }
    else
    {
        qDebug("Can't get tag Groups.");
    }

    return data;
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Checks if the given group has the item with the given toolId and objectId
 * @param groupDomElement group dom element
 * @param toolId tool id
 * @param objectId object id
 * @return true if the given group has the item with the given toolId and objectId
 */
bool VAbstractPattern::GroupHasItem(const QDomElement &groupDomElement, quint32 toolId, quint32 objectId)
{
    bool result = false;

    QDomNode itemNode = groupDomElement.firstChild();
    while (itemNode.isNull() == false) // iterate through the items of the group
    {
        if (itemNode.isElement())
        {
            const QDomElement item = itemNode.toElement();
            if (item.isNull() == false)
            {
                quint32 toolIdIterate = GetParametrUInt(item, AttrTool, QChar('0'));
                quint32 objectIdIterate = GetParametrUInt(item, AttrObject, QString::number(toolIdIterate));

                if(toolIdIterate == toolId && objectIdIterate == objectId)
                {
                    result = true;
                    break;
                }
            }
        }
        itemNode = itemNode.nextSibling();
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPattern::ReadUnits() const -> Unit
{
    Unit units = StrToUnits(UniqueTagText(TagUnit, unitCM));

    if (units == Unit::Px)
    {
        units = Unit::Cm;
    }

    return units;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::ReadPatternNumber() const
{
    return UniqueTagText(TagPatternNum);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::ReadLabelDateFormat() const
{
    const QString globalLabelDateFormat = VAbstractApplication::VApp()->Settings()->GetLabelDateFormat();

    const QDomNodeList list = elementsByTagName(TagPatternLabel);
    if (list.isEmpty())
    {
        return globalLabelDateFormat;
    }

    QString labelDateFormat;

    try
    {
        labelDateFormat = GetParametrString(list.at(0).toElement(), AttrDateFormat);
    }
    catch (const VExceptionEmptyParameter &)
    {
        return globalLabelDateFormat;
    }
    return labelDateFormat;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::ReadPatternName() const
{
    return UniqueTagText(TagPatternName);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::ReadMPath() const
{
    return UniqueTagText(TagMeasurements);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::ReadWatermarkPath() const
{
    return UniqueTagText(TagWatermark);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::ReadCompanyName() const
{
    return UniqueTagText(TagCompanyName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Adds an item to the given group with the given toolId and objectId
 * @param toolId tool id
 * @param objectId object id
 * @param groupId group id
 * @return group element
 */
QDomElement VAbstractPattern::AddItemToGroup(quint32 toolId, quint32 objectId, quint32 groupId)
{
    QDomElement group = elementById(groupId, TagGroup);

    if (not group.isNull())
    {
        if(objectId == 0)
        {
            objectId = toolId;
        }

        QDomElement item = createElement(TagGroupItem);
        item.setAttribute(AttrTool, toolId);
        SetAttributeOrRemoveIf<vidtype>(item, AttrObject, objectId, [toolId](vidtype object) noexcept
        {return object == toolId;});
        group.appendChild(item);

        // to signalised that the pattern was changed and need to be saved
        modified = true;
        emit patternChanged(false);

        // to update the group table of the gui
        emit UpdateGroups();

        // parse the groups to update the drawing, in case the item was added to an invisible group
        QDomElement groups = CreateGroups();
        if (not groups.isNull())
        {
            ParseGroups(groups);
        }

        return item;
    }

    qDebug() << "The group of id " << groupId << " doesn't exist";
    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Removes the item of given toolId and objectId from the group of given groupId
 * @param toolId tool id
 * @param objectId object id
 * @param groupId group id
 * @return item element or null element is none
 */
QDomElement VAbstractPattern::RemoveItemFromGroup(quint32 toolId, quint32 objectId, quint32 groupId)
{
    QDomElement group = elementById(groupId, TagGroup);

    if (not group.isNull())
    {
        if(objectId == 0)
        {
            objectId = toolId;
        }

        QDomNode itemNode = group.firstChild();
        while (itemNode.isNull() == false) // iterate through the items of the group
        {
            if (itemNode.isElement())
            {
                const QDomElement item = itemNode.toElement();
                if (not item.isNull())
                {
                    quint32 toolIdIterate= GetParametrUInt(item, AttrTool, QChar('0'));
                    quint32 objectIdIterate= GetParametrUInt(item, AttrObject, QString::number(toolIdIterate));

                    if(toolIdIterate == toolId && objectIdIterate == objectId)
                    {
                        group.removeChild(itemNode);

                        // to signalised that the pattern was changed and need to be saved
                        modified = true;
                        emit patternChanged(false);

                        // to update the group table of the gui
                        emit UpdateGroups();

                        // parse the groups to update the drawing, in case the item was removed from an invisible group
                        QDomElement groups = CreateGroups();
                        if (not groups.isNull())
                        {
                            ParseGroups(groups);
                        }

                        return item;
                    }
                }
            }
            itemNode = itemNode.nextSibling();
        }
    }
    else
    {
        qDebug() << "The group of id " << groupId << " doesn't exist";
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::GetGroupVisibility(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        return GetParametrBool(group, AttrVisible, trueStr);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::PieceDrawName(quint32 id)
{
    const QDomElement detail = elementById(id, VAbstractPattern::TagDetail);
    if (detail.isNull())
    {
        return QString();
    }

    const QDomElement draw = detail.parentNode().parentNode().toElement();
    if (draw.isNull() || not draw.hasAttribute(VAbstractPattern::AttrName))
    {
        return QString();
    }

    return  draw.attribute(VAbstractPattern::AttrName);
}

//---------------------------------------------------------------------------------------------------------------------
VContainer VAbstractPattern::GetCompleteData() const
{
    return VContainer(nullptr, nullptr, VContainer::UniqueNamespace());
}

//---------------------------------------------------------------------------------------------------------------------
VContainer VAbstractPattern::GetCompletePPData(const QString &name) const
{
    Q_UNUSED(name)
    return VContainer(nullptr, nullptr, VContainer::UniqueNamespace());
}

//---------------------------------------------------------------------------------------------------------------------
Unit VAbstractPattern::Units() const
{
    return m_units;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::FilterGroupTags(const QString &tags)
{
    if (tags.isEmpty())
    {
        return QStringList();
    }

    QStringList list = tags.split(',');
    for (auto &tag : list)
    {
        tag = tag.simplified();
    }

    list.removeAll("");

    return ConvertToList(ConvertToSet<QString>(list));
}
