/************************************************************************
 **
 **  @file   puzzlepreferencesconfigurationpage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 5, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#ifndef PUZZLEPREFERENCESCONFIGURATIONPAGE_H
#define PUZZLEPREFERENCESCONFIGURATIONPAGE_H

#include <QWidget>
#include <memory>

namespace Ui
{
class PuzzlePreferencesConfigurationPage;
}

class PuzzlePreferencesConfigurationPage : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit PuzzlePreferencesConfigurationPage(QWidget *parent = nullptr);
    ~PuzzlePreferencesConfigurationPage() override;

    auto Apply() -> QStringList;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void ShortcutCellDoubleClicked(int row, int column);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(PuzzlePreferencesConfigurationPage) // NOLINT
    std::unique_ptr<Ui::PuzzlePreferencesConfigurationPage> ui;
    bool m_langChanged{false};
    QList<QStringList> m_transientShortcuts{};

    void SetThemeModeComboBox();
    void InitShortcuts(bool defaults = false);
    void UpdateShortcutsTable();
    void RetranslateShortcutsTable();
};

#endif // PUZZLEPREFERENCESCONFIGURATIONPAGE_H
