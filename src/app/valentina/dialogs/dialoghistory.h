/************************************************************************
 **
 **  @file   dialoghistory.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef DIALOGHISTORY_H
#define DIALOGHISTORY_H

#include "../vtools/dialogs/tools/dialogtool.h"

#include <QDomElement>

class VPattern;
class VTableSearch;

struct HistoryRecord
{
    QString description{};
    quint32 id{NULL_ID};
};

namespace Ui
{
class DialogHistory;
}

/**
 * @brief The DialogHistory class show dialog history.
 */
class DialogHistory : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogHistory(VContainer *data, VPattern *doc, QWidget *parent = nullptr);
    ~DialogHistory() override;
public slots:
    void DialogAccepted() override;
    /** TODO ISSUE 79 : create real function
     * @brief DialogApply apply data and emit signal about applied dialog.
     */
    void DialogApply() override {}
    void cellClicked(int row, int column);
    void ChangedCursor(quint32 id);
    void UpdateHistory();
signals:
    /**
     * @brief ShowHistoryTool signal change color of selected in records tool
     * @param id id of tool
     * @param enable true enable selection, false disable selection
     */
    void ShowHistoryTool(quint32 id, bool enable);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto IsValid() const -> bool final { return true; }
    void showEvent(QShowEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogHistory) // NOLINT

    /** @brief ui keeps information about user interface */
    Ui::DialogHistory *ui;

    /** @brief cursorRow save number of row where is cursor */
    qint32 m_cursorRow{0};

    /** @brief cursorToolRecordRow save number of row selected record */
    qint32 m_cursorToolRecordRow{0};
    QSharedPointer<VTableSearch> m_search{};

    QMenu *m_searchHistory;

    void FillTable();
    auto Record(const VToolRecord &tool) const -> HistoryRecord;
    auto RecordDescription(const VToolRecord &tool, HistoryRecord record, const QDomElement &domElem) const
        -> HistoryRecord;
    void InitialTable();
    void ShowPoint();
    auto PointName(quint32 pointId) const -> QString;
    void RetranslateUi();
    auto CursorRow() const -> int;

    void InitSearch();
    void InitSearchHistory();
    void SaveSearchRequest();
    void UpdateSearchControlsTooltips();
};

#endif // DIALOGHISTORY_H
