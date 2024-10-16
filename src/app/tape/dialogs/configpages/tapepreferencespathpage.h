/************************************************************************
 **
 **  @file   tapepreferencespathpage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 10, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef TAPEPREFERENCESPATHPAGE_H
#define TAPEPREFERENCESPATHPAGE_H

#include <QWidget>
#include <memory>

namespace Ui
{
class TapePreferencesPathPage;
}

class TapePreferencesPathPage : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit TapePreferencesPathPage(QWidget *parent = nullptr);
    ~TapePreferencesPathPage() override;

    auto Apply() -> QStringList;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void DefaultPath();
    void EditPath();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(TapePreferencesPathPage) // NOLINT
    std::unique_ptr<Ui::TapePreferencesPathPage> ui;

    void InitTable();
};

#endif // TAPEPREFERENCESPATHPAGE_H
