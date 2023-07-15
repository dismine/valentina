/************************************************************************
 **
 **  @file   dialogdatetimeformats.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 8, 2017
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

#ifndef DIALOGDATETIMEFORMATS_H
#define DIALOGDATETIMEFORMATS_H

#include <QDate>
#include <QDialog>
#include <QTime>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

namespace Ui
{
class DialogDateTimeFormats;
}

class QListWidgetItem;

class DialogDateTimeFormats : public QDialog
{
    Q_OBJECT // NOLINT

public:
    DialogDateTimeFormats(const QDate &date, const QStringList &predefinedFormats,
                          const QStringList &userDefinedFormats, QWidget *parent = nullptr);
    DialogDateTimeFormats(const QTime &time, const QStringList &predefinedFormats,
                          const QStringList &userDefinedFormats, QWidget *parent = nullptr);
    ~DialogDateTimeFormats() override;

    auto GetFormats() const -> QStringList;

private slots:
    void AddLine();
    void RemoveLine();
    void SaveFormat(const QString &text);
    void ShowFormatDetails();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogDateTimeFormats) // NOLINT
    Ui::DialogDateTimeFormats *ui;
    bool m_dateMode;
    QDate m_date{};
    QTime m_time{};
    QStringList m_predefined;

    void Init(const QStringList &predefined, const QStringList &userDefined);
    void SetFormatLines(const QStringList &predefined, const QStringList &userDefined);
    void SetupControls();

    auto IsPredefined() const -> bool;

    auto AddListLine(const QString &format) -> QListWidgetItem *;
};

#endif // DIALOGDATETIMEFORMATS_H
