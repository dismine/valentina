/************************************************************************
 **
 **  @file   puzzlemainwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef PUZZLEMAINWINDOW_H
#define PUZZLEMAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

namespace Ui {
class PuzzleMainWindow;
}

class PuzzleMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PuzzleMainWindow(QWidget *parent = nullptr);
    virtual ~PuzzleMainWindow();

    bool LoadFile(const QString &path);

private:
    Q_DISABLE_COPY(PuzzleMainWindow)
    Ui::PuzzleMainWindow *ui;

    void InitMenuBar();

private slots:
    void New();
    void Open();
    void Save();
    void SaveAs();
    void ImportRawLayout();
    void CloseLayout();

    void AboutQt();
    void AboutPuzzle();

};

#endif // PUZZLEMAINWINDOW_H
