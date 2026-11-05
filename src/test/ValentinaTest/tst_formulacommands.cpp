/************************************************************************
 **
 **  @file   tst_formulacommands.cpp
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

#include "tst_formulacommands.h"

#include "../../libs/vtools/dialogs/formulacommands.h"
#include "../../libs/vtools/dialogs/lengthshortcutbar.h"

#include <QAction>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QScopeGuard>
#include <QTextCursor>
#include <QToolButton>
#include <QWidget>
#include <QtTest>

namespace
{
void SelectRange(QPlainTextEdit *formula, int start, int end)
{
    QTextCursor cursor = formula->textCursor();
    cursor.setPosition(start);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    formula->setTextCursor(cursor);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_FormulaCommands::TST_FormulaCommands(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::PlainLengthLiteralRecognizesClickedDistances()
{
    const QLocale previous = QLocale();
    QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
    const auto restoreLocale = qScopeGuard([previous]() { QLocale::setDefault(previous); });

    QVERIFY(IsPlainLengthLiteral(QStringLiteral("6.26565")));
    QVERIFY(IsPlainLengthLiteral(QStringLiteral("-12.5")));
    QVERIFY(IsPlainLengthLiteral(QStringLiteral("+2")));
    QVERIFY(IsPlainLengthLiteral(QStringLiteral("0")));
    QVERIFY(IsPlainLengthLiteral(QStringLiteral("6,26565")));
    QVERIFY(IsPlainLengthLiteral(QStringLiteral("1.234,56")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::PlainLengthLiteralRejectsFormulas()
{
    QVERIFY(not IsPlainLengthLiteral(QString()));
    QVERIFY(not IsPlainLengthLiteral(QStringLiteral("   ")));
    QVERIFY(not IsPlainLengthLiteral(QStringLiteral("CurrentLength")));
    QVERIFY(not IsPlainLengthLiteral(QStringLiteral("CurrentLength/2")));
    QVERIFY(not IsPlainLengthLiteral(QStringLiteral("6.26565*2")));
    QVERIFY(not IsPlainLengthLiteral(QStringLiteral("2*")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::InsertReplacesClickedDistance()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("6.26565"));
    QTextCursor cursor = formula.textCursor();
    cursor.setPosition(2);
    formula.setTextCursor(cursor);

    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength"));

    QCOMPARE(formula.toPlainText(), QStringLiteral("CurrentLength"));

    formula.setPlainText(QStringLiteral("-3.5"));
    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength/2"));
    QCOMPARE(formula.toPlainText(), QStringLiteral("CurrentLength/2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::InsertReplacesOnlyTheSelection()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("2*3"));
    SelectRange(&formula, 2, 3);

    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength"));

    QCOMPARE(formula.toPlainText(), QStringLiteral("2*CurrentLength"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::InsertAppendsToStartedFormula()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("2*"));
    QTextCursor cursor = formula.textCursor();
    cursor.movePosition(QTextCursor::End);
    formula.setTextCursor(cursor);

    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength/2"));

    QCOMPARE(formula.toPlainText(), QStringLiteral("2*CurrentLength/2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::InsertThirdReplacesClickedDistance()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("-3.5"));

    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength/3"));

    QCOMPARE(formula.toPlainText(), QStringLiteral("CurrentLength/3"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::InsertThirdAppendsToStartedFormula()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("2*"));
    QTextCursor cursor = formula.textCursor();
    cursor.movePosition(QTextCursor::End);
    formula.setTextCursor(cursor);

    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength/3"));

    QCOMPARE(formula.toPlainText(), QStringLiteral("2*CurrentLength/3"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::WrapSelectionWithThird()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("Line*2"));
    SelectRange(&formula, 0, 4);

    WrapFormulaSelection(&formula, 3);

    QCOMPARE(formula.toPlainText(), QStringLiteral("(Line)/3*2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::WrapSelectionWithHalf()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("Line*2"));
    SelectRange(&formula, 0, 4);

    WrapFormulaSelection(&formula, 2);

    QCOMPARE(formula.toPlainText(), QStringLiteral("(Line)/2*2"));

    formula.setPlainText(QStringLiteral("%1+2"));
    SelectRange(&formula, 0, 2);
    WrapFormulaSelection(&formula, 2);
    QCOMPARE(formula.toPlainText(), QStringLiteral("(%1)/2+2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::WrapWithoutSelectionLeavesFormula()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("CurrentLength"));
    QTextCursor cursor = formula.textCursor();
    cursor.movePosition(QTextCursor::End);
    formula.setTextCursor(cursor);

    WrapFormulaSelection(&formula, 2);

    QCOMPARE(formula.toPlainText(), QStringLiteral("CurrentLength"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::LengthShortcutBarFollowsTheEditor()
{
    QWidget page;
    auto *formula = new QPlainTextEdit(&page);
    auto *bar = new LengthShortcutBar(formula, &page);
    bar->SetLengthToken(QStringLiteral("CurrentLength"));

    auto *length = bar->findChild<QToolButton *>(QStringLiteral("lengthShortcutLength"));
    auto *wrap = bar->findChild<QToolButton *>(QStringLiteral("lengthShortcutWrap"));
    auto *half = bar->findChild<QAction *>(QStringLiteral("lengthShortcutHalf"));
    auto *third = bar->findChild<QAction *>(QStringLiteral("lengthShortcutThird"));
    auto *wrapHalf = bar->findChild<QAction *>(QStringLiteral("lengthShortcutWrapHalf"));
    auto *wrapThird = bar->findChild<QAction *>(QStringLiteral("lengthShortcutWrapThird"));
    QVERIFY(length != nullptr);
    QVERIFY(wrap != nullptr);
    QVERIFY(half != nullptr);
    QVERIFY(third != nullptr);
    QVERIFY(wrapHalf != nullptr);
    QVERIFY(wrapThird != nullptr);

    auto *layout = new QHBoxLayout(&page);
    layout->addWidget(formula);
    layout->addWidget(bar);
    page.resize(760, 80);
    page.show();

    formula->setPlainText(QStringLiteral("6.26565"));
    QVERIFY(length->isVisible());
    QVERIFY(not wrap->isEnabled());
    QCOMPARE(half->text(), QStringLiteral("CurrentLength/2"));
    QCOMPARE(third->text(), QStringLiteral("CurrentLength/3"));
    QCOMPARE(wrapHalf->text(), QStringLiteral("()/2"));
    QCOMPARE(wrapThird->text(), QStringLiteral("()/3"));
    QCOMPARE(length->toolTip(), QStringLiteral("Insert the length of the line or curve"));
    QCOMPARE(half->toolTip(), QStringLiteral("Insert half of that length"));
    QCOMPARE(third->toolTip(), QStringLiteral("Insert one third of that length"));
    QCOMPARE(wrap->toolTip(), QStringLiteral("Wrap the selected text and divide it"));
    QCOMPARE(wrapHalf->toolTip(), QStringLiteral("Wrap the selected text as (selection)/2"));
    QCOMPARE(wrapThird->toolTip(), QStringLiteral("Wrap the selected text as (selection)/3"));

    // click() emits clicked(). A pixel click can land on the menu arrow, and this is the only
    // test that waits for a window to be exposed.
    length->click();
    QCOMPARE(formula->toPlainText(), QStringLiteral("CurrentLength"));
    QVERIFY(not wrap->isEnabled());

    formula->setPlainText(QStringLiteral("6.26565"));
    half->trigger();
    QCOMPARE(formula->toPlainText(), QStringLiteral("CurrentLength/2"));

    formula->setPlainText(QStringLiteral("6.26565"));
    third->trigger();
    QCOMPARE(formula->toPlainText(), QStringLiteral("CurrentLength/3"));

    formula->setPlainText(QStringLiteral("Line*2"));
    SelectRange(formula, 0, 4);
    QVERIFY(wrap->isEnabled());
    wrapHalf->trigger();
    QCOMPARE(formula->toPlainText(), QStringLiteral("(Line)/2*2"));
    QVERIFY(not wrap->isEnabled());

    formula->setPlainText(QStringLiteral("Line*2"));
    SelectRange(formula, 0, 4);
    wrapThird->trigger();
    QCOMPARE(formula->toPlainText(), QStringLiteral("(Line)/3*2"));

    bar->SetLengthToken(QString());
    QVERIFY(not length->isVisible());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCommands::UndoRestoresThePreviousFormula()
{
    QPlainTextEdit formula;
    formula.setPlainText(QStringLiteral("6.26565"));
    InsertFormulaSnippet(&formula, QStringLiteral("CurrentLength"));
    QVERIFY(formula.document()->isUndoAvailable());

    formula.undo();

    QCOMPARE(formula.toPlainText(), QStringLiteral("6.26565"));
}
