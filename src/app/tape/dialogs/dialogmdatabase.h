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
#include <QUuid>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

namespace Ui
{
class DialogMDataBase;
}

class QTreeWidgetItem;
struct VKnownMeasurement;
class VPatternImage;

class DialogMDataBase : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogMDataBase(const QUuid &id, const QStringList &usedMeasurements, QWidget *parent = nullptr);
    ~DialogMDataBase() override;

    auto GetNewNames() const -> QStringList;

    static auto ImgTag(const VPatternImage &image) -> QString;

protected:
    void changeEvent(QEvent *event) override;
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
    QStringList m_usedMeasurements{};

    QList<QTreeWidgetItem *> m_groups{};
    QUuid m_knownId;
    QTreeWidgetItem *m_generalGroup{nullptr};

    void InitDataBase(const QStringList &usedMeasurements = QStringList());
    auto InitGroup(const QString &groupName, const QMap<int, VKnownMeasurement> &mlist,
                   const QStringList &list = QStringList()) -> QTreeWidgetItem *;
    void FilterGroup(QTreeWidgetItem *group, const QString &search) const;

    Q_REQUIRED_RESULT auto AddGroup(const QString &text) -> QTreeWidgetItem *;

    void AddMeasurement(QTreeWidgetItem *group, const VKnownMeasurement &measurement, const QStringList &list);

    void ReadSettings();
    void WriteSettings();

    auto ItemFullDescription(QTreeWidgetItem *item, bool showImage = true) const -> QString;

    static void ChangeCheckState(QTreeWidgetItem *group, Qt::CheckState check);
    auto GlobalCheckState() const -> Qt::CheckState;
};

#endif // DIALOGMDATABASE_H
