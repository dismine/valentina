/************************************************************************
 **
 **  @file   vsvghandler.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 4, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "vsvghandler.h"

#include <utility>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QtDebug>

#include "vsvgpathtokenizer.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ApplyMatrixTransform(const QString &str, QTransform &transform) -> void
{
    static QRegularExpression const regex(QStringLiteral(
        R"(matrix\(\s*([-\d.eE]+)[,\s]\s*([-\d.eE]+)[,\s]\s*([-\d.eE]+)[,\s]\s*([-\d.eE]+)[,\s]\s*([-\d.eE]+)[,\s]\s*([-\d.eE]+)\s*\))"));
    if (const QRegularExpressionMatch match = regex.match(str); match.hasMatch())
    {
        const qreal m11 = match.captured(1).toDouble();
        const qreal m12 = match.captured(2).toDouble();
        const qreal m21 = match.captured(3).toDouble();
        const qreal m22 = match.captured(4).toDouble();
        const qreal dx = match.captured(5).toDouble();
        const qreal dy = match.captured(6).toDouble();
        transform.setMatrix(m11, m12, 0, m21, m22, 0, dx, dy, 1);
        return;
    }

    qDebug() << "Not matrix operation: " << str;
}

//---------------------------------------------------------------------------------------------------------------------
auto ApplyTranslateTransform(const QString &str, QTransform &transform) -> void
{
    static QRegularExpression const regex(QStringLiteral(R"(translate\(\s*([-\d.eE]+)(?:[,\s]\s*([-\d.eE]+))?\s*\))"));
    if (const QRegularExpressionMatch match = regex.match(str); match.hasMatch())
    {
        const qreal dx = match.captured(1).toDouble();
        const qreal dy = match.captured(2).isEmpty() ? 0 : match.captured(2).toDouble();
        transform.translate(dx, dy);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ApplyRotateTransform(const QString &str, QTransform &transform) -> void
{
    static QRegularExpression const regex(
        QStringLiteral(R"(rotate\(\s*([-\d.eE]+)(?:[,\s]\s*([-\d.eE]+))?(?:[,\s]\s*([-\d.eE]+))?\s*\))"));
    if (const QRegularExpressionMatch match = regex.match(str); match.hasMatch())
    {
        const qreal angle = match.captured(1).toDouble();
        const qreal cx = match.captured(2).isEmpty() ? 0 : match.captured(2).toDouble();
        const qreal cy = match.captured(3).isEmpty() ? 0 : match.captured(3).toDouble();
        transform.translate(cx, cy);
        transform.rotate(angle);
        transform.translate(-cx, -cy);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ApplyScaleTransform(const QString &str, QTransform &transform) -> void
{
    static QRegularExpression const regex(QStringLiteral(R"(scale\(\s*([-\d.eE]+)(?:[,\s]\s*([-\d.eE]+))?\s*\))"));
    if (const QRegularExpressionMatch match = regex.match(str); match.hasMatch())
    {
        const qreal sx = match.captured(1).toDouble();
        const qreal sy = match.captured(2).isEmpty() ? sx : match.captured(2).toDouble();
        transform.scale(sx, sy);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ApplySkewXTransform(const QString &str, QTransform &transform) -> void
{
    static QRegularExpression const regex(QStringLiteral(R"(skewX\(\s*([-\d.eE]+)\s*\))"));
    if (const QRegularExpressionMatch match = regex.match(str); match.hasMatch())
    {
        const qreal angle = match.captured(1).toDouble();
        const qreal skewX = qTan(qDegreesToRadians(angle));
        transform.shear(skewX, 0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ApplySkewYTransform(const QString &str, QTransform &transform) -> void
{
    static QRegularExpression const regex(QStringLiteral(R"(skewY\(\s*([-\d.eE]+)\s*\))"));
    if (const QRegularExpressionMatch match = regex.match(str); match.hasMatch())
    {
        const qreal angle = match.captured(1).toDouble();
        const qreal skewY = qTan(qDegreesToRadians(angle));
        transform.shear(0, skewY);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto ReadTransform(const QString &transformStr) -> QTransform
{
    QTransform transform;

    // Regular expression to match the entire transformation function, e.g., translate(-200,0)
    static QRegularExpression const regex(QStringLiteral(R"((\w+)\(\s*([^\)]+)\s*\))"));

    // Find all transformations in the string
    auto iter = regex.globalMatch(transformStr);

    // Iterate through each match and apply the corresponding transformation
    while (iter.hasNext())
    {
        const QRegularExpressionMatch match = iter.next();

        // Apply each transformation based on the type
        if (const QString transformType = match.captured(1); transformType == "matrix"_L1)
        {
            ApplyMatrixTransform(match.captured(0), transform);
        }
        else if (transformType == "translate"_L1)
        {
            ApplyTranslateTransform(match.captured(0), transform);
        }
        else if (transformType == "rotate"_L1)
        {
            ApplyRotateTransform(match.captured(0), transform);
        }
        else if (transformType == "scale"_L1)
        {
            ApplyScaleTransform(match.captured(0), transform);
        }
        else if (transformType == "skewX"_L1)
        {
            ApplySkewXTransform(match.captured(0), transform);
        }
        else if (transformType == "skewY"_L1)
        {
            ApplySkewYTransform(match.captured(0), transform);
        }
    }
    return transform;
}

//---------------------------------------------------------------------------------------------------------------------
auto ConvertToPixels(const QString &value) -> qreal
{
    static const QMap<QString, qreal> unitConversion
        = {{"px", 1.0}, {"pt", 1.3333}, {"pc", 16.0}, {"cm", 37.795}, {"mm", 3.7795}, {"in", 96.0}};

    static const QRegularExpression regex(QStringLiteral(R"(^([\d\.]+)(px|pt|pc|cm|mm|in)?$)"));
    QRegularExpressionMatch const match = regex.match(value.trimmed());

    if (!match.hasMatch())
    {
        return 0.0; // Handle invalid input gracefully
    }

    const qreal number = match.captured(1).toDouble();
    QString const unit = match.captured(2);

    return number * unitConversion.value(unit, 1.0); // Default to px if unit is missing
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VSvgHandler::VSvgHandler(const QString &filePath)
{
    ToPainterPath(filePath);
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::ToPainterPath(const QString &filePath)
{
    ResetState();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open file:" << filePath;
        return;
    }
    m_xml.setDevice(&file);

    while (!m_xml.atEnd() && !m_xml.hasError())
    {
        m_xml.readNext();
        if (m_xml.isStartElement())
        {
            HandleStartElement();
        }
        else if (m_xml.isEndElement() && m_xml.name() == "g"_L1)
        {
            RestorePreviousGroup();
        }
    }
    if (m_xml.hasError())
    {
        qWarning() << "XML error:" << m_xml.errorString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::Render(QPainter *painter)
{
    painter->save();

    const qreal penWidth = painter->pen().widthF();
    const QRectF clipRect = m_pageSize.adjusted(-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2);
    painter->setClipRect(clipRect);

    painter->drawPath(m_path);
    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::ResetState()
{
    m_path.clear();
    m_currentTransform.reset();
    m_transformStack.clear();
    m_pathStack.clear();
    m_pageSize = QRectF();
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::HandleStartElement()
{
    const QStringView name = m_xml.name();
    if (name == "defs"_L1)
    {
        m_xml.skipCurrentElement(); // Skip <defs> entirely
        return;
    }

    if (name == "svg"_L1)
    {
        ParseSvgAttributes();
    }
    else if (name == "g"_L1)
    {
        SaveCurrentGroup();
    }
    else if (name == "path"_L1)
    {
        ParsePathElement();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::ParseSvgAttributes()
{
    const QString widthStr = m_xml.attributes().value("width"_L1).toString();
    const QString heightStr = m_xml.attributes().value("height"_L1).toString();

    const qreal width = ConvertToPixels(widthStr);
    const qreal height = ConvertToPixels(heightStr);

    if (width > 0 && height > 0)
    {
        m_pageSize = QRectF(0, 0, width, height);
    }

    InitialTransformFromViewBox();
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::InitialTransformFromViewBox()
{
    if (m_pageSize.width() <= 0 || m_pageSize.height() <= 0)
    {
        return; // Invalid target page size
    }

    const QString viewBoxStr = m_xml.attributes().value("viewBox"_L1).toString();
    if (viewBoxStr.isEmpty())
    {
        return; // Identity, no transform needed
    }

    static const QRegularExpression regex(QStringLiteral(R"(\s+)"));
    const QStringList viewBoxParts = viewBoxStr.split(regex, Qt::SkipEmptyParts);
    if (viewBoxParts.size() != 4)
    {
        return; // Malformed viewBox
    }

    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    bool ok4 = false;
    const qreal minX = viewBoxParts[0].toDouble(&ok1);
    const qreal minY = viewBoxParts[1].toDouble(&ok2);
    const qreal vbWidth = viewBoxParts[2].toDouble(&ok3);
    const qreal vbHeight = viewBoxParts[3].toDouble(&ok4);

    if (!(ok1 && ok2 && ok3 && ok4) || vbWidth <= 0 || vbHeight <= 0)
    {
        return;
    }

    const qreal scaleX = m_pageSize.width() / vbWidth;
    const qreal scaleY = m_pageSize.height() / vbHeight;

    QString par = m_xml.attributes().value("preserveAspectRatio"_L1).toString().trimmed();
    if (par.isEmpty())
    {
        par = "xMidYMid meet"_L1;
    }

    QString align = "xMidYMid"_L1;
    QString meetOrSlice = "meet"_L1;

    if (par == "none"_L1)
    {
        align = "none"_L1;
    }
    else
    {
        static const QRegularExpression aspectRatioRegex(QStringLiteral(R"(\s+)"));
        const QStringList aspectRatioParts = par.split(aspectRatioRegex);
        if (!aspectRatioParts.empty())
        {
            align = aspectRatioParts.at(0);
        }
        if (aspectRatioParts.size() > 1)
        {
            meetOrSlice = aspectRatioParts.at(1);
        }
    }

    m_currentTransform.translate(-minX, -minY);

    if (align == "none"_L1)
    {
        // Non-uniform scaling
        m_currentTransform.scale(scaleX, scaleY);
        return;
    }

    // Uniform scaling
    const qreal scale = (meetOrSlice == "slice"_L1) ? std::max(scaleX, scaleY) : std::min(scaleX, scaleY);

    // Calculate translation (alignment)
    qreal tx = 0;
    qreal ty = 0;
    const qreal vpW = vbWidth * scale;
    const qreal vpH = vbHeight * scale;

    if (align.contains("xMid"_L1))
    {
        tx = (m_pageSize.width() - vpW) / 2.0;
    }
    else if (align.contains("xMax"_L1))
    {
        tx = (m_pageSize.width() - vpW);
    }

    if (align.contains("YMid"_L1))
    {
        ty = (m_pageSize.height() - vpH) / 2.0;
    }
    else if (align.contains("YMax"_L1))
    {
        ty = (m_pageSize.height() - vpH);
    }

    m_currentTransform.translate(tx / scale, ty / scale);
    m_currentTransform.scale(scale, scale);
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::SaveCurrentGroup()
{
    m_transformStack.push(m_currentTransform);
    m_pathStack.push(m_path);
    m_path = QPainterPath();

    if (const QString transform = m_xml.attributes().value("transform").toString(); !transform.isEmpty())
    {
        m_currentTransform = ReadTransform(transform) * m_currentTransform;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::RestorePreviousGroup()
{
    QPainterPath groupPath = m_pathStack.pop();
    groupPath.addPath(m_path);
    m_path = groupPath;
    m_currentTransform = m_transformStack.pop();
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgHandler::ParsePathElement()
{
    const QString d = m_xml.attributes().value("d").toString();
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    VSVGPathTokenizer tokenizer(d);
    tokenizer.SetSinglePath(true); // Do not close subpaths
    tokenizer.ToPainterPath(path);

    QTransform localTransform;

    if (const QString transform = m_xml.attributes().value("transform").toString(); !transform.isEmpty())
    {
        localTransform = ReadTransform(transform);
    }

    path = (localTransform * m_currentTransform).map(path);

    m_path.addPath(path);
}
