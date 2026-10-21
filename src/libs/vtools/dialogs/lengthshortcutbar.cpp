/************************************************************************
 **
 **  @file   lengthshortcutbar.cpp
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

#include "lengthshortcutbar.h"

#include "formulacommands.h"

#include <QAction>
#include <QBoxLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QPlainTextEdit>
#include <QToolButton>

#include "../vmisc/def.h"

//---------------------------------------------------------------------------------------------------------------------
LengthShortcutBar::LengthShortcutBar(QPlainTextEdit *formula, QWidget *parent)
  : QWidget(parent),
    m_formula(formula),
    m_lengthButton(new QToolButton(this)),
    m_wrapButton(new QToolButton(this)),
    m_halfAction(new QAction(this)),
    m_thirdAction(new QAction(this)),
    m_wrapHalfAction(new QAction(this)),
    m_wrapThirdAction(new QAction(this))
{
    SCASSERT(formula != nullptr)

    m_lengthButton->setObjectName(QStringLiteral("lengthShortcutLength"));
    m_wrapButton->setObjectName(QStringLiteral("lengthShortcutWrap"));
    m_halfAction->setObjectName(QStringLiteral("lengthShortcutHalf"));
    m_thirdAction->setObjectName(QStringLiteral("lengthShortcutThird"));
    m_wrapHalfAction->setObjectName(QStringLiteral("lengthShortcutWrapHalf"));
    m_wrapThirdAction->setObjectName(QStringLiteral("lengthShortcutWrapThird"));

    m_lengthButton->setFocusPolicy(Qt::NoFocus);
    m_wrapButton->setFocusPolicy(Qt::NoFocus);
    m_lengthButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_wrapButton->setPopupMode(QToolButton::InstantPopup);
    m_lengthButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_wrapButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_wrapButton->setText(QStringLiteral("()"));
    m_wrapHalfAction->setText(QStringLiteral("()/2"));
    m_wrapThirdAction->setText(QStringLiteral("()/3"));

    auto *lengthMenu = new QMenu(m_lengthButton);
    lengthMenu->addAction(m_halfAction);
    lengthMenu->addAction(m_thirdAction);
    m_lengthButton->setMenu(lengthMenu);

    auto *wrapMenu = new QMenu(m_wrapButton);
    wrapMenu->addAction(m_wrapHalfAction);
    wrapMenu->addAction(m_wrapThirdAction);
    m_wrapButton->setMenu(wrapMenu);

    connect(m_lengthButton, &QToolButton::clicked, this, [this]() { InsertFormulaSnippet(m_formula, m_token); });
    connect(m_halfAction, &QAction::triggered, this, [this]() {
        InsertFormulaSnippet(m_formula, m_token + QStringLiteral("/2"));
    });
    connect(m_thirdAction, &QAction::triggered, this, [this]() {
        InsertFormulaSnippet(m_formula, m_token + QStringLiteral("/3"));
    });
    connect(m_wrapHalfAction, &QAction::triggered, this, [this]() { WrapFormulaSelection(m_formula, 2); });
    connect(m_wrapThirdAction, &QAction::triggered, this, [this]() { WrapFormulaSelection(m_formula, 3); });
    connect(m_formula, &QPlainTextEdit::copyAvailable, m_wrapButton, &QWidget::setEnabled);
    m_wrapButton->setEnabled(m_formula->textCursor().hasSelection());
    m_lengthButton->setVisible(false);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_lengthButton);
    layout->addWidget(m_wrapButton);

    Retranslate();
}

//---------------------------------------------------------------------------------------------------------------------
void LengthShortcutBar::SetLengthToken(const QString &token)
{
    m_token = token;
    const bool hasToken = not token.isEmpty();
    m_lengthButton->setVisible(hasToken);
    if (not hasToken)
    {
        m_lengthButton->setText(QString());
        m_halfAction->setText(QString());
        m_thirdAction->setText(QString());
        return;
    }

    m_lengthButton->setText(token);
    m_halfAction->setText(token + QStringLiteral("/2"));
    m_thirdAction->setText(token + QStringLiteral("/3"));
}

//---------------------------------------------------------------------------------------------------------------------
void LengthShortcutBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        Retranslate();
    }

    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void LengthShortcutBar::Retranslate()
{
    m_lengthButton->setToolTip(tr("Insert the length of the line or curve"));
    m_halfAction->setToolTip(tr("Insert half of that length"));
    m_thirdAction->setToolTip(tr("Insert one third of that length"));
    m_wrapButton->setToolTip(tr("Wrap the selected text and divide it"));
    m_wrapHalfAction->setToolTip(tr("Wrap the selected text as (selection)/2"));
    m_wrapThirdAction->setToolTip(tr("Wrap the selected text as (selection)/3"));
}

//---------------------------------------------------------------------------------------------------------------------
auto InstallLengthShortcutBar(QPlainTextEdit *formula, QBoxLayout *row, QWidget *before) -> LengthShortcutBar *
{
    SCASSERT(formula != nullptr)
    SCASSERT(row != nullptr)
    SCASSERT(before != nullptr)

    auto *bar = new LengthShortcutBar(formula, row->parentWidget());
    const int index = row->indexOf(before);
    SCASSERT(index >= 0)
    row->insertWidget(index, bar);
    return bar;
}
