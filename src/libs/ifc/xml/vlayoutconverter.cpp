/************************************************************************
 **
 **  @file   vlayoutconverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vlayoutconverter.h"
#include "../exception/vexception.h"
#include "../ifcdef.h"
#include "../vlayout/vlayoutpoint.h"
#include "compatibility.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VLayoutConverter::LayoutMinVerStr = QStringLiteral("0.1.0");
const QString VLayoutConverter::LayoutMaxVerStr = QStringLiteral("0.1.5");
const QString VLayoutConverter::CurrentSchema = QStringLiteral("://schema/layout/v0.1.5.xsd");

// VLayoutConverter::LayoutMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
// VLayoutConverter::LayoutMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// The list of all string we use for conversion
// Better to use global variables because repeating QStringLiteral blows up code size
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSeamLineTag, (QLatin1String("seamLine")))              // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strSeamAllowanceTag, (QLatin1String("seamAllowance")))    // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strInternalPathTag, (QLatin1String("internalPath")))      // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strMarkerTag, (QLatin1String("marker")))                  // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPointTag, (QLatin1String("point")))                    // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPieceTag, (QLatin1String("piece")))                    // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strGrainlineTag, (QLatin1String("grainline")))            // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrX, (QLatin1String("x")))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrY, (QLatin1String("y")))                           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrTurnPoint, (QLatin1String("turnPoint")))           // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrCurvePoint, (QLatin1String("curvePoint")))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrId, (QLatin1String("id")))                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrUId, (QLatin1String("uid")))                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrAngle, (QLatin1String("angle")))                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strAttrArrowDirection, (QLatin1String("arrowDirection"))) // NOLINT

QT_WARNING_POP

const QChar groupSep = QLatin1Char(';');
const QChar coordintatesSep = QLatin1Char(',');
const QChar pointsSep = QLatin1Char(' ');
// const QChar itemsSep        = QLatin1Char('*');

//---------------------------------------------------------------------------------------------------------------------
auto StringV0_1_2ToPoint(const QString &point) -> QPointF
{
    QStringList coordinates = point.split(coordintatesSep);
    if (coordinates.count() == 2)
    {
        return {coordinates.at(0).toDouble(), coordinates.at(1).toDouble()};
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto StringV0_1_2ToPath(const QString &path) -> QVector<QPointF>
{
    QVector<QPointF> p;
    QStringList points = path.split(pointsSep);
    p.reserve(points.size());
    for (const auto &point : points)
    {
        p.append(StringV0_1_2ToPoint(point));
    }

    return p;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto NumberToString(T number) -> QString
{
    const QLocale locale = QLocale::c();
    return locale.toString(number, 'g', 12).remove(LocaleGroupSeparator(locale));
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VLayoutConverter::VLayoutConverter(const QString &fileName)
  : VAbstractConverter(fileName)
{
    m_ver = GetFormatVersion(VLayoutConverter::GetFormatVersionStr());
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::GetFormatVersionStr() const -> QString
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        const QDomElement layoutElement = root.toElement();
        if (not layoutElement.isNull())
        {
            return GetParametrString(layoutElement, AttrLayoutVersion, QStringLiteral("0.0.0"));
        }
    }
    return QStringLiteral("0.0.0");
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::XSDSchemas() -> QHash<unsigned int, QString>
{
    static auto schemas = QHash<unsigned, QString>{
        std::make_pair(FormatVersion(0, 1, 0), QStringLiteral("://schema/layout/v0.1.0.xsd")),
        std::make_pair(FormatVersion(0, 1, 1), QStringLiteral("://schema/layout/v0.1.1.xsd")),
        std::make_pair(FormatVersion(0, 1, 2), QStringLiteral("://schema/layout/v0.1.2.xsd")),
        std::make_pair(FormatVersion(0, 1, 3), QStringLiteral("://schema/layout/v0.1.3.xsd")),
        std::make_pair(FormatVersion(0, 1, 4), QStringLiteral("://schema/layout/v0.1.4.xsd")),
        std::make_pair(FormatVersion(0, 1, 5), CurrentSchema),
    };

    return schemas;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::SetVersion(const QString &version)
{
    ValidateVersion(version);

    QDomElement root = documentElement().toElement();
    if (root.isElement() && root.hasAttribute(AttrLayoutVersion))
    {
        root.setAttribute(AttrLayoutVersion, version);
    }
    else
    {
        throw VException(tr("Could not change version."));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (FormatVersion(0, 1, 0)):
        case (FormatVersion(0, 1, 1)):
        case (FormatVersion(0, 1, 2)):
            ToV0_1_3();
            Q_FALLTHROUGH();
        case (FormatVersion(0, 1, 3)):
        case (FormatVersion(0, 1, 4)):
            ToV0_1_5();
            ValidateXML(CurrentSchema);
            Q_FALLTHROUGH();
        case (FormatVersion(0, 1, 5)):
            break;
        default:
            InvalidVersion(m_ver);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(LayoutMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::IsReadOnly() const -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutConverter::Schemas() const -> QHash<unsigned int, QString>
{
    return XSDSchemas();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ConvertPiecesToV0_1_3()
{
    // TODO. Delete if minimal supported version is 0.1.3
    Q_STATIC_ASSERT_X(VLayoutConverter::LayoutMinVer < FormatVersion(0, 1, 3), "Time to refactor the code.");

    const QStringList types{*strSeamLineTag, *strSeamAllowanceTag, *strInternalPathTag};

    for (const auto &tagType : types)
    {
        QDomNodeList tags = elementsByTagName(tagType);
        for (int i = 0; i < tags.size(); ++i)
        {
            QDomElement node = tags.at(i).toElement();
            ConvertPathToV0_1_3(node);
        }
    }

    QDomNodeList tags = elementsByTagName(*strMarkerTag);
    for (int i = 0; i < tags.size(); ++i)
    {
        QDomElement node = tags.at(i).toElement();
        RemoveAllChildren(node);
    }

    QDomNodeList pieceTags = elementsByTagName(*strPieceTag);
    for (int i = 0; i < pieceTags.size(); ++i)
    {
        QDomElement node = pieceTags.at(i).toElement();
        if (node.isElement() && node.hasAttribute(*strAttrId))
        {
            node.setAttribute(*strAttrUId, node.attribute(*strAttrId));
            node.removeAttribute(*strAttrId);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ConvertPathToV0_1_3(QDomElement &node)
{
    QString oldPath = node.text();
    if (oldPath.isEmpty())
    {
        return;
    }

    RemoveAllChildren(node);
    QVector<VLayoutPoint> path;
    CastTo(StringV0_1_2ToPath(oldPath), path);

    for (auto &point : path)
    {
        QDomElement pointTag = createElement(*strPointTag);
        SetAttribute(pointTag, *strAttrX, point.x());
        SetAttribute(pointTag, *strAttrY, point.y());

        if (point.TurnPoint())
        {
            SetAttribute(pointTag, *strAttrTurnPoint, point.TurnPoint());
        }

        if (point.CurvePoint())
        {
            SetAttribute(pointTag, *strAttrCurvePoint, point.CurvePoint());
        }

        node.appendChild(pointTag);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ConvertPiecesToV0_1_5()
{
    // TODO. Delete if minimal supported version is 0.1.5
    Q_STATIC_ASSERT_X(VLayoutConverter::LayoutMinVer < FormatVersion(0, 1, 5), "Time to refactor the code.");

    QDomNodeList grainlineTags = elementsByTagName(*strGrainlineTag);
    for (int i = 0; i < grainlineTags.size(); ++i)
    {
        QDomElement node = grainlineTags.at(i).toElement();
        if (node.isElement())
        {
            // remove angle attribute
            if (node.hasAttribute(*strAttrAngle))
            {
                node.removeAttribute(*strAttrAngle);
            }

            // convert arrowDirection
            if (node.hasAttribute(*strAttrArrowDirection))
            {
                QString arrowDirection = node.attribute(*strAttrArrowDirection);

                const QStringList arrows{
                    "atFront",   // 0
                    "atRear",    // 1
                    "atFourWay", // 2
                    "atBoth"     // 3
                };

                switch (arrows.indexOf(arrowDirection))
                {
                    case 0: // at front
                        SetAttribute(node, *strAttrArrowDirection, QLatin1String("oneWayUp"));
                        break;
                    case 1: // at rear
                        SetAttribute(node, *strAttrArrowDirection, QLatin1String("oneWayDown"));
                        break;
                    case 2: // at four way
                        SetAttribute(node, *strAttrArrowDirection, QLatin1String("fourWays"));
                        break;
                    case 3: // at both
                    default:
                        SetAttribute(node, *strAttrArrowDirection, QLatin1String("twoWaysUpDown"));
                        break;
                }
            }

            auto StringToPath = [](const QString &path) -> QVector<QPointF>
            {
                auto StringToPoint = [](const QString &point) -> QPointF
                {
                    QStringList coordinates = point.split(coordintatesSep);
                    if (coordinates.count() == 2)
                    {
                        return {coordinates.at(0).toDouble(), coordinates.at(1).toDouble()};
                    }

                    return {};
                };

                QVector<QPointF> p;
                if (path.isEmpty())
                {
                    return p;
                }

                QStringList points = path.split(pointsSep);
                p.reserve(points.size());
                for (const auto &point : points)
                {
                    p.append(StringToPoint(point));
                }

                return p;
            };

            const QVector<QPointF> path = StringToPath(node.text());
            if (not path.isEmpty())
            {
                auto LineToString = [](const QLineF &line) -> QString
                {
                    auto PointToString = [](const QPointF &p) -> QString
                    { return NumberToString(p.x()) + coordintatesSep + NumberToString(p.y()); };

                    return PointToString(line.p1()) + groupSep + PointToString(line.p2());
                };

                node.firstChild().toText().setData(LineToString(QLineF(ConstFirst(path), ConstLast(path))));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ToV0_1_3()
{
    // TODO. Delete if minimal supported version is 0.1.3
    Q_STATIC_ASSERT_X(VLayoutConverter::LayoutMinVer < FormatVersion(0, 1, 3), "Time to refactor the code.");

    ConvertPiecesToV0_1_3();
    SetVersion(QStringLiteral("0.1.3"));
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutConverter::ToV0_1_5()
{
    // TODO. Delete if minimal supported version is 0.1.5
    Q_STATIC_ASSERT_X(VLayoutConverter::LayoutMinVer < FormatVersion(0, 1, 5), "Time to refactor the code.");
    ConvertPiecesToV0_1_5();
    SetVersion(QStringLiteral("0.1.5"));
    Save();
}
