/************************************************************************
 **
 **  @file   dialogmdatabase.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 7, 2015
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

#ifndef DIALOGMDATABASE_H
#define DIALOGMDATABASE_H

#include <QDialog>

#include "../vmisc/defglobal.h"

namespace Ui
{
    class DialogMDataBase;
}

class QTreeWidgetItem;

class DialogMDataBase : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogMDataBase(const QStringList &list, QWidget *parent = nullptr);
    explicit DialogMDataBase(QWidget *parent = nullptr);
    ~DialogMDataBase() override;

    auto GetNewNames() const -> QStringList;

    void RetranslateGroups();

    static auto ImgTag(const QString &number) -> QString;

protected:
    void changeEvent(QEvent* event) override;
    auto eventFilter(QObject *target, QEvent *event) -> bool override;

private slots:
    void UpdateChecks(QTreeWidgetItem *item, int column);
    void ShowDescription(QTreeWidgetItem *item, int column);
    void TreeMenu(const QPoint &pos);
    void Recheck();
    void FilterMeasurements(const QString &search);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogMDataBase) // NOLINT
    Ui::DialogMDataBase *ui;
    bool m_selectMode;
    QStringList m_list{};

    QTreeWidgetItem *m_groupA{nullptr};
    QTreeWidgetItem *m_groupB{nullptr};
    QTreeWidgetItem *m_groupC{nullptr};
    QTreeWidgetItem *m_groupD{nullptr};
    QTreeWidgetItem *m_groupE{nullptr};
    QTreeWidgetItem *m_groupF{nullptr};
    QTreeWidgetItem *m_groupG{nullptr};
    QTreeWidgetItem *m_groupH{nullptr};
    QTreeWidgetItem *m_groupI{nullptr};
    QTreeWidgetItem *m_groupJ{nullptr};
    QTreeWidgetItem *m_groupK{nullptr};
    QTreeWidgetItem *m_groupL{nullptr};
    QTreeWidgetItem *m_groupM{nullptr};
    QTreeWidgetItem *m_groupN{nullptr};
    QTreeWidgetItem *m_groupO{nullptr};
    QTreeWidgetItem *m_groupP{nullptr};
    QTreeWidgetItem *m_groupQ{nullptr};

    void InitDataBase(const QStringList &list = QStringList());
    void InitGroup(QTreeWidgetItem **group, const QString &groupName, const QStringList &mList,
                   const QStringList &list = QStringList());
    static void FilterGroup(QTreeWidgetItem *group, const QString &search);

    Q_REQUIRED_RESULT auto AddGroup(const QString &text) -> QTreeWidgetItem *;

    void AddMeasurement(QTreeWidgetItem *group, const QString &name, const QStringList &list);

    void ReadSettings();
    void WriteSettings();

    static auto ItemFullDescription(QTreeWidgetItem *item, bool showImage = true) -> QString;

    static void RetranslateGroup(QTreeWidgetItem *group, const QString &groupText, const QStringList &list);
    static void RetranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name);

    static void ChangeCheckState(QTreeWidgetItem *group, Qt::CheckState check);
    auto GlobalCheckState() const -> Qt::CheckState;
};

#endif // DIALOGMDATABASE_H
