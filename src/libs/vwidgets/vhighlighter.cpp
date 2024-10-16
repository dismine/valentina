/************************************************************************
 **
 **  @file   vhighlighter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 11, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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
#include "vhighlighter.h"
#include <memory>

//---------------------------------------------------------------------------------------------------------------------
VTextBlockData::VTextBlockData()
  : m_parentheses()
{
    // Nothing to do
}

//---------------------------------------------------------------------------------------------------------------------
auto VTextBlockData::Parentheses() -> QVector<ParenthesisInfo *>
{
    return m_parentheses;
}

//---------------------------------------------------------------------------------------------------------------------
void VTextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() && info->position > m_parentheses.at(i)->position)
    {
        ++i;
    }

    m_parentheses.insert(i, info);
}

//---------------------------------------------------------------------------------------------------------------------
VHighlighter::VHighlighter(QTextDocument *document)
  : QSyntaxHighlighter(document)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VHighlighter::highlightBlock(const QString &text)
{
    auto data = std::make_unique<VTextBlockData>();

    vsizetype leftPos = text.indexOf('(');
    while (leftPos != -1)
    {
        auto info = std::make_unique<ParenthesisInfo>();
        info->character = '(';
        info->position = leftPos;

        data->insert(info.release());
        leftPos = text.indexOf('(', leftPos + 1);
    }

    vsizetype rightPos = text.indexOf(')');
    while (rightPos != -1)
    {
        auto info = std::make_unique<ParenthesisInfo>();
        info->character = ')';
        info->position = rightPos;

        data->insert(info.release());

        rightPos = text.indexOf(')', rightPos + 1);
    }

    setCurrentBlockUserData(data.release());
}
