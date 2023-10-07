/************************************************************************
 **
 **  @file   dialoggroup.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 4, 2016
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

#ifndef DIALOGGROUP_H
#define DIALOGGROUP_H

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "dialogtool.h"

namespace Ui
{
class DialogGroup;
}

class DialogGroup : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogGroup(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogGroup() override;

    void SetName(const QString &name);
    auto GetName() const -> QString;

    void SetTags(const QStringList &tags);
    auto GetTags() const -> QStringList;

    void SetGroupCategories(const QStringList &categories) override;

    auto GetGroup() const -> QMap<quint32, quint32>;

    void ShowDialog(bool click) override;

public slots:
    void SelectedObject(bool selected, quint32 object, quint32 tool) override;

protected:
    auto IsValid() const -> bool final;

private slots:
    void NameChanged();

private:
    Q_DISABLE_COPY_MOVE(DialogGroup) // NOLINT
    Ui::DialogGroup *ui;
    QMap<quint32, quint32> group;
    bool flagName;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogGroup::IsValid() const -> bool
{
    return flagName;
}

#endif // DIALOGGROUP_H
