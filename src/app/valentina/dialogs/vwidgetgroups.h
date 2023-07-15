/************************************************************************
 **
 **  @file   vwidgetgroups.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 4, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VWIDGETGROUPS_H
#define VWIDGETGROUPS_H

#include "../vmisc/typedef.h"
#include <QWidget>
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class QTableWidgetItem;
class VAbstractPattern;
struct VGroupData;

namespace Ui
{
class VWidgetGroups;
}

class VWidgetGroups : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VWidgetGroups(VAbstractPattern *doc, QWidget *parent = nullptr);
    ~VWidgetGroups() override;

public slots:
    void UpdateGroups();

private slots:
    void GroupVisibilityChanged(int row, int column);
    void RenameGroup(int row, int column);
    void CtxMenu(const QPoint &pos);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VWidgetGroups) // NOLINT
    Ui::VWidgetGroups *ui;
    VAbstractPattern *m_doc;

    void FillTable(QMap<quint32, VGroupData> groups);
    void SetGroupVisibility(vidtype id, bool visible) const;
    void SetMultipleGroupsVisibility(const QVector<vidtype> &groups, bool visible) const;

    auto FilterGroups(const QMap<quint32, VGroupData> &groups) -> QMap<quint32, VGroupData>;

    auto GroupRow(vidtype id) const -> int;

    void ActionPreferences(quint32 id);
    void ActionHideAll();
    void ActionShowAll();
};

#endif // VWIDGETGROUPS_H
