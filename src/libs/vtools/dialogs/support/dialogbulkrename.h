/************************************************************************
 **
 **  @file   dialogbulkrename.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef DIALOGBULKRENAME_H
#define DIALOGBULKRENAME_H

#include <QDialog>

#include "../vtools/tools/toolsdef.h"

namespace Ui
{
class DialogBulkRename;
}

class DialogBulkRename : public QDialog
{
    Q_OBJECT // NOLINT

public:
    DialogBulkRename(const QVector<SourceItem> &items, const VContainer *data, QWidget *parent = nullptr);
    ~DialogBulkRename() override;

    /** Returns true if the user actually changed at least one name. */
    auto HasChanges() const -> bool;

    /**
     * Returns the full list of SourceItems with updated names for
     * enabled rows that were renamed. Disabled or unchanged rows
     * are returned as-is.
     */
    auto RenamedItems() const -> QVector<SourceItem>;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void UpdatePreview();

private:
    Q_DISABLE_COPY_MOVE(DialogBulkRename)
    Ui::DialogBulkRename *ui;
    QVector<SourceItem> m_items;
    const VContainer *m_data;

    void PopulateTable();
    auto BuildNewName(bool enabled, quint32 id, int &orderIndex) const -> QString;
    auto FullBaseName(quint32 id) const -> QString;
    auto NewFullBaseName(const QString &base, quint32 id) const -> QString;
    auto BaseName(quint32 id) const -> QString;
    auto IsRowEnabled(int row) const -> bool;
    auto GetName(quint32 id) const -> QString;
    void ValidateNames();
    auto IsNameValid(const QString &name, int currentRow) const -> bool;
};

#endif // DIALOGBULKRENAME_H
