/************************************************************************
 **
 **  @file   vwatermark.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 12, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#include "vwatermark.h"

#include "../ifc/xml/vwatermarkconverter.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/projectversion.h"

using namespace Qt::Literals::StringLiterals;

const QString VWatermark::TagWatermark = QStringLiteral("watermark");
const QString VWatermark::TagText = QStringLiteral("text");
const QString VWatermark::TagImage = QStringLiteral("image");

const QString VWatermark::AttrOpacity = QStringLiteral("opacity");
const QString VWatermark::AttrRotation = QStringLiteral("rotation");
const QString VWatermark::AttrFont = QStringLiteral("font");
const QString VWatermark::AttrPath = QStringLiteral("path");
const QString VWatermark::AttrGrayscale = QStringLiteral("grayscale");
const QString VWatermark::AttrShow = QStringLiteral("show");

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FileComment() -> QString
{
    return u"Watermark created with Valentina v%1 (https://smart-pattern.com.ua/)."_s.arg(AppVersionStr());
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VWatermark::VWatermark()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VWatermark::CreateEmptyWatermark()
{
    this->clear();
    QDomElement wElement = this->createElement(TagWatermark);

    wElement.appendChild(createComment(FileComment()));
    wElement.appendChild(CreateElementWithText(TagVersion, VWatermarkConverter::WatermarkMaxVerStr));
    wElement.appendChild(createElement(TagText));
    wElement.appendChild(createElement(TagImage));

    appendChild(wElement);
    insertBefore(
        createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")),
        this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
auto VWatermark::SaveDocument(const QString &fileName, QString &error) -> bool
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
auto VWatermark::GetWatermark() const -> VWatermarkData
{
    VWatermarkData data;

    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        const QDomElement rootElement = root.toElement();
        data.opacity = GetParametrInt(rootElement, AttrOpacity, QChar('2'));

        QDomElement text = rootElement.firstChildElement(TagText);
        if (not text.isNull())
        {
            data.showText = GetParametrBool(text, AttrShow, trueStr);
            data.text = GetParametrEmptyString(text, AttrText);
            data.textRotation = GetParametrInt(text, AttrRotation, QChar('0'));
            data.font = FontFromString(GetParametrEmptyString(text, AttrFont));
            QColor color(GetParametrString(text, AttrColor, QColor(Qt::black).name()));
            if (not color.isValid())
            {
                color = Qt::black;
            }
            data.textColor = color;
        }

        QDomElement image = rootElement.firstChildElement(TagImage);
        if (not image.isNull())
        {
            data.showImage = GetParametrBool(image, AttrShow, trueStr);
            data.path = GetParametrEmptyString(image, AttrPath);
            data.imageRotation = GetParametrInt(image, AttrRotation, QChar('0'));
            data.grayscale = GetParametrBool(image, AttrGrayscale, falseStr);
        }
    }

    return data;
}

//---------------------------------------------------------------------------------------------------------------------
void VWatermark::SetWatermark(const VWatermarkData &data)
{
    QDomNode root = documentElement();
    if (not root.isNull() && root.isElement())
    {
        QDomElement rootElement = root.toElement();
        SetAttribute(rootElement, AttrOpacity, data.opacity);

        QDomElement text = rootElement.firstChildElement(TagText);
        if (not text.isNull())
        {
            SetAttribute(text, AttrShow, data.showText);
            SetAttributeOrRemoveIf<QString>(text, AttrText, data.text,
                                            [](const QString &text) noexcept { return text.isEmpty(); });
            SetAttributeOrRemoveIf<int>(text, AttrRotation, data.textRotation,
                                        [](int textRotation) noexcept { return textRotation == 0; });
            SetAttributeOrRemoveIf<QString>(text, AttrFont, data.font.toString(),
                                            [](const QString &fontString) noexcept { return fontString.isEmpty(); });
            SetAttribute(text, AttrColor, data.textColor.name());
        }

        QDomElement image = rootElement.firstChildElement(TagImage);
        if (not image.isNull())
        {
            SetAttribute(image, AttrShow, data.showImage);
            SetAttributeOrRemoveIf<QString>(image, AttrPath, data.path,
                                            [](const QString &path) noexcept { return path.isEmpty(); });
            SetAttributeOrRemoveIf<int>(image, AttrRotation, data.imageRotation,
                                        [](int imageRotation) noexcept { return imageRotation == 0; });
            SetAttributeOrRemoveIf<bool>(image, AttrGrayscale, data.grayscale,
                                         [](bool grayscale) noexcept { return not grayscale; });
        }
    }
}
