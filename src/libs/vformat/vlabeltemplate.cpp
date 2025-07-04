/************************************************************************
 **
 **  @file   vlabeltemplate.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 8, 2015
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

#include "vlabeltemplate.h"
#include "../ifc/xml/vlabeltemplateconverter.h"

const QString VLabelTemplate::TagTemplate = QStringLiteral("template");
const QString VLabelTemplate::TagLines    = QStringLiteral("lines");

//---------------------------------------------------------------------------------------------------------------------
VLabelTemplate::VLabelTemplate()
    : VDomDocument()
{}

//---------------------------------------------------------------------------------------------------------------------
void VLabelTemplate::CreateEmptyTemplate()
{
    clear();
    QDomElement templateElement = this->createElement(TagTemplate);
    templateElement.appendChild(CreateElementWithText(TagVersion, VLabelTemplateConverter::LabelTemplateMaxVerStr));
    templateElement.appendChild(createElement(TagLines));

    this->appendChild(templateElement);
    insertBefore(createProcessingInstruction(QStringLiteral("xml"),
                                             QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")), this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
void VLabelTemplate::AddLines(const QVector<VLabelTemplateLine> &lines)
{
    const QDomNodeList listLines = elementsByTagName(TagLines);
    if (listLines.isEmpty())
    {
        return;
    }

    QDomElement tag = listLines.at(0).toElement();
    VDomDocument::SetLabelTemplate(tag, lines);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLabelTemplate::ReadLines() const -> QVector<VLabelTemplateLine>
{
    const QDomNodeList listLines = elementsByTagName(TagLines);
    if (listLines.isEmpty())
    {
        return QVector<VLabelTemplateLine>();
    }

    return VDomDocument::GetLabelTemplate(listLines.at(0).toElement());
}
