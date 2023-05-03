/************************************************************************
 **
 **  @file   dialogexporttocsv.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 6, 2016
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

#ifndef DIALOGEXPORTTOCSV_H
#define DIALOGEXPORTTOCSV_H

#include <QDialog>

#include "../defglobal.h"

namespace Ui
{
    class DialogExportToCSV;
}

class DialogExportToCSV : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogExportToCSV(QWidget *parent = nullptr);
    virtual ~DialogExportToCSV();

    auto IsWithHeader() const -> bool;
    void SetWithHeader(bool value);

    auto GetSelectedMib() const -> int;
    void SetSelectedMib(int value);

    auto GetSeparator() const -> QChar;
    void  SetSeparator(const QChar &separator);

    void ShowFilePreview(const QString &fileName);

    static auto MakeHelpCodecsList() -> QString;
    static auto MakeHelpSeparatorList() -> QString;

protected:
    virtual void changeEvent(QEvent* event) override;
    virtual void showEvent(QShowEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogExportToCSV) // NOLINT
    Ui::DialogExportToCSV *ui;
    bool isInitialized;
    QString m_fileName;

    void ShowPreview();
};

#endif // DIALOGEXPORTTOCSV_H
