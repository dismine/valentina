/************************************************************************
 **
 **  @file   vtablesearch.h
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

#ifndef VTABLESEARCH_H
#define VTABLESEARCH_H

#include <QObject>
#include <QList>
#include <QString>
#include <QTableWidget>
#include <QtGlobal>

#include "../vmisc/defglobal.h"

class VTableSearch: public QObject
{
    Q_OBJECT // NOLINT
public:
    explicit VTableSearch(QTableWidget *table, QObject *parent = nullptr);

    void Find(const QString &term);
    void FindPrevious();
    void FindNext();
    void RemoveRow(int row);
    void AddRow(vsizetype row);
    void RefreshList(const QString &term);

    void SetMatchCase(bool value);
    auto IsMatchCase() const -> bool;

    void SetMatchWord(bool value);
    auto IsMatchWord() const -> bool;

    void SetMatchRegexp(bool value);
    auto IsMatchRegexp() const -> bool;

    void SetUseUnicodePreperties(bool value);
    auto IsUseUnicodePreperties() const -> bool;

    auto MatchIndex() const -> vsizetype;
    auto MatchCount() const -> vsizetype;

    auto SearchPlaceholder() const -> QString;

    static const int MaxHistoryRecords;

signals:
    void HasResult(bool state);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VTableSearch) // NOLINT

    QTableWidget *table;
    vsizetype searchIndex{-1};
    QList<QTableWidgetItem *> searchList{};

    bool m_matchCase{false};
    bool m_matchWord{false};
    bool m_matchRegexp{false};
    bool m_useUnicodePreperties{false};

    void Clear();
    void ShowNext(vsizetype newIndex);
    auto FindTableItems(QString term) -> QList<QTableWidgetItem *>;
    auto FindCurrentMatchIndex() const -> int;
};

#endif // VTABLESEARCH_H
