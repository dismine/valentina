/************************************************************************
 **
 **  @file   vmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 7, 2015
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

#include "vmeasurements.h"

#include <QDate>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QGlobalStatic>
#include <QLatin1Char>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QSet>
#include <QUuid>
#include <QtDebug>
#include <qnumeric.h>

#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternimage.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../qmuparser/qmuparsererror.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/def.h"
#include "../vmisc/projectversion.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vcontainer.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString VMeasurements::TagVST = QStringLiteral("vst");
const QString VMeasurements::TagVIT = QStringLiteral("vit");
const QString VMeasurements::TagBodyMeasurements = QStringLiteral("body-measurements");
const QString VMeasurements::TagNotes = QStringLiteral("notes");
const QString VMeasurements::TagPersonal = QStringLiteral("personal");
const QString VMeasurements::TagCustomer = QStringLiteral("customer");
const QString VMeasurements::TagBirthDate = QStringLiteral("birth-date");
const QString VMeasurements::TagGender = QStringLiteral("gender");
const QString VMeasurements::TagPMSystem = QStringLiteral("pm_system");
const QString VMeasurements::TagEmail = QStringLiteral("email");
const QString VMeasurements::TagReadOnly = QStringLiteral("read-only");
const QString VMeasurements::TagMeasurement = QStringLiteral("m");
const QString VMeasurements::TagDimensions = QStringLiteral("dimensions");
const QString VMeasurements::TagDimension = QStringLiteral("dimension");
const QString VMeasurements::TagRestrictions = QStringLiteral("restrictions");
const QString VMeasurements::TagRestriction = QStringLiteral("restriction");
const QString VMeasurements::TagCorrections = QStringLiteral("corrections");
const QString VMeasurements::TagCorrection = QStringLiteral("correction");
const QString VMeasurements::TagLabels = QStringLiteral("labels");
const QString VMeasurements::TagLabel = QStringLiteral("label");
const QString VMeasurements::TagImage = QStringLiteral("image");

const QString VMeasurements::AttrBase = QStringLiteral("base");
const QString VMeasurements::AttrValue = QStringLiteral("value");
const QString VMeasurements::AttrShiftA = QStringLiteral("shiftA");
const QString VMeasurements::AttrShiftB = QStringLiteral("shiftB");
const QString VMeasurements::AttrShiftC = QStringLiteral("shiftC");
const QString VMeasurements::AttrCorrection = QStringLiteral("correction");
const QString VMeasurements::AttrCoordinates = QStringLiteral("coordinates");
const QString VMeasurements::AttrExclude = QStringLiteral("exclude");
const QString VMeasurements::AttrSpecialUnits = QStringLiteral("specialUnits");
const QString VMeasurements::AttrDescription = QStringLiteral("description");
const QString VMeasurements::AttrName = QStringLiteral("name");
const QString VMeasurements::AttrFullName = QStringLiteral("full_name");
const QString VMeasurements::AttrValueAlias = QStringLiteral("valueAlias");
const QString VMeasurements::AttrMin = QStringLiteral("min");
const QString VMeasurements::AttrMax = QStringLiteral("max");
const QString VMeasurements::AttrStep = QStringLiteral("step");
const QString VMeasurements::AttrMeasurement = QStringLiteral("measurement");
const QString VMeasurements::AttrFullCircumference = QStringLiteral("fullCircumference");
const QString VMeasurements::AttrLabel = QStringLiteral("label");
const QString VMeasurements::AttrDimension = QStringLiteral("dimension");
const QString VMeasurements::AttrCustomName = QStringLiteral("customName");
const QString VMeasurements::AttrContentType = QStringLiteral("contentType");

const QString VMeasurements::GenderMale = QStringLiteral("male");
const QString VMeasurements::GenderFemale = QStringLiteral("female");
const QString VMeasurements::GenderUnknown = QStringLiteral("unknown");

const QString VMeasurements::DimensionN = QStringLiteral("n");
const QString VMeasurements::DimensionX = QStringLiteral("x");
const QString VMeasurements::DimensionY = QStringLiteral("y");
const QString VMeasurements::DimensionW = QStringLiteral("w");
const QString VMeasurements::DimensionZ = QStringLiteral("z");

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, defBirthDate, ("1800-01-01"_L1)) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto FileComment() -> QString
{
    return u"Measurements created with Valentina v%1 (https://smart-pattern.com.ua/)."_s.arg(AppVersionStr());
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadCorrections(const QDomElement &mElement) -> QMap<QString, VMeasurementCorrection>
{
    if (mElement.isNull())
    {
        return {};
    }

    QDomElement const correctionsTag = mElement.firstChildElement(VMeasurements::TagCorrections);
    if (correctionsTag.isNull())
    {
        return {};
    }

    QMap<QString, VMeasurementCorrection> corrections;

    QDomNode correctionTag = correctionsTag.firstChild();
    while (not correctionTag.isNull())
    {
        if (correctionTag.isElement())
        {
            const QDomElement c = correctionTag.toElement();
            const QString hash = VDomDocument::GetParametrString(c, VMeasurements::AttrCoordinates);

            corrections.insert(hash,
                               {.correction = VDomDocument::GetParametrDouble(c,
                                                                              VMeasurements::AttrCorrection,
                                                                              QChar('0')),
                                .alias = VDomDocument::GetParametrEmptyString(c, AttrAlias)});
        }
        correctionTag = correctionTag.nextSibling();
    }

    return corrections;
}

//---------------------------------------------------------------------------------------------------------------------
auto EvalFormula(const VContainer *data, const QString &formula, bool *ok) -> qreal
{
    if (formula.isEmpty())
    {
        *ok = true;
        return 0;
    }

    try
    {
        QScopedPointer<Calculator> const cal(new Calculator());
        const qreal result = cal->EvalFormula(data->DataVariables(), formula);

        (qIsInf(result) || qIsNaN(result)) ? *ok = false : *ok = true;
        return result;
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e)
        *ok = false;
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadDimensionLabels(const QDomElement &dElement) -> DimesionLabels
{
    if (dElement.isNull())
    {
        return {};
    }

    QDomElement const labelsTag = dElement.firstChildElement(VMeasurements::TagLabels);
    if (labelsTag.isNull())
    {
        return {};
    }

    DimesionLabels labels;

    QDomNode labelTag = labelsTag.firstChild();
    while (not labelTag.isNull())
    {
        if (labelTag.isElement())
        {
            const QDomElement l = labelTag.toElement();
            const qreal value = VDomDocument::GetParametrDouble(l, VMeasurements::AttrValue, QChar('0'));
            const QString label = VDomDocument::GetParametrEmptyString(l, VMeasurements::AttrLabel);

            if (value > 0 && not label.isEmpty())
            {
                labels.insert(value, label);
            }
        }
        labelTag = labelTag.nextSibling();
    }

    return labels;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VMeasurements::VMeasurements(VContainer *data, QObject *parent)
  : VDomDocument(parent),
    data(data),
    type(MeasurementsType::Unknown)
{
    SCASSERT(data != nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurements::VMeasurements(Unit unit, VContainer *data, QObject *parent)
  : VDomDocument(parent),
    data(data),
    type(MeasurementsType::Individual)
{
    SCASSERT(data != nullptr)

    CreateEmptyIndividualFile(unit);
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurements::VMeasurements(Unit unit, const QVector<MeasurementDimension_p> &dimensions, VContainer *data,
                             QObject *parent)
  : VDomDocument(parent),
    data(data),
    type(MeasurementsType::Multisize)
{
    SCASSERT(data != nullptr)

    CreateEmptyMultisizeFile(unit, dimensions);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::setXMLContent(const QString &fileName)
{
    VDomDocument::setXMLContent(fileName);
    type = ReadType();
    m_units = ReadUnits();

    if (type == MeasurementsType::Multisize &&
        VVSTConverter::MeasurementMaxVer == GetFormatVersion(GetFormatVersionStr()))
    {
        m_dimensions = ReadDimensions();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::SaveDocument(const QString &fileName, QString &error) -> bool
{
    // Update comment with Valentina version
    if (QDomNode const commentNode = documentElement().firstChild(); commentNode.isComment())
    {
        QDomComment comment = commentNode.toComment();
        comment.setData(FileComment());
    }

    return VDomDocument::SaveDocument(fileName, error);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::AddEmpty(const QString &name, const QString &formula, bool specialUnits)
{
    QDomElement element = MakeEmpty(name, formula, MeasurementType::Measurement);

    SetAttributeOrRemoveIf<bool>(element, AttrSpecialUnits, specialUnits,
                                 [](bool special) noexcept { return not special; });

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::AddEmptyAfter(const QString &after, const QString &name, const QString &formula, bool specialUnits)
{
    QDomElement element = MakeEmpty(name, formula, MeasurementType::Measurement);

    SetAttributeOrRemoveIf<bool>(element, AttrSpecialUnits, specialUnits,
                                 [](bool special) noexcept { return not special; });

    const QDomElement sibling = FindM(after);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);

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
void VMeasurements::AddSeparator(const QString &name)
{
    const QDomElement element = MakeEmpty(name, QString(), MeasurementType::Separator);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::AddSeparatorAfter(const QString &after, const QString &name)
{
    const QDomElement element = MakeEmpty(name, QString(), MeasurementType::Separator);
    const QDomElement sibling = FindM(after);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);

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
void VMeasurements::Remove(const QString &name)
{
    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).removeChild(FindM(name));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveTop(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomNodeList mList = elementsByTagName(TagMeasurement);
    if (mList.size() >= 2)
    {
        const QDomNode top = mList.at(0);
        if (not top.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
            list.at(0).insertBefore(node, top);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveUp(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomElement prSibling = node.previousSiblingElement(TagMeasurement);
    if (not prSibling.isNull())
    {
        const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
        list.at(0).insertBefore(node, prSibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveDown(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomElement nextSibling = node.nextSiblingElement(TagMeasurement);
    if (not nextSibling.isNull())
    {
        const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
        list.at(0).insertAfter(node, nextSibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveBottom(const QString &name)
{
    const QDomElement node = FindM(name);
    if (node.isNull())
    {
        return;
    }

    const QDomNodeList mList = elementsByTagName(TagMeasurement);
    if (mList.size() >= 2)
    {
        const QDomNode bottom = mList.at(mList.size() - 1);
        if (not bottom.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
            list.at(0).insertAfter(node, bottom);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Units() const -> Unit
{
    return m_units;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::StoreNames(bool store)
{
    m_keepNames = store;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::ReadMeasurements(qreal baseA, qreal baseB, qreal baseC) const
{
    // For conversion values we must first calculate all data in measurement file's unit.
    // That's why we need two containers: one for converted values, second for real data.

    // Container for values in measurement file's unit
    QSharedPointer<VContainer> tempData;

    if (type == MeasurementsType::Individual)
    {
        tempData = QSharedPointer<VContainer>::create(data->GetTrVars(), data->GetPatternUnit(),
                                                      VContainer::UniqueNamespace());
    }

    const QDomNodeList list = elementsByTagName(TagMeasurement);
    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        const QString name = GetParametrString(dom, AttrName).simplified();
        const QString description = GetParametrEmptyString(dom, AttrDescription);
        const MeasurementType varType = StringToMeasurementType(GetParametrString(dom, AttrType, strTypeMeasurement));

        QSharedPointer<VMeasurement> meash;
        QSharedPointer<VMeasurement> tempMeash;

        if (varType != MeasurementType::Separator)
        {
            ReadMeasurement(dom, tempData, meash, tempMeash, i, baseA, baseB, baseC);
        }
        else
        {
            meash = QSharedPointer<VMeasurement>::create(static_cast<quint32>(i), name);
            meash->SetDescription(description);
        }

        if (m_keepNames)
        {
            if (not tempData.isNull() && not tempMeash.isNull())
            {
                tempData->AddUniqueVariable(tempMeash);
            }
            data->AddUniqueVariable(meash);
        }
        else
        {
            if (not tempData.isNull() && not tempMeash.isNull())
            {
                tempData->AddVariable(tempMeash);
            }
            data->AddVariable(meash);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::ClearForExport()
{
    const QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i = 0; i < list.size(); ++i)
    {
        if (QDomElement domElement = list.at(i).toElement(); not domElement.isNull())
        {
            if (Type() == MeasurementsType::Individual)
            {
                if (qmu::QmuTokenParser::IsSingle(domElement.attribute(AttrValue)))
                {
                    SetAttribute(domElement, AttrValue, QChar('0'));
                }
            }
            else if (Type() == MeasurementsType::Multisize)
            {
                SetAttribute(domElement, AttrBase, QChar('0'));
                domElement.removeAttribute(AttrShiftA);
                domElement.removeAttribute(AttrShiftB);
                domElement.removeAttribute(AttrShiftC);
                RemoveAllChildren(domElement);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Type() const -> MeasurementsType
{
    return type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionABase() const -> qreal
{
    if (type == MeasurementsType::Multisize)
    {
        if (const auto dimensions = Dimensions(); not dimensions.isEmpty())
        {
            return dimensions.first()->BaseValue();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionBBase() const -> qreal
{
    if (type == MeasurementsType::Multisize)
    {
        if (const auto dimensions = Dimensions(); not dimensions.isEmpty() && dimensions.size() >= 2)
        {
            return dimensions.values().at(1)->BaseValue();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionCBase() const -> qreal
{
    if (type == MeasurementsType::Multisize)
    {
        if (const auto dimensions = Dimensions(); not dimensions.isEmpty() && dimensions.size() == 3)
        {
            return dimensions.last()->BaseValue();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionAStep() const -> qreal
{
    if (type == MeasurementsType::Multisize)
    {
        if (const auto dimensions = Dimensions(); not dimensions.isEmpty())
        {
            return dimensions.first()->Step();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionBStep() const -> qreal
{
    if (type == MeasurementsType::Multisize)
    {
        if (const auto dimensions = Dimensions(); not dimensions.isEmpty() && dimensions.size() >= 2)
        {
            return dimensions.values().at(1)->Step();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionCStep() const -> qreal
{
    if (type == MeasurementsType::Multisize)
    {
        if (const auto dimensions = Dimensions(); not dimensions.isEmpty() && dimensions.size() == 3)
        {
            return dimensions.last()->Step();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Notes() const -> QString
{
    return UniqueTagText(TagNotes, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetNotes(const QString &text)
{
    if (not IsReadOnly())
    {
        setTagText(TagNotes, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Customer() const -> QString
{
    return UniqueTagText(TagCustomer, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetCustomer(const QString &text)
{
    if (not IsReadOnly())
    {
        setTagText(TagCustomer, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::BirthDate() const -> QDate
{
    return QDate::fromString(UniqueTagText(TagBirthDate, *defBirthDate), QStringLiteral("yyyy-MM-dd"));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetBirthDate(const QDate &date)
{
    if (not IsReadOnly())
    {
        setTagText(TagBirthDate, date.toString("yyyy-MM-dd"_L1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Gender() const -> GenderType
{
    return StrToGender(UniqueTagText(TagGender, GenderUnknown));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetGender(const GenderType &gender)
{
    if (not IsReadOnly())
    {
        setTagText(TagGender, GenderToStr(gender));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::KnownMeasurements() const -> QUuid
{
    return QUuid(UniqueTagText(TagPMSystem, QString()));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetKnownMeasurements(const QUuid &system)
{
    if (not IsReadOnly())
    {
        setTagText(TagPMSystem, system.toString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Email() const -> QString
{
    return UniqueTagText(TagEmail, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetEmail(const QString &text)
{
    if (not IsReadOnly())
    {
        setTagText(TagEmail, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::IsReadOnly() const -> bool
{
    return UniqueTagText(TagReadOnly, falseStr) == trueStr;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetReadOnly(bool ro)
{
    setTagText(TagReadOnly, ro ? trueStr : falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::IsFullCircumference() const -> bool
{
    if (QDomElement const dimenstionsTag = documentElement().firstChildElement(TagDimensions);
        not dimenstionsTag.isNull())
    {
        return GetParametrBool(dimenstionsTag, AttrFullCircumference, falseStr);
    }

    qDebug() << "Can't read full circumference " << Q_FUNC_INFO;

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetFullCircumference(bool fc)
{
    QDomElement dimenstionsTag = documentElement().firstChildElement(TagDimensions);
    if (not dimenstionsTag.isNull())
    {
        SetAttributeOrRemoveIf<bool>(dimenstionsTag, AttrFullCircumference, fc,
                                     [](bool fc) noexcept { return not fc; });
    }
    else
    {
        qDebug() << "Can't save full circumference " << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMName(const QString &name, const QString &text)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMValue(const QString &name, const QString &text)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrValue, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMBaseValue(const QString &name, double value)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrBase, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMShiftA(const QString &name, double value)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrShiftA, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMShiftB(const QString &name, double value)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrShiftB, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMShiftC(const QString &name, double value)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrShiftC, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMSpecialUnits(const QString &name, bool special)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttributeOrRemoveIf<bool>(node, AttrSpecialUnits, special,
                                     [](bool special) noexcept { return not special; });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMCorrectionValue(const QString &name, qreal baseA, qreal baseB, qreal baseC, double value)
{
    if (QDomElement mElement = FindM(name); not mElement.isNull())
    {
        QMap<QString, VMeasurementCorrection> corrections = ReadCorrections(mElement);
        const QString hash = VMeasurement::CorrectionHash(baseA, baseB, baseC);

        VMeasurementCorrection c = corrections.value(hash);
        c.correction = value;

        if (qFuzzyIsNull(c.correction) && c.alias.isEmpty())
        {
            corrections.remove(hash);
        }
        else
        {
            corrections.insert(hash, c);
        }

        WriteCorrections(mElement, corrections);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMValueAlias(const QString &name, qreal baseA, qreal baseB, qreal baseC, const QString &alias)
{
    if (QDomElement mElement = FindM(name); not mElement.isNull())
    {
        QMap<QString, VMeasurementCorrection> corrections = ReadCorrections(mElement);
        const QString hash = VMeasurement::CorrectionHash(baseA, baseB, baseC);

        VMeasurementCorrection c = corrections.value(hash);
        c.alias = alias;

        if (qFuzzyIsNull(c.correction) && c.alias.isEmpty())
        {
            corrections.remove(hash);
        }
        else
        {
            corrections.insert(hash, c);
        }

        WriteCorrections(mElement, corrections);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMValueAlias(const QString &name, const QString &alias) const
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttributeOrRemoveIf<QString>(node,
                                        AttrValueAlias,
                                        alias,
                                        [](const QString &str) noexcept { return str.isEmpty(); });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMDescription(const QString &name, const QString &text)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttributeOrRemoveIf<QString>(node, AttrDescription, text,
                                        [](const QString &text) noexcept { return text.isEmpty(); });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMFullName(const QString &name, const QString &text)
{
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttribute(node, AttrFullName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMDimension(const QString &name, IMD type)
{
    ClearDimension(type);
    if (QDomElement node = FindM(name); not node.isNull())
    {
        SetAttributeOrRemoveIf<QString>(node, AttrDimension, VMeasurements::IMDToStr(type),
                                        [](const QString &type) noexcept
                                        { return type == VMeasurements::IMDToStr(IMD::N); });
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMImage(const QString &name, const VPatternImage &image)
{
    if (QDomElement mElement = FindM(name); not mElement.isNull())
    {
        WriteImage(mElement, image);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::MeasurementForDimension(IMD type) const -> QString
{
    const QString d = VMeasurements::IMDToStr(type);
    QDomNodeList const list = elementsByTagName(TagMeasurement);

    for (int i = 0; i < list.size(); ++i)
    {
        if (const QDomElement domElement = list.at(i).toElement();
            !domElement.isNull() && domElement.attribute(AttrDimension) == d)
        {
            return domElement.attribute(AttrName);
        }
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Dimensions() const -> VDimensions
{
    return m_dimensions;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::GetRestrictions() const -> QMap<QString, VDimensionRestriction>
{
    QMap<QString, VDimensionRestriction> restrictions;

    const QDomNodeList list = elementsByTagName(TagRestriction);
    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement res = list.at(i).toElement();

        QString const coordinates = GetParametrString(res, AttrCoordinates);
        const qreal min = GetParametrDouble(res, AttrMin, QChar('0'));
        const qreal max = GetParametrDouble(res, AttrMax, QChar('0'));
        const QString exclude = GetParametrEmptyString(res, AttrExclude);

        restrictions.insert(coordinates, VDimensionRestriction(min, max, exclude));
    }

    return restrictions;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetRestrictions(const QMap<QString, VDimensionRestriction> &restrictions)
{
    QDomElement const root = documentElement();
    QDomElement restrictionsTag = root.firstChildElement(TagRestrictions);

    if (restrictionsTag.isNull())
    {
        qDebug() << "Can't find restrictions tag";
    }

    RemoveAllChildren(restrictionsTag);

    auto i = restrictions.constBegin();
    while (i != restrictions.constEnd())
    {
        QDomElement restrictionTag = createElement(TagRestriction);

        SetAttribute(restrictionTag, AttrCoordinates, i.key());
        SetAttributeOrRemoveIf<qreal>(restrictionTag, AttrMin, i.value().GetMin(),
                                      [](qreal min) noexcept { return qFuzzyIsNull(min); });
        SetAttributeOrRemoveIf<qreal>(restrictionTag, AttrMax, i.value().GetMax(),
                                      [](qreal max) noexcept { return qFuzzyIsNull(max); });
        SetAttributeOrRemoveIf<QString>(restrictionTag, AttrExclude, i.value().GetExcludeString(),
                                        [](const QString &exlcuded) noexcept { return exlcuded.isEmpty(); });

        restrictionsTag.appendChild(restrictionTag);
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::Restriction(qreal base, qreal base2) const -> VDimensionRestriction
{
    const QMap<QString, VDimensionRestriction> restrictions = GetRestrictions();
    const QString hash = VMeasurement::CorrectionHash(base, base2, 0);
    return restrictions.value(hash);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetDimensionLabels(const QMap<MeasurementDimension, DimesionLabels> &labels)
{
    const QDomNodeList list = elementsByTagName(TagDimension);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();
        const MeasurementDimension type = StrToDimensionType(GetParametrString(dom, AttrType));

        if (labels.contains(type))
        {
            SaveDimesionLabels(dom, labels.value(type));
        }
    }

    m_dimensions = ReadDimensions(); // Refresh cache
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetDimensionCustomNames(const QMap<MeasurementDimension, QString> &names)
{
    const QDomNodeList list = elementsByTagName(TagDimension);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();
        const MeasurementDimension type = StrToDimensionType(GetParametrString(dom, AttrType));

        if (names.contains(type))
        {
            SetAttributeOrRemoveIf<QString>(dom, AttrCustomName, names.value(type),
                                            [](const QString &name) noexcept { return name.isEmpty(); });
        }
    }

    m_dimensions = ReadDimensions(); // Refresh cache
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::GenderToStr(const GenderType &sex) -> QString
{
    switch (sex)
    {
        case GenderType::Male:
            return GenderMale;
        case GenderType::Female:
            return GenderFemale;
        case GenderType::Unknown:
        default:
            return GenderUnknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::StrToGender(const QString &sex) -> GenderType
{
    const auto genders = QStringList() << GenderMale << GenderFemale << GenderUnknown;
    switch (genders.indexOf(sex))
    {
        case 0: // GenderMale
            return GenderType::Male;
        case 1: // GenderFemale
            return GenderType::Female;
        case 2: // GenderUnknown
        default:
            return GenderType::Unknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::DimensionTypeToStr(const MeasurementDimension &type) -> QString
{
    switch (type)
    {
        case MeasurementDimension::Y:
            return DimensionY;
        case MeasurementDimension::W:
            return DimensionW;
        case MeasurementDimension::Z:
            return DimensionZ;
        case MeasurementDimension::X:
        default:
            return DimensionX;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::StrToDimensionType(const QString &type) -> MeasurementDimension
{
    const auto dimensions = QStringList{DimensionX, DimensionY, DimensionW, DimensionZ};
    switch (dimensions.indexOf(type))
    {
        case 1: // DimensionY
            return MeasurementDimension::Y;
        case 2: // DimensionW
            return MeasurementDimension::W;
        case 3: // DimensionZ
            return MeasurementDimension::Z;
        case 0: // DimensionX
        default:
            return MeasurementDimension::X;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::IMDToStr(const IMD &type) -> QString
{
    switch (type)
    {
        case IMD::X:
            return DimensionX;
        case IMD::Y:
            return DimensionY;
        case IMD::W:
            return DimensionW;
        case IMD::Z:
            return DimensionZ;
        case IMD::N:
        default:
            return DimensionN;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::StrToIMD(const QString &type) -> IMD
{
    const auto dimensions = QStringList{DimensionN, DimensionX, DimensionY, DimensionW, DimensionZ};
    switch (dimensions.indexOf(type))
    {
        case 1: // DimensionX
            return IMD::X;
        case 2: // DimensionY
            return IMD::Y;
        case 3: // DimensionW
            return IMD::W;
        case 4: // DimensionZ
            return IMD::Z;
        case 0: // DimensionN
        default:
            return IMD::N;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::IMDName(IMD type) -> QString
{
    switch (type)
    {
        case IMD::N:
            return tr("None");
        case IMD::X:
            return tr("Height") + " (X)"_L1;
        case IMD::Y:
            return tr("Size") + " (Y)"_L1;
        case IMD::W:
            return tr("Waist") + " (W)"_L1;
        case IMD::Z:
            return tr("Hip") + " (Z)"_L1;
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::ListAll() const -> QStringList
{
    QStringList listNames;
    const QDomNodeList list = elementsByTagName(TagMeasurement);
    listNames.reserve(list.size());

    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (!domElement.isNull())
        {
            listNames.append(domElement.attribute(AttrName));
        }
    }

    return listNames;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::ListKnown() const -> QStringList
{
    QStringList listNames;
    const QStringList list = ListAll();
    listNames.reserve(list.size());
    for (const auto &i : list)
    {
        if (i.indexOf(CustomMSign) != 0)
        {
            listNames.append(i);
        }
    }

    return listNames;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::GetData() const -> VContainer *
{
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::CreateEmptyMultisizeFile(Unit unit, const QVector<MeasurementDimension_p> &dimensions)
{
    this->clear();
    QDomElement mElement = this->createElement(TagVST);

    mElement.appendChild(createComment(FileComment()));
    mElement.appendChild(CreateElementWithText(TagVersion, VVSTConverter::MeasurementMaxVerStr));
    mElement.appendChild(CreateElementWithText(TagReadOnly, falseStr));
    mElement.appendChild(createElement(TagNotes));
    mElement.appendChild(CreateElementWithText(TagUnit, UnitsToStr(unit)));
    mElement.appendChild(createElement(TagPMSystem));
    mElement.appendChild(CreateDimensions(dimensions));
    mElement.appendChild(createElement(TagRestrictions));
    mElement.appendChild(createElement(TagBodyMeasurements));

    this->appendChild(mElement);
    insertBefore(
        createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")),
        this->firstChild());

    // Cache data
    m_units = unit;
    m_dimensions = ReadDimensions();
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::CreateEmptyIndividualFile(Unit unit)
{
    this->clear();
    QDomElement mElement = this->createElement(TagVIT);

    mElement.appendChild(createComment(FileComment()));
    mElement.appendChild(CreateElementWithText(TagVersion, VVITConverter::MeasurementMaxVerStr));
    mElement.appendChild(CreateElementWithText(TagReadOnly, falseStr));
    mElement.appendChild(createElement(TagNotes));
    mElement.appendChild(CreateElementWithText(TagUnit, UnitsToStr(unit)));
    mElement.appendChild(createElement(TagPMSystem));

    QDomElement personal = createElement(TagPersonal);
    personal.appendChild(createElement(TagCustomer));
    personal.appendChild(CreateElementWithText(TagBirthDate, *defBirthDate));
    personal.appendChild(CreateElementWithText(TagGender, GenderToStr(GenderType::Unknown)));

    personal.appendChild(createElement(TagEmail));
    mElement.appendChild(personal);

    mElement.appendChild(createElement(TagBodyMeasurements));

    this->appendChild(mElement);
    insertBefore(
        createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")),
        this->firstChild());

    // Cache data
    m_units = unit;
    m_dimensions = ReadDimensions();
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::CreateDimensions(const QVector<MeasurementDimension_p> &dimensions) -> QDomElement
{
    QDomElement dimensionsTag = createElement(TagDimensions);

    for (const auto &dimension : dimensions)
    {
        QDomElement dimensionTag = createElement(TagDimension);

        SetAttribute(dimensionTag, AttrType, DimensionTypeToStr(dimension->Type()));
        SetAttribute(dimensionTag, AttrBase, dimension->BaseValue());
        SetAttribute(dimensionTag, AttrMin, dimension->MinValue());
        SetAttribute(dimensionTag, AttrMax, dimension->MaxValue());
        SetAttribute(dimensionTag, AttrStep, dimension->Step());
        SetAttributeOrRemoveIf<bool>(dimensionTag, AttrMeasurement, dimension->IsBodyMeasurement(),
                                     [](bool m) noexcept { return m; });
        SetAttributeOrRemoveIf<QString>(dimensionTag, AttrCustomName, dimension->CustomName(),
                                        [](const QString &name) noexcept { return name.isEmpty(); });

        dimensionsTag.appendChild(dimensionTag);
    }

    return dimensionsTag;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::UniqueTagAttr(const QString &tag, const QString &attr, qreal defValue) const -> qreal
{
    const qreal defVal = UnitConvertor(defValue, Unit::Cm, Units());

    const QDomNodeList nodeList = this->elementsByTagName(tag);
    if (nodeList.isEmpty())
    {
        return defVal;
    }

    if (const QDomNode domNode = nodeList.at(0); !domNode.isNull() && domNode.isElement())
    {
        const QDomElement domElement = domNode.toElement();
        if (!domElement.isNull())
        {
            return GetParametrDouble(domElement, attr, u"%1"_s.arg(defVal));
        }
    }

    return defVal;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::MakeEmpty(const QString &name, const QString &formula, MeasurementType varType) -> QDomElement
{
    QDomElement element = createElement(TagMeasurement);

    SetAttribute(element, AttrName, name);

    if (varType == MeasurementType::Measurement)
    {
        if (type == MeasurementsType::Multisize)
        {
            SetAttribute(element, AttrBase, QChar('0'));
        }
        else
        {
            SetAttribute(element, AttrValue, formula.isEmpty() ? QChar('0') : formula);
        }
    }
    else
    {
        SetAttribute(element, AttrType, MeasurementTypeToString(varType));
    }

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::FindM(const QString &name) const -> QDomElement
{
    if (name.isEmpty())
    {
        qWarning() << tr("The measurement name is empty!");
        return {};
    }

    QDomNodeList const list = elementsByTagName(TagMeasurement);

    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull())
        {
            continue;
        }

        const QString parameter = domElement.attribute(AttrName);
        if (parameter == name)
        {
            return domElement;
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::ReadType() const -> MeasurementsType
{
    QDomElement const root = documentElement();
    if (root.tagName() == TagVST)
    {
        return MeasurementsType::Multisize;
    }

    if (root.tagName() == TagVIT)
    {
        return MeasurementsType::Individual;
    }

    return MeasurementsType::Unknown;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::ReadUnits() const -> Unit
{
    Unit units = StrToUnits(UniqueTagText(TagUnit, unitCM));

    if (units == Unit::Px)
    {
        units = Unit::Cm;
    }

    return units;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::ReadDimensions() const -> VDimensions
{
    if (type != MeasurementsType::Multisize)
    {
        return {};
    }

    VDimensions dimensions;

    const Unit units = Units();
    const QDomNodeList list = elementsByTagName(TagDimension);
    for (int i = 0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();
        const MeasurementDimension type = StrToDimensionType(GetParametrString(dom, AttrType));
        const qreal min = GetParametrDouble(dom, AttrMin, QChar('0'));
        const qreal max = GetParametrDouble(dom, AttrMax, QChar('0'));
        const qreal step = GetParametrDouble(dom, AttrStep, QStringLiteral("-1"));

        MeasurementDimension_p dimension;

        if (type == MeasurementDimension::X)
        {
            dimension = QSharedPointer<VXMeasurementDimension>::create(units, min, max, step);
        }
        else if (type == MeasurementDimension::Y)
        {
            dimension = QSharedPointer<VYMeasurementDimension>::create(units, min, max, step);
        }
        else if (type == MeasurementDimension::W)
        {
            dimension = QSharedPointer<VWMeasurementDimension>::create(units, min, max, step);
        }
        else if (type == MeasurementDimension::Z)
        {
            dimension = QSharedPointer<VZMeasurementDimension>::create(units, min, max, step);
        }
        else
        {
            continue;
        }

        dimension->SetBaseValue(GetParametrDouble(dom, AttrBase, QChar('0')));
        dimension->SetBodyMeasurement(GetParametrBool(dom, AttrMeasurement, trueStr));
        dimension->SetCustomName(GetParametrEmptyString(dom, AttrCustomName));
        dimension->SetLabels(ReadDimensionLabels(dom));

        if (not dimension->IsValid())
        {
            VExceptionObjectError excep(tr("Dimension is not valid"), dom);
            excep.AddMoreInformation(dimension->Error());
            throw excep;
        }

        dimensions.insert(type, dimension);
    }

    return dimensions;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::WriteCorrections(QDomElement &mElement, const QMap<QString, VMeasurementCorrection> &corrections)
{
    if (QDomElement correctionsTag = mElement.firstChildElement(TagCorrections); not corrections.isEmpty())
    {
        if (not correctionsTag.isNull())
        {
            RemoveAllChildren(correctionsTag);
        }
        else
        {
            correctionsTag = createElement(TagCorrections);
            mElement.appendChild(correctionsTag);
        }

        QMap<QString, VMeasurementCorrection>::const_iterator i = corrections.constBegin();
        while (i != corrections.constEnd())
        {
            QDomElement correctionTag = createElement(TagCorrection);

            SetAttributeOrRemoveIf<QString>(correctionTag,
                                            AttrCoordinates,
                                            i.key(),
                                            [i](const QString &) noexcept
                                            { return qFuzzyIsNull(i.value().correction) && i.value().alias.isEmpty(); });
            SetAttributeOrRemoveIf<qreal>(correctionTag,
                                          AttrCorrection,
                                          i.value().correction,
                                          [](qreal c) noexcept { return qFuzzyIsNull(c); });
            SetAttributeOrRemoveIf<QString>(correctionTag,
                                            AttrAlias,
                                            i.value().alias,
                                            [](const QString &alias) noexcept { return alias.isEmpty(); });

            correctionsTag.appendChild(correctionTag);
            ++i;
        }
    }
    else
    {
        if (not correctionsTag.isNull())
        {
            mElement.removeChild(correctionsTag);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurements::ReadImage(const QDomElement &mElement) -> VPatternImage
{
    QDomElement const imageTag = mElement.firstChildElement(TagImage);
    if (imageTag.isNull())
    {
        return {};
    }

    VPatternImage image;
    if (not imageTag.isNull())
    {
        image.SetContentData(imageTag.text().toLatin1(), imageTag.attribute(AttrContentType));
    }
    return image;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::WriteImage(QDomElement &mElement, const VPatternImage &image)
{
    QDomElement imageElement = mElement.firstChildElement(TagImage);
    if (not image.IsNull())
    {
        if (not imageElement.isNull())
        {
            setTagText(imageElement, image.ContentData());
            imageElement.setAttribute(AttrContentType, image.ContentType());
        }
        else
        {
            imageElement = createElement(TagImage);
            setTagText(imageElement, image.ContentData());
            imageElement.setAttribute(AttrContentType, image.ContentType());
            mElement.appendChild(imageElement);
        }
    }
    else
    {
        if (not imageElement.isNull())
        {
            mElement.removeChild(imageElement);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SaveDimesionLabels(QDomElement &dElement, const DimesionLabels &labels)
{
    if (dElement.isNull())
    {
        qDebug() << "Invalid dimension tag";
    }

    QDomElement labelsTag = dElement.firstChildElement(TagLabels);
    if (not labels.isEmpty())
    {
        if (not labelsTag.isNull())
        {
            RemoveAllChildren(labelsTag);
        }
        else
        {
            labelsTag = createElement(TagLabels);
            dElement.appendChild(labelsTag);
        }

        DimesionLabels::const_iterator i = labels.constBegin();
        while (i != labels.constEnd())
        {
            if (not i.value().isEmpty())
            {
                QDomElement labelTag = createElement(TagLabel);

                SetAttribute(labelTag, AttrValue, i.key());
                SetAttribute(labelTag, AttrLabel, i.value());

                labelsTag.appendChild(labelTag);
            }
            ++i;
        }
    }
    else
    {
        if (not labelsTag.isNull())
        {
            dElement.removeChild(labelsTag);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::ClearDimension(IMD type)
{
    const QString d = VMeasurements::IMDToStr(type);
    QDomNodeList const list = elementsByTagName(TagMeasurement);

    for (int i = 0; i < list.size(); ++i)
    {
        if (QDomElement domElement = list.at(i).toElement();
            !domElement.isNull() && domElement.attribute(AttrDimension) == d)
        {
            domElement.removeAttribute(AttrDimension);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::ReadMeasurement(const QDomElement &dom, QSharedPointer<VContainer> &tempData,
                                    QSharedPointer<VMeasurement> &meash, QSharedPointer<VMeasurement> &tempMeash, int i,
                                    qreal baseA, qreal baseB, qreal baseC) const
{
    const QString fullName = GetParametrEmptyString(dom, AttrFullName);
    const bool specialUnits = GetParametrBool(dom, AttrSpecialUnits, falseStr);
    const QString name = GetParametrString(dom, AttrName).simplified();
    const QString description = GetParametrEmptyString(dom, AttrDescription);

    if (type == MeasurementsType::Multisize)
    {
        qreal base = GetParametrDouble(dom, AttrBase, QChar('0'));
        qreal shiftA = GetParametrDouble(dom, AttrShiftA, QChar('0'));
        qreal shiftB = GetParametrDouble(dom, AttrShiftB, QChar('0'));
        qreal shiftC = GetParametrDouble(dom, AttrShiftC, QChar('0'));
        QMap<QString, VMeasurementCorrection> corrections = ReadCorrections(dom);

        qreal convertedBaseA = DimensionABase();
        qreal convertedBaseB = DimensionBBase();
        qreal convertedBaseC = DimensionCBase();
        qreal convertedStepA = DimensionAStep();
        qreal convertedStepB = DimensionBStep();
        qreal convertedStepC = DimensionCStep();

        if (not specialUnits)
        {
            base = UnitConvertor(base, Units(), *data->GetPatternUnit());
            shiftA = UnitConvertor(shiftA, Units(), *data->GetPatternUnit());
            shiftB = UnitConvertor(shiftB, Units(), *data->GetPatternUnit());
            shiftC = UnitConvertor(shiftC, Units(), *data->GetPatternUnit());

            QMutableMapIterator iterator(corrections);
            while (iterator.hasNext())
            {
                iterator.next();
                VMeasurementCorrection c = iterator.value();
                c.correction = UnitConvertor(c.correction, Units(), *data->GetPatternUnit());
                iterator.setValue(c);
            }

            convertedBaseA = UnitConvertor(convertedBaseA, Units(), *data->GetPatternUnit());
            convertedBaseB = UnitConvertor(convertedBaseB, Units(), *data->GetPatternUnit());
            convertedBaseC = UnitConvertor(convertedBaseC, Units(), *data->GetPatternUnit());

            convertedStepA = UnitConvertor(convertedStepA, Units(), *data->GetPatternUnit());
            convertedStepB = UnitConvertor(convertedStepB, Units(), *data->GetPatternUnit());
            convertedStepC = UnitConvertor(convertedStepC, Units(), *data->GetPatternUnit());
        }

        meash = QSharedPointer<VMeasurement>::create(static_cast<quint32>(i), name, convertedBaseA, convertedBaseB,
                                                     convertedBaseC, base);
        meash->SetBaseA(baseA);
        meash->SetBaseB(baseB);
        meash->SetBaseC(baseC);

        meash->SetShiftA(shiftA);
        meash->SetShiftB(shiftB);
        meash->SetShiftC(shiftC);

        meash->SetStepA(convertedStepA);
        meash->SetStepB(convertedStepB);
        meash->SetStepC(convertedStepC);

        meash->SetSpecialUnits(specialUnits);
        meash->SetCorrections(corrections);
        meash->SetGuiText(fullName);
        meash->SetDescription(description);
        meash->SetKnownMeasurementsId(KnownMeasurements());

        if (meash->IsCustom())
        {
            meash->SetImage(ReadImage(dom));
        }
    }
    else
    {
        const IMD dimension =
            VMeasurements::StrToIMD(GetParametrString(dom, AttrDimension, VMeasurements::IMDToStr(IMD::N)));
        const QString formula = GetParametrString(dom, AttrValue, QChar('0'));
        bool ok = false;
        qreal value = EvalFormula(tempData.data(), formula, &ok);

        tempMeash =
            QSharedPointer<VMeasurement>::create(tempData.data(), static_cast<quint32>(i), name, value, formula, ok);

        if (not specialUnits)
        {
            value = UnitConvertor(value, Units(), *data->GetPatternUnit());
        }

        const QString alias = GetParametrEmptyString(dom, AttrValueAlias);

        meash = QSharedPointer<VMeasurement>::create(data, static_cast<quint32>(i), name, value, formula, ok);
        meash->SetGuiText(fullName);
        meash->SetDescription(description);
        meash->SetSpecialUnits(specialUnits);
        meash->SetDimension(dimension);
        meash->SetKnownMeasurementsId(KnownMeasurements());
        meash->SetValueAlias(alias);

        if (meash->IsCustom())
        {
            meash->SetImage(ReadImage(dom));
        }
    }
}
