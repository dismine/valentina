/************************************************************************
 **
 **  @file   formulacommands.h
 **  @author Valentina contributors
 **  @date   16 10, 2026
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

#ifndef FORMULACOMMANDS_H
#define FORMULACOMMANDS_H

#include <QLocale>
#include <QPlainTextEdit>
#include <QString>
#include <QTextCursor>

// Length shortcuts share one editor rule:
// a selection is replaced, a field that is still only the clicked distance is replaced,
// and any formula already in progress keeps its text and receives the snippet at the cursor.

//---------------------------------------------------------------------------------------------------------------------
inline auto IsPlainLengthLiteral(const QString &text) -> bool
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
    {
        return false;
    }

    const auto consumes = [&trimmed](const QLocale &locale) -> bool
    {
        bool ok = false;
        locale.toDouble(trimmed, &ok);
        return ok;
    };

    return consumes(QLocale::c()) || consumes(QLocale());
}

//---------------------------------------------------------------------------------------------------------------------
inline void InsertFormulaSnippet(QPlainTextEdit *formula, const QString &snippet)
{
    QTextCursor cursor = formula->textCursor();
    if (not cursor.hasSelection() && IsPlainLengthLiteral(formula->toPlainText()))
    {
        cursor.select(QTextCursor::Document);
    }

    cursor.insertText(snippet);
    formula->setTextCursor(cursor);
    formula->setFocus(Qt::OtherFocusReason);
}

//---------------------------------------------------------------------------------------------------------------------
inline void WrapFormulaSelection(QPlainTextEdit *formula, int divisor)
{
    QTextCursor cursor = formula->textCursor();
    if (not cursor.hasSelection())
    {
        return;
    }

    const int start = cursor.selectionStart();
    const int end = cursor.selectionEnd();
    const QString selected = formula->toPlainText().mid(start, end - start);
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.insertText(QStringLiteral("(%1)/%2").arg(selected, QString::number(divisor)));
    formula->setTextCursor(cursor);
    formula->setFocus(Qt::OtherFocusReason);
}

#endif // FORMULACOMMANDS_H
