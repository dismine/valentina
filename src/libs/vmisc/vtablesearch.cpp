/************************************************************************
 **
 **  @file   vtablesearch.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 9, 2015
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

#include "vtablesearch.h"

#include <QPalette>
#include <QStringBuilder>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <Qt>

#include "../vmisc/def.h"

const int VTableSearch::MaxHistoryRecords = 10;

//---------------------------------------------------------------------------------------------------------------------
VTableSearch::VTableSearch(QTableWidget *table, QObject *parent)
    : QObject(parent),
      table(table)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::Clear()
{
    SCASSERT(table != nullptr)

    for(int i = 0; i < table->rowCount(); ++i)
    {
        for(int j = 0; j < table->columnCount(); ++j)
        {
            if (QTableWidgetItem *item = table->item(i, j))
            {
                if (item->row() % 2 != 0 && table->alternatingRowColors())
                {
                    item->setBackground(QPalette().alternateBase());
                }
                else
                {
                    item->setBackground(QPalette().base());
                }
            }
        }
    }

    searchList.clear();
    searchIndex = -1;

    emit HasResult(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::ShowNext(int newIndex)
{
    if (not searchList.isEmpty())
    {
        QTableWidgetItem *item = searchList.at(searchIndex);
        item->setBackground(Qt::yellow);

        item = searchList.at(newIndex);
        item->setBackground(Qt::red);
        table->scrollToItem(item);
        searchIndex = newIndex;
    }
    else
    {
        Clear();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::FindTableItems(QString term) -> QList<QTableWidgetItem *>
{
    if (term.isEmpty())
    {
        return {};
    }

    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;

    if (not m_matchCase)
    {
        options |= QRegularExpression::CaseInsensitiveOption;
    }

    if (m_matchWord)
    {
        options |= QRegularExpression::UseUnicodePropertiesOption;
        term = "\\b" % term % "\\b";
    }

    if (m_matchRegexp && m_useUnicodePreperties)
    {
        options |= QRegularExpression::UseUnicodePropertiesOption;
    }

    QRegularExpression re(term, options);

    if(not re.isValid())
    {
        return {};
    }

    QList<QTableWidgetItem *> list;

    for (int r=0; r<table->rowCount(); ++r)
    {
        for (int c=0; c<table->columnCount(); ++c)
        {
            QTableWidgetItem *cell = table->item(r, c);
            if (cell != nullptr)
            {
                QString text = cell->text();
                QRegularExpressionMatch match = re.match(text);
                if (match.hasMatch())
                {
                    list.append(cell);
                }
            }
        }
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::FindCurrentMatchIndex() const -> int
{
    if (searchList.isEmpty())
    {
        return 0;
    }

    QList<QTableWidgetItem*> selectedItems = table->selectedItems();
    if (selectedItems.isEmpty())
    {
        return 0;
    }

    QTableWidgetItem* selectedItem = selectedItems.first();

    for (int i=0; i<searchList.size(); ++i)
    {
        QTableWidgetItem* item = searchList.at(i);
        if (item->row()>= selectedItem->row() && item->column()>= selectedItem->column())
        {
            return i;
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::Find(const QString &term)
{
    if(table == nullptr)
    {
        return;
    }

    Clear();

    searchList = FindTableItems(term);

    if (not searchList.isEmpty())
    {
        for (auto *item : qAsConst(searchList))
        {
            item->setBackground(Qt::yellow);
        }

        searchIndex = FindCurrentMatchIndex();
        QTableWidgetItem *item = searchList.at(searchIndex);
        item->setBackground(Qt::red);
        table->scrollToItem(item);

        emit HasResult(true);
        return;
    }

    emit HasResult(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::FindPrevious()
{
    int newIndex = searchIndex - 1;

    if (newIndex < 0)
    {
        newIndex = searchList.size() - 1;
    }

    ShowNext(newIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::FindNext()
{
    int newIndex = searchIndex + 1;

    if (newIndex >= searchList.size())
    {
        newIndex = 0;
    }

    ShowNext(newIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::RemoveRow(int row)
{
    if (searchIndex < 0 || searchIndex >= searchList.size())
    {
        return;
    }

    const int indexRow = searchList.at(searchIndex)->row();

    if (row <= indexRow)
    {
        for (auto *item : qAsConst(searchList))
        {
            if (item->row() == row)
            {
                --searchIndex;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::AddRow(int row)
{
    if (searchIndex < 0 || searchIndex >= searchList.size())
    {
        return;
    }

    const int indexRow = searchList.at(searchIndex)->row();

    if (row <= indexRow)
    {
        for (auto *item : qAsConst(searchList))
        {
            if (item->row() == row)
            {
                ++searchIndex;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::RefreshList(const QString &term)
{
    SCASSERT(table != nullptr)

    searchList = FindTableItems(term);

    if (not searchList.isEmpty())
    {
        for (auto *item : qAsConst(searchList))
        {
            item->setBackground(Qt::yellow);
        }

        if (searchIndex < 0)
        {
           searchIndex = searchList.size() - 1;
        }
        else if (searchIndex >= searchList.size())
        {
           searchIndex = 0;
        }

        QTableWidgetItem *item = searchList.at(searchIndex);
        item->setBackground(Qt::red);
        table->scrollToItem(item);

        emit HasResult(true);
    }
    else
    {
        emit HasResult(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::SetMatchCase(bool value)
{
    m_matchCase = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::IsMatchCase() const -> bool
{
    return m_matchCase;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::SetMatchWord(bool value)
{
    m_matchWord = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::IsMatchWord() const -> bool
{
    return m_matchWord;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::SetMatchRegexp(bool value)
{
    m_matchRegexp = value;
    m_matchWord = false;

    if (not m_matchRegexp)
    {
        m_useUnicodePreperties = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::IsMatchRegexp() const -> bool
{
    return m_matchRegexp;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::SetUseUnicodePreperties(bool value)
{
    m_useUnicodePreperties = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::IsUseUnicodePreperties() const -> bool
{
    return m_useUnicodePreperties;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::MatchIndex() const -> int
{
    return searchIndex;
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::MatchCount() const -> int
{
    return searchList.size();
}

//---------------------------------------------------------------------------------------------------------------------
auto VTableSearch::SearchPlaceholder() const -> QString
{
    if (m_matchCase && not m_matchWord && not m_matchRegexp)
    {
        return tr("Match case");
    }

    if (not m_matchCase && m_matchWord && not m_matchRegexp)
    {
        return tr("Words");
    }

    if (not m_matchCase && m_matchRegexp)
    {
        return tr("Regex");
    }

    if (m_matchCase && m_matchWord)
    {
        return tr("Match case and words");
    }

    if (m_matchCase && m_matchRegexp)
    {
        return tr("Match case and regex");
    }

    return tr("Search");
}
