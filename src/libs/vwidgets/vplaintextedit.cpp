/************************************************************************
 **
 **  @file   vplaintextedit.cpp
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
#include "vplaintextedit.h"
#include <QDebug>

// Header <ciso646> is removed in C++20.
#if defined(Q_CC_MSVC) && __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

//---------------------------------------------------------------------------------------------------------------------
VPlainTextEdit::VPlainTextEdit(QWidget *parent)
  : QPlainTextEdit(parent),
    m_highlighter(document())
{
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &VPlainTextEdit::MatchParentheses);
}

//---------------------------------------------------------------------------------------------------------------------
VPlainTextEdit::VPlainTextEdit(const QString &text, QWidget *parent)
  : QPlainTextEdit(text, parent),
    m_highlighter(document())
{
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &VPlainTextEdit::MatchParentheses);
}

//---------------------------------------------------------------------------------------------------------------------
VPlainTextEdit::~VPlainTextEdit()
{
    document()->blockSignals(true); // prevent crash
}

//---------------------------------------------------------------------------------------------------------------------
void VPlainTextEdit::SetFilter(const QString &filter)
{
    if (m_filter.isEmpty() && not filter.isEmpty())
    {
        QTextDocument *doc = document();
        m_allLines.clear();
        m_allLines.reserve(doc->lineCount());

        for (int i = 0; i < doc->blockCount(); ++i)
        {
            m_allLines.append(doc->findBlockByNumber(i).text());
        }
    }

    m_filter = filter;

    Filter();

    if (m_filter.isEmpty())
    {
        m_allLines.clear();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPlainTextEdit::appendFilteredText(const QString &text)
{
    if (m_filter.isEmpty())
    {
        QPlainTextEdit::appendPlainText(text);
    }
    else
    {
        m_allLines.append(text);
        if (const vsizetype diff = m_allLines.size() - maximumBlockCount(); diff > 0)
        {
            m_allLines = m_allLines.mid(diff);
        }
        Filter();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPlainTextEdit::MatchParentheses()
{
    QList<QTextEdit::ExtraSelection> const selections;
    setExtraSelections(selections);

    auto *data = static_cast<VTextBlockData *>(textCursor().block().userData());

    if (data)
    {
        QVector<ParenthesisInfo *> const infos = data->Parentheses();

        int const pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i)
        {
            ParenthesisInfo *info = infos.at(i);

            int const curPos = textCursor().position() - textCursor().block().position();
            if ((info->position == curPos - 1 || info->position == curPos) && info->character == '(')
            {
                CreateParenthesisSelection(pos + static_cast<int>(info->position),
                                           MatchLeftParenthesis(textCursor().block(), i + 1, 0));
                return;
            }

            if ((info->position == curPos - 1 || info->position == curPos) && info->character == ')')
            {
                CreateParenthesisSelection(pos + static_cast<int>(info->position),
                                           MatchRightParenthesis(textCursor().block(), i - 1, 0));
                return;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlainTextEdit::MatchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses) -> bool
{
    auto *data = static_cast<VTextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> const infos = data->Parentheses();

    int const docPos = currentBlock.position();
    for (; i < infos.size(); ++i)
    {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == '(')
        {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == ')' && numLeftParentheses == 0)
        {
            CreateParenthesisSelection(docPos + static_cast<int>(info->position));
            return true;
        }

        --numLeftParentheses;
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
    {
        return MatchLeftParenthesis(currentBlock, 0, numLeftParentheses);
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPlainTextEdit::MatchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses) -> bool
{
    auto *data = static_cast<VTextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> const parentheses = data->Parentheses();

    int const docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i)
    {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == ')')
        {
            ++numRightParentheses;
            continue;
        }

        if (info->character == '(' && numRightParentheses == 0)
        {
            CreateParenthesisSelection(docPos + static_cast<int>(info->position));
            return true;
        }

        --numRightParentheses;
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
    {
        return MatchRightParenthesis(currentBlock, 0, numRightParentheses);
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VPlainTextEdit::CreateParenthesisSelection(int pos, bool match)
{
    if (pos < 0 || pos >= toPlainText().length())
    {
        qDebug() << "String:" << toPlainText() << "Position '" << pos << "' out of range";
        return;
    }

    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    if (match)
    {
        format.setBackground(Qt::lightGray);
        format.setForeground(Qt::yellow);
    }
    else
    {
        format.setForeground(Qt::red);
    }

    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}

//---------------------------------------------------------------------------------------------------------------------
void VPlainTextEdit::Filter()
{
    clear();
    if (not m_filter.isEmpty())
    {
        for (auto &line : m_allLines)
        {
            if (line.contains(m_filter))
            {
                QPlainTextEdit::appendPlainText(line);
            }
        }
    }
    else
    {
        for (auto &line : m_allLines)
        {
            QPlainTextEdit::appendPlainText(line);
        }
    }
}
