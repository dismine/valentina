/************************************************************************
 **
 **  @file   VVSTConverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
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

#include "vvstconverter.h"

#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QGlobalStatic>
#include <QLatin1String>
#include <QList>
#include <QMap>
#include <QMultiMap>

#include "../vmisc/def.h"
#include "vabstractmconverter.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VVSTConverter::MeasurementMinVerStr = QStringLiteral("0.3.0");
const QString VVSTConverter::MeasurementMaxVerStr = QStringLiteral("0.5.4");
const QString VVSTConverter::CurrentSchema = QStringLiteral("://schema/multisize_measurements/v0.5.4.xsd");

// VVSTConverter::MeasurementMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
// VVSTConverter::MeasurementMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, strTagRead_Only, ("read-only"_L1))          // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrCircumference, ("circumference"_L1)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrMeasurement, ("measurement"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strTagDimension, ("dimension"_L1))          // NOLINT

QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VVSTConverter::VVSTConverter(const QString &fileName)
  : VAbstractMConverter(fileName)
{
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
auto VVSTConverter::XSDSchemas() -> QHash<unsigned int, QString>
{
    static auto schemas = QHash<unsigned, QString>{
        std::make_pair(FormatVersion(0, 3, 0), QStringLiteral("://schema/multisize_measurements/v0.3.0.xsd")),
        std::make_pair(FormatVersion(0, 4, 0), QStringLiteral("://schema/multisize_measurements/v0.4.0.xsd")),
        std::make_pair(FormatVersion(0, 4, 1), QStringLiteral("://schema/multisize_measurements/v0.4.1.xsd")),
        std::make_pair(FormatVersion(0, 4, 2), QStringLiteral("://schema/multisize_measurements/v0.4.2.xsd")),
        std::make_pair(FormatVersion(0, 4, 3), QStringLiteral("://schema/multisize_measurements/v0.4.3.xsd")),
        std::make_pair(FormatVersion(0, 4, 4), QStringLiteral("://schema/multisize_measurements/v0.4.4.xsd")),
        std::make_pair(FormatVersion(0, 5, 0), QStringLiteral("://schema/multisize_measurements/v0.5.0.xsd")),
        std::make_pair(FormatVersion(0, 5, 1), QStringLiteral("://schema/multisize_measurements/v0.5.1.xsd")),
        std::make_pair(FormatVersion(0, 5, 2), QStringLiteral("://schema/multisize_measurements/v0.5.2.xsd")),
        std::make_pair(FormatVersion(0, 5, 3), QStringLiteral("://schema/multisize_measurements/v0.5.3.xsd")),
        std::make_pair(FormatVersion(0, 5, 4), CurrentSchema),
    };

    return schemas;
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (FormatVersion(0, 3, 0)):
            ToV0_4_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 4, 0)):
            ToV0_4_1();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 4, 1)):
            ToV0_4_2();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 4, 2)):
        case (FormatVersion(0, 4, 3)):
        case (FormatVersion(0, 4, 4)):
            ToV0_5_0();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 5, 0)):
        case (FormatVersion(0, 5, 1)):
        case (FormatVersion(0, 5, 2)):
        case (FormatVersion(0, 5, 3)):
            ToV0_5_4();
            ValidateXML(CurrentSchema);
            Q_FALLTHROUGH();
        case (FormatVersion(0, 5, 4)):
            break;
        default:
            InvalidVersion(m_ver);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(MeasurementMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
auto VVSTConverter::IsReadOnly() const -> bool
{
    // Check if attribute read-only was not changed in file format
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMaxVer == FormatVersion(0, 5, 4), "Check attribute read-only.");

    // Possibly in future attribute read-only will change position etc.
    // For now position is the same for all supported format versions.
    // But don't forget to keep all versions of attribute until we support that format versions

    return UniqueTagText(*strTagRead_Only, falseStr) == trueStr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VVSTConverter::Schemas() const -> QHash<unsigned int, QString>
{
    return XSDSchemas();
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::AddNewTagsForV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    QDomElement rootElement = this->documentElement();
    QDomNode refChild = rootElement.firstChildElement(QStringLiteral("version"));

    refChild = rootElement.insertAfter(CreateElementWithText(QStringLiteral("read-only"), falseStr), refChild);

    rootElement.insertAfter(
        CreateElementWithText(QStringLiteral("notes"), UniqueTagText(QStringLiteral("description"))), refChild);
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::RemoveTagsForV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    QDomElement rootElement = this->documentElement();

    {
        const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("description"));
        if (not nodeList.isEmpty())
        {
            rootElement.removeChild(nodeList.at(0));
        }
    }

    {
        const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("id"));
        if (not nodeList.isEmpty())
        {
            rootElement.removeChild(nodeList.at(0));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ConvertMeasurementsToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    const QString tagBM = QStringLiteral("body-measurements");

    QDomElement bm = createElement(tagBM);

    const QMultiMap<QString, QString> names = OldNamesToNewNames_InV0_3_0();
    const QList<QString> keys = names.uniqueKeys();
    for (const auto &key : keys)
    {
        qreal resValue = 0;
        qreal resSizeIncrease = 0;
        qreal resHeightIncrease = 0;

        // This has the same effect as a .values(), just isn't as elegant
        const QList<QString> list = names.values(key);
        for (const auto &val : list)
        {
            const QDomNodeList nodeList = this->elementsByTagName(val);
            if (nodeList.isEmpty())
            {
                continue;
            }

            QDomElement m = nodeList.at(0).toElement();
            const qreal value = GetParametrDouble(m, QStringLiteral("value"), "0.0");
            const qreal size_increase = GetParametrDouble(m, QStringLiteral("size_increase"), "0.0");
            const qreal height_increase = GetParametrDouble(m, QStringLiteral("height_increase"), "0.0");

            if (not qFuzzyIsNull(value))
            {
                resValue = value;
                resSizeIncrease = size_increase;
                resHeightIncrease = height_increase;
            }
        }

        bm.appendChild(AddMV0_4_0(key, resValue, resSizeIncrease, resHeightIncrease));
    }

    QDomElement rootElement = this->documentElement();
    const QDomNodeList listBM = elementsByTagName(tagBM);
    rootElement.replaceChild(bm, listBM.at(0));
}

//---------------------------------------------------------------------------------------------------------------------
auto VVSTConverter::AddMV0_4_0(const QString &name, qreal value, qreal sizeIncrease, qreal heightIncrease)
    -> QDomElement
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    QDomElement element = createElement(QStringLiteral("m"));

    SetAttribute(element, QStringLiteral("name"), name);
    SetAttribute(element, QStringLiteral("base"), QString().setNum(value));
    SetAttribute(element, QStringLiteral("size_increase"), QString().setNum(sizeIncrease));
    SetAttribute(element, QStringLiteral("height_increase"), QString().setNum(heightIncrease));
    SetAttribute(element, QStringLiteral("description"), QString());
    SetAttribute(element, QStringLiteral("full_name"), QString());

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::PM_SystemV0_4_1()
{
    // TODO. Delete if minimal supported version is 0.4.1
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 1), "Time to refactor the code.");

    const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("size"));
    QDomElement personal = nodeList.at(0).toElement();

    QDomElement parent = personal.parentNode().toElement();
    parent.insertBefore(CreateElementWithText(QStringLiteral("pm_system"), QStringLiteral("998")), personal);
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ConvertMeasurementsToV0_4_2()
{
    // TODO. Delete if minimal supported version is 0.4.2
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 2), "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_3_3();
    auto i = names.constBegin();
    while (i != names.constEnd())
    {
        const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("m"));
        if (nodeList.isEmpty())
        {
            ++i;
            continue;
        }

        for (int ii = 0; ii < nodeList.size(); ++ii)
        {
            const QString attrName = QStringLiteral("name");
            QDomElement element = nodeList.at(ii).toElement();
            const QString name = GetParametrString(element, attrName);
            if (name == i.value())
            {
                SetAttribute(element, attrName, i.key());
            }
        }

        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::AddNewTagsForV0_5_0()
{
    // TODO. Delete if minimal supported version is 0.5.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 5, 0), "Time to refactor the code.");

    QDomElement root = documentElement();
    const QDomElement pmSystemTag = root.firstChildElement(QStringLiteral("pm_system"));
    if (pmSystemTag.isNull())
    {
        return;
    }

    QDomElement dimensionsTag = createElement(QStringLiteral("dimensions"));

    auto Base = [this](const QString &base)
    {
        const QDomElement root = documentElement();
        const QDomElement baseTag = root.firstChildElement(base);
        if (baseTag.isNull())
        {
            return 0;
        }

        return GetParametrInt(baseTag, QStringLiteral("base"), QChar('0'));
    };

    const Unit units = Units();

    {
        const int step = static_cast<int>(UnitConvertor(6, Unit::Cm, units));
        const int min = static_cast<int>(UnitConvertor(50, Unit::Cm, units));
        const int max = static_cast<int>(UnitConvertor(200, Unit::Cm, units));

        QDomElement dimensionX = createElement(QStringLiteral("dimension"));
        SetAttribute(dimensionX, QStringLiteral("type"), QChar('x'));
        SetAttribute(dimensionX, QStringLiteral("step"), step);
        SetAttribute(dimensionX, QStringLiteral("min"), min);
        SetAttribute(dimensionX, QStringLiteral("max"), max);
        SetAttribute(dimensionX, QStringLiteral("base"), Base(QStringLiteral("height")));
        dimensionsTag.appendChild(dimensionX);
    }

    {
        const int step = static_cast<int>(UnitConvertor(2, Unit::Cm, units));
        const int min = static_cast<int>(UnitConvertor(22, Unit::Cm, units));
        const int max = static_cast<int>(UnitConvertor(72, Unit::Cm, units));

        QDomElement dimensionY = createElement(QStringLiteral("dimension"));
        SetAttribute(dimensionY, QStringLiteral("type"), QChar('y'));
        SetAttribute(dimensionY, QStringLiteral("step"), step);
        SetAttribute(dimensionY, QStringLiteral("min"), min);
        SetAttribute(dimensionY, QStringLiteral("max"), max);
        SetAttribute(dimensionY, QStringLiteral("base"), Base(QStringLiteral("size")));
        SetAttribute(dimensionY, QStringLiteral("circumference"), true);
        dimensionsTag.appendChild(dimensionY);
    }

    root.insertAfter(dimensionsTag, pmSystemTag);

    root.insertAfter(createElement(QStringLiteral("restrictions")), dimensionsTag);
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::RemoveTagsForV0_5_0()
{
    // TODO. Delete if minimal supported version is 0.5.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 5, 0), "Time to refactor the code.");

    QDomElement root = documentElement();

    const QDomElement sizeTag = root.firstChildElement(QStringLiteral("size"));
    if (not sizeTag.isNull())
    {
        root.removeChild(sizeTag);
    }

    const QDomElement heightTag = root.firstChildElement(QStringLiteral("height"));
    if (not heightTag.isNull())
    {
        root.removeChild(heightTag);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ConvertMeasurementsToV0_5_0()
{
    // TODO. Delete if minimal supported version is 0.5.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 5, 0), "Time to refactor the code.");

    const QDomNodeList measurements = elementsByTagName(QStringLiteral("m"));
    for (int i = 0; i < measurements.size(); ++i)
    {
        QDomElement m = measurements.at(i).toElement();

        SetAttribute(m, QStringLiteral("shiftA"), GetParametrString(m, QStringLiteral("height_increase")));
        m.removeAttribute(QStringLiteral("height_increase"));

        SetAttribute(m, QStringLiteral("shiftB"), GetParametrString(m, QStringLiteral("size_increase")));
        m.removeAttribute(QStringLiteral("size_increase"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ConvertCircumferenceAttreibuteToV0_5_4()
{
    const QDomNodeList list = elementsByTagName(*strTagDimension);
    for (int i = 0; i < list.size(); ++i)
    {
        QDomElement dom = list.at(i).toElement();
        if (dom.hasAttribute(*strAttrCircumference))
        {
            bool m = GetParametrBool(dom, *strAttrCircumference, trueStr);
            dom.removeAttribute(*strAttrCircumference);
            SetAttribute(dom, *strAttrMeasurement, m);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 0), "Time to refactor the code.");

    AddRootComment();
    SetVersion(QStringLiteral("0.4.0"));
    AddNewTagsForV0_4_0();
    RemoveTagsForV0_4_0();
    ConvertMeasurementsToV0_4_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ToV0_4_1()
{
    // TODO. Delete if minimal supported version is 0.4.1
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 1), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.1"));
    PM_SystemV0_4_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ToV0_4_2()
{
    // TODO. Delete if minimal supported version is 0.4.2
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 4, 2), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.2"));
    ConvertMeasurementsToV0_4_2();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ToV0_5_0()
{
    // TODO. Delete if minimal supported version is 0.5.0
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 5, 0), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.5.0"));
    AddNewTagsForV0_5_0();
    RemoveTagsForV0_5_0();
    ConvertMeasurementsToV0_5_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVSTConverter::ToV0_5_4()
{
    // TODO. Delete if minimal supported version is 0.5.4
    Q_STATIC_ASSERT_X(VVSTConverter::MeasurementMinVer < FormatVersion(0, 5, 4), "Time to refactor the code.");

    SetVersion(QStringLiteral("0.5.4"));
    ConvertCircumferenceAttreibuteToV0_5_4();
    Save();
}
