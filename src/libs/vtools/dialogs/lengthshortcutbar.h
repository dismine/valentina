/************************************************************************
 **
 **  @file   lengthshortcutbar.h
 **  @author Valentina contributors
 **  @date   20 10, 2026
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

#ifndef LENGTHSHORTCUTBAR_H
#define LENGTHSHORTCUTBAR_H

#include <QtGlobal>
#include <QWidget>

class QAction;
class QBoxLayout;
class QPlainTextEdit;
class QToolButton;

class LengthShortcutBar : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit LengthShortcutBar(QPlainTextEdit *formula, QWidget *parent = nullptr);

    void SetLengthToken(const QString &token);

protected:
    void changeEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(LengthShortcutBar) // NOLINT

    QPlainTextEdit *m_formula;
    QToolButton *m_lengthButton;
    QToolButton *m_wrapButton;
    QAction *m_halfAction;
    QAction *m_thirdAction;
    QAction *m_wrapHalfAction;
    QAction *m_wrapThirdAction;
    QString m_token{};

    void Retranslate();
};

auto InstallLengthShortcutBar(QPlainTextEdit *formula, QBoxLayout *row, QWidget *before) -> LengthShortcutBar *;

#endif // LENGTHSHORTCUTBAR_H
